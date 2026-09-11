// ==WindhawkMod==
// @id              native-bluetooth-l2-tray
// @name            Native Bluetooth L2 Tray
// @description     Adds a tray icon which opens the native Windows 11 Bluetooth Control Center pane.
// @version         0.2
// @author          Local
// @include         explorer.exe
// @architecture    x86-64
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Native Bluetooth L2 Tray

Experimental Windows 11 25H2 / build 26200.9278 mod.

StartAllBack was observed opening the native Bluetooth device list with:

    ms-controlcenter:bluetooth

Windows then starts:

    ShellHost.exe url=ms-controlcenter:bluetooth

and Control Center routes that into the native Bluetooth DevicesFlow L2 pane.

This mod intentionally does not use UI Automation, mouse coordinates,
ControlCenter.dll hooks, or direct XAML object calls. It only adds a tray icon
and invokes the same URI StartAllBack uses.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <shellapi.h>

HWND g_trayHwnd = nullptr;
HICON g_trayIcon = nullptr;
UINT g_taskbarCreatedMessage = 0;
ULONGLONG g_lastTrayClickTick = 0;

constexpr UINT kTrayIconId = 1;
constexpr UINT kTrayCallbackMessage = WM_APP + 0x42;

void OpenBluetoothFlyout() {
    HINSTANCE result = ShellExecuteW(nullptr, nullptr,
                                     L"ms-controlcenter:bluetooth", nullptr,
                                     nullptr, SW_SHOWNORMAL);
    Wh_Log(L"ShellExecuteW(ms-controlcenter:bluetooth) returned %p", result);

    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        Wh_Log(L"ShellExecuteW(ms-controlcenter:bluetooth) failed: code=%Id",
               reinterpret_cast<INT_PTR>(result));
    }
}

void AddTrayIcon() {
    if (!g_trayHwnd) {
        return;
    }

    if (!g_trayIcon) {
        WCHAR bluetoothCpl[MAX_PATH];
        UINT len = GetSystemDirectoryW(bluetoothCpl, ARRAYSIZE(bluetoothCpl));
        if (len && len < ARRAYSIZE(bluetoothCpl)) {
            wcscat_s(bluetoothCpl, L"\\bthprops.cpl");
            ExtractIconExW(bluetoothCpl, 0, nullptr, &g_trayIcon, 1);
        }
    }

    if (!g_trayIcon) {
        g_trayIcon = LoadIconW(nullptr, IDI_APPLICATION);
    }

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_trayHwnd;
    nid.uID = kTrayIconId;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = kTrayCallbackMessage;
    nid.hIcon = g_trayIcon;
    wcscpy_s(nid.szTip, L"Bluetooth devices");

    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        Wh_Log(L"Shell_NotifyIcon(NIM_ADD) failed: %lu", GetLastError());
        return;
    }

    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &nid);
    Wh_Log(L"Bluetooth tray icon added.");
}

void RemoveTrayIcon() {
    if (!g_trayHwnd) {
        return;
    }

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_trayHwnd;
    nid.uID = kTrayIconId;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == kTrayCallbackMessage) {
        switch (LOWORD(lParam)) {
            case NIN_SELECT:
            case NIN_KEYSELECT:
            case WM_LBUTTONUP:
                if (GetTickCount64() - g_lastTrayClickTick < 250) {
                    return 0;
                }

                g_lastTrayClickTick = GetTickCount64();
                Wh_Log(L"Bluetooth tray icon clicked.");
                OpenBluetoothFlyout();
                return 0;
        }
    }

    if (msg == g_taskbarCreatedMessage) {
        AddTrayIcon();
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BOOL Wh_ModInit() {
    g_taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");

    const wchar_t className[] = L"WindhawkNativeBluetoothL2TrayWindow";

    WNDCLASSW wc{};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = className;
    RegisterClassW(&wc);

    g_trayHwnd = CreateWindowExW(0, className, className, 0, 0, 0, 0, 0,
                                 nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_trayHwnd) {
        Wh_Log(L"Failed to create tray window: %lu", GetLastError());
        return FALSE;
    }

    AddTrayIcon();
    Wh_Log(L"Native Bluetooth L2 tray mod loaded.");
    return TRUE;
}

void Wh_ModUninit() {
    RemoveTrayIcon();

    if (g_trayHwnd) {
        DestroyWindow(g_trayHwnd);
        g_trayHwnd = nullptr;
    }

    if (g_trayIcon) {
        DestroyIcon(g_trayIcon);
        g_trayIcon = nullptr;
    }
}
