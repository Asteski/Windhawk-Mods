// ==WindhawkMod==
// @id              legacy-context-menu-styler
// @name            Legacy Context Menu Styler
// @description     Style classic Win32 context menus with custom sizing, highlight colors, and translucent effects
// @version         1.0
// @author          asteski
// @include         *
// @compilerOptions -ldwmapi -luxtheme -lcomctl32 -lgdi32 -lmsimg32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Legacy Context Menu Styler

Styles non-immersive/classic Win32 context menus (`#32768` popup menu windows).
It combines the classic menu sizing approach from Custom Menu Height with the
menu/flyout DWM effect path used by Translucent Windows.

## Notes

The translucent effect code does not convert menus to immersive context menus.
It watches classic popup menu windows, subclasses them, and applies DWM blur or
system backdrop attributes after the menu window is sized. The optional "Force
classic menus" setting uses the same Eradicate Immersive Menus technique as
Custom Menu Height to prevent supported shell components from using immersive
menus.

System backdrop effects require Windows 11 22H2 or newer. AccentBlurBehind is
the compatibility blur path.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- forceClassicMenus: FALSE
  $name: Force classic menus
  $description: >-
    Disable supported immersive shell menus so the classic Win32 menu styling
    can apply.
- RenderingMod:
    - ThemeBackground: TRUE
      $name: Windows theme custom rendering
      $description: >-
        Repaints classic menu background pieces so DWM translucency can show
        through.
    - SysColors: TRUE
      $name: New menu system colors
      $description: >-
        Intercepts menu-related system colors in the current process so legacy
        menus blend better with translucent backgrounds.
    - AccentColorControls: TRUE
      $name: Windows theme accent colorizer
      $description: >-
        Allows the accent color mode to use the Windows accent color.
  $name: Rendering customization
- popupMenuItemHeight: 32
  $name: Context menu item height
  $description: >-
    The minimum height of context menu items in pixels.

    Set to 0 to use the default system value.
- popupMenuItemWidth: 0
  $name: Context menu item width
  $description: >-
    The minimum width of context menu items in pixels.

    Set to 0 to use the default system value.
- highlightColorMode: default
  $name: Highlight background color
  $options:
  - default: Default neutral grey
  - accent: Accent color
  - custom: Custom color
- customHighlightColor: "606060"
  $name: Custom highlight color
  $description: >-
    Color in hexadecimal RGB format, for example 606060.
- backgroundEffect: acrylicblur
  $name: Background translucent effects
  $description: >-
    Windows 11 version 22H2 or newer is required for SystemBackdrop effects.
  $options:
  - none: Default
  - acrylicblur: Blur (AccentBlurBehind)
  - acrylicsystem: Acrylic (SystemBackdrop)
  - mica: Mica (SystemBackdrop)
  - mica_tabbed: MicaAlt (SystemBackdrop)
- accentBlurBehindColor: "3A232323"
  $name: AccentBlurBehind color blend
  $description: >-
    Blending color with blur background.

    Color in hexadecimal ARGB format, for example 3A232323.
- immersiveDarkMode: TRUE
  $name: Immersive dark mode
  $description: Apply DWM immersive dark mode to menu windows.
- smallRadius: 6
  $name: Small corner radius
  $description: >-
    Corner radius for classic context menus. Default Win11 is 4.

    Set to -1 to keep the original radius.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <algorithm>
#include <array>
#include <commctrl.h>
#include <dwmapi.h>
#include <mutex>
#include <string>
#include <unordered_set>
#include <uxtheme.h>
#include <utility>
#include <vssym32.h>
#include <windows.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMWCP_DEFAULT
#define DWMWCP_DEFAULT 0
#endif

#ifndef DWMWCP_ROUNDSMALL
#define DWMWCP_ROUNDSMALL 3
#endif

#ifndef DWMSBT_AUTO
#define DWMSBT_AUTO 0
#endif

#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif

#ifndef DWMSBT_TRANSIENTWINDOW
#define DWMSBT_TRANSIENTWINDOW 3
#endif

#ifndef DWMSBT_TABBEDWINDOW
#define DWMSBT_TABBEDWINDOW 4
#endif

#ifdef _WIN64
#define DRAW_IMMERSIVE_MENU \
    L"bool __cdecl ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *)"
#else
#define DRAW_IMMERSIVE_MENU \
    L"bool __stdcall ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *)"
#endif

#define RECTWIDTH(lprc) ((lprc)->right - (lprc)->left)
#define RECTHEIGHT(lprc) ((lprc)->bottom - (lprc)->top)

constexpr LPCWSTR MENUPOPUP_CLASS = L"#32768";
constexpr UINT WM_UAHMEASUREMENUITEM = 0x0094;
constexpr UINT MN_SIZEWINDOW = 0x01E2;
constexpr UINT WM_APPLY_MENU_CORNER_RADIUS = WM_APP + 0x3A11;
constexpr DWORD MFISPOPUP = 0x00000001;

constexpr UINT ENABLE = 1;
constexpr UINT DISABLE = 0;

enum class HighlightColorMode {
    Default,
    Accent,
    Custom,
};

enum class BackgroundEffect {
    None,
    AccentBlurBehind,
    AcrylicSystemBackdrop,
    Mica,
    MicaAlt,
};

struct {
    bool forceClassicMenus = false;
    bool themeBackground = true;
    bool sysColors = true;
    bool accentColorControls = true;
    int popupMenuItemHeight = 32;
    int popupMenuItemWidth = 0;
    HighlightColorMode highlightColorMode = HighlightColorMode::Default;
    COLORREF customHighlightColor = RGB(96, 96, 96);
    COLORREF accentColor = RGB(0, 120, 212);
    BackgroundEffect backgroundEffect = BackgroundEffect::AccentBlurBehind;
    DWORD accentBlurBehindColor = 0x3A232323;
    bool immersiveDarkMode = true;
    int smallRadius = 6;
} g_settings;

std::mutex g_subclassedMenusMutex;
std::unordered_set<HWND> g_subclassedMenus;

using NtUserCreateWindowEx_t = HWND(WINAPI*)(
    DWORD,
    PVOID,
    LPCWSTR,
    PVOID,
    DWORD,
    LONG,
    LONG,
    LONG,
    LONG,
    HWND,
    HMENU,
    HINSTANCE,
    LPVOID,
    DWORD,
    DWORD,
    DWORD,
    VOID*);
NtUserCreateWindowEx_t NtUserCreateWindowEx_Original;

using DrawThemeBackground_t = decltype(&DrawThemeBackground);
DrawThemeBackground_t DrawThemeBackground_Original;

using DrawThemeBackgroundEx_t = decltype(&DrawThemeBackgroundEx);
DrawThemeBackgroundEx_t DrawThemeBackgroundEx_Original;

using GetSysColor_t = decltype(&GetSysColor);
GetSysColor_t GetSysColor_Original;

using GetSysColorBrush_t = decltype(&GetSysColorBrush);
GetSysColorBrush_t GetSysColorBrush_Original;

using DefWindowProcW_t = decltype(&DefWindowProcW);
DefWindowProcW_t DefWindowProcW_Original;

using DefWindowProcA_t = decltype(&DefWindowProcA);
DefWindowProcA_t DefWindowProcA_Original;

using DefFrameProcW_t = decltype(&DefFrameProcW);
DefFrameProcW_t DefFrameProcW_Original;

using DefFrameProcA_t = decltype(&DefFrameProcA);
DefFrameProcA_t DefFrameProcA_Original;

using DefDlgProcW_t = decltype(&DefDlgProcW);
DefDlgProcW_t DefDlgProcW_Original;

using DefDlgProcA_t = decltype(&DefDlgProcA);
DefDlgProcA_t DefDlgProcA_Original;

using DrawImmersiveMenu_t = bool(__fastcall*)(HMENU, HWND);
DrawImmersiveMenu_t ExplorerFrame_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t shell32_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t explorer_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t twinui_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t twinui_pcshell_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t Narrator_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t Taskmgr_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t MoNotificationUx_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t museuxdocked_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t pnidui_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t SecurityHealthSSO_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t SecurityHealthSsoUdk_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t SecurityHealthSystray_DrawImmersiveMenu_Original = nullptr;
DrawImmersiveMenu_t SndVolSSO_DrawImmersiveMenu_Original = nullptr;

std::array<HBRUSH, COLOR_MENUBAR + 1> g_sysColorBrushes{};

enum WINDOWCOMPOSITIONATTRIB {
    WCA_ACCENT_POLICY = 19,
};

struct WINCOMPATTRDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

struct ACCENT_POLICY {
    INT AccentState;
    INT AccentFlags;
    INT GradientColor;
    INT AnimationId;
};

enum ACCENT_STATE {
    ACCENT_STATE_DISABLED,
    ACCENT_STATE_ENABLE_GRADIENT,
    ACCENT_STATE_ENABLE_TRANSPARENTGRADIENT,
    ACCENT_STATE_ENABLE_BLURBEHIND,
    ACCENT_STATE_ENABLE_ACRYLICBLURBEHIND,
    ACCENT_STATE_ENABLE_HOSTBACKDROP,
    ACCENT_STATE_INVALID_STATE,
};

union UAHMENUITEMMETRICS {
    struct {
        DWORD cx;
        DWORD cy;
    } rgsizeBar[2];
    struct {
        DWORD cx;
        DWORD cy;
    } rgsizePopup[4];
};

struct UAHMENUPOPUPMETRICS {
    DWORD rgcx[4];
    DWORD fUpdateMaxWidths : 2;
};

struct UAHMENU {
    HMENU hMenu;
    HDC hdc;
    DWORD dwFlags;
};

struct UAHMENUITEM {
    int iPosition;
    UAHMENUITEMMETRICS uahMenuItemMetrics;
    UAHMENUPOPUPMETRICS uahMenuPopupMetrics;
};

struct UAHMEASUREMENUITEM {
    MEASUREITEMSTRUCT measureItemStruct;
    UAHMENU uahMenu;
    UAHMENUITEM uahMenuItem;
};

LRESULT CALLBACK MenuSubclassProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    DWORD_PTR dwRefData);

LPCWSTR GetCurrentProcessName()
{
    static WCHAR processPath[MAX_PATH];
    static LPCWSTR processName = nullptr;

    if (!processName) {
        GetModuleFileNameW(nullptr, processPath, ARRAYSIZE(processPath));
        processName = wcsrchr(processPath, L'\\');
        processName = processName ? processName + 1 : processPath;
    }

    return processName;
}

bool IsWindows10OrGreater()
{
    using RtlGetVersion_t = LONG(WINAPI*)(OSVERSIONINFOW*);
    static auto RtlGetVersion = reinterpret_cast<RtlGetVersion_t>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));

    if (!RtlGetVersion) {
        return false;
    }

    OSVERSIONINFOW osVersionInfo{sizeof(osVersionInfo)};
    return RtlGetVersion(&osVersionInfo) == 0 &&
           osVersionInfo.dwMajorVersion >= 10;
}

UINT GetSystemDpi()
{
    using GetDpiForSystem_t = decltype(&GetDpiForSystem);
    static auto GetDpiForSystemPtr = reinterpret_cast<GetDpiForSystem_t>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForSystem"));

    if (GetDpiForSystemPtr) {
        return GetDpiForSystemPtr();
    }

    UINT dpi = 96;
    HDC hdc = GetDC(nullptr);
    if (hdc) {
        dpi = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(nullptr, hdc);
    }

    return dpi;
}

UINT GetWindowDpi(HWND hWnd)
{
    using GetDpiForWindow_t = decltype(&GetDpiForWindow);
    static auto GetDpiForWindowPtr = reinterpret_cast<GetDpiForWindow_t>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));

    if (hWnd && GetDpiForWindowPtr) {
        return GetDpiForWindowPtr(hWnd);
    }

    return GetSystemDpi();
}

std::wstring GetWindowClass(HWND hWnd)
{
    WCHAR buffer[256] = {};
    GetClassNameW(hWnd, buffer, ARRAYSIZE(buffer));
    return buffer;
}

bool IsWindowClass(HWND hWnd, LPCWSTR className)
{
    return GetWindowClass(hWnd) == className;
}

bool ParseHexColor(LPCWSTR hexColor, COLORREF& outColor, bool preserveAlpha)
{
    if (!hexColor) {
        return false;
    }

    while (*hexColor == L' ' || *hexColor == L'\t' || *hexColor == L'#') {
        hexColor++;
    }

    size_t len = wcslen(hexColor);
    if (len != 6 && len != 8) {
        return false;
    }

    auto hexToByte = [](WCHAR c) -> int {
        if (c >= L'0' && c <= L'9') {
            return c - L'0';
        }
        if (c >= L'A' && c <= L'F') {
            return 10 + (c - L'A');
        }
        if (c >= L'a' && c <= L'f') {
            return 10 + (c - L'a');
        }
        return -1;
    };

    BYTE alpha = 0xFF;
    if (len == 8) {
        int high = hexToByte(hexColor[0]);
        int low = hexToByte(hexColor[1]);
        if (high < 0 || low < 0) {
            return false;
        }
        alpha = static_cast<BYTE>((high << 4) | low);
        hexColor += 2;
    }

    BYTE rgb[3] = {};
    for (int i = 0; i < 3; i++) {
        int high = hexToByte(hexColor[i * 2]);
        int low = hexToByte(hexColor[i * 2 + 1]);
        if (high < 0 || low < 0) {
            return false;
        }
        rgb[i] = static_cast<BYTE>((high << 4) | low);
    }

    outColor = preserveAlpha
                   ? ((alpha << 24) | (rgb[2] << 16) | (rgb[1] << 8) |
                      rgb[0])
                   : RGB(rgb[0], rgb[1], rgb[2]);
    return true;
}

bool GetAccentColor(COLORREF& outColor)
{
    static const auto GetImmersiveColorFromColorSetEx =
        reinterpret_cast<DWORD(WINAPI*)(DWORD, DWORD, BOOL, DWORD)>(
            GetProcAddress(GetModuleHandleW(L"uxtheme.dll"),
                           MAKEINTRESOURCEA(95)));
    static const auto GetImmersiveColorTypeFromName =
        reinterpret_cast<DWORD(WINAPI*)(LPCWSTR)>(
            GetProcAddress(GetModuleHandleW(L"uxtheme.dll"),
                           MAKEINTRESOURCEA(96)));
    static const auto GetImmersiveUserColorSetPreference =
        reinterpret_cast<DWORD(WINAPI*)(BOOL, BOOL)>(
            GetProcAddress(GetModuleHandleW(L"uxtheme.dll"),
                           MAKEINTRESOURCEA(98)));

    DWORD accentColor = 0;
    BOOL opaque = FALSE;

    if (GetImmersiveColorFromColorSetEx && GetImmersiveColorTypeFromName &&
        GetImmersiveUserColorSetPreference) {
        accentColor = GetImmersiveColorFromColorSetEx(
            GetImmersiveUserColorSetPreference(FALSE, FALSE),
            GetImmersiveColorTypeFromName(L"ImmersiveStartHoverBackground"),
            TRUE,
            0);
        outColor = RGB(accentColor & 0xFF, (accentColor >> 8) & 0xFF,
                       (accentColor >> 16) & 0xFF);
        return true;
    }

    if (SUCCEEDED(DwmGetColorizationColor(&accentColor, &opaque))) {
        outColor = RGB((accentColor >> 16) & 0xFF,
                       (accentColor >> 8) & 0xFF, accentColor & 0xFF);
        return true;
    }

    return false;
}

COLORREF GetMenuHighlightColor()
{
    if (g_settings.highlightColorMode == HighlightColorMode::Accent) {
        if (!g_settings.accentColorControls) {
            return RGB(96, 96, 96);
        }

        COLORREF accentColor;
        if (GetAccentColor(accentColor)) {
            g_settings.accentColor = accentColor;
            return accentColor;
        }
    }

    if (g_settings.highlightColorMode == HighlightColorMode::Custom) {
        return g_settings.customHighlightColor;
    }

    return RGB(96, 96, 96);
}

COLORREF WINAPI GetSysColor_Hook(int index)
{
    if (!g_settings.sysColors) {
        return GetSysColor_Original(index);
    }

    switch (index) {
        case COLOR_MENU:
        case COLOR_MENUBAR:
            return g_settings.backgroundEffect == BackgroundEffect::None
                       ? GetSysColor_Original(index)
                       : RGB(0, 0, 0);

        case COLOR_MENUTEXT:
            return RGB(220, 220, 220);

        case COLOR_HIGHLIGHT:
        case COLOR_MENUHILIGHT:
            if (g_settings.highlightColorMode == HighlightColorMode::Default) {
                return GetSysColor_Original(index);
            }
            return GetMenuHighlightColor();

        case COLOR_HIGHLIGHTTEXT:
            return RGB(255, 255, 255);
    }

    return GetSysColor_Original(index);
}

HBRUSH WINAPI GetSysColorBrush_Hook(int index)
{
    COLORREF color = GetSysColor_Hook(index);

    if (index < 0 ||
        index >= static_cast<int>(g_sysColorBrushes.size()) ||
        !g_settings.sysColors) {
        return GetSysColorBrush_Original(index);
    }

    if (!g_sysColorBrushes[index]) {
        g_sysColorBrushes[index] = CreateSolidBrush(color);
    }

    return g_sysColorBrushes[index];
}

std::wstring GetThemeClass(HTHEME hTheme)
{
    using GetThemeClass_t = HRESULT(WINAPI*)(HTHEME, LPCTSTR, int);
    static auto GetThemeClassPtr = reinterpret_cast<GetThemeClass_t>(
        GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(74)));

    if (!GetThemeClassPtr) {
        return L"";
    }

    WCHAR buffer[256] = {};
    HRESULT hr = GetThemeClassPtr(hTheme, buffer, ARRAYSIZE(buffer));
    return SUCCEEDED(hr) ? buffer : L"";
}

void DrawSolidMenuHighlight(HDC hdc, LPCRECT rect)
{
    RECT fillRect = *rect;
    if (RECTWIDTH(&fillRect) > 6) {
        InflateRect(&fillRect, -2, 0);
    }

    COLORREF color = GetMenuHighlightColor();
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    int radius = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 96);
    RoundRect(hdc, fillRect.left, fillRect.top, fillRect.right,
              fillRect.bottom, radius, radius);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

bool IsMenuPopupHotItem(int partId, int stateId)
{
    return (partId == MENU_POPUPITEM || partId == 27) &&
           (stateId == MPI_HOT || stateId == 2);
}

bool IsMenuTransparentBackgroundPart(int partId, int stateId)
{
    if (g_settings.backgroundEffect == BackgroundEffect::None) {
        return false;
    }

    return partId == MENU_POPUPBACKGROUND || partId == MENU_POPUPBORDERS ||
           partId == MENU_POPUPGUTTER ||
           ((partId == MENU_POPUPITEM || partId == 27) &&
            !(stateId == MPI_HOT || stateId == 2));
}

void DrawMenuSeparator(HDC hdc, LPCRECT rect)
{
    RECT line = *rect;
    int y = line.top + RECTHEIGHT(&line) / 2;
    COLORREF color = RGB(96, 96, 96);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    MoveToEx(hdc, line.left + 24, y, nullptr);
    LineTo(hdc, line.right - 8, y);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

HRESULT WINAPI DrawThemeBackground_Hook(HTHEME hTheme,
                                        HDC hdc,
                                        int partId,
                                        int stateId,
                                        LPCRECT rect,
                                        LPCRECT clipRect)
{
    if (!g_settings.themeBackground) {
        return DrawThemeBackground_Original(hTheme, hdc, partId, stateId, rect,
                                            clipRect);
    }

    std::wstring themeClass = GetThemeClass(hTheme);
    if (themeClass != L"Menu") {
        return DrawThemeBackground_Original(hTheme, hdc, partId, stateId, rect,
                                            clipRect);
    }

    RECT drawRect = *rect;
    if (clipRect) {
        IntersectRect(&drawRect, rect, clipRect);
    }

    if (IsMenuPopupHotItem(partId, stateId)) {
        if (g_settings.highlightColorMode == HighlightColorMode::Default) {
            return DrawThemeBackground_Original(hTheme, hdc, partId, stateId,
                                                rect, clipRect);
        }
        DrawSolidMenuHighlight(hdc, &drawRect);
        return S_OK;
    }

    if (partId == MENU_POPUPSEPARATOR) {
        DrawMenuSeparator(hdc, &drawRect);
        return S_OK;
    }

    if (IsMenuTransparentBackgroundPart(partId, stateId)) {
        FillRect(hdc, &drawRect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        return S_OK;
    }

    return DrawThemeBackground_Original(hTheme, hdc, partId, stateId, rect,
                                        clipRect);
}

HRESULT WINAPI DrawThemeBackgroundEx_Hook(HTHEME hTheme,
                                          HDC hdc,
                                          int partId,
                                          int stateId,
                                          LPCRECT rect,
                                          const DTBGOPTS* options)
{
    if (!g_settings.themeBackground) {
        return DrawThemeBackgroundEx_Original(hTheme, hdc, partId, stateId,
                                              rect, options);
    }

    std::wstring themeClass = GetThemeClass(hTheme);
    if (themeClass == L"Menu") {
        RECT drawRect = *rect;
        if (options && (options->dwFlags & DTBG_CLIPRECT)) {
            IntersectRect(&drawRect, rect, &options->rcClip);
        }

        if (IsMenuPopupHotItem(partId, stateId)) {
            if (g_settings.highlightColorMode == HighlightColorMode::Default) {
                return DrawThemeBackgroundEx_Original(hTheme, hdc, partId,
                                                      stateId, rect, options);
            }
            DrawSolidMenuHighlight(hdc, &drawRect);
            return S_OK;
        }

        if (partId == MENU_POPUPSEPARATOR) {
            DrawMenuSeparator(hdc, &drawRect);
            return S_OK;
        }

        if (IsMenuTransparentBackgroundPart(partId, stateId)) {
            FillRect(hdc, &drawRect,
                     static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
            return S_OK;
        }
    }

    return DrawThemeBackgroundEx_Original(hTheme, hdc, partId, stateId, rect,
                                          options);
}

void TriggerWindowNCRendering(HWND hWnd)
{
    DefWindowProcW(hWnd, WM_NCACTIVATE, TRUE, 0);
    SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_DRAWFRAME | SWP_NOACTIVATE);
}

void DwmMakeWindowTransparent(HWND hWnd)
{
    DWM_BLURBEHIND blurBehind{
        DWM_BB_ENABLE | DWM_BB_BLURREGION | DWM_BB_TRANSITIONONMAXIMIZED,
        TRUE,
        CreateRectRgn(0, 0, -1, -1),
        TRUE};
    DwmEnableBlurBehindWindow(hWnd, &blurBehind);
    DeleteObject(blurBehind.hRgnBlur);
}

void EnableBlurBehind(HWND hWnd)
{
    DWM_BLURBEHIND blurBehind{};
    blurBehind.dwFlags =
        DWM_BB_ENABLE | DWM_BB_BLURREGION | DWM_BB_TRANSITIONONMAXIMIZED;
    blurBehind.fEnable = TRUE;
    blurBehind.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
    blurBehind.fTransitionOnMaximized = TRUE;
    DwmEnableBlurBehindWindow(hWnd, &blurBehind);
    DeleteObject(blurBehind.hRgnBlur);

    ACCENT_POLICY accent{};
    accent.AccentState = ACCENT_STATE_ENABLE_ACRYLICBLURBEHIND;
    accent.GradientColor = g_settings.accentBlurBehindColor;

    WINCOMPATTRDATA attrib{};
    attrib.Attrib = WCA_ACCENT_POLICY;
    attrib.pvData = &accent;
    attrib.cbData = sizeof(accent);

    using SetWindowCompositionAttribute_t =
        BOOL(WINAPI*)(HWND, WINCOMPATTRDATA*);
    auto SetWindowCompositionAttribute =
        reinterpret_cast<SetWindowCompositionAttribute_t>(GetProcAddress(
            GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
    if (SetWindowCompositionAttribute) {
        SetWindowCompositionAttribute(hWnd, &attrib);
    }
}

void SetSystemBackdrop(HWND hWnd, UINT type)
{
    DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &type, sizeof(type));
}

void RestoreMenuEffects(HWND hWnd)
{
    if (!IsWindow(hWnd) || !IsWindowClass(hWnd, MENUPOPUP_CLASS)) {
        return;
    }

    UINT backdrop = DWMSBT_AUTO;
    DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop,
                          sizeof(backdrop));

    UINT corner = DWMWCP_DEFAULT;
    DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner,
                          sizeof(corner));

    UINT darkMode = DISABLE;
    DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode,
                          sizeof(darkMode));

    ACCENT_POLICY accent{};
    accent.AccentState = ACCENT_STATE_DISABLED;

    WINCOMPATTRDATA attrib{};
    attrib.Attrib = WCA_ACCENT_POLICY;
    attrib.pvData = &accent;
    attrib.cbData = sizeof(accent);

    using SetWindowCompositionAttribute_t =
        BOOL(WINAPI*)(HWND, WINCOMPATTRDATA*);
    auto SetWindowCompositionAttribute =
        reinterpret_cast<SetWindowCompositionAttribute_t>(GetProcAddress(
            GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
    if (SetWindowCompositionAttribute) {
        SetWindowCompositionAttribute(hWnd, &attrib);
    }

    SetWindowRgn(hWnd, nullptr, TRUE);
}

void ApplyMenuCornerRadius(HWND hWnd)
{
    if (g_settings.smallRadius < 0) {
        return;
    }

    RECT windowRect{};
    if (!GetWindowRect(hWnd, &windowRect)) {
        return;
    }

    int width = RECTWIDTH(&windowRect);
    int height = RECTHEIGHT(&windowRect);
    if (width <= 0 || height <= 0) {
        return;
    }

    UINT dpi = GetWindowDpi(hWnd);
    int radius = MulDiv(g_settings.smallRadius, dpi, 96);
    int diameter = std::max(1, radius * 2 + 1);

    HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1, diameter,
                                     diameter);
    if (!region) {
        return;
    }

    if (!SetWindowRgn(hWnd, region, TRUE)) {
        DeleteObject(region);
    }
    // SetWindowRgn owns the region on success.
}

void ScheduleMenuCornerRadius(HWND hWnd)
{
    if (g_settings.smallRadius >= 0) {
        PostMessageW(hWnd, WM_APPLY_MENU_CORNER_RADIUS, 0, 0);
    }
}

void ApplyMenuEffects(HWND hWnd)
{
    if (!IsWindow(hWnd) || !IsWindowClass(hWnd, MENUPOPUP_CLASS)) {
        return;
    }

    if (g_settings.immersiveDarkMode) {
        DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &ENABLE,
                              sizeof(ENABLE));
    }

    switch (g_settings.backgroundEffect) {
        case BackgroundEffect::AccentBlurBehind:
            EnableBlurBehind(hWnd);
            break;

        case BackgroundEffect::AcrylicSystemBackdrop:
            DwmMakeWindowTransparent(hWnd);
            TriggerWindowNCRendering(hWnd);
            SetSystemBackdrop(hWnd, DWMSBT_TRANSIENTWINDOW);
            break;

        case BackgroundEffect::Mica:
            DwmMakeWindowTransparent(hWnd);
            TriggerWindowNCRendering(hWnd);
            SetSystemBackdrop(hWnd, DWMSBT_MAINWINDOW);
            break;

        case BackgroundEffect::MicaAlt:
            DwmMakeWindowTransparent(hWnd);
            TriggerWindowNCRendering(hWnd);
            SetSystemBackdrop(hWnd, DWMSBT_TABBEDWINDOW);
            break;

        case BackgroundEffect::None:
            break;
    }

    if (g_settings.smallRadius >= 0 &&
        g_settings.backgroundEffect != BackgroundEffect::None) {
        UINT corner = DWMWCP_ROUNDSMALL;
        DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner,
                              sizeof(corner));
    }

    ScheduleMenuCornerRadius(hWnd);
}

void AddMenuSubclass(HWND hWnd)
{
    if (!IsWindow(hWnd) || !IsWindowClass(hWnd, MENUPOPUP_CLASS)) {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(g_subclassedMenusMutex);
        if (g_subclassedMenus.find(hWnd) != g_subclassedMenus.end()) {
            return;
        }
    }

    if (WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd, MenuSubclassProc,
                                                      0)) {
        std::lock_guard<std::mutex> guard(g_subclassedMenusMutex);
        g_subclassedMenus.insert(hWnd);
    }
}

void RemoveMenuSubclass(HWND hWnd)
{
    std::lock_guard<std::mutex> guard(g_subclassedMenusMutex);
    g_subclassedMenus.erase(hWnd);
}

LRESULT CALLBACK MenuSubclassProc(HWND hWnd,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam,
                                  DWORD_PTR dwRefData)
{
    switch (uMsg) {
        case MN_SIZEWINDOW:
            ApplyMenuEffects(hWnd);
            break;

        case WM_WINDOWPOSCHANGED:
        {
            auto windowPos = reinterpret_cast<WINDOWPOS*>(lParam);
            if (windowPos &&
                !(windowPos->flags & SWP_NOSIZE) &&
                !(windowPos->flags & SWP_HIDEWINDOW)) {
                ScheduleMenuCornerRadius(hWnd);
            }
            break;
        }

        case WM_SHOWWINDOW:
            if (wParam) {
                ApplyMenuEffects(hWnd);
            }
            break;

        case WM_APPLY_MENU_CORNER_RADIUS:
            ApplyMenuCornerRadius(hWnd);
            return 0;

        case WM_NCDESTROY:
            RemoveMenuSubclass(hWnd);
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

HWND WINAPI NtUserCreateWindowEx_Hook(DWORD exStyle,
                                      PVOID unsafeClassName,
                                      LPCWSTR versionedClass,
                                      PVOID unsafeWindowName,
                                      DWORD style,
                                      LONG x,
                                      LONG y,
                                      LONG width,
                                      LONG height,
                                      HWND parent,
                                      HMENU menu,
                                      HINSTANCE instance,
                                      LPVOID param,
                                      DWORD showMode,
                                      DWORD unknown1,
                                      DWORD unknown2,
                                      VOID* unknown3)
{
    HWND hWnd = NtUserCreateWindowEx_Original(
        exStyle, unsafeClassName, versionedClass, unsafeWindowName, style, x, y,
        width, height, parent, menu, instance, param, showMode, unknown1,
        unknown2, unknown3);

    if (hWnd) {
        if (IsWindowClass(hWnd, MENUPOPUP_CLASS)) {
            AddMenuSubclass(hWnd);
        }
    }

    return hWnd;
}

BOOL CALLBACK ExistingWindowsEnumProc(HWND hWnd, LPARAM lParam)
{
    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);
    if (processId == GetCurrentProcessId()) {
        if (IsWindowClass(hWnd, MENUPOPUP_CLASS)) {
            AddMenuSubclass(hWnd);
            ApplyMenuEffects(hWnd);
        }
    }

    return TRUE;
}

void AdjustUahMenuItemMetrics(HWND hWnd, LPARAM lParam)
{
    auto* measure = reinterpret_cast<UAHMEASUREMENUITEM*>(lParam);
    if (!measure || !(measure->uahMenu.dwFlags & MFISPOPUP)) {
        return;
    }

    bool isSeparator = false;
    MENUITEMINFOW menuItemInfo{sizeof(menuItemInfo)};
    menuItemInfo.fMask = MIIM_FTYPE;
    if (GetMenuItemInfoW(measure->uahMenu.hMenu, measure->uahMenuItem.iPosition,
                         TRUE, &menuItemInfo)) {
        isSeparator = (menuItemInfo.fType & MFT_SEPARATOR) != 0;
    }

    UINT dpi = GetWindowDpi(hWnd);

    if (!isSeparator && g_settings.popupMenuItemHeight > 0) {
        UINT targetHeight = MulDiv(g_settings.popupMenuItemHeight, dpi, 96);
        if (measure->measureItemStruct.itemHeight < targetHeight) {
            measure->measureItemStruct.itemHeight = targetHeight;
        }
    }

    if (g_settings.popupMenuItemWidth > 0) {
        UINT targetWidth = MulDiv(g_settings.popupMenuItemWidth, dpi, 96);
        if (measure->measureItemStruct.itemWidth < targetWidth) {
            measure->measureItemStruct.itemWidth = targetWidth;
        }
    }
}

LRESULT CALLBACK DefWindowProcW_Hook(HWND hWnd,
                                     UINT msg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
    LRESULT result = DefWindowProcW_Original(hWnd, msg, wParam, lParam);
    if (msg == WM_UAHMEASUREMENUITEM) {
        AdjustUahMenuItemMetrics(hWnd, lParam);
    }
    return result;
}

LRESULT CALLBACK DefWindowProcA_Hook(HWND hWnd,
                                     UINT msg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
    LRESULT result = DefWindowProcA_Original(hWnd, msg, wParam, lParam);
    if (msg == WM_UAHMEASUREMENUITEM) {
        AdjustUahMenuItemMetrics(hWnd, lParam);
    }
    return result;
}

LRESULT CALLBACK DefFrameProcW_Hook(HWND hWnd,
                                    HWND mdiClient,
                                    UINT msg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    LRESULT result =
        DefFrameProcW_Original(hWnd, mdiClient, msg, wParam, lParam);
    if (msg == WM_UAHMEASUREMENUITEM) {
        AdjustUahMenuItemMetrics(hWnd, lParam);
    }
    return result;
}

LRESULT CALLBACK DefFrameProcA_Hook(HWND hWnd,
                                    HWND mdiClient,
                                    UINT msg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    LRESULT result =
        DefFrameProcA_Original(hWnd, mdiClient, msg, wParam, lParam);
    if (msg == WM_UAHMEASUREMENUITEM) {
        AdjustUahMenuItemMetrics(hWnd, lParam);
    }
    return result;
}

LRESULT CALLBACK DefDlgProcW_Hook(HWND hWnd,
                                  UINT msg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
    LRESULT result = DefDlgProcW_Original(hWnd, msg, wParam, lParam);
    if (msg == WM_UAHMEASUREMENUITEM) {
        AdjustUahMenuItemMetrics(hWnd, lParam);
    }
    return result;
}

LRESULT CALLBACK DefDlgProcA_Hook(HWND hWnd,
                                  UINT msg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
    LRESULT result = DefDlgProcA_Original(hWnd, msg, wParam, lParam);
    if (msg == WM_UAHMEASUREMENUITEM) {
        AdjustUahMenuItemMetrics(hWnd, lParam);
    }
    return result;
}

bool __fastcall DrawImmersiveMenu_Hook(HMENU hPopupMenu, HWND hWnd)
{
    return false;
}

bool ApplyImmersiveMenuHook(LPCWSTR moduleName, DrawImmersiveMenu_t* original)
{
    HMODULE module = moduleName
                         ? LoadLibraryExW(moduleName, nullptr,
                                          LOAD_LIBRARY_SEARCH_SYSTEM32)
                         : GetModuleHandleW(nullptr);

    LPCWSTR targetName = moduleName ? moduleName : GetCurrentProcessName();
    if (!module) {
        Wh_Log(L"Failed to load %s", targetName);
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{DRAW_IMMERSIVE_MENU}, original, DrawImmersiveMenu_Hook, true},
    };

    if (!WindhawkUtils::HookSymbols(module, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"Failed to hook immersive menu helper in %s", targetName);
        return false;
    }

    return true;
}

void ApplyClassicMenuHooks()
{
    if (!g_settings.forceClassicMenus || !IsWindows10OrGreater()) {
        return;
    }

    LPCWSTR processName = GetCurrentProcessName();
    bool shouldExcludeProcess =
        _wcsicmp(processName, L"windhawk.exe") == 0 ||
        _wcsicmp(processName, L"consent.exe") == 0 ||
        _wcsicmp(processName, L"dwm.exe") == 0 ||
        _wcsicmp(processName, L"SearchIndexer.exe") == 0 ||
        _wcsicmp(processName, L"ShellExperienceHost.exe") == 0 ||
        _wcsicmp(processName, L"svchost.exe") == 0 ||
        _wcsicmp(processName, L"wlanext.exe") == 0;
    if (shouldExcludeProcess) {
        return;
    }

    if (_wcsicmp(processName, L"SecurityHealthSystray.exe") != 0 &&
        _wcsicmp(processName, L"MoNotificationUx.exe") != 0) {
        ApplyImmersiveMenuHook(L"ExplorerFrame.dll",
                               &ExplorerFrame_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"shell32.dll",
                               &shell32_DrawImmersiveMenu_Original);
    }

    if (_wcsicmp(processName, L"explorer.exe") == 0) {
        ApplyImmersiveMenuHook(nullptr, &explorer_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"pnidui.dll", &pnidui_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"SndVolSSO.dll",
                               &SndVolSSO_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"twinui.dll", &twinui_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"twinui.pcshell.dll",
                               &twinui_pcshell_DrawImmersiveMenu_Original);
    } else if (_wcsicmp(processName, L"MoNotificationUx.exe") == 0) {
        ApplyImmersiveMenuHook(nullptr,
                               &MoNotificationUx_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"museuxdocked.dll",
                               &museuxdocked_DrawImmersiveMenu_Original);
    } else if (_wcsicmp(processName, L"Narrator.exe") == 0) {
        ApplyImmersiveMenuHook(nullptr, &Narrator_DrawImmersiveMenu_Original);
    } else if (_wcsicmp(processName, L"SecurityHealthSystray.exe") == 0) {
        ApplyImmersiveMenuHook(nullptr,
                               &SecurityHealthSystray_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"SecurityHealthSSO.dll",
                               &SecurityHealthSSO_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"SecurityHealthSsoUdk.dll",
                               &SecurityHealthSsoUdk_DrawImmersiveMenu_Original);
    } else if (_wcsicmp(processName, L"Taskmgr.exe") == 0) {
        ApplyImmersiveMenuHook(nullptr, &Taskmgr_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"twinui.dll", &twinui_DrawImmersiveMenu_Original);
        ApplyImmersiveMenuHook(L"twinui.pcshell.dll",
                               &twinui_pcshell_DrawImmersiveMenu_Original);
    }
}

void LoadSettings()
{
    g_settings.forceClassicMenus = Wh_GetIntSetting(L"forceClassicMenus");
    g_settings.themeBackground =
        Wh_GetIntSetting(L"RenderingMod.ThemeBackground");
    g_settings.sysColors = Wh_GetIntSetting(L"RenderingMod.SysColors");
    g_settings.accentColorControls =
        Wh_GetIntSetting(L"RenderingMod.AccentColorControls");

    g_settings.popupMenuItemHeight = Wh_GetIntSetting(L"popupMenuItemHeight");
    if (g_settings.popupMenuItemHeight != 0) {
        g_settings.popupMenuItemHeight =
            std::max(g_settings.popupMenuItemHeight, 22);
    }

    g_settings.popupMenuItemWidth = Wh_GetIntSetting(L"popupMenuItemWidth");
    if (g_settings.popupMenuItemWidth < 0) {
        g_settings.popupMenuItemWidth = 0;
    }

    auto colorMode =
        WindhawkUtils::StringSetting(Wh_GetStringSetting(L"highlightColorMode"));
    if (wcscmp(colorMode.get(), L"accent") == 0) {
        g_settings.highlightColorMode = HighlightColorMode::Accent;
    } else if (wcscmp(colorMode.get(), L"custom") == 0) {
        g_settings.highlightColorMode = HighlightColorMode::Custom;
    } else {
        g_settings.highlightColorMode = HighlightColorMode::Default;
    }

    COLORREF customHighlightColor;
    auto customHighlightColorText = WindhawkUtils::StringSetting(
        Wh_GetStringSetting(L"customHighlightColor"));
    if (ParseHexColor(customHighlightColorText.get(), customHighlightColor,
                      false)) {
        g_settings.customHighlightColor = customHighlightColor;
    }

    GetAccentColor(g_settings.accentColor);

    auto effect =
        WindhawkUtils::StringSetting(Wh_GetStringSetting(L"backgroundEffect"));
    if (wcscmp(effect.get(), L"acrylicblur") == 0) {
        g_settings.backgroundEffect = BackgroundEffect::AccentBlurBehind;
    } else if (wcscmp(effect.get(), L"acrylicsystem") == 0) {
        g_settings.backgroundEffect = BackgroundEffect::AcrylicSystemBackdrop;
    } else if (wcscmp(effect.get(), L"mica") == 0) {
        g_settings.backgroundEffect = BackgroundEffect::Mica;
    } else if (wcscmp(effect.get(), L"mica_tabbed") == 0) {
        g_settings.backgroundEffect = BackgroundEffect::MicaAlt;
    } else {
        g_settings.backgroundEffect = BackgroundEffect::None;
    }

    COLORREF accentBlurBehindColor;
    auto accentBlurBehindColorText = WindhawkUtils::StringSetting(
        Wh_GetStringSetting(L"accentBlurBehindColor"));
    if (ParseHexColor(accentBlurBehindColorText.get(), accentBlurBehindColor,
                      true)) {
        g_settings.accentBlurBehindColor = accentBlurBehindColor;
    }

    g_settings.immersiveDarkMode = Wh_GetIntSetting(L"immersiveDarkMode");
    g_settings.smallRadius = Wh_GetIntSetting(L"smallRadius");
}

BOOL Wh_ModInit()
{
    Wh_Log(L"Init");

    LoadSettings();

    WindhawkUtils::SetFunctionHook(DefWindowProcW, DefWindowProcW_Hook,
                                   &DefWindowProcW_Original);
    WindhawkUtils::SetFunctionHook(DefWindowProcA, DefWindowProcA_Hook,
                                   &DefWindowProcA_Original);
    WindhawkUtils::SetFunctionHook(DefFrameProcW, DefFrameProcW_Hook,
                                   &DefFrameProcW_Original);
    WindhawkUtils::SetFunctionHook(DefFrameProcA, DefFrameProcA_Hook,
                                   &DefFrameProcA_Original);
    WindhawkUtils::SetFunctionHook(DefDlgProcW, DefDlgProcW_Hook,
                                   &DefDlgProcW_Original);
    WindhawkUtils::SetFunctionHook(DefDlgProcA, DefDlgProcA_Hook,
                                   &DefDlgProcA_Original);

    WindhawkUtils::SetFunctionHook(DrawThemeBackground,
                                   DrawThemeBackground_Hook,
                                   &DrawThemeBackground_Original);
    WindhawkUtils::SetFunctionHook(DrawThemeBackgroundEx,
                                   DrawThemeBackgroundEx_Hook,
                                   &DrawThemeBackgroundEx_Original);
    WindhawkUtils::SetFunctionHook(GetSysColor, GetSysColor_Hook,
                                   &GetSysColor_Original);
    WindhawkUtils::SetFunctionHook(GetSysColorBrush, GetSysColorBrush_Hook,
                                   &GetSysColorBrush_Original);

    HMODULE win32u = GetModuleHandleW(L"win32u.dll");
    if (win32u) {
        auto NtUserCreateWindowExPtr =
            reinterpret_cast<NtUserCreateWindowEx_t>(
                GetProcAddress(win32u, "NtUserCreateWindowEx"));
        if (NtUserCreateWindowExPtr) {
            WindhawkUtils::SetFunctionHook(NtUserCreateWindowExPtr,
                                           NtUserCreateWindowEx_Hook,
                                           &NtUserCreateWindowEx_Original);
        }
    }

    ApplyClassicMenuHooks();

    return TRUE;
}

void Wh_ModAfterInit()
{
    EnumWindows(ExistingWindowsEnumProc, 0);
}

void Wh_ModUninit()
{
    Wh_Log(L"Uninit");

    std::unordered_set<HWND> subclassedMenus;
    {
        std::lock_guard<std::mutex> guard(g_subclassedMenusMutex);
        subclassedMenus = std::move(g_subclassedMenus);
        g_subclassedMenus.clear();
    }

    for (HWND hWnd : subclassedMenus) {
        RestoreMenuEffects(hWnd);
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
                                                         MenuSubclassProc);
    }

    for (HBRUSH brush : g_sysColorBrushes) {
        if (brush) {
            DeleteObject(brush);
        }
    }
}

BOOL Wh_ModSettingsChanged(BOOL* reload)
{
    *reload = TRUE;
    return TRUE;
}
