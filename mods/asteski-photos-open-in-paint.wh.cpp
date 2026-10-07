// ==WindhawkMod==
// @id              asteski-photos-open-in-paint
// @name            Photos: Open in Paint
// @description     Open photos in Paint using Edit or the Open with menu
// @version         0.3.1
// @author          Asteski
// @github          https://github.com/Asteski
// @include         Photos.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -luuid -lshell32 -lshlwapi
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Photos: Open in Paint
Use the existing Edit button to open the current image in Paint, and/or add Paint
inside the native Open with menu. Both features have independent switches.
After Edit opens Paint, choose to keep Photos open (default) or close only that
viewer window. Open with > Paint always keeps Photos open.
The extra toolbar button and custom icon settings from earlier versions are removed.
The current file is resolved at click time. No images are saved or overwritten.
Videos and unavailable paths retain native Edit behavior and disable the Paint item.
Developed for Photos 2026.11080.24002.0, Windows 11 x64. Private Photos interfaces
may require updates when Photos changes. Disabling restores the native Edit command
and removes the Paint menu entry. Restart Photos if needed after changing settings.

XAML diagnostics scaffolding is adapted from m417z's Windows 11 Settings Styler
(GPL-3.0), based on UWPSpy VisualTreeWatcher, as in Paint Tweaks.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- editInPaint: true
  $name: Make Edit open images in Paint
- afterEdit: keep
  $name: After Edit opens Paint
  $options:
  - keep: Keep Photos open
  - close: Close this Photos viewer
- addToOpenWith: true
  $name: Add Paint to Open with
*/
// ==/WindhawkModSettings==

#undef GetCurrentTime
#include <windows.h>
#include <appmodel.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <ocidl.h>
#include <xamlom.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <atomic>
#include <string>
#include <vector>
#include <algorithm>
using namespace winrt;
namespace wf=winrt::Windows::Foundation;
namespace wux=winrt::Microsoft::UI::Xaml;
using namespace wux;
std::atomic<bool> g_stopping{false};
HANDLE g_stopEvent=nullptr,g_worker=nullptr;
std::wstring g_paintIcon,g_paintExe;
bool g_editInPaint=true,g_closeAfterEdit=false,g_addToOpenWith=true;

static Controls::IconElement PaintIcon() {
    if(!g_paintIcon.empty()) {
        Controls::BitmapIcon icon;
        icon.UriSource(wf::Uri(hstring(g_paintIcon)));
        icon.ShowAsMonochrome(false);
        return icon;
    }
    Controls::FontIcon icon;
    icon.FontFamily(Media::FontFamily(L"Segoe Fluent Icons"));
    icon.Glyph(L"\uE790");
    return icon;
}

// Interface IDs and getter slots from the installed Lightbox.winmd. Query each
// interface before calling its ABI, so unsupported Photos versions fail closed.
constexpr GUID IID_TitleBar={0x14bfcbe6,0xe086,0x5a4b,{0x95,0x48,0x5b,0xa3,0x89,0x9f,0x76,0x49}};
constexpr GUID IID_TitleVM={0xdce82107,0x9f0e,0x507e,{0xa7,0xfe,0x3c,0xee,0x2d,0x8a,0x15,0x08}};
constexpr GUID IID_Items={0x3e290619,0x891a,0x5ad7,{0x88,0x5d,0xd5,0x10,0xc1,0x07,0xee,0x3c}};
constexpr GUID IID_Item={0x9dc78563,0xf619,0x58db,{0x99,0xf6,0xa2,0x92,0x43,0x21,0xe5,0x87}};
template<class T> HRESULT ReadGetter(wf::IInspectable const& object,const GUID& iid,size_t slot,T* value) {
    if(!object) return E_POINTER;
    com_ptr<::IInspectable> iface;
    HRESULT hr=reinterpret_cast<::IUnknown*>(get_abi(object))->QueryInterface(iid,iface.put_void());
    if(FAILED(hr)) return hr;
    auto table=*reinterpret_cast<void***>(iface.get());
    return reinterpret_cast<HRESULT(STDMETHODCALLTYPE*)(::IInspectable*,T*)>(table[slot])(iface.get(),value);
}
static wf::IInspectable ObjectGetter(wf::IInspectable const& object,const GUID& iid,size_t slot) {
    wf::IInspectable result{nullptr};
    check_hresult(ReadGetter(object,iid,slot,reinterpret_cast<::IInspectable**>(put_abi(result))));
    return result;
}
static std::wstring CurrentPath(FrameworkElement const& toolbar) noexcept {
    try {
        DependencyObject parent=toolbar;
        for(int depth=0;parent && depth<24;depth++,parent=Media::VisualTreeHelper::GetParent(parent)) {
            com_ptr<::IInspectable> title;
            if(FAILED(reinterpret_cast<::IUnknown*>(get_abi(parent))->QueryInterface(IID_TitleBar,title.put_void()))) continue;
            auto vm=ObjectGetter(parent,IID_TitleBar,6);
            auto items=ObjectGetter(vm,IID_TitleVM,8);
            auto item=ObjectGetter(items,IID_Items,28);
            boolean video=true;
            check_hresult(ReadGetter(item,IID_Item,44,&video));
            if(video) return {};
            hstring path;
            check_hresult(ReadGetter(item,IID_Item,43,reinterpret_cast<HSTRING*>(put_abi(path))));
            if(path.empty()) return {};
            DWORD attributes=GetFileAttributesW(path.c_str());
            if(attributes==INVALID_FILE_ATTRIBUTES || (attributes&FILE_ATTRIBUTE_DIRECTORY)) return {};
            return std::wstring(path);
        }
    } catch(...) {}
    return {};
}
static std::wstring FileUri(std::wstring const& path) {
    std::vector<wchar_t> uri(path.size()*3+32);
    DWORD count=static_cast<DWORD>(uri.size());
    if(FAILED(UrlCreateFromPathW(path.c_str(),uri.data(),&count,0))) return {};
    return uri.data();
}
static void FindPaintIcon() {
    UINT32 count=0,length=0;
    if(GetPackagesByPackageFamily(L"Microsoft.Paint_8wekyb3d8bbwe",&count,nullptr,&length,nullptr)!=ERROR_INSUFFICIENT_BUFFER) return;
    std::vector<PWSTR> names(count);
    std::vector<wchar_t> buffer(length);
    if(GetPackagesByPackageFamily(L"Microsoft.Paint_8wekyb3d8bbwe",&count,names.data(),&length,buffer.data())!=ERROR_SUCCESS) return;
    for(auto name:names) {
        UINT32 size=0;
        if(GetPackagePathByFullName(name,&size,nullptr)!=ERROR_INSUFFICIENT_BUFFER) continue;
        std::vector<wchar_t> path(size);
        if(GetPackagePathByFullName(name,&size,path.data())!=ERROR_SUCCESS) continue;
        std::wstring executable=std::wstring(path.data())+L"\\PaintApp\\mspaint.exe";
        if(GetFileAttributesW(executable.c_str())!=INVALID_FILE_ATTRIBUTES) g_paintExe=executable;
        std::wstring icon=std::wstring(path.data())+L"\\Assets\\PaintAppList.targetsize-32_altform-unplated.png";
        if(GetFileAttributesW(icon.c_str())!=INVALID_FILE_ATTRIBUTES) {g_paintIcon=FileUri(icon);break;}
    }
}
static bool OpenInPaint(FrameworkElement const& origin,bool closeViewer) {
    auto path=CurrentPath(origin);
    if(path.empty()) return false;
    auto argument=L"\""+path+L"\"";
    SHELLEXECUTEINFOW info{sizeof(info)};
    info.fMask=SEE_MASK_FLAG_NO_UI;
    info.lpVerb=L"open"; info.lpFile=g_paintExe.c_str();
    info.lpParameters=argument.c_str(); info.nShow=SW_SHOWNORMAL;
    if(!ShellExecuteExW(&info)) {
        Wh_Log(L"Paint launch failed: %lu",GetLastError());
        return false;
    }
    if(closeViewer) try {
        auto id=origin.XamlRoot().ContentIslandEnvironment().AppWindowId();
        origin.DispatcherQueue().TryEnqueue([id] {
            if(g_stopping) return;
            try { winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(id).Destroy(); }
            catch(...) {Wh_Log(L"Couldn't close Photos viewer: %08X",to_hresult());}
        });
    } catch(...) {Wh_Log(L"Couldn't identify Photos viewer: %08X",to_hresult());}
    return true;
}
struct PaintEditCommand : implements<PaintEditCommand,Input::ICommand> {
    weak_ref<FrameworkElement> origin;
    Input::ICommand original{nullptr};
    event<wf::EventHandler<wf::IInspectable>> changed;
    PaintEditCommand(FrameworkElement const& element,Input::ICommand const& command):origin(make_weak(element)),original(command) {}
    bool CanExecute(wf::IInspectable const& parameter) {
        if(auto e=origin.get(); e && !CurrentPath(e).empty()) return true;
        return original && original.CanExecute(parameter);
    }
    void Execute(wf::IInspectable const& parameter) {
        if(g_stopping) return;
        if(auto e=origin.get(); e && !CurrentPath(e).empty()) {OpenInPaint(e,g_closeAfterEdit);return;}
        if(original && original.CanExecute(parameter)) original.Execute(parameter);
    }
    event_token CanExecuteChanged(wf::EventHandler<wf::IInspectable> const& handler) {return changed.add(handler);}
    void CanExecuteChanged(event_token const& token) noexcept {changed.remove(token);}
};
struct EditControl {
    weak_ref<Controls::Button> button;
    Input::ICommand native{nullptr},replacement{nullptr};
    wf::IInspectable originalLocal{nullptr};
};
struct OpenControl {
    weak_ref<Controls::Button> button;
    weak_ref<Controls::Panel> parent;
    Controls::Button item{nullptr};
    event_token click{};
};
thread_local std::vector<EditControl> t_edits;
thread_local std::vector<OpenControl> t_open;
thread_local UINT_PTR t_timer=0;
static FrameworkElement FindNamed(DependencyObject const& root,const wchar_t* name,int depth=0) {
    if(!root || depth>20) return nullptr;
    if(auto e=root.try_as<FrameworkElement>();e &&
       (e.Name()==name || Automation::AutomationProperties::GetAutomationId(e)==name)) return e;
    for(int i=0,n=Media::VisualTreeHelper::GetChildrenCount(root);i<n;i++)
        if(auto e=FindNamed(Media::VisualTreeHelper::GetChild(root,i),name,depth+1)) return e;
    return nullptr;
}
static void EnsureMenu(OpenControl& entry,Controls::Button const& origin) {
    if(entry.item) {entry.item.IsEnabled(!CurrentPath(origin).empty());return;}
    auto base=origin.Flyout();
    if(!base) base=Controls::Primitives::FlyoutBase::GetAttachedFlyout(origin);
    auto flyout=base.try_as<Controls::Flyout>();
    if(!flyout) return;
    auto first=FindNamed(flyout.Content(),L"OpenIn_FileExplorerButton");
    if(!first) return;
    auto panel=Media::VisualTreeHelper::GetParent(first).try_as<Controls::Panel>();
    if(!panel) return;
    uint32_t index;
    if(!panel.Children().IndexOf(first,index)) return;
    Controls::Button item;
    item.Name(L"WindhawkOpenWithPaint");
    item.HorizontalAlignment(HorizontalAlignment::Stretch);
    item.HorizontalContentAlignment(HorizontalAlignment::Stretch);
    item.BorderThickness(Thickness{0});
    item.Background(Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
    item.Padding(Thickness{12,8,12,8});
    Controls::StackPanel row;
    row.Orientation(Controls::Orientation::Horizontal); row.Spacing(12);
    auto icon=PaintIcon(); icon.Width(20); icon.Height(20);
    icon.VerticalAlignment(VerticalAlignment::Center);
    row.Children().Append(icon);
    Controls::StackPanel labels;
    Controls::TextBlock title;title.Text(L"Paint");title.FontSize(14);
    Controls::TextBlock description;description.Text(L"Edit image in Paint");description.FontSize(12);description.Opacity(0.75);
    labels.Children().Append(title); labels.Children().Append(description);
    row.Children().Append(labels); item.Content(row);
    Automation::AutomationProperties::SetName(item,L"Paint");
    auto weak=entry.button;
    entry.click=item.Click([weak](auto&&,auto&&) {
        if(g_stopping) return;
        if(auto button=weak.get()) {if(auto flyout=button.Flyout()) flyout.Hide();OpenInPaint(button,false);}
    });
    entry.item=item;entry.parent=make_weak(panel);
    panel.Children().InsertAt(index,item);
    item.IsEnabled(!CurrentPath(origin).empty());
}
void CALLBACK UiTick(HWND,UINT,UINT_PTR,DWORD) {
    if(g_stopping) return;
    for(auto& entry:t_edits) try {
        auto button=entry.button.get();if(!button || !button.IsLoaded()) continue;
        if(!entry.replacement) {
            entry.native=button.Command();
            entry.originalLocal=button.ReadLocalValue(Controls::Primitives::ButtonBase::CommandProperty());
            entry.replacement=make<PaintEditCommand>(button,entry.native);
            button.Command(entry.replacement);
        }
    } catch(...) {Wh_Log(L"Edit override: %08X",to_hresult());}
    for(auto& entry:t_open) try {
        if(auto button=entry.button.get();button && button.IsLoaded()) EnsureMenu(entry,button);
    } catch(...) {Wh_Log(L"Open with entry: %08X",to_hresult());}
}
static void Track(FrameworkElement const& element,const wchar_t*) {
    auto button=element.try_as<Controls::Button>();if(!button) return;
    if(g_editInPaint && element.Name()==L"EditButton") {
        if(std::none_of(t_edits.begin(),t_edits.end(),[&](auto& e){return e.button.get()==button;}))
            t_edits.push_back({make_weak(button)});
    } else if(g_addToOpenWith && (element.Name()==L"OpenInButton" || element.Name()==L"OpenInDropDownButton")) {
        if(std::none_of(t_open.begin(),t_open.end(),[&](auto& e){return e.button.get()==button;}))
            t_open.push_back({make_weak(button)});
    } else return;
    if(!t_timer) t_timer=SetTimer(nullptr,0,200,UiTick);
}
void WINAPI CleanupUi(void*) {
    if(t_timer) {KillTimer(nullptr,t_timer);t_timer=0;}
    for(auto& entry:t_edits) try {
        if(auto button=entry.button.get();button && entry.replacement && button.Command()==entry.replacement) {
            auto property=Controls::Primitives::ButtonBase::CommandProperty();
            if(entry.originalLocal==DependencyProperty::UnsetValue()) button.ClearValue(property);
            else button.SetValue(property,entry.originalLocal);
        }
    } catch(...) {}
    for(auto& entry:t_open) try {
        if(entry.item) entry.item.Click(entry.click);
        if(auto panel=entry.parent.get();panel && entry.item) {
            uint32_t index;if(panel.Children().IndexOf(entry.item,index)) panel.Children().RemoveAt(index);
        }
    } catch(...) {}
    t_edits.clear();t_open.clear();
}

HMODULE GetCurrentModuleHandle() {
    HMODULE module=nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandle),&module);
    return module;
}
class VisualTreeWatcher : public winrt::implements<VisualTreeWatcher,IVisualTreeServiceCallback2,winrt::non_agile> {
public:
    com_ptr<IXamlDiagnostics> diagnostics;
    HANDLE worker=nullptr;
    VisualTreeWatcher(com_ptr<IUnknown> site):diagnostics(site.as<IXamlDiagnostics>()) {
        worker=CreateThread(nullptr,0,[](void* p)->DWORD {
            auto self=static_cast<VisualTreeWatcher*>(p);
            auto hr=self->diagnostics.as<IVisualTreeService3>()->AdviseVisualTreeChange(self);
            Wh_Log(L"XAML subscription: %08X",hr);
            return 0;
        },this,0,nullptr);
    }
    void UnadviseVisualTreeChange() {
        if(worker) { WaitForSingleObject(worker,INFINITE); CloseHandle(worker); worker=nullptr; }
        diagnostics.as<IVisualTreeService3>()->UnadviseVisualTreeChange(this);
    }
    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation,VisualElement element,VisualMutationType kind) override {
        if(g_stopping || kind!=Add) return S_OK;
        try {
            wf::IInspectable object{nullptr};
            check_hresult(diagnostics->GetIInspectableFromHandle(element.Handle,reinterpret_cast<::IInspectable**>(put_abi(object))));
            if(auto e=object.try_as<FrameworkElement>()) Track(e,element.Type?element.Type:L"");
        } catch(...) { Wh_Log(L"XAML tracking error: %08X",to_hresult()); }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle,VisualElementState,LPCWSTR) noexcept override { return S_OK; }
};
winrt::com_ptr<VisualTreeWatcher> g_visualTreeWatcher;

// {7152D421-5463-40E8-A432-F5916B6427E5}
static constexpr CLSID CLSID_WindhawkTAP = { 0xf1b89c20, 0x96a3, 0x4f56, { 0xb6, 0x10, 0x53, 0xc7, 0x0a, 0x95, 0x2d, 0x13 } };

class WindhawkTAP : public winrt::implements<WindhawkTAP, IObjectWithSite, winrt::non_agile>
{
public:
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown *pUnkSite) override;
    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void **ppvSite) noexcept override;

private:
    winrt::com_ptr<IUnknown> site;
};

#pragma endregion  // tap_hpp

#pragma region tap_cpp

HRESULT WindhawkTAP::SetSite(IUnknown *pUnkSite) try
{
    // Only ever 1 VTW at once.
    if (g_visualTreeWatcher)
    {
        g_visualTreeWatcher->UnadviseVisualTreeChange();
        g_visualTreeWatcher = nullptr;
    }

    site.copy_from(pUnkSite);

    if (site)
    {
        // Decrease refcount increased by InitializeXamlDiagnosticsEx.
        FreeLibrary(GetCurrentModuleHandle());

        g_visualTreeWatcher = winrt::make_self<VisualTreeWatcher>(site);
    }

    return S_OK;
}
catch (...)
{
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"Error %08X", hr);
    return hr;
}

HRESULT WindhawkTAP::GetSite(REFIID riid, void **ppvSite) noexcept
{
    return site.as(riid, ppvSite);
}

#pragma endregion  // tap_cpp

#pragma region simplefactory_hpp

#include <Unknwn.h>

template<class T>
struct SimpleFactory : winrt::implements<SimpleFactory<T>, IClassFactory, winrt::non_agile>
{
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override try
    {
        if (!pUnkOuter)
        {
            *ppvObject = nullptr;
            return winrt::make<T>().as(riid, ppvObject);
        }
        else
        {
            return CLASS_E_NOAGGREGATION;
        }
    }
    catch (...)
    {
        HRESULT hr = winrt::to_hresult();
        Wh_Log(L"Error %08X", hr);
        return hr;
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};

#pragma endregion  // simplefactory_hpp

#pragma region module_cpp

#include <combaseapi.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"

__declspec(dllexport)
_Use_decl_annotations_ STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) try
{
    if (rclsid == CLSID_WindhawkTAP)
    {
        *ppv = nullptr;
        return winrt::make<SimpleFactory<WindhawkTAP>>().as(riid, ppv);
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
}
catch (...)
{
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"Error %08X", hr);
    return hr;
}

__declspec(dllexport)
_Use_decl_annotations_ STDAPI DllCanUnloadNow()
{
    if (winrt::get_module_lock())
    {
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

#pragma clang diagnostic pop

#pragma endregion  // module_cpp

#pragma region api_cpp

using PFN_INITIALIZE_XAML_DIAGNOSTICS_EX = decltype(&InitializeXamlDiagnosticsEx);

HRESULT InjectWindhawkTAP() noexcept
{
    HMODULE module = GetCurrentModuleHandle();
    if (!module)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    WCHAR location[MAX_PATH];
    switch (GetModuleFileName(module, location, ARRAYSIZE(location)))
    {
    case 0:
    case ARRAYSIZE(location):
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const HMODULE wux(GetModuleHandle(L"Microsoft.Internal.FrameworkUdk.dll"));
    if (!wux) [[unlikely]]
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const auto ixde = reinterpret_cast<PFN_INITIALIZE_XAML_DIAGNOSTICS_EX>(GetProcAddress(wux, "InitializeXamlDiagnosticsEx"));
    if (!ixde) [[unlikely]]
    {
        auto error=HRESULT_FROM_WIN32(GetLastError());
        
        return error;
    }

    // I didn't find a better way than trying many connections until one works.
    // Reference:
    // https://github.com/microsoft/microsoft-ui-xaml/blob/d74a0332cf0d5e58f12eddce1070fa7a79b4c2db/src/dxaml/xcp/dxaml/lib/DXamlCore.cpp#L2782
    HRESULT hr;
    for (int i = 0; i < 10000; i++)
    {
        if(g_stopping) {  return E_ABORT; }
        WCHAR connectionName[256];
        wsprintf(connectionName, L"WinUIVisualDiagConnection%d", i + 1);

        hr = ixde(connectionName, GetCurrentProcessId(), L"", location, CLSID_WindhawkTAP, nullptr);
        if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
            break;
        }
    }

    
    return hr;
}


using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID parameter);

bool RunFromWindowThread(HWND hWnd,
                         RunFromWindowThreadProc_t proc,
                         PVOID procParam) {
    static const UINT runFromWindowThreadRegisteredMsg =
        RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

    struct RUN_FROM_WINDOW_THREAD_PARAM {
        RunFromWindowThreadProc_t proc;
        PVOID procParam;
    };

    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (dwThreadId == 0) {
        return false;
    }

    if (dwThreadId == GetCurrentThreadId()) {
        proc(procParam);
        return true;
    }

    HHOOK hook = SetWindowsHookEx(
        WH_CALLWNDPROC,
        [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (nCode == HC_ACTION) {
                const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
                if (cwp->message == runFromWindowThreadRegisteredMsg) {
                    RUN_FROM_WINDOW_THREAD_PARAM* param =
                        (RUN_FROM_WINDOW_THREAD_PARAM*)cwp->lParam;
                    param->proc(param->procParam);
                }
            }

            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr, dwThreadId);
    if (!hook) {
        return false;
    }

    RUN_FROM_WINDOW_THREAD_PARAM param;
    param.proc = proc;
    param.procParam = procParam;
    SendMessage(hWnd, runFromWindowThreadRegisteredMsg, 0, (LPARAM)&param);

    UnhookWindowsHookEx(hook);

    return true;
}



BOOL Wh_ModInit() {
    auto read=[](PCWSTR key) {
        auto value=Wh_GetStringSetting(key);
        std::wstring result=value; Wh_FreeStringSetting(value); return result;
    };
    g_editInPaint=Wh_GetIntSetting(L"editInPaint");
    g_closeAfterEdit=read(L"afterEdit")==L"close";
    g_addToOpenWith=Wh_GetIntSetting(L"addToOpenWith");
    wchar_t system[MAX_PATH];
    if(!GetSystemDirectoryW(system,ARRAYSIZE(system))) return FALSE;
    g_paintExe=std::wstring(system)+L"\\mspaint.exe";
    if(GetFileAttributesW(g_paintExe.c_str())==INVALID_FILE_ATTRIBUTES) g_paintExe=L"mspaint.exe";
    FindPaintIcon();
    g_stopEvent=CreateEvent(nullptr,TRUE,FALSE,nullptr);
    return g_stopEvent!=nullptr;
}
void Wh_ModAfterInit() {
    g_worker=CreateThread(nullptr,0,[](void*)->DWORD {
        for(int i=0;i<120 && WaitForSingleObject(g_stopEvent,500)==WAIT_TIMEOUT;i++) {
            if(!GetModuleHandle(L"Microsoft.Internal.FrameworkUdk.dll")) continue;
            HRESULT hr=InjectWindhawkTAP();
            Wh_Log(L"Photos XAML connect: %08X",hr);
            if(SUCCEEDED(hr)) break;
        }
        return 0;
    },nullptr,0,nullptr);
}
void Wh_ModUninit() {
    g_stopping=true;
    SetEvent(g_stopEvent);
    if(g_worker) {WaitForSingleObject(g_worker,INFINITE);CloseHandle(g_worker);}
    if(g_visualTreeWatcher) {g_visualTreeWatcher->UnadviseVisualTreeChange();g_visualTreeWatcher=nullptr;}
    EnumWindows([](HWND window,LPARAM)->BOOL {
        DWORD pid;GetWindowThreadProcessId(window,&pid);
        if(pid==GetCurrentProcessId()) RunFromWindowThread(window,CleanupUi,nullptr);
        return TRUE;
    },0);
    CloseHandle(g_stopEvent);
}
BOOL Wh_ModSettingsChanged(BOOL* reload) { *reload=TRUE; return TRUE; }
