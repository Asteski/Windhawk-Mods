#include <windows.h>
#include <roapi.h>
#include <inspectable.h>
#include <shellapi.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <stdio.h>
struct SettingItem : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE Reserved6()=0;
    virtual HRESULT STDMETHODCALLTYPE Reserved7()=0;
    virtual HRESULT STDMETHODCALLTYPE Reserved8()=0;
    virtual HRESULT STDMETHODCALLTYPE Reserved9()=0;
    virtual HRESULT STDMETHODCALLTYPE Reserved10()=0;
    virtual HRESULT STDMETHODCALLTYPE Reserved11()=0;
    virtual HRESULT STDMETHODCALLTYPE Reserved12()=0;
    virtual HRESULT STDMETHODCALLTYPE GetValue(HSTRING,IInspectable**)=0;
    virtual HRESULT STDMETHODCALLTYPE SetValue(HSTRING,IInspectable*)=0;
};
__CRT_UUID_DECL(SettingItem,0x40c037cc,0xd8bf,0x489e,0x86,0x97,0xd6,0x6b,0xaa,0x32,0x21,0xbf)
int main() {
    setvbuf(stdout,nullptr,_IONBF,0);
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    for(auto suffix:{L"Size",L"Search",L"TaskView",L"Da"}) {
        try {
            winrt::hstring name{std::wstring(L"SystemSettings.Desktop.Taskbar.DesktopTaskbar")+suffix+L"Setting"};
            winrt::com_ptr<IInspectable> object;
            winrt::check_hresult(RoActivateInstance((HSTRING)winrt::get_abi(name),object.put()));
            auto setting=object.as<SettingItem>();
            winrt::hstring valueName{L"Value"};
            winrt::Windows::Foundation::IInspectable original{nullptr};
            winrt::check_hresult(setting->GetValue((HSTRING)winrt::get_abi(valueName),(IInspectable**)winrt::put_abi(original)));
            auto prop=original.as<winrt::Windows::Foundation::IPropertyValue>();
            printf("%ls type=%d\n",suffix,(int)prop.Type());
            auto value=(*suffix==L'S') ? winrt::box_value((int32_t)(suffix[1]==L'i'?0:1)) : winrt::box_value(true);
            auto hr=setting->SetValue((HSTRING)winrt::get_abi(valueName),(IInspectable*)winrt::get_abi(value));
            printf("set=%lx\n",(unsigned long)hr);
            Sleep(3000); DWORD stored=99, bytes=4; const wchar_t* regName = suffix[0]==L'D' ? L"TaskbarDa" : suffix[0]==L'T' ? L"ShowTaskViewButton" : suffix[1]==L'i' ? L"TaskbarSize" : L"SearchboxTaskbarMode"; const wchar_t* key=suffix[0]==L'S' && suffix[1]==L'e' ? L"Software\\Microsoft\\Windows\\CurrentVersion\\Search" : L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"; auto read=RegGetValueW(HKEY_CURRENT_USER,key,regName,RRF_RT_REG_DWORD,nullptr,&stored,&bytes); printf("stored=%lu read=%ld\n",stored,read);
            APPBARDATA data{sizeof(data)};SHAppBarMessage(ABM_GETTASKBARPOS,&data);printf("rectangle=%ld,%ld,%ld,%ld\n",data.rc.left,data.rc.top,data.rc.right,data.rc.bottom);
            auto restore=setting->SetValue((HSTRING)winrt::get_abi(valueName),(IInspectable*)winrt::get_abi(original));
            printf("restore=%lx\n",(unsigned long)restore);
        } catch(winrt::hresult_error const& e) {printf("%ls error=%lx\n",suffix,(unsigned long)e.code().value);}
    }
}



