// ==WindhawkMod==
// @id              trace-quick-settings-l2-uri
// @name            Trace Quick Settings L2 URI
// @description     Diagnostic: records the native Quick Settings L2 URI published by each quick action.
// @version         1.0
// @author          Local
// @include         ShellHost.exe
// @architecture    x86-64
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Trace Quick Settings L2 URI

This diagnostic mod does not change any UI or invoke any action. Its purpose is
to discover the exact L2-page URI that Windows assigns to the Bluetooth quick
action on the current Insider build.

## Use

1. Enable the mod and restart `ShellHost.exe` (or sign out and back in).
2. Press Win+A once. Do not click any quick-action arrow.
3. Open the mod's Windhawk log and copy the lines beginning `Quick action`.

The important result is the pair which has an Id containing `Bluetooth`:

```text
Quick action vm=... Id=[...Bluetooth...]
Quick action vm=... UriPathAdvanced=[...]
```

The second value is the only value suitable for
`ms-controlcenter:?L2PageUriPath=...`. This avoids inferring a page path from
XAML filenames or accessibility properties.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <winternl.h>
#include <windhawk_utils.h>

using HSTRING_ = void*;
using WindowsGetStringRawBuffer_t = PCWSTR(WINAPI*)(HSTRING_, UINT32*);
using WindowsDeleteString_t = HRESULT(WINAPI*)(HSTRING_);
using QuickActionStringGet_t = HSTRING_(__fastcall*)(void*);
struct HStringByValue {
    HSTRING_ handle;
};
using QuickActionViewModelConstruct_t = void(__fastcall*)(void*, HStringByValue*);
using LdrLoadDll_t = NTSTATUS(NTAPI*)(PWSTR, ULONG, PUNICODE_STRING, PHANDLE);

WindowsGetStringRawBuffer_t g_windowsGetStringRawBuffer = nullptr;
WindowsDeleteString_t g_windowsDeleteString = nullptr;
QuickActionStringGet_t g_quickActionIdOriginal = nullptr;
QuickActionStringGet_t g_uriPathAdvancedOriginal = nullptr;
QuickActionViewModelConstruct_t g_quickActionViewModelConstructOriginal = nullptr;
LdrLoadDll_t g_ldrLoadDllOriginal = nullptr;
volatile LONG g_controlCenterHooked = 0;

void LogString(PCWSTR property, void* viewModel, HSTRING_ value) {
    UINT32 length = 0;
    PCWSTR text = g_windowsGetStringRawBuffer
                      ? g_windowsGetStringRawBuffer(value, &length)
                      : nullptr;
    Wh_Log(L"Quick action vm=%p %s=[%.*s]", viewModel, property,
           static_cast<int>(length), text ? text : L"");
}

HSTRING_ __fastcall QuickActionIdHook(void* viewModel) {
    HSTRING_ result = g_quickActionIdOriginal(viewModel);
    LogString(L"Id", viewModel, result);
    return result;
}

HSTRING_ __fastcall UriPathAdvancedHook(void* viewModel) {
    HSTRING_ result = g_uriPathAdvancedOriginal(viewModel);
    LogString(L"UriPathAdvanced", viewModel, result);
    return result;
}

// On this build, XAML sometimes reads the backing fields directly and never
// calls the public getters. The constructor is the reliable point at which a
// fresh Quick Action model enters ControlCenter.dll.
void __fastcall QuickActionViewModelConstructHook(void* viewModel,
                                                   HStringByValue* id) {
    if (id) {
        LogString(L"constructor argument", viewModel, id->handle);
    }

    g_quickActionViewModelConstructOriginal(viewModel, id);

    // Query the now-fully-created object ourselves. These results are owned by
    // this call, hence the matching WindowsDeleteString calls below.
    HSTRING_ actionId = g_quickActionIdOriginal(viewModel);
    LogString(L"Id (construction)", viewModel, actionId);
    if (actionId && g_windowsDeleteString) {
        g_windowsDeleteString(actionId);
    }

    HSTRING_ uri = g_uriPathAdvancedOriginal(viewModel);
    LogString(L"UriPathAdvanced (construction)", viewModel, uri);
    if (uri && g_windowsDeleteString) {
        g_windowsDeleteString(uri);
    }
}

bool HookControlCenter(HMODULE module, bool applyNow) {
    if (!module || InterlockedExchange(&g_controlCenterHooked, 1)) {
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(public: virtual struct winrt::hstring __cdecl winrt::ControlCenter::implementation::QuickActionViewModel::Id(void))"},
            (void**)&g_quickActionIdOriginal,
            (void*)QuickActionIdHook,
        },
        {
            {LR"(public: virtual struct winrt::hstring __cdecl winrt::ControlCenter::implementation::QuickActionViewModel::UriPathAdvanced(void))"},
            (void**)&g_uriPathAdvancedOriginal,
            (void*)UriPathAdvancedHook,
        },
        {
            {LR"(public: __cdecl winrt::ControlCenter::implementation::QuickActionViewModel::QuickActionViewModel(struct winrt::hstring))"},
            (void**)&g_quickActionViewModelConstructOriginal,
            (void*)QuickActionViewModelConstructHook,
        },
    };

    if (!WindhawkUtils::HookSymbols(module, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"Could not resolve the QuickActionViewModel getters. Public symbols may have changed.");
        return false;
    }

    if (applyNow) {
        Wh_ApplyHookOperations();
    }

    Wh_Log(L"Quick Settings L2 URI trace armed.");
    return true;
}

NTSTATUS NTAPI LdrLoadDllHook(PWSTR searchPath,
                              ULONG flags,
                              PUNICODE_STRING moduleFileName,
                              PHANDLE moduleHandle) {
    NTSTATUS status = g_ldrLoadDllOriginal(searchPath, flags, moduleFileName,
                                           moduleHandle);
    if (NT_SUCCESS(status) && moduleHandle && *moduleHandle) {
        HMODULE result = reinterpret_cast<HMODULE>(*moduleHandle);
        PCWSTR name = moduleFileName ? moduleFileName->Buffer : nullptr;
        USHORT bytes = moduleFileName ? moduleFileName->Length : 0;
        if (name && bytes >= sizeof(L"ControlCenter.dll") - sizeof(wchar_t) &&
            _wcsnicmp(name + (bytes / sizeof(wchar_t)) -
                          (ARRAYSIZE(L"ControlCenter.dll") - 1),
                      L"ControlCenter.dll",
                      ARRAYSIZE(L"ControlCenter.dll") - 1) == 0) {
            HookControlCenter(result, true);
        }
    }
    return status;
}

BOOL Wh_ModInit() {
    HMODULE combase = GetModuleHandleW(L"combase.dll");
    g_windowsGetStringRawBuffer = reinterpret_cast<WindowsGetStringRawBuffer_t>(
        combase ? GetProcAddress(combase, "WindowsGetStringRawBuffer") : nullptr);
    g_windowsDeleteString = reinterpret_cast<WindowsDeleteString_t>(
        combase ? GetProcAddress(combase, "WindowsDeleteString") : nullptr);
    if (!g_windowsGetStringRawBuffer || !g_windowsDeleteString) {
        Wh_Log(L"Required WinRT string helpers are unavailable.");
        return FALSE;
    }

    if (HMODULE controlCenter = GetModuleHandleW(L"ControlCenter.dll")) {
        HookControlCenter(controlCenter, false);
        return TRUE;
    }

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    void* ldrLoadDll = ntdll
                           ? reinterpret_cast<void*>(GetProcAddress(ntdll, "LdrLoadDll"))
                           : nullptr;
    if (!ldrLoadDll ||
        !Wh_SetFunctionHook(ldrLoadDll, (void*)LdrLoadDllHook,
                            (void**)&g_ldrLoadDllOriginal)) {
        Wh_Log(L"Could not hook LdrLoadDll; restart ShellHost and try again.");
        return FALSE;
    }

    Wh_Log(L"Waiting for ControlCenter.dll.");
    return TRUE;
}
