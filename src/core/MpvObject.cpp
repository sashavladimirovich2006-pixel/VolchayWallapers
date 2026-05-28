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
    }

    void render() override {
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
    mpv_set_option_string(m_mpv, "msg-level", "all=v");
    mpv_set_option_string(m_mpv, "loop-file", "inf");
    mpv_set_option_string(m_mpv, "keep-open", "yes");
    mpv_set_option_string(m_mpv, "hwdec", "auto-safe");
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "audio", "no");

    if (mpv_initialize(m_mpv) < 0) {
        Logger::instance().log(Logger::Error, "Mpv", "mpv_initialize failed");
        mpv_destroy(m_mpv);
        m_mpv = nullptr;
        emit mpvError("mpv_initialize failed");
        return;
    }

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
        stop();
        return;
    }
    Logger::instance().log(Logger::Info, "Mpv", "loadfile: " + src);
    command({ "loadfile", src, "replace" });
    m_playing = true;
    emit playingChanged();
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
                Logger::instance().log(lvl, QStringLiteral("mpv:%1").arg(msg->prefix),
                                       QString::fromUtf8(msg->text).trimmed());
                break;
            }
            case MPV_EVENT_END_FILE: {
                Logger::instance().log(Logger::Debug, "Mpv", "end-file");
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
