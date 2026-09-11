// ==WindhawkMod==
// @id              startallback-flyout-trace
// @name            StartAllBack Flyout Trace
// @description     Black-box trace of shell/WinRT activation used by StartAllBack tray flyout buttons.
// @version         0.4
// @author          Local
// @include         explorer.exe
// @include         ShellHost.exe
// @include         ShellExperienceHost.exe
// @include         StartAllBack*.exe
// @architecture    x86-64
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# StartAllBack Flyout Trace

Enable this together with StartAllBack's enhanced taskbar, then click the
StartAllBack Bluetooth / network tray buttons once each.

This does not inspect or copy StartAllBack code. It only logs public process
behavior around shell activation:

- ShellExecute/ShellExecuteEx URI launches
- CreateProcess command lines
- CoCreateInstance CLSID/RIID pairs
- RoActivateInstance class IDs
- WinRT HSTRING values containing ControlCenter, Bluetooth, QuickAction, etc.

Send the filtered Windhawk log lines back to Codex.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <shellapi.h>
#include <roapi.h>
#include <winstring.h>
#include <winternl.h>
#include <windhawk_utils.h>

#include <cstdarg>
#include <cstdio>
#include <cwchar>

void TraceLog(PCWSTR format, ...) {
    WCHAR message[2048]{};

    va_list args;
    va_start(args, format);
    _vsnwprintf_s(message, ARRAYSIZE(message), _TRUNCATE, format, args);
    va_end(args);

    WCHAR line[2300]{};
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    _snwprintf_s(line, ARRAYSIZE(line), _TRUNCATE,
                 L"[SABTRACE pid=%lu tid=%lu] %s\r\n", pid, tid, message);

    OutputDebugStringW(line);

    WCHAR path[MAX_PATH]{};
    DWORD userProfileLen =
        GetEnvironmentVariableW(L"USERPROFILE", path, ARRAYSIZE(path));
    if (userProfileLen && userProfileLen < ARRAYSIZE(path)) {
        wcscat_s(path, L"\\Desktop\\StartAllBackFlyoutTrace.log");

        HANDLE file = CreateFileW(path, FILE_APPEND_DATA,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE |
                                      FILE_SHARE_DELETE,
                                  nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                  nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            int utf8Bytes = WideCharToMultiByte(CP_UTF8, 0, line, -1, nullptr,
                                                0, nullptr, nullptr);
            if (utf8Bytes > 1) {
                char buffer[4096]{};
                if (utf8Bytes <= static_cast<int>(sizeof(buffer))) {
                    WideCharToMultiByte(CP_UTF8, 0, line, -1, buffer,
                                        utf8Bytes, nullptr, nullptr);
                    DWORD written = 0;
                    WriteFile(file, buffer, utf8Bytes - 1, &written, nullptr);
                }
            }
            CloseHandle(file);
        }
    }
}

#define Wh_Log TraceLog

using ShellExecuteExW_t = BOOL(WINAPI*)(SHELLEXECUTEINFOW*);
using ShellExecuteW_t = HINSTANCE(WINAPI*)(HWND,
                                           LPCWSTR,
                                           LPCWSTR,
                                           LPCWSTR,
                                           LPCWSTR,
                                           INT);
using CreateProcessW_t = BOOL(WINAPI*)(LPCWSTR,
                                        LPWSTR,
                                        LPSECURITY_ATTRIBUTES,
                                        LPSECURITY_ATTRIBUTES,
                                        BOOL,
                                        DWORD,
                                        LPVOID,
                                        LPCWSTR,
                                        LPSTARTUPINFOW,
                                        LPPROCESS_INFORMATION);
using CoCreateInstance_t = HRESULT(WINAPI*)(REFCLSID,
                                            LPUNKNOWN,
                                            DWORD,
                                            REFIID,
                                            LPVOID*);
using RoActivateInstance_t = HRESULT(WINAPI*)(HSTRING, IInspectable**);
using WindowsCreateString_t = HRESULT(WINAPI*)(PCWSTR, UINT32, HSTRING*);
using WindowsCreateStringReference_t =
    HRESULT(WINAPI*)(PCWSTR, UINT32, HSTRING_HEADER*, HSTRING*);
using WindowsGetStringRawBuffer_t = PCWSTR(WINAPI*)(HSTRING, UINT32*);
using LdrLoadDll_t = NTSTATUS(NTAPI*)(PWSTR, ULONG, PUNICODE_STRING, PHANDLE);
using ControlCenterViewCtor_t = void(__fastcall*)(void* self);
using InitializeAndNavigateToL2Page1_t =
    void(__fastcall*)(void* self, HSTRING page);
using InitializeAndNavigateToL2Page2_t =
    void(__fastcall*)(void* self, HSTRING page, HSTRING advanced);
using OnL2FrameContentChanged_t =
    void(__fastcall*)(void* self, void* sender, void* args);

ShellExecuteExW_t g_shellExecuteExWOriginal = nullptr;
ShellExecuteW_t g_shellExecuteWOriginal = nullptr;
CreateProcessW_t g_createProcessWOriginal = nullptr;
CoCreateInstance_t g_coCreateInstanceOriginal = nullptr;
RoActivateInstance_t g_roActivateInstanceOriginal = nullptr;
WindowsCreateString_t g_windowsCreateStringOriginal = nullptr;
WindowsCreateStringReference_t g_windowsCreateStringReferenceOriginal =
    nullptr;
WindowsGetStringRawBuffer_t g_windowsGetStringRawBuffer = nullptr;
LdrLoadDll_t g_ldrLoadDllOriginal = nullptr;
ControlCenterViewCtor_t g_controlCenterViewCtorOriginal = nullptr;
InitializeAndNavigateToL2Page1_t g_initL2OneOriginal = nullptr;
InitializeAndNavigateToL2Page2_t g_initL2TwoOriginal = nullptr;
OnL2FrameContentChanged_t g_onL2FrameContentChangedOriginal = nullptr;

volatile LONG g_controlCenterHooked = 0;

bool ContainsInsensitive(PCWSTR haystack, PCWSTR needle) {
    if (!haystack || !needle || !*needle) {
        return false;
    }

    const size_t needleLen = wcslen(needle);
    for (PCWSTR cursor = haystack; *cursor; ++cursor) {
        if (_wcsnicmp(cursor, needle, needleLen) == 0) {
            return true;
        }
    }

    return false;
}

bool InterestingText(PCWSTR text) {
    if (!text || !*text) {
        return false;
    }

    return ContainsInsensitive(text, L"controlcenter") ||
           ContainsInsensitive(text, L"quickaction") ||
           ContainsInsensitive(text, L"bluetooth") ||
           ContainsInsensitive(text, L"network") ||
           ContainsInsensitive(text, L"devicesflow") ||
           ContainsInsensitive(text, L"ms-availablenetworks") ||
           ContainsInsensitive(text, L"ms-controlcenter") ||
           ContainsInsensitive(text, L"ShellExperience");
}

bool InterestingDllName(PCWSTR text) {
    if (!text || !*text) {
        return false;
    }

    return ContainsInsensitive(text, L"StartAllBack") ||
           ContainsInsensitive(text, L"SAB") ||
           ContainsInsensitive(text, L"ControlCenter") ||
           ContainsInsensitive(text, L"DevicesFlow") ||
           ContainsInsensitive(text, L"twinui.pcshell") ||
           ContainsInsensitive(text, L"ShellExperience");
}

void LogInterestingHString(PCWSTR source, HSTRING string) {
    if (!string || !g_windowsGetStringRawBuffer) {
        return;
    }

    UINT32 length = 0;
    PCWSTR text = g_windowsGetStringRawBuffer(string, &length);
    if (InterestingText(text)) {
        Wh_Log(L"%s HSTRING: [%.*s]", source, length, text);
    }
}

void LogAnyHString(PCWSTR source, HSTRING string) {
    if (!string || !g_windowsGetStringRawBuffer) {
        Wh_Log(L"%s HSTRING: <null/unavailable>", source);
        return;
    }

    UINT32 length = 0;
    PCWSTR text = g_windowsGetStringRawBuffer(string, &length);
    Wh_Log(L"%s HSTRING: [%.*s]", source, length, text ? text : L"");
}

void __fastcall ControlCenterViewCtor_Hook(void* self) {
    Wh_Log(L"ControlCenterView::ctor self=%p", self);
    g_controlCenterViewCtorOriginal(self);
}

void __fastcall InitializeAndNavigateToL2Page1_Hook(void* self, HSTRING page) {
    Wh_Log(L"ControlCenterView::InitializeAndNavigateToL2Page(one) self=%p",
           self);
    LogAnyHString(L"  page", page);
    g_initL2OneOriginal(self, page);
}

void __fastcall InitializeAndNavigateToL2Page2_Hook(void* self,
                                                    HSTRING page,
                                                    HSTRING advanced) {
    Wh_Log(L"ControlCenterView::InitializeAndNavigateToL2Page(two) self=%p",
           self);
    LogAnyHString(L"  page", page);
    LogAnyHString(L"  advanced", advanced);
    g_initL2TwoOriginal(self, page, advanced);
}

void __fastcall OnL2FrameContentChanged_Hook(void* self,
                                             void* sender,
                                             void* args) {
    Wh_Log(L"ControlCenterView::OnL2FrameContentChanged self=%p sender=%p "
           L"args=%p",
           self, sender, args);
    g_onL2FrameContentChangedOriginal(self, sender, args);
}

bool HookControlCenter(HMODULE module, bool applyNow) {
    if (!module || InterlockedExchange(&g_controlCenterHooked, 1)) {
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(public: __cdecl winrt::ControlCenter::implementation::ControlCenterView::ControlCenterView(void))"},
            reinterpret_cast<void**>(&g_controlCenterViewCtorOriginal),
            reinterpret_cast<void*>(ControlCenterViewCtor_Hook),
        },
        {
            {LR"(public: void __cdecl winrt::ControlCenter::implementation::ControlCenterView::InitializeAndNavigateToL2Page(struct winrt::hstring))"},
            reinterpret_cast<void**>(&g_initL2OneOriginal),
            reinterpret_cast<void*>(InitializeAndNavigateToL2Page1_Hook),
        },
        {
            {LR"(public: void __cdecl winrt::ControlCenter::implementation::ControlCenterView::InitializeAndNavigateToL2Page(struct winrt::hstring,struct winrt::hstring))"},
            reinterpret_cast<void**>(&g_initL2TwoOriginal),
            reinterpret_cast<void*>(InitializeAndNavigateToL2Page2_Hook),
        },
        {
            {LR"(public: void __cdecl winrt::ControlCenter::implementation::ControlCenterView::OnL2FrameContentChanged(struct winrt::Windows::Foundation::IInspectable const &,struct winrt::Windows::Foundation::IInspectable const &))"},
            reinterpret_cast<void**>(&g_onL2FrameContentChangedOriginal),
            reinterpret_cast<void*>(OnL2FrameContentChanged_Hook),
        },
    };

    if (!WindhawkUtils::HookSymbols(module, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"ControlCenter symbol trace did not resolve every requested "
               L"symbol.");
    }

    if (applyNow) {
        Wh_ApplyHookOperations();
    }

    Wh_Log(L"ControlCenter symbol trace armed for module=%p", module);
    return true;
}

BOOL WINAPI ShellExecuteExW_Hook(SHELLEXECUTEINFOW* info) {
    if (info &&
        (InterestingText(info->lpFile) || InterestingText(info->lpParameters) ||
         InterestingText(info->lpDirectory) || InterestingText(info->lpVerb))) {
        Wh_Log(L"ShellExecuteExW verb=[%s] file=[%s] params=[%s] dir=[%s]",
               info->lpVerb ? info->lpVerb : L"",
               info->lpFile ? info->lpFile : L"",
               info->lpParameters ? info->lpParameters : L"",
               info->lpDirectory ? info->lpDirectory : L"");
    }

    return g_shellExecuteExWOriginal(info);
}

HINSTANCE WINAPI ShellExecuteW_Hook(HWND hwnd,
                                    LPCWSTR operation,
                                    LPCWSTR file,
                                    LPCWSTR parameters,
                                    LPCWSTR directory,
                                    INT showCmd) {
    if (InterestingText(operation) || InterestingText(file) ||
        InterestingText(parameters) || InterestingText(directory)) {
        Wh_Log(L"ShellExecuteW op=[%s] file=[%s] params=[%s] dir=[%s] show=%d",
               operation ? operation : L"", file ? file : L"",
               parameters ? parameters : L"", directory ? directory : L"",
               showCmd);
    }

    return g_shellExecuteWOriginal(hwnd, operation, file, parameters, directory,
                                   showCmd);
}

BOOL WINAPI CreateProcessW_Hook(LPCWSTR appName,
                                LPWSTR commandLine,
                                LPSECURITY_ATTRIBUTES processAttributes,
                                LPSECURITY_ATTRIBUTES threadAttributes,
                                BOOL inheritHandles,
                                DWORD creationFlags,
                                LPVOID environment,
                                LPCWSTR currentDirectory,
                                LPSTARTUPINFOW startupInfo,
                                LPPROCESS_INFORMATION processInformation) {
    if (InterestingText(appName) || InterestingText(commandLine) ||
        InterestingText(currentDirectory)) {
        Wh_Log(L"CreateProcessW app=[%s] cmd=[%s] cwd=[%s]",
               appName ? appName : L"",
               commandLine ? commandLine : L"",
               currentDirectory ? currentDirectory : L"");
    }

    return g_createProcessWOriginal(
        appName, commandLine, processAttributes, threadAttributes,
        inheritHandles, creationFlags, environment, currentDirectory,
        startupInfo, processInformation);
}

HRESULT WINAPI CoCreateInstance_Hook(REFCLSID clsid,
                                     LPUNKNOWN outer,
                                     DWORD clsContext,
                                     REFIID iid,
                                     LPVOID* object) {
    HRESULT hr =
        g_coCreateInstanceOriginal(clsid, outer, clsContext, iid, object);

    // Log every CoCreateInstance while doing the SAB click trace. It is noisy,
    // but the interesting activation might be a GUID we don't know yet.
    Wh_Log(L"CoCreateInstance clsid={%08lX-%04X-%04X-%02X%02X-"
           L"%02X%02X%02X%02X%02X%02X} iid={%08lX-%04X-%04X-%02X%02X-"
           L"%02X%02X%02X%02X%02X%02X} ctx=0x%08X hr=0x%08X object=%p",
           clsid.Data1, clsid.Data2, clsid.Data3, clsid.Data4[0],
           clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
           clsid.Data4[4], clsid.Data4[5], clsid.Data4[6],
           clsid.Data4[7], iid.Data1, iid.Data2, iid.Data3, iid.Data4[0],
           iid.Data4[1], iid.Data4[2], iid.Data4[3], iid.Data4[4],
           iid.Data4[5], iid.Data4[6], iid.Data4[7], clsContext, hr,
           object ? *object : nullptr);

    return hr;
}

HRESULT WINAPI RoActivateInstance_Hook(HSTRING classId,
                                       IInspectable** instance) {
    LogInterestingHString(L"RoActivateInstance", classId);

    HRESULT hr = g_roActivateInstanceOriginal(classId, instance);

    if (classId && g_windowsGetStringRawBuffer) {
        UINT32 length = 0;
        PCWSTR text = g_windowsGetStringRawBuffer(classId, &length);
        if (InterestingText(text)) {
            Wh_Log(L"RoActivateInstance returned hr=0x%08X class=[%.*s] "
                   L"instance=%p",
                   hr, length, text, instance ? *instance : nullptr);
        }
    }

    return hr;
}

HRESULT WINAPI WindowsCreateString_Hook(PCWSTR sourceString,
                                        UINT32 length,
                                        HSTRING* string) {
    if (InterestingText(sourceString)) {
        Wh_Log(L"WindowsCreateString source=[%.*s]", length, sourceString);
    }

    HRESULT hr = g_windowsCreateStringOriginal(sourceString, length, string);
    if (InterestingText(sourceString)) {
        Wh_Log(L"WindowsCreateString returned hr=0x%08X hstring=%p", hr,
               string ? *string : nullptr);
    }

    return hr;
}

HRESULT WINAPI WindowsCreateStringReference_Hook(PCWSTR sourceString,
                                                 UINT32 length,
                                                 HSTRING_HEADER* header,
                                                 HSTRING* string) {
    if (InterestingText(sourceString)) {
        Wh_Log(L"WindowsCreateStringReference source=[%.*s]", length,
               sourceString);
    }

    HRESULT hr = g_windowsCreateStringReferenceOriginal(sourceString, length,
                                                       header, string);
    if (InterestingText(sourceString)) {
        Wh_Log(L"WindowsCreateStringReference returned hr=0x%08X hstring=%p",
               hr, string ? *string : nullptr);
    }

    return hr;
}

NTSTATUS NTAPI LdrLoadDll_Hook(PWSTR searchPath,
                               ULONG flags,
                               PUNICODE_STRING moduleFileName,
                               PHANDLE moduleHandle) {
    NTSTATUS status =
        g_ldrLoadDllOriginal(searchPath, flags, moduleFileName, moduleHandle);

    if (moduleFileName && moduleFileName->Buffer) {
        const int cch = moduleFileName->Length / sizeof(wchar_t);
        if (cch > 0) {
            WCHAR buffer[MAX_PATH]{};
            const int maxCopy = static_cast<int>(ARRAYSIZE(buffer) - 1);
            const int copy = cch < maxCopy ? cch : maxCopy;
            wcsncpy_s(buffer, moduleFileName->Buffer, copy);

            if (InterestingDllName(buffer)) {
                Wh_Log(L"LdrLoadDll name=[%s] status=0x%08X module=%p",
                       buffer, status,
                       moduleHandle ? reinterpret_cast<void*>(*moduleHandle)
                                    : nullptr);
            }

            if (ContainsInsensitive(buffer, L"ControlCenter.dll") && status >= 0 &&
                moduleHandle && *moduleHandle) {
                HookControlCenter(reinterpret_cast<HMODULE>(*moduleHandle),
                                  true);
            }
        }
    }

    return status;
}

BOOL Wh_ModInit() {
    HMODULE shell32 = GetModuleHandleW(L"shell32.dll");
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    HMODULE combase = GetModuleHandleW(L"combase.dll");
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

    WCHAR processPath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, processPath, ARRAYSIZE(processPath));
    Wh_Log(L"StartAllBack flyout trace initializing in process [%s].",
           processPath);

    if (!shell32 || !kernel32 || !combase || !ntdll) {
        Wh_Log(L"Required module missing: shell32=%p kernel32=%p combase=%p "
               L"ntdll=%p",
               shell32, kernel32, combase, ntdll);
        return FALSE;
    }

    g_windowsGetStringRawBuffer =
        reinterpret_cast<WindowsGetStringRawBuffer_t>(
            GetProcAddress(combase, "WindowsGetStringRawBuffer"));

    if (!g_windowsGetStringRawBuffer) {
        Wh_Log(L"WindowsGetStringRawBuffer not resolved.");
        return FALSE;
    }

    void* shellExecuteExW = reinterpret_cast<void*>(
        GetProcAddress(shell32, "ShellExecuteExW"));
    void* shellExecuteW = reinterpret_cast<void*>(
        GetProcAddress(shell32, "ShellExecuteW"));
    void* createProcessW = reinterpret_cast<void*>(
        GetProcAddress(kernel32, "CreateProcessW"));
    void* coCreateInstance = reinterpret_cast<void*>(
        GetProcAddress(combase, "CoCreateInstance"));
    void* roActivateInstance = reinterpret_cast<void*>(
        GetProcAddress(combase, "RoActivateInstance"));
    void* windowsCreateString = reinterpret_cast<void*>(
        GetProcAddress(combase, "WindowsCreateString"));
    void* windowsCreateStringReference = reinterpret_cast<void*>(
        GetProcAddress(combase, "WindowsCreateStringReference"));
    void* ldrLoadDll = reinterpret_cast<void*>(
        GetProcAddress(ntdll, "LdrLoadDll"));

    Wh_Log(L"Trace hook targets: ShellExecuteExW=%p ShellExecuteW=%p "
           L"CreateProcessW=%p CoCreateInstance=%p RoActivateInstance=%p "
           L"WindowsCreateString=%p WindowsCreateStringReference=%p "
           L"LdrLoadDll=%p",
           shellExecuteExW, shellExecuteW, createProcessW, coCreateInstance,
           roActivateInstance, windowsCreateString, windowsCreateStringReference,
           ldrLoadDll);

    Wh_SetFunctionHook(shellExecuteExW,
                       reinterpret_cast<void*>(ShellExecuteExW_Hook),
                       reinterpret_cast<void**>(&g_shellExecuteExWOriginal));

    Wh_SetFunctionHook(shellExecuteW,
                       reinterpret_cast<void*>(ShellExecuteW_Hook),
                       reinterpret_cast<void**>(&g_shellExecuteWOriginal));

    Wh_SetFunctionHook(createProcessW,
                       reinterpret_cast<void*>(CreateProcessW_Hook),
                       reinterpret_cast<void**>(&g_createProcessWOriginal));

    Wh_SetFunctionHook(coCreateInstance,
                       reinterpret_cast<void*>(CoCreateInstance_Hook),
                       reinterpret_cast<void**>(&g_coCreateInstanceOriginal));

    Wh_SetFunctionHook(roActivateInstance,
                       reinterpret_cast<void*>(RoActivateInstance_Hook),
                       reinterpret_cast<void**>(&g_roActivateInstanceOriginal));

    Wh_SetFunctionHook(windowsCreateString,
                       reinterpret_cast<void*>(WindowsCreateString_Hook),
                       reinterpret_cast<void**>(&g_windowsCreateStringOriginal));

    Wh_SetFunctionHook(windowsCreateStringReference,
                       reinterpret_cast<void*>(WindowsCreateStringReference_Hook),
                       reinterpret_cast<void**>(
                           &g_windowsCreateStringReferenceOriginal));

    Wh_SetFunctionHook(ldrLoadDll, reinterpret_cast<void*>(LdrLoadDll_Hook),
                       reinterpret_cast<void**>(&g_ldrLoadDllOriginal));

    if (HMODULE controlCenter = GetModuleHandleW(L"ControlCenter.dll")) {
        HookControlCenter(controlCenter, false);
    }

    Wh_Log(L"StartAllBack flyout trace loaded.");
    return TRUE;
}
