#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char** argv) {
    APPBARDATA data{sizeof(data)};
    SHAppBarMessage(ABM_GETTASKBARPOS, &data);
    printf("Before: edge=%u rect=%ld,%ld,%ld,%ld\n", data.uEdge, data.rc.left, data.rc.top, data.rc.right, data.rc.bottom);
    if (argc > 1) {
        DWORD edge = strtoul(argv[1], nullptr, 10);
        if (edge > 3) return 1;
        auto result = RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"TaskbarLocation", REG_DWORD, &edge, sizeof(edge));
        printf("Write=%ld\n", result);
        DWORD_PTR unused;
        SendMessageTimeoutW(FindWindowW(L"Shell_TrayWnd", nullptr), 0x5CA, 6, edge, SMTO_ABORTIFHUNG, 2000, &unused);
        Sleep(1500);
        SHAppBarMessage(ABM_GETTASKBARPOS, &data);
        printf("After: edge=%u rect=%ld,%ld,%ld,%ld\n", data.uEdge, data.rc.left, data.rc.top, data.rc.right, data.rc.bottom);
    }
}


