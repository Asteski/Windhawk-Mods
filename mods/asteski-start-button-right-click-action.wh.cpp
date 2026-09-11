// ==WindhawkMod==
// @id           start-button-advanced-actions
// @name         Start Button Advanced Actions
// @description  Assign custom actions to Start button left/right click and Shift+clicks
// @version      1.0.0
// @author       Asteski
// @github       https://github.com/Asteski
// @include      explorer.exe
// @compilerOptions -lgdi32 -luser32 -lshell32 -ladvapi32 -lole32 -loleaut32 -luuid
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Start Button Advanced Actions

This mod allows you to assign custom actions to the Start button left-click, right-click, Shift+left-click, and Shift+right-click. All other Start button and Windows key actions remain unchanged.

## Features

- **Start Button Left Click Action**: Configure a custom command or disable the left-click on the Start button
- **Start Button Right Click Action**: Configure a custom command or disable the right-click on the Start button
- **Shift+Left/Right Click Actions**: Assign different actions for Shift+left-click and Shift+right-click
- **Default Behavior**: All other Start button and Windows key actions are not affected
- **Environment Variables**: Support for environment variables in custom commands (e.g., %USERPROFILE%)

## Settings

- **Left Click**: Default Start menu, custom command, or disabled
- **Right Click**: Default WinX menu, custom command, or disabled
- **Shift+Left Click**: Custom command or disabled
- **Shift+Right Click**: Custom command or disabled

### Command Examples
- `notepad.exe` - Opens Notepad
- `cmd.exe` - Opens Command Prompt
- `%PROGRAMFILES%\\Everything\\Everything.exe` - Launch Everything search
- `powershell.exe -Command "Get-Process"` - PowerShell command

## Compatibility

- Works on Windows 10 and Windows 11
- Does not interfere with existing Windows functionality except for the Start button actions you override
- Can be safely disabled/enabled without system restart
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
startButtonLeftClickAction: default
    $name: Start Button Left Click Action
    $description: What to do when Start button is left-clicked
    $options:
        - default: Default (normal behavior)
        - custom: Custom Command
        - disabled: Disabled (do nothing)
startButtonLeftClickCommand: explorer.exe %USERPROFILE%
    $name: Start Button Left Click Command
    $description: Command to execute for Start button left click (default opens user profile folder)
startButtonRightClickAction: default
    $name: Start Button Right Click Action
    $description: What to do when Start button is right-clicked
    $options:
        - default: Default (normal behavior)
        - custom: Custom Command
        - disabled: Disabled (do nothing)
startButtonRightClickCommand: cmd.exe /k systeminfo
    $name: Start Button Right Click Command
    $description: Command to execute for Start button right click (default shows system information)
startButtonShiftLeftClickAction: default
    $name: Start Button Shift+Left Click Action
    $description: What to do when Start button is Shift+left-clicked
    $options:
        - default: Default (normal behavior)
        - custom: Custom Command
        - disabled: Disabled (do nothing)
startButtonShiftLeftClickCommand: taskmgr.exe
    $name: Start Button Shift+Left Click Command
    $description: Command to execute for Start button Shift+left click (default opens Task Manager)
startButtonShiftRightClickAction: default
    $name: Start Button Shift+Right Click Action
    $description: What to do when Start button is Shift+right-clicked
    $options:
        - default: Default (normal behavior)
        - custom: Custom Command
        - disabled: Disabled (do nothing)
startButtonShiftRightClickCommand: notepad.exe
    $name: Start Button Shift+Right Click Command
    $description: Command to execute for Start button Shift+right click (default opens Notepad)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <string>
#include <atomic>
#ifndef Wh_Log
#define Wh_Log(...) do {} while (0)
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

// Global variables
HHOOK g_mouseHook = nullptr;
std::atomic<bool> g_quit{false};
HANDLE g_executeStartButtonEvent = nullptr;
std::atomic<int> g_startButtonClickType{0}; // 0=none, 1=left, 2=right, 3=shift+left, 4=shift+right
HWND g_startButtonHwnd = nullptr;
RECT g_startButtonRect = {0};
std::atomic<bool> g_suppressNextMouseUp{false};

// Settings
std::string g_startButtonLeftClickAction = "default";
std::string g_startButtonLeftClickCommand = "explorer.exe %USERPROFILE%";
std::string g_startButtonRightClickAction = "default";
std::string g_startButtonRightClickCommand = "cmd.exe /k systeminfo";
std::string g_startButtonShiftLeftClickAction = "default";
std::string g_startButtonShiftLeftClickCommand = "taskmgr.exe";
std::string g_startButtonShiftRightClickAction = "default";
std::string g_startButtonShiftRightClickCommand = "notepad.exe";

void LoadSettings() {
    PCWSTR startButtonLeftClickAction = Wh_GetStringSetting(L"startButtonLeftClickAction");
    PCWSTR startButtonLeftClickCommand = Wh_GetStringSetting(L"startButtonLeftClickCommand");
    PCWSTR startButtonRightClickAction = Wh_GetStringSetting(L"startButtonRightClickAction");
    PCWSTR startButtonRightClickCommand = Wh_GetStringSetting(L"startButtonRightClickCommand");
    PCWSTR startButtonShiftLeftClickAction = Wh_GetStringSetting(L"startButtonShiftLeftClickAction");
    PCWSTR startButtonShiftLeftClickCommand = Wh_GetStringSetting(L"startButtonShiftLeftClickCommand");
    PCWSTR startButtonShiftRightClickAction = Wh_GetStringSetting(L"startButtonShiftRightClickAction");
    PCWSTR startButtonShiftRightClickCommand = Wh_GetStringSetting(L"startButtonShiftRightClickCommand");

    if (startButtonLeftClickAction) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonLeftClickAction, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonLeftClickAction = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonLeftClickAction, -1, &g_startButtonLeftClickAction[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonLeftClickAction);
    } else {
        g_startButtonLeftClickAction = "default";
    }
    if (startButtonLeftClickCommand) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonLeftClickCommand, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonLeftClickCommand = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonLeftClickCommand, -1, &g_startButtonLeftClickCommand[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonLeftClickCommand);
    } else {
        g_startButtonLeftClickCommand = "explorer.exe %USERPROFILE%";
    }
    if (startButtonRightClickAction) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonRightClickAction, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonRightClickAction = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonRightClickAction, -1, &g_startButtonRightClickAction[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonRightClickAction);
    } else {
        g_startButtonRightClickAction = "default";
    }
    if (startButtonRightClickCommand) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonRightClickCommand, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonRightClickCommand = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonRightClickCommand, -1, &g_startButtonRightClickCommand[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonRightClickCommand);
    } else {
        g_startButtonRightClickCommand = "cmd.exe /k systeminfo";
    }
    if (startButtonShiftLeftClickAction) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonShiftLeftClickAction, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonShiftLeftClickAction = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonShiftLeftClickAction, -1, &g_startButtonShiftLeftClickAction[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonShiftLeftClickAction);
    } else {
        g_startButtonShiftLeftClickAction = "default";
    }
    if (startButtonShiftLeftClickCommand) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonShiftLeftClickCommand, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonShiftLeftClickCommand = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonShiftLeftClickCommand, -1, &g_startButtonShiftLeftClickCommand[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonShiftLeftClickCommand);
    } else {
        g_startButtonShiftLeftClickCommand = "taskmgr.exe";
    }
    if (startButtonShiftRightClickAction) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonShiftRightClickAction, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonShiftRightClickAction = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonShiftRightClickAction, -1, &g_startButtonShiftRightClickAction[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonShiftRightClickAction);
    } else {
        g_startButtonShiftRightClickAction = "default";
    }
    if (startButtonShiftRightClickCommand) {
        int len = WideCharToMultiByte(CP_UTF8, 0, startButtonShiftRightClickCommand, -1, nullptr, 0, nullptr, nullptr);
        g_startButtonShiftRightClickCommand = std::string(len ? len - 1 : 0, '\0');
        if (len)
            WideCharToMultiByte(CP_UTF8, 0, startButtonShiftRightClickCommand, -1, &g_startButtonShiftRightClickCommand[0], len, nullptr, nullptr);
        Wh_FreeStringSetting(startButtonShiftRightClickCommand);
    } else {
        g_startButtonShiftRightClickCommand = "notepad.exe";
    }
}

void UpdateStartButtonInfo() {
    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!taskbar) {
        g_startButtonHwnd = nullptr;
        return;
    }
    HWND startButton = FindWindowExW(taskbar, nullptr, L"Start", nullptr);
    if (!startButton) {
        startButton = FindWindowExW(taskbar, nullptr, L"Windows.UI.Input.InputSite.WindowClass", nullptr);
    }
    if (startButton) {
        g_startButtonHwnd = startButton;
        GetWindowRect(startButton, &g_startButtonRect);
    } else {
        g_startButtonHwnd = nullptr;
    }
}

void ExecuteCommand(const std::string& command) {
    if (command.empty()) return;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return;
    std::wstring wcommand(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wcommand[0], wlen);
    wchar_t expandedCommand[MAX_PATH * 2];
    DWORD result = ExpandEnvironmentStringsW(wcommand.c_str(), expandedCommand, _countof(expandedCommand));
    if (result == 0 || result > _countof(expandedCommand)) {
        wcscpy_s(expandedCommand, wcommand.c_str());
    }
    std::wstring expandedStr(expandedCommand);
    std::wstring executable;
    std::wstring parameters;
    if (!expandedStr.empty() && expandedStr[0] == L'"') {
        size_t endQuote = expandedStr.find(L'"', 1);
        if (endQuote != std::wstring::npos) {
            executable = expandedStr.substr(1, endQuote - 1);
            if (endQuote + 1 < expandedStr.size()) {
                if (expandedStr[endQuote + 1] == L' ') {
                    parameters = expandedStr.substr(endQuote + 2);
                } else {
                    parameters = expandedStr.substr(endQuote + 1);
                }
            }
        } else {
            executable = expandedStr;
        }
    } else {
        size_t spacePos = expandedStr.find(L' ');
        if (spacePos != std::wstring::npos) {
            executable = expandedStr.substr(0, spacePos);
            parameters = expandedStr.substr(spacePos + 1);
        } else {
            executable = expandedStr;
        }
    }
    HINSTANCE hResult = ShellExecuteW(nullptr, L"open", executable.c_str(),
                                      parameters.empty() ? nullptr : parameters.c_str(),
                                      nullptr, SW_SHOWNORMAL);
    if ((INT_PTR)hResult <= 32) {
        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);
        wchar_t cmdCopy[MAX_PATH * 2];
        wcscpy_s(cmdCopy, expandedCommand);
        if (CreateProcessW(nullptr, cmdCopy, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}

void ExecuteStartButtonAction() {
    int clickType = g_startButtonClickType.load();
    std::string action;
    std::string command;
    switch (clickType) {
        case 1: // Left click
            action = g_startButtonLeftClickAction;
            command = g_startButtonLeftClickCommand;
            break;
        case 2: // Right click
            action = g_startButtonRightClickAction;
            command = g_startButtonRightClickCommand;
            break;
        case 3: // Shift+Left click
            action = g_startButtonShiftLeftClickAction;
            command = g_startButtonShiftLeftClickCommand;
            break;
        case 4: // Shift+Right click
            action = g_startButtonShiftRightClickAction;
            command = g_startButtonShiftRightClickCommand;
            break;
        default:
            return;
    }
    if (action == "custom") {
        ExecuteCommand(command);
    }
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT* mouse = (MSLLHOOKSTRUCT*)lParam;
        if (!g_startButtonHwnd) {
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        }
        POINT pt = mouse->pt;
        if (!PtInRect(&g_startButtonRect, pt)) {
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        }
        if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP) {
            if (g_suppressNextMouseUp) {
                g_suppressNextMouseUp = false;
                return 1;
            }
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        }
        std::string action;
        int clickType = 0;
        bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        if (wParam == WM_LBUTTONDOWN) {
            if (shiftPressed) {
                action = g_startButtonShiftLeftClickAction;
                clickType = 3;
            } else {
                action = g_startButtonLeftClickAction;
                clickType = 1;
            }
        } else if (wParam == WM_RBUTTONDOWN) {
            if (shiftPressed) {
                action = g_startButtonShiftRightClickAction;
                clickType = 4;
            } else {
                action = g_startButtonRightClickAction;
                clickType = 2;
            }
        } else {
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        }
        if (action == "default") {
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        }
        if (action == "custom") {
            g_startButtonClickType = clickType;
            ExecuteStartButtonAction();
        }
        g_suppressNextMouseUp = true;
        return 1;
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

BOOL Wh_ModInit() {
    LoadSettings();
    UpdateStartButtonInfo();
    g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);
    return g_mouseHook != nullptr;
}

void Wh_ModUninit() {
    if (g_mouseHook) {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = nullptr;
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    UpdateStartButtonInfo();
}
