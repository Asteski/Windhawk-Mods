// ==WindhawkMod==
// @id           toggle-taskbar-visibility
// @name         Toggle Taskbar Visibility
// @description  Toggle the Windows taskbar visibility using Ctrl+Win+T.
// @version      0.2.0
// @author       Asteski
// @include      explorer.exe
// @compilerOptions -luser32 -lshell32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- osTarget: win11
  $name: Target OS
  $description: Which Windows version taskbar behavior to use. Set to Windows 10 to fix reserved space issues on Win10, or Windows 11 for Win11-specific fixes.
  $options:
  - win11: Windows 11
  - win10: Windows 10
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>

// Windhawk provides these at build time; declarations are for editor diagnostics.
PCWSTR Wh_GetStringSetting(PCWSTR valueName, ...);
void Wh_FreeStringSetting(PCWSTR string);

// Settings
bool g_osTarget_win11 = true;  // true for Win11, false for Win10

// Function to get Windows version
BOOL IsWindows11OrLater() {
    OSVERSIONINFOW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExW(&osvi);
    
    // Windows 11 is version 10.0.22000 or higher
    return (osvi.dwMajorVersion > 10) || 
           (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0 && osvi.dwBuildNumber >= 22000);
}

HWND g_hHotkeyWnd = NULL;

void RefreshWindowsUI_Win11() {
    DWORD_PTR result = 0;
    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        SPI_SETWORKAREA,
        reinterpret_cast<LPARAM>(L"TraySettings"),
        SMTO_ABORTIFHUNG,
        200,
        &result);
}

void SetSecondaryTaskbarsVisible(BOOL visible) {
    HWND hSecondary = NULL;
    while ((hSecondary = FindWindowExA(NULL, hSecondary, "Shell_SecondaryTrayWnd", NULL)) != NULL) {
        ShowWindow(hSecondary, visible ? SW_SHOW : SW_HIDE);
        SetWindowPos(hSecondary,
                     NULL,
                     0,
                     0,
                     0,
                     0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

BOOL IsTaskbarAutoHideEnabled(HWND hTaskbar) {
    APPBARDATA abd = { 0 };
    abd.cbSize = sizeof(abd);
    abd.hWnd = hTaskbar;
    DWORD state = static_cast<DWORD>(SHAppBarMessage(ABM_GETSTATE, &abd));
    return (state & ABS_AUTOHIDE) != 0;
}

void SetTaskbarAutoHide(HWND hTaskbar, BOOL enable) {
    APPBARDATA abd = { 0 };
    abd.cbSize = sizeof(abd);
    abd.hWnd = hTaskbar;

    DWORD state = static_cast<DWORD>(SHAppBarMessage(ABM_GETSTATE, &abd));
    DWORD newState = enable ? (state | ABS_AUTOHIDE) : (state & ~ABS_AUTOHIDE);

    abd.lParam = newState;
    SHAppBarMessage(ABM_SETSTATE, &abd);
    SHAppBarMessage(ABM_WINDOWPOSCHANGED, &abd);
}

void ToggleTaskbarVisibility_Win10() {
    HWND hTaskbar = FindWindowA("Shell_TrayWnd", NULL);
    if (!hTaskbar) return;
    
    BOOL isVisible = IsWindowVisible(hTaskbar);
    HMONITOR hMonitor = MonitorFromWindow(hTaskbar, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hMonitor, &mi)) return;
    
    if (isVisible) {
        // Hide taskbar - Win10 approach
        ShowWindow(hTaskbar, SW_HIDE);
        SetWindowPos(hTaskbar, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        SystemParametersInfo(SPI_SETWORKAREA, 0, &mi.rcMonitor, SPIF_SENDCHANGE);
    } else {
        // Show taskbar - Win10 approach
        ShowWindow(hTaskbar, SW_SHOW);
        SetWindowPos(hTaskbar, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        
        APPBARDATA abd = { 0 };
        abd.cbSize = sizeof(abd);
        abd.hWnd = hTaskbar;
        if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
            RECT newWorkArea = mi.rcMonitor;
            // Adjust work area based on taskbar position
            if (abd.rc.top == mi.rcMonitor.top && abd.rc.bottom > abd.rc.top) {
                newWorkArea.top += (abd.rc.bottom - abd.rc.top);
            } else if (abd.rc.bottom == mi.rcMonitor.bottom && abd.rc.bottom > abd.rc.top) {
                newWorkArea.bottom -= (abd.rc.bottom - abd.rc.top);
            } else if (abd.rc.left == mi.rcMonitor.left && abd.rc.right > abd.rc.left) {
                newWorkArea.left += (abd.rc.right - abd.rc.left);
            } else if (abd.rc.right == mi.rcMonitor.right && abd.rc.right > abd.rc.left) {
                newWorkArea.right -= (abd.rc.right - abd.rc.left);
            }
            SystemParametersInfo(SPI_SETWORKAREA, 0, &newWorkArea, SPIF_SENDCHANGE);
        }
    }
}

void ToggleTaskbarVisibility_Win11() {
    HWND hTaskbar = FindWindowA("Shell_TrayWnd", NULL);
    if (!hTaskbar) return;

    BOOL isHidden = IsTaskbarAutoHideEnabled(hTaskbar);
    HMONITOR hMonitor = MonitorFromWindow(hTaskbar, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hMonitor, &mi)) return;

    if (!isHidden) {
        // Win11 hide: use auto-hide mode, which removes reserved maximize space.
        SetTaskbarAutoHide(hTaskbar, TRUE);
        SetWindowPos(hTaskbar,
                     NULL,
                     0,
                     0,
                     0,
                     0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        SetSecondaryTaskbarsVisible(TRUE);

        // Keep work area in sync with hidden state.
        SystemParametersInfo(SPI_SETWORKAREA, 0, &mi.rcMonitor, SPIF_SENDCHANGE);
        RefreshWindowsUI_Win11();
    } else {
        // Win11 show: disable auto-hide and restore normal reserved area.
        ShowWindow(hTaskbar, SW_SHOW);
        SetWindowPos(hTaskbar,
                     NULL,
                     0,
                     0,
                     0,
                     0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        SetSecondaryTaskbarsVisible(TRUE);
        SetTaskbarAutoHide(hTaskbar, FALSE);

        APPBARDATA abd = { 0 };
        abd.cbSize = sizeof(abd);
        abd.hWnd = hTaskbar;

        if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
            RECT newWorkArea = mi.rcMonitor;
            // Adjust work area based on taskbar position
            if (abd.rc.top == mi.rcMonitor.top && abd.rc.bottom > abd.rc.top) {
                newWorkArea.top += (abd.rc.bottom - abd.rc.top);
            } else if (abd.rc.bottom == mi.rcMonitor.bottom && abd.rc.bottom > abd.rc.top) {
                newWorkArea.bottom -= (abd.rc.bottom - abd.rc.top);
            } else if (abd.rc.left == mi.rcMonitor.left && abd.rc.right > abd.rc.left) {
                newWorkArea.left += (abd.rc.right - abd.rc.left);
            } else if (abd.rc.right == mi.rcMonitor.right && abd.rc.right > abd.rc.left) {
                newWorkArea.right -= (abd.rc.right - abd.rc.left);
            }
            SystemParametersInfo(SPI_SETWORKAREA, 0, &newWorkArea, SPIF_SENDCHANGE);
        }

        RefreshWindowsUI_Win11();
    }
}

void ToggleTaskbarVisibility() {
    // Use OS-specific implementation based on settings
    if (g_osTarget_win11) {
        ToggleTaskbarVisibility_Win11();
    } else {
        ToggleTaskbarVisibility_Win10();
    }
}

LRESULT CALLBACK HotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_HOTKEY && wParam == 1) {
        ToggleTaskbarVisibility();
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateHotkeyWindow() {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = HotkeyWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WhHotkeyWnd";
    RegisterClass(&wc);
    g_hHotkeyWnd = CreateWindow(L"WhHotkeyWnd", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);
}

BOOL Wh_ModInit() {
    // Read settings
    PCWSTR osTarget = Wh_GetStringSetting(L"osTarget");
    if (osTarget) {
        g_osTarget_win11 = (wcscmp(osTarget, L"win11") == 0);
        Wh_FreeStringSetting(osTarget);
    } else {
        // Default: auto-detect OS
        g_osTarget_win11 = IsWindows11OrLater();
    }
    
    CreateHotkeyWindow();
    if (!RegisterHotKey(g_hHotkeyWnd, 1, MOD_CONTROL | MOD_WIN, 'T')) {
        // Silent failure, no prompt
    }
    return TRUE;
}

void Wh_ModUninit() {
    if (g_hHotkeyWnd) {
        UnregisterHotKey(g_hHotkeyWnd, 1);
        DestroyWindow(g_hHotkeyWnd);
        g_hHotkeyWnd = NULL;
    }
}
