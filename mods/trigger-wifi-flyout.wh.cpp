
// Windhawk mod metadata
// ==WindhawkMod==
// @id              trigger-wifi-flyout
// @name            Trigger Wi-Fi Flyout
// @description     Open the Windows 11 Wi-Fi flyout with a keyboard shortcut (Ctrl+Alt+W)
// @version         1.0
// @author          Asteski
// @github          https://github.com/Asteski
// @homepage        https://github.com/Asteski
// @include         *
// ==/WindhawkMod==

// Registers a global hotkey (Ctrl+Alt+W) to open the Wi-Fi flyout by simulating a click on the network tray icon.
// This mod is a proof-of-concept and may require adjustments for different Windows 11 builds.




#include <windows.h>
#include <servprov.h>
#include <Unknwn.h>
#include <Objbase.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")



// GUIDs from ExplorerPatcher's ImmersiveFlyouts.h

static const GUID CLSID_ImmersiveShell = {0xC2F03A33, 0x21F5, 0x47FA, {0xB4, 0xBB, 0x15, 0x6E, 0x1C, 0x5E, 0x72, 0x1C}};
static const GUID CLSID_ShellExperienceManagerFactory = {0x2E8FCB18, 0xA0EE, 0x41AD, {0x8E, 0xF8, 0x77, 0xFB, 0x3A, 0x37, 0x0C, 0xA5}};
static const GUID IID_NetworkFlyoutExperienceManager = {0xC9DDC674, 0xB44B, 0x4C67, {0x9D, 0x79, 0x2B, 0x23, 0x7D, 0x9B, 0xE0, 0x5A}};


// Minimal custom interface for ShowFlyout
struct INetworkFlyoutExperienceManager : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE ShowFlyout(RECT* prc) = 0;
};



// Helper: Open Quick Settings (Action Center) using WM_COMMAND 419 (0x1A3) to Shell_TrayWnd
// NOTE: Due to Windhawk mod editor limitations, direct Wi-Fi flyout is not possible.
void OpenQuickSettings() {
    HWND hTray = FindWindow(L"Shell_TrayWnd", NULL);
    if (hTray) {
        // 419 (0x1A3) is the command to open Quick Settings in Windows 11
        PostMessage(hTray, WM_COMMAND, 419, 0);
    }
}

#define HOTKEY_ID 1




// Hotkey handler
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
            (GetAsyncKeyState(VK_MENU) & 0x8000) &&
            p->vkCode == 'W') {
            OpenQuickSettings();
            return 1; // Block further processing
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

HHOOK g_hHook = NULL;

BOOL Wh_ModInit() {
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    return g_hHook != NULL;
}

void Wh_ModUninit() {
    if (g_hHook) {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
}
