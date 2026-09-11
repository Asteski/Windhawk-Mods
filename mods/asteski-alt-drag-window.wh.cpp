// ==WindhawkMod==
// @id           asteski-alt-drag-window
// @name         Alt Drag Window
// @description  Hold Alt and click anywhere on any window to drag and move it, inspired by AltSnap / AltDrag
// @version      1.0.0
// @author       Asteski
// @github       https://github.com/Asteski
// @include      explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Alt Drag Window

Enables moving any window by holding **Alt** and clicking anywhere on it — inspired by utilities like
[AltSnap](https://github.com/RamonUnch/AltSnap) and [AltDrag](https://stefansundin.github.io/altdrag/).

No more hunting for the title bar — grab any window from anywhere and drag it where you want.

## How to use

1. Hold **Alt** (plus the optional extra modifier key if configured)
2. Press and hold the configured mouse button anywhere on a window
3. Drag to move the window
4. Release the button to finish

## Features

- Works on any part of the window, not just the title bar
- Maximized windows are automatically restored before dragging begins
- Configurable trigger mouse button: **Left** (default), Right, or Middle
- Optional extra modifier key alongside Alt: **None** (default), Ctrl, or Shift
- Releasing Alt mid-drag gracefully stops the movement without leaving the mouse in a bad state

## Settings

### Trigger Mouse Button

Choose which mouse button, combined with Alt, will initiate window dragging:

- **Left** (default) — Alt + Left-click drag
- **Right** — Alt + Right-click drag (suppresses the context menu)
- **Middle** — Alt + Middle-click drag

### Additional Modifier Key

Optionally require a second modifier key so the shortcut does not interfere with
applications that already use Alt + mouse button combinations:

- **None** (default) — Alt alone is sufficient
- **Ctrl** — Must also hold Ctrl (Alt + Ctrl + button)
- **Shift** — Must also hold Shift (Alt + Shift + button)
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- triggerButton: left
  $name: Trigger Mouse Button
  $description: Mouse button to hold with Alt to start dragging a window
  $options:
  - left: Left mouse button
  - right: Right mouse button
  - middle: Middle mouse button
- additionalModifier: none
  $name: Additional Modifier Key
  $description: Optional extra modifier key required alongside Alt to avoid conflicts with other Alt+click shortcuts
  $options:
  - none: None
  - ctrl: Ctrl
  - shift: Shift
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <atomic>

static HHOOK  g_mouseHook    = nullptr;
static HANDLE g_hookThread   = nullptr;
static DWORD  g_hookThreadId = 0;

// Settings — written by the main thread, read by the hook thread.
// std::atomic ensures safe cross-thread access without locks.
static std::atomic<int> g_triggerButton{0};      // 0=left, 1=right, 2=middle
static std::atomic<int> g_additionalModifier{0}; // 0=none, 1=ctrl, 2=shift

// Drag state — only ever touched inside LowLevelMouseProc, which runs
// serially on the single hook thread, so no synchronisation is needed here.
static struct {
    bool  active;         // currently moving the window on each WM_MOUSEMOVE
    bool  pendingBtnUp;   // a button-down was suppressed; suppress matching button-up too
    HWND  hwnd;
    POINT clickPt;        // screen coords where the drag was initiated
    POINT windowOrigin;   // window top-left at the time the drag was initiated
} g_drag{};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Returns true when a window is a reasonable candidate for alt-dragging.
static bool IsValidDragTarget(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd))       return false;
    if (hwnd == GetDesktopWindow())     return false;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style & WS_VISIBLE))          return false;
    if (style  & WS_DISABLED)           return false;
    if (style  & WS_MINIMIZE)           return false; // iconified
    if (style  & WS_CHILD)              return false; // not a top-level window

    // Skip shell / desktop surfaces that should never be moved.
    WCHAR cls[256];
    GetClassNameW(hwnd, cls, 256);
    if (wcscmp(cls, L"Shell_TrayWnd")          == 0) return false;
    if (wcscmp(cls, L"Shell_SecondaryTrayWnd") == 0) return false;
    if (wcscmp(cls, L"Progman")                == 0) return false;
    if (wcscmp(cls, L"WorkerW")                == 0) return false;

    return true;
}

// Test whether the required modifier keys are currently held.
static bool AreModifiersHeld()
{
    if (!(GetKeyState(VK_MENU) & 0x8000))
        return false;

    switch (g_additionalModifier.load(std::memory_order_relaxed)) {
        case 1: return (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        case 2: return (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
        default: return true;
    }
}

// ---------------------------------------------------------------------------
// Low-level mouse hook
// ---------------------------------------------------------------------------

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

    auto* ms = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    const POINT pt = ms->pt;

    // Resolve the configured trigger button to concrete message IDs.
    UINT btnDown, btnUp;
    switch (g_triggerButton.load(std::memory_order_relaxed)) {
        case 1:  btnDown = WM_RBUTTONDOWN; btnUp = WM_RBUTTONUP; break;
        case 2:  btnDown = WM_MBUTTONDOWN; btnUp = WM_MBUTTONUP; break;
        default: btnDown = WM_LBUTTONDOWN; btnUp = WM_LBUTTONUP; break;
    }

    // ── Button down: attempt to start a drag ─────────────────────────────────
    if ((UINT)wParam == btnDown && !g_drag.pendingBtnUp) {
        if (AreModifiersHeld()) {
            HWND hwnd = WindowFromPoint(pt);
            if (hwnd)
                hwnd = GetAncestor(hwnd, GA_ROOT);

            if (IsValidDragTarget(hwnd)) {
                // Restore a maximized window first so it can be freely positioned.
                if (IsZoomed(hwnd))
                    ShowWindow(hwnd, SW_RESTORE);

                RECT rc;
                GetWindowRect(hwnd, &rc);

                g_drag.hwnd         = hwnd;
                g_drag.clickPt      = pt;
                g_drag.windowOrigin = { rc.left, rc.top };
                g_drag.active       = true;
                g_drag.pendingBtnUp = true;

                // Bring the window to focus so dragging feels natural.
                SetForegroundWindow(hwnd);

                return 1; // suppress the original button-down
            }
        }
    }

    // ── Mouse move: reposition the window ────────────────────────────────────
    if (wParam == WM_MOUSEMOVE && g_drag.active) {
        if (AreModifiersHeld() && IsWindow(g_drag.hwnd)) {
            int newX = g_drag.windowOrigin.x + (pt.x - g_drag.clickPt.x);
            int newY = g_drag.windowOrigin.y + (pt.y - g_drag.clickPt.y);
            SetWindowPos(g_drag.hwnd, nullptr, newX, newY, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        } else {
            // Alt (or extra modifier) was released mid-drag.
            // Stop moving, but keep pendingBtnUp so the button-up is still
            // suppressed — the original button-down was already swallowed.
            g_drag.active = false;
        }
        // Always pass mouse moves through so hover states and cursors work normally.
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    // ── Button up: finish drag ────────────────────────────────────────────────
    if ((UINT)wParam == btnUp && g_drag.pendingBtnUp) {
        g_drag.active       = false;
        g_drag.pendingBtnUp = false;
        return 1; // suppress the matching button-up
    }

    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Hook thread — owns the LL hook so that its message pump keeps it alive
// ---------------------------------------------------------------------------

static DWORD WINAPI HookThreadProc(LPVOID)
{
    g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);
    if (!g_mouseHook) {
        Wh_Log(L"AltDragWindow: SetWindowsHookEx failed (error %lu)", GetLastError());
        return 1;
    }
    Wh_Log(L"AltDragWindow: hook installed");

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(g_mouseHook);
    g_mouseHook = nullptr;
    Wh_Log(L"AltDragWindow: hook removed");
    return 0;
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

static void LoadSettings()
{
    PCWSTR val;

    val = Wh_GetStringSetting(L"triggerButton");
    if (val) {
        if      (wcscmp(val, L"right")  == 0) g_triggerButton = 1;
        else if (wcscmp(val, L"middle") == 0) g_triggerButton = 2;
        else                                  g_triggerButton = 0;
        Wh_FreeStringSetting(val);
    }

    val = Wh_GetStringSetting(L"additionalModifier");
    if (val) {
        if      (wcscmp(val, L"ctrl")  == 0) g_additionalModifier = 1;
        else if (wcscmp(val, L"shift") == 0) g_additionalModifier = 2;
        else                                 g_additionalModifier = 0;
        Wh_FreeStringSetting(val);
    }
}

// ---------------------------------------------------------------------------
// Windhawk entry points
// ---------------------------------------------------------------------------

BOOL Wh_ModInit()
{
    Wh_Log(L"AltDragWindow: init");
    LoadSettings();

    g_hookThread = CreateThread(nullptr, 0, HookThreadProc, nullptr, 0, &g_hookThreadId);
    if (!g_hookThread) {
        Wh_Log(L"AltDragWindow: CreateThread failed (error %lu)", GetLastError());
        return FALSE;
    }
    return TRUE;
}

void Wh_ModUninit()
{
    Wh_Log(L"AltDragWindow: uninit");

    // Signal the hook thread to exit, then wait for it to clean up the hook.
    if (g_hookThreadId) {
        PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
        g_hookThreadId = 0;
    }
    if (g_hookThread) {
        WaitForSingleObject(g_hookThread, 3000);
        CloseHandle(g_hookThread);
        g_hookThread = nullptr;
    }
}

void Wh_ModSettingsChanged()
{
    Wh_Log(L"AltDragWindow: settings changed");
    // Settings take effect on the next drag; an in-progress drag is unaffected.
    LoadSettings();
}
