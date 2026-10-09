// ==WindhawkMod==
// @id              asteski-task-view-customizer
// @name            Task View Customizer
// @description     Customize Windows 11 Task View layouts, backgrounds, colors, desktop highlights and animations
// @version         0.1.0
// @author          Asteski
// @github          https://github.com/Asteski
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Task View Customizer

Customize Windows 11 Task View's layout and appearance using its existing
virtual desktop thumbnails and window overview. Choose where the desktop
list appears, style its backgrounds and highlights, and adjust its behavior
without replacing the native controls.

## Task View modes

- **0 — Windows default**: shows the window overview above the desktop list
  at the bottom, with the native slide direction. This is the default mode;
  the mod's layout and appearance controls still apply.
- **1 — Full Task View with desktops at the top**: moves the desktop list above
  the window overview, with entrance and exit motion from the top.
- **2 — No Task View with Desktop list in the top**: shows only the desktop list
  near the top of the screen.
- **3 — No Task View with Desktop list at the center**: shows only the desktop
  list, centered on the screen.
- **4 — No Task View with Desktop list at the bottom**: shows only the desktop
  list near the bottom, with entrance and exit motion from the bottom.

The mod does not change Alt+Tab or the desktop preview shown when hovering
over the taskbar's Task View button.

## Settings

### Task View behavior

Choose the mode or hide the New desktop button in any mode.
**Close Task View when Windows key is released** also applies to every mode:
hold Windows while using Task View, then release it to close.
Opening Task View by clicking its taskbar button keeps it open unless you
hold and release Windows afterward. In list-only modes, Tab navigates
desktops and clicking outside the list closes it.

### Desktop list layout

Choose full width, fit the desktop tiles, or set a percentage of the screen.
Adjust the frame margin. These controls apply to **all five modes**, including
Windows default. The New desktop button stays
on the right while existing desktops scroll in the remaining space.
The top layout includes 32 pixels of taskbar clearance. Windows default mode
keeps the desktop list at the bottom while applying these sizing controls.

### Background materials

Choose **Native**, **Solid color**, **Acrylic**, or **Clear**. Solid starts
opaque and can be made translucent without blur.
Acrylic uses a live blurred backdrop and tint. Clear removes the material.
**Disable desktop list background** hides the strip's background in every
mode. **Remove desktop list background border** removes its outer frame
without removing desktop selection borders.

Material transparency accepts **0-100**. Zero keeps the material's default
strength; 100 makes Native, Solid and Acrylic backgrounds clear. Values
outside that range use default strength; **-1** is the default setting.
Thumbnails and labels remain visible, and native hover/focus brush alpha is
preserved rather than turning faint highlights opaque.

Windows can also use Acrylic's solid fallback when transparency is disabled.
Hiding the desktop list background takes priority over material styling.
The experimental Mica/local-blur option has been removed because it caused
Explorer crashes. No custom panel-local blur implementation is currently
provided.

### Desktop highlights

Keep Windows' highlight or choose border only, background only, or both.
The highlight background opacity controls the hovered/selected desktop fill.

### Theme colors

Set background and border colors independently for dark and light mode.
Accepts **native**, **accent**, **#RRGGBB**, or **#AARRGGBB**.
`accent` follows the current Windows accent color; invalid values preserve
native colors. Background colors also provide Acrylic tint and custom
highlight fill. Border colors apply to panel borders and custom highlights.

### Full-screen backdrop

Keep the native backdrop, clear it, or use Acrylic with adjustable dimming.
**Disable full-screen background blur** applies to every mode, even when
Windows transparency is enabled. It is independent of the desktop strip's
background. An explicit **Backdrop Blur** choice overrides this switch;
choose **Off** to keep the full-screen backdrop clear.

### Animations and scrolling

**Disable Task View animations** applies to all modes independently of the
background material. It also makes desktop reveal scrolling immediate.
With animations enabled, adjust desktop reveal scrolling speed from 10-500%:
100% uses Windows' current speed, 50% is half speed, and 200% is double speed.

## Installation and use

Install as a local Windhawk mod, compile and enable it. Close and reopen
Task View after changing mode, materials or colors. Disabling the mod restores
saved native layout properties, brushes and bindings.

For reliable Task View detection, keep **Windows 11 Taskbar Styler** or
another Explorer XAML diagnostics consumer enabled. The mod observes existing
diagnostics lookups; it does not register its own consumer or change another
mod's settings. Windhawk downloads Windows debugging symbols when needed.

## Compatibility

Designed for Windows 11. Windows builds can change Task View's internal
control names and layout; unrecognized controls are left alone. Mixed-DPI
monitors, desktop drag and drop, and interactions
with other shell customization mods can depend on the Windows build.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- General:
    - viewMode: "0"
      $name: Task View mode
      $options:
      - "0": Windows default
      - "1": Full Task View with desktops at the top
      - "2": No Task View with Desktop list in the top
      - "3": No Task View with Desktop list at the center
      - "4": No Task View with Desktop list at the bottom
      $description: "Windows default places desktops at the bottom with the native entrance direction. Desktop-list sizing applies to all modes. List-only modes hide Task View's window overview and background. Reopen Task View after changing this setting."
    - hideNewDesktop: false
      $name: Hide New desktop button
    - closeOnWindowsRelease: false
      $name: Close Task View when Windows key is released
      $description: "All modes: hold Windows while using Task View, then release it to close. Task View opened by clicking the taskbar button stays open unless you hold and release Windows. Does not affect Alt+Tab."
  $name: Task View behavior
- DesktopList:
    - desktopFrameWidth: "full"
      $name: Desktop list background width
      $options:
      - full: Full available width
      - fit: Fit desktop tiles
      - percent: Percentage of screen
    - desktopFramePercent: 70
      $name: Desktop list width (% of screen)
      $description: "Used with Percentage of screen. Range: 10–100. Overflowing desktop tiles remain scrollable."
    - desktopFrameMargin: 8
      $name: Virtual desktop list frame margin (px)
      $description: "Space around the desktop list frame. Default: 8. Range: 0–200. The top margin also includes the existing 32 px taskbar clearance."
  $name: Desktop list layout
- Materials:
    - materialStyle: native
      $name: Background material style
      $options:
      - native: Native Windows material
      - solid: Solid color (no blur)
      - acrylic: Acrylic
      - clear: Clear (no background)
      $description: "Solid starts opaque; the transparency percentage can make it translucent without blur. Acrylic blurs content behind panels. Clear removes the material. Disable desktop list background takes priority."
    - materialTransparency: -1
      $name: Task View background material transparency (%)
      $description: "0 = default material strength; 100 = clear for Native, Solid and Acrylic. Thumbnails and labels stay solid. Outside 0–100 uses default strength (-1). Hidden strip settings take priority. Reopen Task View after changing."
    - removeListBorder: false
      $name: Remove desktop list background border
      $description: "Removes the outer desktop strip frame, without removing desktop selection borders."
    - disableListBackground: true
      $name: Disable desktop list background
      $description: "Hide the desktop list background in every mode, including its material and blur."
  $name: Background materials
- Highlights:
    - highlightStyle: native
      $name: Desktop highlight style
      $options:
      - native: Native Windows highlight
      - border: Border only
      - fill: Background only
      - both: Border and background
    - highlightOpacity: 20
      $name: Highlight background opacity (%)
      $description: "Accent-colored fill for hovered or selected desktops. Range: 0–100. Used with Background only and Border and background."
  $name: Desktop highlights
- Colors:
    - backgroundColorDark: "native"
      $name: Background color (dark mode)
      $description: "native, accent, #RRGGBB or #AARRGGBB. Solid defaults to #202020. Acrylic use this as their tint. Reopen Task View after changing colors."
    - borderColorDark: "native"
      $name: Border color (dark mode)
      $description: "native, accent, #RRGGBB or #AARRGGBB. Applies to panel borders and custom desktop highlights. Invalid values preserve native colors."
    - backgroundColorLight: "native"
      $name: Background color (light mode)
      $description: "native, accent, #RRGGBB or #AARRGGBB. Solid defaults to #F3F3F3."
    - borderColorLight: "native"
      $name: Border color (light mode)
      $description: "native, accent, #RRGGBB or #AARRGGBB. accent follows the current Windows accent color."
  $name: Theme colors
- Backdrop:
    - backdropBlur: native
      $name: Backdrop Blur
      $options:
      - native: Follow native background / disable-blur setting
      - off: Off (clear full-screen background)
      - acrylic: Acrylic backdrop
      $description: "Separate from panel materials. An explicit backdrop selection overrides Disable full-screen background blur. Acrylic uses a live XAML backdrop, rather than SWS's captured snapshot; Windows may use its solid fallback when system transparency is disabled."
    - backdropDimOpacity: 18
      $name: Backdrop Dim Opacity (%)
      $description: "Black tint strength of the Acrylic backdrop, 0–100. Separate from panel transparency."
    - disableFullscreenBlur: true
      $name: Disable full-screen background blur
      $description: "Keep the wallpaper clear even when Windows transparency effects are enabled. Independent of the desktop thumbnail strip background. Applies to every Task View mode. Reopen Task View after changing this setting."
  $name: Full-screen backdrop
- Animations:
    - disableListAnimations: false
      $name: Disable Task View animations
      $description: "Disables Task View animations in every mode. Independent of background material and transparency."
    - scrollAnimationSpeed: 100
      $name: Desktop scroll animation speed (%)
      $description: "100 = current Windows speed (default). 50 = half speed. 200 = double speed. Range: 10–500. Applies to horizontal scrolling that reveals a desktop tile."
  $name: Animations and scrolling
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <windhawk_utils.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.ViewManagement.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

namespace wf = winrt::Windows::Foundation;
namespace xaml = winrt::Windows::UI::Xaml;
namespace controls = winrt::Windows::UI::Xaml::Controls;
namespace media = winrt::Windows::UI::Xaml::Media;
namespace animation = winrt::Windows::UI::Xaml::Media::Animation;

namespace {
std::atomic<bool> g_stopping{false};
std::atomic<bool> g_hooksReady{false};
std::atomic_flag g_installing = ATOMIC_FLAG_INIT;
std::atomic<int> g_scrollSpeed{100};
std::atomic<int> g_desktopFrameMargin{8};
std::atomic<int> g_viewMode{0};
bool IsListOnlyMode() { int mode = g_viewMode.load(); return mode >= 2 && mode <= 4; }
std::atomic<bool> g_hideNewDesktop{false};
std::atomic<int> g_frameWidthMode{0};
std::atomic<int> g_framePercent{70};
std::atomic<bool> g_disableListAnimations{false};
std::atomic<bool> g_disableListBackground{true};
std::atomic<bool> g_disableFullscreenBlur{true};
std::atomic<int> g_materialTransparency{-1};
std::atomic<int> g_materialStyle{0}, g_highlightStyle{0}, g_highlightOpacity{20};
std::atomic<int> g_backdropBlur{0}, g_backdropDimOpacity{18};
std::atomic<bool> g_closeOnWindowsRelease{true};
std::atomic<bool> g_removeListBorder{false};
std::mutex g_colorMutex;
std::wstring g_colors[4];
std::atomic<int> g_colorRevision{0};
void LoadFrameSettings() {
    auto readChoice = [](PCWSTR key, std::initializer_list<PCWSTR> choices) {
        auto value = Wh_GetStringSetting(key);
        int result = 0, index = 0;
        for (auto choice : choices) { if (value && wcscmp(value, choice) == 0) result = index; index++; }
        Wh_FreeStringSetting(value);
        return result;
    };
    g_materialStyle = readChoice(L"Materials.materialStyle", {L"native", L"solid", L"acrylic", L"clear"});
    g_removeListBorder = Wh_GetIntSetting(L"Materials.removeListBorder") != 0;
    PCWSTR colorKeys[] = {L"Colors.backgroundColorDark", L"Colors.backgroundColorLight", L"Colors.borderColorDark", L"Colors.borderColorLight"};
    { std::lock_guard lock(g_colorMutex);
      for (int i = 0; i < 4; i++) {
          auto value = Wh_GetStringSetting(colorKeys[i]);
          g_colors[i] = value ? value : L"native";
          Wh_FreeStringSetting(value);
      }
    }
    ++g_colorRevision;
    g_highlightStyle = readChoice(L"Highlights.highlightStyle", {L"native", L"border", L"fill", L"both"});
    g_highlightOpacity = (std::max)(0, (std::min)(100, Wh_GetIntSetting(L"Highlights.highlightOpacity")));
    g_backdropBlur = readChoice(L"Backdrop.backdropBlur", {L"native", L"off", L"acrylic"});
    g_backdropDimOpacity = (std::max)(0, (std::min)(100, Wh_GetIntSetting(L"Backdrop.backdropDimOpacity")));
    g_closeOnWindowsRelease = Wh_GetIntSetting(L"General.closeOnWindowsRelease") != 0;
    g_disableListAnimations = Wh_GetIntSetting(L"Animations.disableListAnimations") != 0;
    g_disableListBackground = Wh_GetIntSetting(L"Materials.disableListBackground") != 0;
    g_disableFullscreenBlur = Wh_GetIntSetting(L"Backdrop.disableFullscreenBlur") != 0;
    // Do not clamp: an out-of-range value explicitly requests native opacity.
    g_materialTransparency = Wh_GetIntSetting(L"Materials.materialTransparency");
    g_hideNewDesktop = Wh_GetIntSetting(L"General.hideNewDesktop") != 0;
    auto width = Wh_GetStringSetting(L"DesktopList.desktopFrameWidth");
    g_frameWidthMode = width && wcscmp(width, L"fit") == 0 ? 1 :
                       width && wcscmp(width, L"percent") == 0 ? 2 : 0;
    Wh_FreeStringSetting(width);
    g_framePercent = (std::max)(10, (std::min)(Wh_GetIntSetting(L"DesktopList.desktopFramePercent"), 100));
}
std::atomic<int> g_nativeScrollDuration{250};
thread_local bool g_insideLayout = false;
thread_local bool g_insideScroll = false;

struct LayoutGuard {
    LayoutGuard() { g_insideLayout = true; }
    ~LayoutGuard() { g_insideLayout = false; }
};

// Preserve local values and bindings, not just effective values. Clearing an
// originally unset property restores the theme/style's ownership of it.
struct SavedProperty {
    xaml::DependencyProperty property{nullptr};
    wf::IInspectable local{nullptr};
    xaml::Data::Binding binding{nullptr};

    SavedProperty() = default;
    SavedProperty(xaml::FrameworkElement const& element,
                  xaml::DependencyProperty const& dependencyProperty)
        : property(dependencyProperty), local(element.ReadLocalValue(property)) {
        if (auto expression = element.GetBindingExpression(property)) {
            binding = expression.ParentBinding();
        }
    }

    void Restore(xaml::FrameworkElement const& element) const {
        if (binding) {
            element.SetBinding(property, binding);
        } else if (local == xaml::DependencyProperty::UnsetValue()) {
            element.ClearValue(property);
        } else {
            element.SetValue(property, local);
        }
    }
};

bool SameMargin(xaml::Thickness const& a, xaml::Thickness const& b) {
    return a.Left == b.Left && a.Top == b.Top && a.Right == b.Right &&
           a.Bottom == b.Bottom;
}

void ClearAnimationTargets();

struct ScrollCallGuard {
    ScrollCallGuard() { g_insideScroll = true; }
    ~ScrollCallGuard() { g_insideScroll = false; }
};

struct ScrollMotion : std::enable_shared_from_this<ScrollMotion> {
    winrt::weak_ref<controls::ScrollViewer> scroll;
    winrt::event_token renderingToken{}, viewChangedToken{};
    bool rendering = false;
    bool observing = false;
    ULONGLONG start = 0, nativeStart = 0;
    double from = 0, target = 0, duration = 0;

    void CancelRendering() {
        if (rendering) {
            media::CompositionTarget::Rendering(renderingToken);
            rendering = false;
        }
    }
    void Stop() {
        CancelRendering();
        if (observing) {
            if (auto control = scroll.get()) control.ViewChanged(viewChangedToken);
            observing = false;
        }
    }
    bool Request(double offset, int speed) {
        auto control = scroll.get();
        if (!control) return false;
        offset = (std::max)(0.0, (std::min)(offset, control.ScrollableWidth()));
        if (g_disableListAnimations) {
            CancelRendering();
            nativeStart = 0;
            ScrollCallGuard guard;
            return control.ChangeView(wf::IReference<double>{offset}, nullptr, nullptr, true);
        }
        if (!observing) {
            viewChangedToken = control.ViewChanged([weak = weak_from_this()](auto const&, auto const& args) {
                if (auto current = weak.lock(); current && current->nativeStart && !args.IsIntermediate()) {
                    auto elapsed = GetTickCount64() - current->nativeStart;
                    current->nativeStart = 0;
                    if (elapsed >= 40 && elapsed <= 2000) {
                        g_nativeScrollDuration = static_cast<int>(elapsed);
                        Wh_SetIntValue(L"nativeScrollDurationMs", static_cast<int>(elapsed));
                    }
                }
            });
            observing = true;
        }
        if (speed == 100) {
            CancelRendering();
            nativeStart = GetTickCount64();
            ScrollCallGuard guard;
            bool changed = control.ChangeView(wf::IReference<double>{offset}, nullptr, nullptr, false);
            if (!changed) nativeStart = 0;
            return changed;
        }
        nativeStart = 0;
        if (rendering && target == offset) return true;
        CancelRendering();
        from = control.HorizontalOffset();
        target = offset;
        if (std::abs(target - from) < 0.5) return false;
        duration = static_cast<double>(g_nativeScrollDuration.load()) * 100.0 / speed;
        start = GetTickCount64();
        renderingToken = media::CompositionTarget::Rendering([weak = weak_from_this()](auto const&, auto const&) {
            auto current = weak.lock();
            if (!current) return;
            try {
                auto control = current->scroll.get();
                if (!control || g_stopping) { current->CancelRendering(); return; }
                double progress = (std::min)(1.0, (GetTickCount64() - current->start) / current->duration);
                double remaining = 1.0 - progress;
                double eased = 1.0 - remaining * remaining * remaining;
                double offset = current->from + (current->target - current->from) * eased;
                { ScrollCallGuard guard;
                  control.ChangeView(wf::IReference<double>{offset}, nullptr, nullptr, true); }
                if (progress >= 1) current->CancelRendering();
            } catch (...) { current->CancelRendering(); }
        });
        rendering = true;
        return true;
    }
};

template <typename T>
T FindNamedChild(xaml::DependencyObject const& parent, wchar_t const* name, int depth = 0) {
    if (!parent || depth > 16) return T{nullptr};
    if (auto element = parent.try_as<xaml::FrameworkElement>(); element && element.Name() == name)
        return parent.try_as<T>();
    int count = media::VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        if (auto found = FindNamedChild<T>(media::VisualTreeHelper::GetChild(parent, i), name, depth + 1))
            return found;
    }
    return T{nullptr};
}

xaml::FrameworkElement FindDesktopTile(xaml::DependencyObject const& parent, int depth = 0) {
    if (!parent || depth > 16) return nullptr;
    if (winrt::get_class_name(parent) ==
        L"WindowsInternal.ComposableShell.Experiences.Switcher.VirtualDesktopElementThemed")
        return parent.try_as<xaml::FrameworkElement>();
    int count = media::VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        if (auto found = FindDesktopTile(media::VisualTreeHelper::GetChild(parent, i), depth + 1))
            return found;
    }
    return nullptr;
}

struct MaterialState {
    winrt::weak_ref<xaml::FrameworkElement> element;
    SavedProperty property;
    media::Brush original{nullptr}, replacement{nullptr};
    int percent = -1;
    int style = -1;
    int revision = -1;
    bool dark = false;
    winrt::Windows::UI::Color color{};
    bool applied = false;
    void Restore() {
        if (!applied) return;
        if (auto current = element.get()) property.Restore(current);
        applied = false;
        percent = -1;
        style = -1;
        replacement = nullptr;
    }
};

using Color = winrt::Windows::UI::Color;

bool DarkTheme(xaml::FrameworkElement const& element) {
    return element.ActualTheme() == xaml::ElementTheme::Dark;
}
Color AccentColor() {
    return winrt::Windows::UI::ViewManagement::UISettings().GetColorValue(
        winrt::Windows::UI::ViewManagement::UIColorType::Accent);
}
bool CustomColor(xaml::FrameworkElement const& element, bool border, Color& color) {
    std::wstring value;
    { std::lock_guard lock(g_colorMutex); value = g_colors[(border ? 2 : 0) + (DarkTheme(element) ? 0 : 1)]; }
    if (_wcsicmp(value.c_str(), L"accent") == 0) { color = AccentColor(); return true; }
    if (value.size() != 7 && value.size() != 9) return false;
    if (value[0] != L'#') return false;
    uint32_t number = 0;
    for (size_t i = 1; i < value.size(); i++) {
        wchar_t c = value[i];
        int digit = c >= L'0' && c <= L'9' ? c - L'0' :
            c >= L'a' && c <= L'f' ? c - L'a' + 10 : c >= L'A' && c <= L'F' ? c - L'A' + 10 : -1;
        if (digit < 0) return false;
        number = (number << 4) | digit;
    }
    color = {static_cast<uint8_t>(value.size() == 7 ? 255 : number >> 24),
             static_cast<uint8_t>(number >> 16), static_cast<uint8_t>(number >> 8), static_cast<uint8_t>(number)};
    return true;
}

struct BorderState {
    winrt::weak_ref<controls::Border> element;
    SavedProperty brush, thickness;
    media::SolidColorBrush replacement{nullptr};
    bool applied = false;
    bool removed = false;
    void Restore() {
        if (applied) if (auto target = element.get()) { brush.Restore(target); thickness.Restore(target); }
        applied = false;
        removed = false;
    }
};
void ApplyBorders(xaml::DependencyObject const& node, std::vector<BorderState>& saved, int depth = 0) {
    if (!node || depth > 24) return;
    if (auto border = node.try_as<controls::Border>()) {
        bool frame = border.Name() == L"VirtualDesktopBarBackground" || border.Name() == L"VirtualDesktopSwitcherBackground";
        bool remove = frame && g_removeListBorder;
        Color color{};
        bool custom = CustomColor(border, true, color);
        // Custom highlight borders have their own saved state.
        if (!(g_highlightStyle != 0 && border.Name() == L"MainBorder")) {
            BorderState* state = nullptr;
            for (auto& candidate : saved) if (candidate.element.get() == border) { state = &candidate; break; }
            if (!state && (remove || custom)) {
                saved.emplace_back(); state = &saved.back(); state->element = winrt::make_weak(border);
                state->brush = SavedProperty(border, controls::Border::BorderBrushProperty());
                state->thickness = SavedProperty(border, controls::Border::BorderThicknessProperty());
            }
            if (state) {
                if (!remove && !custom) state->Restore();
                else {
                    if (remove) {
                        if (!SameMargin(border.BorderThickness(), {})) border.BorderThickness({});
                    } else if (state->removed) state->thickness.Restore(border);
                    state->removed = remove;
                    if (custom) {
                        if (!state->replacement) state->replacement = media::SolidColorBrush(color);
                        else state->replacement.Color(color);
                        if (border.BorderBrush() != state->replacement) border.BorderBrush(state->replacement);
                    } else if (state->applied) state->brush.Restore(border);
                    state->applied = true;
                }
            }
        }
    }
    for (int i = 0, count = media::VisualTreeHelper::GetChildrenCount(node); i < count; i++)
        ApplyBorders(media::VisualTreeHelper::GetChild(node, i), saved, depth + 1);
}

struct HighlightState {
    winrt::weak_ref<xaml::FrameworkElement> tile, container;
    winrt::weak_ref<controls::Grid> panel;
    winrt::weak_ref<controls::Border> border;
    SavedProperty background, borderBrush, borderThickness, primaryFocus, secondaryFocus;
    SavedProperty tilePrimaryFocus, tileSecondaryFocus;
    winrt::event_token entered{}, exited{};
    bool hovered = false, applied = false;
    media::SolidColorBrush fill{nullptr}, accent{nullptr}, clear{nullptr};
    void Restore() {
        if (!applied) return;
        if (auto current = panel.get()) background.Restore(current);
        if (auto current = border.get()) { borderBrush.Restore(current); borderThickness.Restore(current); }
        if (auto current = container.get()) { primaryFocus.Restore(current); secondaryFocus.Restore(current); }
        if (auto current = tile.get()) { tilePrimaryFocus.Restore(current); tileSecondaryFocus.Restore(current); }
        applied = false;
    }
    void Stop() {
        Restore();
        if (auto current = tile.get()) { current.PointerEntered(entered); current.PointerExited(exited); }
    }
};

void ApplyMaterials(xaml::DependencyObject const& node,
                    std::vector<MaterialState>& saved, int depth = 0) {
    if (!node || depth > 24) return;
    auto element = node.try_as<xaml::FrameworkElement>();
    if (element) {
        auto name = element.Name();
        // These visuals are owned by the independent background-hide options.
        bool hiddenStrip = g_disableListBackground &&
            (name == L"VirtualDesktopBarBackground" || name == L"VirtualDesktopSwitcherBackground");
        if (hiddenStrip) return;
        xaml::DependencyProperty property{nullptr};
        if (element.try_as<controls::Border>()) property = controls::Border::BackgroundProperty();
        else if (element.try_as<controls::Panel>()) property = controls::Panel::BackgroundProperty();
        else if (element.try_as<controls::Control>()) property = controls::Control::BackgroundProperty();
        if (g_highlightStyle != 0 && name == L"MainGrid") {
            auto ancestor = media::VisualTreeHelper::GetParent(element);
            for (int level = 0; ancestor && level < 6; level++) {
                if (winrt::get_class_name(ancestor) ==
                    L"WindowsInternal.ComposableShell.Experiences.Switcher.VirtualDesktopElementThemed") {
                    property = nullptr; // Owned by the independent highlight setting.
                    break;
                }
                ancestor = media::VisualTreeHelper::GetParent(ancestor);
            }
        }
        if (property) {
            auto brush = element.GetValue(property).try_as<media::Brush>();
            MaterialState* state = nullptr;
            for (auto& candidate : saved) {
                if (candidate.element.get() == element) { state = &candidate; break; }
            }
            int percent = g_materialTransparency.load();
            int style = g_materialStyle.load();
            bool frame = name == L"VirtualDesktopBarBackground" || name == L"VirtualDesktopSwitcherBackground";
            Color configured{};
            bool custom = CustomColor(element, false, configured);
            if (brush) {
                // Capture changes from native visual states or other stylers.
                if (!state || (brush != state->original && brush != state->replacement)) {
                    auto solid = brush.try_as<media::SolidColorBrush>();
                    auto acrylic = brush.try_as<media::AcrylicBrush>();
                    // Leave intentionally clear surfaces and image brushes alone.
                    if (acrylic || (solid && ((solid.Color().A != 0 && brush.Opacity() != 0) || frame))) {
                        if (!state) {
                            saved.emplace_back();
                            state = &saved.back();
                        }
                        state->element = winrt::make_weak(element);
                        state->property = SavedProperty(element, property);
                        state->original = brush;
                        state->replacement = nullptr;
                        state->percent = -1;
                        state->style = -1;
                        state->applied = false;
                        double nativeOpacity = acrylic ? acrylic.TintOpacity() : solid.Color().A / 255.0;
                        Wh_Log(L"Material %s: native tint/color transparency %.1f%%, brush opacity %.3f",
                               name.c_str(), (1.0 - nativeOpacity) * 100, brush.Opacity());
                    }
                }
                if (state && style == 0 && !custom && (percent < 0 || percent > 100)) {
                    state->Restore();
                } else if (state && (state->percent != percent || state->style != style ||
                           state->revision != g_colorRevision || state->color != configured || state->dark != DarkTheme(element))) {
                    double opacity = percent >= 0 && percent <= 100 ? 1.0 - percent / 100.0 : 1.0;
                    auto acrylic = state->original.try_as<media::AcrylicBrush>();
                    auto solid = state->original.try_as<media::SolidColorBrush>();
                    auto color = acrylic ? acrylic.TintColor() : solid ? solid.Color() : winrt::Windows::UI::Color{255, 32, 32, 32};
                    // Material conversion is limited to base panels; faint solid
                    // hover brushes retain their color and alpha.
                    bool basePanel = frame || acrylic || (solid && solid.Color().A * solid.Opacity() >= 128);
                    if (basePanel && custom) color = configured;
                    if (style == 3 && basePanel) {
                        state->replacement = media::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0});
                    } else if (style == 1 && basePanel) {
                        if (!custom) color = DarkTheme(element) ? Color{255, 32, 32, 32} : Color{255, 243, 243, 243};
                        color.A = static_cast<uint8_t>(std::lround(color.A * opacity));
                        state->replacement = media::SolidColorBrush(color);
                    } else if ((style == 2 && basePanel) || (style == 0 && acrylic)) {
                        auto original = acrylic;
                        media::AcrylicBrush replacement;
                        replacement.BackgroundSource(original ? original.BackgroundSource() : media::AcrylicBackgroundSource::Backdrop);
                        replacement.AlwaysUseFallback(original ? original.AlwaysUseFallback() : false);
                        replacement.TintColor(color);
                        replacement.TintOpacity(original ? original.TintOpacity() : 0.65);
                        if (original) replacement.TintLuminosityOpacity(original.TintLuminosityOpacity());
                        auto fallback = original && !custom ? original.FallbackColor() : color;
                        replacement.FallbackColor(fallback);
                        // Fade the material itself, including blur/noise. Tint
                        // alone may remain opaque due to the luminosity layer.
                        replacement.Opacity(state->original.Opacity() * opacity);
                        state->replacement = replacement;
                    } else if (auto original = state->original.try_as<media::SolidColorBrush>()) {
                        auto color = original.Color();
                        if (basePanel && custom) color = configured;
                        // Hover/focus brushes often use faint white. Increasing
                        // their alpha to the requested opacity creates white tiles.
                        color.A = static_cast<uint8_t>(std::lround(color.A * opacity));
                        state->replacement = media::SolidColorBrush(color);
                        state->replacement.Opacity(original.Opacity());
                    }
                    if (state->replacement) {
                        element.SetValue(property, state->replacement);
                        state->percent = percent;
                        state->style = style;
                        state->revision = g_colorRevision;
                        state->dark = DarkTheme(element);
                        state->color = configured;
                        state->applied = true;
                    }
                }
            }
        }
    }
    int count = media::VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < count; i++)
        ApplyMaterials(media::VisualTreeHelper::GetChild(node, i), saved, depth + 1);
}

struct LayoutState {
    std::vector<std::shared_ptr<HighlightState>> highlights;
    media::AcrylicBrush backdropAcrylic{nullptr};
    void ApplyHighlights() {
        auto list = FindNamedChild<controls::ListView>(desktops.get(), L"DesktopsList");
        if (!list) return;
        for (uint32_t i = 0; i < list.Items().Size(); i++) {
            auto container = list.ContainerFromIndex(i).try_as<xaml::FrameworkElement>();
            auto tile = FindDesktopTile(container);
            if (!tile || !container) continue;
            std::shared_ptr<HighlightState> saved;
            for (auto const& current : highlights) if (current->tile.get() == tile) { saved = current; break; }
            if (!saved) {
                if (g_highlightStyle == 0) continue;
                auto panel = FindNamedChild<controls::Grid>(tile, L"MainGrid");
                auto border = FindNamedChild<controls::Border>(tile, L"MainBorder");
                if (!panel || !border) continue;
                saved = std::make_shared<HighlightState>();
                saved->tile = winrt::make_weak(tile); saved->container = winrt::make_weak(container);
                saved->panel = winrt::make_weak(panel); saved->border = winrt::make_weak(border);
                saved->background = SavedProperty(panel, controls::Panel::BackgroundProperty());
                saved->borderBrush = SavedProperty(border, controls::Border::BorderBrushProperty());
                saved->borderThickness = SavedProperty(border, controls::Border::BorderThicknessProperty());
                saved->primaryFocus = SavedProperty(container, xaml::FrameworkElement::FocusVisualPrimaryThicknessProperty());
                saved->secondaryFocus = SavedProperty(container, xaml::FrameworkElement::FocusVisualSecondaryThicknessProperty());
                saved->tilePrimaryFocus = SavedProperty(tile, xaml::FrameworkElement::FocusVisualPrimaryThicknessProperty());
                saved->tileSecondaryFocus = SavedProperty(tile, xaml::FrameworkElement::FocusVisualSecondaryThicknessProperty());
                auto color = winrt::Windows::UI::Color{255, 0, 120, 215};
                try { color = winrt::unbox_value<winrt::Windows::UI::Color>(xaml::Application::Current().Resources().Lookup(winrt::box_value(L"SystemAccentColor"))); } catch (...) {}
                saved->accent = media::SolidColorBrush(color);
                color.A = static_cast<uint8_t>(g_highlightOpacity * 255 / 100);
                saved->fill = media::SolidColorBrush(color);
                saved->clear = media::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0});
                saved->entered = tile.PointerEntered([weak = std::weak_ptr(saved)](auto const&, auto const&) {
                    if (auto current = weak.lock()) current->hovered = true;
                });
                saved->exited = tile.PointerExited([weak = std::weak_ptr(saved)](auto const&, auto const&) {
                    if (auto current = weak.lock()) current->hovered = false;
                });
                highlights.push_back(saved);
            }
            int style = g_highlightStyle.load();
            if (style == 0) { saved->Restore(); continue; }
            auto panel = saved->panel.get(); auto border = saved->border.get();
            if (!panel || !border) continue;
            bool active = saved->hovered || list.SelectedIndex() == static_cast<int>(i);
            auto color = AccentColor();
            CustomColor(border, true, color);
            if (saved->accent.Color() != color) saved->accent.Color(color);
            color = AccentColor();
            CustomColor(panel, false, color);
            color.A = static_cast<uint8_t>(color.A * g_highlightOpacity / 100);
            if (saved->fill.Color() != color) saved->fill.Color(color);
            auto background = active && style != 1 ? saved->fill : saved->clear;
            if (panel.Background() != background) panel.Background(background);
            auto brush = active && style != 2 ? saved->accent : saved->clear;
            if (border.BorderBrush() != brush) border.BorderBrush(brush);
            xaml::Thickness thickness = active && style != 2 ? xaml::Thickness{2, 2, 2, 2} : xaml::Thickness{};
            if (!SameMargin(border.BorderThickness(), thickness)) border.BorderThickness(thickness);
            if (!SameMargin(container.FocusVisualPrimaryThickness(), {})) container.FocusVisualPrimaryThickness({});
            if (!SameMargin(container.FocusVisualSecondaryThickness(), {})) container.FocusVisualSecondaryThickness({});
            if (!SameMargin(tile.FocusVisualPrimaryThickness(), {})) tile.FocusVisualPrimaryThickness({});
            if (!SameMargin(tile.FocusVisualSecondaryThickness(), {})) tile.FocusVisualSecondaryThickness({});
            saved->applied = true;
        }
    }
    bool windowsHeld = false;
    winrt::event_token releaseToken{};
    bool DismissList() {
        HWND foreground = GetForegroundWindow();
        DWORD process = 0;
        DWORD thread = GetWindowThreadProcessId(foreground, &process);
        if (process != GetCurrentProcessId()) return false;
        GUITHREADINFO info{sizeof(info)};
        HWND target = foreground;
        if (GetGUIThreadInfo(thread, &info) && info.hwndFocus) target = info.hwndFocus;
        PostMessageW(target, WM_KEYDOWN, VK_ESCAPE, 0x00010001);
        PostMessageW(target, WM_KEYUP, VK_ESCAPE, 0xC0010001);
        return true;
    }
    void Navigate(int direction) {
        auto stack = desktopStack.get();
        auto list = FindNamedChild<controls::ListView>(stack, L"DesktopsList");
        if (!list || list.Items().Size() == 0) return;
        int count = static_cast<int>(list.Items().Size());
        int index = list.SelectedIndex();
        int next = index < 0 ? (direction < 0 ? count - 1 : 0) : (index + direction + count) % count;
        list.SelectedIndex(next);
        list.ScrollIntoView(list.Items().GetAt(next));
        if (auto container = list.ContainerFromIndex(next).try_as<controls::Control>())
            container.Focus(xaml::FocusState::Keyboard);
        Wh_SetIntValue(L"lastTabDesktop", next);
    }
    winrt::event_token keyToken{};
    winrt::event_token outsideClickToken{};
    SavedProperty originalInputBackground;
    bool inputBackgroundApplied = false;
    std::vector<MaterialState> materials;
    std::vector<BorderState> borders;
    struct ModeElement {
        winrt::weak_ref<xaml::FrameworkElement> element;
        SavedProperty opacity;
        SavedProperty hitTest;
    };
    std::vector<ModeElement> modeElements;
    std::vector<std::shared_ptr<ScrollMotion>> scrollMotions;
    DWORD threadId;
    winrt::weak_ref<xaml::FrameworkElement> root;
    winrt::weak_ref<xaml::FrameworkElement> desktops;
    winrt::weak_ref<xaml::FrameworkElement> windows;
    winrt::weak_ref<controls::Grid> grid;
    std::vector<controls::RowDefinition> originalRows;
    std::vector<SavedProperty> desktopProperties;
    std::vector<SavedProperty> windowProperties;
    struct SavedKeyFrame {
        animation::DoubleKeyFrame frame{nullptr};
        double original;
    };
    std::vector<SavedKeyFrame> animationFrames[2];
    winrt::event_token layoutToken{};
    bool applied = false;
    bool detached = false;
    int appliedMode = -1;
    winrt::weak_ref<controls::Grid> desktopGrid;
    winrt::weak_ref<controls::StackPanel> desktopStack;
    winrt::weak_ref<controls::ScrollViewer> desktopScroll;
    winrt::weak_ref<xaml::FrameworkElement> newDesktopButton;
    winrt::weak_ref<xaml::FrameworkElement> desktopBackground;
    struct ListBackground {
        winrt::weak_ref<controls::Border> element;
        SavedProperty background;
        SavedProperty opacity;
        media::SolidColorBrush transparent{nullptr};
        bool hidden = false;
    };
    ListBackground listBackgrounds[2];
    ListBackground fullscreenBackground;
    std::vector<controls::ColumnDefinition> originalDesktopColumns;
    std::vector<SavedProperty> buttonProperties;
    std::vector<SavedProperty> scrollProperties;
    std::vector<SavedProperty> backgroundProperties;
    uint32_t originalButtonIndex = 0;
    bool buttonPinned = false;
    xaml::Thickness originalButtonMargin{};
    xaml::Thickness originalScrollMargin{};

    void CenterDesktopList() {
        auto scroll = desktopScroll.get();
        auto stack = desktopStack.get();
        auto host = desktopGrid.get();
        auto button = newDesktopButton.get();
        if (!scroll || !stack || !host || !button || host.ActualWidth() <= 0) return;
        auto buttonMargin = button.Margin();
        double reservedRight = button.Visibility() == xaml::Visibility::Collapsed ? 0 :
            button.ActualWidth() + buttonMargin.Left + buttonMargin.Right;
        double free = host.ActualWidth() - reservedRight - originalScrollMargin.Left -
                      originalScrollMargin.Right - stack.DesiredSize().Width;
        // Balance the fixed button's space on the left when the list fits.
        // Release that space as the list fills, preserving the full scrolling
        // viewport and keeping thumbnails from overlapping the fixed button.
        auto margin = originalScrollMargin;
        margin.Left += (std::max)(0.0, (std::min)(reservedRight, free));
        if (!SameMargin(scroll.Margin(), margin)) scroll.Margin(margin);
    }

    void MatchNewDesktopFrame() {
        auto button = newDesktopButton.get();
        auto stack = desktopStack.get();
        auto host = desktopGrid.get();
        if (!button || !stack || !host) return;
        auto tile = FindDesktopTile(stack);
        if (!tile || tile.ActualHeight() <= 0) return;
        // Reparenting removed the ScrollViewer/ListView's content inset and
        // let the native button stretch across the scrollbar's extra height.
        // Follow the real tile's arranged bounds instead of a fixed DPI size.
        auto origin = tile.TransformToVisual(host).TransformPoint({0, 0});
        auto margin = originalButtonMargin;
        margin.Top = origin.Y - 2;
        margin.Right += 7;
        if (!SameMargin(button.Margin(), margin)) button.Margin(margin);
        if (button.VerticalAlignment() != xaml::VerticalAlignment::Top)
            button.VerticalAlignment(xaml::VerticalAlignment::Top);
        if (button.Height() != tile.ActualHeight()) button.Height(tile.ActualHeight());
        if (tile.ActualWidth() > 0 && button.Width() != tile.ActualWidth())
            button.Width(tile.ActualWidth());
    }

    void PinNewDesktopButton(xaml::FrameworkElement const& desktopElement) {
        if (buttonPinned) return;
        auto host = FindNamedChild<controls::Grid>(desktopElement, L"GridElement");
        auto scroll = FindNamedChild<controls::ScrollViewer>(host, L"DesktopsListScrollViewer");
        auto stack = FindNamedChild<controls::StackPanel>(scroll, L"DesktopsListStackPanel");
        auto button = FindNamedChild<xaml::FrameworkElement>(stack, L"NewVirtualDesktopButtonThemed");
        if (!host || !scroll || !stack || !button ||
            media::VisualTreeHelper::GetParent(button) != stack) return;
        auto background = FindNamedChild<xaml::FrameworkElement>(host, L"VirtualDesktopBarBackground");
        uint32_t index = 0;
        if (!stack.Children().IndexOf(button, index)) return;

        for (auto const& column : host.ColumnDefinitions()) originalDesktopColumns.push_back(column);
        for (auto const& property : {controls::Grid::ColumnProperty(), controls::Grid::ColumnSpanProperty()}) {
            buttonProperties.emplace_back(button, property);
            scrollProperties.emplace_back(scroll, property);
            if (background) backgroundProperties.emplace_back(background, property);
        }
        for (auto const& property : {xaml::FrameworkElement::MarginProperty(),
                                    xaml::FrameworkElement::HeightProperty(),
                                    xaml::FrameworkElement::WidthProperty(),
                                    xaml::FrameworkElement::VerticalAlignmentProperty(),
                                    xaml::UIElement::VisibilityProperty()})
            buttonProperties.emplace_back(button, property);
        originalButtonMargin = button.Margin();
        originalScrollMargin = scroll.Margin();
        scrollProperties.emplace_back(scroll, xaml::FrameworkElement::MarginProperty());
        // Move the native control, retaining its click handlers, accessibility,
        // theme and data bindings. Auto measures its current thumbnail size.
        stack.Children().RemoveAt(index);
        try {
            host.Children().Append(button);
        } catch (...) {
            stack.Children().InsertAt(index, button);
            throw;
        }
        desktopGrid = winrt::make_weak(host);
        desktopStack = winrt::make_weak(stack);
        desktopScroll = winrt::make_weak(scroll);
        newDesktopButton = winrt::make_weak(button);
        if (background) desktopBackground = winrt::make_weak(background);
        originalButtonIndex = index;
        buttonPinned = true;
        controls::ColumnDefinition listColumn;
        controls::ColumnDefinition buttonColumn;
        listColumn.Width({1, xaml::GridUnitType::Star});
        buttonColumn.Width({1, xaml::GridUnitType::Auto});
        host.ColumnDefinitions().Clear();
        host.ColumnDefinitions().Append(listColumn);
        host.ColumnDefinitions().Append(buttonColumn);
        controls::Grid::SetColumn(scroll, 0);
        controls::Grid::SetColumnSpan(scroll, 1);
        controls::Grid::SetColumn(button, 1);
        controls::Grid::SetColumnSpan(button, 1);
        if (background) {
            controls::Grid::SetColumn(background, 0);
            controls::Grid::SetColumnSpan(background, 2);
        }
        Wh_SetIntValue(L"newDesktopPinned", 1);
    }

    void UnpinNewDesktopButton() {
        if (buttonPinned) {
            auto host = desktopGrid.get();
            auto stack = desktopStack.get();
            auto button = newDesktopButton.get();
            if (host && stack && button) {
                uint32_t index;
                if (host.Children().IndexOf(button, index)) {
                    host.Children().RemoveAt(index);
                    stack.Children().InsertAt((std::min)(originalButtonIndex, stack.Children().Size()), button);
                }
                for (auto const& property : buttonProperties) property.Restore(button);
            }
            if (auto scroll = desktopScroll.get())
                for (auto const& property : scrollProperties) property.Restore(scroll);
            if (auto background = desktopBackground.get())
                for (auto const& property : backgroundProperties) property.Restore(background);
            if (host) {
                host.ColumnDefinitions().Clear();
                for (auto const& column : originalDesktopColumns) host.ColumnDefinitions().Append(column);
            }
            buttonPinned = false;
            originalDesktopColumns.clear(); buttonProperties.clear();
            scrollProperties.clear(); backgroundProperties.clear();
        }
    }

    void Apply() {
        if (g_insideLayout || g_stopping) return;
        LayoutGuard guard;
        auto desktopElement = desktops.get();
        auto windowElement = windows.get();
        if (!desktopElement || !windowElement) return;

        auto layoutGrid = grid.get();
        if (!layoutGrid) return;
        int mode = g_viewMode.load();
        bool listOnly = IsListOnlyMode();
        // The full-screen dimming/Acrylic border can belong to the outer
        // switcher host, rather than the TaskViewTimeline's RootGridElement.
        auto fullscreen = fullscreenBackground.element.get();
        if (!fullscreen) {
            xaml::DependencyObject ancestor = root.get();
            for (int depth = 0; ancestor && depth < 4; depth++) {
                fullscreen = FindNamedChild<controls::Border>(ancestor, L"BackgroundDimmingLayer");
                if (fullscreen) break;
                ancestor = media::VisualTreeHelper::GetParent(ancestor);
            }
            if (fullscreen) {
                fullscreenBackground.element = winrt::make_weak(fullscreen);
                fullscreenBackground.background = SavedProperty(fullscreen, controls::Border::BackgroundProperty());
                fullscreenBackground.opacity = SavedProperty(fullscreen, xaml::UIElement::OpacityProperty());
                fullscreenBackground.transparent = media::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0});
                fullscreenBackground.hidden = false;
                Wh_Log(L"Found full-screen Task View background");
            }
        }
        if (fullscreen) {
            int backdrop = g_backdropBlur.load();
            if (backdrop == 2) {
                if (!backdropAcrylic) {
                    backdropAcrylic = media::AcrylicBrush();
                    backdropAcrylic.BackgroundSource(media::AcrylicBackgroundSource::Backdrop);
                    backdropAcrylic.TintColor({255, 0, 0, 0});
                    backdropAcrylic.FallbackColor({255, 24, 24, 24});
                    backdropAcrylic.TintLuminosityOpacity(wf::IReference<double>{0.0});
                }
                double dim = g_backdropDimOpacity.load() / 100.0;
                if (backdropAcrylic.TintOpacity() != dim) backdropAcrylic.TintOpacity(dim);
                if (fullscreen.Background() != backdropAcrylic) fullscreen.Background(backdropAcrylic);
                if (fullscreen.Opacity() != 1) fullscreen.Opacity(1);
                fullscreenBackground.hidden = true;
            } else if (backdrop == 1 || g_disableFullscreenBlur) {
                if (fullscreen.Background() != fullscreenBackground.transparent)
                    fullscreen.Background(fullscreenBackground.transparent);
                if (fullscreen.Opacity() != 0) fullscreen.Opacity(0);
                fullscreenBackground.hidden = true;
            } else if (fullscreenBackground.hidden) {
                fullscreenBackground.background.Restore(fullscreen);
                fullscreenBackground.opacity.Restore(fullscreen);
                fullscreenBackground.hidden = false;
            }
        }
        // Windows uses separate backgrounds for the Task View desktop bar and
        // desktop switcher. Replace the brushes as well as hiding the visuals:
        // lowering opacity alone leaves an Acrylic/composition effect attached.
        wchar_t const* backgroundNames[] = {
            L"VirtualDesktopBarBackground", L"VirtualDesktopSwitcherBackground"};
        for (int i = 0; i < 2; i++) {
            auto& saved = listBackgrounds[i];
            auto background = saved.element.get();
            if (!background) {
                background = FindNamedChild<controls::Border>(desktopElement, backgroundNames[i]);
                if (!background) continue;
                saved.element = winrt::make_weak(background);
                saved.background = SavedProperty(background, controls::Border::BackgroundProperty());
                saved.opacity = SavedProperty(background, xaml::UIElement::OpacityProperty());
                saved.transparent = media::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0});
                saved.hidden = false;
                Wh_Log(L"Found desktop list background: %s", backgroundNames[i]);
            }
            if (g_disableListBackground) {
                if (background.Background() != saved.transparent) background.Background(saved.transparent);
                if (background.Opacity() != 0) background.Opacity(0);
                saved.hidden = true;
            } else if (saved.hidden) {
                saved.background.Restore(background);
                saved.opacity.Restore(background);
                saved.hidden = false;
            }
        }
        ApplyMaterials(desktopElement, materials);
        ApplyMaterials(windowElement, materials);
        ApplyBorders(desktopElement, borders);
        ApplyBorders(windowElement, borders);
        ApplyHighlights();
        // A transparent background receives clicks in the empty part of this
        // XAML surface without changing the underlying desktop's appearance.
        if (listOnly && !inputBackgroundApplied) {
            layoutGrid.Background(media::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0}));
            inputBackgroundApplied = true;
        } else if (!listOnly && inputBackgroundApplied) {
            originalInputBackground.Restore(layoutGrid);
            inputBackgroundApplied = false;
        }
        for (auto const& saved : modeElements) {
            if (auto element = saved.element.get()) {
                if (listOnly) {
                    if (element.Opacity() != 0) element.Opacity(0);
                    if (element.IsHitTestVisible()) element.IsHitTestVisible(false);
                } else if (element.Opacity() == 0) {
                    saved.opacity.Restore(element);
                    saved.hitTest.Restore(element);
                }
            }
        }
        if (!applied || appliedMode != mode) {
            controls::RowDefinition desktopRow;
            controls::RowDefinition windowRow;
            desktopRow.Height({1, xaml::GridUnitType::Auto});
            windowRow.Height({1, xaml::GridUnitType::Star});
            layoutGrid.RowDefinitions().Clear();
            layoutGrid.RowDefinitions().Append(mode == 0 ? windowRow : desktopRow);
            layoutGrid.RowDefinitions().Append(mode == 0 ? desktopRow : windowRow);
            Wh_SetIntValue(L"layoutApplied", 1);
            Wh_Log(L"Applied Task View layout mode %d", mode);
        }
        int desktopRowIndex = mode == 0 ? 1 : 0;
        int windowRowIndex = mode == 0 ? 0 : 1;
        if (controls::Grid::GetRow(desktopElement) != desktopRowIndex) controls::Grid::SetRow(desktopElement, desktopRowIndex);
        int desktopSpan = mode == 3 || mode == 4 ? 2 : 1;
        if (controls::Grid::GetRowSpan(desktopElement) != desktopSpan) controls::Grid::SetRowSpan(desktopElement, desktopSpan);
        if (controls::Grid::GetRow(windowElement) != windowRowIndex) controls::Grid::SetRow(windowElement, windowRowIndex);
        if (controls::Grid::GetRowSpan(windowElement) != 1) controls::Grid::SetRowSpan(windowElement, 1);
        auto alignment = mode == 3 ? xaml::VerticalAlignment::Center : mode == 0 || mode == 4 ? xaml::VerticalAlignment::Bottom : xaml::VerticalAlignment::Top;
        if (desktopElement.VerticalAlignment() != alignment) {
            desktopElement.VerticalAlignment(alignment);
        }
        if (windowElement.VerticalAlignment() != xaml::VerticalAlignment::Stretch)
            windowElement.VerticalAlignment(xaml::VerticalAlignment::Stretch);
        if (!std::isnan(desktopElement.Height())) desktopElement.Height(NAN);
        if (!std::isnan(windowElement.Height())) windowElement.Height(NAN);
        xaml::Thickness zero{};
        double frameMargin = g_desktopFrameMargin.load();
        xaml::Thickness desktopMargin{frameMargin, 32 + frameMargin, frameMargin, frameMargin};
        if (mode == 0 || mode == 3 || mode == 4) desktopMargin.Top = frameMargin;
        if (!SameMargin(desktopElement.Margin(), desktopMargin)) desktopElement.Margin(desktopMargin);
        if (!SameMargin(windowElement.Margin(), zero)) windowElement.Margin(zero);
        PinNewDesktopButton(desktopElement);
        if (buttonPinned) {
            auto button = newDesktopButton.get();
            if (button) {
                auto visibility = g_hideNewDesktop ? xaml::Visibility::Collapsed : xaml::Visibility::Visible;
                if (button.Visibility() != visibility) button.Visibility(visibility);
            }
            int widthMode = g_frameWidthMode.load();
            auto horizontal = widthMode ? xaml::HorizontalAlignment::Center : xaml::HorizontalAlignment::Stretch;
            if (desktopElement.HorizontalAlignment() != horizontal)
                desktopElement.HorizontalAlignment(horizontal);
            double width = NAN;
            double available = (std::max)(0.0, layoutGrid.ActualWidth() - 2 * frameMargin);
            if (widthMode == 2) width = (std::min)(available, layoutGrid.ActualWidth() * g_framePercent.load() / 100.0);
            if (widthMode == 1) {
                auto stack = desktopStack.get();
                if (stack && stack.DesiredSize().Width > 0) {
                    double extra = 0;
                    if (button && !g_hideNewDesktop) {
                        auto margin = button.Margin();
                        extra = button.Width() + margin.Left + margin.Right;
                    }
                    // Balance the pinned button at the opposite edge as well.
                    width = (std::min)(available, stack.DesiredSize().Width + 2 * extra + 16);
                }
            }
            if ((std::isnan(width) && !std::isnan(desktopElement.Width())) ||
                (!std::isnan(width) && desktopElement.Width() != width)) desktopElement.Width(width);
            MatchNewDesktopFrame();
            CenterDesktopList();

        }
        applied = true;
        appliedMode = mode;
    }

    void Restore() {
        LayoutGuard guard;
        media::CompositionTarget::Rendering(releaseToken);
        ClearAnimationTargets();
        for (auto const& motion : scrollMotions) motion->Stop();
        if (auto rootElement = root.get()) {
            rootElement.LayoutUpdated(layoutToken);
            rootElement.KeyDown(keyToken);
            rootElement.PointerPressed(outsideClickToken);
        }
        detached = true;
        for (auto const& highlight : highlights) highlight->Stop();
        for (auto& material : materials) material.Restore();
        for (auto& border : borders) border.Restore();
        if (fullscreenBackground.hidden) {
            if (auto background = fullscreenBackground.element.get()) {
                fullscreenBackground.background.Restore(background);
                fullscreenBackground.opacity.Restore(background);
            }
            fullscreenBackground.hidden = false;
        }
        for (auto& saved : listBackgrounds) {
            if (saved.hidden) {
                if (auto background = saved.element.get()) {
                    saved.background.Restore(background);
                    saved.opacity.Restore(background);
                }
                saved.hidden = false;
            }
        }
        for (auto const& saved : modeElements) {
            if (auto element = saved.element.get()) {
                saved.opacity.Restore(element);
                saved.hitTest.Restore(element);
            }
        }
        UnpinNewDesktopButton();
        if (!applied) return;
        for (auto const& frames : animationFrames) {
            for (auto const& saved : frames) saved.frame.Value(saved.original);
        }
        if (auto desktopElement = desktops.get()) {
            for (auto const& property : desktopProperties) property.Restore(desktopElement);
        }
        if (auto windowElement = windows.get()) {
            for (auto const& property : windowProperties) property.Restore(windowElement);
        }
        if (auto layoutGrid = grid.get()) {
            if (inputBackgroundApplied) originalInputBackground.Restore(layoutGrid);
            layoutGrid.RowDefinitions().Clear();
            for (auto const& row : originalRows) layoutGrid.RowDefinitions().Append(row);
        }
    }
};

std::mutex g_statesMutex;
std::vector<std::shared_ptr<LayoutState>> g_states;

void ObserveElement(xaml::FrameworkElement const& element, bool walkAncestors = false) {
    if (!element || g_insideLayout || g_stopping) return;
    LayoutGuard guard;
    auto name = element.Name();
    if (!walkAncestors && name != L"VirtualDesktopBar" && name != L"SwitchItemListControl" &&
        name != L"RootGridElement" && name != L"TaskViewTimelineRoot") return;

    xaml::FrameworkElement rootElement = element;
    // Require the full Task View root. The taskbar hover preview has a desktop
    // strip too and must not be changed.
    for (int depth = 0; depth < 16; depth++) {
        if (winrt::get_class_name(rootElement) ==
            L"WindowsInternal.ComposableShell.Experiences.Switcher.TaskViewTimeline") {
            break;
        }
        rootElement = media::VisualTreeHelper::GetParent(rootElement)
                          .try_as<xaml::FrameworkElement>();
        if (!rootElement) return;
    }
    if (winrt::get_class_name(rootElement) !=
        L"WindowsInternal.ComposableShell.Experiences.Switcher.TaskViewTimeline") return;

    {
        std::lock_guard lock(g_statesMutex);
        for (auto const& state : g_states) {
            if (state->threadId == GetCurrentThreadId() &&
                state->root.get() == rootElement) return;
        }
    }

    // Locate only the immediate overview controls, never a nested Alt+Tab list.
    controls::Grid grid{nullptr};
    auto childCount = media::VisualTreeHelper::GetChildrenCount(rootElement);
    for (int i = 0; i < childCount; i++) {
        auto child = media::VisualTreeHelper::GetChild(rootElement, i).try_as<controls::Grid>();
        if (child && child.Name() == L"RootGridElement") { grid = child; break; }
    }
    if (!grid) return;
    xaml::FrameworkElement desktops{nullptr}, windows{nullptr};
    for (auto const& child : grid.Children()) {
        if (auto frameworkChild = child.try_as<xaml::FrameworkElement>()) {
            if (frameworkChild.Name() == L"VirtualDesktopBar") desktops = frameworkChild;
            if (frameworkChild.Name() == L"SwitchItemListControl") windows = frameworkChild;
        }
    }
    if (!desktops || !windows) return;

    auto state = std::make_shared<LayoutState>();
    state->threadId = GetCurrentThreadId();
    state->root = winrt::make_weak(rootElement);
    state->desktops = winrt::make_weak(desktops);
    state->windows = winrt::make_weak(windows);
    state->grid = winrt::make_weak(grid);
    state->originalInputBackground = SavedProperty(grid, controls::Panel::BackgroundProperty());
    state->releaseToken = media::CompositionTarget::Rendering(
        [weak = std::weak_ptr(state)](auto const&, auto const&) {
            auto current = weak.lock();
            if (!current || current->detached || g_stopping) return;
            try {
                if (!g_insideLayout) { LayoutGuard guard; current->ApplyHighlights(); }
            } catch (...) { Wh_Log(L"Desktop highlight update failed: %08X", winrt::to_hresult()); }
            wchar_t name[128]{};
            HWND foreground = GetForegroundWindow();
            GetClassNameW(foreground, name, ARRAYSIZE(name));
            DWORD process = 0;
            DWORD foregroundThread = GetWindowThreadProcessId(foreground, &process);
            if (!g_closeOnWindowsRelease || process != GetCurrentProcessId() ||
                foregroundThread != current->threadId ||
                (wcscmp(name, L"XamlExplorerHostIslandWindow") != 0 &&
                 wcscmp(name, L"MultitaskingViewFrame") != 0)) {
                current->windowsHeld = false;
                return;
            }
            bool held = ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000) != 0;
            if (held) current->windowsHeld = true;
            else if (current->windowsHeld) {
                current->windowsHeld = false;
                if (current->DismissList()) Wh_SetIntValue(L"windowsReleaseDismissed", 1);
            }
        });
    state->outsideClickToken = rootElement.PointerPressed(
        [weak = std::weak_ptr(state)](auto const&, xaml::Input::PointerRoutedEventArgs const& args) {
            if (g_stopping || !IsListOnlyMode()) return;
            try {
                auto current = weak.lock();
                if (!current || current->detached) return;
                auto desktops = current->desktops.get();
                if (!desktops) return;
                auto point = args.GetCurrentPoint(desktops).Position();
                if (point.X >= 0 && point.X < desktops.ActualWidth() &&
                    point.Y >= 0 && point.Y < desktops.ActualHeight()) return;
                if (!current->DismissList()) return;
                args.Handled(true);
                Wh_SetIntValue(L"outsideClickDismissed", 1);
            } catch (...) { Wh_Log(L"Outside-click dismissal failed: %08X", winrt::to_hresult()); }
        });
    state->keyToken = rootElement.KeyDown([weak = std::weak_ptr(state)](auto const&, xaml::Input::KeyRoutedEventArgs const& args) {
        if (g_stopping || !IsListOnlyMode() || args.Key() != winrt::Windows::System::VirtualKey::Tab) return;
        try {
            auto current = weak.lock();
            if (!current) return;
            auto stack = current->desktopStack.get();
            if (!stack) return;
            current->Navigate((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? -1 : 1);
            args.Handled(true);
        } catch (...) {}
    });
    for (auto const* name : {L"SwitchItemListControl", L"BackgroundThumbnailHost"}) {
        if (auto element = FindNamedChild<xaml::FrameworkElement>(grid, name)) {
            state->modeElements.push_back({winrt::make_weak(element),
                SavedProperty(element, xaml::UIElement::OpacityProperty()),
                SavedProperty(element, xaml::UIElement::IsHitTestVisibleProperty())});
        }
    }
    for (auto const& row : grid.RowDefinitions()) state->originalRows.push_back(row);
    for (auto const& property : {controls::Grid::RowProperty(), controls::Grid::RowSpanProperty(),
                                xaml::FrameworkElement::VerticalAlignmentProperty(),
                                xaml::FrameworkElement::HorizontalAlignmentProperty(),
                                xaml::FrameworkElement::WidthProperty(),
                                xaml::FrameworkElement::HeightProperty(),
                                xaml::FrameworkElement::MarginProperty()}) {
        state->desktopProperties.emplace_back(desktops, property);
        state->windowProperties.emplace_back(windows, property);
    }
    // The event owns only a weak state; the registry owns the state itself.
    state->layoutToken = rootElement.LayoutUpdated(
        [weak = std::weak_ptr(state)](auto const&, auto const&) {
            try {
                if (auto current = weak.lock()) current->Apply();
            } catch (...) {
                Wh_Log(L"Task View layout update failed: %08X", winrt::to_hresult());
            }
        });
    {
        std::lock_guard lock(g_statesMutex);
        g_states.push_back(state);
    }
    Wh_Log(L"Found full Task View root on thread %u", state->threadId);
    // Apply on the next layout pass; the reentrancy guard is active here.
    rootElement.InvalidateArrange();
}

using Arrange_t = HRESULT(WINAPI*)(void*, wf::Rect);
Arrange_t Arrange_Original;
HRESULT WINAPI Arrange_Hook(void* self, wf::Rect rectangle) {
    if (!g_insideLayout && !g_stopping) {
        try {
            xaml::IUIElement element{nullptr};
            winrt::copy_from_abi(element, self);
            ObserveElement(element.try_as<xaml::FrameworkElement>());
        } catch (...) {
            Wh_Log(L"Task View arrange observation failed: %08X", winrt::to_hresult());
        }
    }
    return Arrange_Original(self, rectangle);
}

using ActualHeight_t = HRESULT(WINAPI*)(void*, double*);
ActualHeight_t ActualHeight_Original;
HRESULT WINAPI ActualHeight_Hook(void* self, double* height) {
    HRESULT result = ActualHeight_Original(self, height);
    if (SUCCEEDED(result) && !g_insideLayout && !g_stopping) {
        try {
            xaml::IFrameworkElement element{nullptr};
            winrt::copy_from_abi(element, self);
            ObserveElement(element.as<xaml::FrameworkElement>());
        } catch (...) {
            Wh_Log(L"Task View height observation failed: %08X", winrt::to_hresult());
        }
    }
    return result;
}

using DiagnosticsLookup_t = HRESULT(WINAPI*)(void*, unsigned long long, void**);
DiagnosticsLookup_t DiagnosticsLookup_Original;
HRESULT WINAPI DiagnosticsLookup_Hook(void* self, unsigned long long handle,
                                     void** inspectable) {
    HRESULT result = DiagnosticsLookup_Original(self, handle, inspectable);
    if (SUCCEEDED(result) && inspectable && *inspectable && !g_insideLayout && !g_stopping) {
        try {
            wf::IInspectable object{nullptr};
            winrt::copy_from_abi(object, *inspectable);
            ObserveElement(object.try_as<xaml::FrameworkElement>(), true);
        } catch (...) {
            Wh_Log(L"Diagnostics observation failed: %08X", winrt::to_hresult());
        }
    }
    return result;
}

struct AnimationTarget {
    winrt::weak_ref<animation::Timeline> timeline;
    winrt::weak_ref<xaml::FrameworkElement> target;
};
thread_local std::vector<AnimationTarget> g_animationTargets;
void ClearAnimationTargets() { g_animationTargets.clear(); }
bool IsTaskViewAnimation(animation::Timeline const& timeline) {
    for (auto const& tracked : g_animationTargets)
        if (tracked.timeline.get() == timeline) return true;
    if (auto storyboard = timeline.try_as<animation::Storyboard>())
        for (auto const& child : storyboard.Children())
            if (IsTaskViewAnimation(child)) return true;
    return false;
}
using GetPeer_t = HRESULT(WINAPI*)(void*, GUID const&, void**);
GetPeer_t GetPeer;
using CoreSetTarget_t = HRESULT(WINAPI*)(void*, void*);
CoreSetTarget_t CoreSetTarget_Original;
HRESULT WINAPI CoreSetTarget_Hook(void* self, void* target) {
    HRESULT result = CoreSetTarget_Original(self, target);
    if (SUCCEEDED(result) && !g_stopping && target) {
        try {
            animation::Timeline timeline{nullptr};
            xaml::FrameworkElement element{nullptr};
            HRESULT timelineResult = GetPeer(self, winrt::guid_of<animation::ITimeline>(), winrt::put_abi(timeline));
            HRESULT targetResult = GetPeer(target, winrt::guid_of<xaml::IFrameworkElement>(), winrt::put_abi(element));
            if (SUCCEEDED(timelineResult) && SUCCEEDED(targetResult) && timeline && element) {
                auto ancestor = element;
                bool fullTaskView = false;
                for (int depth = 0; ancestor && depth < 16; depth++) {
                    if (winrt::get_class_name(ancestor) ==
                        L"WindowsInternal.ComposableShell.Experiences.Switcher.TaskViewTimeline") {
                        fullTaskView = true;
                        break;
                    }
                    ancestor = media::VisualTreeHelper::GetParent(ancestor).try_as<xaml::FrameworkElement>();
                }
                if (!fullTaskView) return result;
                ObserveElement(element, true);
                for (auto it = g_animationTargets.begin(); it != g_animationTargets.end();) {
                    auto existing = it->timeline.get();
                    if (!existing || existing == timeline) it = g_animationTargets.erase(it);
                    else ++it;
                }
                g_animationTargets.push_back({winrt::make_weak(timeline), winrt::make_weak(element)});
            }
        } catch (...) {}
    }
    return result;
}
void ReverseDesktopSlide(animation::Timeline const& timeline) {
    if (auto storyboard = timeline.try_as<animation::Storyboard>()) {
        for (auto const& child : storyboard.Children()) ReverseDesktopSlide(child);
        return;
    }
    if (animation::Storyboard::GetTargetProperty(timeline) !=
        L"(UIElement.RenderTransform).(TranslateTransform.Y)") return;
    auto keyframes = timeline.try_as<animation::DoubleAnimationUsingKeyFrames>();
    if (!keyframes) return;
    for (auto const& tracked : g_animationTargets) {
        if (tracked.timeline.get() != timeline) continue;
        auto target = tracked.target.get();
        if (!target) return;
        std::lock_guard lock(g_statesMutex);
        for (auto const& state : g_states) {
            if (state->threadId != GetCurrentThreadId() || state->desktops.get() != target) continue;
            auto frames = keyframes.KeyFrames();
            if (frames.Size() == 0) return;
            auto& savedFrames = state->animationFrames[frames.GetAt(0).Value() == 0 ? 1 : 0];
            savedFrames.clear();
            for (auto const& frame : frames) {
                savedFrames.push_back({frame, std::abs(frame.Value())});
                frame.Value((g_viewMode == 0 || g_viewMode == 4) ? std::abs(frame.Value()) : -std::abs(frame.Value()));
                Wh_SetIntValue(L"animationReversed", 1);
            }
            return;
        }
    }
}

using StoryboardBegin_t = HRESULT(WINAPI*)(void*);
StoryboardBegin_t StoryboardBegin_Original;
HRESULT WINAPI StoryboardBegin_Hook(void* self) {
    bool skip = false;
    if (!g_stopping && !g_insideLayout) {
        try {
            animation::IStoryboard storyboardInterface{nullptr};
            winrt::copy_from_abi(storyboardInterface, self);
            auto storyboard = storyboardInterface.as<animation::Storyboard>();
            skip = g_disableListAnimations && IsTaskViewAnimation(storyboard);
            { LayoutGuard guard; ReverseDesktopSlide(storyboard); }
        } catch (...) {}
    }
    auto result = StoryboardBegin_Original(self);
    if (SUCCEEDED(result) && skip) {
        try {
            animation::IStoryboard storyboard{nullptr};
            winrt::copy_from_abi(storyboard, self);
            storyboard.as<animation::Storyboard>().SkipToFill();
        } catch (...) {}
    }
    return result;
}

bool IsDesktopScroll(controls::ScrollViewer const& scroll) {
    if (!scroll || g_stopping || g_insideLayout) return false;
    xaml::FrameworkElement ancestor = scroll;
    bool desktopBar = false;
    for (int depth = 0; ancestor && depth < 24; depth++) {
        auto className = winrt::get_class_name(ancestor);
        if (className == L"WindowsInternal.ComposableShell.Experiences.Switcher.VirtualDesktopBarElement")
            desktopBar = true;
        if (className == L"WindowsInternal.ComposableShell.Experiences.Switcher.TaskViewTimeline")
            return desktopBar;
        ancestor = media::VisualTreeHelper::GetParent(ancestor).try_as<xaml::FrameworkElement>();
    }
    return false;
}

using ChangeView_t = HRESULT(WINAPI*)(void*, void*, void*, void*, bool, bool*);
ChangeView_t ChangeView_Original;
bool AnimateDesktopScroll(controls::ScrollViewer const& scroll, double offset) {
    std::shared_ptr<ScrollMotion> motion;
    {
        std::lock_guard lock(g_statesMutex);
        for (auto const& state : g_states) {
            if (state->threadId != GetCurrentThreadId() || !state->root.get()) continue;
            for (auto const& candidate : state->scrollMotions) {
                if (candidate->scroll.get() == scroll) { motion = candidate; break; }
            }
            if (!motion) {
                motion = std::make_shared<ScrollMotion>();
                motion->scroll = winrt::make_weak(scroll);
                state->scrollMotions.push_back(motion);
            }
            break;
        }
    }
    if (motion) return motion->Request(offset, g_scrollSpeed.load());
    ScrollCallGuard guard;
    return scroll.ChangeView(wf::IReference<double>{offset}, nullptr, nullptr, false);
}
HRESULT WINAPI ChangeView_Hook(void* self, void* horizontal, void* vertical,
                               void* zoom, bool disableAnimation, bool* changed) {
    if (horizontal && !g_insideScroll && !g_stopping && !g_insideLayout) {
        try {
            controls::IScrollViewer2 scrollInterface{nullptr};
            winrt::copy_from_abi(scrollInterface, self);
            auto scroll = scrollInterface.as<controls::ScrollViewer>();
            if (IsDesktopScroll(scroll)) {
                // Preserve requests that also change another axis or zoom.
                // Windows sometimes passes the unchanged values explicitly.
                if (vertical) {
                    wf::IReference<double> value{nullptr};
                    winrt::copy_from_abi(value, vertical);
                    if (std::abs(value.Value() - scroll.VerticalOffset()) > 0.5)
                        return ChangeView_Original(self, horizontal, vertical, zoom, false, changed);
                }
                if (zoom) {
                    wf::IReference<float> value{nullptr};
                    winrt::copy_from_abi(value, zoom);
                    if (std::abs(value.Value() - scroll.ZoomFactor()) > 0.001f)
                        return ChangeView_Original(self, horizontal, vertical, zoom, false, changed);
                }
                wf::IReference<double> offset{nullptr};
                winrt::copy_from_abi(offset, horizontal);
                bool result = AnimateDesktopScroll(scroll, offset.Value());
                if (changed) *changed = result;
                Wh_SetIntValue(L"desktopScrollAnimated", 1);
                return S_OK;
            }
        } catch (...) {}
    }
    return ChangeView_Original(self, horizontal, vertical, zoom, disableAnimation, changed);
}

using ScrollHorizontal_t = HRESULT(WINAPI*)(void*, double);
ScrollHorizontal_t ScrollHorizontal_Original;
HRESULT WINAPI ScrollHorizontal_Hook(void* self, double offset) {
    if (!g_stopping && !g_insideLayout && !g_insideScroll) {
        try {
            controls::IScrollViewer scrollInterface{nullptr};
            winrt::copy_from_abi(scrollInterface, self);
            auto scroll = scrollInterface.as<controls::ScrollViewer>();
            if (IsDesktopScroll(scroll)) {
                AnimateDesktopScroll(scroll, offset);
                Wh_SetIntValue(L"desktopScrollAnimated", 1);
                return S_OK;
            }
        } catch (...) {}
    }
    return ScrollHorizontal_Original(self, offset);
}

void InstallXamlHooks() {
    if (g_hooksReady || g_stopping || g_installing.test_and_set()) return;
    try {
        // Run on an existing Explorer XAML thread. Creating a Grid on Windhawk's
        // worker thread would violate XAML's thread-affinity requirement.
        controls::Grid probe;
        auto ui = probe.as<xaml::IUIElement>();
        auto framework = probe.as<xaml::IFrameworkElement>();
        auto uiVtable = *reinterpret_cast<void***>(winrt::get_abi(ui));
        auto frameworkVtable = *reinterpret_cast<void***>(winrt::get_abi(framework));
        controls::ScrollViewer scrollProbe;
        auto scrollInterface = scrollProbe.as<controls::IScrollViewer>();
        auto scrollInterface2 = scrollProbe.as<controls::IScrollViewer2>();
        auto scrollVtable = *reinterpret_cast<void***>(winrt::get_abi(scrollInterface));
        auto scrollVtable2 = *reinterpret_cast<void***>(winrt::get_abi(scrollInterface2));
        // Public ABI slots verified against the installed C++/WinRT headers.
        bool scrollOffset = WindhawkUtils::SetFunctionHook(
            reinterpret_cast<ScrollHorizontal_t>(scrollVtable[58]), ScrollHorizontal_Hook, &ScrollHorizontal_Original);
        bool changeView = WindhawkUtils::SetFunctionHook(
            reinterpret_cast<ChangeView_t>(scrollVtable2[15]), ChangeView_Hook, &ChangeView_Original);
        animation::Storyboard storyboard;
        auto storyboardInterface = storyboard.as<animation::IStoryboard>();
        auto storyboardVtable = *reinterpret_cast<void***>(winrt::get_abi(storyboardInterface));
        bool begin = WindhawkUtils::SetFunctionHook(reinterpret_cast<StoryboardBegin_t>(storyboardVtable[9]),
                                      StoryboardBegin_Hook, &StoryboardBegin_Original);
        // Fixed slots in the published WinRT IUIElement/IFrameworkElement ABI,
        // including the six IInspectable entries; no object offsets involved.
        bool arrange = WindhawkUtils::SetFunctionHook(
            reinterpret_cast<Arrange_t>(uiVtable[92]), Arrange_Hook, &Arrange_Original);
        bool height = WindhawkUtils::SetFunctionHook(
            reinterpret_cast<ActualHeight_t>(frameworkVtable[14]), ActualHeight_Hook, &ActualHeight_Original);
        if (arrange && height && begin && scrollOffset && changeView) {
            Wh_ApplyHookOperations();
            g_hooksReady = true;
            Wh_Log(L"Installed Windows.UI.Xaml layout hooks");
        } else {
            if (scrollOffset) Wh_RemoveFunctionHook(scrollVtable[58]);
            if (changeView) Wh_RemoveFunctionHook(scrollVtable2[15]);
            if (begin) Wh_RemoveFunctionHook(storyboardVtable[9]);
            if (arrange) Wh_RemoveFunctionHook(uiVtable[92]);
            if (height) Wh_RemoveFunctionHook(frameworkVtable[14]);
            Wh_ApplyHookOperations();
            g_installing.clear();
            Wh_Log(L"Couldn't install all XAML hooks");
            return;
        }
    } catch (...) {
        Wh_Log(L"XAML thread initialization failed: %08X", winrt::to_hresult());
    }
    g_installing.clear();
}

using WindowThreadProc = void(*)(void*);
struct WindowThreadCall { WindowThreadProc proc; void* parameter; };
UINT g_threadMessage;

LRESULT CALLBACK CallWindowHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        auto message = reinterpret_cast<CWPSTRUCT*>(lParam);
        if (message->message == g_threadMessage && message->lParam) {
            auto call = reinterpret_cast<WindowThreadCall*>(message->lParam);
            call->proc(call->parameter);
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

bool RunOnWindowThread(HWND window, WindowThreadProc proc, void* parameter) {
    DWORD processId = 0;
    DWORD threadId = GetWindowThreadProcessId(window, &processId);
    if (!threadId || processId != GetCurrentProcessId()) return false;
    if (threadId == GetCurrentThreadId()) {
        proc(parameter);
        return true;
    }
    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, CallWindowHook, nullptr, threadId);
    if (!hook) return false;
    WindowThreadCall call{proc, parameter};
    SendMessageW(window, g_threadMessage, 0, reinterpret_cast<LPARAM>(&call));
    UnhookWindowsHookEx(hook);
    return true;
}

void InitializeWindow(HWND window) {
    wchar_t className[128];
    if (g_hooksReady || g_stopping ||
        !GetClassNameW(window, className, ARRAYSIZE(className))) return;
    if (wcscmp(className, L"Windows.UI.Composition.DesktopWindowContentBridge") == 0 ||
        wcscmp(className, L"XamlExplorerHostIslandWindow") == 0) {
        RunOnWindowThread(window, [](void*) { InstallXamlHooks(); }, nullptr);
    }
}

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;
HWND WINAPI CreateWindowExW_Hook(DWORD exStyle, LPCWSTR className,
                               LPCWSTR title, DWORD style, int x, int y,
                               int width, int height, HWND parent, HMENU menu,
                               HINSTANCE instance, LPVOID parameter) {
    HWND window = CreateWindowExW_Original(exStyle, className, title, style, x, y,
                                        width, height, parent, menu, instance, parameter);
    if (window) InitializeWindow(window);
    return window;
}
}  // namespace

BOOL Wh_ModInit() {
    LoadFrameSettings();
    const wchar_t* viewMode = Wh_GetStringSetting(L"General.viewMode");
    g_viewMode = (std::max)(0, (std::min)(viewMode ? _wtoi(viewMode) : 0, 4));
    Wh_FreeStringSetting(viewMode);
    g_desktopFrameMargin = (std::max)(0, (std::min)(Wh_GetIntSetting(L"DesktopList.desktopFrameMargin"), 200));
    int speed = Wh_GetIntSetting(L"Animations.scrollAnimationSpeed");
    g_scrollSpeed = (std::max)(10, (std::min)(speed > 0 ? speed : 100, 500));
    g_nativeScrollDuration = (std::max)(40, (std::min)(Wh_GetIntValue(L"nativeScrollDurationMs", 250), 2000));
    Wh_SetIntValue(L"desktopScrollAnimated", 0);
    Wh_SetIntValue(L"newDesktopPinned", 0);
    Wh_SetIntValue(L"layoutApplied", 0);
    Wh_SetIntValue(L"animationReversed", 0);
    HMODULE xamlModule = LoadLibraryExW(L"Windows.UI.Xaml.dll", nullptr,
                                      LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!xamlModule) return FALSE;
    WindhawkUtils::SYMBOL_HOOK diagnosticHooks[] = {
        {{L"?GetIInspectableFromHandle@XamlDiagnostics@@UEAAJ_KPEAPEAUIInspectable@@@Z"},
         &DiagnosticsLookup_Original, DiagnosticsLookup_Hook},
        {{L"?GetPeer@DXamlServices@DirectUI@@YAJPEAVCDependencyObject@@AEBU_GUID@@PEAPEAX@Z"},
         &GetPeer},
        {{L"?Storyboard_SetTarget@@YAJPEAVCTimeline@@PEAVCDependencyObject@@@Z"},
         &CoreSetTarget_Original, CoreSetTarget_Hook},
    };
    WH_HOOK_SYMBOLS_OPTIONS options{sizeof(options)};
    options.noUndecoratedSymbols = TRUE;
    if (!WindhawkUtils::HookSymbols(xamlModule, diagnosticHooks,
                                    ARRAYSIZE(diagnosticHooks), &options)) {
        Wh_Log(L"Required XAML diagnostics lookup hook is unavailable");
        return FALSE;
    }
    Wh_Log(L"Installed passive XAML diagnostics lookup hook");
    g_threadMessage = RegisterWindowMessageW(
        L"Windhawk.TaskViewCustomizer.RunOnThread");
    return g_threadMessage && WindhawkUtils::SetFunctionHook(
        CreateWindowExW, CreateWindowExW_Hook, &CreateWindowExW_Original);
}

void Wh_ModAfterInit() {
    EnumWindows([](HWND window, LPARAM) __attribute__((stdcall)) -> BOOL {
        DWORD processId;
        GetWindowThreadProcessId(window, &processId);
        if (processId != GetCurrentProcessId()) return TRUE;
        InitializeWindow(window);
        EnumChildWindows(window, [](HWND child, LPARAM) __attribute__((stdcall)) -> BOOL {
            InitializeWindow(child);
            return TRUE;
        }, 0);
        return TRUE;
    }, 0);
    if (!g_hooksReady) Wh_Log(L"Waiting for an Explorer XAML window");
}

void Wh_ModSettingsChanged() {
    LoadFrameSettings();
    const wchar_t* viewMode = Wh_GetStringSetting(L"General.viewMode");
    g_viewMode = (std::max)(0, (std::min)(viewMode ? _wtoi(viewMode) : 0, 4));
    Wh_FreeStringSetting(viewMode);
    g_desktopFrameMargin = (std::max)(0, (std::min)(Wh_GetIntSetting(L"DesktopList.desktopFrameMargin"), 200));
    int speed = Wh_GetIntSetting(L"Animations.scrollAnimationSpeed");
    g_scrollSpeed = (std::max)(10, (std::min)(speed > 0 ? speed : 100, 500));
}

void Wh_ModBeforeUninit() {
    g_stopping = true;
    std::vector<std::shared_ptr<LayoutState>> states;
    {
        std::lock_guard lock(g_statesMutex);
        states.swap(g_states);
    }
    for (auto const& state : states) {
        // Task View has a window on its UI thread. A synchronous thread call
        // removes event handlers before Windhawk unloads this DLL.
        HWND threadWindow = nullptr;
        EnumThreadWindows(state->threadId, [](HWND window, LPARAM parameter) __attribute__((stdcall)) -> BOOL {
            *reinterpret_cast<HWND*>(parameter) = window;
            return FALSE;
        }, reinterpret_cast<LPARAM>(&threadWindow));
        bool restored = false;
        if (threadWindow) {
            restored = RunOnWindowThread(threadWindow, [](void* parameter) {
                try {
                    static_cast<LayoutState*>(parameter)->Restore();
                } catch (...) {
                    Wh_Log(L"Task View restoration failed: %08X", winrt::to_hresult());
                }
            }, state.get());
        }
        if (!restored || !state->detached) {
            // An event delegate may still belong to a live root. Keep this
            // module mapped if its UI thread can no longer be reached; the
            // callback sees g_stopping and its weak state is already expired.
            HMODULE retainedModule;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_PIN,
                               reinterpret_cast<LPCWSTR>(&Arrange_Hook),
                               &retainedModule);
            Wh_Log(L"UI thread unavailable; retaining module until Explorer exits");
        }
    }
}
