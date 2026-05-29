#include "PowerWatcher.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <shellapi.h>
#endif

namespace volchay {

PowerWatcher::PowerWatcher(QObject* parent) : QObject(parent) {
    m_timer.setInterval(1000);
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &PowerWatcher::poll);
    m_timer.start();
    poll();
}

void PowerWatcher::poll() {
#ifdef Q_OS_WIN
    // ---------- Fullscreen / busy detection ----------
    QUERY_USER_NOTIFICATION_STATE state = QUNS_NOT_PRESENT;
    bool fullscreen = m_fullscreen;
    if (SUCCEEDED(SHQueryUserNotificationState(&state))) {
        fullscreen = (state == QUNS_BUSY)
                  || (state == QUNS_RUNNING_D3D_FULL_SCREEN)
                  || (state == QUNS_PRESENTATION_MODE);
    }
    if (fullscreen != m_fullscreen) {
        m_fullscreen = fullscreen;
        emit fullscreenActiveChanged(m_fullscreen);
    }

    // ---------- Battery / AC detection ----------
    SYSTEM_POWER_STATUS sps{};
    if (GetSystemPowerStatus(&sps)) {
        // ACLineStatus: 0 = offline (battery), 1 = online, 255 = unknown.
        // На «unknown» сохраняем предыдущее значение, чтобы не дёргать
        // подписчиков ложными переключениями на ноутбуках без датчика.
        if (sps.ACLineStatus != 255) {
            const bool on = (sps.ACLineStatus == 0);
            if (on != m_onBattery) {
                m_onBattery = on;
                emit onBatteryChanged(m_onBattery);
            }
        }
    }
#else
    // Не-Windows платформы: класс — no-op. Поля остаются false навсегда.
#endif
}

} // namespace volchay
