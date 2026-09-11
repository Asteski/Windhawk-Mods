// ==WindhawkMod==
// @id              alt-left-drag-move
// @name            Alt + Left Drag Move Window
// @description     Hold Alt and drag with left mouse button from anywhere inside a window to move it.
// @version         1.0.0
// @author          Asteski
// @github          https://github.com/Asteski
// @include         explorer.exe
// @compilerOptions -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Alt + Left Drag Move Window

Implements the classic AltSnap-style behavior:

- Hold Alt
- Press and drag with the left mouse button anywhere in a window client area
- The window starts moving as if dragged by its title bar

The hook is hosted in explorer.exe and works globally.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <atomic>

BOOL Wh_ModInit();

static HHOOK g_mouseHook = nullptr;
static HANDLE g_hookThread = nullptr;
static DWORD g_hookThreadId = 0;
static HANDLE g_hookReadyEvent = nullptr;
static std::atomic<bool> g_unloading = false;
static bool g_suppressNextLButtonUp = false;

HMODULE GetThisModule() {
    HMODULE h = nullptr;
    if (GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&Wh_ModInit), &h)) {
        return h;
    }
    return nullptr;
}

bool IsMoveCandidateWindow(HWND hWnd) {
    if (!hWnd || !IsWindow(hWnd) || !IsWindowVisible(hWnd)) {
        return false;
    }

    if (hWnd == GetDesktopWindow() || hWnd == GetShellWindow()) {
        return false;
    }

    LONG_PTR style = GetWindowLongPtrW(hWnd, GWL_STYLE);
    if (style & WS_CHILD) {
        return false;
    }

    if ((style & (WS_CAPTION | WS_THICKFRAME)) == 0) {
        return false;
    }

    if (IsIconic(hWnd) || IsZoomed(hWnd)) {
        return false;
    }

    wchar_t className[64] = {};
    if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) > 0) {
        if (wcscmp(className, L"Shell_TrayWnd") == 0 ||
            wcscmp(className, L"Progman") == 0 ||
            wcscmp(className, L"WorkerW") == 0) {
            return false;
        }
    }

    return true;
}

bool IsClientAreaHit(HWND hWnd, POINT screenPt) {
    DWORD_PTR hitResult = 0;
    LRESULT ok = SendMessageTimeoutW(
        hWnd, WM_NCHITTEST, 0, MAKELPARAM(screenPt.x, screenPt.y),
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &hitResult);

    if (!ok) {
        return false;
    }

    return static_cast<LRESULT>(hitResult) == HTCLIENT;
}

bool BeginWindowMove(HWND hWnd, POINT screenPt) {
    SetForegroundWindow(hWnd);
    ReleaseCapture();

    return PostMessageW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION,
                        MAKELPARAM(screenPt.x, screenPt.y)) != 0;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION || g_unloading.load()) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    if (wParam == WM_LBUTTONUP && g_suppressNextLButtonUp) {
        g_suppressNextLButtonUp = false;
        return 1;
    }

    if (wParam != WM_LBUTTONDOWN) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    if ((GetAsyncKeyState(VK_MENU) & 0x8000) == 0) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    const MSLLHOOKSTRUCT* mouse = reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
    POINT pt = mouse->pt;

    HWND hWnd = WindowFromPoint(pt);
    if (!hWnd) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    hWnd = GetAncestor(hWnd, GA_ROOT);
    if (!IsMoveCandidateWindow(hWnd)) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    if (!IsClientAreaHit(hWnd, pt)) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    if (!BeginWindowMove(hWnd, pt)) {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

    g_suppressNextLButtonUp = true;
    return 1;
}

DWORD WINAPI HookThreadProc(LPVOID) {
    HMODULE hMod = GetThisModule();
    if (!hMod) {
        Wh_Log(L"Alt drag move: warning - module handle lookup failed, trying null module handle");
    }

    g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hMod, 0);

    if (!g_mouseHook) {
        Wh_Log(L"Alt drag move: SetWindowsHookExW failed, error=%lu", GetLastError());
        if (g_hookReadyEvent) {
            SetEvent(g_hookReadyEvent);
        }
        return 0;
    }

    if (g_hookReadyEvent) {
        SetEvent(g_hookReadyEvent);
    }

    MSG msg;
    while (!g_unloading.load()) {
        BOOL gm = GetMessageW(&msg, nullptr, 0, 0);
        if (gm <= 0) {
            break;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_mouseHook) {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = nullptr;
    }

    return 0;
}

BOOL Wh_ModInit() {
    g_unloading = false;
    g_suppressNextLButtonUp = false;
    g_hookThreadId = 0;

    g_hookReadyEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_hookReadyEvent) {
        Wh_Log(L"Alt drag move: failed to create ready event");
        return FALSE;
    }

    g_hookThread = CreateThread(nullptr, 0, HookThreadProc, nullptr, 0, &g_hookThreadId);
    if (!g_hookThread) {
        Wh_Log(L"Alt drag move: failed to create hook thread");
        CloseHandle(g_hookReadyEvent);
        g_hookReadyEvent = nullptr;
        return FALSE;
    }

    WaitForSingleObject(g_hookReadyEvent, 3000);
    CloseHandle(g_hookReadyEvent);
    g_hookReadyEvent = nullptr;

    if (!g_mouseHook) {
        Wh_Log(L"Alt drag move: hook initialization failed");
        if (g_hookThreadId) {
            PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
        }
        if (g_hookThread) {
            WaitForSingleObject(g_hookThread, 3000);
            CloseHandle(g_hookThread);
            g_hookThread = nullptr;
        }
        g_hookThreadId = 0;
        return FALSE;
    }

    Wh_Log(L"Alt drag move: initialized");
    return TRUE;
}

void Wh_ModUninit() {
    g_unloading = true;

    if (g_hookThreadId) {
        PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
    }

    if (g_hookThread) {
        WaitForSingleObject(g_hookThread, 3000);
        CloseHandle(g_hookThread);
        g_hookThread = nullptr;
    }

    g_hookThreadId = 0;

    if (g_hookReadyEvent) {
        CloseHandle(g_hookReadyEvent);
        g_hookReadyEvent = nullptr;
    }

    Wh_Log(L"Alt drag move: uninitialized");
}
