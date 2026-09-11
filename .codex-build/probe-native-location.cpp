#include <windows.h>
#include <roapi.h>
#include <inspectable.h>
#include <shellapi.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <cstdio>
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

int main() {
    setvbuf(stdout,nullptr,_IONBF,0);
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::hstring name{L"SystemSettings.Desktop.Taskbar.DesktopTaskbarLoSetting"};
    winrt::hstring key{L"Value"};
    winrt::com_ptr<IInspectable> object;
    auto hr=RoActivateInstance((HSTRING)winrt::get_abi(name),object.put());
    if(FAILED(hr)){printf("activation=%lx\n",(unsigned long)hr);return 1;}
    auto setting=object.as<NativeSettingItem>();
    winrt::Windows::Foundation::IInspectable original{nullptr};
    winrt::check_hresult(setting->GetValue((HSTRING)winrt::get_abi(key),(IInspectable**)winrt::put_abi(original)));
    printf("original selection=%d\n",winrt::unbox_value<int32_t>(original));
    boolean enabled{},applicable{},managed{};
    setting->get_IsEnabled(&enabled);setting->get_IsApplicable(&applicable);setting->get_IsSetByGroupPolicy(&managed);
    printf("enabled=%d applicable=%d managed=%d\n",enabled,applicable,managed);
    const UINT edges[]={ABE_BOTTOM,ABE_TOP,ABE_LEFT,ABE_RIGHT};
    bool passed=true;
    for(int32_t index=0;index<4;index++){
        auto value=winrt::box_value(index);
        hr=setting->SetValue((HSTRING)winrt::get_abi(key),(IInspectable*)winrt::get_abi(value));
        Sleep(500);
        APPBARDATA data{sizeof(data)};SHAppBarMessage(ABM_GETTASKBARPOS,&data);
        printf("selection=%d hr=%lx edge=%u expected=%u\n",index,(unsigned long)hr,data.uEdge,edges[index]);
        passed=passed && SUCCEEDED(hr) && data.uEdge==edges[index];
    }
    hr=setting->SetValue((HSTRING)winrt::get_abi(key),(IInspectable*)winrt::get_abi(original));
    printf("restore=%lx\n",(unsigned long)hr);
    return passed && SUCCEEDED(hr)?0:1;
}
