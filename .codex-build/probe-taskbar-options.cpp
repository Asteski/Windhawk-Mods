#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
constexpr auto advanced=L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
constexpr auto search=L"Software\\Microsoft\\Windows\\CurrentVersion\\Search";
struct Setting {const wchar_t* key; const wchar_t* name; DWORD value; DWORD command;};
void Apply(const Setting& s, DWORD value) {
    auto status=RegSetKeyValueW(HKEY_CURRENT_USER,s.key,s.name,REG_DWORD,&value,sizeof(value));
    DWORD_PTR result=0;
    if(s.command) SendMessageTimeoutW(FindWindowW(L"Shell_TrayWnd",nullptr),0x5CA,s.command,value,SMTO_ABORTIFHUNG,2000,&result);
    if(s.name==advanced) return;
    printf("write %ls=%lu status=%ld\n",s.name,value,status);
}
void Size() {
    DWORD_PTR result;
    SendMessageTimeoutW(HWND_BROADCAST,WM_SETTINGCHANGE,0,(LPARAM)L"TraySettings",SMTO_ABORTIFHUNG,2000,&result);
    Sleep(2000);
    APPBARDATA data{sizeof(data)}; SHAppBarMessage(ABM_GETTASKBARPOS,&data);
    printf("edge=%u rectangle=%ld,%ld,%ld,%ld\n",data.uEdge,data.rc.left,data.rc.top,data.rc.right,data.rc.bottom);
}
int main(int argc,char**) {
    setvbuf(stdout,nullptr,_IONBF,0);
    Setting settings[]={{advanced,L"TaskbarSize",0,0},{search,L"SearchboxTaskbarMode",1,102},{advanced,L"ShowTaskViewButton",1,103},{advanced,L"TaskbarDa",1,0}};
    DWORD original[4]{};
    for(int i=0;i<4;i++) {DWORD size=sizeof(DWORD); if(RegGetValueW(HKEY_CURRENT_USER,settings[i].key,settings[i].name,RRF_RT_REG_DWORD,nullptr,&original[i],&size))return 1;}
    for(int i=0;i<4;i++) Apply(settings[i],settings[i].value);
    Size();
    Sleep(argc>1?20000:3000);
    for(int i=0;i<4;i++) Apply(settings[i],original[i]);
    Size();
}
