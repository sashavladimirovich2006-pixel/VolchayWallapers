#include "WallpaperEngine.h"
#include "Settings.h"
#include "Logger.h"

#include <QQuickWindow>
#include <QGuiApplication>
#include <QScreen>

#ifdef Q_OS_WIN
  #include <windows.h>
#endif

namespace volchay {

WallpaperEngine::WallpaperEngine(Settings* settings, QObject* parent)
    : QObject(parent), m_settings(settings)
{
    if (m_settings) {
        connect(m_settings, &Settings::targetMonitorChanged,
                this, &WallpaperEngine::resyncGeometry);
    }
    // Re-publish monitors list whenever displays change.
    if (auto* gui = qGuiApp) {
        connect(gui, &QGuiApplication::screenAdded,   this, &WallpaperEngine::monitorsChanged);
        connect(gui, &QGuiApplication::screenRemoved, this, &WallpaperEngine::monitorsChanged);
    }
}

WallpaperEngine::~WallpaperEngine() {
    detach();
}

QStringList WallpaperEngine::monitors() const {
    QStringList out;
    out << QStringLiteral("Все мониторы (виртуальный рабочий стол)");
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        const QRect g = screens.at(i)->geometry();
        const QString name = screens.at(i)->name();
        out << QStringLiteral("%1. %2x%3 — %4")
                  .arg(i + 1).arg(g.width()).arg(g.height()).arg(name);
    }
    return out;
}

QRect WallpaperEngine::computeTargetGeometry() const {
    const auto screens = QGuiApplication::screens();
    if (screens.isEmpty()) return {};
    const int idx = m_settings ? m_settings->targetMonitor() : -1;
    if (idx < 0) {
        // Virtual desktop: union all screens
        QRect u;
        for (auto* s : screens) u = u.united(s->geometry());
        return u;
    }
    if (idx < screens.size()) return screens.at(idx)->geometry();
    return screens.first()->geometry();
}

#ifdef Q_OS_WIN

struct EnumCtx {
    HWND result = nullptr;
    int  totalWorkerW = 0;
    int  withDefView  = 0;
};

static BOOL CALLBACK enumProc(HWND top, LPARAM lp) {
    auto* ctx = reinterpret_cast<EnumCtx*>(lp);

    wchar_t cls[64] = {0};
    GetClassNameW(top, cls, 64);
    if (lstrcmpW(cls, L"WorkerW") != 0) return TRUE;

    ctx->totalWorkerW++;
    HWND defView = FindWindowExW(top, nullptr, L"SHELLDLL_DefView", nullptr);
    if (defView) {
        // This is the "icons" WorkerW — it hosts SHELLDLL_DefView/SysListView32.
        // We do NOT want this one as our parent.
        ctx->withDefView++;
        return TRUE;
    }
    // A top-level WorkerW WITHOUT SHELLDLL_DefView is the wallpaper layer.
    // Take the last such window encountered (closest to desktop in Z-order).
    ctx->result = top;
    return TRUE;
}

void* WallpaperEngine::findWorkerW() {
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman) {
        Logger::instance().log(Logger::Error, "Engine", "Progman not found");
        return nullptr;
    }
    DWORD_PTR result = 0;
    // 0x052C asks Progman to spawn the second WorkerW (the wallpaper layer).
    // Several wparam/lparam combinations are documented for different Windows
    // builds; send all known-good ones.
    SendMessageTimeoutW(progman, 0x052C, 0,    0, SMTO_NORMAL, 1000, &result);
    SendMessageTimeoutW(progman, 0x052C, 0xD,  0, SMTO_NORMAL, 1000, &result);
    SendMessageTimeoutW(progman, 0x052C, 0xD,  1, SMTO_NORMAL, 1000, &result);

    EnumCtx ctx;
    EnumWindows(&enumProc, reinterpret_cast<LPARAM>(&ctx));

    Logger::instance().log(Logger::Info, "Engine",
        QStringLiteral("WorkerW scan: total=%1 with-defview=%2 picked=0x%3")
            .arg(ctx.totalWorkerW)
            .arg(ctx.withDefView)
            .arg(reinterpret_cast<quintptr>(ctx.result), 0, 16));

    if (!ctx.result) {
        // Refuse rather than fall back to Progman — parenting to Progman
        // paints over the desktop icons (black-screen bug).
        Logger::instance().log(Logger::Error, "Engine",
            "WorkerW (wallpaper layer) not found — refusing to attach");
        return nullptr;
    }
    return ctx.result;
}

bool WallpaperEngine::attach(QQuickWindow* window) {
    if (!window) {
        Logger::instance().log(Logger::Error, "Engine", "attach: null window");
        return false;
    }
    detach();

    m_window = window;
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    HWND parent = static_cast<HWND>(findWorkerW());
    if (!parent) {
        emit engineError("Не удалось найти WorkerW");
        return false;
    }
    m_workerW = parent;

    // Remember previous parent and styles so detach() can restore them.
    m_previousParent  = GetParent(hwnd);
    m_previousStyle   = GetWindowLongPtrW(hwnd, GWL_STYLE);
    m_previousExStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);

    SetWindowLongPtrW(hwnd, GWL_STYLE,
        (m_previousStyle & ~(WS_OVERLAPPEDWINDOW)) | WS_CHILD | WS_VISIBLE);
    // WS_EX_LAYERED breaks libmpv's hwdec init on Windows 11 24H2 — and we
    // don't need per-pixel alpha for opaque video. Keep NOACTIVATE+TOOLWINDOW
    // so the host window stays out of Alt-Tab and never steals focus.
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
        (m_previousExStyle & ~WS_EX_LAYERED) | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);

    if (!SetParent(hwnd, parent)) {
        Logger::instance().log(Logger::Error, "Engine",
            QStringLiteral("SetParent failed: %1").arg(GetLastError()));
        emit engineError("SetParent не удался");
        return false;
    }

    // After SetParent, the QQuickWindow loses its normal expose flow. Tell
    // Qt to keep the scene graph + GL context alive even if the platform
    // briefly thinks the window is unexposed, and force a repaint so the
    // MpvObject FBO actually renders and mpv_render_context_create can run.
    m_window->setPersistentSceneGraph(true);
    m_window->setPersistentGraphics(true);

    resyncGeometry();

    m_window->requestUpdate();

    m_active = true;
    emit activeChanged();
    Logger::instance().log(Logger::Info, "Engine", "Wallpaper attached to WorkerW");
    return true;
}

void WallpaperEngine::detach() {
    if (!m_window) {
        m_active = false;
        return;
    }
    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    if (hwnd) {
        SetParent(hwnd, static_cast<HWND>(m_previousParent));
        SetWindowLongPtrW(hwnd, GWL_STYLE,   m_previousStyle);
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, m_previousExStyle);
    }
    m_window.clear();
    m_workerW = nullptr;
    m_previousParent  = nullptr;
    m_previousStyle   = 0;
    m_previousExStyle = 0;
    if (m_active) {
        m_active = false;
        emit activeChanged();
        Logger::instance().log(Logger::Info, "Engine", "Wallpaper detached");
    }
}

void WallpaperEngine::resyncGeometry() {
    if (!m_window) return;
    const QRect r = computeTargetGeometry();
    if (!r.isValid()) return;

    // Tell Qt about the geometry so contentItem (and anchored MpvObject)
    // size correctly — otherwise the FBO is zero-size and never renders.
    m_window->setGeometry(r);

    // ...and force the same on the native HWND. After SetParent to WorkerW
    // the QPlatformWindow may not push setGeometry through to the HWND.
    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    if (hwnd) {
        MoveWindow(hwnd, r.x(), r.y(), r.width(), r.height(), TRUE);
    }

    Logger::instance().log(Logger::Info, "Engine",
        QStringLiteral("geometry %1x%2 @ (%3,%4)")
            .arg(r.width()).arg(r.height()).arg(r.x()).arg(r.y()));
}

#else

bool WallpaperEngine::attach(QQuickWindow*) {
    Logger::instance().log(Logger::Warn, "Engine",
        "WallpaperEngine: only Windows is supported in this build");
    emit engineError("Поддерживается только Windows");
    return false;
}

void WallpaperEngine::detach() {
    if (m_active) { m_active = false; emit activeChanged(); }
}

void WallpaperEngine::resyncGeometry() {}

#endif

} // namespace volchay
