// ==WindhawkMod==
// @id              explorer-context-menu-animation
// @name            Explorer Context Menu Animation
// @description     Enable the native WinUI popup animation for modern Explorer and desktop context menus
// @version         0.1
// @author          asteski
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Explorer Context Menu Animation

Enables WinUI's native popup theme transition when the modern Windows 11
context menu opens in File Explorer or on the desktop. The menu, its commands,
positioning, and acrylic remain managed by Windows.

Windows uses different controls for its context menus. In particular,
CommandBarFlyout disables the standard FlyoutBase open/close animation and
normally relies on its own template animations. This mod explicitly enables
the standard flyout animation on the menu passed to Explorer's menu host.
The exact motion and timing are supplied by your installed WinUI version;
they aren't guaranteed to match every taskbar or notification menu exactly.

## Requirements and use

- Windows 11, x64, with modern Explorer context menus enabled.
- Keep **Settings > Accessibility > Visual effects > Animation effects** on.
- Create a new mod in Windhawk, replace its source with this file, and compile.
- Public symbols for Windows.UI.FileExplorer.dll must be available. Windhawk
  resolves the hook by name, without hardcoded addresses or object offsets.

This is an experimental first version. The hook signature was checked against
Windows build 26100.9278 and the source was compiled with Windhawk's compiler;
the visual result still needs testing in a live Explorer session.

The setting stays on the native flyout object so it also covers deferred
opening and layout. No mod callbacks are left attached to XAML objects.
After disabling the mod, restart Explorer to reset any cached flyout objects
and transitions. The mod does not restart Explorer automatically.

Classic menus, Show more options, and other processes are outside its scope.

Implementation references:
- https://github.com/microsoft/microsoft-ui-xaml/blob/main/controls/dev/CommandBarFlyout/CommandBarFlyout.cpp
- https://github.com/microsoft/microsoft-ui-xaml/blob/main/dxaml/xcp/dxaml/lib/FlyoutBase_partial.cpp
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <windhawk_utils.h>

#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

namespace muxcp = winrt::Microsoft::UI::Xaml::Controls::Primitives;

HMODULE g_menuModule;

// The native function takes a nontrivial C++/WinRT FlyoutBase by value.
// MSVC x64 passes that object indirectly. Preserve the caller's object and
// let the original function perform its normal ownership/destruction work;
// a projected FlyoutBase by-value hook would use the wrong compiler ABI.
using ShowXamlFlyoutNow_t = void(__cdecl*)(void* host,
                                         void* flyoutStorage,
                                         const POINT* position,
                                         unsigned int flags);
ShowXamlFlyoutNow_t g_showXamlFlyoutNow;

void __cdecl ShowXamlFlyoutNow_Hook(void* host,
                                  void* flyoutStorage,
                                  const POINT* position,
                                  unsigned int flags) {
    try {
        if (flyoutStorage && *static_cast<void**>(flyoutStorage)) {
            // Take our own reference, rather than adopting the caller's.
            muxcp::FlyoutBase flyout{nullptr};
            winrt::copy_from_abi(flyout, *static_cast<void**>(flyoutStorage));
            const bool wasEnabled = flyout.AreOpenCloseAnimationsEnabled();
            if (!wasEnabled) {
                flyout.AreOpenCloseAnimationsEnabled(true);
            }
            Wh_Log(L"Explorer flyout: native animations %s",
                   wasEnabled ? L"already enabled" : L"enabled by mod");
        }
    } catch (const winrt::hresult_error& e) {
        Wh_Log(L"Unable to enable flyout animation: 0x%08X",
               static_cast<unsigned int>(e.code().value));
    } catch (...) {
        Wh_Log(L"Unable to enable flyout animation");
    }

    // Never swallow an exception from Explorer or call its show method twice.
    g_showXamlFlyoutNow(host, flyoutStorage, position, flags);
}

BOOL Wh_ModInit() {
    g_menuModule = LoadLibraryExW(L"Windows.UI.FileExplorer.dll", nullptr,
                                 LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!g_menuModule) {
        Wh_Log(L"Unable to load the modern Explorer menu component: %lu",
               GetLastError());
        return FALSE;
    }

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{L"private: void __cdecl ContextMenuHost::ShowXamlFlyoutNow(struct winrt::Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase,struct tagPOINT const &,enum CONTEXT_MENU_PRESENTER_FLAGS)"},
         &g_showXamlFlyoutNow, ShowXamlFlyoutNow_Hook},
    };
    if (!WindhawkUtils::HookSymbols(g_menuModule, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"Unsupported Explorer build: context-menu host symbol unavailable");
        FreeLibrary(g_menuModule);
        g_menuModule = nullptr;
        return FALSE;
    }

    return TRUE;
}

void Wh_ModUninit() {
    if (g_menuModule) {
        FreeLibrary(g_menuModule);
        g_menuModule = nullptr;
    }
}
