#pragma once

#include <QObject>
#include <QTimer>

namespace volchay {

/**
 * PowerWatcher — лёгкий опрос состояния системы для фоновых решений
 * движка живых обоев:
 *  - fullscreenActive: D3D-приложение на переднем плане или режим презентации
 *    (Win32 SHQueryUserNotificationState). Используется для паузы воспроизведения.
 *  - onBattery: ноутбук работает от батареи (Win32 GetSystemPowerStatus).
 *    Используется для авто-mute, если включён соответствующий пункт настроек.
 *
 * Опрос — раз в секунду, чтобы не нагружать CPU. Сигналы выпускаются только
 * на смене состояния, поэтому подписчики могут реагировать дёшево.
 */
class PowerWatcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool fullscreenActive READ fullscreenActive NOTIFY fullscreenActiveChanged)
    Q_PROPERTY(bool onBattery        READ onBattery        NOTIFY onBatteryChanged)

public:
    explicit PowerWatcher(QObject* parent = nullptr);

    bool fullscreenActive() const { return m_fullscreen; }
    bool onBattery()        const { return m_onBattery; }

signals:
    void fullscreenActiveChanged(bool active);
    void onBatteryChanged(bool on);

private:
    void poll();

    QTimer m_timer;
    bool   m_fullscreen = false;
    bool   m_onBattery  = false;
};

} // namespace volchay
