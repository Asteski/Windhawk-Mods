// Minimal elevated helper to center a window. Intended to be launched with elevation
// by the Windhawk mod when SetWindowPos fails due to UIPI/Access Denied.

#include <windows.h>
#include <string>

static std::wstring GetArgValue(const std::wstring& arg, const std::wstring& prefix) {
    if (arg.rfind(prefix, 0) == 0) return arg.substr(prefix.size());
    return L"";
}

int wmain(int argc, wchar_t* argv[]) {
    unsigned long long hwndVal = 0;
    int width = 0, height = 0;

    for (int i = 1; i < argc; ++i) {
        std::wstring a = argv[i];
        if (a.rfind(L"--hwnd=", 0) == 0) hwndVal = _wcstoui64(a.c_str() + 7, NULL, 0);
        else if (a.rfind(L"--width=", 0) == 0) width = _wtoi(a.c_str() + 8);
        else if (a.rfind(L"--height=", 0) == 0) height = _wtoi(a.c_str() + 9);
    }

    HWND hWnd = (HWND)(uintptr_t)hwndVal;
    if (!hWnd || !IsWindow(hWnd)) return 2;

    RECT r;
    if (!GetWindowRect(hWnd, &r)) return 3;

    int curW = r.right - r.left;
    int curH = r.bottom - r.top;
    if (width <= 0) width = curW;
    if (height <= 0) height = curH;

    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hMonitor, &mi)) return 4;
    RECT screen = mi.rcWork;

    int newX = screen.left + (screen.right - screen.left - width) / 2;
    int newY = screen.top + (screen.bottom - screen.top - height) / 2;

    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
    if (width <= 0 && height <= 0) flags |= SWP_NOSIZE;

    SetLastError(0);
    BOOL ok = SetWindowPos(hWnd, NULL, newX, newY, width, height, flags);
    return ok ? 0 : 5;
}
