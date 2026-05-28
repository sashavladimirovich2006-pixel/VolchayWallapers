#pragma once

#include <QQuickFramebufferObject>
#include <QString>
#include <QSize>
#include <atomic>

#if VOLCHAY_HAVE_MPV
struct mpv_handle;
struct mpv_render_context;
#endif

namespace volchay {

/**
 * MpvObject — QML-видимый видеоплеер на libmpv, рендерит кадры
 * напрямую в FBO Qt Quick (OpenGL backend).
 *
 * Когда libmpv недоступен на этапе сборки (VOLCHAY_HAVE_MPV=0),
 * объект превращается в безопасную заглушку и пишет в лог
 * предупреждения. Это позволяет CI/dev среде собрать UI без mpv.
 */
class MpvObject : public QQuickFramebufferObject {
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(QString scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)

public:
    explicit MpvObject(QQuickItem* parent = nullptr);
    ~MpvObject() override;

    Renderer* createRenderer() const override;

    QString source() const { return m_source; }
    void setSource(const QString& src);

    bool playing() const { return m_playing; }

    int volume() const { return m_volume; }
    void setVolume(int v);

    bool mute() const { return m_mute; }
    void setMute(bool v);

    QString scaleMode() const { return m_scaleMode; }
    void setScaleMode(const QString& v);

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();

#if VOLCHAY_HAVE_MPV
    mpv_handle*         mpv()        const { return m_mpv; }
    mpv_render_context* renderCtx()  const { return m_renderCtx; }
    void setRenderCtx(mpv_render_context* ctx) { m_renderCtx = ctx; }
#endif

signals:
    void sourceChanged();
    void playingChanged();
    void volumeChanged();
    void muteChanged();
    void scaleModeChanged();
    void mpvError(const QString& message);

private slots:
    void onMpvWakeup();

private:
    void initializeMpv();
    void destroyMpv();
    void command(const QStringList& args);
    void setOption(const QString& name, const QString& value);

    QString m_source;
    bool    m_playing = false;
    int     m_volume  = 0;
    bool    m_mute    = true;
    QString m_scaleMode = "fill";

#if VOLCHAY_HAVE_MPV
    mpv_handle*         m_mpv       = nullptr;
    mpv_render_context* m_renderCtx = nullptr;
#endif
};

} // namespace volchay
