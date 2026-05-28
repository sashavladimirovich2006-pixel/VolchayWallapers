#include "SystemTray.h"
#include "Logger.h"

#include <QIcon>
#include <QAction>

namespace volchay {

SystemTray::SystemTray(QObject* parent) : QObject(parent) {
    m_icon = new QSystemTrayIcon(this);
    QIcon icon(":/icons/logo.svg");
    if (icon.isNull()) icon = QIcon::fromTheme("applications-multimedia");
    m_icon->setIcon(icon);
    m_icon->setToolTip("Volchay Wallpapers");

    m_menu = new QMenu;
    QAction* show = m_menu->addAction("Открыть");
    QAction* toggle = m_menu->addAction("Включить / выключить обои");
    m_menu->addSeparator();
    QAction* quit = m_menu->addAction("Выход");
    m_icon->setContextMenu(m_menu);

    connect(show,   &QAction::triggered, this, &SystemTray::showRequested);
    connect(toggle, &QAction::triggered, this, &SystemTray::toggleWallpaperRequested);
    connect(quit,   &QAction::triggered, this, &SystemTray::quitRequested);

    connect(m_icon, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::Trigger || r == QSystemTrayIcon::DoubleClick)
            emit showRequested();
    });
}

void SystemTray::show() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        Logger::instance().log(Logger::Warn, "Tray", "System tray not available");
        return;
    }
    m_icon->show();
}

} // namespace volchay
