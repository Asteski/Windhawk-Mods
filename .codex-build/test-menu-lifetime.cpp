#include <windows.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
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
    winrt::weak_ref<wuxc::MenuFlyoutItemBase> item;
    winrt::event_token token;
    bool toggle;
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
        winrt::make_weak(item.template as<wuxc::MenuFlyoutItemBase>()), {},
        std::is_same_v<Item, wuxc::ToggleMenuFlyoutItem>});
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
                if (it->toggle) {
                    item.as<wuxc::ToggleMenuFlyoutItem>().Click(it->token);
                } else {
                    item.as<wuxc::MenuFlyoutItem>().Click(it->token);
                }
                item.IsEnabled(false);
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
        manager.Close();
        return 0;
    } catch(winrt::hresult_error const&e){printf("XAML test unavailable: %lx\n",(unsigned long)e.code().value);return 2;}
}

