// ==WindhawkMod==
// @id              asteski-task-view-desktops-on-top
// @name            Task View: Desktops on Top
// @description     Move virtual desktops above the window overview in Windows 11 Task View
// @version         0.7.7
// @author          Asteski
// @github          https://github.com/Asteski
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Task View: Desktops on Top

Experimental Windows 11 mod. In the full Win+Tab overview, place the
virtual desktop strip at the top and the window overview below it, similar
to the arrangement in macOS Mission Control.

This changes the existing Windows controls and their layout, including their
hit testing. It does not replace Task View or change Alt+Tab or the desktop
preview shown when hovering over the taskbar button.

The strip has a 32-pixel top margin to clear the taskbar and uses an
Auto-sized top grid row, with the window overview in the
remaining space below. The desktop strip slides down from the top on entrance
and retreats upward on exit, retaining Windows' original timing and easing.
The native New desktop button stays in a fixed column on the right, while
the existing desktops scroll in the remaining space. Its width follows the
current Windows theme and thumbnail size.
Layout properties and the original row definitions
are restored when the mod is disabled.

The mod observes the existing XAML diagnostics consumer's object lookups.
It does not register its own diagnostics consumer or change another mod's
settings. For reliable detection, keep Windows 11 Taskbar Styler (or another
XAML diagnostics consumer for Explorer) enabled. Windows debugging symbols
are downloaded by Windhawk when needed.

## Installation

Create a new local mod in Windhawk, replace its source with this file,
then compile and enable it. Close and reopen Win+Tab after enabling.
Disable the mod to restore the saved layout properties.

## Status

Version 0.7.7 is experimental. Compilation can be checked independently of
Explorer. Live layout, animations, desktop drag and drop, mixed DPI monitors,
and compatibility with individual Windows builds still require testing.
If Windows changes the Task View control names or layout, the mod leaves
unrecognized controls alone. Logs identify when a Task View root is found.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- closeOnWindowsRelease: true
  $name: Close desktop list when Windows key is released
  $description: "List-only modes: hold Windows while using the list, then release it to close. Does not affect Alt+Tab or full Task View."
- disableListAnimations: false
  $name: Disable animations in desktop-list-only modes
- hideNewDesktop: false
  $name: Hide New desktop button
- desktopFrameWidth: "full"
  $name: Desktop list background width
  $options:
  - full: Full available width
  - fit: Fit desktop tiles
  - percent: Percentage of screen
- desktopFramePercent: 70
  $name: Desktop list width (% of screen)
  $description: "Used with Percentage of screen. Range: 10–100. Overflowing desktop tiles remain scrollable."
- viewMode: "0"
  $name: Task View mode (experimental)
  $options:
  - "0": Full Task View
  - "1": Desktop list in the center
  - "2": Desktop list at the top
  $description: "List-only modes hide Task View's window overview and background. Reopen Task View after changing this setting."
- desktopFrameMargin: 8
  $name: Virtual desktop list frame margin (px)
  $description: "Space around the desktop list frame. Default: 8. Range: 0–200. The top margin also includes the existing 32 px taskbar clearance."
- scrollAnimationSpeed: 100
  $name: Desktop scroll animation speed (%)
  $description: "100 = current Windows speed (default). 50 = half speed. 200 = double speed. Range: 10–500. Applies to horizontal scrolling that reveals a desktop tile."
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

#include <atomic>
#include <cmath>
#include <memory>
#include <mutex>
#include <vector>

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
std::atomic<bool> g_hideNewDesktop{false};
std::atomic<int> g_frameWidthMode{0};
std::atomic<int> g_framePercent{70};
std::atomic<bool> g_disableListAnimations{false};
std::atomic<bool> g_closeOnWindowsRelease{true};
void LoadFrameSettings() {
    g_closeOnWindowsRelease = Wh_GetIntSetting(L"closeOnWindowsRelease") != 0;
    g_disableListAnimations = Wh_GetIntSetting(L"disableListAnimations") != 0;
    g_hideNewDesktop = Wh_GetIntSetting(L"hideNewDesktop") != 0;
    auto width = Wh_GetStringSetting(L"desktopFrameWidth");
    g_frameWidthMode = width && wcscmp(width, L"fit") == 0 ? 1 :
                       width && wcscmp(width, L"percent") == 0 ? 2 : 0;
    Wh_FreeStringSetting(width);
    g_framePercent = (std::max)(10, (std::min)(Wh_GetIntSetting(L"desktopFramePercent"), 100));
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

struct LayoutState {
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
    winrt::weak_ref<controls::Grid> desktopGrid;
    winrt::weak_ref<controls::StackPanel> desktopStack;
    winrt::weak_ref<controls::ScrollViewer> desktopScroll;
    winrt::weak_ref<xaml::FrameworkElement> newDesktopButton;
    winrt::weak_ref<xaml::FrameworkElement> desktopBackground;
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

    void Apply() {
        if (g_insideLayout || g_stopping) return;
        LayoutGuard guard;
        auto desktopElement = desktops.get();
        auto windowElement = windows.get();
        if (!desktopElement || !windowElement) return;

        auto layoutGrid = grid.get();
        if (!layoutGrid) return;
        int mode = g_viewMode.load();
        // A transparent background receives clicks in the empty part of this
        // XAML surface without changing the underlying desktop's appearance.
        if (mode && !inputBackgroundApplied) {
            layoutGrid.Background(media::SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0}));
            inputBackgroundApplied = true;
        } else if (!mode && inputBackgroundApplied) {
            originalInputBackground.Restore(layoutGrid);
            inputBackgroundApplied = false;
        }
        for (auto const& saved : modeElements) {
            if (auto element = saved.element.get()) {
                if (mode) {
                    if (element.Opacity() != 0) element.Opacity(0);
                    if (element.IsHitTestVisible()) element.IsHitTestVisible(false);
                } else if (element.Opacity() == 0) {
                    saved.opacity.Restore(element);
                    saved.hitTest.Restore(element);
                }
            }
        }
        if (!applied) {
            controls::RowDefinition desktopRow;
            controls::RowDefinition windowRow;
            desktopRow.Height({1, xaml::GridUnitType::Auto});
            windowRow.Height({1, xaml::GridUnitType::Star});
            layoutGrid.RowDefinitions().Clear();
            layoutGrid.RowDefinitions().Append(desktopRow);
            layoutGrid.RowDefinitions().Append(windowRow);
            Wh_SetIntValue(L"layoutApplied", 1);
            Wh_Log(L"Applied Task View desktop row 0 and window row 1");
        }
        if (controls::Grid::GetRow(desktopElement) != 0) controls::Grid::SetRow(desktopElement, 0);
        int desktopSpan = mode == 1 ? 2 : 1;
        if (controls::Grid::GetRowSpan(desktopElement) != desktopSpan) controls::Grid::SetRowSpan(desktopElement, desktopSpan);
        if (controls::Grid::GetRow(windowElement) != 1) controls::Grid::SetRow(windowElement, 1);
        if (controls::Grid::GetRowSpan(windowElement) != 1) controls::Grid::SetRowSpan(windowElement, 1);
        auto alignment = mode == 1 ? xaml::VerticalAlignment::Center : xaml::VerticalAlignment::Top;
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
        if (mode == 1) desktopMargin.Top = frameMargin;
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
        for (auto const& saved : modeElements) {
            if (auto element = saved.element.get()) {
                saved.opacity.Restore(element);
                saved.hitTest.Restore(element);
            }
        }
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
        }
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
            wchar_t name[128]{};
            HWND foreground = GetForegroundWindow();
            GetClassNameW(foreground, name, ARRAYSIZE(name));
            DWORD process = 0;
            DWORD foregroundThread = GetWindowThreadProcessId(foreground, &process);
            if (g_viewMode == 0 || !g_closeOnWindowsRelease || process != GetCurrentProcessId() ||
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
            if (g_stopping || g_viewMode == 0) return;
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
        if (g_stopping || g_viewMode == 0 || args.Key() != winrt::Windows::System::VirtualKey::Tab) return;
        try {
            auto current = weak.lock();
            if (!current) return;
            auto stack = current->desktopStack.get();
            if (!stack) return;
            current->Navigate((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? -1 : 1);
            args.Handled(true);
        } catch (...) {}
    });
    for (auto const* name : {L"SwitchItemListControl", L"BackgroundThumbnailHost", L"BackgroundDimmingLayer"}) {
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
    animation::Timeline timeline{nullptr};
    winrt::weak_ref<xaml::FrameworkElement> target;
};
thread_local std::vector<AnimationTarget> g_animationTargets;
void ClearAnimationTargets() { g_animationTargets.clear(); }
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
            if (SUCCEEDED(timelineResult) && SUCCEEDED(targetResult) && timeline && element && element.Name() == L"VirtualDesktopBar") {
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
                    auto existing = it->timeline;
                    if (!existing || existing == timeline) it = g_animationTargets.erase(it);
                    else ++it;
                }
                g_animationTargets.push_back({timeline, winrt::make_weak(element)});
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
        if (tracked.timeline != timeline) continue;
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
                frame.Value(-std::abs(frame.Value()));
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
            skip = g_viewMode != 0 && g_disableListAnimations && !g_animationTargets.empty();
            { LayoutGuard guard; ReverseDesktopSlide(storyboard); }
            g_animationTargets.clear();
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

using WindowThreadProc = void(WINAPI*)(void*);
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
    const wchar_t* viewMode = Wh_GetStringSetting(L"viewMode");
    g_viewMode = (std::max)(0, (std::min)(viewMode ? _wtoi(viewMode) : 0, 2));
    Wh_FreeStringSetting(viewMode);
    g_desktopFrameMargin = (std::max)(0, (std::min)(Wh_GetIntSetting(L"desktopFrameMargin"), 200));
    int speed = Wh_GetIntSetting(L"scrollAnimationSpeed");
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
        L"Windhawk.TaskViewDesktopsOnTop.RunOnThread");
    return g_threadMessage && WindhawkUtils::SetFunctionHook(
        CreateWindowExW, CreateWindowExW_Hook, &CreateWindowExW_Original);
}

void Wh_ModAfterInit() {
    EnumWindows([](HWND window, LPARAM) -> BOOL {
        DWORD processId;
        GetWindowThreadProcessId(window, &processId);
        if (processId != GetCurrentProcessId()) return TRUE;
        InitializeWindow(window);
        EnumChildWindows(window, [](HWND child, LPARAM) -> BOOL {
            InitializeWindow(child);
            return TRUE;
        }, 0);
        return TRUE;
    }, 0);
    if (!g_hooksReady) Wh_Log(L"Waiting for an Explorer XAML window");
}

void Wh_ModSettingsChanged() {
    LoadFrameSettings();
    const wchar_t* viewMode = Wh_GetStringSetting(L"viewMode");
    g_viewMode = (std::max)(0, (std::min)(viewMode ? _wtoi(viewMode) : 0, 2));
    Wh_FreeStringSetting(viewMode);
    g_desktopFrameMargin = (std::max)(0, (std::min)(Wh_GetIntSetting(L"desktopFrameMargin"), 200));
    int speed = Wh_GetIntSetting(L"scrollAnimationSpeed");
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
        EnumThreadWindows(state->threadId, [](HWND window, LPARAM parameter) -> BOOL {
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
