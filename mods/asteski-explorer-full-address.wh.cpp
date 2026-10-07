// ==WindhawkMod==
// @id              asteski-explorer-full-address
// @name            Explorer: Always Show Full Address
// @description     Keeps the Windows 11 Explorer address bar in its text view instead of breadcrumbs.
// @version         0.1.0
// @author          Asteski
// @github          https://github.com/Asteski
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Explorer: Always Show Full Address

Experimental mod for the modern Windows 11 File Explorer navigation bar.
Shows the native address text box even when the file list has focus. Clicking
the box still lets you type a destination. No simulated keyboard shortcuts,
timers, or focus changes are used.

Close and reopen Explorer windows after enabling or disabling the mod.
The first use can require downloading Microsoft debugging symbols.
Only the modern Windows 11 address bar is supported (not the classic ribbon).
Virtual locations such as Home or This PC have no filesystem path and keep
Explorer's native address representation. Paths wider than the box still
require horizontal scrolling.

This initial version is compiled but requires a live Explorer test, especially
for navigation, tabs, Enter, Escape, and restoring the text after focus loss.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <inspectable.h>
#include <winstring.h>
#include <atomic>
#include <cwchar>

using GoToState_t = bool(__cdecl*)(const void*, const HSTRING*, bool);
GoToState_t g_goToState;
std::atomic<bool> g_unloading{false};
std::atomic<bool> g_attempted{false};

bool __cdecl GoToState_Hook(const void* control, const HSTRING* state,
                           bool transitions) {
    // C++/WinRT projected controls hold their ABI interface pointer as their
    // first member. The param::hstring argument likewise starts with HSTRING.
    // Check the runtime class so other controls using "Normal" are untouched.
    if (!g_unloading && control && state &&
        wcscmp(WindowsGetStringRawBuffer(*state, nullptr), L"Normal") == 0) {
        auto inspectable = *static_cast<IInspectable* const*>(control);
        HSTRING className = nullptr;
        if (inspectable && SUCCEEDED(inspectable->GetRuntimeClassName(&className))) {
            const bool addressBar = wcscmp(
                WindowsGetStringRawBuffer(className, nullptr),
                L"FileExplorerExtensions.AddressBarControl") == 0;
            WindowsDeleteString(className);
            if (addressBar) {
                HSTRING edit = nullptr;
                if (SUCCEEDED(WindowsCreateString(L"Edit", 4, &edit))) {
                    // Preserve Explorer's actual editing/focus state. Only its
                    // visual state changes; navigation and focus handlers run.
                    try {
                        const bool result = g_goToState(control, &edit, false);
                        WindowsDeleteString(edit);
                        return result;
                    } catch (...) {
                        WindowsDeleteString(edit);
                        throw;
                    }
                }
            }
        }
    }
    return g_goToState(control, state, transitions);
}

bool HookAddressModule(HMODULE module, bool apply) {
    if (g_unloading || g_attempted.exchange(true)) return true;
    constexpr wchar_t symbolName[] =
        L"?GoToState@VisualStateManager@Xaml@UI@Microsoft@winrt@@SA@AEBUControl@Controls@2345@AEBUhstring@param@5@_N@Z";
    WH_FIND_SYMBOL_OPTIONS options{};
    options.optionsSize = sizeof(options);
    options.noUndecoratedSymbols = TRUE;
    WH_FIND_SYMBOL symbol{};
    HANDLE search = Wh_FindFirstSymbol(module, &options, &symbol);
    if (!search) {
        Wh_Log(L"Could not load address bar symbols");
        return false;
    }
    void* target = nullptr;
    do {
        if (symbol.symbolDecorated &&
            wcscmp(symbol.symbolDecorated, symbolName) == 0) {
            target = symbol.address;
            break;
        }
    } while (Wh_FindNextSymbol(search, &symbol));
    Wh_FindCloseSymbol(search);
    if (!target || !Wh_SetFunctionHook(target,
            reinterpret_cast<void*>(GoToState_Hook),
            reinterpret_cast<void**>(&g_goToState))) {
        Wh_Log(L"Unsupported Explorer build: address visual state hook unavailable");
        return false;
    }
    if (apply) Wh_ApplyHookOperations();
    Wh_Log(L"Address text view hook installed");
    return true;
}

decltype(&LoadLibraryExW) g_loadLibraryExW;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR name, HANDLE file, DWORD flags) {
    HMODULE module = g_loadLibraryExW(name, file, flags);
    if (module && name && !g_unloading &&
        !(flags & (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE |
                   LOAD_LIBRARY_AS_IMAGE_RESOURCE))) {
        const wchar_t* leaf = name;
        for (const wchar_t* p = name; *p; ++p)
            if (*p == L'\\' || *p == L'/') leaf = p + 1;
        if (_wcsicmp(leaf, L"FileExplorerExtensions.dll") == 0 ||
            _wcsicmp(leaf, L"FileExplorerExtensions") == 0)
            HookAddressModule(module, true);
    }
    return module;
}

BOOL Wh_ModInit() {
    HMODULE module = GetModuleHandleW(L"FileExplorerExtensions.dll");
    if (module) return HookAddressModule(module, false);
    auto kernelBase = GetModuleHandleW(L"kernelbase.dll");
    auto target = kernelBase ? GetProcAddress(kernelBase, "LoadLibraryExW") : nullptr;
    return target && Wh_SetFunctionHook(reinterpret_cast<void*>(target),
        reinterpret_cast<void*>(LoadLibraryExW_Hook),
        reinterpret_cast<void**>(&g_loadLibraryExW));
}

void Wh_ModBeforeUninit() {
    g_unloading = true;
}
