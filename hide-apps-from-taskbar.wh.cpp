
// ==WindhawkMod==
// @id           hide-apps-from-taskbar
// @name         Hide Apps from Taskbar
// @description  Hide specified apps from the taskbar when minimized. Special handling for File Explorer windows.
// @version      1.0.0
// @author       Copilot
// @github       https://github.com/your-github
// @include      *
// @compilerOptions -std=c++20
// ==/WindhawkMod==
// ==WindhawkModSettings==
/*
- appList: "msedge.exe\r\nnotepad.exe"
  $name: App Executables
  $description: List of app executable names (e.g., msedge.exe, notepad.exe) to hide from the taskbar when minimized.
- enableExplorer: false
  $name: Enable for File Explorer windows
  $description: If enabled, hides File Explorer windows from the taskbar when minimized (only actual Explorer windows, not all explorer.exe processes).
*/
// ==/WindhawkModSettings==
// ==WindhawkModReadme==
/*
# Hide Apps from Taskbar

This mod hides specified apps from the taskbar when minimized. It also provides a toggle to enable support for File Explorer windows, ensuring only actual folder windows are affected (not all explorer.exe processes).

## Features
- Hide taskbar button for minimized windows of selected apps
- Special handling for File Explorer: only folder windows are affected
- Restore taskbar button when window is restored
- App list and Explorer toggle configurable in settings

## Settings
- **App Executables**: List of app executable names (e.g., msedge.exe, notepad.exe) to hide from the taskbar when minimized
- **Enable for File Explorer windows**: If enabled, hides File Explorer windows from the taskbar when minimized (only actual Explorer windows, not all explorer.exe processes)

## Technical Details
- Uses Win32 API to change window styles and force refresh
- Detects process name and window class to ensure only intended windows are affected
- Safe and stable: does not impact unrelated windows or processes
*/


#include <windows.h>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <psapi.h> // For GetModuleFileNameExW
#pragma comment(lib, "Psapi.lib")
#include <windhawk_mod.h>

// --- Settings ---

WH_MOD_SETTINGS(
    WH_MOD_SETTING(
        appList,
        L"App Executables",
        L"List of app executable names (e.g., msedge.exe, notepad.exe) to hide from the taskbar when minimized.",
        L"msedge.exe\r\nnotepad.exe"
    )
    WH_MOD_SETTING(
        enableExplorer,
        L"Enable for File Explorer windows",
        L"If enabled, hides File Explorer windows from the taskbar when minimized (only actual Explorer windows, not all explorer.exe processes).",
        false
    )
)

// --- Helper functions ---

static std::unordered_set<std::wstring> g_appSet;
static bool g_enableExplorer = false;

static void UpdateAppSet() {
    g_appSet.clear();
    std::wstring list = Wh_GetStringSetting(L"appList");
    size_t pos = 0;
    while (pos < list.size()) {
        size_t end = list.find_first_of(L"\r\n", pos);
        std::wstring exe = list.substr(pos, end - pos);
        if (!exe.empty()) {
            std::transform(exe.begin(), exe.end(), exe.begin(), ::towlower);
            g_appSet.insert(exe);
        }
        if (end == std::wstring::npos) break;
        pos = end + 1;
    }
    g_enableExplorer = Wh_GetBoolSetting(L"enableExplorer");
}

static std::wstring GetWindowProcessName(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return L"";
    wchar_t exePath[MAX_PATH] = {};
    DWORD len = GetModuleFileNameExW(hProcess, NULL, exePath, MAX_PATH);
    CloseHandle(hProcess);
    if (len == 0) return L"";
    std::wstring exeName = exePath;
    size_t pos = exeName.find_last_of(L"\\/");
    if (pos != std::wstring::npos) exeName = exeName.substr(pos + 1);
    std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::towlower);
    return exeName;
}

// For File Explorer: check if window is a folder window (not all explorer.exe windows)
static bool IsExplorerFolderWindow(HWND hwnd) {
    wchar_t className[64] = {};
    GetClassNameW(hwnd, className, ARRAYSIZE(className));
    // Folder windows are usually "CabinetWClass"
    return wcscmp(className, L"CabinetWClass") == 0;
}

// --- Main logic ---

static void HideTaskbarButton(HWND hwnd, bool hide) {
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (hide) {
        style |= WS_EX_TOOLWINDOW;
        style &= ~WS_EX_APPWINDOW;
    } else {
        style |= WS_EX_APPWINDOW;
        style &= ~WS_EX_TOOLWINDOW;
    }
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, style);
    // Force refresh
    ShowWindow(hwnd, SW_HIDE);
    ShowWindow(hwnd, SW_SHOW);
}

static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        CWPSTRUCT* p = (CWPSTRUCT*)lParam;
        if (p->message == WM_SIZE) {
            if (p->wParam == SIZE_MINIMIZED || p->wParam == SIZE_RESTORED) {
                std::wstring exe = GetWindowProcessName(p->hwnd);
                bool isTarget = g_appSet.count(exe) > 0;
                if (!isTarget && g_enableExplorer && exe == L"explorer.exe" && IsExplorerFolderWindow(p->hwnd)) {
                    isTarget = true;
                }
                if (isTarget) {
                    HideTaskbarButton(p->hwnd, p->wParam == SIZE_MINIMIZED);
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static HHOOK g_hook = NULL;

BOOL Wh_ModInit() {
    UpdateAppSet();
    g_hook = SetWindowsHookExW(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
    return TRUE;
}

void Wh_ModUninit() {
    if (g_hook) {
        UnhookWindowsHookEx(g_hook);
        g_hook = NULL;
    }
}

void Wh_ModSettingsChanged() {
    UpdateAppSet();
}
