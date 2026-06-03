#include "WallpaperEngine.h"
#include "Settings.h"
#include "Logger.h"

#include <QQuickWindow>
#include <QGuiApplication>
#include <QScreen>
#include <QExposeEvent>

#ifdef Q_OS_WIN
  #include <windows.h>
  #include <commctrl.h>
  #pragma comment(lib, "comctl32.lib")

  // Subclass procedure that blocks WM_DESTROY and WM_NCDESTROY.
  // When we SetParent a QQuickWindow to WorkerW, Windows/Explorer sends
  // WM_DESTROY (e.g. on desktop refresh, theme change, or display change),
  // which tears down the Qt window and kills the render context.
  // We swallow these messages so Qt stays alive.
  static LRESULT CALLBACK WallpaperSubclassProc(
      HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
      UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
  {
      Q_UNUSED(uIdSubclass); Q_UNUSED(dwRefData); Q_UNUSED(hwnd);
      switch (msg) {
      case WM_DESTROY:
      case WM_NCDESTROY:
          // Block destruction — the QQuickWindow must survive as long as
          // we keep it attached to WorkerW.
          return 0;
      }
      return DefSubclassProc(hwnd, msg, wp, lp);
  }
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

    // Render pump: after SetParent to WorkerW the QQuickWindow stops
    // receiving expose events from the platform plugin, so the render
    // loop can stall. We pump QExposeEvent + update() at ~30 Hz until
    // the first render() occurs, then STOP the pump entirely — mpv's
    // own update callback handles frame delivery from that point.
    m_renderPumpTimer.setInterval(33);  // ~30 Hz
    m_renderPumpFirstRender = false;
    connect(&m_renderPumpTimer, &QTimer::timeout, this, [this]() {
        if (!m_window) return;
        if (m_renderPumpFirstRender) {
            // Render is alive — stop the pump, mpv handles frames.
            m_renderPumpTimer.stop();
            return;
        }
        // Bootstrap phase: force expose + update to get first render.
        QCoreApplication::postEvent(m_window,
            new QExposeEvent(m_window->geometry()));
        m_window->requestUpdate();
    });
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

void WallpaperEngine::notifyFirstRender() {
    m_renderPumpFirstRender = true;
    Logger::instance().log(Logger::Info, "Engine",
        "Render pump switched to steady mode (no QExposeEvent)");
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
    HWND defViewParent = nullptr;
    int  totalWorkerW = 0;
    int  withDefView  = 0;
};

// Recursive child search — FindWindowEx only checks direct children,
// but on Windows 11 24H2 SHELLDLL_DefView may be nested deeper inside
// a WorkerW. We walk the entire child tree.
struct DefViewSearch {
    HWND found = nullptr;
};
static BOOL CALLBACK findDefViewProc(HWND hwnd, LPARAM lp) {
    auto* s = reinterpret_cast<DefViewSearch*>(lp);
    wchar_t cls[64] = {0};
    GetClassNameW(hwnd, cls, 64);
    if (lstrcmpW(cls, L"SHELLDLL_DefView") == 0) {
        s->found = hwnd;
        return FALSE; // stop
    }
    // Recurse into children
    EnumChildWindows(hwnd, findDefViewProc, lp);
    return s->found ? FALSE : TRUE;
}

static bool workerWHasDefView(HWND workerW) {
    DefViewSearch s;
    EnumChildWindows(workerW, findDefViewProc, reinterpret_cast<LPARAM>(&s));
    return s.found != nullptr;
}

static BOOL CALLBACK enumProc(HWND top, LPARAM lp) {
    auto* ctx = reinterpret_cast<EnumCtx*>(lp);

    wchar_t cls[64] = {0};
    GetClassNameW(top, cls, 64);
    if (lstrcmpW(cls, L"WorkerW") != 0) return TRUE;

    ctx->totalWorkerW++;
    if (workerWHasDefView(top)) {
        ctx->withDefView++;
        ctx->defViewParent = top;
        return TRUE; // skip this one — it hosts icons
    }
    // A top-level WorkerW WITHOUT SHELLDLL_DefView is the wallpaper layer.
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
        QStringLiteral("WorkerW scan: total=%1 with-defview=%2 picked=0x%3 defview-parent=0x%4")
            .arg(ctx.totalWorkerW)
            .arg(ctx.withDefView)
            .arg(reinterpret_cast<quintptr>(ctx.result), 0, 16)
            .arg(reinterpret_cast<quintptr>(ctx.defViewParent), 0, 16));

    // Fallback: if no wallpaper-layer WorkerW found but we found the
    // icons WorkerW, use the icon WorkerW's NEXT sibling as parent.
    if (!ctx.result && ctx.defViewParent) {
        ctx.result = GetWindow(ctx.defViewParent, GW_HWNDNEXT);
        Logger::instance().log(Logger::Info, "Engine",
            QStringLiteral("Fallback: using next sibling of DefView parent: 0x%1")
                .arg(reinterpret_cast<quintptr>(ctx.result), 0, 16));
    }

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

    // Step 1: hide the window BEFORE any style changes.
    ShowWindow(hwnd, SW_HIDE);

    // Step 2: install subclass to block WM_DESTROY/WM_NCDESTROY.
    // Windows/Explorer sends these when we SetParent to WorkerW or on
    // desktop refresh. Without blocking them, the QQuickWindow is
    // destroyed and render context is lost.
    if (!SetWindowSubclass(hwnd, WallpaperSubclassProc, 1, 0)) {
        Logger::instance().log(Logger::Error, "Engine",
            QStringLiteral("SetWindowSubclass failed: %1").arg(GetLastError()));
    }

    // Step 3: change styles to WS_CHILD while hidden.
    SetWindowLongPtrW(hwnd, GWL_STYLE,
        (m_previousStyle & ~(WS_OVERLAPPEDWINDOW | WS_POPUP))
            | WS_CHILD | WS_CLIPCHILDREN);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
        (m_previousExStyle & ~(WS_EX_LAYERED | WS_EX_OVERLAPPEDWINDOW))
            | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);

    // Step 4: reparent to WorkerW while hidden.
    if (!SetParent(hwnd, parent)) {
        Logger::instance().log(Logger::Error, "Engine",
            QStringLiteral("SetParent failed: %1").arg(GetLastError()));
        emit engineError("SetParent не удался");
        return false;
    }

    // Step 5: apply style changes and show the window.
    // HWND_BOTTOM places our wallpaper window BELOW all other children
    // of WorkerW (including the icons/SysListView32). This ensures:
    //   - Desktop icons are visible on top of wallpaper
    //   - Our wallpaper never covers the main application window
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE
            | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);

    // After SetParent, the QQuickWindow loses its normal expose flow. Tell
    // Qt to keep the scene graph + GL context alive even if the platform
    // briefly thinks the window is unexposed.
    m_window->setPersistentSceneGraph(true);
    m_window->setPersistentGraphics(true);

    // Step 6: manually send QExposeEvent to trick Qt Quick into thinking
    // the window is exposed. Without this, scene graph won't render because
    // Qt Quick checks isExposed() before rendering.
    QExposeEvent exposeEvent(m_window->geometry());
    QCoreApplication::sendEvent(m_window, &exposeEvent);

    resyncGeometry();

    // After sizing, push our window to the BOTTOM of the Z-order within
    // the WorkerW parent. This ensures desktop icons and other windows
    // appear on top of our wallpaper.
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    m_window->requestUpdate();

    // Pump the render loop — child windows of WorkerW don't get expose
    // events from the platform plugin, so the scene graph can stall.
    m_renderPumpTimer.start();

    m_active = true;
    emit activeChanged();
    emit attached();
    Logger::instance().log(Logger::Info, "Engine", "Wallpaper attached to WorkerW");
    return true;
}

void WallpaperEngine::detach() {
    m_renderPumpTimer.stop();
    if (!m_window) {
        m_active = false;
        return;
    }
    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    if (hwnd) {
        // Remove subclass first, before any other operations.
        RemoveWindowSubclass(hwnd, WallpaperSubclassProc, 1);

        // Restore original styles first (before re-parenting back to
        // top-level), so the window transitions cleanly.
        SetWindowLongPtrW(hwnd, GWL_STYLE,   m_previousStyle);
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, m_previousExStyle);
        SetParent(hwnd, static_cast<HWND>(m_previousParent));
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE
                | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
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
