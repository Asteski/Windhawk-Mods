// ==WindhawkMod==
// @id              asteski-system-flyout-fixes
// @name            System Flyout Fixes
// @description     Place the Snipping Tool toolbar below a top taskbar and make system OSDs click-through
// @version         0.1.0
// @author          Asteski
// @github          https://github.com/Asteski
// @include         SnippingTool.exe
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -luser32 -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# System Flyout Fixes

On a monitor with the taskbar at the top, move the Snipping Tool capture
toolbar below it, retaining the native horizontal alignment and capture
surface. The gap is in logical pixels and follows monitor scaling.
The toolbar, mode menus and capture surface remain interactive.

Make recognized hardware confirmation overlays (volume, brightness and
virtual desktop name) click-through, including the native window's invisible
margins. This applies to transient OSDs, not Quick Settings or Task View.
Volume/brightness OSD buttons also become click-through by design; use keys
or Quick Settings to change these values.

Uses Windows.UI.Xaml desktop source interface symbols, downloaded by Windhawk.
Designed for Windows 11 and Snipping Tool's Windows.UI.Xaml capture overlay.
OSD windows are identified by the hardware-confirmator DLL's own style call.
If an already-created OSD has no recognizable host title, restart Explorer
once after enabling so Windows recreates its host with the new input styles.
Unrecognized overlay implementations are left untouched. Close and reopen
an overlay after enabling. Disable the mod to restore its changes.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- toolbarGap: 24
  $name: Gap below the top taskbar
  $description: Logical pixels (scaled with the monitor), from 0 to 200.
- clickThroughOsd: true
  $name: Make system OSDs fully click-through
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <windhawk_utils.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace xaml = winrt::Windows::UI::Xaml;
namespace media = xaml::Media;
namespace wf = winrt::Windows::Foundation;
namespace hosting = xaml::Hosting;

namespace {
std::atomic<bool> g_stopping{}, g_ready{};
std::atomic_flag g_installing = ATOMIC_FLAG_INIT;
std::atomic<HMODULE> g_attemptedModule{};
std::atomic<int> g_gap{24};
std::atomic<bool> g_clickThrough{true};
thread_local bool g_inside{};
bool g_snipping{};
constexpr wchar_t kWindowTag[] = L"Asteski.SystemFlyoutFixes.0.1";
struct Guard { Guard(){g_inside=true;} ~Guard(){g_inside=false;} };

std::wstring Class(HWND h) {
    wchar_t c[160]{};
    GetClassNameW(h, c, ARRAYSIZE(c));
    return c;
}

struct SavedProperty {
    xaml::DependencyProperty property{nullptr};
    wf::IInspectable local{nullptr};
    xaml::Data::Binding binding{nullptr};
    void Save(xaml::FrameworkElement const& e, xaml::DependencyProperty const& p) {
        property=p; local=e.ReadLocalValue(p);
        if(auto expression=e.GetBindingExpression(p)) binding=expression.ParentBinding();
    }
    void Restore(xaml::FrameworkElement const& e) {
        if(!property) return;
        if(binding) e.SetBinding(property,binding);
        else if(local==xaml::DependencyProperty::UnsetValue()) e.ClearValue(property);
        else e.SetValue(property,local);
    }
};

struct SearchResult {
    xaml::FrameworkElement capture{nullptr};
       std::wstring tree;
};
void Search(xaml::DependencyObject const& node, SearchResult& r, int& budget, int depth=0) {
    if(!node || --budget<0 || depth>60) return;
    if(auto e=node.try_as<xaml::FrameworkElement>()) {
        auto name=e.Name();
        auto type=winrt::get_class_name(e);
        if(name==L"CaptureModeToggleSwitch" || name==L"SnippingModeComboBox") r.capture=e;
        if(r.tree.size()<24000) {
            r.tree.append(depth,L' ');r.tree+=type.c_str();r.tree+=L"#";r.tree+=name.c_str();r.tree+=L"\n";
        }
    }
    int count=media::VisualTreeHelper::GetChildrenCount(node);
    for(int i=0;i<count && budget>0;i++) Search(media::VisualTreeHelper::GetChild(node,i),r,budget,depth+1);
}

struct State : std::enable_shared_from_this<State> {
    winrt::weak_ref<xaml::FrameworkElement> root;
    winrt::weak_ref<xaml::FrameworkElement> toolbar;
    HWND bridge{};
    HWND native{};
    xaml::FrameworkElement::Loaded_revoker loaded;
    xaml::FrameworkElement::LayoutUpdated_revoker layout;
    xaml::FrameworkElement::Unloaded_revoker unloaded;
    SavedProperty transformProperty;
    media::TranslateTransform shift{nullptr};
    LONG_PTR addedStyles{};
    bool nativeChanged{}, recognized{}, verifiedNative{};
    std::atomic<bool> stopped{};
    int scans{};

    bool TopTaskbar(RECT& screen, int& bottom) {
        RECT sourceRect{};
        if(!GetWindowRect(bridge,&sourceRect)) return false;
        HMONITOR monitor=MonitorFromWindow(bridge,MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{sizeof(mi)};
        if(!GetMonitorInfoW(monitor,&mi)) return false;
        screen=sourceRect;
        struct Context {HMONITOR monitor; RECT monitorRect; int bottom; bool found;} c{monitor,mi.rcMonitor};
        EnumWindows([](HWND h,LPARAM p)->BOOL {
            auto& c=*reinterpret_cast<Context*>(p);
            auto cls=Class(h);
            if(cls!=L"Shell_TrayWnd" && cls!=L"Shell_SecondaryTrayWnd") return TRUE;
            if(MonitorFromWindow(h,MONITOR_DEFAULTTONEAREST)!=c.monitor) return TRUE;
            RECT r{};if(!GetWindowRect(h,&r)) return TRUE;
            int width=r.right-r.left,height=r.bottom-r.top;
            // A bottom/side taskbar must never cause a toolbar displacement.
            if(height>0 && width>height && r.top<=c.monitorRect.top+2 &&
               r.bottom<c.monitorRect.top+(c.monitorRect.bottom-c.monitorRect.top)/2) {
                c.bottom=c.monitorRect.top+height;c.found=true;
            }
            return TRUE;
        },reinterpret_cast<LPARAM>(&c));
        bottom=c.bottom;
        return c.found;
    }

    void Position() {
        auto e=toolbar.get();auto r=root.get();
        if(!e || !r || !shift) return;
        RECT screen{};int bottom{};
        if(!TopTaskbar(screen,bottom)) {if(shift.Y()!=0) shift.Y(0);return;}
        double scale=1;
        if(auto xr=r.XamlRoot()) scale=xr.RasterizationScale();
        if(scale<=0) return;
        auto point=e.TransformToVisual(r).TransformPoint({0,0});
        double target=(bottom-screen.top)/scale+g_gap.load();
        double next=shift.Y()+target-point.Y;
        // Protect against a fullscreen parent or invalid/incomplete layout.
        if(std::isfinite(next) && std::abs(next-shift.Y())>0.25) shift.Y(next);
    }

    void EnableNative() {
        if(nativeChanged || !native || !g_clickThrough || !IsWindow(native)) return;
        auto cls=Class(native);
        // Root/taskbar/input hosts can contain unrelated controls. Only a
        // dedicated popup can have all of its native input bounds disabled.
        if(cls==L"Shell_TrayWnd" || cls==L"Shell_SecondaryTrayWnd" ||
           cls==L"Progman" || cls==L"WorkerW" || cls==L"XamlWindow" ||
           cls==L"CabinetWClass") return;
        if(!verifiedNative && cls!=L"XamlExplorerHostIslandWindow" && cls!=L"Windows.UI.Core.CoreWindow" &&
           cls!=L"NativeHWNDHost") return;
        LONG_PTR ex=GetWindowLongPtrW(native,GWL_EXSTYLE);
        constexpr LONG_PTR bits=WS_EX_LAYERED|WS_EX_TRANSPARENT;
        addedStyles=bits&~ex;
        SetLastError(0);
        if(!SetWindowLongPtrW(native,GWL_EXSTYLE,ex|bits) && GetLastError()) return;
        if(!(ex&WS_EX_LAYERED) && !SetLayeredWindowAttributes(native,0,255,LWA_ALPHA)) {
            SetWindowLongPtrW(native,GWL_EXSTYLE,ex);return;
        }
        if(!SetPropW(native,kWindowTag,reinterpret_cast<HANDLE>(this))) {
            SetWindowLongPtrW(native,GWL_EXSTYLE,ex);return;
        }
        nativeChanged=true;
        Wh_Log(L"Click-through OSD: %s (%p)",cls.c_str(),native);
        Wh_SetStringValue(L"osdWindowClass",cls.c_str());
        Wh_SetIntValue(L"osdWindowsRecognized",Wh_GetIntValue(L"osdWindowsRecognized",0)+1);
    }

    void Update() {
        if(stopped || g_stopping || g_inside) return;
        Guard guard;
        try {
            auto r=root.get();if(!r) return;
            if(!recognized && ++scans<=12) {
                SearchResult result;int budget=4096;Search(r,result,budget);
                if(g_snipping && result.capture && r.ActualHeight()>0) {
                    auto candidate=result.capture;
                    // Choose the outermost compact toolbar ancestor, retaining
                    // the background/border and all controls as one unit.
                    for(auto p=media::VisualTreeHelper::GetParent(candidate);p;p=media::VisualTreeHelper::GetParent(p)) {
                        auto e=p.try_as<xaml::FrameworkElement>();if(!e) continue;
                        double h=e.ActualHeight();
                        if(h<=0 || h>160 || e==r) break;
                        candidate=e;
                    }
                    if(candidate!=result.capture && candidate.ActualHeight()>0) {
                        toolbar=winrt::make_weak(candidate);
                        transformProperty.Save(candidate,xaml::UIElement::RenderTransformProperty());
                        media::TransformGroup group;
                        if(auto original=candidate.RenderTransform()) group.Children().Append(original);
                        shift=media::TranslateTransform();group.Children().Append(shift);
                        candidate.RenderTransform(group);
                        recognized=true;
                        Wh_Log(L"Snipping toolbar: %s#%s",winrt::get_class_name(candidate).c_str(),candidate.Name().c_str());
                        Wh_SetIntValue(L"toolbarRecognized",1);
                        Wh_SetStringValue(L"lastSnippingTree",result.tree.c_str());
                    }
                }
            }
            Position();
        } catch(...) {Wh_Log(L"Overlay update: %08X",winrt::to_hresult());}
    }

    void Restore() {
        stopped=true;
        loaded.revoke();layout.revoke();unloaded.revoke();
        if(auto e=toolbar.get()) transformProperty.Restore(e);
        if(nativeChanged && IsWindow(native) && GetPropW(native,kWindowTag)==reinterpret_cast<HANDLE>(this)) {
            auto ex=GetWindowLongPtrW(native,GWL_EXSTYLE);
            SetWindowLongPtrW(native,GWL_EXSTYLE,ex&~addedStyles);
            RemovePropW(native,kWindowTag);
        }
        nativeChanged=false;
    }
};
std::mutex g_mutex;
std::vector<std::shared_ptr<State>> g_states;

void Observe(void* self,void* content) {
    if(!g_snipping || g_stopping || g_inside || !content) return;
    Guard guard;
    try {
        xaml::IUIElement ui{nullptr};winrt::copy_from_abi(ui,content);
        auto root=ui.try_as<xaml::FrameworkElement>();if(!root) return;
        hosting::IDesktopWindowXamlSource source{nullptr};winrt::copy_from_abi(source,self);
        HWND bridge{};
        winrt::check_hresult(source.as<IDesktopWindowXamlSourceNative>()->get_WindowHandle(&bridge));
        if(!bridge) return;
        {
            std::lock_guard lock(g_mutex);
            std::erase_if(g_states,[](auto const& state){return state->stopped.load();});
            for(auto const& state:g_states) if(state->root.get()==root) return;
        }
        auto state=std::make_shared<State>();
        state->root=winrt::make_weak(root);state->bridge=bridge;
        state->native=GetAncestor(bridge,GA_ROOT);
        std::weak_ptr<State> weak=state;
        state->loaded=root.Loaded(winrt::auto_revoke,[weak](auto const&,auto const&) {if(auto s=weak.lock())s->Update();});
        state->layout=root.LayoutUpdated(winrt::auto_revoke,[weak](auto const&,auto const&) {if(auto s=weak.lock())s->Update();});
        state->unloaded=root.Unloaded(winrt::auto_revoke,[weak](auto const&,auto const&) {
            if(auto s=weak.lock()) {try{s->Restore();}catch(...) {}}
        });
        {std::lock_guard lock(g_mutex);g_states.push_back(state);}
        Wh_Log(L"Observed XAML root in %s (%p)",Class(state->native).c_str(),state->native);
    } catch(...) {Wh_Log(L"Observe source: %08X",winrt::to_hresult());}
}
using Content_t=HRESULT(WINAPI*)(void*,void*);
Content_t g_putContent;
using GetContent_t=HRESULT(WINAPI*)(void*,void**);
GetContent_t g_getContent;
HRESULT WINAPI PutContent(void* self,void* content) {
    HRESULT hr=g_putContent(self,content);
    if(SUCCEEDED(hr)) Observe(self,content);
    return hr;
}
HRESULT WINAPI GetContent(void* self,void** content) {
    HRESULT hr=g_getContent(self,content);
    if(SUCCEEDED(hr) && content) Observe(self,*content);
    return hr;
}

void Install(HMODULE module) {
    if(!g_snipping || !module || g_ready || g_stopping || g_attemptedModule==module || g_installing.test_and_set()) return;
    g_attemptedModule=module;
    WH_FIND_SYMBOL symbol{};
    void* get{};void* put{};
    HANDLE find=Wh_FindFirstSymbol(module,nullptr,&symbol);
    if(find) {
        do {
            std::wstring name=symbol.symbol;
            if(name.find(L"ctl::interface_forwarder<")==std::wstring::npos ||
               name.find(L"IDesktopWindowXamlSource,")==std::wstring::npos ||
               name.find(L"DesktopWindowXamlSourceGenerated>")==std::wstring::npos) continue;
            if(name.find(L"::get_Content(")!=std::wstring::npos) get=symbol.address;
            if(name.find(L"::put_Content(")!=std::wstring::npos) put=symbol.address;
            if(get && put) break;
        } while(Wh_FindNextSymbol(find,&symbol));
        Wh_FindCloseSymbol(find);
    }
    if(get && put && WindhawkUtils::SetFunctionHook(reinterpret_cast<GetContent_t>(get),GetContent,&g_getContent) &&
       WindhawkUtils::SetFunctionHook(reinterpret_cast<Content_t>(put),PutContent,&g_putContent)) {
        Wh_ApplyHookOperations();g_ready=true;
        Wh_Log(L"Desktop XAML source hooks ready");
        Wh_SetIntValue(L"hooksReady",1);
    } else {
        if(get) Wh_RemoveFunctionHook(get);
        if(put) Wh_RemoveFunctionHook(put);
        Wh_Log(L"Required desktop XAML source symbols unavailable");
    }
    g_installing.clear();
}
using Load_t=decltype(&LoadLibraryExW);
Load_t g_load;
HMODULE WINAPI LoadHook(LPCWSTR name,HANDLE file,DWORD flags) {
    auto module=g_load(name,file,flags);
    if(module && !(flags&(LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE|LOAD_LIBRARY_AS_IMAGE_RESOURCE)))
        Install(GetModuleHandleW(L"Windows.UI.Xaml.dll"));
    return module;
}
using SetLong_t = decltype(&SetWindowLongPtrW);
SetLong_t g_setLong;
bool HardwareCaller(void* address) {
    HMODULE caller{};
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(address), &caller) &&
        caller == GetModuleHandleW(L"Windows.Internal.HardwareConfirmator.dll");
}
LONG_PTR WINAPI SetLongHook(HWND hwnd,int index,LONG_PTR value) {
    bool hardware=!g_snipping && !g_stopping && !g_inside &&
        index==GWL_EXSTYLE && HardwareCaller(__builtin_return_address(0));
    std::shared_ptr<State> existing;
    if(!g_snipping && !g_stopping && !g_inside && index==GWL_EXSTYLE) {
        std::lock_guard lock(g_mutex);
        for(auto const& state:g_states) if(state->native==hwnd && state->nativeChanged &&
            GetPropW(hwnd,kWindowTag)==reinterpret_cast<HANDLE>(state.get())) {existing=state;break;}
    }
    if(existing && g_clickThrough) value|=WS_EX_LAYERED|WS_EX_TRANSPARENT;
    LONG_PTR result=g_setLong(hwnd,index,value);
    DWORD error=GetLastError();
    if(hardware && !existing && g_clickThrough) {
        DWORD pid{};GetWindowThreadProcessId(hwnd,&pid);
        if(pid==GetCurrentProcessId()) {
            auto state=std::make_shared<State>();
            state->native=hwnd;state->bridge=hwnd;state->verifiedNative=true;
            Guard guard;
            state->EnableNative();
            if(state->nativeChanged) {std::lock_guard lock(g_mutex);g_states.push_back(state);}
        }
    }
    SetLastError(error);
    return result;
}
void AdoptExistingOsds() {
    if(g_snipping || !g_clickThrough || g_stopping) return;
    EnumWindows([](HWND hwnd,LPARAM)->BOOL {
        DWORD pid{};GetWindowThreadProcessId(hwnd,&pid);
        if(pid!=GetCurrentProcessId() || Class(hwnd)!=L"XamlExplorerHostIslandWindow") return TRUE;
        wchar_t title[80]{};GetWindowTextW(hwnd,title,ARRAYSIZE(title));
        // Exact host name from Windows.Internal.HardwareConfirmator.dll;
        // no generic Explorer or shell popup is eligible for this fallback.
        if(wcscmp(title,L"Hardware Confirmator")!=0) return TRUE;
        if(GetPropW(hwnd,kWindowTag)) return TRUE;
        auto state=std::make_shared<State>();state->native=hwnd;state->bridge=hwnd;state->verifiedNative=true;
        Guard guard;state->EnableNative();
        if(state->nativeChanged) {std::lock_guard lock(g_mutex);g_states.push_back(state);}
        return TRUE;
    },0);
}
UINT g_restoreMessage;
LRESULT CALLBACK ThreadHook(int code,WPARAM wp,LPARAM lp) {
    if(code==HC_ACTION) {
        auto msg=reinterpret_cast<CWPSTRUCT*>(lp);
        if(msg->message==g_restoreMessage && msg->lParam) {
            auto state=reinterpret_cast<State*>(msg->lParam);
            try {state->Restore();}catch(...) {Wh_Log(L"Restore: %08X",winrt::to_hresult());}
        }
    }
    return CallNextHookEx(nullptr,code,wp,lp);
}
void RestoreState(std::shared_ptr<State> const& state) {
    DWORD pid{};DWORD tid=GetWindowThreadProcessId(state->bridge,&pid);
    if(state->stopped) return;
    if(state->verifiedNative) {state->Restore();return;}
    if(!tid || pid!=GetCurrentProcessId()) return;
    if(tid==GetCurrentThreadId()) {state->Restore();return;}
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,ThreadHook,nullptr,tid);
    if(!hook) return;
    SendMessageW(state->bridge,g_restoreMessage,0,reinterpret_cast<LPARAM>(state.get()));
    UnhookWindowsHookEx(hook);
}
}

BOOL Wh_ModInit() {
    wchar_t path[MAX_PATH]{};GetModuleFileNameW(nullptr,path,ARRAYSIZE(path));
    auto base=wcsrchr(path,L'\\');base=base?base+1:path;
    g_snipping=_wcsicmp(base,L"SnippingTool.exe")==0;
    if(!g_snipping && _wcsicmp(base,L"explorer.exe")!=0) return FALSE;
    g_gap=std::clamp(Wh_GetIntSetting(L"toolbarGap"),0,200);
    g_clickThrough=Wh_GetIntSetting(L"clickThroughOsd")!=0;
    g_restoreMessage=RegisterWindowMessageW(L"Asteski.SystemFlyoutFixes.Restore.0.1");
    if(g_snipping) {Wh_SetIntValue(L"hooksReady",0);Wh_SetIntValue(L"toolbarRecognized",0);}
    if(g_snipping) {
        if(!WindhawkUtils::SetFunctionHook(LoadLibraryExW,LoadHook,&g_load)) return FALSE;
    } else {
        if(!WindhawkUtils::SetFunctionHook(SetWindowLongPtrW,SetLongHook,&g_setLong)) return FALSE;
    }
    Install(GetModuleHandleW(L"Windows.UI.Xaml.dll"));
    return TRUE;
}
void Wh_ModAfterInit() {
    Install(GetModuleHandleW(L"Windows.UI.Xaml.dll"));
    AdoptExistingOsds();
}
void Wh_ModSettingsChanged(BOOL* reload) {*reload=TRUE;}
void Wh_ModBeforeUninit() {
    g_stopping=true;
    std::vector<std::shared_ptr<State>> states;
    {std::lock_guard lock(g_mutex);states.swap(g_states);}
    for(auto const& state:states) RestoreState(state);
}







