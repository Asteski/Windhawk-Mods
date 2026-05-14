// ==WindhawkMod==
// @id              asteski-force-app-corner
// @name            Force App Window to Screen Corner
// @description     Force selected apps to open in a chosen screen corner.
// @version         1.0
// @author          Asteski
// @license         MIT
// @github          https://github.com/Asteski
// @include         *
// @compilerOptions -luser32 -lshlwapi
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Force App Window Corner

Moves selected applications to a specific corner when their top-level windows are shown.

## How it works
- The mod checks the current process executable name.
- If it matches one of your configured rules, shown windows are moved to the selected corner.
- If there is no match, the process is ignored.

## Rules
Each rule has:
- **Executable name(s)**: one or multiple names in one field.
- **Screen corner**: top-left, top-right, bottom-left, bottom-right.

You can add as many rules as you want.

## Executable name format
- Case-insensitive.
- You can write `notepad` or `notepad.exe`.
- You can include multiple values separated by `,` `;` or `|`.
- Full paths are also accepted (file name is used).

Examples:
- `notepad.exe`
- `notepad; calc.exe`
- `C:\Windows\System32\mspaint.exe | calc`
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- rules:
    - - executable_names: "notepad.exe; calc.exe"
        $name: Executable name(s)
        $description: "One or more executable names. Separators: comma, semicolon, pipe."
      - corner: top-right
        $name: Screen corner
        $options:
          - top-left: Top-left
          - top-right: Top-right
          - bottom-left: Bottom-left
          - bottom-right: Bottom-right
      - offset_horizontal: 0
        $name: Horizontal offset (pixels)
        $description: "Distance from left/right edge. Positive = away from edge."
      - offset_vertical: 0
        $name: Vertical offset (pixels)
        $description: "Distance from top/bottom edge. Positive = away from edge."
  $name: Rules
  $description: "Add one item per executable/corner combination."
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <shlwapi.h>

#include <algorithm>
#include <cwctype>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

enum class ScreenCorner {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

struct Rule {
    std::vector<std::wstring> executables;
    ScreenCorner corner = ScreenCorner::TopLeft;
    int offsetHorizontal = 0;
    int offsetVertical = 0;
};

static std::vector<Rule> g_rules;
static std::mutex g_rulesMutex;

static std::unordered_set<HWND> g_movedWindows;
static std::mutex g_movedMutex;

static std::wstring g_currentExeName;
static bool g_isTargetProcess = false;
static ScreenCorner g_targetCorner = ScreenCorner::TopLeft;
static int g_targetOffsetHorizontal = 0;
static int g_targetOffsetVertical = 0;

static thread_local bool g_internalMove = false;

using ShowWindow_t = decltype(&ShowWindow);
using ShowWindowAsync_t = decltype(&ShowWindowAsync);
using SetWindowPos_t = decltype(&SetWindowPos);

static ShowWindow_t ShowWindow_Original;
static ShowWindowAsync_t ShowWindowAsync_Original;
static SetWindowPos_t SetWindowPos_Original;

static std::wstring Trim(const std::wstring& s) {
    size_t start = 0;
    while (start < s.size() && iswspace(s[start])) {
        start++;
    }

    size_t end = s.size();
    while (end > start && iswspace(s[end - 1])) {
        end--;
    }

    return s.substr(start, end - start);
}

static std::wstring ToLower(std::wstring s) {
    for (auto& c : s) {
        c = (wchar_t)towlower(c);
    }
    return s;
}

static std::wstring NormalizeExecutableName(std::wstring name) {
    name = Trim(name);
    if (name.size() >= 2 && name.front() == L'"' && name.back() == L'"') {
        name = name.substr(1, name.size() - 2);
    }

    if (name.empty()) {
        return name;
    }

    const wchar_t* fileName = PathFindFileNameW(name.c_str());
    if (fileName && *fileName) {
        name = fileName;
    }

    name = ToLower(Trim(name));

    if (!name.empty() && name.find(L'.') == std::wstring::npos) {
        name += L".exe";
    }

    return name;
}

static std::vector<std::wstring> SplitExecutableList(const std::wstring& value) {
    std::vector<std::wstring> result;

    size_t start = 0;
    while (start <= value.size()) {
        size_t pos = value.find_first_of(L",;|", start);
        std::wstring part = pos == std::wstring::npos
            ? value.substr(start)
            : value.substr(start, pos - start);

        std::wstring normalized = NormalizeExecutableName(part);
        if (!normalized.empty() &&
            std::find(result.begin(), result.end(), normalized) == result.end()) {
            result.push_back(normalized);
        }

        if (pos == std::wstring::npos) {
            break;
        }
        start = pos + 1;
    }

    return result;
}

static ScreenCorner ParseCorner(const std::wstring& corner) {
    std::wstring v = ToLower(Trim(corner));
    if (v == L"top-right") {
        return ScreenCorner::TopRight;
    }
    if (v == L"bottom-left") {
        return ScreenCorner::BottomLeft;
    }
    if (v == L"bottom-right") {
        return ScreenCorner::BottomRight;
    }
    return ScreenCorner::TopLeft;
}

static std::wstring GetCurrentProcessExecutableName() {
    wchar_t path[MAX_PATH] = {};
    DWORD n = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (n == 0 || n == ARRAYSIZE(path)) {
        return L"";
    }

    const wchar_t* fileName = PathFindFileNameW(path);
    if (!fileName || !*fileName) {
        return L"";
    }

    return ToLower(fileName);
}

static void ResetMovedWindows() {
    std::lock_guard<std::mutex> lock(g_movedMutex);
    g_movedWindows.clear();
}

static bool HasMovedWindow(HWND hWnd) {
    std::lock_guard<std::mutex> lock(g_movedMutex);
    return g_movedWindows.find(hWnd) != g_movedWindows.end();
}

static void MarkWindowMoved(HWND hWnd) {
    std::lock_guard<std::mutex> lock(g_movedMutex);
    g_movedWindows.insert(hWnd);
}

static bool IsEligibleTopLevelWindow(HWND hWnd) {
    if (!hWnd || !IsWindow(hWnd)) {
        return false;
    }

    if (GetAncestor(hWnd, GA_ROOT) != hWnd) {
        return false;
    }

    if (GetWindow(hWnd, GW_OWNER) != nullptr) {
        return false;
    }

    LONG_PTR exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return false;
    }

    if (IsIconic(hWnd)) {
        return false;
    }

    RECT rc = {};
    if (!GetWindowRect(hWnd, &rc)) {
        return false;
    }

    if (rc.right <= rc.left || rc.bottom <= rc.top) {
        return false;
    }

    return true;
}

static bool MoveWindowToCorner(HWND hWnd, ScreenCorner corner, int offsetH, int offsetV) {
    RECT windowRect = {};
    if (!GetWindowRect(hWnd, &windowRect)) {
        return false;
    }

    const int width = windowRect.right - windowRect.left;
    const int height = windowRect.bottom - windowRect.top;
    if (width <= 0 || height <= 0) {
        return false;
    }

    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(monitor, &mi)) {
        return false;
    }

    const RECT& work = mi.rcWork;
    int x = work.left;
    int y = work.top;

    switch (corner) {
        case ScreenCorner::TopLeft:
            x = work.left + offsetH;
            y = work.top + offsetV;
            break;
        case ScreenCorner::TopRight:
            x = work.right - width - offsetH;
            y = work.top + offsetV;
            break;
        case ScreenCorner::BottomLeft:
            x = work.left + offsetH;
            y = work.bottom - height - offsetV;
            break;
        case ScreenCorner::BottomRight:
            x = work.right - width - offsetH;
            y = work.bottom - height - offsetV;
            break;
    }

    if (x < work.left) {
        x = work.left;
    }
    if (y < work.top) {
        y = work.top;
    }

    g_internalMove = true;
    BOOL ok = SetWindowPos_Original(
        hWnd,
        nullptr,
        x,
        y,
        0,
        0,
        SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    g_internalMove = false;

    return ok != FALSE;
}

static void TryApplyToWindow(HWND hWnd) {
    if (!g_isTargetProcess) {
        return;
    }

    if (!IsEligibleTopLevelWindow(hWnd)) {
        return;
    }

    if (HasMovedWindow(hWnd)) {
        return;
    }

    if (MoveWindowToCorner(hWnd, g_targetCorner, g_targetOffsetHorizontal, g_targetOffsetVertical)) {
        MarkWindowMoved(hWnd);
    }
}

static void RefreshTargetStateLocked() {
    g_isTargetProcess = false;

    for (const auto& rule : g_rules) {
        for (const auto& exe : rule.executables) {
            if (exe == g_currentExeName) {
                g_targetCorner = rule.corner;
                g_targetOffsetHorizontal = rule.offsetHorizontal;
                g_targetOffsetVertical = rule.offsetVertical;
                g_isTargetProcess = true;
                return;
            }
        }
    }
}

static void LoadSettings() {
    std::vector<Rule> newRules;

    for (int i = 0; i <= 255; i++) {
        wchar_t key[256] = {};

        swprintf_s(key, L"rules[%d].executable_names", i);
        PCWSTR pExec = Wh_GetStringSetting(key);
        std::wstring executableNames = pExec ? pExec : L"";
        if (pExec) {
            Wh_FreeStringSetting(pExec);
        }

        swprintf_s(key, L"rules[%d].corner", i);
        PCWSTR pCorner = Wh_GetStringSetting(key);
        std::wstring cornerValue = pCorner ? pCorner : L"";
        if (pCorner) {
            Wh_FreeStringSetting(pCorner);
        }

        swprintf_s(key, L"rules[%d].offset_horizontal", i);
        PCWSTR pOffsetH = Wh_GetStringSetting(key);
        int offsetH = pOffsetH ? _wtoi(pOffsetH) : 0;
        if (pOffsetH) {
            Wh_FreeStringSetting(pOffsetH);
        }

        swprintf_s(key, L"rules[%d].offset_vertical", i);
        PCWSTR pOffsetV = Wh_GetStringSetting(key);
        int offsetV = pOffsetV ? _wtoi(pOffsetV) : 0;
        if (pOffsetV) {
            Wh_FreeStringSetting(pOffsetV);
        }

        bool allEmpty = executableNames.empty() && cornerValue.empty();
        if (allEmpty) {
            if (i >= (int)newRules.size() + 2) {
                break;
            }
            continue;
        }

        Rule rule;
        rule.executables = SplitExecutableList(executableNames);
        rule.corner = ParseCorner(cornerValue);
        rule.offsetHorizontal = offsetH;
        rule.offsetVertical = offsetV;

        if (!rule.executables.empty()) {
            newRules.push_back(std::move(rule));
        }
    }

    {
        std::lock_guard<std::mutex> lock(g_rulesMutex);
        g_rules = std::move(newRules);
        RefreshTargetStateLocked();
    }

    ResetMovedWindows();

    if (g_isTargetProcess) {
        Wh_Log(L"Process %s matched a corner rule", g_currentExeName.c_str());
    }
}

static BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == GetCurrentProcessId()) {
        TryApplyToWindow(hWnd);
    }
    return TRUE;
}

static void ApplyToExistingWindows() {
    if (!g_isTargetProcess) {
        return;
    }

    EnumWindows(EnumWindowsCallback, 0);
}

static bool ShouldProcessShowCommand(int nCmdShow) {
    return nCmdShow != SW_HIDE &&
           nCmdShow != SW_MINIMIZE &&
           nCmdShow != SW_SHOWMINIMIZED &&
           nCmdShow != SW_SHOWMINNOACTIVE;
}

BOOL WINAPI ShowWindow_Hook(HWND hWnd, int nCmdShow) {
    BOOL result = ShowWindow_Original(hWnd, nCmdShow);

    if (result && !g_internalMove && ShouldProcessShowCommand(nCmdShow)) {
        TryApplyToWindow(hWnd);
    }

    return result;
}

BOOL WINAPI ShowWindowAsync_Hook(HWND hWnd, int nCmdShow) {
    BOOL result = ShowWindowAsync_Original(hWnd, nCmdShow);

    if (result && !g_internalMove && ShouldProcessShowCommand(nCmdShow)) {
        TryApplyToWindow(hWnd);
    }

    return result;
}

BOOL WINAPI SetWindowPos_Hook(
    HWND hWnd,
    HWND hWndInsertAfter,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags) {
    BOOL result = SetWindowPos_Original(
        hWnd,
        hWndInsertAfter,
        X,
        Y,
        cx,
        cy,
        uFlags);

    if (!result || g_internalMove) {
        return result;
    }

    if (uFlags & SWP_HIDEWINDOW) {
        return result;
    }

    const bool becameVisible = (uFlags & SWP_SHOWWINDOW) != 0;
    const bool movedOrSized = ((uFlags & SWP_NOMOVE) == 0) || ((uFlags & SWP_NOSIZE) == 0);

    if (becameVisible || movedOrSized) {
        TryApplyToWindow(hWnd);
    }

    return result;
}

BOOL Wh_ModInit() {
    g_currentExeName = GetCurrentProcessExecutableName();

    Wh_SetFunctionHook((void*)ShowWindow, (void*)ShowWindow_Hook,
                       (void**)&ShowWindow_Original);
    Wh_SetFunctionHook((void*)ShowWindowAsync, (void*)ShowWindowAsync_Hook,
                       (void**)&ShowWindowAsync_Original);
    Wh_SetFunctionHook((void*)SetWindowPos, (void*)SetWindowPos_Hook,
                       (void**)&SetWindowPos_Original);

    LoadSettings();
    ApplyToExistingWindows();

    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    ApplyToExistingWindows();
}

void Wh_ModUninit() {
    ResetMovedWindows();
}
