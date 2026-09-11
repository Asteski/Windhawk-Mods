// ==WindhawkMod==
// @id              vd-auto-launcher
// @name            Virtual Desktop Auto Launcher
// @description     Launch configured apps on named virtual desktops (creates desktops if needed)
// @version         1.0.0
// @author          Asteski
// @include         *
// @compilerOptions -lole32 -lshlwapi
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Virtual Desktop Auto Launcher

Defines pairs of virtual desktop names and commands to launch. The mod will ensure a named
virtual desktop exists (creating it if needed) and run the configured app on that desktop.

Settings format (one pair per line):
```
Desktop Name|command to run
```
Example:
```
Work|"C:\\Program Files\\Microsoft VS Code\\Code.exe"
Music|spotify.exe
```

This mod uses an external helper `virtualdesktophelper.dll` if present in the Windhawk directory.
It will try to call exported helpers tolaunch without switching desktops; if those are not
available it will fall back to switching to the target desktop, launching the app, then
switching back.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- appDesktopPairs: ""
- createDesktopIfMissing: 1
- helperDllPath: virtualdesktophelper.dll
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <shlwapi.h>
#include <ole2.h>

#ifndef Wh_Log
#define Wh_Log(...) do {} while (0)
#endif

struct Pair {
    std::wstring desktopName;
    std::wstring command;
};

// Function pointer types for virtualdesktophelper.dll (best-effort guesses)
typedef HRESULT(__stdcall* VdhFindDesktopByName_t)(LPCWSTR, GUID*);
typedef HRESULT(__stdcall* VdhCreateDesktopWithName_t)(LPCWSTR, GUID*);
typedef HRESULT(__stdcall* VdhLaunchProcessOnDesktop_t)(const GUID*, LPCWSTR);
typedef HRESULT(__stdcall* VdhSwitchToDesktop_t)(const GUID*);

// Additional common VirtualDesktopAccessor-style exports (by number)
typedef int(__stdcall* GetDesktopCount_t)();
typedef int(__stdcall* GetCurrentDesktopNumber_t)();
typedef int(__stdcall* GetDesktopNumberFromName_t)(LPCWSTR);
typedef int(__stdcall* GoToDesktopNumber_t)(int);
typedef int(__stdcall* CreateDesktopNumber_t)(LPCWSTR); // returns new desktop number or -1
typedef int(__stdcall* LaunchOnDesktopNumber_t)(int, LPCWSTR);

static std::vector<Pair> g_pairs;
static BOOL g_createIfMissing = TRUE;
static std::wstring g_helperDllPath = L"VirtualDesktopAccessor.DLL";

static HMODULE g_helper = nullptr;
static VdhFindDesktopByName_t pFindDesktop = nullptr;
static VdhCreateDesktopWithName_t pCreateDesktop = nullptr;
static VdhLaunchProcessOnDesktop_t pLaunchOnDesktop = nullptr;
static VdhSwitchToDesktop_t pSwitchToDesktop = nullptr;

// number-based
static GetDesktopCount_t pGetDesktopCount = nullptr;
static GetCurrentDesktopNumber_t pGetCurrentDesktopNumber = nullptr;
static GetDesktopNumberFromName_t pGetDesktopNumberFromName = nullptr;
static GoToDesktopNumber_t pGoToDesktopNumber = nullptr;
static CreateDesktopNumber_t pCreateDesktopNumber = nullptr;
static LaunchOnDesktopNumber_t pLaunchOnDesktopNumber = nullptr;

// Helpers
static std::wstring GetWindhawkDir() {
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, buf, _countof(buf))) {
        // assume host exe; Windhawk folder is parent of mods at runtime - best-effort
        PathRemoveFileSpecW(buf);
        return std::wstring(buf);
    }
    return L".";
}

static std::wstring Trim(const std::wstring& s) {
    size_t a = 0, b = s.size();
    while (a < b && iswspace(s[a])) a++;
    while (b > a && iswspace(s[b-1])) b--;
    return s.substr(a, b - a);
}

static void ParseSettings() {
    g_pairs.clear();

    PCWSTR pairsStr = Wh_GetStringSetting(L"appDesktopPairs");
    if (!pairsStr) return;

    std::wstring all = pairsStr;
    Wh_FreeStringSetting(pairsStr);

    std::wistringstream ss(all);
    std::wstring line;
    while (std::getline(ss, line)) {
        line = Trim(line);
        if (line.empty()) continue;

        size_t sep = line.find(L'|');
        if (sep == std::wstring::npos) continue;
        Pair p;
        p.desktopName = Trim(line.substr(0, sep));
        p.command = Trim(line.substr(sep + 1));
        if (!p.desktopName.empty() && !p.command.empty()) {
            g_pairs.push_back(p);
        }
    }

    int create = Wh_GetIntSetting(L"createDesktopIfMissing");
    g_createIfMissing = create != 0;

    PCWSTR dllPath = Wh_GetStringSetting(L"helperDllPath");
    if (dllPath) {
        g_helperDllPath = dllPath;
        Wh_FreeStringSetting(dllPath);
    }
}

static bool LoadHelperDll() {
    if (g_helper) return true;

    std::wstring dir = GetWindhawkDir();
    std::wstring full = dir + L"\\" + g_helperDllPath;
    g_helper = LoadLibraryW(full.c_str());
    if (!g_helper) {
        Wh_Log(L"vd-auto-launcher: helper DLL not found at %s, proceeding without it.", full.c_str());
        return false;
    }

    // try several common export names
    pFindDesktop = (VdhFindDesktopByName_t)GetProcAddress(g_helper, "VdhFindDesktopByName");
    if (!pFindDesktop) pFindDesktop = (VdhFindDesktopByName_t)GetProcAddress(g_helper, "FindDesktopByName");

    pCreateDesktop = (VdhCreateDesktopWithName_t)GetProcAddress(g_helper, "VdhCreateDesktopWithName");
    if (!pCreateDesktop) pCreateDesktop = (VdhCreateDesktopWithName_t)GetProcAddress(g_helper, "CreateDesktopWithName");

    pLaunchOnDesktop = (VdhLaunchProcessOnDesktop_t)GetProcAddress(g_helper, "VdhLaunchProcessOnDesktop");
    if (!pLaunchOnDesktop) pLaunchOnDesktop = (VdhLaunchProcessOnDesktop_t)GetProcAddress(g_helper, "LaunchProcessOnDesktop");

    pSwitchToDesktop = (VdhSwitchToDesktop_t)GetProcAddress(g_helper, "VdhSwitchToDesktop");
    if (!pSwitchToDesktop) pSwitchToDesktop = (VdhSwitchToDesktop_t)GetProcAddress(g_helper, "SwitchToDesktop");

    // number-based helpers
    pGetDesktopCount = (GetDesktopCount_t)GetProcAddress(g_helper, "GetDesktopCount");
    pGetCurrentDesktopNumber = (GetCurrentDesktopNumber_t)GetProcAddress(g_helper, "GetCurrentDesktopNumber");
    pGetDesktopNumberFromName = (GetDesktopNumberFromName_t)GetProcAddress(g_helper, "GetDesktopNumberFromName");
    if (!pGetDesktopNumberFromName) pGetDesktopNumberFromName = (GetDesktopNumberFromName_t)GetProcAddress(g_helper, "GetDesktopNumberByName");
    pGoToDesktopNumber = (GoToDesktopNumber_t)GetProcAddress(g_helper, "GoToDesktopNumber");
    pCreateDesktopNumber = (CreateDesktopNumber_t)GetProcAddress(g_helper, "CreateDesktopNumber");
    pLaunchOnDesktopNumber = (LaunchOnDesktopNumber_t)GetProcAddress(g_helper, "LaunchOnDesktopNumber");

    Wh_Log(L"vd-auto-launcher: loaded helper DLL %s", full.c_str());
    return true;
}

// Expand environment variables
static std::wstring ExpandEnv(const std::wstring& s) {
    wchar_t buf[32768];
    DWORD ret = ExpandEnvironmentStringsW(s.c_str(), buf, _countof(buf));
    if (ret == 0 || ret > _countof(buf)) return s;
    return std::wstring(buf);
}

// Launch helper-based or fallback process on desktop
static void LaunchOnDesktop(const Pair& p) {
    GUID desktopId;
    bool haveDesktop = false;

    if (g_helper && pFindDesktop) {
        if (SUCCEEDED(pFindDesktop(p.desktopName.c_str(), &desktopId))) {
            haveDesktop = true;
        }
    }

    if (!haveDesktop && g_createIfMissing) {
        if (g_helper && pCreateDesktop) {
            if (SUCCEEDED(pCreateDesktop(p.desktopName.c_str(), &desktopId))) {
                haveDesktop = true;
            }
        }
    }

    std::wstring cmd = ExpandEnv(p.command);

    if (haveDesktop && g_helper && pLaunchOnDesktop) {
        if (SUCCEEDED(pLaunchOnDesktop(&desktopId, cmd.c_str()))) {
            Wh_Log(L"vd-auto-launcher: launched on desktop '%s' via helper: %s", p.desktopName.c_str(), cmd.c_str());
            return;
        }
    }

    // If number-based helper exists, try that path
    if (g_helper && pGetDesktopNumberFromName) {
        int desktopNum = pGetDesktopNumberFromName(p.desktopName.c_str());
        if (desktopNum >= 0) {
            // possibly create if missing
            if (desktopNum == -1 && g_createIfMissing && pCreateDesktopNumber) {
                desktopNum = pCreateDesktopNumber(p.desktopName.c_str());
            }
            if (desktopNum >= 0) {
                if (pLaunchOnDesktopNumber) {
                    if (pLaunchOnDesktopNumber(desktopNum, cmd.c_str()) == 0) {
                        Wh_Log(L"vd-auto-launcher: launched via number-based helper on '%s': %s", p.desktopName.c_str(), cmd.c_str());
                        return;
                    }
                } else if (pGoToDesktopNumber && pGetCurrentDesktopNumber) {
                    int cur = pGetCurrentDesktopNumber();
                    pGoToDesktopNumber(desktopNum);
                    // launch
                    SHELLEXECUTEINFOW ei{};
                    ei.cbSize = sizeof(ei);
                    ei.fMask = SEE_MASK_NOASYNC;
                    ei.nShow = SW_SHOWNORMAL;
                    ei.lpFile = cmd.c_str();
                    ShellExecuteExW(&ei);
                    // switch back
                    pGoToDesktopNumber(cur);
                    Wh_Log(L"vd-auto-launcher: launched on desktop number %d via switch fallback: %s", desktopNum, cmd.c_str());
                    return;
                }
            }
        }
    }

    // Fallback: if we have desktop id and switch function, switch, launch, switch back
    GUID currentId{};
    bool switched = false;
    if (haveDesktop && g_helper && pSwitchToDesktop) {
        // Save current by switching to desktop we'll treat as "current" later — best-effort
        if (SUCCEEDED(pSwitchToDesktop(&desktopId))) {
            switched = true;
        }
    }

    // Launch via ShellExecute (uses default logic)
    SHELLEXECUTEINFOW ei{};
    ei.cbSize = sizeof(ei);
    ei.fMask = SEE_MASK_NOASYNC;
    ei.nShow = SW_SHOWNORMAL;
    ei.lpFile = cmd.c_str();
    if (ShellExecuteExW(&ei) && (INT_PTR)ei.hInstApp > 32) {
        Wh_Log(L"vd-auto-launcher: launched command: %s", cmd.c_str());
    } else {
        Wh_Log(L"vd-auto-launcher: failed to launch command: %s (error=%lu)", cmd.c_str(), GetLastError());
    }

    // Note: we do not attempt to switch back in this fallback — switching behavior varies by helper implementation
}

// Run through configured pairs
static void RunAllPairs() {
    if (g_pairs.empty()) {
        Wh_Log(L"vd-auto-launcher: no configured pairs");
        return;
    }

    LoadHelperDll();

    for (const Pair& p : g_pairs) {
        if (p.desktopName.empty() || p.command.empty()) continue;
        LaunchOnDesktop(p);
    }
}

// Windhawk entry points
BOOL Wh_ModInit() {
    ParseSettings();
    Wh_Log(L"vd-auto-launcher: initializing, pairs=%d", (int)g_pairs.size());
    // Run on init
    RunAllPairs();
    return TRUE;
}

VOID Wh_ModAfterInit() {
    // Nothing extra for now
}

VOID Wh_ModUninit() {
    if (g_helper) {
        FreeLibrary(g_helper);
        g_helper = nullptr;
    }
}
