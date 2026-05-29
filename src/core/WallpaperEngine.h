#pragma once

#include <QObject>
#include <QRect>
#include <QPointer>

class QQuickWindow;

namespace volchay {

class Settings;

/**
 * WallpaperEngine — внедряет окно живых обоев между Progman/иконками
 * рабочего стола (Windows-only).
 *
 * Алгоритм:
 *  1) FindWindow(Progman)
 *  2) SendMessageTimeout 0x052C — заставить Progman породить WorkerW
 *  3) EnumWindows — найти WorkerW, у которого есть дочерний SHELLDLL_DefView
 *  4) WorkerW (следующий в Z-order, без DefView) — наш родитель
 *  5) SetParent(quickWindow, workerw); MoveWindow(..., virtualDesktop)
 */
class WallpaperEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QStringList monitors READ monitors NOTIFY monitorsChanged)
public:
    explicit WallpaperEngine(Settings* settings, QObject* parent = nullptr);
    ~WallpaperEngine() override;

    bool active() const { return m_active; }

    /// Список «человекочитаемых» имён мониторов: "1: 1920x1080 — Dell U2412M".
    QStringList monitors() const;

    /// Применить переданное Qt-окно как живые обои.
    Q_INVOKABLE bool attach(QQuickWindow* window);

    /// Открепить окно (вернуть его «нормальное» поведение).
    Q_INVOKABLE void detach();

    /// Изменить геометрию окна (например, при смене монитора).
    Q_INVOKABLE void resyncGeometry();

signals:
    void activeChanged();
    void monitorsChanged();
    void engineError(const QString& message);

private:
    QRect computeTargetGeometry() const;
#ifdef Q_OS_WIN
    void* findWorkerW();   // returns HWND
#endif

    Settings*               m_settings = nullptr;
    QPointer<QQuickWindow>  m_window;
    bool                    m_active = false;
#ifdef Q_OS_WIN
    void*                   m_previousParent = nullptr;
    void*                   m_workerW = nullptr;
    // Saved GWL_STYLE / GWL_EXSTYLE so detach() can restore the host window
    // exactly to its pre-attach state. qintptr matches LONG_PTR on x64.
    qintptr                 m_previousStyle = 0;
    qintptr                 m_previousExStyle = 0;
#endif
};

} // namespace volchay
