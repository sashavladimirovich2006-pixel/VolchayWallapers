#include "MpvObject.h"
#include "Logger.h"

#include <QMetaObject>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QStringList>
#include <QThread>

#if VOLCHAY_HAVE_MPV
  #include <mpv/client.h>
  #include <mpv/render.h>
  #include <mpv/render_gl.h>
#endif

namespace volchay {

#if VOLCHAY_HAVE_MPV

// --- Renderer ---
class MpvRenderer : public QQuickFramebufferObject::Renderer {
public:
    explicit MpvRenderer(MpvObject* obj) : m_obj(obj) {}

    static void* getProcAddress(void* ctx, const char* name) {
        Q_UNUSED(ctx);
        QOpenGLContext* gl = QOpenGLContext::currentContext();
        if (!gl) return nullptr;
        return reinterpret_cast<void*>(gl->getProcAddress(QByteArray(name)));
    }

    void ensureContext() {
        if (m_obj->renderCtx()) return;
        if (!m_obj->mpv()) return;
        mpv_opengl_init_params gl_init{ &MpvRenderer::getProcAddress, nullptr };
        int advanced = 1;
        mpv_render_param params[]{
            { MPV_RENDER_PARAM_API_TYPE,           const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL) },
            { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init },
            { MPV_RENDER_PARAM_ADVANCED_CONTROL,   &advanced },
            { MPV_RENDER_PARAM_INVALID,            nullptr }
        };
        mpv_render_context* ctx = nullptr;
        int r = mpv_render_context_create(&ctx, m_obj->mpv(), params);
        if (r < 0) {
            Logger::instance().log(Logger::Error, "Mpv",
                QStringLiteral("render_context_create failed: %1").arg(mpv_error_string(r)));
            return;
        }
        m_obj->setRenderCtx(ctx);

        mpv_render_context_set_update_callback(ctx,
            [](void* data) {
                auto* self = static_cast<MpvObject*>(data);
                QMetaObject::invokeMethod(self, "update", Qt::QueuedConnection);
            }, m_obj);

        // Tell the MpvObject (on its own thread) that the render context is
        // up. Until this fires, setSource only stores the path; the actual
        // loadfile happens here. Without this, vo=libmpv has no context to
        // bind to when loadfile arrives and mpv refuses to play video.
        QMetaObject::invokeMethod(m_obj, "onRenderReady", Qt::QueuedConnection);
    }

    void render() override {
        if (!m_loggedFirstRender) {
            Logger::instance().log(Logger::Info, "Mpv",
                "First render() called — scene graph is alive");
            m_loggedFirstRender = true;
        }
        ensureContext();
        auto* ctx = m_obj->renderCtx();
        if (!ctx) return;

        QOpenGLFramebufferObject* fbo = framebufferObject();
        const QSize sz = fbo->size();

        mpv_opengl_fbo mpfbo{ static_cast<int>(fbo->handle()), sz.width(), sz.height(), 0 };
        int flip_y = 1;
        mpv_render_param params[]{
            { MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo },
            { MPV_RENDER_PARAM_FLIP_Y,     &flip_y },
            { MPV_RENDER_PARAM_INVALID,    nullptr }
        };
        if (m_obj->window()) m_obj->window()->beginExternalCommands();
        mpv_render_context_render(ctx, params);
        if (m_obj->window()) m_obj->window()->endExternalCommands();
    }

    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override {
        QOpenGLFramebufferObjectFormat fmt;
        fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        return new QOpenGLFramebufferObject(size, fmt);
    }

private:
    MpvObject* m_obj;
    bool       m_loggedFirstRender = false;
};

#else

// --- Stub renderer when libmpv is unavailable ---
class MpvRenderer : public QQuickFramebufferObject::Renderer {
public:
    explicit MpvRenderer(MpvObject*) {}
    void render() override {}
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override {
        return new QOpenGLFramebufferObject(size);
    }
};

#endif

// --- MpvObject ---

MpvObject::MpvObject(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    setTextureFollowsItemSize(true);
    initializeMpv();

    // Kick the scene graph until the render context exists. Once we've gone
    // through ensureContext() the timer self-stops in onRenderReady().
    m_renderKickTimer.setInterval(100);
    connect(&m_renderKickTimer, &QTimer::timeout, this, [this]() {
        if (m_renderReady) { m_renderKickTimer.stop(); return; }
        // Asking the item to redraw will cause Qt Quick to schedule a paint
        // pass for the host window, which in turn calls MpvRenderer::render
        // → ensureContext → mpv_render_context_create.
        update();
        if (auto* w = window()) w->update();
    });
    m_renderKickTimer.start();
}

MpvObject::~MpvObject() {
    destroyMpv();
}

QQuickFramebufferObject::Renderer* MpvObject::createRenderer() const {
    return new MpvRenderer(const_cast<MpvObject*>(this));
}

void MpvObject::initializeMpv() {
#if VOLCHAY_HAVE_MPV
    m_mpv = mpv_create();
    if (!m_mpv) {
        Logger::instance().log(Logger::Error, "Mpv", "mpv_create() returned null");
        emit mpvError("mpv_create() failed");
        return;
    }
    mpv_set_option_string(m_mpv, "terminal", "no");
    // Want enough chatter to see what's happening when a file fails to load.
    // 'v' on player+ao+vo+demux+vd surfaces track selection / decoder pick /
    // demuxer errors, while leaving the rest of mpv quiet.
    mpv_set_option_string(m_mpv, "msg-level", "all=warn,player=v,ao=v,vo=v,vd=v,demux=v");
    mpv_set_option_string(m_mpv, "loop-file", "inf");
    mpv_set_option_string(m_mpv, "keep-open", "yes");
    // Software decode. hwdec=auto-safe + WS_EX_LAYERED parent (which we used to
    // set in WallpaperEngine::attach) failed to initialize on Windows 11 24H2:
    // mpv emitted MPV_END_FILE_REASON_ERROR right after loadfile and never
    // produced a frame. Wallpapers loop short clips — software decode is
    // cheap and bulletproof.
    mpv_set_option_string(m_mpv, "hwdec", "no");
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    // NOT audio=no (== aid=no). That made mpv refuse to play files whenever
    // the video track failed to auto-select, with the cryptic
    // "no video or audio streams selected" error — because we'd told it
    // there's no audio either. Instead keep audio tracks selectable but
    // muted by default; QML controls mute via the property.
    mpv_set_option_string(m_mpv, "mute", "yes");

    if (mpv_initialize(m_mpv) < 0) {
        Logger::instance().log(Logger::Error, "Mpv", "mpv_initialize failed");
        mpv_destroy(m_mpv);
        m_mpv = nullptr;
        emit mpvError("mpv_initialize failed");
        return;
    }

    // Forward warn+ log lines from libmpv into our logger. Anything ERROR
    // also bubbles up as mpvError() → QML toast (see onMpvWakeup). The
    // msg-level option above keeps the verbose chatter on player/ao/vo/vd/demux,
    // so the log file has decoder pick + track selection trail when a file
    // fails to load.
    mpv_request_log_messages(m_mpv, "v");

    mpv_set_wakeup_callback(m_mpv,
        [](void* data) {
            auto* self = static_cast<MpvObject*>(data);
            QMetaObject::invokeMethod(self, "onMpvWakeup", Qt::QueuedConnection);
        }, this);

    Logger::instance().log(Logger::Info, "Mpv", "Initialized");
#else
    Logger::instance().log(Logger::Warn, "Mpv", "Built without libmpv (stub mode)");
#endif
}

void MpvObject::destroyMpv() {
#if VOLCHAY_HAVE_MPV
    if (m_renderCtx) {
        mpv_render_context_free(m_renderCtx);
        m_renderCtx = nullptr;
    }
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
#endif
}

void MpvObject::command(const QStringList& args) {
#if VOLCHAY_HAVE_MPV
    if (!m_mpv) return;
    QVector<QByteArray> bytes;
    bytes.reserve(args.size());
    for (const auto& a : args) bytes.push_back(a.toUtf8());
    QVector<const char*> argv;
    argv.reserve(bytes.size() + 1);
    for (const auto& b : bytes) argv.push_back(b.constData());
    argv.push_back(nullptr);
    int r = mpv_command(m_mpv, argv.data());
    if (r < 0) {
        Logger::instance().log(Logger::Warn, "Mpv",
            QStringLiteral("command failed (%1): %2")
                .arg(args.join(' '), QString::fromUtf8(mpv_error_string(r))));
    }
#else
    Q_UNUSED(args);
#endif
}

void MpvObject::setOption(const QString& name, const QString& value) {
#if VOLCHAY_HAVE_MPV
    if (!m_mpv) return;
    mpv_set_option_string(m_mpv, name.toUtf8().constData(), value.toUtf8().constData());
#else
    Q_UNUSED(name); Q_UNUSED(value);
#endif
}

void MpvObject::setSource(const QString& src) {
    if (src == m_source) return;
    m_source = src;
    emit sourceChanged();
    if (src.isEmpty()) {
        m_pendingSource.clear();
        stop();
        return;
    }
    if (!m_renderReady) {
        // vo=libmpv would fail right now — render context isn't up yet.
        // Replay this load from onRenderReady().
        m_pendingSource = src;
        Logger::instance().log(Logger::Info, "Mpv",
            QStringLiteral("setSource: render not ready, deferring loadfile: %1").arg(src));
        return;
    }
    Logger::instance().log(Logger::Info, "Mpv", "loadfile: " + src);
    command({ "loadfile", src, "replace" });
    m_playing = true;
    emit playingChanged();
}

void MpvObject::onRenderReady() {
    if (m_renderReady) return; // idempotent: only first context create matters
    m_renderReady = true;
    m_renderKickTimer.stop();
    Logger::instance().log(Logger::Info, "Mpv", "Render context ready");
    if (!m_pendingSource.isEmpty()) {
        const QString src = m_pendingSource;
        m_pendingSource.clear();
        Logger::instance().log(Logger::Info, "Mpv",
            QStringLiteral("Replaying deferred loadfile: %1").arg(src));
        command({ "loadfile", src, "replace" });
        m_playing = true;
        emit playingChanged();
    }
}

void MpvObject::setVolume(int v) {
    v = qBound(0, v, 100);
    if (v == m_volume) return;
    m_volume = v;
    setOption("volume", QString::number(v));
    emit volumeChanged();
}

void MpvObject::setMute(bool v) {
    if (v == m_mute) return;
    m_mute = v;
    setOption("mute", v ? "yes" : "no");
    emit muteChanged();
}

void MpvObject::setScaleMode(const QString& v) {
    if (v == m_scaleMode) return;
    m_scaleMode = v;
    QString panscan = "0";
    QString keep = "yes";
    QString video_unscaled = "no";
    if (v == "fill")      { panscan = "1.0"; keep = "yes"; }
    else if (v == "fit")  { panscan = "0";   keep = "yes"; }
    else if (v == "stretch") { panscan = "0"; keep = "no"; }
    else if (v == "center")  { panscan = "0"; keep = "yes"; video_unscaled = "yes"; }
    setOption("panscan", panscan);
    setOption("keepaspect", keep);
    setOption("video-unscaled", video_unscaled);
    emit scaleModeChanged();
}

void MpvObject::setFpsLimit(int v) {
    v = qBound(1, v, 240);
    if (v == m_fpsLimit) return;
    m_fpsLimit = v;
    // Cap rendering to v fps. display-fps-override pretends the monitor runs
    // at v Hz; combined with video-sync=display-resample mpv won't render
    // more frames than that. Works whether the actual display is 60/120/144 Hz.
    setOption("display-fps-override", QString::number(v));
    setOption("video-sync", "display-resample");
    emit fpsLimitChanged();
}

void MpvObject::play() {
    command({ "set", "pause", "no" });
    m_playing = true;
    emit playingChanged();
}

void MpvObject::pause() {
    command({ "set", "pause", "yes" });
    m_playing = false;
    emit playingChanged();
}

void MpvObject::stop() {
    command({ "stop" });
    m_playing = false;
    emit playingChanged();
}

#if VOLCHAY_HAVE_MPV
// Best-effort fetch of a string property; returns empty if mpv has nothing.
static QString mpvStr(mpv_handle* h, const char* name) {
    if (!h) return {};
    char* s = nullptr;
    if (mpv_get_property(h, name, MPV_FORMAT_STRING, &s) < 0 || !s) return {};
    const QString out = QString::fromUtf8(s);
    mpv_free(s);
    return out;
}
#endif

void MpvObject::onMpvWakeup() {
#if VOLCHAY_HAVE_MPV
    if (!m_mpv) return;
    while (m_mpv) {
        mpv_event* ev = mpv_wait_event(m_mpv, 0);
        if (!ev || ev->event_id == MPV_EVENT_NONE) break;
        switch (ev->event_id) {
            case MPV_EVENT_LOG_MESSAGE: {
                auto* msg = static_cast<mpv_event_log_message*>(ev->data);
                Logger::Level lvl = Logger::Debug;
                if (msg->log_level <= MPV_LOG_LEVEL_ERROR) lvl = Logger::Error;
                else if (msg->log_level <= MPV_LOG_LEVEL_WARN) lvl = Logger::Warn;
                else if (msg->log_level <= MPV_LOG_LEVEL_INFO) lvl = Logger::Info;
                const QString text = QString::fromUtf8(msg->text).trimmed();
                Logger::instance().log(lvl, QStringLiteral("mpv:%1").arg(msg->prefix), text);
                if (msg->log_level <= MPV_LOG_LEVEL_ERROR && !text.isEmpty()) {
                    // Translate the most common opaque mpv messages into
                    // something a non-developer can act on.
                    QString user = QStringLiteral("mpv: %1").arg(text);
                    if (text.contains("no video or audio streams selected", Qt::CaseInsensitive)) {
                        user = QStringLiteral("Файл не содержит распознаваемого видеопотока "
                                              "(или его кодек не поддерживается этой сборкой libmpv).");
                    } else if (text.contains("Failed to recognize file format", Qt::CaseInsensitive)) {
                        user = QStringLiteral("Не удалось распознать формат файла.");
                    } else if (text.contains("Can not open external file", Qt::CaseInsensitive)
                            || text.contains("Failed to open", Qt::CaseInsensitive)) {
                        user = QStringLiteral("Не удалось открыть файл (нет доступа или путь неверен).");
                    }
                    emit mpvError(user);
                }
                break;
            }
            case MPV_EVENT_START_FILE: {
                Logger::instance().log(Logger::Info, "Mpv",
                    QStringLiteral("start-file: %1").arg(m_source));
                break;
            }
            case MPV_EVENT_FILE_LOADED: {
                // mpv successfully parsed the container and chose tracks.
                // Dump everything we need to diagnose stream/codec issues
                // without needing the user to reproduce.
                const QString fmt    = mpvStr(m_mpv, "file-format");
                const QString vcodec = mpvStr(m_mpv, "video-codec");
                const QString acodec = mpvStr(m_mpv, "audio-codec");
                const QString vfmt   = mpvStr(m_mpv, "video-format");
                const QString afmt   = mpvStr(m_mpv, "audio-format");
                int64_t w = 0, h = 0;
                mpv_get_property(m_mpv, "width",  MPV_FORMAT_INT64, &w);
                mpv_get_property(m_mpv, "height", MPV_FORMAT_INT64, &h);
                Logger::instance().log(Logger::Info, "Mpv",
                    QStringLiteral("file-loaded: container=%1 video=%2 (%3) %4x%5 audio=%6 (%7)")
                        .arg(fmt.isEmpty()    ? "?" : fmt,
                             vcodec.isEmpty() ? "—" : vcodec,
                             vfmt.isEmpty()   ? "?" : vfmt)
                        .arg(w).arg(h)
                        .arg(acodec.isEmpty() ? "—" : acodec,
                             afmt.isEmpty()   ? "?" : afmt));

                // Full track-list — verbose but exactly what you want to see
                // when "no streams selected" fires next time. JSON string is
                // what mpv returns for the property in string form.
                const QString tracks = mpvStr(m_mpv, "track-list");
                if (!tracks.isEmpty()) {
                    Logger::instance().log(Logger::Debug, "Mpv",
                        QStringLiteral("track-list: %1").arg(tracks));
                }
                break;
            }
            case MPV_EVENT_END_FILE: {
                auto* ef = static_cast<mpv_event_end_file*>(ev->data);
                const char* reason = "unknown";
                switch (ef->reason) {
                    case MPV_END_FILE_REASON_EOF:      reason = "eof";      break;
                    case MPV_END_FILE_REASON_STOP:     reason = "stop";     break;
                    case MPV_END_FILE_REASON_QUIT:     reason = "quit";     break;
                    case MPV_END_FILE_REASON_ERROR:    reason = "error";    break;
                    case MPV_END_FILE_REASON_REDIRECT: reason = "redirect"; break;
                }
                if (ef->reason == MPV_END_FILE_REASON_ERROR) {
                    const QString err = QString::fromUtf8(mpv_error_string(ef->error));
                    Logger::instance().log(Logger::Error, "Mpv",
                        QStringLiteral("end-file reason=error code=%1 (%2) source=%3")
                            .arg(ef->error).arg(err, m_source));
                    emit mpvError(QStringLiteral("Видео не открылось: %1").arg(err));
                } else {
                    Logger::instance().log(Logger::Debug, "Mpv",
                        QStringLiteral("end-file reason=%1").arg(reason));
                }
                break;
            }
            case MPV_EVENT_SHUTDOWN: {
                Logger::instance().log(Logger::Info, "Mpv", "shutdown");
                return;
            }
            default: break;
        }
    }
#endif
}

} // namespace volchay
