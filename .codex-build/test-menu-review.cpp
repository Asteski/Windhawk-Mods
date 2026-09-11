#include <windows.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <vector>
#include <memory>
#include <cassert>
#include <cstdio>
#include <thread>
#define Wh_Log(...) ((void)0)
namespace wf=winrt::Windows::Foundation;
namespace wux=winrt::Windows::UI::Xaml;
namespace wuxc=winrt::Windows::UI::Xaml::Controls;
std::atomic<bool> g_unloading=false;
struct ClickRegistration {
    DWORD threadId;
    winrt::weak_ref<wuxc::MenuFlyoutItem> item;
    winrt::event_token token;
};
std::mutex g_clickMutex;
std::vector<ClickRegistration> g_clickRegistrations;
std::atomic<unsigned> g_activeClicks = 0;

struct ActiveClick {
    ActiveClick() { ++g_activeClicks; }
    ~ActiveClick() { --g_activeClicks; }
};

template <typename Item, typename Handler>
void TrackClick(const Item& item, Handler handler) {
    std::lock_guard lock(g_clickMutex);
    if (g_unloading) {
        return;
    }
    DWORD threadId = GetCurrentThreadId();
    // Resolve XAML weak references only on their owning UI thread.
    std::erase_if(g_clickRegistrations, [threadId](const auto& registration) {
        return registration.threadId == threadId && !registration.item.get();
    });
    // Allocate tracking storage before attaching anything to a XAML object.
    g_clickRegistrations.push_back({threadId,
        winrt::make_weak(item.template as<wuxc::MenuFlyoutItem>()), {}});
    try {
        g_clickRegistrations.back().token = item.Click(
            [handler](wf::IInspectable const& sender, wux::RoutedEventArgs const& args) {
                ActiveClick active;
                if (!g_unloading) {
                    try {
                        handler(sender, args);
                    } catch (...) {
                        Wh_Log(L"Taskbar menu action failed");
                    }
                }
            });
    } catch (...) {
        g_clickRegistrations.pop_back();
        throw;
    }
}

void RevokeClicksOnCurrentThread() {
    std::lock_guard lock(g_clickMutex);
    DWORD threadId = GetCurrentThreadId();
    for (auto it = g_clickRegistrations.begin(); it != g_clickRegistrations.end();) {
        if (it->threadId != threadId) {
            ++it;
            continue;
        }
        try {
            if (auto item = it->item.get()) {
                // ToggleMenuFlyoutItem also implements IMenuFlyoutItem.
                // Check the ABI HRESULT: the projected Click(token) removal
                // discards errors, which would hide a failed revocation.
                auto clickInterface = item.as<wuxc::IMenuFlyoutItem>();
                auto abi = static_cast<winrt::impl::abi_t<wuxc::IMenuFlyoutItem>*>(
                    winrt::get_abi(clickInterface));
                winrt::check_hresult(abi->remove_Click(it->token));
                try {
                    item.IsEnabled(false);
                } catch (...) {
                    // Cosmetic: the callback has already been revoked.
                }
            }
            it = g_clickRegistrations.erase(it);
        } catch (...) {
            // Keep failed revocations tracked so the unload fallback can
            // retain the module instead of leaving a dangling delegate.
            ++it;
        }
    }
}

UINT g_revokeMessage;
LRESULT CALLBACK RevokeClicksHook(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && g_unloading &&
        reinterpret_cast<CWPSTRUCT*>(lp)->message == g_revokeMessage) {
        RevokeClicksOnCurrentThread();
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

void RevokeAllClicks() {
    std::vector<DWORD> threads;
    {
        std::lock_guard lock(g_clickMutex);
        for (const auto& registration : g_clickRegistrations) {
            if (std::find(threads.begin(), threads.end(), registration.threadId) == threads.end()) {
                threads.push_back(registration.threadId);
            }
        }
    }
    for (DWORD threadId : threads) {
        if (threadId == GetCurrentThreadId()) {
            RevokeClicksOnCurrentThread();
            continue;
        }
        HWND window = nullptr;
        EnumThreadWindows(threadId, [](HWND hwnd, LPARAM data) -> BOOL {
            *reinterpret_cast<HWND*>(data) = hwnd;
            return FALSE;
        }, reinterpret_cast<LPARAM>(&window));
        if (!window) {
            continue;
        }
        HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, RevokeClicksHook, nullptr, threadId);
        if (hook) {
            // Synchronous: the hook callback must finish before it is unhooked
            // and before Windhawk is allowed to unload this module.
            SendMessageW(window, g_revokeMessage, 0, 0);
            UnhookWindowsHookEx(hook);
        }
    }
    while (g_activeClicks.load() != 0) {
        Sleep(1);
    }
    std::lock_guard lock(g_clickMutex);
    if (!g_clickRegistrations.empty()) {
        // A vanished UI thread or failed revocation must never turn a stale
        // XAML delegate into a call to unmapped code. Retain code only on error.
        HMODULE module;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_PIN,
                              reinterpret_cast<LPCWSTR>(&RevokeAllClicks), &module)) {
            Wh_Log(L"Callback cleanup incomplete; module retained until Explorer exits");
        }
    }
}

struct FakeSetting { int* get() const { return nullptr; } };
struct TaskbarOption {const wchar_t* text; bool search;};
constexpr TaskbarOption kLocationOption{L"Move taskbar",false};
constexpr TaskbarOption kItemOptions[]={{L"Search",true},{L"Task View",false},{L"Widgets",false}};
constexpr wchar_t kItemName[]=L"WindhawkMoveTaskbarItem";
constexpr wchar_t kMoveTaskbarText[]=L"Move taskbar";
constexpr wchar_t kSeparatorName[]=L"WindhawkMoveTaskbarSeparator";
bool g_currentMenuInjected=false;
FakeSetting OpenNativeSetting(const TaskbarOption&){throw winrt::hresult_error(REGDB_E_CLASSNOTREG);}
bool CanChangeOption(int*){return false;}
bool GetCurrentTaskbarEdge(DWORD*){return false;}
DWORD ReadOption(int*,const TaskbarOption&){return 0;}
bool SetOption(const TaskbarOption&,DWORD,int*){return false;}
void MoveTaskbar(DWORD){}
void Wh_SetIntValue(const wchar_t*,int){}
int Wh_GetIntValue(const wchar_t*,int value){return value;}
void MenuFlyoutItemBaseVector_Append_Original(void* p,const wuxc::MenuFlyoutItemBase& item){
    static_cast<wf::Collections::IVector<wuxc::MenuFlyoutItemBase>*>(p)->Append(item);
}
void AppendInjectedItems(void* vectorThis) {
    // Claim this build before adding our block; do not inject twice.
    g_currentMenuInjected = true;
    bool addedPositionMenu = false;
    try {
        auto location = OpenNativeSetting(kLocationOption);
        DWORD currentEdge;
        if (CanChangeOption(location.get()) && GetCurrentTaskbarEdge(&currentEdge)) {
            wuxc::MenuFlyoutSubItem item;
            item.Name(kItemName);
            item.Text(kMoveTaskbarText);
            struct Position {
                const wchar_t* text;
                DWORD edge;
                const wchar_t* glyph;
            };
            constexpr Position positions[] = {
                {L"Top", ABE_TOP, L"\xE74A"},
                {L"Bottom", ABE_BOTTOM, L"\xE74B"},
                {L"Left", ABE_LEFT, L"\xE72B"},
                {L"Right", ABE_RIGHT, L"\xE72A"},
            };
            for (const auto& position : positions) {
                if (position.edge == currentEdge) {
                    continue;
                }
                wuxc::MenuFlyoutItem child;
                child.Text(position.text);
                wuxc::FontIcon icon;
                icon.FontFamily(wux::Media::FontFamily(L"Segoe Fluent Icons"));
                icon.Glyph(position.glyph);
                icon.FontSize(16);
                child.Icon(icon);
                TrackClick(child, [edge = position.edge](wf::IInspectable const&,
                                                        wux::RoutedEventArgs const&) {
                    MoveTaskbar(edge);
                });
                item.Items().Append(child);
            }
            MenuFlyoutItemBaseVector_Append_Original(vectorThis, item);
            addedPositionMenu = true;
        }
    } catch (...) {
        Wh_Log(L"Move taskbar unavailable; keeping Taskbar items");
    }

    if (addedPositionMenu) {
        wuxc::MenuFlyoutSeparator separator;
        separator.Name(L"WindhawkTaskbarItemsSeparator");
        MenuFlyoutItemBaseVector_Append_Original(vectorThis, separator);
    }

    wuxc::MenuFlyoutSubItem itemsMenu;
    itemsMenu.Name(L"WindhawkTaskbarItemsMenu");
    itemsMenu.Text(L"Taskbar items");
    for (const auto& option : kItemOptions) {
        wuxc::ToggleMenuFlyoutItem toggle;
        toggle.Text(option.text);
        toggle.IsEnabled(false);
        try {
            // One activation per option, shared by value and availability reads.
            // GetValue also supplies Windows' effective default when no registry
            // value exists. All access stays on this item's owning UI thread.
            auto setting = OpenNativeSetting(option);
            DWORD value = ReadOption(setting.get(), option);
            toggle.IsChecked(value != 0);
            toggle.IsEnabled(CanChangeOption(setting.get()));
            TrackClick(toggle, [option, setting](wf::IInspectable const& sender,
                                                wux::RoutedEventArgs const&) {
                auto toggle = sender.as<wuxc::ToggleMenuFlyoutItem>();
                DWORD current = ReadOption(setting.get(), option);
                DWORD next = toggle.IsChecked() ? 1 : 0;
                if (option.search) {
                    if (current >= 1 && current <= 3) {
                        Wh_SetIntValue(L"LastVisibleSearchSelection", current);
                    }
                    if (next != 0) {
                        auto previous = Wh_GetIntValue(L"LastVisibleSearchSelection", 1);
                        next = previous >= 1 && previous <= 3 ? previous : 1;
                    }
                }
                bool changed = SetOption(option, next, setting.get());
                if (!changed && option.search && next > 1) {
                    // Available search styles can change with Windows/build or
                    // layout. Fall back to the first visible choice if needed.
                    changed = SetOption(option, 1, setting.get());
                }
                if (!changed) {
                    toggle.IsChecked(current != 0);
                }
            });
        } catch (...) {
            Wh_Log(L"%s setting unavailable", option.text);
        }
        itemsMenu.Items().Append(toggle);
    }
    MenuFlyoutItemBaseVector_Append_Original(vectorThis, itemsMenu);

    wuxc::MenuFlyoutSeparator separator;
    separator.Name(kSeparatorName);
    MenuFlyoutItemBaseVector_Append_Original(vectorThis, separator);
    Wh_Log(L"Injected taskbar menu controls");
}

int main(){
    setvbuf(stdout,nullptr,_IONBF,0);
    try {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        auto manager=wux::Hosting::WindowsXamlManager::InitializeForCurrentThread();
        g_revokeMessage=RegisterWindowMessageW(L"TaskbarMove.Test.Revoke");
        wuxc::MenuFlyoutItem item;
        wuxc::ToggleMenuFlyoutItem toggle;
        auto captured=std::make_shared<int>(42);
        std::weak_ptr<int> lifetime=captured;
        TrackClick(item,[captured](auto const&,auto const&){});
        TrackClick(toggle,[captured](auto const&,auto const&){});
        captured.reset();
        assert(!lifetime.expired());
        assert(g_clickRegistrations.size()==2);
        g_unloading=true;
        RevokeAllClicks();
        assert(g_clickRegistrations.empty());
        assert(lifetime.expired());
        assert(!item.IsEnabled() && !toggle.IsEnabled());
        puts("PASS: both XAML delegate types revoked while items remain alive");
        TrackClick(item,[](auto const&,auto const&){});
        assert(g_clickRegistrations.empty());
        puts("PASS: no callbacks attached during unload");
        g_unloading=false;
        { wuxc::MenuFlyoutItem expired; TrackClick(expired,[](auto const&,auto const&){}); }
        TrackClick(item,[](auto const&,auto const&){});
        assert(g_clickRegistrations.size()==1);
        puts("PASS: expired item registrations pruned");
        g_unloading=true;
        RevokeAllClicks();
        g_unloading=false;
        auto held=std::make_shared<int>(7);
        std::weak_ptr<int> crossThreadLifetime=held;
        TrackClick(item,[held](auto const&,auto const&){});
        held.reset();
        HWND marshalWindow=CreateWindowW(L"STATIC",L"Lifetime test",0,0,0,1,1,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        assert(marshalWindow);
        DWORD uiThread=GetCurrentThreadId();
        std::thread cleanup([uiThread]{
            g_unloading=true;
            RevokeAllClicks();
            PostThreadMessageW(uiThread,WM_APP+1,0,0);
        });
        MSG message;
        while(GetMessageW(&message,nullptr,0,0)>0){
            if(message.message==WM_APP+1)break;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        cleanup.join();
        assert(crossThreadLifetime.expired());
        assert(g_clickRegistrations.empty());
        DestroyWindow(marshalWindow);
        puts("PASS: background unload marshals revocation to owning UI thread");
        g_unloading=false;
        wuxc::MenuFlyout menu;
        auto items=menu.Items();
        AppendInjectedItems(&items);
        assert(items.Size()==2);
        auto submenu=items.GetAt(0).as<wuxc::MenuFlyoutSubItem>();
        assert(submenu.Text()==L"Taskbar items");
        assert(submenu.Items().Size()==3);
        assert(items.GetAt(1).try_as<wuxc::MenuFlyoutSeparator>());
        puts("PASS: missing position handler keeps all three item toggles and no leading separator");
        manager.Close();
        return 0;
    } catch(winrt::hresult_error const&e){printf("XAML test unavailable: %lx\n",(unsigned long)e.code().value);return 2;}
}



