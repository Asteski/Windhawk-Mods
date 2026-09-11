// ==WindhawkMod==
// @id              taskbar-background-helper-plus
// @name            Taskbar Background Helper Plus
// @description     Sets the taskbar background for the transparent parts, always or when selected window conditions are met, designed to be used with Windows 11 Taskbar Styler
// @version         1.0
// @author          m417z
// @github          https://github.com/m417z
// @twitter         https://twitter.com/m417z
// @homepage        https://m417z.com/
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -ldwmapi -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// Source code is published under The GNU General Public License v3.0.
//
// For bug reports and feature requests, please open an issue here:
// https://github.com/ramensoftware/windhawk-mods/issues
//
// For pull requests, development takes place here:
// https://github.com/m417z/my-windhawk-mods

// ==WindhawkModReadme==
/*
# Taskbar Background Helper Plus

Sets the taskbar background for the transparent parts, always or when selected
window conditions are met, designed to be used with [Windows 11 Taskbar
Styler](https://windhawk.net/mods/windows-11-taskbar-styler).

The Default background style restores the regular Windows taskbar background.
The Transparent background style removes this mod's backdrop effect and the
Windows 11 taskbar fill and border rectangles using these control styles:

```
- target: Windows.UI.Xaml.Shapes.Rectangle#BackgroundStroke
  styles:
  - Fill=Transparent
- target: Taskbar.TaskbarFrame > Grid#RootGrid > Taskbar.TaskbarBackground > Grid > Rectangle#BackgroundFill
  styles:
  - Fill=Transparent
```

Also, Windows 11 Taskbar Styler has [a known
limitation](https://github.com/ramensoftware/windhawk-mods/issues/742) which
makes some styles only work if there's a single taskbar. This mod can be used as
a workaround.

![Demonstration](https://i.imgur.com/lMp8OLp.gif)
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- backgroundStyle: blur
  $name: Background style
  $description: >-
    Default restores the regular Windows taskbar background.
  $options:
  - default: Default
  - blur: Blur
  - acrylicBlur: Acrylic blur
  - color: Color
  - transparent: Transparent
- color: "#80FF7F27"
  $name: Custom color
  $description: ARGB hex color in #AARRGGBB format. Alpha controls transparency.
- applyWhen: maximized
  $name: Apply when
  $options:
  - always: Always
  - maximized: Only when maximized
  - dynamic: Dynamic
  $description: >-
    Only when maximized applies the style when a window is maximized or
    fullscreen. Dynamic applies the style when two or more windows fully touch
    the taskbar.
- excludedPrograms: [""]
  $name: Excluded programs
  $description: >-
    These programs will be ignored when checking window conditions.

    Entries can be process names, paths or application IDs, for example:

    mspaint.exe

    C:\Windows\System32\notepad.exe

    Microsoft.WindowsCalculator_8wekyb3d8bbwe!App
- styleForDarkMode:
  - use: false
    $name: Use a separate background for dark mode
  - backgroundStyle: blur
    $name: Background style
    $description: >-
      Default restores the regular Windows taskbar background.
    $options:
    - default: Default
    - blur: Blur
    - acrylicBlur: Acrylic blur
    - color: Color
    - transparent: Transparent
  - color: "#80FF7F27"
    $name: Custom color
    $description: ARGB hex color in #AARRGGBB format. Alpha controls transparency.
  $name: Dark mode
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <dwmapi.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.h>
#include <Unknwn.h>
#include <ocidl.h>
#include <xamlom.h>

#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class BackgroundStyle {
    defaultStyle,
    blur,
    acrylicBlur,
    color,
    transparent,
};

enum class ApplyWhen {
    always,
    maximized,
    dynamic,
};

namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxm = winrt::Windows::UI::Xaml::Media;
namespace wuxs = winrt::Windows::UI::Xaml::Shapes;

struct TaskbarStyle {
    BackgroundStyle backgroundStyle;
    COLORREF color;
};

struct {
    TaskbarStyle style;
    ApplyWhen applyWhen;
    std::unordered_set<std::wstring> excludedPrograms;
    std::optional<TaskbarStyle> darkModeStyle;
} g_settings;

std::mutex g_winEventHookThreadMutex;
std::atomic<HANDLE> g_winEventHookThread;

#if __cplusplus < 202302L
// Missing in older MinGW headers.
DECLARE_HANDLE(CO_MTA_USAGE_COOKIE);
WINOLEAPI CoIncrementMTAUsage(CO_MTA_USAGE_COOKIE* pCookie);
WINOLEAPI CoDecrementMTAUsage(CO_MTA_USAGE_COOKIE Cookie);
#endif

// Missing in older MinGW headers.
#ifndef EVENT_OBJECT_CLOAKED
#define EVENT_OBJECT_CLOAKED 0x8017
#endif
#ifndef EVENT_OBJECT_UNCLOAKED
#define EVENT_OBJECT_UNCLOAKED 0x8018
#endif

// Aero Peek events (Show desktop preview).
// Reference:
// https://github.com/TranslucentTB/TranslucentTB/blob/9cfa9eeed5c264f33c8f005512a6649124a69845/Common/undoc/winuser.hpp
static constexpr DWORD EVENT_SYSTEM_PEEKSTART = 0x0021;
static constexpr DWORD EVENT_SYSTEM_PEEKEND = 0x0022;

enum WINDOWCOMPOSITIONATTRIB {
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_CORNER_STYLE = 27,
    WCA_PART_COLOR = 28,
    WCA_DISABLE_MOVESIZE_FEEDBACK = 29,
    WCA_SYSTEMBACKDROP_TYPE = 30,
    WCA_SET_TAGGED_WINDOW_RECT = 31,
    WCA_CLEAR_TAGGED_WINDOW_RECT = 32,
    WCA_REMOTEAPP_POLICY = 33,
    WCA_HAS_ACCENT_POLICY = 34,
    WCA_REDIRECTIONBITMAP_FILL_COLOR = 35,
    WCA_REDIRECTIONBITMAP_ALPHA = 36,
    WCA_BORDER_MARGINS = 37,
    WCA_LAST = 38,
};

// Affects the rendering of the background of a window.
enum ACCENT_STATE {
    // Default value. Background is black.
    ACCENT_DISABLED = 0,
    // Background is GradientColor, alpha channel ignored.
    ACCENT_ENABLE_GRADIENT = 1,
    // Background is GradientColor.
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    // Background is GradientColor, with blur effect.
    ACCENT_ENABLE_BLURBEHIND = 3,
    // Background is GradientColor, with acrylic blur effect.
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    // Allows desktop apps to use Compositor.CreateHostBackdropBrush
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    // Unknown. Seems to draw background fully transparent.
    ACCENT_INVALID_STATE = 6,
};

struct ACCENTPOLICY {
    ACCENT_STATE accentState;
    UINT accentFlags;
    COLORREF gradientColor;
    LONG animationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB attrib;
    void* pvData;
    UINT cbData;
};

using SetWindowCompositionAttribute_t =
    BOOL(WINAPI*)(HWND hWnd, const WINDOWCOMPOSITIONATTRIBDATA* pAttrData);
SetWindowCompositionAttribute_t SetWindowCompositionAttribute_Original;

// Private API for window band (z-order band).
// https://blog.adeltax.com/window-z-order-in-windows-10/
using GetWindowBand_t = BOOL(WINAPI*)(HWND hWnd, PDWORD pdwBand);
GetWindowBand_t pGetWindowBand;

// https://stackoverflow.com/a/51336913
bool IsWindowsDarkModeEnabled() {
    constexpr WCHAR kSubKeyPath[] =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";

    DWORD value = 0;
    DWORD valueSize = sizeof(value);
    LONG result =
        RegGetValue(HKEY_CURRENT_USER, kSubKeyPath, L"AppsUseLightTheme",
                    RRF_RT_REG_DWORD, nullptr, &value, &valueSize);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    return value == 0;
}

int HexDigitValue(WCHAR ch) {
    if (ch >= L'0' && ch <= L'9') {
        return ch - L'0';
    }

    if (ch >= L'a' && ch <= L'f') {
        return ch - L'a' + 10;
    }

    if (ch >= L'A' && ch <= L'F') {
        return ch - L'A' + 10;
    }

    return -1;
}

bool TryParseArgbColor(PCWSTR value, COLORREF* color) {
    while (*value == L' ' || *value == L'\t') {
        value++;
    }

    if (*value == L'#') {
        value++;
    } else if (value[0] == L'0' &&
               (value[1] == L'x' || value[1] == L'X')) {
        value += 2;
    }

    DWORD argb = 0;
    for (int i = 0; i < 8; i++) {
        int digit = HexDigitValue(value[i]);
        if (digit < 0) {
            return false;
        }

        argb = (argb << 4) | static_cast<DWORD>(digit);
    }

    if (value[8] != L'\0') {
        return false;
    }

    BYTE alpha = (argb >> 24) & 0xFF;
    BYTE red = (argb >> 16) & 0xFF;
    BYTE green = (argb >> 8) & 0xFF;
    BYTE blue = argb & 0xFF;

    *color = static_cast<COLORREF>(red | (green << 8) | (blue << 16) |
                                   (alpha << 24));
    return true;
}

COLORREF ReadArgbColorSetting(PCWSTR settingName) {
    static constexpr COLORREF kDefaultColor =
        static_cast<COLORREF>(0x80277FFF);

    PCWSTR value = Wh_GetStringSetting(settingName);
    COLORREF color = kDefaultColor;
    if (!TryParseArgbColor(value, &color)) {
        Wh_Log(L"Invalid ARGB color for %s: %s", settingName, value);
    }
    Wh_FreeStringSetting(value);

    return color;
}

// https://devblogs.microsoft.com/oldnewthing/20200302-00/?p=103507
bool IsWindowCloaked(HWND hwnd) {
    BOOL isCloaked = FALSE;
    return SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &isCloaked,
                                           sizeof(isCloaked))) &&
           isCloaked;
}

class MonitorState {
   public:
    struct MonitorChange {
        HMONITOR monitor;
        bool shouldApplyStyle;
    };

    // Update window state, returns list of monitors whose state changed
    std::vector<MonitorChange> UpdateWindowState(HWND hWnd,
                                                 bool isMaximized,
                                                 bool touchesTaskbar,
                                                 HMONITOR monitor) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return UpdateWindowStateInternal(hWnd, isMaximized, touchesTaskbar,
                                         monitor);
    }

    // Remove window entirely (on destruction)
    std::vector<MonitorChange> RemoveWindow(HWND hWnd) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return UpdateWindowStateInternal(hWnd, false, false, nullptr);
    }

    bool ShouldApplyStyle(HMONITOR monitor) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return ShouldApplyStyleWhileLocked(monitor);
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maximizedWindowsPerMonitor.clear();
        m_touchingWindowsPerMonitor.clear();
        m_windowStates.clear();
    }

    // For atomic repopulation - clears state, updates cached values, and
    // returns a lock guard. Use AddWindowWhileLocked() to add windows while
    // holding the lock.
    [[nodiscard]] std::unique_lock<std::mutex> BeginRepopulate(
        DWORD dwTaskbarThreadId,
        HWND hShellWindow) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_maximizedWindowsPerMonitor.clear();
        m_touchingWindowsPerMonitor.clear();
        m_windowStates.clear();
        m_taskbarThreadId = dwTaskbarThreadId;
        m_shellWindow = hShellWindow;
        return lock;
    }

    // Add a window while holding the lock from BeginRepopulate()
    void AddWindowWhileLocked(HWND hWnd,
                              HMONITOR monitor,
                              bool isMaximized,
                              bool touchesTaskbar) {
        m_windowStates[hWnd] = {monitor, isMaximized, touchesTaskbar};
        if (isMaximized) {
            m_maximizedWindowsPerMonitor[monitor].insert(hWnd);
        }
        if (touchesTaskbar) {
            m_touchingWindowsPerMonitor[monitor].insert(hWnd);
        }
    }

    // Get cached values for event processing (single lock acquisition)
    void GetCachedState(DWORD* pdwTaskbarThreadId, HWND* phShellWindow) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        *pdwTaskbarThreadId = m_taskbarThreadId;
        *phShellWindow = m_shellWindow;
    }

   private:
    struct WindowState {
        HMONITOR monitor;
        bool isMaximized;
        bool touchesTaskbar;
    };

    std::vector<MonitorChange> UpdateWindowStateInternal(HWND hWnd,
                                                         bool isMaximized,
                                                         bool touchesTaskbar,
                                                         HMONITOR monitor) {
        std::vector<MonitorChange> changes;

        auto windowIt = m_windowStates.find(hWnd);
        HMONITOR oldMonitor =
            (windowIt != m_windowStates.end()) ? windowIt->second.monitor
                                               : nullptr;
        bool shouldTrack = monitor && (isMaximized || touchesTaskbar);

        bool oldShouldApply =
            oldMonitor && ShouldApplyStyleWhileLocked(oldMonitor);
        bool newShouldApplyBefore =
            monitor && monitor != oldMonitor &&
            ShouldApplyStyleWhileLocked(monitor);

        if (windowIt != m_windowStates.end()) {
            if (windowIt->second.isMaximized) {
                m_maximizedWindowsPerMonitor[oldMonitor].erase(hWnd);
            }
            if (windowIt->second.touchesTaskbar) {
                m_touchingWindowsPerMonitor[oldMonitor].erase(hWnd);
            }
            m_windowStates.erase(windowIt);
        }

        if (shouldTrack) {
            m_windowStates[hWnd] = {monitor, isMaximized, touchesTaskbar};
            if (isMaximized) {
                m_maximizedWindowsPerMonitor[monitor].insert(hWnd);
            }
            if (touchesTaskbar) {
                m_touchingWindowsPerMonitor[monitor].insert(hWnd);
            }
        }

        bool oldShouldApplyAfter =
            oldMonitor && ShouldApplyStyleWhileLocked(oldMonitor);
        bool newShouldApplyAfter =
            monitor && monitor != oldMonitor &&
            ShouldApplyStyleWhileLocked(monitor);

        if (oldMonitor && oldShouldApply != oldShouldApplyAfter) {
            changes.push_back({oldMonitor, oldShouldApplyAfter});
        }

        if (monitor && monitor != oldMonitor &&
            newShouldApplyBefore != newShouldApplyAfter) {
            changes.push_back({monitor, newShouldApplyAfter});
        }

        return changes;
    }

    bool ShouldApplyStyleWhileLocked(HMONITOR monitor) const {
        if (g_settings.applyWhen == ApplyWhen::always) {
            return true;
        }

        if (g_settings.applyWhen == ApplyWhen::maximized) {
            auto maximizedIt = m_maximizedWindowsPerMonitor.find(monitor);
            return maximizedIt != m_maximizedWindowsPerMonitor.end() &&
                   !maximizedIt->second.empty();
        }

        auto touchingIt = m_touchingWindowsPerMonitor.find(monitor);
        return touchingIt != m_touchingWindowsPerMonitor.end() &&
               touchingIt->second.size() >= 2;
    }

    mutable std::mutex m_mutex;

    // Track dynamic trigger sources independently because they use different
    // thresholds.
    std::unordered_map<HMONITOR, std::unordered_set<HWND>>
        m_maximizedWindowsPerMonitor;
    std::unordered_map<HMONITOR, std::unordered_set<HWND>>
        m_touchingWindowsPerMonitor;

    // Track each window's current monitor and dynamic flags.
    std::unordered_map<HWND, WindowState> m_windowStates;

    // Cached values to avoid expensive lookups in event processing
    DWORD m_taskbarThreadId = 0;
    HWND m_shellWindow = nullptr;
};

MonitorState g_monitorState;

// Encapsulates special view modes (Peek, Multitasking View) that should
// temporarily reset taskbar style to inactive.
class SpecialViewModeState {
   public:
    enum class Mode { None, Peek, MultitaskingView };

    // Returns true if any special view mode is active.
    bool IsActive() const { return m_mode != Mode::None; }

    // Sets the special view mode. Returns true if mode changed.
    bool SetMode(Mode mode) {
        Mode expected = Mode::None;
        return m_mode.compare_exchange_strong(expected, mode);
    }

    // Clears a specific mode (only if it's the current mode).
    // Returns true if mode was cleared.
    bool ClearMode(Mode mode) {
        Mode expected = mode;
        return m_mode.compare_exchange_strong(expected, Mode::None);
    }

    void Reset() { m_mode = Mode::None; }

   private:
    std::atomic<Mode> m_mode{Mode::None};
};

SpecialViewModeState g_specialViewMode;

std::mutex g_transparentXamlElementsMutex;
std::mutex g_transparentXamlTaskbarWindowsMutex;
std::atomic<bool> g_transparentXamlActive;
std::atomic<bool> g_transparentXamlDiagnosticsStarted;
std::unordered_set<HWND> g_transparentXamlTaskbarWindows;

struct TransparentXamlElementState {
    winrt::weak_ref<wuxs::Rectangle> rectangle;
    wf::IInspectable originalFill;
};

std::unordered_map<ULONG_PTR, TransparentXamlElementState>
    g_transparentXamlElements;

ULONG_PTR XamlObjectKey(wf::IInspectable const& object) {
    return reinterpret_cast<ULONG_PTR>(winrt::get_abi(object));
}

bool IsClassName(wux::DependencyObject const& object, PCWSTR className) {
    return winrt::get_class_name(object) == className;
}

bool HasTaskbarFrameAncestor(wux::DependencyObject object) {
    for (wux::DependencyObject current = object; current;
         current = wuxm::VisualTreeHelper::GetParent(current)) {
        if (IsClassName(current, L"Taskbar.TaskbarFrame")) {
            return true;
        }
    }

    return false;
}

bool IsTaskbarBackgroundFillRectangle(wuxs::Rectangle const& rectangle) {
    if (!rectangle || rectangle.Name() != L"BackgroundFill") {
        return false;
    }

    auto parent = wuxm::VisualTreeHelper::GetParent(rectangle);
    if (!parent || !IsClassName(parent, L"Windows.UI.Xaml.Controls.Grid")) {
        return false;
    }

    auto taskbarBackground = wuxm::VisualTreeHelper::GetParent(parent);
    if (!taskbarBackground ||
        !IsClassName(taskbarBackground, L"Taskbar.TaskbarBackground")) {
        return false;
    }

    return HasTaskbarFrameAncestor(taskbarBackground);
}

bool IsTaskbarBackgroundStrokeRectangle(wuxs::Rectangle const& rectangle) {
    return rectangle && rectangle.Name() == L"BackgroundStroke";
}

void ApplyTransparentStateToRectangle(wuxs::Rectangle const& rectangle,
                                      bool enable) {
    if (!IsTaskbarBackgroundFillRectangle(rectangle) &&
        !IsTaskbarBackgroundStrokeRectangle(rectangle)) {
        return;
    }

    ULONG_PTR key = XamlObjectKey(rectangle.as<wf::IInspectable>());
    std::lock_guard<std::mutex> lock(g_transparentXamlElementsMutex);

    if (enable) {
        if (!g_transparentXamlElements.contains(key)) {
            g_transparentXamlElements.emplace(
                key,
                TransparentXamlElementState{
                    winrt::make_weak(rectangle),
                    rectangle.ReadLocalValue(wuxs::Shape::FillProperty()),
                });
        }

        rectangle.Fill(
            wuxm::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0}));
        return;
    }

    auto it = g_transparentXamlElements.find(key);
    if (it == g_transparentXamlElements.end()) {
        return;
    }

    if (it->second.originalFill == wux::DependencyProperty::UnsetValue()) {
        rectangle.ClearValue(wuxs::Shape::FillProperty());
    } else {
        rectangle.SetValue(wuxs::Shape::FillProperty(),
                           it->second.originalFill);
    }

    g_transparentXamlElements.erase(it);
}

void ProcessTransparentXamlSubtree(wux::DependencyObject const& root,
                                   bool enable) {
    if (!root) {
        return;
    }

    std::vector<wux::DependencyObject> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        if (auto rectangle = current.try_as<wuxs::Rectangle>()) {
            ApplyTransparentStateToRectangle(rectangle, enable);
        }

        int childCount = wuxm::VisualTreeHelper::GetChildrenCount(current);
        for (int i = childCount - 1; i >= 0; i--) {
            if (auto child = wuxm::VisualTreeHelper::GetChild(current, i)) {
                stack.push_back(child);
            }
        }
    }
}

void ApplyTransparentStateToKnownRectangles(bool enable) {
    std::vector<winrt::weak_ref<wuxs::Rectangle>> rectangles;
    {
        std::lock_guard<std::mutex> lock(g_transparentXamlElementsMutex);
        rectangles.reserve(g_transparentXamlElements.size());
        for (auto const& [key, elementState] : g_transparentXamlElements) {
            rectangles.push_back(elementState.rectangle);
        }
    }

    for (auto const& weakRectangle : rectangles) {
        try {
            auto rectangle = weakRectangle.get();
            if (!rectangle) {
                continue;
            }

            auto dispatcher = rectangle.Dispatcher();
            if (dispatcher && !dispatcher.HasThreadAccess()) {
                dispatcher.RunAsync(
                    winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                    [rectangle, enable]() {
                        ApplyTransparentStateToRectangle(rectangle, enable);
                    });
            } else {
                ApplyTransparentStateToRectangle(rectangle, enable);
            }
        } catch (...) {
            Wh_Log(L"ApplyTransparentStateToKnownRectangles error: %08X",
                   winrt::to_hresult());
        }
    }
}

bool UpdateTransparentXamlActiveTaskbars(HWND hWnd, bool enable) {
    std::lock_guard<std::mutex> lock(g_transparentXamlTaskbarWindowsMutex);
    if (enable) {
        g_transparentXamlTaskbarWindows.insert(hWnd);
    } else {
        g_transparentXamlTaskbarWindows.erase(hWnd);
    }

    return !g_transparentXamlTaskbarWindows.empty();
}

HMODULE GetCurrentModuleHandle() {
    HMODULE module = nullptr;
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<PCWSTR>(&GetCurrentModuleHandle),
                           &module)) {
        return nullptr;
    }

    return module;
}

static constexpr GUID IID_IXamlDiagnosticsTestHooks = {
    0x735941a2,
    0x3ee3,
    0x495a,
    {0x8d, 0xa9, 0x97, 0x26, 0x27, 0x00, 0x30, 0x75}};

struct IXamlDiagnosticsTestHooks : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE UnregisterInstance(
        InstanceHandle handle) = 0;
    virtual HRESULT STDMETHODCALLTYPE TryGetDispatcherQueueForObject(
        InstanceHandle handle,
        void** dispatcherQueue) = 0;
};

class VisualTreeWatcher
    : public winrt::implements<VisualTreeWatcher,
                               IVisualTreeServiceCallback2,
                               winrt::non_agile> {
   public:
    VisualTreeWatcher(winrt::com_ptr<IUnknown> site)
        : m_XamlDiagnostics(site.as<IXamlDiagnostics>()) {
        Wh_Log(L"Constructing VisualTreeWatcher");

        HRESULT hr = m_XamlDiagnostics->QueryInterface(
            IID_IXamlDiagnosticsTestHooks,
            m_XamlDiagnosticsTestHooks.put_void());
        if (FAILED(hr)) {
            Wh_Log(L"IXamlDiagnosticsTestHooks unavailable: %08X", hr);
        }

        HANDLE thread = CreateThread(
            nullptr, 0, AdviseVisualTreeChangeThreadProc, this, 0, nullptr);
        if (thread) {
            AddRef();
            CloseHandle(thread);
        }
    }

    ~VisualTreeWatcher() { Wh_Log(L"Destructing VisualTreeWatcher"); }

    void UnadviseVisualTreeChange() {
        HRESULT hr =
            m_XamlDiagnostics.as<IVisualTreeService3>()
                ->UnadviseVisualTreeChange(this);
        if (FAILED(hr)) {
            Wh_Log(L"UnadviseVisualTreeChange failed: %08X", hr);
        }
    }

   private:
    static DWORD WINAPI AdviseVisualTreeChangeThreadProc(LPVOID lpParam) {
        auto watcher = reinterpret_cast<VisualTreeWatcher*>(lpParam);
        auto service = watcher->m_XamlDiagnostics.as<IVisualTreeService3>();
        HRESULT hr = service->AdviseVisualTreeChange(watcher);
        watcher->Release();
        if (FAILED(hr)) {
            Wh_Log(L"AdviseVisualTreeChange failed: %08X", hr);
        }
        return 0;
    }

    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(
        ParentChildRelation relation,
        VisualElement element,
        VisualMutationType mutationType) override try {
        if (mutationType == Add) {
            wf::IInspectable inspectable;
            HRESULT hr = m_XamlDiagnostics->GetIInspectableFromHandle(
                element.Handle,
                reinterpret_cast<::IInspectable**>(
                    winrt::put_abi(inspectable)));
            if (SUCCEEDED(hr) && inspectable) {
                if (auto rectangle = inspectable.try_as<wuxs::Rectangle>()) {
                    ApplyTransparentStateToRectangle(
                        rectangle, g_transparentXamlActive);
                } else if (auto dependencyObject =
                               inspectable.try_as<wux::DependencyObject>()) {
                    ProcessTransparentXamlSubtree(
                        dependencyObject, g_transparentXamlActive);
                }
            }
        } else if (mutationType == Remove) {
            std::lock_guard<std::mutex> lock(g_transparentXamlElementsMutex);
            g_transparentXamlElements.erase(
                static_cast<ULONG_PTR>(element.Handle));
        }

        ReleaseDiagnosticsReference(element.Handle);
        ReleaseDiagnosticsReference(relation.Parent);
        return S_OK;
    } catch (...) {
        Wh_Log(L"OnVisualTreeChange error: %08X", winrt::to_hresult());
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnElementStateChanged(
        InstanceHandle,
        VisualElementState,
        LPCWSTR) noexcept override {
        return S_OK;
    }

    void ReleaseDiagnosticsReference(InstanceHandle handle) {
        if (!m_XamlDiagnosticsTestHooks || !handle) {
            return;
        }

        HRESULT hr = m_XamlDiagnosticsTestHooks->UnregisterInstance(handle);
        if (FAILED(hr)) {
            Wh_Log(L"UnregisterInstance failed: %08X", hr);
        }
    }

    winrt::com_ptr<IXamlDiagnostics> m_XamlDiagnostics;
    winrt::com_ptr<IXamlDiagnosticsTestHooks> m_XamlDiagnosticsTestHooks;
};

winrt::com_ptr<VisualTreeWatcher> g_visualTreeWatcher;

// {9F5A4D70-5D77-4DB3-9A5C-56210C2E8D43}
static constexpr CLSID CLSID_WindhawkTAP = {
    0x9f5a4d70,
    0x5d77,
    0x4db3,
    {0x9a, 0x5c, 0x56, 0x21, 0x0c, 0x2e, 0x8d, 0x43}};

class WindhawkTAP
    : public winrt::implements<WindhawkTAP,
                               IObjectWithSite,
                               winrt::non_agile> {
   public:
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown* pUnkSite) override try {
        if (g_visualTreeWatcher) {
            g_visualTreeWatcher->UnadviseVisualTreeChange();
            g_visualTreeWatcher = nullptr;
        }

        site.copy_from(pUnkSite);
        if (site) {
            FreeLibrary(GetCurrentModuleHandle());
            g_visualTreeWatcher = winrt::make_self<VisualTreeWatcher>(site);
        }

        return S_OK;
    } catch (...) {
        HRESULT hr = winrt::to_hresult();
        Wh_Log(L"WindhawkTAP::SetSite error: %08X", hr);
        return hr;
    }

    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid,
                                      void** ppvSite) noexcept override {
        return site.as(riid, ppvSite);
    }

   private:
    winrt::com_ptr<IUnknown> site;
};

template <class T>
struct SimpleFactory
    : winrt::implements<SimpleFactory<T>, IClassFactory, winrt::non_agile> {
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter,
                                             REFIID riid,
                                             void** ppvObject) override try {
        if (pUnkOuter) {
            return CLASS_E_NOAGGREGATION;
        }

        *ppvObject = nullptr;
        return winrt::make<T>().as(riid, ppvObject);
    } catch (...) {
        HRESULT hr = winrt::to_hresult();
        Wh_Log(L"SimpleFactory::CreateInstance error: %08X", hr);
        return hr;
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override {
        return S_OK;
    }
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"

__declspec(dllexport) _Use_decl_annotations_ STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID* ppv) try {
    if (rclsid == CLSID_WindhawkTAP) {
        *ppv = nullptr;
        return winrt::make<SimpleFactory<WindhawkTAP>>().as(riid, ppv);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
} catch (...) {
    HRESULT hr = winrt::to_hresult();
    Wh_Log(L"DllGetClassObject error: %08X", hr);
    return hr;
}

__declspec(dllexport) _Use_decl_annotations_ STDAPI DllCanUnloadNow() {
    return winrt::get_module_lock() ? S_FALSE : S_OK;
}

#pragma clang diagnostic pop

using PFN_INITIALIZE_XAML_DIAGNOSTICS_EX =
    decltype(&InitializeXamlDiagnosticsEx);

HRESULT InjectWindhawkTAP() noexcept {
    HMODULE module = GetCurrentModuleHandle();
    if (!module) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    WCHAR location[MAX_PATH];
    switch (GetModuleFileName(module, location, ARRAYSIZE(location))) {
        case 0:
        case ARRAYSIZE(location):
            return HRESULT_FROM_WIN32(GetLastError());
    }

    HMODULE wux =
        LoadLibraryEx(L"Windows.UI.Xaml.dll", nullptr,
                      LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!wux) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    auto ixde = reinterpret_cast<PFN_INITIALIZE_XAML_DIAGNOSTICS_EX>(
        GetProcAddress(wux, "InitializeXamlDiagnosticsEx"));
    if (!ixde) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT hr = E_FAIL;
    for (int i = 0; i < 10000; i++) {
        WCHAR connectionName[256];
        wsprintf(connectionName, L"VisualDiagConnection%d", i + 1);

        hr = ixde(connectionName, GetCurrentProcessId(), L"", location,
                  CLSID_WindhawkTAP, nullptr);
        if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
            break;
        }
    }

    return hr;
}

void EnsureTransparentXamlDiagnosticsStarted() {
    bool expected = false;
    if (!g_transparentXamlDiagnosticsStarted.compare_exchange_strong(expected,
                                                                     true)) {
        return;
    }

    HRESULT hr = InjectWindhawkTAP();
    if (FAILED(hr)) {
        Wh_Log(L"InjectWindhawkTAP failed: %08X", hr);
        g_transparentXamlDiagnosticsStarted = false;
    }
}

void ApplyTransparentXamlStyleForCurrentThread(bool enable) {
    try {
        auto window = wux::Window::Current();
        if (!window) {
            return;
        }

        ProcessTransparentXamlSubtree(
            window.Content().try_as<wux::DependencyObject>(), enable);
    } catch (...) {
        Wh_Log(L"ApplyTransparentXamlStyleForCurrentThread error: %08X",
               winrt::to_hresult());
    }
}

using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID parameter);

struct RUN_FROM_WINDOW_THREAD_PARAM {
    RunFromWindowThreadProc_t proc;
    PVOID procParam;
};

UINT GetRunFromWindowThreadRegisteredMsg() {
    static const UINT runFromWindowThreadRegisteredMsg =
        RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    return runFromWindowThreadRegisteredMsg;
}

LRESULT CALLBACK RunFromWindowThreadHookProc(int nCode,
                                             WPARAM wParam,
                                             LPARAM lParam) {
    if (nCode == HC_ACTION) {
        const CWPSTRUCT* cwp = reinterpret_cast<const CWPSTRUCT*>(lParam);
        if (cwp->message == GetRunFromWindowThreadRegisteredMsg()) {
            auto* param =
                reinterpret_cast<RUN_FROM_WINDOW_THREAD_PARAM*>(cwp->lParam);
            param->proc(param->procParam);
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool RunFromWindowThread(HWND hWnd,
                         RunFromWindowThreadProc_t proc,
                         PVOID procParam) {
    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (dwThreadId == 0) {
        return false;
    }

    HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, RunFromWindowThreadHookProc,
                                  nullptr, dwThreadId);
    if (!hook) {
        return false;
    }

    RUN_FROM_WINDOW_THREAD_PARAM param = {proc, procParam};
    SendMessage(hWnd, GetRunFromWindowThreadRegisteredMsg(), 0,
                reinterpret_cast<LPARAM>(&param));

    UnhookWindowsHookEx(hook);

    return true;
}

void WINAPI ApplyTransparentXamlStyleForCurrentThreadProc(PVOID parameter) {
    ApplyTransparentXamlStyleForCurrentThread(
        reinterpret_cast<ULONG_PTR>(parameter) != 0);
}

void ApplyTransparentXamlStyleForTaskbarWindow(HWND hWnd, bool enable) {
    bool shouldEnable = UpdateTransparentXamlActiveTaskbars(hWnd, enable);
    g_transparentXamlActive = shouldEnable;

    if (shouldEnable) {
        EnsureTransparentXamlDiagnosticsStarted();
    }

    ApplyTransparentStateToKnownRectangles(shouldEnable);

    HWND hTaskbarUiWnd = FindWindowEx(
        hWnd, nullptr, L"Windows.UI.Composition.DesktopWindowContentBridge",
        nullptr);
    if (!hTaskbarUiWnd) {
        return;
    }

    RunFromWindowThread(hTaskbarUiWnd,
                        ApplyTransparentXamlStyleForCurrentThreadProc,
                        reinterpret_cast<PVOID>(shouldEnable ? 1 : 0));
}

BOOL SetTaskbarStyle(HWND hWnd) {
    Wh_Log(L">");

    TaskbarStyle& style =
        (g_settings.darkModeStyle && IsWindowsDarkModeEnabled())
            ? *g_settings.darkModeStyle
            : g_settings.style;
    bool transparentStyle =
        style.backgroundStyle == BackgroundStyle::transparent;

    ACCENT_STATE accentState;
    UINT accentFlags = 0;
    switch (style.backgroundStyle) {
        case BackgroundStyle::defaultStyle:
            accentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
            accentFlags = 0x13;
            break;

        case BackgroundStyle::blur:
            accentState = ACCENT_ENABLE_BLURBEHIND;
            break;

        case BackgroundStyle::acrylicBlur:
            accentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
            break;

        case BackgroundStyle::color:
            accentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
            accentFlags = 0x13;
            break;

        case BackgroundStyle::transparent:
            accentState = ACCENT_INVALID_STATE;
            break;
    }

    COLORREF color =
        style.backgroundStyle == BackgroundStyle::defaultStyle ||
                style.backgroundStyle == BackgroundStyle::transparent
            ? 0
            : style.color;

    ACCENTPOLICY policy = {accentState, accentFlags, color, 0};

    WINDOWCOMPOSITIONATTRIBDATA data = {WCA_ACCENT_POLICY, &policy,
                                        sizeof(policy)};
    BOOL result = SetWindowCompositionAttribute_Original(hWnd, &data);
    ApplyTransparentXamlStyleForTaskbarWindow(hWnd, transparentStyle);
    return result;
}

BOOL ResetTaskbarStyle(HWND hWnd) {
    Wh_Log(L">");

    ApplyTransparentXamlStyleForTaskbarWindow(hWnd, false);

    // TrayUI::_OnThemeChanged
    // TrayUI::OnShellModeChanged
    ACCENTPOLICY policy = {ACCENT_ENABLE_TRANSPARENTGRADIENT, 0x13, 0, 0};
    WINDOWCOMPOSITIONATTRIBDATA data = {WCA_ACCENT_POLICY, &policy,
                                        sizeof(policy)};
    return SetWindowCompositionAttribute_Original(hWnd, &data);
}

template <typename Callback>
BOOL CALLBACK EnumWindowsProcThunk(HWND hWnd, LPARAM lParam) {
    auto& callback = *reinterpret_cast<Callback*>(lParam);
    return callback(hWnd);
}

HWND FindCurrentProcessTaskbarWnd() {
    HWND hTaskbarWnd = nullptr;

    auto enumWindowsProc = [&hTaskbarWnd](HWND hWnd) -> BOOL {
        DWORD dwProcessId;
        WCHAR className[32];
        if (GetWindowThreadProcessId(hWnd, &dwProcessId) &&
            dwProcessId == GetCurrentProcessId() &&
            GetClassName(hWnd, className, ARRAYSIZE(className)) &&
            _wcsicmp(className, L"Shell_TrayWnd") == 0) {
            hTaskbarWnd = hWnd;
            return FALSE;
        }
        return TRUE;
    };

    EnumWindows(EnumWindowsProcThunk<decltype(enumWindowsProc)>,
                reinterpret_cast<LPARAM>(&enumWindowsProc));

    return hTaskbarWnd;
}

bool IsTaskbarWindow(HWND hWnd) {
    WCHAR szClassName[32];
    if (!GetClassName(hWnd, szClassName, ARRAYSIZE(szClassName))) {
        return false;
    }

    return _wcsicmp(szClassName, L"Shell_TrayWnd") == 0 ||
           _wcsicmp(szClassName, L"Shell_SecondaryTrayWnd") == 0;
}

HWND FindTaskbarWindows(std::unordered_set<HWND>* secondaryTaskbarWindows) {
    secondaryTaskbarWindows->clear();

    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) {
        return nullptr;
    }

    DWORD taskbarThreadId = GetWindowThreadProcessId(hTaskbarWnd, nullptr);
    if (!taskbarThreadId) {
        return nullptr;
    }

    auto enumWindowsProc = [&secondaryTaskbarWindows](HWND hWnd) -> BOOL {
        WCHAR szClassName[32];
        if (GetClassName(hWnd, szClassName, ARRAYSIZE(szClassName)) == 0) {
            return TRUE;
        }

        if (_wcsicmp(szClassName, L"Shell_SecondaryTrayWnd") == 0) {
            secondaryTaskbarWindows->insert(hWnd);
        }

        return TRUE;
    };

    EnumThreadWindows(taskbarThreadId,
                      EnumWindowsProcThunk<decltype(enumWindowsProc)>,
                      reinterpret_cast<LPARAM>(&enumWindowsProc));

    return hTaskbarWnd;
}

HWND GetTaskbarForMonitor(HMONITOR monitor) {
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) {
        return nullptr;
    }

    HMONITOR taskbarMonitor = (HMONITOR)GetProp(hTaskbarWnd, L"TaskbarMonitor");
    if (taskbarMonitor == monitor) {
        return hTaskbarWnd;
    }

    DWORD taskbarThreadId = GetWindowThreadProcessId(hTaskbarWnd, nullptr);
    if (!taskbarThreadId) {
        return nullptr;
    }

    HWND hResultWnd = nullptr;

    auto enumWindowsProc = [monitor, &hResultWnd](HWND hWnd) -> BOOL {
        WCHAR szClassName[32];
        if (GetClassName(hWnd, szClassName, ARRAYSIZE(szClassName)) == 0) {
            return TRUE;
        }

        if (_wcsicmp(szClassName, L"Shell_SecondaryTrayWnd") != 0) {
            return TRUE;
        }

        HMONITOR taskbarMonitor = (HMONITOR)GetProp(hWnd, L"TaskbarMonitor");
        if (taskbarMonitor != monitor) {
            return TRUE;
        }

        hResultWnd = hWnd;
        return FALSE;
    };

    EnumThreadWindows(taskbarThreadId,
                      EnumWindowsProcThunk<decltype(enumWindowsProc)>,
                      reinterpret_cast<LPARAM>(&enumWindowsProc));

    return hResultWnd;
}

void UpdateTaskbarStyleForMonitor(HMONITOR monitor, bool shouldApplyStyle) {
    HWND hMMTaskbarWnd = GetTaskbarForMonitor(monitor);
    if (!hMMTaskbarWnd) {
        return;
    }

    if (shouldApplyStyle) {
        SetTaskbarStyle(hMMTaskbarWnd);
    } else {
        ResetTaskbarStyle(hMMTaskbarWnd);
    }
}

BOOL ApplyTaskbarStyleForWindow(HWND hWnd) {
    if (g_settings.applyWhen == ApplyWhen::always) {
        return SetTaskbarStyle(hWnd);
    }

    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    if (g_specialViewMode.IsActive() ||
        !g_monitorState.ShouldApplyStyle(monitor)) {
        return ResetTaskbarStyle(hWnd);
    }

    return SetTaskbarStyle(hWnd);
}

void EnsureMonitoringThreadStarted();

// Updates all taskbars based on current special view mode and selected
// condition state.
void UpdateAllTaskbarStyles() {
    std::unordered_set<HWND> secondaryTaskbarWindows;
    HWND hTaskbarWnd = FindTaskbarWindows(&secondaryTaskbarWindows);
    if (!hTaskbarWnd) {
        return;
    }

    EnsureMonitoringThreadStarted();
    ApplyTaskbarStyleForWindow(hTaskbarWnd);

    for (HWND hSecondaryWnd : secondaryTaskbarWindows) {
        ApplyTaskbarStyleForWindow(hSecondaryWnd);
    }
}

// https://gist.github.com/m417z/451dfc2dad88d7ba88ed1814779a26b4
std::wstring GetWindowAppId(HWND hWnd) {
    // {c8900b66-a973-584b-8cae-355b7f55341b}
    constexpr winrt::guid CLSID_StartMenuCacheAndAppResolver{
        0x660b90c8,
        0x73a9,
        0x4b58,
        {0x8c, 0xae, 0x35, 0x5b, 0x7f, 0x55, 0x34, 0x1b}};

    // {de25675a-72de-44b4-9373-05170450c140}
    constexpr winrt::guid IID_IAppResolver_8{
        0xde25675a,
        0x72de,
        0x44b4,
        {0x93, 0x73, 0x05, 0x17, 0x04, 0x50, 0xc1, 0x40}};

    struct IAppResolver_8 : public IUnknown {
       public:
        virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcut() = 0;
        virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcutObject() = 0;
        virtual HRESULT STDMETHODCALLTYPE
        GetAppIDForWindow(HWND hWnd,
                          WCHAR** pszAppId,
                          void* pUnknown1,
                          void* pUnknown2,
                          void* pUnknown3) = 0;
        virtual HRESULT STDMETHODCALLTYPE
        GetAppIDForProcess(DWORD dwProcessId,
                           WCHAR** pszAppId,
                           void* pUnknown1,
                           void* pUnknown2,
                           void* pUnknown3) = 0;
    };

    HRESULT hr;
    std::wstring result;

    CO_MTA_USAGE_COOKIE cookie;
    bool mtaUsageIncreased = SUCCEEDED(CoIncrementMTAUsage(&cookie));

    winrt::com_ptr<IAppResolver_8> appResolver;
    hr = CoCreateInstance(CLSID_StartMenuCacheAndAppResolver, nullptr,
                          CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                          IID_IAppResolver_8, appResolver.put_void());
    if (SUCCEEDED(hr)) {
        WCHAR* pszAppId;
        hr = appResolver->GetAppIDForWindow(hWnd, &pszAppId, nullptr, nullptr,
                                            nullptr);
        if (SUCCEEDED(hr)) {
            result = pszAppId;
            CoTaskMemFree(pszAppId);
        }
    }

    appResolver = nullptr;

    if (mtaUsageIncreased) {
        CoDecrementMTAUsage(cookie);
    }

    return result;
}

std::wstring GetProcessFileName(DWORD dwProcessId) {
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        return std::wstring{};
    }

    WCHAR processPath[MAX_PATH];

    DWORD dwSize = ARRAYSIZE(processPath);
    if (!QueryFullProcessImageName(hProcess, 0, processPath, &dwSize)) {
        CloseHandle(hProcess);
        return std::wstring{};
    }

    CloseHandle(hProcess);

    PCWSTR processFileNameUpper = wcsrchr(processPath, L'\\');
    if (!processFileNameUpper) {
        return std::wstring{};
    }

    processFileNameUpper++;
    return processFileNameUpper;
}

std::wstring GetWindowLogInfo(HWND hWnd) {
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    std::wstring processName = GetProcessFileName(dwProcessId);

    WCHAR className[256];
    if (!GetClassName(hWnd, className, ARRAYSIZE(className))) {
        wcscpy_s(className, L"<unknown>");
    }

    WCHAR windowName[256];
    if (!GetWindowText(hWnd, windowName, ARRAYSIZE(windowName))) {
        windowName[0] = L'\0';
    }

    LONG style = GetWindowLong(hWnd, GWL_STYLE);
    LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

    RECT rect{};
    GetWindowRect(hWnd, &rect);

    WCHAR buffer[1024];
    swprintf_s(buffer,
               L"window %08X: PID=%u, process=%s, class=%s, name=%s, "
               L"style=0x%08X, exStyle=0x%08X, rect={%d,%d,%d,%d}",
               (DWORD)(DWORD_PTR)hWnd, dwProcessId, processName.c_str(),
               className, windowName, style, exStyle, rect.left, rect.top,
               rect.right, rect.bottom);
    return buffer;
}

bool IsWindowExcluded(HWND hWnd) {
    if (g_settings.excludedPrograms.empty()) {
        return false;
    }

    DWORD resolvedWindowProcessPathLen = 0;
    WCHAR resolvedWindowProcessPath[MAX_PATH];
    WCHAR resolvedWindowProcessPathUpper[MAX_PATH];

    DWORD dwProcessId = 0;
    if (GetWindowThreadProcessId(hWnd, &dwProcessId)) {
        HANDLE hProcess =
            OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
        if (hProcess) {
            DWORD dwSize = ARRAYSIZE(resolvedWindowProcessPath);
            if (QueryFullProcessImageName(hProcess, 0,
                                          resolvedWindowProcessPath, &dwSize)) {
                resolvedWindowProcessPathLen = dwSize;
            }

            CloseHandle(hProcess);
        }
    }

    if (resolvedWindowProcessPathLen > 0) {
        LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE,
                      resolvedWindowProcessPath,
                      resolvedWindowProcessPathLen + 1,
                      resolvedWindowProcessPathUpper,
                      resolvedWindowProcessPathLen + 1, nullptr, nullptr, 0);
    } else {
        *resolvedWindowProcessPath = L'\0';
        *resolvedWindowProcessPathUpper = L'\0';
    }

    if (resolvedWindowProcessPathLen > 0 &&
        g_settings.excludedPrograms.contains(resolvedWindowProcessPathUpper)) {
        return true;
    }

    if (PCWSTR programFileNameUpper =
            wcsrchr(resolvedWindowProcessPathUpper, L'\\')) {
        programFileNameUpper++;
        if (*programFileNameUpper &&
            g_settings.excludedPrograms.contains(programFileNameUpper)) {
            return true;
        }
    }

    std::wstring appId = GetWindowAppId(hWnd);
    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, appId.data(),
                  appId.length(), appId.data(), appId.length(), nullptr,
                  nullptr, 0);
    if (g_settings.excludedPrograms.contains(appId.c_str())) {
        return true;
    }

    return false;
}

// Detects Win+Tab / Task View window.
// Uses ZBID_IMMERSIVE_APPCHROME band, MultitaskingView thread description, and
// taskbar process.
bool IsMultitaskingViewWindow(HWND hWnd) {
    // Must be in the current process (explorer.exe).
    DWORD dwProcessId = 0;
    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
    if (!dwThreadId || dwProcessId != GetCurrentProcessId()) {
        return false;
    }

    // Check window band - must be ZBID_IMMERSIVE_APPCHROME (5).
    if (pGetWindowBand) {
        DWORD band = 0;
        if (!pGetWindowBand(hWnd, &band) || band != 5) {
            return false;
        }
    }

    // Check thread description for "MultitaskingView".
    HANDLE hThread =
        OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, dwThreadId);
    if (!hThread) {
        return false;
    }

    bool isMultitaskingView = false;
    PWSTR description = nullptr;
    if (SUCCEEDED(GetThreadDescription(hThread, &description)) && description) {
        isMultitaskingView = wcscmp(description, L"MultitaskingView") == 0;
        LocalFree(description);
    }

    CloseHandle(hThread);
    return isMultitaskingView;
}

bool IsWindowActiveCandidate(HWND hWnd,
                             DWORD dwTaskbarThreadId,
                             HWND hShellWindow) {
    DWORD dwProcessId = 0;
    if (GetWindowThreadProcessId(hWnd, &dwProcessId) == dwTaskbarThreadId) {
        return false;
    }

    if (!IsWindowVisible(hWnd) || IsWindowCloaked(hWnd) || IsIconic(hWnd) ||
        (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE)) {
        return false;
    }

    if (hWnd == hShellWindow || GetProp(hWnd, L"DesktopWindow")) {
        return false;
    }

    // Exclude DWM aero peek window.
    WCHAR className[256];
    if (GetClassName(hWnd, className, ARRAYSIZE(className)) &&
        _wcsicmp(className, L"LivePreview") == 0) {
        return false;
    }

    // Check this after the other checks, as it's the most expensive one.
    if (IsWindowExcluded(hWnd)) {
        return false;
    }

    return true;
}

bool IsWindowMaximizedOrFullscreen(HWND hWnd, HMONITOR monitor) {
    WINDOWPLACEMENT wp{
        .length = sizeof(WINDOWPLACEMENT),
    };
    if (GetWindowPlacement(hWnd, &wp) && wp.showCmd == SW_SHOWMAXIMIZED) {
        Wh_Log(L"Maximized %s for monitor %p", GetWindowLogInfo(hWnd).c_str(),
               monitor);
        return true;
    }

    MONITORINFO monitorInfo{
        .cbSize = sizeof(monitorInfo),
    };
    GetMonitorInfo(monitor, &monitorInfo);

    RECT windowRect{};
    DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &windowRect,
                          sizeof(windowRect));

    if (EqualRect(&windowRect, &monitorInfo.rcMonitor)) {
        // Spans across the whole monitor, e.g. Win+Tab view.
        Wh_Log(L"Fullscreen %s for monitor %p", GetWindowLogInfo(hWnd).c_str(),
               monitor);
        return true;
    }

    return false;
}

bool IsNearEdge(LONG edge1, LONG edge2) {
    constexpr LONG kEdgeTolerance = 0;
    LONG diff = edge1 - edge2;
    return (diff < 0 ? -diff : diff) <= kEdgeTolerance;
}

bool HasOverlap(LONG start1, LONG end1, LONG start2, LONG end2) {
    LONG overlapStart = start1 > start2 ? start1 : start2;
    LONG overlapEnd = end1 < end2 ? end1 : end2;
    return overlapStart < overlapEnd;
}

bool IsWindowTouchingTaskbar(HWND hWnd, HMONITOR monitor) {
    HWND hTaskbarWnd = GetTaskbarForMonitor(monitor);
    if (!hTaskbarWnd) {
        return false;
    }

    MONITORINFO monitorInfo{
        .cbSize = sizeof(monitorInfo),
    };
    if (!GetMonitorInfo(monitor, &monitorInfo)) {
        return false;
    }

    RECT taskbarRect{};
    if (!GetWindowRect(hTaskbarWnd, &taskbarRect)) {
        return false;
    }

    RECT windowRect{};
    if (FAILED(DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS,
                                     &windowRect, sizeof(windowRect)))) {
        if (!GetWindowRect(hWnd, &windowRect)) {
            return false;
        }
    }

    bool touches = false;
    if (monitorInfo.rcWork.bottom < monitorInfo.rcMonitor.bottom) {
        touches = IsNearEdge(windowRect.bottom, monitorInfo.rcWork.bottom) &&
                  HasOverlap(windowRect.left, windowRect.right,
                             taskbarRect.left, taskbarRect.right);
    } else if (monitorInfo.rcWork.top > monitorInfo.rcMonitor.top) {
        touches = IsNearEdge(windowRect.top, monitorInfo.rcWork.top) &&
                  HasOverlap(windowRect.left, windowRect.right,
                             taskbarRect.left, taskbarRect.right);
    } else if (monitorInfo.rcWork.right < monitorInfo.rcMonitor.right) {
        touches = IsNearEdge(windowRect.right, monitorInfo.rcWork.right) &&
                  HasOverlap(windowRect.top, windowRect.bottom,
                             taskbarRect.top, taskbarRect.bottom);
    } else if (monitorInfo.rcWork.left > monitorInfo.rcMonitor.left) {
        touches = IsNearEdge(windowRect.left, monitorInfo.rcWork.left) &&
                  HasOverlap(windowRect.top, windowRect.bottom,
                             taskbarRect.top, taskbarRect.bottom);
    }

    if (touches) {
        Wh_Log(L"Touches taskbar %s for monitor %p",
               GetWindowLogInfo(hWnd).c_str(), monitor);
    }

    return touches;
}

struct DynamicWindowState {
    bool isMaximized;
    bool touchesTaskbar;
};

DynamicWindowState GetWindowDynamicState(HWND hWnd, HMONITOR monitor) {
    return {
        IsWindowMaximizedOrFullscreen(hWnd, monitor),
        IsWindowTouchingTaskbar(hWnd, monitor),
    };
}

bool IsWindowActiveForSelectedCondition(HWND hWnd, HMONITOR monitor) {
    switch (g_settings.applyWhen) {
        case ApplyWhen::always:
            return true;

        case ApplyWhen::maximized:
            return IsWindowMaximizedOrFullscreen(hWnd, monitor);

        case ApplyWhen::dynamic:
            return IsWindowTouchingTaskbar(hWnd, monitor);
    }

    return false;
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook,
                           DWORD event,
                           HWND hWnd,
                           LONG idObject,
                           LONG idChild,
                           DWORD dwEventThread,
                           DWORD dwmsEventTime) {
    if (idObject != OBJID_WINDOW ||
        (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) || IsTaskbarWindow(hWnd)) {
        return;
    }

    HWND hParentWnd = GetAncestor(hWnd, GA_PARENT);
    if (hParentWnd && hParentWnd != GetDesktopWindow()) {
        return;
    }

    auto logEvent = [event, hWnd] {
        Wh_Log(
            L"Event %s for %s",
            [](DWORD event) -> PCWSTR {
                switch (event) {
                    case EVENT_OBJECT_CREATE:
                        return L"OBJECT_CREATE";
                    case EVENT_OBJECT_DESTROY:
                        return L"OBJECT_DESTROY";
                    case EVENT_OBJECT_SHOW:
                        return L"OBJECT_SHOW";
                    case EVENT_OBJECT_HIDE:
                        return L"OBJECT_HIDE";
                    case EVENT_OBJECT_LOCATIONCHANGE:
                        return L"OBJECT_LOCATIONCHANGE";
                    case EVENT_OBJECT_CLOAKED:
                        return L"OBJECT_CLOAKED";
                    case EVENT_OBJECT_UNCLOAKED:
                        return L"OBJECT_UNCLOAKED";
                    default:
                        return L"UNKNOWN";
                }
            }(event),
            GetWindowLogInfo(hWnd).c_str());
    };

    // Check for Multitasking View (Win+Tab) window state changes.
    if (IsMultitaskingViewWindow(hWnd)) {
        bool entering = event == EVENT_OBJECT_SHOW ||
                        event == EVENT_OBJECT_UNCLOAKED ||
                        event == EVENT_OBJECT_CREATE;
        bool leaving = event == EVENT_OBJECT_HIDE ||
                       event == EVENT_OBJECT_CLOAKED ||
                       event == EVENT_OBJECT_DESTROY;

        if (entering) {
            Wh_Log(L"MultitaskingView entering");
            if (g_specialViewMode.SetMode(
                    SpecialViewModeState::Mode::MultitaskingView)) {
                UpdateAllTaskbarStyles();
            }
        } else if (leaving) {
            Wh_Log(L"MultitaskingView leaving");
            if (g_specialViewMode.ClearMode(
                    SpecialViewModeState::Mode::MultitaskingView)) {
                UpdateAllTaskbarStyles();
            }
        }

        return;
    }

    std::vector<MonitorState::MonitorChange> changes;

    if (event == EVENT_OBJECT_DESTROY) {
        logEvent();

        // Window is being destroyed, remove it from tracking
        changes = g_monitorState.RemoveWindow(hWnd);
    } else if (event == EVENT_OBJECT_HIDE || event == EVENT_OBJECT_CLOAKED) {
        logEvent();

        // Window is being hidden or cloaked, mark it as inactive
        HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

        changes =
            g_monitorState.UpdateWindowState(hWnd, false, false, monitor);
    } else {
        // Use cached taskbar thread ID and shell window for candidate check
        DWORD dwTaskbarThreadId;
        HWND hShellWindow;
        g_monitorState.GetCachedState(&dwTaskbarThreadId, &hShellWindow);

        HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

        // Check if window is an active candidate (visible, not cloaked, etc.)
        bool isCandidate =
            IsWindowActiveCandidate(hWnd, dwTaskbarThreadId, hShellWindow);

        if (isCandidate) {
            logEvent();
        }

        DynamicWindowState dynamicState{};
        if (isCandidate) {
            dynamicState = GetWindowDynamicState(hWnd, monitor);
        }

        changes = g_monitorState.UpdateWindowState(
            hWnd, dynamicState.isMaximized, dynamicState.touchesTaskbar,
            monitor);
    }

    // Update taskbar style for each monitor that changed
    // (but not if in special view mode - handlers will restore when done)
    if (!g_specialViewMode.IsActive()) {
        for (const auto& change : changes) {
            Wh_Log(L"Monitor %p state changed to %s", change.monitor,
                   change.shouldApplyStyle ? L"active" : L"inactive");
            UpdateTaskbarStyleForMonitor(change.monitor,
                                         change.shouldApplyStyle);
        }
    }
}

void CALLBACK PeekEventProc(HWINEVENTHOOK hWinEventHook,
                            DWORD event,
                            HWND hWnd,
                            LONG idObject,
                            LONG idChild,
                            DWORD dwEventThread,
                            DWORD dwmsEventTime) {
    bool entering = (event == EVENT_SYSTEM_PEEKSTART);
    Wh_Log(L"Peek %s", entering ? L"start" : L"end");

    if (entering) {
        if (g_specialViewMode.SetMode(SpecialViewModeState::Mode::Peek)) {
            UpdateAllTaskbarStyles();
        }
    } else {
        if (g_specialViewMode.ClearMode(SpecialViewModeState::Mode::Peek)) {
            UpdateAllTaskbarStyles();
        }
    }
}

DWORD WINAPI WinEventHookThread(LPVOID lpThreadParameter) {
    HWINEVENTHOOK winObjectEventHook1 =
        SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_HIDE, nullptr,
                        WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (!winObjectEventHook1) {
        Wh_Log(L"Error: SetWinEventHook");
    }

    HWINEVENTHOOK winObjectEventHook2 = SetWinEventHook(
        EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr,
        WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (!winObjectEventHook2) {
        Wh_Log(L"Error: SetWinEventHook");
    }

    HWINEVENTHOOK winObjectEventHook3 =
        SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr,
                        WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (!winObjectEventHook3) {
        Wh_Log(L"Error: SetWinEventHook");
    }

    HWINEVENTHOOK winPeekEventHook =
        SetWinEventHook(EVENT_SYSTEM_PEEKSTART, EVENT_SYSTEM_PEEKEND, nullptr,
                        PeekEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (!winPeekEventHook) {
        Wh_Log(L"Error: SetWinEventHook for PEEK");
    }

    BOOL bRet;
    MSG msg;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            msg.wParam = 0;
            break;
        }

        if (msg.hwnd == NULL && msg.message == WM_APP) {
            PostQuitMessage(0);
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (winObjectEventHook1) {
        UnhookWinEvent(winObjectEventHook1);
    }

    if (winObjectEventHook2) {
        UnhookWinEvent(winObjectEventHook2);
    }

    if (winObjectEventHook3) {
        UnhookWinEvent(winObjectEventHook3);
    }

    if (winPeekEventHook) {
        UnhookWinEvent(winPeekEventHook);
    }

    g_specialViewMode.Reset();
    g_monitorState.Clear();

    return 0;
}

void PopulateMonitorState() {
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    DWORD dwTaskbarThreadId =
        hTaskbarWnd ? GetWindowThreadProcessId(hTaskbarWnd, nullptr) : 0;
    HWND hShellWindow = GetShellWindow();

    // Hold the lock for the entire repopulation to ensure atomicity.
    // Also caches taskbar thread ID and shell window for event processing.
    auto lock = g_monitorState.BeginRepopulate(dwTaskbarThreadId, hShellWindow);

    auto enumWindowsProc = [&](HWND hWnd) -> BOOL {
        if (!IsWindowActiveCandidate(hWnd, dwTaskbarThreadId, hShellWindow)) {
            return TRUE;
        }

        HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        DynamicWindowState dynamicState = GetWindowDynamicState(hWnd, monitor);
        if (dynamicState.isMaximized || dynamicState.touchesTaskbar) {
            Wh_Log(L"Initial scan: tracking %s",
                   GetWindowLogInfo(hWnd).c_str());
            g_monitorState.AddWindowWhileLocked(
                hWnd, monitor, dynamicState.isMaximized,
                dynamicState.touchesTaskbar);
        }

        return TRUE;
    };

    EnumWindows(EnumWindowsProcThunk<decltype(enumWindowsProc)>,
                reinterpret_cast<LPARAM>(&enumWindowsProc));
}

void EnsureMonitoringThreadStarted() {
    if (g_settings.applyWhen == ApplyWhen::always || g_winEventHookThread) {
        return;
    }

    std::lock_guard<std::mutex> guard(g_winEventHookThreadMutex);
    if (!g_winEventHookThread) {
        PopulateMonitorState();
        g_winEventHookThread =
            CreateThread(nullptr, 0, WinEventHookThread, nullptr, 0, nullptr);
    }
}

BOOL WINAPI SetWindowCompositionAttribute_Hook(
    HWND hWnd,
    const WINDOWCOMPOSITIONATTRIBDATA* pAttrData) {
    auto original = [=]() {
        return SetWindowCompositionAttribute_Original(hWnd, pAttrData);
    };

    if (pAttrData->attrib != WCA_ACCENT_POLICY) {
        return original();
    }

    DWORD dwProcessId = 0;
    if (!GetWindowThreadProcessId(hWnd, &dwProcessId) ||
        dwProcessId != GetCurrentProcessId()) {
        return original();
    }

    if (!IsTaskbarWindow(hWnd)) {
        return original();
    }

    EnsureMonitoringThreadStarted();
    return ApplyTaskbarStyleForWindow(hWnd);
}

void LoadSettings() {
    PCWSTR backgroundStyle = Wh_GetStringSetting(L"backgroundStyle");
    g_settings.style.backgroundStyle = BackgroundStyle::blur;
    if (wcscmp(backgroundStyle, L"default") == 0) {
        g_settings.style.backgroundStyle = BackgroundStyle::defaultStyle;
    } else if (wcscmp(backgroundStyle, L"acrylicBlur") == 0) {
        g_settings.style.backgroundStyle = BackgroundStyle::acrylicBlur;
    } else if (wcscmp(backgroundStyle, L"color") == 0) {
        g_settings.style.backgroundStyle = BackgroundStyle::color;
    } else if (wcscmp(backgroundStyle, L"transparent") == 0) {
        g_settings.style.backgroundStyle = BackgroundStyle::transparent;
    }
    Wh_FreeStringSetting(backgroundStyle);

    g_settings.style.color = ReadArgbColorSetting(L"color");

    PCWSTR applyWhen = Wh_GetStringSetting(L"applyWhen");
    g_settings.applyWhen = ApplyWhen::maximized;
    if (wcscmp(applyWhen, L"always") == 0) {
        g_settings.applyWhen = ApplyWhen::always;
    } else if (wcscmp(applyWhen, L"dynamic") == 0 ||
               wcscmp(applyWhen, L"maximizedOrTouchingTaskbar") == 0 ||
               wcscmp(applyWhen, L"taskbarCoveredByMultipleWindows") == 0) {
        g_settings.applyWhen = ApplyWhen::dynamic;
    }
    Wh_FreeStringSetting(applyWhen);

    g_settings.excludedPrograms.clear();

    for (int i = 0;; i++) {
        PCWSTR program = Wh_GetStringSetting(L"excludedPrograms[%d]", i);

        bool hasProgram = *program;
        if (hasProgram) {
            std::wstring programUpper = program;
            LCMapStringEx(
                LOCALE_NAME_USER_DEFAULT, LCMAP_UPPERCASE, &programUpper[0],
                static_cast<int>(programUpper.length()), &programUpper[0],
                static_cast<int>(programUpper.length()), nullptr, nullptr, 0);

            g_settings.excludedPrograms.insert(std::move(programUpper));
        }

        Wh_FreeStringSetting(program);

        if (!hasProgram) {
            break;
        }
    }

    if (Wh_GetIntSetting(L"styleForDarkMode.use")) {
        TaskbarStyle style;

        PCWSTR backgroundStyle =
            Wh_GetStringSetting(L"styleForDarkMode.backgroundStyle");
        style.backgroundStyle = BackgroundStyle::blur;
        if (wcscmp(backgroundStyle, L"default") == 0) {
            style.backgroundStyle = BackgroundStyle::defaultStyle;
        } else if (wcscmp(backgroundStyle, L"acrylicBlur") == 0) {
            style.backgroundStyle = BackgroundStyle::acrylicBlur;
        } else if (wcscmp(backgroundStyle, L"color") == 0) {
            style.backgroundStyle = BackgroundStyle::color;
        } else if (wcscmp(backgroundStyle, L"transparent") == 0) {
            style.backgroundStyle = BackgroundStyle::transparent;
        }
        Wh_FreeStringSetting(backgroundStyle);

        style.color = ReadArgbColorSetting(L"styleForDarkMode.color");

        g_settings.darkModeStyle = std::move(style);
    } else {
        g_settings.darkModeStyle.reset();
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L">");

    LoadSettings();

    HMODULE hUser32 =
        LoadLibraryEx(L"user32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!hUser32) {
        Wh_Log(L"Error loading user32.dll");
        return FALSE;
    }

    SetWindowCompositionAttribute_t pSetWindowCompositionAttribute =
        (SetWindowCompositionAttribute_t)GetProcAddress(
            hUser32, "SetWindowCompositionAttribute");
    if (!pSetWindowCompositionAttribute) {
        Wh_Log(L"Error getting SetWindowCompositionAttribute");
        return FALSE;
    }

    WindhawkUtils::Wh_SetFunctionHookT(pSetWindowCompositionAttribute,
                                       SetWindowCompositionAttribute_Hook,
                                       &SetWindowCompositionAttribute_Original);

    pGetWindowBand = (GetWindowBand_t)GetProcAddress(hUser32, "GetWindowBand");

    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L">");

    WNDCLASS wndclass;
    if (GetClassInfo(GetModuleHandle(nullptr), L"Shell_TrayWnd", &wndclass)) {
        UpdateAllTaskbarStyles();
    }
}

void Wh_ModUninit() {
    Wh_Log(L">");

    if (g_winEventHookThread) {
        PostThreadMessage(GetThreadId(g_winEventHookThread), WM_APP, 0, 0);
        WaitForSingleObject(g_winEventHookThread, INFINITE);
        CloseHandle(g_winEventHookThread);
        g_winEventHookThread = nullptr;
    }

    std::unordered_set<HWND> secondaryTaskbarWindows;
    HWND hWnd = FindTaskbarWindows(&secondaryTaskbarWindows);
    if (hWnd) {
        ResetTaskbarStyle(hWnd);
    }

    for (HWND hSecondaryWnd : secondaryTaskbarWindows) {
        ResetTaskbarStyle(hSecondaryWnd);
    }

    {
        std::lock_guard<std::mutex> lock(g_transparentXamlTaskbarWindowsMutex);
        g_transparentXamlTaskbarWindows.clear();
    }
    g_transparentXamlActive = false;
    ApplyTransparentStateToKnownRectangles(false);

    if (g_visualTreeWatcher) {
        g_visualTreeWatcher->UnadviseVisualTreeChange();
        g_visualTreeWatcher = nullptr;
    }
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");

    LoadSettings();

    {
        std::lock_guard<std::mutex> guard(g_winEventHookThreadMutex);

        if (g_winEventHookThread) {
            PostThreadMessage(GetThreadId(g_winEventHookThread), WM_APP, 0, 0);
            WaitForSingleObject(g_winEventHookThread, INFINITE);
            CloseHandle(g_winEventHookThread);
            g_winEventHookThread = nullptr;
        }
    }

    UpdateAllTaskbarStyles();
}
