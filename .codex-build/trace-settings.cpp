#include <windows.h>
#include <stdio.h>
LRESULT CALLBACK Proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_SETTINGCHANGE) wprintf(L"WM_SETTINGCHANGE w=%llu text=%s\n", (unsigned long long)wp, lp ? (wchar_t*)lp : L"<null>");
    if (msg >= 0xC000) { wchar_t name[256]{}; GetClipboardFormatNameW(msg, name, 256); wprintf(L"Registered %x %s w=%llu l=%lld\n", msg, name, (unsigned long long)wp, (long long)lp); }
    fflush(stdout);
    return DefWindowProcW(hwnd, msg, wp, lp);
}
int main() {
    WNDCLASSW wc{}; wc.lpfnWndProc=Proc; wc.hInstance=GetModuleHandleW(nullptr); wc.lpszClassName=L"TaskbarSettingsProbe";
    RegisterClassW(&wc); CreateWindowW(wc.lpszClassName, L"Taskbar settings probe", 0,0,0,0,0,nullptr,nullptr,wc.hInstance,nullptr);
    SetTimer(nullptr, 0, 45000, [](HWND,UINT,UINT_PTR,DWORD){PostQuitMessage(0);});
    MSG msg; while(GetMessageW(&msg,nullptr,0,0)>0) DispatchMessageW(&msg);
}
