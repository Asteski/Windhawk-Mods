// ==WindhawkMod==
// @id              taskbar-options-probe
// @name            Temporary taskbar options probe
// @description     Adds taskbar position, size, and item visibility controls to the native Windows 11 context menu.
// @version         1.1
// @author          Mgrmjp
// @github          https://github.com/Mgrmjp
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -ladvapi32 -lshell32 -lcomctl32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Adds **Move taskbar** to the native Windows 11 empty-taskbar context menu.
Its submenu contains Top, Bottom, Left, and Right, excluding the current edge
so there are exactly three choices.

Requires Windows 11 with the native **Taskbar position** setting enabled.
Tested movement on build 26200.9278. This does not add positioning support to
older Windows versions. The entry is hidden if the native TaskbarLocation
setting or the current taskbar edge cannot be read.

Changes the same persistent, user-wide setting as Windows Settings, and tells
Explorer to apply it immediately. No Explorer restart is needed. Unloading the
mod leaves the chosen position in place.

Based on Taskbar Restart Explorer by Mgrmjp. Uses the same native XAML menu
hooks; unsupported menu symbols cause initialization to fail safely.
*/
// ==/WindhawkModReadme==

#include <windhawk_utils.h>

#include <windows.h>
#include <winver.h>
#include <shellapi.h>
#include <roapi.h>
#include <inspectable.h>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/base.h>

#include <atomic>
#include <string_view>

namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxc = winrt::Windows::UI::Xaml::Controls;
namespace wuxi = winrt::Windows::UI::Xaml::Input;

// Native Settings ISettingItem ABI, verified against the Windows 11 public
// symbols. QueryInterface validates the contract before any methods are used.
struct NativeSettingItem : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE get_Id(HSTRING*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Type(INT32*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsSetByGroupPolicy(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsEnabled(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsApplicable(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Description(HSTRING*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsUpdating(boolean*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetValue(HSTRING, IInspectable**) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetValue(HSTRING, IInspectable*) = 0;
};
__CRT_UUID_DECL(NativeSettingItem, 0x40c037cc, 0xd8bf, 0x489e,
                0x86, 0x97, 0xd6, 0x6b, 0xaa, 0x32, 0x21, 0xbf)

constexpr wchar_t kSearchKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Search";
struct TaskbarOption {
    const wchar_t* text;
    const wchar_t* runtimeClass;
    const wchar_t* registryName;
    bool search;
    bool booleanValue;
};
constexpr TaskbarOption kSizeOption{
    L"Taskbar size", L"SystemSettings.Desktop.Taskbar.DesktopTaskbarSizeSetting",
    L"TaskbarSize", false, false};
constexpr TaskbarOption kItemOptions[] = {
    {L"Search", L"SystemSettings.Desktop.Taskbar.DesktopTaskbarSearchSetting",
     L"SearchboxTaskbarMode", true, false},
    {L"Task View", L"SystemSettings.Desktop.Taskbar.DesktopTaskbarTaskViewSetting",
     L"ShowTaskViewButton", false, true},
    {L"Widgets", L"SystemSettings.Desktop.Taskbar.DesktopTaskbarDaSetting",
     L"TaskbarDa", false, true},
};

static constexpr wchar_t kMoveTaskbarText[] = L"Move taskbar";
static constexpr wchar_t kItemName[] = L"WindhawkMoveTaskbarItem";
static constexpr wchar_t kSeparatorName[] = L"WindhawkMoveTaskbarSeparator";

std::atomic<bool> g_taskbarViewModuleHooked = false;

thread_local int g_taskbarSettingsMenuDepth = 0;
thread_local bool g_currentMenuInjected = false;

bool HStringEquals(winrt::hstring const& value, const wchar_t* text) {
    return std::wstring_view(value.c_str(), value.size()) == text;
}

bool IsNamedItem(wuxc::MenuFlyoutItemBase const& baseItem,
                 const wchar_t* name) {
    try {
        if (auto frameworkElement = baseItem.try_as<wux::FrameworkElement>()) {
            return HStringEquals(frameworkElement.Name(), name);
        }
    } catch (...) {
    }

    return false;
}

bool IsSeparator(wuxc::MenuFlyoutItemBase const& baseItem) {
    try {
        return !!baseItem.try_as<wuxc::MenuFlyoutSeparator>();
    } catch (...) {
    }

    return false;
}

// SettingsHandlers_DesktopTaskbar.dll, DesktopTaskbarSettingsSingleton::Location
// on 26200.9278 writes this DWORD, then sends Shell_TrayWnd 0x5CA, 6, edge.
// The values match ABE_LEFT/TOP/RIGHT/BOTTOM (0/1/2/3).
constexpr wchar_t kAdvancedKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
constexpr UINT kTaskbarSettingChanged = WM_USER + 0x1CA;
constexpr WPARAM kLocationSetting = 6;

winrt::com_ptr<NativeSettingItem> OpenNativeSetting(const TaskbarOption& option) {
    winrt::hstring name{option.runtimeClass};
    winrt::com_ptr<IInspectable> object;
    winrt::check_hresult(RoActivateInstance(
        reinterpret_cast<HSTRING>(winrt::get_abi(name)), object.put()));
    return object.as<NativeSettingItem>();
}

bool ReadOption(const TaskbarOption& option, DWORD* value) {
    DWORD bytes = sizeof(*value);
    return RegGetValueW(HKEY_CURRENT_USER,
                        option.search ? kSearchKey : kAdvancedKey,
                        option.registryName, RRF_RT_REG_DWORD, nullptr,
                        value, &bytes) == ERROR_SUCCESS;
}

bool CanChangeOption(NativeSettingItem* setting) {
    boolean enabled = false, applicable = false, managed = true;
    return SUCCEEDED(setting->get_IsEnabled(&enabled)) && enabled &&
           SUCCEEDED(setting->get_IsApplicable(&applicable)) && applicable &&
           SUCCEEDED(setting->get_IsSetByGroupPolicy(&managed)) && !managed;
}

bool SetOption(const TaskbarOption& option, DWORD value) {
    try {
        auto setting = OpenNativeSetting(option);
        if (!CanChangeOption(setting.get())) {
            Wh_Log(L"%s is unavailable or managed by Windows", option.text);
            return false;
        }
        winrt::hstring property{L"Value"};
        auto boxed = option.booleanValue ? winrt::box_value(value != 0)
                                         : winrt::box_value(static_cast<int32_t>(value));
        winrt::check_hresult(setting->SetValue(
            reinterpret_cast<HSTRING>(winrt::get_abi(property)),
            reinterpret_cast<IInspectable*>(winrt::get_abi(boxed))));
        // Some Settings handlers swallow a failed registry write. Verify it.
        DWORD saved;
        if (!ReadOption(option, &saved) || saved != value) {
            Wh_Log(L"%s: Windows did not save the requested value %lu",
                   option.text, value);
            return false;
        }
        Wh_Log(L"%s changed to %lu", option.text, value);
        return true;
    } catch (winrt::hresult_error const& e) {
        Wh_Log(L"%s failed: %08X", option.text, static_cast<unsigned>(e.code().value));
    } catch (...) {
        Wh_Log(L"%s failed", option.text);
    }
    return false;
}

bool IsOptionAvailable(const TaskbarOption& option) {
    try {
        return CanChangeOption(OpenNativeSetting(option).get());
    } catch (...) {
        return false;
    }
}

#include <stdio.h>
#include <commctrl.h>
int phase=0;
DWORD originals[4]{};
UINT_PTR timer=0;
HWND tray=nullptr;
const TaskbarOption& Opt(int i) {return i==0 ? kSizeOption : kItemOptions[i-1];}
void Log(const wchar_t* action,int i,bool success) {
    FILE* file=_wfopen(L"C:\\Users\\Adams\\Code\\windhawk-mods\\.codex-build\\options-live.log",L"a");
    if(!file)return;
    DWORD stored=99; ReadOption(Opt(i),&stored);
    APPBARDATA data{sizeof(data)};SHAppBarMessage(ABM_GETTASKBARPOS,&data);
    fprintf(file,"%ls %ls success=%d value=%lu rect=%ld,%ld,%ld,%ld\n",action,Opt(i).text,success,stored,data.rc.left,data.rc.top,data.rc.right,data.rc.bottom);
    fclose(file);
}
LRESULT CALLBACK Proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp,DWORD_PTR) {
    if(msg==WM_TIMER && wp==timer) {
        int i=phase/2;
        if(phase%2==0) {
            bool result=SetOption(Opt(i), originals[i] ? 0 : 1);
            Log(L"set",i,result);
        } else {
            Log(L"observed",i,true);
            bool result=SetOption(Opt(i),originals[i]);
            Log(L"restore",i,result);
        }
        if(++phase==8){KillTimer(hwnd,timer);timer=0;}
        return 0;
    }
    return DefSubclassProc(hwnd,msg,wp,lp);
}
BOOL Wh_ModInit(){return TRUE;}
void Wh_ModAfterInit(){
    tray=FindWindowW(L"Shell_TrayWnd",nullptr);
    if(!tray)return;
    for(int i=0;i<4;i++)if(!ReadOption(Opt(i),&originals[i]))return;
    if(WindhawkUtils::SetWindowSubclassFromAnyThread(tray,Proc,0))timer=SetTimer(tray,0x524D5052,2000,nullptr);
}
void Wh_ModUninit(){
    if(timer){KillTimer(tray,timer); for(int i=0;i<4;i++)SetOption(Opt(i),originals[i]);}
    if(tray)WindhawkUtils::RemoveWindowSubclassFromAnyThread(tray,Proc);
}
