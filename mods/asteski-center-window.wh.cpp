// ==WindhawkMod==
// @id              center-window
// @name            Center Active Window
// @description     Centers the currently focused window on the screen (configurable size).
// @version         1.0.1
// @author          Asteski
// @include         explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- centerWidth: ""
  $name: Center Width
  $description: Custom width (px) to apply when centering the window. Leave empty to keep original width.
- centerHeight: ""
  $name: Center Height
  $description: Custom height (px) to apply when centering the window. Leave empty to keep original height.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <string>
#include <algorithm>

HWND g_hHotkeyWnd = NULL;
int g_centerWidth = 0;
int g_centerHeight = 0;
UINT g_hotkeyModifiersFlags = MOD_WIN;
UINT g_hotkeyVKey = 'J';

void CenterActiveWindow() {
    HWND hWnd = GetForegroundWindow();
    if (!hWnd || !IsWindow(hWnd)) return;

    RECT r;
    if (!GetWindowRect(hWnd, &r)) return;
    int width = r.right - r.left;
    int height = r.bottom - r.top;

    // Apply custom size if configured (empty/0 = keep original)
    if (g_centerWidth > 0) {
        width = g_centerWidth;
    }
    if (g_centerHeight > 0) {
        height = g_centerHeight;
    }

    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hMonitor, &mi)) return;
    RECT screen = mi.rcWork;

    int newX = screen.left + (screen.right - screen.left - width) / 2;
    int newY = screen.top + (screen.bottom - screen.top - height) / 2;

    // If custom size was provided, allow resizing; otherwise keep current size
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
    if (g_centerWidth <= 0 && g_centerHeight <= 0) {
        flags |= SWP_NOSIZE;
    }

    SetWindowPos(hWnd, NULL, newX, newY, width, height, flags);
}

LRESULT CALLBACK HotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_HOTKEY && wParam == 1) {
        CenterActiveWindow();
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateHotkeyWindow() {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = HotkeyWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WhCenterHotkeyWnd";
    RegisterClass(&wc);
    g_hHotkeyWnd = CreateWindow(L"WhCenterHotkeyWnd", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);
}

// Parse modifiers string (e.g., "win+shift") into RegisterHotKey flags
static UINT ParseModifiers(PCWSTR modifiers) {
    if (!modifiers) return MOD_WIN;
    std::wstring s(modifiers);
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    UINT flags = 0;
    if (s.find(L"win") != std::wstring::npos) flags |= MOD_WIN;
    if (s.find(L"shift") != std::wstring::npos) flags |= MOD_SHIFT;
    if (s.find(L"alt") != std::wstring::npos) flags |= MOD_ALT;
    if (s.find(L"ctrl") != std::wstring::npos || s.find(L"control") != std::wstring::npos) flags |= MOD_CONTROL;
    if (flags == 0) flags = MOD_WIN; // default
    return flags;
}

// Map common key names to virtual-key codes
static UINT ParseKey(PCWSTR keyStr) {
    if (!keyStr || wcslen(keyStr) == 0) return 'J';
    std::wstring s(keyStr);
    // trim
    while (!s.empty() && iswspace(s.front())) s.erase(s.begin());
    while (!s.empty() && iswspace(s.back())) s.pop_back();
    if (s.empty()) return 'J';
    std::wstring lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    if (lower.size() == 1) {
        SHORT vk = VkKeyScanW(lower[0]);
        if (vk != -1) return LOBYTE(vk);
        return (UINT)towupper(lower[0]);
    }

    if (lower.size() > 1 && lower[0] == L'F' && iswdigit(lower[1])) {
        int fn = _wtoi(lower.c_str() + 1);
        if (fn >= 1 && fn <= 24) return VK_F1 + (fn - 1);
    }

    if (lower == L"enter" || lower == L"return") return VK_RETURN;
    if (lower == L"space" || lower == L"spacebar") return VK_SPACE;
    if (lower == L"tab") return VK_TAB;
    if (lower == L"esc" || lower == L"escape") return VK_ESCAPE;
    if (lower == L"left") return VK_LEFT;
    if (lower == L"right") return VK_RIGHT;
    if (lower == L"up") return VK_UP;
    if (lower == L"down") return VK_DOWN;
    if (lower == L"home") return VK_HOME;
    if (lower == L"end") return VK_END;
    if (lower == L"pageup") return VK_PRIOR;
    if (lower == L"pagedown") return VK_NEXT;

    // Fallback: try VkKeyScan on first character
    SHORT vk = VkKeyScanW(s[0]);
    if (vk != -1) return LOBYTE(vk);
    return 'J';
}

// Load settings from Windhawk and apply to globals
void LoadSettings() {
    PCWSTR wCenterWidth = Wh_GetStringSetting(L"centerWidth");
    PCWSTR wCenterHeight = Wh_GetStringSetting(L"centerHeight");
    PCWSTR wHotkeyModifiers = Wh_GetStringSetting(L"hotkeyModifiers");
    PCWSTR wHotkeyKey = Wh_GetStringSetting(L"hotkeyKey");

    if (wCenterWidth && wcslen(wCenterWidth) > 0) {
        g_centerWidth = _wtoi(wCenterWidth);
    } else {
        g_centerWidth = 0;
    }
    if (wCenterHeight && wcslen(wCenterHeight) > 0) {
        g_centerHeight = _wtoi(wCenterHeight);
    } else {
        g_centerHeight = 0;
    }

    g_hotkeyModifiersFlags = ParseModifiers(wHotkeyModifiers);
    g_hotkeyVKey = ParseKey(wHotkeyKey);

    if (wCenterWidth) Wh_FreeStringSetting(wCenterWidth);
    if (wCenterHeight) Wh_FreeStringSetting(wCenterHeight);
    if (wHotkeyModifiers) Wh_FreeStringSetting(wHotkeyModifiers);
    if (wHotkeyKey) Wh_FreeStringSetting(wHotkeyKey);

    Wh_Log(L"Center Window: settings loaded width=%d height=%d modifiers=0x%X vkey=0x%X",
        g_centerWidth, g_centerHeight, g_hotkeyModifiersFlags, g_hotkeyVKey);
}

// Called when settings change
void Wh_ModSettingsChanged() {
    // Unregister previous hotkey
    if (g_hHotkeyWnd) {
        UnregisterHotKey(g_hHotkeyWnd, 1);
    }
    LoadSettings();
    // Register new hotkey
    if (g_hHotkeyWnd) {
        RegisterHotKey(g_hHotkeyWnd, 1, g_hotkeyModifiersFlags, (UINT)g_hotkeyVKey);
    }
}

BOOL Wh_ModInit() {
    CreateHotkeyWindow();
    // Load settings and register configured hotkey
    LoadSettings();
    if (!RegisterHotKey(g_hHotkeyWnd, 1, g_hotkeyModifiersFlags, (UINT)g_hotkeyVKey)) {
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
