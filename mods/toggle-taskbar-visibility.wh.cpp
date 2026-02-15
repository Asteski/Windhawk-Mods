// ==WindhawkMod==
// @id           toggle-taskbar-visibility
// @name         Toggle Taskbar Visibility (Ctrl+Alt+D)
// @description  Toggle the Windows taskbar visibility using Ctrl+Alt+D. No prompts, no Explorer restart.
// @version      0.1.0
// @author       Asteski
// @include      explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

#include <windows.h>

HWND g_hHotkeyWnd = NULL;

void ToggleTaskbarVisibility() {
    HWND hTaskbar = FindWindowA("Shell_TrayWnd", NULL);
    if (hTaskbar) {
        bool isVisible = IsWindowVisible(hTaskbar);
        ShowWindow(hTaskbar, isVisible ? SW_HIDE : SW_SHOW);
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
