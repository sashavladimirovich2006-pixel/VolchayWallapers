#include "Settings.h"
#include "Logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#  include <QSettings>
#endif

namespace volchay {

static constexpr auto kOrg = "Volchay";
static constexpr auto kApp = "VolchayWallpapers";

Settings::Settings(QObject* parent)
    : QObject(parent),
      m_s(QSettings::IniFormat,
          QSettings::UserScope,
          QString::fromLatin1(kOrg),
          QString::fromLatin1(kApp))
{
    Logger::instance().log(Logger::Info, "Settings",
        QStringLiteral("Settings file: %1").arg(m_s.fileName()));
}

QString Settings::themeName() const { return m_s.value("ui/theme", "dark").toString(); }
void Settings::setThemeName(const QString& v) {
    if (v == themeName()) return;
    m_s.setValue("ui/theme", v);
    Logger::instance().log(Logger::Info, "Settings", "Theme -> " + v);
    emit themeNameChanged();
}

int Settings::volume() const { return m_s.value("playback/volume", 0).toInt(); }
void Settings::setVolume(int v) {
    if (v == volume()) return;
    m_s.setValue("playback/volume", v);
    emit volumeChanged();
}

bool Settings::muteOnBattery() const { return m_s.value("playback/muteOnBattery", true).toBool(); }
void Settings::setMuteOnBattery(bool v) {
    if (v == muteOnBattery()) return;
    m_s.setValue("playback/muteOnBattery", v);
    emit muteOnBatteryChanged();
}

bool Settings::pauseOnFullscreen() const { return m_s.value("playback/pauseOnFullscreen", true).toBool(); }
void Settings::setPauseOnFullscreen(bool v) {
    if (v == pauseOnFullscreen()) return;
    m_s.setValue("playback/pauseOnFullscreen", v);
    emit pauseOnFullscreenChanged();
}

int Settings::fpsLimit() const { return m_s.value("playback/fpsLimit", 60).toInt(); }
void Settings::setFpsLimit(int v) {
    if (v == fpsLimit()) return;
    m_s.setValue("playback/fpsLimit", v);
    emit fpsLimitChanged();
}

QString Settings::scaleMode() const { return m_s.value("playback/scaleMode", "fill").toString(); }
void Settings::setScaleMode(const QString& v) {
    if (v == scaleMode()) return;
    m_s.setValue("playback/scaleMode", v);
    emit scaleModeChanged();
}

int Settings::targetMonitor() const { return m_s.value("display/monitor", -1).toInt(); }
void Settings::setTargetMonitor(int v) {
    if (v == targetMonitor()) return;
    m_s.setValue("display/monitor", v);
    emit targetMonitorChanged();
}

bool Settings::autoStart() const { return m_s.value("system/autoStart", false).toBool(); }
void Settings::setAutoStart(bool v) {
    if (v == autoStart()) return;
    m_s.setValue("system/autoStart", v);
#ifdef Q_OS_WIN
    QSettings run(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
                  QSettings::NativeFormat);
    if (v) {
        const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        run.setValue("VolchayWallpapers", QString("\"%1\"").arg(exe));
        Logger::instance().log(Logger::Info, "Settings",
            QStringLiteral("Autostart enabled: %1").arg(exe));
    } else {
        run.remove("VolchayWallpapers");
        Logger::instance().log(Logger::Info, "Settings", "Autostart disabled");
    }
#endif
    emit autoStartChanged();
}

QString Settings::currentWallpaper() const { return m_s.value("state/currentWallpaper").toString(); }
void Settings::setCurrentWallpaper(const QString& v) {
    if (v == currentWallpaper()) return;
    m_s.setValue("state/currentWallpaper", v);
    Logger::instance().log(Logger::Info, "Settings",
        QStringLiteral("currentWallpaper -> %1").arg(v));
    emit currentWallpaperChanged();
}

bool Settings::wallpaperEnabled() const { return m_s.value("state/wallpaperEnabled", false).toBool(); }
void Settings::setWallpaperEnabled(bool v) {
    if (v == wallpaperEnabled()) return;
    m_s.setValue("state/wallpaperEnabled", v);
    Logger::instance().log(Logger::Info, "Settings",
        QStringLiteral("Wallpaper %1").arg(v ? "ON" : "OFF"));
    emit wallpaperEnabledChanged();
}

QStringList Settings::libraryPaths() const {
    return m_s.value("library/paths").toStringList();
}
void Settings::addLibraryPath(const QString& path) {
    auto paths = libraryPaths();
    if (paths.contains(path)) return;
    paths.prepend(path);
    m_s.setValue("library/paths", paths);
    Logger::instance().log(Logger::Info, "Settings", "Library + " + path);
    emit libraryChanged();
}
void Settings::removeLibraryPath(const QString& path) {
    auto paths = libraryPaths();
    if (!paths.removeOne(path)) return;
    m_s.setValue("library/paths", paths);
    Logger::instance().log(Logger::Info, "Settings", "Library - " + path);
    emit libraryChanged();
}

} // namespace volchay
