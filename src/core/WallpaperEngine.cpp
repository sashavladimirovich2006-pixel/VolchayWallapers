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
}

WallpaperEngine::~WallpaperEngine() {
    detach();
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
};

static BOOL CALLBACK enumProc(HWND top, LPARAM lp) {
    auto* ctx = reinterpret_cast<EnumCtx*>(lp);
    HWND defView = FindWindowExW(top, nullptr, L"SHELLDLL_DefView", nullptr);
    if (defView) {
        // The WorkerW we want is the NEXT sibling at the top level that
        // does NOT contain SHELLDLL_DefView — it floats above wallpaper, below icons.
        HWND worker = FindWindowExW(nullptr, top, L"WorkerW", nullptr);
        if (worker) ctx->result = worker;
    }
    return TRUE;
}

void* WallpaperEngine::findWorkerW() {
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman) {
        Logger::instance().log(Logger::Error, "Engine", "Progman not found");
        return nullptr;
    }
    DWORD_PTR result = 0;
    // 0x052C is the magic message that asks Progman to spawn a WorkerW behind icons.
    SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, &result);
    SendMessageTimeoutW(progman, 0x052C, 0xD, 0, SMTO_NORMAL, 1000, &result);
    SendMessageTimeoutW(progman, 0x052C, 0xD, 1, SMTO_NORMAL, 1000, &result);

    EnumCtx ctx;
    EnumWindows(&enumProc, reinterpret_cast<LPARAM>(&ctx));
    if (!ctx.result) {
        Logger::instance().log(Logger::Warn, "Engine",
            "WorkerW not found, falling back to Progman");
        return progman;
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

    // Remember previous parent so detach() can restore it
    m_previousParent = GetParent(hwnd);

    SetWindowLongPtrW(hwnd, GWL_STYLE,
        (GetWindowLongPtrW(hwnd, GWL_STYLE) & ~(WS_OVERLAPPEDWINDOW)) | WS_CHILD | WS_VISIBLE);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
        GetWindowLongPtrW(hwnd, GWL_EXSTYLE)
        | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

    if (!SetParent(hwnd, parent)) {
        Logger::instance().log(Logger::Error, "Engine",
            QStringLiteral("SetParent failed: %1").arg(GetLastError()));
        emit engineError("SetParent не удался");
        return false;
    }

    resyncGeometry();
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
        SetWindowLongPtrW(hwnd, GWL_STYLE,
            (GetWindowLongPtrW(hwnd, GWL_STYLE) & ~WS_CHILD) | WS_OVERLAPPEDWINDOW);
    }
    m_window.clear();
    m_workerW = nullptr;
    m_previousParent = nullptr;
    if (m_active) {
        m_active = false;
        emit activeChanged();
        Logger::instance().log(Logger::Info, "Engine", "Wallpaper detached");
    }
}

void WallpaperEngine::resyncGeometry() {
    if (!m_window) return;
    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    const QRect r = computeTargetGeometry();
    if (r.isValid() && hwnd) {
        MoveWindow(hwnd, r.x(), r.y(), r.width(), r.height(), TRUE);
        Logger::instance().log(Logger::Debug, "Engine",
            QStringLiteral("geometry %1x%2 @ (%3,%4)")
                .arg(r.width()).arg(r.height()).arg(r.x()).arg(r.y()));
    }
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
