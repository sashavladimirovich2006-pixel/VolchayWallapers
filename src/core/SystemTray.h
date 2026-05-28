#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

namespace volchay {

class SystemTray : public QObject {
    Q_OBJECT
public:
    explicit SystemTray(QObject* parent = nullptr);
    void show();

signals:
    void showRequested();
    void quitRequested();
    void toggleWallpaperRequested();

private:
    QSystemTrayIcon* m_icon = nullptr;
    QMenu*           m_menu = nullptr;
};

} // namespace volchay
