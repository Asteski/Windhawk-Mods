// ==WindhawkMod==
// @id           toggle-taskbar-visibility
// @name         Toggle Taskbar Visibility (Ctrl+Alt+D)
// @description  Toggle the Windows taskbar visibility using Ctrl+Alt+D. No prompts, no Explorer restart.
// @version      0.2.0
// @author       Asteski
// @include      explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

#include <windows.h>

HWND g_hHotkeyWnd = NULL;

void ToggleTaskbarVisibility() {
    HWND hTaskbar = FindWindowA("Shell_TrayWnd", NULL);
    if (hTaskbar) {
        BOOL isVisible = IsWindowVisible(hTaskbar);
        if (isVisible) {
            // Hide taskbar
            ShowWindow(hTaskbar, SW_HIDE);
            SetWindowPos(hTaskbar, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            // Set work area to full screen
            RECT screenRect;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
            HMONITOR hMonitor = MonitorFromWindow(hTaskbar, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO mi = { 0 };
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfo(hMonitor, &mi)) {
                SystemParametersInfo(SPI_SETWORKAREA, 0, &mi.rcMonitor, SPIF_SENDCHANGE);
            }
        } else {
            // Show taskbar
            ShowWindow(hTaskbar, SW_SHOW);
            SetWindowPos(hTaskbar, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            // Restore work area to exclude taskbar
            APPBARDATA abd = { 0 };
            abd.cbSize = sizeof(abd);
            abd.hWnd = hTaskbar;
            if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
                RECT workArea = { 0 };
                SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
                // Only shrink if the taskbar is on the same monitor as the work area
                HMONITOR hMonitor = MonitorFromWindow(hTaskbar, MONITOR_DEFAULTTOPRIMARY);
                MONITORINFO mi = { 0 };
                mi.cbSize = sizeof(mi);
                if (GetMonitorInfo(hMonitor, &mi)) {
                    RECT newWorkArea = mi.rcMonitor;
                    // Adjust work area based on taskbar position
                    if (abd.rc.top == mi.rcMonitor.top && abd.rc.bottom > abd.rc.top) {
                        // Taskbar on top
                        newWorkArea.top += (abd.rc.bottom - abd.rc.top);
                    } else if (abd.rc.bottom == mi.rcMonitor.bottom && abd.rc.bottom > abd.rc.top) {
                        // Taskbar on bottom
                        newWorkArea.bottom -= (abd.rc.bottom - abd.rc.top);
                    } else if (abd.rc.left == mi.rcMonitor.left && abd.rc.right > abd.rc.left) {
                        // Taskbar on left
                        newWorkArea.left += (abd.rc.right - abd.rc.left);
                    } else if (abd.rc.right == mi.rcMonitor.right && abd.rc.right > abd.rc.left) {
                        // Taskbar on right
                        newWorkArea.right -= (abd.rc.right - abd.rc.left);
                    }
                    SystemParametersInfo(SPI_SETWORKAREA, 0, &newWorkArea, SPIF_SENDCHANGE);
                }
            }
        }
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
    CreateHotkeyWindow();
    if (!RegisterHotKey(g_hHotkeyWnd, 1, MOD_CONTROL | MOD_ALT, 'D')) {
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
