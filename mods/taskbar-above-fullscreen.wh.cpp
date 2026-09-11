// ==WindhawkMod==
// @id           taskbar-above-fullscreen
// @name         Taskbar Above Fullscreen
// @description  Keeps the taskbar above fullscreen apps. Includes optional macOS-style edge-reveal behavior for fullscreen.
// @version      1.2.0
// @author       Asteski
// @include      explorer.exe
// @compilerOptions -luser32 -lshell32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- checkInterval: 100
  $name: Check Interval (ms)
    $description: How often to re-apply fullscreen/taskbar state.
  $options:
  - 50: Very Fast (50ms)
  - 100: Fast (100ms, default)
  - 250: Balanced (250ms)
  - 500: Light (500ms)
- osTarget: win10
  $name: Target OS Behavior
    $description: Win10 mode applies taskbar AppBar state logic.
  $options:
  - win10: Windows 10 behavior
  - win11: Windows 11 behavior
- macStyleFullscreenAutohide: false
    $name: macOS-style Fullscreen Edge Reveal
    $description: When enabled, fullscreen switches taskbar to auto-hide. Move pointer to taskbar edge to reveal it.
    $options:
    - false: Disabled (Always visible over fullscreen)
    - true: Enabled (Auto-hide only during fullscreen)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>
#include <atomic>

// Windhawk provides these at build time; declarations are for editor diagnostics.
PCWSTR Wh_GetStringSetting(PCWSTR valueName, ...);
void Wh_FreeStringSetting(PCWSTR string);

int g_checkInterval = 100;
bool g_osTarget_win11 = false;
bool g_macStyleFullscreenAutohide = false;

static std::atomic<bool> g_running = false;
static HANDLE g_monitorThread = nullptr;
static DWORD g_originalAppBarState = 0;
static bool g_originalAppBarStateValid = false;

BOOL IsWindows11OrLater() {
    OSVERSIONINFOW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExW(&osvi);

    return (osvi.dwMajorVersion > 10) ||
           (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0 &&
            osvi.dwBuildNumber >= 22000);
}

int ClampCheckInterval(int value) {
    if (value < 50) {
        return 50;
    }

    if (value > 2000) {
        return 2000;
    }

    return value;
}

int ReadIntSetting(PCWSTR valueName, int defaultValue) {
    int value = defaultValue;
    PCWSTR setting = Wh_GetStringSetting(valueName);

    if (setting) {
        wchar_t* endPtr = nullptr;
        long parsed = wcstol(setting, &endPtr, 10);
        if (endPtr != setting) {
            value = static_cast<int>(parsed);
        }
        Wh_FreeStringSetting(setting);
    }

    return value;
}

bool ReadBoolSetting(PCWSTR valueName, bool defaultValue) {
    bool value = defaultValue;
    PCWSTR setting = Wh_GetStringSetting(valueName);

    if (setting) {
        if (_wcsicmp(setting, L"true") == 0 ||
            _wcsicmp(setting, L"1") == 0 ||
            _wcsicmp(setting, L"yes") == 0 ||
            _wcsicmp(setting, L"on") == 0) {
            value = true;
        } else if (_wcsicmp(setting, L"false") == 0 ||
                   _wcsicmp(setting, L"0") == 0 ||
                   _wcsicmp(setting, L"no") == 0 ||
                   _wcsicmp(setting, L"off") == 0) {
            value = false;
        }

        Wh_FreeStringSetting(setting);
    }

    return value;
}

DWORD GetTaskbarAppBarState(HWND hTaskbar) {
    APPBARDATA abd = { 0 };
    abd.cbSize = sizeof(abd);
    abd.hWnd = hTaskbar;
    return static_cast<DWORD>(SHAppBarMessage(ABM_GETSTATE, &abd));
}

void SetTaskbarAppBarState(HWND hTaskbar, DWORD state) {
    APPBARDATA abd = { 0 };
    abd.cbSize = sizeof(abd);
    abd.hWnd = hTaskbar;
    abd.lParam = state;

    SHAppBarMessage(ABM_SETSTATE, &abd);
    SHAppBarMessage(ABM_WINDOWPOSCHANGED, &abd);
}

void EnforcePrimaryTaskbarAppBarState(HWND hTaskbar, bool fullscreenActive) {
    if (!g_originalAppBarStateValid) {
        g_originalAppBarState = GetTaskbarAppBarState(hTaskbar);
        g_originalAppBarStateValid = true;
    }

    DWORD desiredState = g_originalAppBarState;

    if (g_macStyleFullscreenAutohide) {
        if (fullscreenActive) {
            // macOS-style mode: only fullscreen should switch to auto-hide.
            desiredState = g_originalAppBarState | ABS_AUTOHIDE;
        }
    } else {
        // Always-visible-over-fullscreen mode: keep auto-hide off while mod is active.
        desiredState = g_originalAppBarState & ~ABS_AUTOHIDE;
    }

    DWORD currentState = GetTaskbarAppBarState(hTaskbar);
    if (currentState != desiredState) {
        SetTaskbarAppBarState(hTaskbar, desiredState);
    }
}

void ForceTaskbarWindowVisibleTopmost(HWND hWnd) {
    if (!hWnd || !IsWindow(hWnd)) {
        return;
    }

    if (!IsWindowVisible(hWnd)) {
        ShowWindow(hWnd, SW_SHOWNA);
    }

    SetWindowPos(hWnd,
                 HWND_TOPMOST,
                 0,
                 0,
                 0,
                 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE |
                     SWP_NOOWNERZORDER);
}

void ForceTaskbarWindowNotTopmost(HWND hWnd);
void EnforceSecondaryTaskbarsVisibleTopmost();
void RestoreSecondaryTaskbarsNotTopmost();

bool IsWindowFullscreen(HWND hWnd) {
    if (!hWnd || !IsWindow(hWnd) || !IsWindowVisible(hWnd) || IsIconic(hWnd)) {
        return false;
    }

    LONG_PTR style = GetWindowLongPtrW(hWnd, GWL_STYLE);
    LONG_PTR exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if (style & WS_CHILD) {
        return false;
    }

    if (exStyle & WS_EX_TOOLWINDOW) {
        return false;
    }

    wchar_t className[64] = { 0 };
    if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) > 0) {
        if (wcscmp(className, L"Shell_TrayWnd") == 0 ||
            wcscmp(className, L"Shell_SecondaryTrayWnd") == 0 ||
            wcscmp(className, L"Progman") == 0 ||
            wcscmp(className, L"WorkerW") == 0) {
            return false;
        }
    }

    RECT rc = { 0 };
    if (!GetWindowRect(hWnd, &rc)) {
        return false;
    }

    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
    if (!hMonitor) {
        return false;
    }

    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hMonitor, &mi)) {
        return false;
    }

    const int tolerance = 2;
    bool coversMonitor =
        (rc.left <= mi.rcMonitor.left + tolerance) &&
        (rc.top <= mi.rcMonitor.top + tolerance) &&
        (rc.right >= mi.rcMonitor.right - tolerance) &&
        (rc.bottom >= mi.rcMonitor.bottom - tolerance);

    if (!coversMonitor) {
        return false;
    }

    // Exclude standard maximized windows with normal caption/frame.
    if (IsZoomed(hWnd) && (style & WS_CAPTION) != 0 && (style & WS_THICKFRAME) != 0) {
        return false;
    }

    return true;
}

bool HasFullscreenWindow() {
    // Foreground-only detection avoids false positives from background windows.
    HWND hForeground = GetForegroundWindow();
    if (!hForeground) {
        return false;
    }

    HWND hRoot = GetAncestor(hForeground, GA_ROOT);
    if (hRoot) {
        hForeground = hRoot;
    }

    return IsWindowFullscreen(hForeground);
}

void ApplyTaskbarMode(HWND hTaskbar, bool fullscreenActive) {
    if (!hTaskbar || !IsWindow(hTaskbar)) {
        return;
    }

    if (!g_osTarget_win11) {
        EnforcePrimaryTaskbarAppBarState(hTaskbar, fullscreenActive);
    }

    if (g_macStyleFullscreenAutohide) {
        // In fullscreen, do not touch z-order; let AppBar edge-reveal work naturally.
        if (!fullscreenActive) {
            ForceTaskbarWindowNotTopmost(hTaskbar);
            RestoreSecondaryTaskbarsNotTopmost();
        }
        return;
    }

    if (fullscreenActive) {
        ForceTaskbarWindowVisibleTopmost(hTaskbar);
        EnforceSecondaryTaskbarsVisibleTopmost();
    } else {
        // Don't keep topmost on desktop; this avoids Start button rendering issues.
        ForceTaskbarWindowNotTopmost(hTaskbar);
        RestoreSecondaryTaskbarsNotTopmost();
    }
}

void ForceTaskbarWindowNotTopmost(HWND hWnd) {
    if (!hWnd || !IsWindow(hWnd)) {
        return;
    }

    SetWindowPos(hWnd,
                 HWND_NOTOPMOST,
                 0,
                 0,
                 0,
                 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void EnforceSecondaryTaskbarsVisibleTopmost() {
    HWND hSecondary = nullptr;
    while ((hSecondary = FindWindowExA(nullptr,
                                       hSecondary,
                                       "Shell_SecondaryTrayWnd",
                                       nullptr)) != nullptr) {
        ForceTaskbarWindowVisibleTopmost(hSecondary);
    }
}

void RestoreSecondaryTaskbarsNotTopmost() {
    HWND hSecondary = nullptr;
    while ((hSecondary = FindWindowExA(nullptr,
                                       hSecondary,
                                       "Shell_SecondaryTrayWnd",
                                       nullptr)) != nullptr) {
        ForceTaskbarWindowNotTopmost(hSecondary);
    }
}

DWORD WINAPI MonitorThreadProc(LPVOID) {
    while (g_running) {
        HWND hTaskbar = FindWindowA("Shell_TrayWnd", nullptr);
        bool fullscreenActive = HasFullscreenWindow();

        if (hTaskbar && IsWindow(hTaskbar)) {
            ApplyTaskbarMode(hTaskbar, fullscreenActive);
        }

        int sleepTime = g_checkInterval;
        for (int elapsed = 0; elapsed < sleepTime && g_running;
             elapsed += 25) {
            Sleep(25);
        }
    }

    return 0;
}

BOOL Wh_ModInit() {
    PCWSTR osTarget = Wh_GetStringSetting(L"osTarget");
    if (osTarget) {
        g_osTarget_win11 = (wcscmp(osTarget, L"win11") == 0);
        Wh_FreeStringSetting(osTarget);
    } else {
        g_osTarget_win11 = IsWindows11OrLater();
    }

    g_checkInterval = ClampCheckInterval(ReadIntSetting(L"checkInterval", 100));
    g_macStyleFullscreenAutohide = ReadBoolSetting(L"macStyleFullscreenAutohide", false);

    HWND hTaskbar = FindWindowA("Shell_TrayWnd", nullptr);
    if (hTaskbar && IsWindow(hTaskbar)) {
        if (!g_osTarget_win11) {
            g_originalAppBarState = GetTaskbarAppBarState(hTaskbar);
            g_originalAppBarStateValid = true;
        }

        ApplyTaskbarMode(hTaskbar, HasFullscreenWindow());
    }

    g_running = true;
    g_monitorThread = CreateThread(nullptr, 0, MonitorThreadProc, nullptr, 0, nullptr);
    if (!g_monitorThread) {
        g_running = false;

        if (hTaskbar && IsWindow(hTaskbar)) {
            if (!g_osTarget_win11 && g_originalAppBarStateValid) {
                SetTaskbarAppBarState(hTaskbar, g_originalAppBarState);
            }
            ForceTaskbarWindowNotTopmost(hTaskbar);
        }

        RestoreSecondaryTaskbarsNotTopmost();
        return FALSE;
    }

    return TRUE;
}

void Wh_ModUninit() {
    if (g_monitorThread) {
        g_running = false;
        WaitForSingleObject(g_monitorThread, 2000);
        CloseHandle(g_monitorThread);
        g_monitorThread = nullptr;
    }

    HWND hTaskbar = FindWindowA("Shell_TrayWnd", nullptr);
    if (hTaskbar && IsWindow(hTaskbar)) {
        if (!g_osTarget_win11 && g_originalAppBarStateValid) {
            SetTaskbarAppBarState(hTaskbar, g_originalAppBarState);
        }

        ForceTaskbarWindowNotTopmost(hTaskbar);
    }

    RestoreSecondaryTaskbarsNotTopmost();
    g_originalAppBarStateValid = false;
}

void Wh_ModSettingsChanged() {
    bool previousWin11Target = g_osTarget_win11;
    bool previousMacStyleFullscreenAutohide = g_macStyleFullscreenAutohide;

    PCWSTR osTarget = Wh_GetStringSetting(L"osTarget");
    if (osTarget) {
        g_osTarget_win11 = (wcscmp(osTarget, L"win11") == 0);
        Wh_FreeStringSetting(osTarget);
    }

    g_checkInterval = ClampCheckInterval(ReadIntSetting(L"checkInterval", 100));
    g_macStyleFullscreenAutohide = ReadBoolSetting(L"macStyleFullscreenAutohide", false);

    HWND hTaskbar = FindWindowA("Shell_TrayWnd", nullptr);
    if (!hTaskbar || !IsWindow(hTaskbar)) {
        return;
    }

    if (previousWin11Target && !g_osTarget_win11) {
        g_originalAppBarState = GetTaskbarAppBarState(hTaskbar);
        g_originalAppBarStateValid = true;
    }

    if (!previousWin11Target && g_osTarget_win11 && g_originalAppBarStateValid) {
        SetTaskbarAppBarState(hTaskbar, g_originalAppBarState);
    }

    if (previousMacStyleFullscreenAutohide != g_macStyleFullscreenAutohide) {
        // Clear any previously forced topmost so the new mode starts clean.
        ForceTaskbarWindowNotTopmost(hTaskbar);
        RestoreSecondaryTaskbarsNotTopmost();
    }

    ApplyTaskbarMode(hTaskbar, HasFullscreenWindow());
}
