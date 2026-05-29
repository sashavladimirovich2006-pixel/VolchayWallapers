#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QDir>
#include <QSurfaceFormat>
#include <QIcon>

#include "core/Logger.h"
#include "core/Settings.h"
#include "core/ThemeManager.h"
#include "core/WallpaperLibrary.h"
#include "core/WallpaperEngine.h"
#include "core/SystemTray.h"
#include "core/MpvObject.h"
#include "core/PowerWatcher.h"

using namespace volchay;

int main(int argc, char* argv[]) {
    // OpenGL surface for libmpv render (must be set before QApplication).
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setMajorVersion(3);
    fmt.setMinorVersion(2);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication::setApplicationName("VolchayWallpapers");
    QApplication::setOrganizationName("Volchay");
    QApplication::setApplicationDisplayName("Volchay Wallpapers");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setQuitOnLastWindowClosed(false);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/logo.svg"));

    // ----- Logger init -----
    const QString appData =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(appData);
    Logger::instance().init(appData);
    installQtMessageHandler();
    Logger::instance().log(Logger::Info, "App",
        QStringLiteral("Volchay Wallpapers %1 starting; data=%2")
            .arg(QApplication::applicationVersion(), appData));

    // ----- Core singletons -----
    Settings         settings;
    ThemeManager     theme(&settings);
    WallpaperLibrary library(&settings);
    WallpaperEngine  engine(&settings);
    SystemTray       tray;
    PowerWatcher     power;

    // ----- QML -----
    qmlRegisterType<MpvObject>("Volchay.Mpv", 1, 0, "MpvObject");

    QQmlApplicationEngine qml;
    qml.rootContext()->setContextProperty("Settings",  &settings);
    qml.rootContext()->setContextProperty("Theme",     &theme);
    qml.rootContext()->setContextProperty("Library",   &library);
    qml.rootContext()->setContextProperty("Engine",    &engine);
    qml.rootContext()->setContextProperty("Power",     &power);

    QObject::connect(&qml, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() {
        Logger::instance().log(Logger::Fatal, "App", "QML object creation failed");
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    qml.load(QUrl(QStringLiteral("qrc:/src/qml/Main.qml")));
    if (qml.rootObjects().isEmpty()) {
        Logger::instance().log(Logger::Fatal, "App", "Failed to load Main.qml");
        return -1;
    }

    // ----- Tray wiring -----
    QObject::connect(&tray, &SystemTray::showRequested, &app, [&] {
        for (auto* o : qml.rootObjects()) {
            if (auto* w = qobject_cast<QQuickWindow*>(o)) {
                w->show();
                w->raise();
                w->requestActivate();
            }
        }
    });
    QObject::connect(&tray, &SystemTray::quitRequested, &app, [&] {
        engine.detach();
        QCoreApplication::quit();
    });
    QObject::connect(&tray, &SystemTray::toggleWallpaperRequested, &app, [&] {
        const bool desired = !settings.wallpaperEnabled();
        if (desired && settings.currentWallpaper().isEmpty()) {
            Logger::instance().log(Logger::Warn, "Tray",
                "Toggle requested but no wallpaper selected");
            return;
        }
        settings.setWallpaperEnabled(desired);
    });
    tray.show();

    const int rc = app.exec();
    Logger::instance().log(Logger::Info, "App",
        QStringLiteral("Exiting with code %1").arg(rc));
    Logger::instance().shutdown();
    return rc;
}
