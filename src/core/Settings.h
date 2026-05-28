#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

namespace volchay {

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString themeName READ themeName WRITE setThemeName NOTIFY themeNameChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muteOnBattery READ muteOnBattery WRITE setMuteOnBattery NOTIFY muteOnBatteryChanged)
    Q_PROPERTY(bool pauseOnFullscreen READ pauseOnFullscreen WRITE setPauseOnFullscreen NOTIFY pauseOnFullscreenChanged)
    Q_PROPERTY(int fpsLimit READ fpsLimit WRITE setFpsLimit NOTIFY fpsLimitChanged)
    Q_PROPERTY(QString scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(int targetMonitor READ targetMonitor WRITE setTargetMonitor NOTIFY targetMonitorChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper WRITE setCurrentWallpaper NOTIFY currentWallpaperChanged)
    Q_PROPERTY(bool wallpaperEnabled READ wallpaperEnabled WRITE setWallpaperEnabled NOTIFY wallpaperEnabledChanged)

public:
    explicit Settings(QObject* parent = nullptr);

    QString themeName() const;
    void setThemeName(const QString& v);

    int volume() const;
    void setVolume(int v);

    bool muteOnBattery() const;
    void setMuteOnBattery(bool v);

    bool pauseOnFullscreen() const;
    void setPauseOnFullscreen(bool v);

    int fpsLimit() const;
    void setFpsLimit(int v);

    QString scaleMode() const; // "fill" | "fit" | "stretch" | "center"
    void setScaleMode(const QString& v);

    int targetMonitor() const;
    void setTargetMonitor(int v);

    bool autoStart() const;
    void setAutoStart(bool v);

    QString currentWallpaper() const;
    void setCurrentWallpaper(const QString& v);

    bool wallpaperEnabled() const;
    void setWallpaperEnabled(bool v);

    Q_INVOKABLE QStringList libraryPaths() const;
    Q_INVOKABLE void addLibraryPath(const QString& path);
    Q_INVOKABLE void removeLibraryPath(const QString& path);

signals:
    void themeNameChanged();
    void volumeChanged();
    void muteOnBatteryChanged();
    void pauseOnFullscreenChanged();
    void fpsLimitChanged();
    void scaleModeChanged();
    void targetMonitorChanged();
    void autoStartChanged();
    void currentWallpaperChanged();
    void wallpaperEnabledChanged();
    void libraryChanged();

private:
    QSettings m_s;
};

} // namespace volchay
