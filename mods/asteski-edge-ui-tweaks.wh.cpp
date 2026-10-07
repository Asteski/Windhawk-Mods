// ==WindhawkMod==
// @id              asteski-edge-ui-tweaks
// @name            Edge UI Tweaks
// @description     Customize Edge menus, fonts, favorites, tabs and extensions; preserve menu corners and optionally remove shadows
// @version         0.3.25
// @author          Asteski
// @github          https://github.com/Asteski
// @include         msedge.exe
// @include         msedgewebview2.exe
// @architecture    x86-64
// @compilerOptions -lshell32 -lgdi32
// ==/WindhawkMod==

// Local updates: windhawk-cli mod install --file <this file> compiles and
// replaces the installed source. `mod compile` only rebuilds stored source.

// ==WindhawkModReadme==
/*
# Edge UI Tweaks

Combined Edge port of **Chrome UI Tweaks** by Vasher and **Chrome Native UI
Tweaks** by Dron007, with the original shadow-independent rounded-corner fix.

- **Browser interface:** native UI font family and configurable folder/new-tab
  glyphs; consistent Fluent left/right/up/down chevrons by default.
- **Menus:** icon and shortcut-label visibility, font size, row spacing, selection and popup corners, separators,
  submenu arrows/overlap/delay, scrolling arrows, bubble borders, shadow removal
  and separate main/submenu shadow elevations.
- **Favorites:** centered bar items, icon visibility, font size, Windows folder icons, optional suppression of the editor when
  adding a new favorite.
- **Address bar:** text font size.
- **Tabs:** horizontal tab centering, title font, close buttons and icon-to-title spacing.
- **Extensions:** pinned button width.

Menus have a single set of spacing and corner controls shared by the two ports.
The corner and shadow options also cover Windhawk UI's own WebView2 menus;
other applications' WebViews are rejected. Browser-only tweaks run in Edge.
Web page content is untouched. Context-menu item hiding/reordering from the
separate Chrome Context Menu Items mod is not part of this release.

## Settings

**Browser interface / Browser font family** defaults to **Segoe UI Variable
Text**, the installed text face of Segoe UI Variable. Clear it for Edge's font.
This affects native browser text, retaining size, weight, italics, symbol and
emoji fonts. Websites, the new-tab webpage and web-based Edge surfaces keep
their own fonts. No website-enforcement option is installed.

**Glyph replacements** use **Segoe Fluent Icons** by default. Folder/new-tab
replacements start blank (native icons); chevrons use the Windows 11 glyphs.
Enter a single character or `U+E76C`-style code point. Blank, invalid, unavailable
fonts or missing glyphs fall back to Edge. Replacements cover the mapped native
icons, not website graphics. Images retain Edge's color/opacity and use a 2x
raster representation; scaling beyond 200% may soften them. A custom folder
glyph takes precedence over the Windows-folder option.

Visibility switches start on; centering switches start off. Centering targets
native horizontal tabs and favorites that fit their available region, retaining
overflow and fixed favorites controls. Menu checkmarks and submenu arrows remain
visible when item icons are hidden. Keyboard shortcuts still work when hidden.

**-1** restores Edge's original value. Sizes are device-independent pixels
unless specified otherwise. For the two options allowing negative offsets,
**-2** means -1 DIP, **-3** means -2 DIP, matching Chrome UI Tweaks.

**Hide menu shadows** is independent of **Keep rounded corners when Windows
shadows are disabled**. It suppresses both the drawn bubble shadow and the
native menu-window shadow. Shadow elevation settings apply only when hiding
shadows is off. No Windows-wide shadow preference is changed.

The same **Keep rounded corners** switch now covers native Edge toolbar popups
(including extension bubbles) and dialog frames. It bypasses only their corner
and clipping checks; shadow-selection checks keep the real Windows preference.
Close and reopen popups after changing the setting. History/download surfaces
still need visual confirmation on each Edge layout.

`corner_radius` covers regular/bubble menus and native toolbar bubbles; `auxiliary_corner_radius` covers
comboboxes and non-bubble context menus. Touch-mode corners retain Edge's routing.

Close and reopen menus after saving. After installing/enabling, fully restart
Edge once to capture existing browser controls. **Font-family changes require
a full Edge restart**, including background processes. Cached glyphs and icon
visibility may also require a restart. Disabling the mod requires a restart to
restore already-created font and image objects. Captured font sizes, tab controls
and extension buttons update live. Favorites icons may need a new window if
its bar existed before injection. The Windows folder icon is a fixed raster
icon and can look different at high DPI. Disabling restores tracked controls
and the original menu configuration; close any open menus afterward.

## Compatibility and validation

The extended port targets **Edge and Edge WebView2 154.0.4258.48 / .53 x64**. It
checks the DLL's exact CodeView identity and every hook entry before using
verified Edge offsets. Other builds retain only the original corner fix if
its full signature and helpers match; extended controls do not apply there.
No runtime debug-symbol download is needed. ARM64 and 32-bit builds are not
supported. Edge updates can require an updated hook map.

The original corner fix was visually confirmed in Edge and Windhawk UI.
New features need visual testing across menu types, DPI, themes and tab modes.
The extended port is experimental; source/test validation is not a visual test.

MenuConfig option design: Vasher's Chrome UI Tweaks. Native font, control
tracking, folder-image and UI-thread update routines: Dron007's Chrome Native
UI Tweaks, adapted for Edge. Original project licenses apply. Chromium menu
behavior was checked against its MenuHost, MenuConfig and BubbleBorder sources.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- interface:
  - font_family: Segoe UI Variable Text
    $name: Browser font family
    $description: 'Native browser text only; websites and web-based Edge surfaces keep their own fonts. Blank restores Edge. Fully exit and restart Edge after changing. Symbol and emoji fonts are preserved.'
  - glyphs:
    - font_family: Segoe Fluent Icons
      $name: Glyph font
      $description: 'Font containing the replacement characters below. Segoe Fluent Icons matches Windows 11.'
    - folder: ''
      $name: Folder glyph
      $description: 'Blank keeps the native icon. Enter one character or a Unicode value such as U+E8B7. Overrides Use Windows folder icons when set.'
    - new_tab: ''
      $name: New-tab glyph
      $description: 'Blank keeps the native icon. Enter one character or a Unicode value such as U+E710.'
    - right: U+E76C
      $name: Right chevron
      $description: 'Fluent right chevron, including submenu arrows. Blank keeps Edge.'
    - left: U+E76B
      $name: Left chevron
      $description: 'Fluent left chevron. Blank keeps Edge.'
    - down: U+E70D
      $name: Down chevron
      $description: 'Fluent down chevron for native dropdown and overflow controls. Blank keeps Edge.'
    - up: U+E70E
      $name: Up chevron
      $description: 'Fluent up chevron. Blank keeps Edge.'
    $name: Glyph replacements
  $name: Browser interface
- menus:
  - text:
    - font_size: -1
      $name: Font size
      $description: '-1 uses the original font. Otherwise 12–48 pixels.'
    - show_shortcuts: true
      $name: Show keyboard shortcut labels
      $description: 'Hide labels such as Ctrl+C without disabling the shortcuts. Reopen menus after changing.'
    $name: Text
  - icons:
    - show_icons: true
      $name: Show menu item icons
      $description: 'Native menu item icons and glyphs. Checkmarks and submenu arrows remain visible. Reopen menus after changing.'
    $name: Icons
  - items:
    - item_vertical_margin: -1
      $name: Top and bottom spacing
      $description: 'Clickable padding inside a row. -1 uses Edge; 0–64 DIP.'
    - between_item_vertical_padding: -1
      $name: Gap between rows
      $description: 'Non-clickable space between rows. -1 uses Edge; 0–64 DIP.'
    - item_horizontal_padding: -1
      $name: Space between row components
      $description: '-1 uses Edge; 0–64 DIP.'
    - item_horizontal_border_padding: -1
      $name: Left and right padding
      $description: '-1 uses Edge. Negative offsets use the original mod convention: -3 means -2 DIP. Range -65–64.'
    $name: Item spacing
  - corners:
    - preserve_rounded_corners: true
      $name: Keep rounded corners when Windows shadows are disabled
      $description: 'Preserve Edge menu, native toolbar popup and dialog corners. Also covers Windhawk UI menus. Does not enable Windows shadows.'
    - corner_radius: -1
      $name: corner_radius — regular menus and toolbar popups
      $description: '-1 uses Edge; 0 is square; 1–64 DIP is a custom radius.'
    - auxiliary_corner_radius: -1
      $name: auxiliary_corner_radius — auxiliary menus
      $description: 'Comboboxes and non-bubble context menus. -1 uses Edge; 0–64 DIP.'
    - item_corner_radius: -1
      $name: Selected row corner radius
      $description: '-1 uses Edge; 0–64 DIP.'
    - use_bubble_border: -1
      $name: Bubble border
      $description: '-1 uses Edge; 0 disables it; 1 enables it.'
    $name: Corners and border
  - separators:
    - separator_height: -1
      $name: Normal separator height
      $description: '-1 uses Edge. Values are in DIP.'
    - separator_upper_height: -1
      $name: Upper separator height
      $description: '-1 uses Edge. Values are in DIP.'
    - separator_lower_height: -1
      $name: Lower separator height
      $description: '-1 uses Edge. Values are in DIP.'
    - separator_spacing_height: -1
      $name: Blank separator height
      $description: '-1 uses Edge. Values are in DIP.'
    - separator_thickness: -1
      $name: Line thickness
      $description: '-1 uses Edge. Values are in DIP.'
    - double_separator_height: -1
      $name: Double separator height
      $description: '-1 uses Edge. Values are in DIP.'
    - double_separator_thickness: -1
      $name: Double separator line thickness
      $description: '-1 uses Edge. Values are in DIP.'
    - separator_horizontal_border_padding: -1
      $name: Left and right padding
      $description: '-1 uses Edge. Values are in DIP.'
    - padded_separator_start_padding: -1
      $name: Indented separator start padding
      $description: '-1 uses Edge. Values are in DIP.'
    $name: Separators
  - submenus:
    - show_delay: -1
      $name: Submenu opening delay
      $description: '-1 uses Edge; 0–5000 milliseconds.'
    - arrow_size: -1
      $name: Submenu arrow size
      $description: '-1 uses Edge; 0–64 DIP.'
    - arrow_to_edge_padding: -1
      $name: Arrow-to-edge spacing
      $description: '-1 uses Edge; 0–64 DIP.'
    - submenu_horizontal_overlap: -1
      $name: Submenu overlap
      $description: '-1 uses Edge. Positive values overlap; -3 creates a 2 DIP gap. Range -65–64.'
    - scroll_arrow_height: -1
      $name: Scroll arrow height
      $description: '-1 uses Edge; 0–64 DIP.'
    $name: Submenus and scrolling
  - shadows:
    - hide_shadows: true
      $name: Hide menu shadows
      $description: 'Suppress both the drawn menu shadow and the menu-window shadow in Edge and Windhawk UI. Corners remain controlled by the corner settings. Close and reopen menus after changing this.'
    - bubble_menu_shadow_elevation: -1
      $name: Main menu shadow elevation
      $description: 'Used only when Hide menu shadows is off. -1 uses Edge; 0–64.'
    - bubble_submenu_shadow_elevation: -1
      $name: Submenu shadow elevation
      $description: 'Used only when Hide menu shadows is off. -1 uses Edge; 0–64.'
    $name: Shadows
  $name: Menus
- favorites:
  - appearance:
    - show_icons: true
      $name: Show favorites bar icons
      $description: 'Show website favicons on the favorites bar. Folder icons remain visible. Fully restart Edge after changing.'
    - font_size: -1
      $name: Favorites bar font size
      $description: '-1 uses Edge; 12–48 pixels. New browser windows may be needed after first enabling the mod.'
    - windows_folder_icon: false
      $name: Use Windows folder icons
      $description: 'Replace favorites folder icons with the Windows system icon. This is a fixed raster icon.'
    $name: Appearance
  - layout:
    - center: false
      $name: Center favorites
      $description: 'Center native favorites in the available bar space. Fixed controls and overflowing favorites keep their native layout. Fully restart Edge after changing.'
    $name: Layout
  - behavior:
    - hide_editor_on_add: false
      $name: Hide the editor when adding a favorite
      $description: 'Save new favorites without opening the editor. Existing favorites can still be edited. Disabled preserves Edge behavior.'
    $name: Behavior
  $name: Favorites
- address_bar:
  - font_size: -1
    $name: Font size
    $description: '-1 uses Edge; 12–48 pixels. Applies to URL and search text.'
  - appearance:
    - corner_radius: -1
      $name: Corner radius
      $description: '-1 follows Edge; 0�32 DIP. Try 4 for a File Explorer-like shape. Restart Edge after changing.'
    - hide_border: false
      $name: Hide address field border
    - hide_focus_outline: false
      $name: Hide focus outline
      $description: Hide the blue outline when typing in the address bar. Text selection remains visible; high contrast keeps its focus indicator.
    - border_color: ""
      $name: Border color
      $description: 'Hex RGB, for example #D0D0D0. Blank follows Edge. Ignored when the border is hidden.'
    - background_color: ""
      $name: Background color
      $description: 'Hex RGB, for example #FFFFFF. Blank follows Edge. Choose a color that keeps text readable in your theme. Restart Edge after changing.'
    $name: Field appearance
  - icons:
    - size: 18
      $name: Glyph size
      $description: 'Visible glyph size in physical pixels (8–48), excluding transparent padding; -1 follows Edge. The longest visible dimension is sized, preserving proportions. Pixel rounding and available button space can limit large sizes. Restart Edge after changing.'
    $name: Glyphs
  $name: Address bar
- tabs:
  - appearance:
    - common_color: false
      $name: Apply common color everywhere
      $description: 'Match the horizontal tab bar to the active-tab color, including custom themes and workspaces. When off, the default dark theme uses #1D1D1D. Vertical tabs and high contrast retain their native colors. Restart Edge after changing.'
    - active_title_weight: regular
      $name: Active tab title weight
      $options:
      - light: Light
      - semilight: Semilight
      - regular: Regular
      - semibold: Semibold
      - bold: Bold
    - active_title_italic: false
      $name: Italic active tab title
    - active_title_underline: false
      $name: Underline active tab title
    - font_size: -1
      $name: Title font size
      $description: 'Horizontal tab titles. -1 uses Edge; 12–48 pixels.'
    $name: Appearance
  - layout:
    - center: false
      $name: Center horizontal tabs
      $description: 'Center the native tab container when the tabs fit. Overflow keeps the normal layout. Vertical tabs are unaffected. Fully restart Edge after changing.'
    - close_buttons: show
      $name: Close buttons
      $options:
      - show: Show
      - hover: Show on hover
      - hide: Hide
      $description: 'Show normally, show only while the tab is hovered, or hide and reclaim the reserved space.'
    - icon_title_spacing: -1
      $name: Space between icon and title
      $description: '-1 uses Edge; 0–32 DIP.'
    $name: Layout
  $name: Tabs
- content:
  - hide_border: false
    $name: Hide border around website content
    $description: 'Remove the decorative outline, surrounding space, and the Windows-drawn outer border of Edge main windows. Screen-sharing indicators remain unchanged. Restart Edge after changing.'
  - square_corners: false
    $name: Remove rounded website corners
    $description: 'Use square website-content corners, independently of border visibility. Restart Edge after changing.'
  $name: Website frame
- toolbar:
  - hide_extensions: false
    $name: Hide extensions button
  - hide_profile: false
    $name: Hide profile button
  $name: Toolbar buttons
- extensions:
  - button_width: -1
    $name: Pinned extension button width
    $description: 'Icon width plus horizontal padding. -1 keeps Edge defaults; 20–64 DIP.'
  $name: Extensions
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <windhawk_api.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <string>
#include <vector>
#include <span>
#include <cmath>

namespace {
bool g_isWebview = false;
void LoadExtraSettings();

bool IsSupportedProcess(const wchar_t* filename, int argc, wchar_t* const* argv,
                        bool windhawkParent) {
    const bool edge = _wcsicmp(filename, L"msedge.exe") == 0;
    const bool webview = _wcsicmp(filename, L"msedgewebview2.exe") == 0;
    if (!edge && !webview) return false;
    bool windhawkHost = false;
    for (int i = 1; i < argc; ++i) {
        if (wcsncmp(argv[i], L"--type=", 7) == 0 || wcscmp(argv[i], L"--type") == 0)
            return false;
        if (_wcsicmp(argv[i], L"--webview-exe-name=windhawk-ui.exe") == 0)
            windhawkHost = true;
    }
    return edge || (windhawkHost && windhawkParent);
}

bool HasWindhawkUiParent() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    DWORD parentId = 0;
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (entry.th32ProcessID == GetCurrentProcessId()) {
                parentId = entry.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    HANDLE parent = parentId ? OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, parentId) : nullptr;
    if (!parent) return false;
    wchar_t path[32768];
    DWORD length = ARRAYSIZE(path);
    const bool queried = QueryFullProcessImageNameW(parent, 0, path, &length);
    CloseHandle(parent);
    if (!queried) return false;
    const wchar_t* name = wcsrchr(path, L'\\');
    return _wcsicmp(name ? name + 1 : path, L"windhawk-ui.exe") == 0;
}

// Full CornerRadiusForMenu body from Edge 154.0.4258.48. Only the three CALL
// displacements are variable. Thus every object-field offset, branch, and
// instruction used below is checked before we accept this implementation.
constexpr unsigned char kRadiusCode[] = {
    0x56,0x57,0x53,0x48,0x83,0xec,0x20,0x48,0x89,0xd7,0x48,0x89,0xce,0x31,0xdb,
    0xe8,0,0,0,0,0x84,0xc0,0x74,0x25,0x48,0x85,0xff,0x74,0x1a,
    0x80,0xbf,0x7a,0x02,0,0,0x01,0x75,0x21,0xf6,0x87,0x68,0x03,0,0,0x01,
    0x75,0x0e,0x8b,0x9e,0x34,0x01,0,0,0xeb,0x06,0x8b,0x9e,0x2c,0x01,0,0,
    0x89,0xd8,0x48,0x83,0xc4,0x20,0x5b,0x5f,0x5e,0xc3,0x48,0x89,0xf9,
    0xe8,0,0,0,0,0x84,0xc0,0x75,0x15,0x80,0xbe,0x28,0x01,0,0,0,0x75,0xdb,
    0x48,0x89,0xf9,0xe8,0,0,0,0,0x84,0xc0,0x74,0xcf,0x8b,0x9e,0x30,0x01,0,0,
    0xeb,0xcd,
};
static_assert(sizeof(kRadiusCode) == 112);
constexpr size_t kCalls[] = {15, 74, 95};

using RadiusFn = int (*)(const void*, const void*);
using PredicateFn = bool (*)(const void*);
using ShadowsFn = bool (*)();
RadiusFn g_originalRadius;
PredicateFn g_isCombobox;
PredicateFn g_isContextMenu;
ShadowsFn g_shadowsEnabled;
std::atomic<uint32_t> g_settings{1u << 16};
std::atomic<bool> g_stopping{false};
std::atomic<bool> g_installStarted{false};
std::atomic<bool> g_installed{false};
decltype(&LoadLibraryExW) g_originalLoadLibraryExW;

int NormalizeRadius(int value) {
    return std::clamp(value, -1, 64);
}

void LoadSettings() {
    LoadExtraSettings();
    const auto radius = NormalizeRadius(Wh_GetIntSetting(L"menus.corners.corner_radius"));
    const auto auxiliary = NormalizeRadius(Wh_GetIntSetting(L"menus.corners.auxiliary_corner_radius"));
    g_settings.store(static_cast<uint32_t>(radius + 1) |
                     (static_cast<uint32_t>(auxiliary + 1) << 8) |
                     (Wh_GetIntSetting(L"menus.corners.preserve_rounded_corners") ? 1u << 16 : 0));
}

template<typename T>
T ReadField(const void* object, size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(object) + offset,
                sizeof(value));
    return value;
}

int RadiusHook(const void* config, const void* controller) {
    if (g_stopping.load()) {
        return g_originalRadius(config, controller);
    }
    const uint32_t settings = g_settings.load();
    if (!(settings & (1u << 16)) && !g_shadowsEnabled()) {
        return g_originalRadius(config, controller);
    }

    // Preserve the native touch/ASH branch, including its explicitly supplied
    // rounded-corners flag. Never write to Edge's singleton or controller.
    if (controller && ReadField<unsigned char>(controller, 0x27a) == 1) {
        return (ReadField<unsigned char>(controller, 0x368) & 1)
                   ? 0 : ReadField<int>(config, 0x134);
    }

    const bool auxiliary = controller &&
        (g_isCombobox(controller) ||
         (!ReadField<unsigned char>(config, 0x128) && g_isContextMenu(controller)));
    const int configured = static_cast<int>((settings >> (auxiliary ? 8 : 0)) & 0xff) - 1;
    return configured >= 0 ? configured : ReadField<int>(config, auxiliary ? 0x130 : 0x12c);
}

struct CodeRange {
    const unsigned char* data;
    size_t size;
};

bool Contains(const std::vector<CodeRange>& ranges, const unsigned char* address,
              size_t size) {
    const auto value = reinterpret_cast<uintptr_t>(address);
    for (const auto& range : ranges) {
        const auto start = reinterpret_cast<uintptr_t>(range.data);
        if (value >= start && value - start <= range.size &&
            size <= range.size - (value - start)) {
            return true;
        }
    }
    return false;
}

bool MatchesRadius(const unsigned char* code) {
    for (size_t i = 0; i < sizeof(kRadiusCode); i++) {
        bool displacement = false;
        for (size_t call : kCalls) {
            displacement |= i > call && i <= call + 4;
        }
        if (!displacement && code[i] != kRadiusCode[i]) {
            return false;
        }
    }
    return true;
}

const unsigned char* RelativeTarget(const unsigned char* instruction) {
    return reinterpret_cast<const unsigned char*>(
        reinterpret_cast<intptr_t>(instruction) + 5 + ReadField<int32_t>(instruction, 1));
}

bool ValidateHelpers(const unsigned char* code, const std::vector<CodeRange>& ranges) {
    const auto* shadows = RelativeTarget(code + kCalls[0]);
    const auto* combobox = RelativeTarget(code + kCalls[1]);
    const auto* context = RelativeTarget(code + kCalls[2]);
    constexpr unsigned char comboCode[] = {
        0x8b,0x81,0x74,0x02,0,0,0xff,0xc8,0x83,0xf8,0x02,0x0f,0x92,0xc0,0xc3};
    constexpr unsigned char contextCode[] = {
        0x8b,0x81,0xa8,0,0,0,0xff,0xc8,0x83,0xf8,0x02,0x0f,0x92,0xc0,0xc3};
    if (!Contains(ranges, combobox, sizeof(comboCode)) ||
        !Contains(ranges, context, sizeof(contextCode)) ||
        std::memcmp(combobox, comboCode, sizeof(comboCode)) ||
        std::memcmp(context, contextCode, sizeof(contextCode)) ||
        !Contains(ranges, shadows, 5)) {
        return false;
    }
    // This build has a JMP entry thunk for the shadow helper.
    const auto* body = shadows;
    if (body[0] == 0xe9) {
        body = RelativeTarget(body);
    }
    constexpr unsigned char shadowQuery[] = {
        0xb9,0x24,0x10,0,0,0x31,0xd2,0x49,0x89,0xf0,0x45,0x31,0xc9,0xff,0x15};
    if (!Contains(ranges, body, 0x5d) ||
        std::memcmp(body, "\x56\x48\x83\xec\x30", 5) ||
        std::memcmp(body + 0x1f, shadowQuery, sizeof(shadowQuery))) {
        return false;
    }
    g_shadowsEnabled = reinterpret_cast<ShadowsFn>(const_cast<unsigned char*>(shadows));
    g_isCombobox = reinterpret_cast<PredicateFn>(const_cast<unsigned char*>(combobox));
    g_isContextMenu = reinterpret_cast<PredicateFn>(const_cast<unsigned char*>(context));
    return true;
}

// Native UI routines adapted from Dron007's Chrome Native UI Tweaks.
// Edge ABI adjustments and the menu configuration port are specific to the
// CodeView identity checked below. Never use these addresses on another build.
namespace Native {
static std::mutex g_browserStyleMutex;
static std::string g_browserFont;
static std::wstring g_glyphFont;
enum class GlyphKind { folder, new_tab, right, left, down, up, count };
static std::wstring g_glyphs[static_cast<size_t>(GlyphKind::count)];
static std::atomic<bool> g_showMenuIcons{true}, g_showShortcuts{true};
static std::atomic<bool> g_showFavoriteIcons{true}, g_centerFavorites{false}, g_centerTabs{false};
static std::atomic<bool> g_commonTabColor{false};
static std::mutex g_bookmarkButtonsMutex;
static std::unordered_map<void*, DWORD> g_bookmarkButtons;
static bool HasFolderGlyph() {
    std::lock_guard<std::mutex> lock(g_browserStyleMutex);
    return !g_glyphs[0].empty() && !g_stopping.load();
}
static constexpr wchar_t kChromeWidgetWindowClassPrefix[] = L"Chrome_WidgetWin_";
static constexpr int kChromeDefaultTabPreTitlePadding = -1;
static constexpr int kChromeDefaultExtensionButtonWidth = -1;
static constexpr int kLayoutTabAfterTitlePadding = 0x85;
static constexpr int kLayoutTabCloseButtonSize = 0x88;
static constexpr int kLayoutTabPreTitlePadding = 0x8d;
struct FontListOpaque;
struct ImageModelOpaque;

struct GfxSizeOpaque {
  int width;
  int height;
};

static_assert(sizeof(GfxSizeOpaque) == 8);

// The Windows-folder image is created once while the mod is loaded. Chromium
// object layouts aren't available to this mod at compile time, so use
// generously oversized storage and verify a trailing guard
// after each foreign constructor call. If a guard is overwritten, don't run
// the foreign destructor on an object whose representation may be corrupted.
static constexpr size_t kOpaqueObjectStorageSize = 4096;
static constexpr size_t kOpaqueObjectGuardSize = 256;
static constexpr unsigned char kOpaqueObjectGuardValue = 0xA5;

struct alignas(64) OpaqueObjectStorage {
  unsigned char data[kOpaqueObjectStorageSize];
  unsigned char guard[kOpaqueObjectGuardSize];
};

static OpaqueObjectStorage g_skBitmapStorage;
static OpaqueObjectStorage g_imageSkiaStorage;

static void PrepareOpaqueObjectStorage(OpaqueObjectStorage& storage) {
  std::fill_n(storage.guard, kOpaqueObjectGuardSize, kOpaqueObjectGuardValue);
}

static bool IsOpaqueObjectGuardIntact(const OpaqueObjectStorage& storage) {
  return std::all_of(storage.guard, storage.guard + kOpaqueObjectGuardSize,
                     [](unsigned char value) { return value == kOpaqueObjectGuardValue; });
}

// -----------------------------------------------------------------------------
// Function types
// -----------------------------------------------------------------------------

using BookmarkButtonBaseCtorFn = void (*)(void*, void*, const void*);

using BookmarkMenuButtonBaseCtorFn = void (*)(void*, void*, void*, const void*);

using LabelButtonLabelCtorFn = void (*)(void*, const void*, int);

using LabelButtonLabelDeletingDtorFn = void* (*)(void*, unsigned int);

using TypographyGetFontFn = const FontListOpaque* (*)(const void*, int, int);

using LabelSetFontListFn = void (*)(void*, const FontListOpaque*);

// Chrome 152 declares MenuItemView::GetFontList() as returning
// `const gfx::FontList` by value. On Win64 the member-function ABI uses:
// RCX = this, RDX = hidden result buffer.
using MenuItemGetFontListFn = FontListOpaque* (*)(const void*, FontListOpaque*);



using FontListCopyCtorFn = void (*)(FontListOpaque*, const FontListOpaque*);

using FontListGetFontSizeFn = int (*)(const FontListOpaque*);

// gfx::FontList is returned through a hidden sret buffer on Win64:
// RCX = this
// RDX = result buffer
// R8  = size delta
using FontListDeriveWithSizeDeltaFn =
    FontListOpaque* (*)(const FontListOpaque*, FontListOpaque*, int);

using FontListDtorFn = void (*)(FontListOpaque*);

// OmniboxViewViews(bool, OmniboxController*, LocationBarView*,
//                  const gfx::FontList&)
using OmniboxViewViewsCtorFn =
    void (*)(void*, bool, void*, void*, const FontListOpaque*);

using OmniboxViewViewsDtorFn = void* (*)(void*, unsigned int);

using TextfieldCtorFn = void (*)(void*);
using TextfieldSetFontListFn = void (*)(void*, const FontListOpaque*);

using TabTitleCtorFn = void (*)(void*);
using TabTitleDtorFn = void* (*)(void*, unsigned int);

using TabCloseButtonCtorFn = void (*)(void*, void*, void*);

using TabCloseButtonDtorFn = void (*)(void*);

using ViewSetVisibleFn = void (*)(void*, bool);

using ViewInvalidateLayoutFn = void (*)(void*, bool);

using ViewPreferredSizeChangedFn = void (*)(void*);

using GetLayoutConstantFn = int (*)(int);

using ToolbarActionViewCtorFn = void (*)(void*, void*, void*);

// gfx::Size is returned through a hidden sret buffer for this Win64 C++
// instance method:
// RCX = this
// RDX = result buffer
// R8  = const views::SizeBounds&
using ToolbarActionViewCalculatePreferredSizeFn = GfxSizeOpaque* (*)(const void*, GfxSizeOpaque*, const void*);

using ToolbarActionViewDeletingDtorFn = void* (*)(void*, unsigned int);

using ToolbarActionViewUpdateStateFn = void (*)(void*);

// ExtensionsToolbarDesktop(Browser*, DisplayMode)
using ExtensionsToolbarDesktopCtorFn = void (*)(void*, void*, int);

using ExtensionsToolbarDesktopDeletingDtorFn = void* (*)(void*, unsigned int);

// Logical Chromium signature:
//   ui::ImageModel chrome::GetBookmarkFolderIcon(
//       BookmarkFolderIconType icon_type,
//       ui::ColorVariant color);
// Win64 ABI uses a hidden ImageModel result buffer in RCX.
using GetBookmarkFolderIconFn =
    ImageModelOpaque* (*)(ImageModelOpaque* result, int iconType, uintptr_t colorVariantOpaque);

// static SkBitmap IconUtil::CreateSkBitmapFromHICON(HICON);
using CreateSkBitmapFromHICONFn = void* (*)(void* result, HICON icon);

// static gfx::ImageSkia gfx::ImageSkia::CreateFrom1xBitmap(const SkBitmap&);
using ImageSkiaCreateFrom1xBitmapFn = void* (*)(void* result, const void* bitmap);

using OpaqueObjectDtorFn = void (*)(void*);

// static ui::ImageModel ui::ImageModel::FromImageSkia(const gfx::ImageSkia&);
using ImageModelFromImageSkiaFn = ImageModelOpaque* (*)(ImageModelOpaque* result, const void* imageSkia);

// BookmarkBarView::UpdateAppearanceForTheme() reconfigures all existing
// bookmark-bar buttons, including their folder ImageModels.
using BookmarkBarViewUpdateAppearanceForThemeFn = void (*)(void*);

using BookmarkBarViewDeletingDtorFn = void* (*)(void*, unsigned int);

// -----------------------------------------------------------------------------
// Resolved functions
// -----------------------------------------------------------------------------

static BookmarkButtonBaseCtorFn g_BookmarkButtonBaseCtorOriginal;
static BookmarkMenuButtonBaseCtorFn g_BookmarkMenuButtonBaseCtorOriginal;
static LabelButtonLabelCtorFn g_LabelButtonLabelCtorOriginal;
static LabelButtonLabelDeletingDtorFn g_LabelButtonLabelDeletingDtorOriginal;
static TypographyGetFontFn g_TypographyGetFontOriginal;
static LabelSetFontListFn g_LabelSetFontList;

static MenuItemGetFontListFn g_MenuItemGetFontListOriginal;
static FontListCopyCtorFn g_FontListCopyCtor;

static FontListGetFontSizeFn g_FontListGetFontSize;
static FontListDeriveWithSizeDeltaFn g_FontListDeriveWithSizeDelta;
static FontListDtorFn g_FontListDtor;

static OmniboxViewViewsCtorFn g_OmniboxViewViewsCtorOriginal;
static OmniboxViewViewsDtorFn g_OmniboxViewViewsDtorOriginal;
static TextfieldCtorFn g_TextfieldCtorOriginal;
static TextfieldSetFontListFn g_TextfieldSetFontListOriginal;

static TabTitleCtorFn g_TabTitleCtorOriginal;
static TabTitleDtorFn g_TabTitleDtorOriginal;
static TabCloseButtonCtorFn g_TabCloseButtonCtorOriginal;
static TabCloseButtonDtorFn g_TabCloseButtonDtorOriginal;

static ViewSetVisibleFn g_ViewSetVisibleOriginal;
static ViewInvalidateLayoutFn g_ViewInvalidateLayout;
static ViewPreferredSizeChangedFn g_ViewPreferredSizeChanged;
static GetLayoutConstantFn g_GetLayoutConstantOriginal;

static ToolbarActionViewCtorFn g_ToolbarActionViewCtorOriginal;

static ToolbarActionViewCalculatePreferredSizeFn g_ToolbarActionViewCalculatePreferredSizeOriginal;

static ToolbarActionViewDeletingDtorFn g_ToolbarActionViewDeletingDtorOriginal;

static ToolbarActionViewUpdateStateFn g_ToolbarActionViewUpdateState;

static ExtensionsToolbarDesktopCtorFn g_ExtensionsToolbarDesktopCtorOriginal;

static ExtensionsToolbarDesktopDeletingDtorFn g_ExtensionsToolbarDesktopDeletingDtorOriginal;

static GetBookmarkFolderIconFn g_GetBookmarkFolderIconOriginal;
static CreateSkBitmapFromHICONFn g_CreateSkBitmapFromHICON;
static ImageSkiaCreateFrom1xBitmapFn g_ImageSkiaCreateFrom1xBitmap;
static OpaqueObjectDtorFn g_SkBitmapDtor;
static OpaqueObjectDtorFn g_ImageSkiaDtor;
static ImageModelFromImageSkiaFn g_ImageModelFromImageSkia;
static BookmarkBarViewUpdateAppearanceForThemeFn g_BookmarkBarViewUpdateAppearanceForThemeOriginal;
static BookmarkBarViewDeletingDtorFn g_BookmarkBarViewDeletingDtorOriginal;

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------

static std::atomic_bool g_hooksActivated = false;

static std::atomic_int g_bookmarkFontSize = -1;
static std::atomic_int g_addressBarFontSize = -1;
static std::atomic_int g_menuFontSize = -1;
static std::atomic_int g_tabFontSize = -1;

static std::atomic_int g_tabPreTitlePadding = kChromeDefaultTabPreTitlePadding;

static std::atomic_int g_extensionButtonWidth = kChromeDefaultExtensionButtonWidth;

// -1 = Chrome default.

static std::atomic_bool g_tabCloseButtonsHidden = false;
static std::atomic<int> g_closeButtonMode{0}, g_activeTitleStyle{400 << 2};
static std::atomic<bool> g_hideContentBorder{false}, g_squareContentCorners{false};
static thread_local const void* g_enteringTab = nullptr;
static std::atomic<bool> g_hideExtensions{false}, g_hideProfile{false};
static bool (*g_isMouseHovered)(const void*);
static bool ShouldHideToolbarButton(const void* view);
static void RememberToolbarVisibility(void* view, bool visible);
static bool IsCloseButtonSuppressed(const void* view) {
    if (g_stopping.load()) return false;
    if (g_tabCloseButtonsHidden.load()) return true;
    if (g_closeButtonMode.load() != 1) return false;
    const void* parent = ReadField<const void*>(view, 0x198);
    return parent && parent != g_enteringTab && g_isMouseHovered && !g_isMouseHovered(parent);
}
static std::atomic_bool g_useWindowsFolderIcon = false;

static std::atomic_bool g_addressBarFontHooksReady = false;
static std::atomic_bool g_tabFontHooksReady = false;
static std::atomic_bool g_tabCloseHooksReady = false;

static std::atomic_bool g_extensionTrackingReady = false;
static std::atomic_bool g_extensionContainerTrackingReady = false;
static std::atomic_bool g_windowsFolderReady = false;
static std::atomic_bool g_bookmarkFolderLiveUpdateReady = false;

static void* (*g_copyImageSkia)(void*, const void*);
static std::mutex g_windowsFolderImageMutex;
static DWORD g_windowsFolderThreadId = 0;

//  0 = not checked
//  1 = compatible
// -1 = incompatible

static thread_local int g_bookmarkCtorDepth = 0;
static thread_local int g_bookmarkMenuCtorDepth = 0;
static thread_local const FontListOpaque* g_pendingBookmarkOriginalFont = nullptr;
static thread_local std::vector<std::unique_ptr<OpaqueObjectStorage>>
    g_bookmarkCtorDerivedFonts;

static thread_local int g_tabTitleCtorDepth = 0;
static thread_local const FontListOpaque* g_pendingTabTitleOriginalFont = nullptr;

static thread_local int g_omniboxCtorDepth = 0;
static thread_local void* g_pendingOmniboxTextfield = nullptr;

struct BookmarkLabelInfo {
  DWORD threadId;
  std::unique_ptr<OpaqueObjectStorage> originalFontStorage;
};

static std::mutex g_labelsMutex;
static std::unordered_map<void*, BookmarkLabelInfo> g_labels;

static std::mutex g_bookmarkBarsMutex;
static std::unordered_map<void*, DWORD> g_bookmarkBars;

struct OmniboxInfo {
  DWORD threadId;
  void* textfield;
  std::unique_ptr<OpaqueObjectStorage> originalFontStorage;
};

static std::mutex g_omniboxesMutex;
static std::unordered_map<void*, OmniboxInfo> g_omniboxes;

struct TabTitleInfo {
  DWORD threadId;
  std::unique_ptr<OpaqueObjectStorage> originalFontStorage;
  int appliedStyle = -1;
};

static std::mutex g_tabObjectsMutex;
static std::unordered_map<void*, TabTitleInfo> g_tabTitles;
static std::unordered_map<void*, DWORD> g_tabCloseButtons;

static std::mutex g_extensionViewsMutex;
static std::unordered_map<void*, DWORD> g_extensionViews;
static std::unordered_map<void*, DWORD> g_extensionContainers;

static const FontListOpaque* GetOwnedFontList(
    const std::unique_ptr<OpaqueObjectStorage>& storage) {
  return storage ? reinterpret_cast<const FontListOpaque*>(storage->data) : nullptr;
}

static std::unique_ptr<OpaqueObjectStorage> CopyFontListToOwnedStorage(
    const FontListOpaque* font, const wchar_t* description) {
  if (!font || !g_FontListCopyCtor || !g_FontListDtor) return nullptr;

  auto storage = std::make_unique<OpaqueObjectStorage>();
  PrepareOpaqueObjectStorage(*storage);

  auto* copy = reinterpret_cast<FontListOpaque*>(storage->data);
  g_FontListCopyCtor(copy, font);

  if (!IsOpaqueObjectGuardIntact(*storage)) {
    Wh_Log(L"%ls storage guard was overwritten; skipping destructor", description);
    return nullptr;
  }

  return storage;
}

static void DestroyOwnedFontListStorage(
    std::unique_ptr<OpaqueObjectStorage> storage, const wchar_t* description) {
  if (!storage || !g_FontListDtor) return;

  if (!IsOpaqueObjectGuardIntact(*storage)) {
    Wh_Log(L"%ls storage guard was overwritten; skipping destructor", description);
    return;
  }

  g_FontListDtor(reinterpret_cast<FontListOpaque*>(storage->data));
}

static std::unique_ptr<OpaqueObjectStorage> DeriveFontListToOwnedStorage(
    const FontListOpaque* originalFont,
    int targetSize,
    const wchar_t* description) {
  if (!originalFont || !g_FontListGetFontSize || !g_FontListDeriveWithSizeDelta ||
      !g_FontListDtor) {
    return nullptr;
  }

  int originalSize = g_FontListGetFontSize(originalFont);

  if (targetSize == originalSize) return nullptr;

  auto storage = std::make_unique<OpaqueObjectStorage>();
  PrepareOpaqueObjectStorage(*storage);

  auto* derivedFont = reinterpret_cast<FontListOpaque*>(storage->data);
  g_FontListDeriveWithSizeDelta(originalFont, derivedFont, targetSize - originalSize);

  if (!IsOpaqueObjectGuardIntact(*storage)) {
    Wh_Log(L"%ls derived FontList storage guard was overwritten; skipping destructor",
           description);
    return nullptr;
  }

  return storage;
}

static void ReleaseBookmarkCtorDerivedFonts() {
  for (auto& storage : g_bookmarkCtorDerivedFonts) {
    DestroyOwnedFontListStorage(std::move(storage), L"Bookmark constructor FontList");
  }

  g_bookmarkCtorDerivedFonts.clear();
}

static bool SetLabelFontForTargetSize(void* label,
                                      const FontListOpaque* originalFont,
                                      int targetSize,
                                      const wchar_t* description) {
  if (!label || !originalFont || !g_LabelSetFontList) return false;

  // "Chrome default" restores the exact FontList that Chrome originally chose
  // for this object. Numeric settings are absolute target sizes.
  if (targetSize < 0) {
    g_LabelSetFontList(label, originalFont);
    return true;
  }

  if (!g_FontListGetFontSize || !g_FontListDeriveWithSizeDelta || !g_FontListDtor) {
    return false;
  }

  int originalSize = g_FontListGetFontSize(originalFont);

  if (targetSize == originalSize) {
    g_LabelSetFontList(label, originalFont);
    return true;
  }

  auto derivedFontStorage =
      DeriveFontListToOwnedStorage(originalFont, targetSize, description);

  if (!derivedFontStorage) return false;

  g_LabelSetFontList(label, GetOwnedFontList(derivedFontStorage));
  DestroyOwnedFontListStorage(std::move(derivedFontStorage), description);
  return true;
}

// -----------------------------------------------------------------------------
// Address bar
// -----------------------------------------------------------------------------

static const FontListOpaque* GetOmniboxOriginalFont(const OmniboxInfo& info) {
  return GetOwnedFontList(info.originalFontStorage);
}

static bool ApplyAddressBarFont(void* textfield, const FontListOpaque* originalFont) {
  if (!textfield || !originalFont || !g_TextfieldSetFontListOriginal) return false;

  int targetSize = g_addressBarFontSize.load(std::memory_order_relaxed);

  if (targetSize < 0) {
    g_TextfieldSetFontListOriginal(textfield, originalFont);
    return true;
  }

  if (!g_FontListGetFontSize || !g_FontListDeriveWithSizeDelta || !g_FontListDtor) return false;

  int originalSize = g_FontListGetFontSize(originalFont);

  if (targetSize == originalSize) {
    g_TextfieldSetFontListOriginal(textfield, originalFont);
    return true;
  }

  auto derivedFontStorage =
      DeriveFontListToOwnedStorage(originalFont, targetSize, L"Address bar");

  if (!derivedFontStorage) return false;

  g_TextfieldSetFontListOriginal(textfield, GetOwnedFontList(derivedFontStorage));
  DestroyOwnedFontListStorage(std::move(derivedFontStorage), L"Address bar");
  return true;
}

// OmniboxViewViews reaches views::Textfield through a multiple-inheritance
// mixin, so the complete OmniboxViewViews pointer isn't necessarily a valid
// Textfield pointer. Capture Chromium's exact Textfield subobject pointer while
// the OmniboxViewViews constructor is constructing its bases. All unrelated
// Textfield constructors pass through unchanged.
static void TextfieldCtorHook(void* self) {
  g_TextfieldCtorOriginal(self);

  if (g_omniboxCtorDepth > 0 && !g_pendingOmniboxTextfield) {
    g_pendingOmniboxTextfield = self;
  }
}

static void OmniboxViewViewsCtorHook(void* self,
                                     bool popupWindowMode,
                                     void* controller,
                                     void* locationBarView,
                                     const FontListOpaque* fontList) {
  void* previousPendingTextfield = g_pendingOmniboxTextfield;
  g_pendingOmniboxTextfield = nullptr;
  g_omniboxCtorDepth++;

  int originalSize = fontList && g_FontListGetFontSize ? g_FontListGetFontSize(fontList) : -1;

  const FontListOpaque* constructorFont = fontList;
  std::unique_ptr<OpaqueObjectStorage> derivedFontStorage;

  int targetSize = g_addressBarFontSize.load(std::memory_order_relaxed);

  if (targetSize >= 0 && fontList && originalSize >= 0 && targetSize != originalSize) {
    derivedFontStorage = DeriveFontListToOwnedStorage(
        fontList, targetSize, L"Address bar constructor");

    if (derivedFontStorage) {
      constructorFont = GetOwnedFontList(derivedFontStorage);
    }
  }

  g_OmniboxViewViewsCtorOriginal(self, popupWindowMode, controller, locationBarView,
                                 constructorFont);

  g_omniboxCtorDepth--;

  DestroyOwnedFontListStorage(std::move(derivedFontStorage),
                              L"Address bar constructor");

  void* textfield = g_pendingOmniboxTextfield;
  g_pendingOmniboxTextfield = previousPendingTextfield;

  if (!textfield || !fontList) {
    Wh_Log(L"Address bar live tracking unavailable for this omnibox");
    return;
  }

  auto originalFontStorage =
      CopyFontListToOwnedStorage(fontList, L"Address bar original FontList");

  if (!originalFontStorage) {
    Wh_Log(L"Address bar original font capture failed; live restore unavailable for this omnibox");
    return;
  }

  std::lock_guard<std::mutex> lock(g_omniboxesMutex);
  g_omniboxes[self] = {GetCurrentThreadId(), textfield, std::move(originalFontStorage)};
}

static void* OmniboxViewViewsDtorHook(void* self, unsigned int flags) {
  std::unique_ptr<OpaqueObjectStorage> originalFontStorage;

  {
    std::lock_guard<std::mutex> lock(g_omniboxesMutex);
    auto it = g_omniboxes.find(self);

    if (it != g_omniboxes.end()) {
      originalFontStorage = std::move(it->second.originalFontStorage);
      g_omniboxes.erase(it);
    }
  }

  DestroyOwnedFontListStorage(std::move(originalFontStorage),
                              L"Address bar original FontList");

  return g_OmniboxViewViewsDtorOriginal(self, flags);
}

// -----------------------------------------------------------------------------
// Bookmark bar
// -----------------------------------------------------------------------------

static void BookmarkButtonBaseCtorHook(void* self, void* pressedCallback, const void* title) {
  bool outermostBookmarkCtor = g_bookmarkCtorDepth == 0 && g_bookmarkMenuCtorDepth == 0;
  g_bookmarkCtorDepth++;

  g_BookmarkButtonBaseCtorOriginal(self, pressedCallback, title);
  {
    std::lock_guard<std::mutex> lock(g_bookmarkButtonsMutex);
    g_bookmarkButtons[self] = GetCurrentThreadId();
  }

  g_bookmarkCtorDepth--;

  if (outermostBookmarkCtor) ReleaseBookmarkCtorDerivedFonts();
}

static void BookmarkMenuButtonBaseCtorHook(void* self, void* pressedCallback, void* showMenuCallback,
                                           const void* title) {
  bool outermostBookmarkCtor = g_bookmarkCtorDepth == 0 && g_bookmarkMenuCtorDepth == 0;
  g_bookmarkMenuCtorDepth++;

  g_BookmarkMenuButtonBaseCtorOriginal(self, pressedCallback, showMenuCallback, title);

  g_bookmarkMenuCtorDepth--;

  if (outermostBookmarkCtor) ReleaseBookmarkCtorDerivedFonts();
}

static void LabelButtonLabelCtorHook(void* self, const void* text, int textContext) {
  bool isBookmarkLabel = g_bookmarkCtorDepth > 0 || g_bookmarkMenuCtorDepth > 0;

  const FontListOpaque* previousPendingFont = g_pendingBookmarkOriginalFont;
  g_pendingBookmarkOriginalFont = nullptr;

  g_LabelButtonLabelCtorOriginal(self, text, textContext);

  const FontListOpaque* originalFont = g_pendingBookmarkOriginalFont;
  g_pendingBookmarkOriginalFont = previousPendingFont;

  if (!isBookmarkLabel) return;

  auto originalFontStorage =
      CopyFontListToOwnedStorage(originalFont, L"Bookmark original FontList");

  if (!originalFontStorage) {
    Wh_Log(L"Bookmark original font capture failed; live restore unavailable for this label");

    if (originalFont && g_LabelSetFontList) {
      g_LabelSetFontList(self, originalFont);
    }

    return;
  }

  const FontListOpaque* ownedOriginalFont = GetOwnedFontList(originalFontStorage);
  int targetSize = g_bookmarkFontSize.load(std::memory_order_relaxed);

  if (targetSize >= 0 &&
      !SetLabelFontForTargetSize(self, ownedOriginalFont, targetSize, L"Bookmark")) {
    Wh_Log(L"Bookmark exact font sizing unavailable; leaving Chrome's original font");
  }

  std::lock_guard<std::mutex> lock(g_labelsMutex);
  g_labels.insert_or_assign(
      self, BookmarkLabelInfo{GetCurrentThreadId(), std::move(originalFontStorage)});
}

static void* LabelButtonLabelDeletingDtorHook(void* self, unsigned int flags) {
  std::unique_ptr<OpaqueObjectStorage> originalFontStorage;

  {
    std::lock_guard<std::mutex> lock(g_labelsMutex);
    auto it = g_labels.find(self);

    if (it != g_labels.end()) {
      originalFontStorage = std::move(it->second.originalFontStorage);
      g_labels.erase(it);
    }
  }

  DestroyOwnedFontListStorage(std::move(originalFontStorage),
                              L"Bookmark original FontList");

  return g_LabelButtonLabelDeletingDtorOriginal(self, flags);
}

static const FontListOpaque* TypographyGetFontHook(const void* self, int context, int style) {
  const FontListOpaque* originalFont = g_TypographyGetFontOriginal(self, context, style);

  if (g_tabTitleCtorDepth > 0 && !g_pendingTabTitleOriginalFont) {
    g_pendingTabTitleOriginalFont = originalFont;
  }

  if (g_bookmarkCtorDepth <= 0 && g_bookmarkMenuCtorDepth <= 0) {
    return originalFont;
  }

  if (!g_pendingBookmarkOriginalFont) {
    g_pendingBookmarkOriginalFont = originalFont;
  }

  int targetSize = g_bookmarkFontSize.load(std::memory_order_relaxed);

  if (targetSize < 0 || !g_FontListCopyCtor ||
      !g_FontListGetFontSize || !g_FontListDeriveWithSizeDelta || !g_FontListDtor ||
      targetSize == g_FontListGetFontSize(originalFont)) {
    return originalFont;
  }

  auto derivedFontStorage = DeriveFontListToOwnedStorage(
      originalFont, targetSize, L"Bookmark constructor");

  if (!derivedFontStorage) return originalFont;

  const FontListOpaque* derivedFont = GetOwnedFontList(derivedFontStorage);
  g_bookmarkCtorDerivedFonts.push_back(std::move(derivedFontStorage));
  return derivedFont;
}

// -----------------------------------------------------------------------------
// Bookmark folder icon
// -----------------------------------------------------------------------------

static void CreateWindowsFolderImage() {
  std::lock_guard<std::mutex> lock(g_windowsFolderImageMutex);
  if (g_windowsFolderReady.load(std::memory_order_acquire)) return;

  if (!g_CreateSkBitmapFromHICON || !g_ImageSkiaCreateFrom1xBitmap || !g_SkBitmapDtor || !g_ImageSkiaDtor ||
      !g_ImageModelFromImageSkia) {
    Wh_Log(L"Windows bookmark folder image helpers unavailable");
    return;
  }

  SHSTOCKICONINFO iconInfo = {};
  iconInfo.cbSize = sizeof(iconInfo);

  HRESULT hr = SHGetStockIconInfo(SIID_FOLDER, SHGSI_ICON | SHGSI_SMALLICON, &iconInfo);

  if (FAILED(hr) || !iconInfo.hIcon) {
    Wh_Log(L"SHGetStockIconInfo(SIID_FOLDER) failed: 0x%08X", static_cast<unsigned int>(hr));
    return;
  }

  // SHGSI_SMALLICON follows Windows DPI (e.g. 28 px at 175%).
  // CreateFrom1xBitmap interprets pixels as DIP, so normalize its source
  // to the native 16-DIP favorites icon size to avoid scaling twice.
  HICON normalizedIcon = static_cast<HICON>(CopyImage(iconInfo.hIcon, IMAGE_ICON,
                                                     16, 16, 0));
  if (!normalizedIcon) {
    DestroyIcon(iconInfo.hIcon);
    Wh_Log(L"Could not normalize Windows folder icon size");
    return;
  }
  PrepareOpaqueObjectStorage(g_skBitmapStorage);
  g_CreateSkBitmapFromHICON(g_skBitmapStorage.data, normalizedIcon);
  DestroyIcon(normalizedIcon);
  DestroyIcon(iconInfo.hIcon);

  if (!IsOpaqueObjectGuardIntact(g_skBitmapStorage)) {
    Wh_Log(L"ERROR: SkBitmap exceeded reserved opaque storage; skipping destructor");
    return;
  }

  PrepareOpaqueObjectStorage(g_imageSkiaStorage);
  g_ImageSkiaCreateFrom1xBitmap(g_imageSkiaStorage.data, g_skBitmapStorage.data);
  g_SkBitmapDtor(g_skBitmapStorage.data);

  if (!IsOpaqueObjectGuardIntact(g_imageSkiaStorage)) {
    Wh_Log(L"ERROR: gfx::ImageSkia exceeded reserved opaque storage");
    return;
  }

  g_windowsFolderThreadId = GetCurrentThreadId();
  g_windowsFolderReady.store(true, std::memory_order_release);

  Wh_Log(L"Windows bookmark folder image: ready on UI thread %lu", g_windowsFolderThreadId);
}

static void DestroyWindowsFolderImageOnCurrentThread() {
  std::lock_guard<std::mutex> lock(g_windowsFolderImageMutex);

  if (!g_windowsFolderReady.load(std::memory_order_acquire)) return;

  DWORD threadId = GetCurrentThreadId();

  if (g_windowsFolderThreadId != threadId) {
    Wh_Log(L"Windows bookmark folder image cleanup skipped on non-owning thread %lu (owner %lu)",
           threadId, g_windowsFolderThreadId);
    return;
  }

  g_windowsFolderReady.store(false, std::memory_order_release);
  g_ImageSkiaDtor(g_imageSkiaStorage.data);
  g_windowsFolderThreadId = 0;

  Wh_Log(L"Windows bookmark folder image: destroyed on UI thread %lu", threadId);
}

static ImageModelOpaque* GetBookmarkFolderIconHook(ImageModelOpaque* result, int iconType,
                                                   uintptr_t colorVariantOpaque) {
  constexpr int kNormal = 0;

  if (HasFolderGlyph() || iconType != kNormal || !g_useWindowsFolderIcon.load(std::memory_order_relaxed)) {
    return g_GetBookmarkFolderIconOriginal(result, iconType, colorVariantOpaque);
  }

  CreateWindowsFolderImage();

  std::lock_guard<std::mutex> lock(g_windowsFolderImageMutex);

  if (!g_useWindowsFolderIcon.load(std::memory_order_relaxed) ||
      !g_windowsFolderReady.load(std::memory_order_acquire)) {
    return g_GetBookmarkFolderIconOriginal(result, iconType, colorVariantOpaque);
  }

  return g_ImageModelFromImageSkia(result, g_imageSkiaStorage.data);
}

static void BookmarkBarViewUpdateAppearanceForThemeHook(void* self) {
  g_BookmarkBarViewUpdateAppearanceForThemeOriginal(self);

  if (!g_bookmarkFolderLiveUpdateReady.load(std::memory_order_relaxed)) return;

  std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);
  g_bookmarkBars[self] = GetCurrentThreadId();
}

static void* BookmarkBarViewDeletingDtorHook(void* self, unsigned int flags) {
  {
    std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);
    g_bookmarkBars.erase(self);
  }

  return g_BookmarkBarViewDeletingDtorOriginal(self, flags);
}

// -----------------------------------------------------------------------------
// Menus
// -----------------------------------------------------------------------------

static FontListOpaque* MenuItemGetFontListHook(const void* self, FontListOpaque* result) {
  int targetSize = g_menuFontSize.load(std::memory_order_relaxed);

  FontListOpaque* originalResult = g_MenuItemGetFontListOriginal(self, result);

  if (targetSize < 0 || !originalResult ||
      !g_FontListCopyCtor || !g_FontListGetFontSize ||
      !g_FontListDeriveWithSizeDelta || !g_FontListDtor) {
    return originalResult;
  }

  int originalSize = g_FontListGetFontSize(originalResult);

  if (targetSize == originalSize) return originalResult;

  auto derivedFontStorage =
      DeriveFontListToOwnedStorage(originalResult, targetSize, L"Menu");

  if (!derivedFontStorage) return originalResult;

  const FontListOpaque* derivedFont = GetOwnedFontList(derivedFontStorage);

  // The original sret object is already constructed in |result|. Replace it
  // with a copy of the derived font only after destroying that original object.
  g_FontListDtor(originalResult);
  g_FontListCopyCtor(result, derivedFont);
  DestroyOwnedFontListStorage(std::move(derivedFontStorage), L"Menu");

  return result;
}

static void TabTitleCtorHook(void* self) {
  const FontListOpaque* previousPendingFont = g_pendingTabTitleOriginalFont;
  g_pendingTabTitleOriginalFont = nullptr;
  g_tabTitleCtorDepth++;

  g_TabTitleCtorOriginal(self);

  g_tabTitleCtorDepth--;

  const FontListOpaque* originalFont = g_pendingTabTitleOriginalFont;
  g_pendingTabTitleOriginalFont = previousPendingFont;

  if (!g_tabFontHooksReady.load(std::memory_order_relaxed)) {
    return;
  }

  auto originalFontStorage =
      CopyFontListToOwnedStorage(originalFont, L"Tab title original FontList");

  if (!originalFontStorage) {
    Wh_Log(L"Tab title original font capture failed; leaving this title unchanged");
    return;
  }

  const FontListOpaque* ownedOriginalFont = GetOwnedFontList(originalFontStorage);
  int targetSize = g_tabFontSize.load(std::memory_order_relaxed);

  if (targetSize >= 0 &&
      !SetLabelFontForTargetSize(self, ownedOriginalFont, targetSize, L"Tab title")) {
    Wh_Log(L"Tab title exact font sizing unavailable; leaving Chrome's original font");
  }

  {
    std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
    g_tabTitles.insert_or_assign(
        self, TabTitleInfo{GetCurrentThreadId(), std::move(originalFontStorage)});
  }
}

static void* TabTitleDtorHook(void* self, unsigned int flags) {
  std::unique_ptr<OpaqueObjectStorage> originalFontStorage;

  {
    std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
    auto it = g_tabTitles.find(self);

    if (it != g_tabTitles.end()) {
      originalFontStorage = std::move(it->second.originalFontStorage);
      g_tabTitles.erase(it);
    }
  }

  DestroyOwnedFontListStorage(std::move(originalFontStorage),
                              L"Tab title original FontList");

  return g_TabTitleDtorOriginal(self, flags);
}

static void TabCloseButtonCtorHook(void* self, void* pressedCallback, void* mouseEventCallback) {
  g_TabCloseButtonCtorOriginal(self, pressedCallback, mouseEventCallback);

  if (!g_tabCloseHooksReady.load(std::memory_order_relaxed)) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_tabObjectsMutex);

  g_tabCloseButtons[self] = GetCurrentThreadId();
}

static void TabCloseButtonDtorHook(void* self) {
  {
    std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
    g_tabCloseButtons.erase(self);
  }

  g_TabCloseButtonDtorOriginal(self);
}

// Ordinals verified against Edge's Tab::Layout in the pinned build.
static bool ValidateTabLayoutConstants() { return true; }

static int GetLayoutConstantHook(int constant) {
  int originalValue = g_GetLayoutConstantOriginal(constant);

  if (constant == kLayoutTabPreTitlePadding) {
    int configuredPadding = g_tabPreTitlePadding.load(std::memory_order_relaxed);

    if (configuredPadding != kChromeDefaultTabPreTitlePadding && ValidateTabLayoutConstants()) {
      return configuredPadding;
    }

    return originalValue;
  }

  if (!g_tabCloseHooksReady.load(std::memory_order_relaxed) ||
      !g_tabCloseButtonsHidden.load(std::memory_order_relaxed)) {
    return originalValue;
  }

  if (constant != kLayoutTabAfterTitlePadding && constant != kLayoutTabCloseButtonSize) {
    return originalValue;
  }

  if (!ValidateTabLayoutConstants()) return originalValue;

  return 0;
}

static void ViewSetVisibleHook(void* self, bool visible) {
  if (visible && g_tabCloseHooksReady.load(std::memory_order_relaxed) &&
      IsCloseButtonSuppressed(self) && ValidateTabLayoutConstants()) {
    bool isTabCloseButton = false;

    {
      std::lock_guard<std::mutex> lock(g_tabObjectsMutex);

      isTabCloseButton = g_tabCloseButtons.find(self) != g_tabCloseButtons.end();
    }

    if (isTabCloseButton) visible = false;
  }

  if (ShouldHideToolbarButton(self)) { RememberToolbarVisibility(self, visible); visible = false; }
  g_ViewSetVisibleOriginal(self, visible);
}

// -----------------------------------------------------------------------------
// Extension toolbar
// -----------------------------------------------------------------------------

static void ToolbarActionViewCtorHook(void* self, void* viewModel, void* delegate) {
  g_ToolbarActionViewCtorOriginal(self, viewModel, delegate);

  if (!g_extensionTrackingReady.load(std::memory_order_relaxed)) return;

  std::lock_guard<std::mutex> lock(g_extensionViewsMutex);

  g_extensionViews[self] = GetCurrentThreadId();
}

static GfxSizeOpaque* ToolbarActionViewCalculatePreferredSizeHook(const void* self, GfxSizeOpaque* result,
                                                                  const void* availableSize) {
  GfxSizeOpaque* returned = g_ToolbarActionViewCalculatePreferredSizeOriginal(self, result, availableSize);

  if (!result) return returned;

  const int configured = g_extensionButtonWidth.load();
  if (configured >= 0) result->width = configured;

  return returned;
}

static void* ToolbarActionViewDeletingDtorHook(void* self, unsigned int flags) {
  {
    std::lock_guard<std::mutex> lock(g_extensionViewsMutex);

    g_extensionViews.erase(self);
  }

  return g_ToolbarActionViewDeletingDtorOriginal(self, flags);
}

// Track ExtensionsToolbarDesktop itself so that a settings update can
// propagate the new preferred width to ToolbarView.

static void ExtensionsToolbarDesktopCtorHook(void* self, void* browser, int displayMode) {
  g_ExtensionsToolbarDesktopCtorOriginal(self, browser, displayMode);

  if (!g_extensionContainerTrackingReady.load(std::memory_order_relaxed)) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_extensionViewsMutex);

  g_extensionContainers[self] = GetCurrentThreadId();
}

static void* ExtensionsToolbarDesktopDeletingDtorHook(void* self, unsigned int flags) {
  {
    std::lock_guard<std::mutex> lock(g_extensionViewsMutex);

    g_extensionContainers.erase(self);
  }

  return g_ExtensionsToolbarDesktopDeletingDtorOriginal(self, flags);
}

// -----------------------------------------------------------------------------
// Run code on Chrome UI thread
// -----------------------------------------------------------------------------

using RunFromWindowThreadProc = void(WINAPI*)(void* parameter);

static UINT GetRunFromWindowThreadMessage() {
  static const UINT message = RegisterWindowMessageW(L"Windhawk_EdgeUiTweaks_RunFromWindowThread");

  return message;
}

static bool RunFromWindowThread(HWND hwnd, RunFromWindowThreadProc proc, void* parameter) {
  DWORD threadId = GetWindowThreadProcessId(hwnd, nullptr);

  if (!threadId) return false;

  if (threadId == GetCurrentThreadId()) {
    proc(parameter);
    return true;
  }

  struct Param {
    RunFromWindowThreadProc proc;
    void* parameter;
  };

  HHOOK hook = SetWindowsHookExW(
      WH_CALLWNDPROC,
      [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (code == HC_ACTION) {
          const CWPSTRUCT* cwp = reinterpret_cast<const CWPSTRUCT*>(lParam);

          if (cwp->message == GetRunFromWindowThreadMessage()) {
            Param* param = reinterpret_cast<Param*>(cwp->lParam);

            param->proc(param->parameter);
          }
        }

        return CallNextHookEx(nullptr, code, wParam, lParam);
      },
      nullptr, threadId);

  if (!hook) return false;

  Param param{proc, parameter};

  SendMessageW(hwnd, GetRunFromWindowThreadMessage(), 0, reinterpret_cast<LPARAM>(&param));

  UnhookWindowsHookEx(hook);

  return true;
}

// -----------------------------------------------------------------------------
// Find Chrome window for UI thread
// -----------------------------------------------------------------------------

static HWND FindWindowForThread(DWORD threadId) {
  struct Context {
    HWND first = nullptr;
    HWND chrome = nullptr;
  } context;

  EnumThreadWindows(
      threadId,
      [](HWND hwnd, LPARAM lParam) -> BOOL {
        Context* context = reinterpret_cast<Context*>(lParam);

        if (!context->first) context->first = hwnd;

        wchar_t className[128] = {};

        if (GetClassNameW(hwnd, className, ARRAYSIZE(className))) {
          if (wcsncmp(className, kChromeWidgetWindowClassPrefix,
                      ARRAYSIZE(kChromeWidgetWindowClassPrefix) - 1) == 0) {
            context->chrome = hwnd;
            return FALSE;
          }
        }

        return TRUE;
      },
      reinterpret_cast<LPARAM>(&context));

  return context.chrome ? context.chrome : context.first;
}

static void WINAPI DestroyWindowsFolderImageOnCurrentThreadProc(void*) {
  DestroyWindowsFolderImageOnCurrentThread();
}

static void DestroyWindowsFolderImageOnOwningThread() {
  DWORD threadId = 0;

  {
    std::lock_guard<std::mutex> lock(g_windowsFolderImageMutex);

    if (!g_windowsFolderReady.load(std::memory_order_acquire)) return;

    threadId = g_windowsFolderThreadId;
  }

  if (!threadId) return;

  HWND hwnd = FindWindowForThread(threadId);

  if (!hwnd) {
    Wh_Log(L"Windows bookmark folder image cleanup: no window found for owning UI thread %lu; leaving it alive",
           threadId);
    return;
  }

  if (!RunFromWindowThread(hwnd, DestroyWindowsFolderImageOnCurrentThreadProc, nullptr)) {
    Wh_Log(L"Windows bookmark folder image cleanup: failed to dispatch to owning UI thread %lu; leaving it alive",
           threadId);
  }
}

// -----------------------------------------------------------------------------
// Live address-bar update
// -----------------------------------------------------------------------------

struct AddressBarApplyParams {
  bool teardown;
};

static void WINAPI ApplyAddressBarFontOnCurrentThread(void* parameter) {
  const auto* params = static_cast<const AddressBarApplyParams*>(parameter);
  bool teardown = params && params->teardown;

  if (!teardown && !g_addressBarFontHooksReady.load(std::memory_order_relaxed)) return;

  DWORD threadId = GetCurrentThreadId();

  if (!teardown) {
    std::vector<std::pair<void*, const FontListOpaque*>> omniboxes;

    {
      std::lock_guard<std::mutex> lock(g_omniboxesMutex);

      for (const auto& [omnibox, info] : g_omniboxes) {
        if (info.threadId == threadId) {
          omniboxes.push_back({info.textfield, GetOmniboxOriginalFont(info)});
        }
      }
    }

    for (const auto& [textfield, originalFont] : omniboxes) {
      ApplyAddressBarFont(textfield, originalFont);
    }

    return;
  }

  struct TeardownEntry {
    void* textfield;
    std::unique_ptr<OpaqueObjectStorage> originalFontStorage;
  };

  std::vector<TeardownEntry> omniboxes;

  {
    std::lock_guard<std::mutex> lock(g_omniboxesMutex);

    for (auto it = g_omniboxes.begin(); it != g_omniboxes.end();) {
      if (it->second.threadId == threadId) {
        omniboxes.push_back({it->second.textfield, std::move(it->second.originalFontStorage)});
        it = g_omniboxes.erase(it);
      } else {
        ++it;
      }
    }
  }

  for (auto& entry : omniboxes) {
    const FontListOpaque* originalFont = GetOwnedFontList(entry.originalFontStorage);

    if (originalFont) {
      ApplyAddressBarFont(entry.textfield, originalFont);
    }

    DestroyOwnedFontListStorage(std::move(entry.originalFontStorage),
                                L"Address bar original FontList");
  }

  Wh_Log(L"Restored and released %llu address bar fonts on UI thread %lu",
         static_cast<unsigned long long>(omniboxes.size()), threadId);
}

static void ApplyFontToExistingAddressBars(bool teardown = false) {
  if (!teardown && !g_addressBarFontHooksReady.load(std::memory_order_relaxed)) return;

  std::vector<DWORD> threadIds;

  {
    std::lock_guard<std::mutex> lock(g_omniboxesMutex);

    for (const auto& [omnibox, info] : g_omniboxes) {
      threadIds.push_back(info.threadId);
    }
  }

  std::sort(threadIds.begin(), threadIds.end());
  threadIds.erase(std::unique(threadIds.begin(), threadIds.end()), threadIds.end());

  AddressBarApplyParams params{teardown};

  for (DWORD threadId : threadIds) {
    HWND hwnd = FindWindowForThread(threadId);

    if (!hwnd) {
      Wh_Log(L"Address bar %ls: no window found for UI thread %lu",
             teardown ? L"teardown" : L"live update", threadId);
      continue;
    }

    if (!RunFromWindowThread(hwnd, ApplyAddressBarFontOnCurrentThread, &params)) {
      Wh_Log(L"Address bar %ls: failed to dispatch to UI thread %lu",
             teardown ? L"teardown" : L"live update", threadId);
    }
  }
}

// -----------------------------------------------------------------------------
// Live bookmark update
// -----------------------------------------------------------------------------

struct BookmarkFontApplyParams {
  bool teardown;
};

static void WINAPI ApplyBookmarkFontOnCurrentThread(void* parameter) {
  if (!g_LabelSetFontList) return;

  const auto* params = static_cast<const BookmarkFontApplyParams*>(parameter);
  bool teardown = params && params->teardown;
  DWORD threadId = GetCurrentThreadId();

  if (!teardown) {
    int targetSize = g_bookmarkFontSize.load(std::memory_order_relaxed);
    std::vector<std::pair<void*, const FontListOpaque*>> labels;

    {
      std::lock_guard<std::mutex> lock(g_labelsMutex);

      for (const auto& [label, info] : g_labels) {
        if (info.threadId == threadId) {
          labels.push_back({label, GetOwnedFontList(info.originalFontStorage)});
        }
      }
    }

    for (const auto& [label, originalFont] : labels) {
      SetLabelFontForTargetSize(label, originalFont, targetSize, L"Bookmark");
    }

    return;
  }

  struct TeardownEntry {
    void* label;
    std::unique_ptr<OpaqueObjectStorage> originalFontStorage;
  };

  std::vector<TeardownEntry> labels;

  {
    std::lock_guard<std::mutex> lock(g_labelsMutex);

    for (auto it = g_labels.begin(); it != g_labels.end();) {
      if (it->second.threadId == threadId) {
        labels.push_back({it->first, std::move(it->second.originalFontStorage)});
        it = g_labels.erase(it);
      } else {
        ++it;
      }
    }
  }

  for (auto& entry : labels) {
    const FontListOpaque* originalFont = GetOwnedFontList(entry.originalFontStorage);

    if (originalFont) {
      g_LabelSetFontList(entry.label, originalFont);
    }

    DestroyOwnedFontListStorage(std::move(entry.originalFontStorage),
                                L"Bookmark original FontList");
  }

  Wh_Log(L"Restored and released %llu bookmark fonts on UI thread %lu",
         static_cast<unsigned long long>(labels.size()), threadId);
}

static void ApplyFontToExistingBookmarkLabels(bool teardown = false) {
  std::vector<DWORD> threadIds;

  {
    std::lock_guard<std::mutex> lock(g_labelsMutex);

    for (const auto& [label, info] : g_labels) {
      threadIds.push_back(info.threadId);
    }
  }

  std::sort(threadIds.begin(), threadIds.end());

  threadIds.erase(std::unique(threadIds.begin(), threadIds.end()), threadIds.end());

  BookmarkFontApplyParams params{teardown};

  for (DWORD threadId : threadIds) {
    HWND hwnd = FindWindowForThread(threadId);

    if (!hwnd) {
      Wh_Log(L"Bookmark font %ls: no window found for UI thread %lu",
             teardown ? L"teardown" : L"live update", threadId);
      continue;
    }

    if (!RunFromWindowThread(hwnd, ApplyBookmarkFontOnCurrentThread, &params)) {
      Wh_Log(L"Bookmark font %ls: failed to dispatch to UI thread %lu",
             teardown ? L"teardown" : L"live update", threadId);
    }
  }
}

static void WINAPI ApplyBookmarkFolderIconOnCurrentThread(void*) {
  if (!g_BookmarkBarViewUpdateAppearanceForThemeOriginal) return;

  DWORD threadId = GetCurrentThreadId();
  std::vector<void*> bookmarkBars;

  {
    std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);

    for (const auto& [bookmarkBar, bookmarkBarThreadId] : g_bookmarkBars) {
      if (bookmarkBarThreadId == threadId) bookmarkBars.push_back(bookmarkBar);
    }
  }

  for (void* bookmarkBar : bookmarkBars) {
    g_BookmarkBarViewUpdateAppearanceForThemeOriginal(bookmarkBar);
  }

  Wh_Log(L"Updated %llu bookmark bars on UI thread %lu",
         static_cast<unsigned long long>(bookmarkBars.size()), threadId);
}

static void ApplyFolderIconToExistingBookmarkBars() {
  if (!g_bookmarkFolderLiveUpdateReady.load(std::memory_order_relaxed)) return;

  std::vector<DWORD> threadIds;

  {
    std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);

    for (const auto& [bookmarkBar, threadId] : g_bookmarkBars) {
      threadIds.push_back(threadId);
    }
  }

  std::sort(threadIds.begin(), threadIds.end());
  threadIds.erase(std::unique(threadIds.begin(), threadIds.end()), threadIds.end());

  for (DWORD threadId : threadIds) {
    HWND hwnd = FindWindowForThread(threadId);

    if (hwnd) {
      RunFromWindowThread(hwnd, ApplyBookmarkFolderIconOnCurrentThread, nullptr);
    }
  }
}

// -----------------------------------------------------------------------------
// Live tab update
// -----------------------------------------------------------------------------

struct TabApplyParams {
  bool teardown;
};

static void WINAPI ApplyTabTweaksOnCurrentThread(void* parameter) {
  const auto* params = static_cast<const TabApplyParams*>(parameter);
  bool teardown = params && params->teardown;
  DWORD threadId = GetCurrentThreadId();

  if (!teardown) {
    std::vector<std::pair<void*, const FontListOpaque*>> titles;
    std::vector<void*> closeButtons;

    {
      std::lock_guard<std::mutex> lock(g_tabObjectsMutex);

      for (const auto& [title, info] : g_tabTitles) {
        if (info.threadId == threadId) {
          titles.push_back({title, GetOwnedFontList(info.originalFontStorage)});
        }
      }

      for (const auto& [closeButton, closeButtonThreadId] : g_tabCloseButtons) {
        if (closeButtonThreadId == threadId) {
          closeButtons.push_back(closeButton);
        }
      }
    }

    int targetSize = g_tabFontSize.load(std::memory_order_relaxed);

    for (const auto& [title, originalFont] : titles) {
      if (g_tabFontHooksReady.load(std::memory_order_relaxed)) {
        SetLabelFontForTargetSize(title, originalFont, targetSize, L"Tab title");
      }

      if (g_ViewInvalidateLayout) {
        g_ViewInvalidateLayout(title, false);
      }
    }

    // A close-button-only symbol set still needs a live relayout path even if
    // TabTitle tracking isn't available. InvalidateLayout propagates to parents.
    if (g_ViewInvalidateLayout) {
      for (void* closeButton : closeButtons) {
        g_ViewInvalidateLayout(closeButton, false);
      }
    }

    return;
  }

  struct TeardownEntry {
    void* title;
    std::unique_ptr<OpaqueObjectStorage> originalFontStorage;
  };

  std::vector<TeardownEntry> titles;
  std::vector<void*> closeButtons;

  {
    std::lock_guard<std::mutex> lock(g_tabObjectsMutex);

    for (auto it = g_tabTitles.begin(); it != g_tabTitles.end();) {
      if (it->second.threadId == threadId) {
        titles.push_back({it->first, std::move(it->second.originalFontStorage)});
        it = g_tabTitles.erase(it);
      } else {
        ++it;
      }
    }

    for (auto it = g_tabCloseButtons.begin(); it != g_tabCloseButtons.end();) {
      if (it->second == threadId) {
        closeButtons.push_back(it->first);
        it = g_tabCloseButtons.erase(it);
      } else {
        ++it;
      }
    }
  }

  for (auto& entry : titles) {
    const FontListOpaque* originalFont = GetOwnedFontList(entry.originalFontStorage);

    // Teardown already knows these objects were tracked while the required
    // symbols were available. Don't depend on runtime ready flags that a
    // resolver/abandon path can clear independently.
    if (originalFont && g_LabelSetFontList) {
      g_LabelSetFontList(entry.title, originalFont);
    }

    if (g_ViewInvalidateLayout) {
      g_ViewInvalidateLayout(entry.title, false);
    }

    DestroyOwnedFontListStorage(std::move(entry.originalFontStorage),
                                L"Tab title original FontList");
  }

  if (g_ViewInvalidateLayout) {
    for (void* closeButton : closeButtons) {
      g_ViewInvalidateLayout(closeButton, false);
    }
  }

  Wh_Log(L"Restored/released %llu tab title fonts and invalidated %llu close buttons on UI thread %lu",
         static_cast<unsigned long long>(titles.size()),
         static_cast<unsigned long long>(closeButtons.size()), threadId);
}

static void ApplyTweaksToExistingTabs(bool teardown = false) {
  std::vector<DWORD> threadIds;

  {
    std::lock_guard<std::mutex> lock(g_tabObjectsMutex);

    for (const auto& [title, info] : g_tabTitles) {
      threadIds.push_back(info.threadId);
    }

    for (const auto& [closeButton, threadId] : g_tabCloseButtons) {
      threadIds.push_back(threadId);
    }
  }

  std::sort(threadIds.begin(), threadIds.end());

  threadIds.erase(std::unique(threadIds.begin(), threadIds.end()), threadIds.end());

  TabApplyParams params{teardown};

  for (DWORD threadId : threadIds) {
    HWND hwnd = FindWindowForThread(threadId);

    if (!hwnd) {
      Wh_Log(L"Tab %ls: no window found for UI thread %lu",
             teardown ? L"teardown" : L"live update", threadId);
      continue;
    }

    if (!RunFromWindowThread(hwnd, ApplyTabTweaksOnCurrentThread, &params)) {
      Wh_Log(L"Tab %ls: failed to dispatch to UI thread %lu",
             teardown ? L"teardown" : L"live update", threadId);
    }
  }
}

// -----------------------------------------------------------------------------
// Live extension update
// -----------------------------------------------------------------------------

static void WINAPI ApplyExtensionWidthOnCurrentThread(void*) {
  DWORD threadId = GetCurrentThreadId();

  std::vector<void*> views;
  std::vector<void*> containers;

  {
    std::lock_guard<std::mutex> lock(g_extensionViewsMutex);

    for (const auto& [view, viewThreadId] : g_extensionViews) {
      if (viewThreadId == threadId) {
        views.push_back(view);
      }
    }

    for (const auto& [container, containerThreadId] : g_extensionContainers) {
      if (containerThreadId == threadId) {
        containers.push_back(container);
      }
    }
  }

  // Rebuild icon/badge image if this symbol is available,
  // then tell each action view that its preferred size changed.
  for (void* view : views) {
    if (g_ToolbarActionViewUpdateState) {
      g_ToolbarActionViewUpdateState(view);
    }

    if (g_ViewPreferredSizeChanged) {
      g_ViewPreferredSizeChanged(view);
    }
  }

  // Propagate the container's new preferred width to ToolbarView.
  for (void* container : containers) {
    if (g_ViewPreferredSizeChanged) {
      g_ViewPreferredSizeChanged(container);
    }
  }

  Wh_Log(L"Updated %llu extension buttons and %llu containers on UI thread %lu",
         static_cast<unsigned long long>(views.size()), static_cast<unsigned long long>(containers.size()), threadId);
}

static void ApplyWidthToExistingExtensionButtons() {
  if (!g_extensionTrackingReady.load(std::memory_order_relaxed) &&
      !g_extensionContainerTrackingReady.load(std::memory_order_relaxed)) {
    return;
  }

  std::vector<DWORD> threadIds;

  {
    std::lock_guard<std::mutex> lock(g_extensionViewsMutex);

    for (const auto& [view, threadId] : g_extensionViews) {
      threadIds.push_back(threadId);
    }

    for (const auto& [container, threadId] : g_extensionContainers) {
      threadIds.push_back(threadId);
    }
  }

  std::sort(threadIds.begin(), threadIds.end());

  threadIds.erase(std::unique(threadIds.begin(), threadIds.end()), threadIds.end());

  for (DWORD threadId : threadIds) {
    HWND hwnd = FindWindowForThread(threadId);

    if (hwnd) {
      RunFromWindowThread(hwnd, ApplyExtensionWidthOnCurrentThread, nullptr);
    }
  }
}

// -----------------------------------------------------------------------------
// Resolve all chrome.dll symbols in ONE pass
// -----------------------------------------------------------------------------


// MenuConfig settings adapted from Vasher's Chrome UI Tweaks, with offsets
// independently verified against Edge 154.0.4258.48. No Chrome layout is reused.
struct MenuField {
    const wchar_t* key;
    size_t offset;
    int minimum;
    int maximum;
    bool byte;
    std::atomic<int> value{-1};
    int original = 0;
};
static MenuField g_menuFields[] = {
    {L"menus.items.item_vertical_margin", 0x24, 0, 64, false},
    {L"menus.items.between_item_vertical_padding", 0x84, 0, 64, false},
    {L"menus.items.item_horizontal_padding", 0x3c, 0, 64, false},
    {L"menus.items.item_horizontal_border_padding", 0x44, -64, 64, false},
    {L"menus.corners.item_corner_radius", 0xb4, 0, 64, false},
    // Edge's newer focus/selection outline has a separate radius field.
    {L"menus.corners.item_corner_radius", 0x11c, 0, 64, false},
    {L"menus.separators.separator_height", 0x64, 0, 128, false},
    {L"menus.separators.double_separator_height", 0x68, 0, 128, false},
    {L"menus.separators.separator_upper_height", 0x6c, 0, 128, false},
    {L"menus.separators.separator_lower_height", 0x70, 0, 128, false},
    {L"menus.separators.separator_spacing_height", 0x74, 0, 128, false},
    {L"menus.separators.separator_thickness", 0x78, 0, 32, false},
    {L"menus.separators.double_separator_thickness", 0x7c, 0, 32, false},
    {L"menus.separators.separator_horizontal_border_padding", 0x80, 0, 128, false},
    {L"menus.separators.padded_separator_start_padding", 0xd8, 0, 256, false},
    {L"menus.submenus.arrow_size", 0x4c, 0, 64, false},
    {L"menus.submenus.arrow_to_edge_padding", 0x50, 0, 64, false},
    {L"menus.submenus.submenu_horizontal_overlap", 0x20, -64, 64, false},
    {L"menus.submenus.show_delay", 0xb0, 0, 5000, false},
    {L"menus.submenus.scroll_arrow_height", 0x8c, 0, 64, false},
    {L"menus.shadows.bubble_menu_shadow_elevation", 0xc8, 0, 64, false},
    {L"menus.shadows.bubble_submenu_shadow_elevation", 0xcc, 0, 64, false},
    {L"menus.corners.use_bubble_border", 0x128, 0, 1, true},
};
static std::atomic<bool> g_hideMenuShadows{false};
static std::atomic<bool> g_hideFavoriteEditorOnAdd{false};
static std::mutex g_menuConfigMutex;
static void* g_menuConfig;
static DWORD g_menuThread;
static thread_local bool g_creatingSubmenuBorder;
static thread_local unsigned g_menuBorderDepth;
static thread_local unsigned g_menuHostDepth;
using ConfigFn = const void* (*)();
static ConfigFn g_configOriginal;
using UnaryFn = void (*)(void*);
static UnaryFn g_createBubbleBorderOriginal;
using MenuHostFn = void (*)(void*, const void*);
static MenuHostFn g_menuHostOriginal;
using WidgetInitFn = void (*)(void*, void*);
static WidgetInitFn g_widgetInitOriginal;
using BorderCtorFn = void (*)(void*, int, int);
static BorderCtorFn g_borderCtorOriginal;
using MenuItemFn = const void* (*)(const void*);
static MenuItemFn g_submenuGetItem;
using BookmarkFlowFn = void (*)(void*, const void*, bool);
static BookmarkFlowFn g_bookmarkFlowOriginal;

template<class T> static void WriteField(void* object, size_t offset, T value) {
    std::memcpy(static_cast<unsigned char*>(object) + offset, &value, sizeof(value));
}
static int ReadMenuField(const void* config, const MenuField& field) {
    return field.byte ? ReadField<unsigned char>(config, field.offset)
                      : ReadField<int>(config, field.offset);
}
static void WriteMenuField(void* config, const MenuField& field, int value) {
    if (field.byte) WriteField<unsigned char>(config, field.offset, value != 0);
    else WriteField<int>(config, field.offset, value);
}
// -1 restores Edge; the old mod's convention for other negative values remains:
// -2 means -1 DIP, -3 means -2 DIP, etc.
static int NormalizeMenuValue(int value, const MenuField& field) {
    if (value == -1) return -1;
    if (value < -1 && field.minimum < 0) ++value;
    return std::clamp(value, field.minimum, field.maximum);
}
static const void* MenuConfigHook() {
    void* config = const_cast<void*>(g_configOriginal());
    std::lock_guard<std::mutex> lock(g_menuConfigMutex);
    if (!g_menuConfig) {
        g_menuConfig = config;
        g_menuThread = GetCurrentThreadId();
        for (auto& field : g_menuFields) field.original = ReadMenuField(config, field);
    }
    // Only the verified process-wide singleton is changed, never other objects.
    if (config != g_menuConfig) return config;
    for (auto& field : g_menuFields) {
        const int setting = field.value.load();
        const int value = g_stopping.load() || setting == -1
                              ? field.original : NormalizeMenuValue(setting, field);
        WriteMenuField(config, field, value);
    }
    if (!g_stopping.load()) {
        if (g_creatingSubmenuBorder) {
            WriteField<int>(config, 0xc8, ReadField<int>(config, 0xcc));
        }
        if (g_hideMenuShadows.load()) {
            WriteField<int>(config, 0xc8, 0);
            WriteField<int>(config, 0xcc, 0);
        }
    }
    return config;
}
static bool HideMenuShadows() { return !g_stopping.load() && g_hideMenuShadows.load(); }
static void CreateBubbleBorderHook(void* self) {
    const bool previous = g_creatingSubmenuBorder;
    const void* item = g_submenuGetItem(ReadField<const void*>(self, 0x358));
    g_creatingSubmenuBorder = item && ReadField<const void*>(item, 0x360);
    ++g_menuBorderDepth;
    g_createBubbleBorderOriginal(self);
    --g_menuBorderDepth;
    g_creatingSubmenuBorder = previous;
    MenuConfigHook(); // Restore the main-menu elevation after the scoped override.
}
static void BubbleBorderCtorHook(void* self, int arrow, int shadow) {
    // NO_SHADOW=1 in this Windows build. Other browser bubbles are untouched.
    g_borderCtorOriginal(self, arrow, g_menuBorderDepth && HideMenuShadows() ? 1 : shadow);
}
static void MenuHostHook(void* self, const void* params) {
    ++g_menuHostDepth;
    g_menuHostOriginal(self, params);
    --g_menuHostDepth;
}
static void WidgetInitHook(void* self, void* params) {
    if (!g_isWebview && !g_stopping.load() && ReadField<int>(params, 0) == 5) {
        // TYPE_TOOLTIP: a rounded background requires a translucent widget.
        // Native TooltipAura otherwise leaves an opaque rectangular backing.
        WriteField<int>(params, 0x2c, 2);
    }
    if (g_menuHostDepth && HideMenuShadows() && ReadField<int>(params, 0) == 4) {
        // The parameter is passed by value in C++ (indirect on Win64), so this
        // changes only the callee's InitParams, not a retained caller object.
        WriteField<int>(params, 0x4c, 1); // ShadowType::kNone
        WriteField<int>(params, 0x50, 0); // explicit shadow elevation
        WriteField<unsigned char>(params, 0x54, 1); // optional is engaged
    }
    g_widgetInitOriginal(self, params);
}
static void BookmarkFlowHook(void* self, const void* url, bool alreadyBookmarked) {
    if (!alreadyBookmarked && g_hideFavoriteEditorOnAdd.load() && !g_stopping.load()) return;
    g_bookmarkFlowOriginal(self, url, alreadyBookmarked);
}
// Edge 154 native interface styling. All hooks in this section are browser-only;
// renderer processes and Windhawk's HTML interface never receive these hooks.
// Foreign std::string is a read-only libc++ alternate-layout view, not an
// owning C++ object. Its backing std::string stays alive for the entire call.
struct BorrowedEdgeString {
    const char* data;
    size_t length;
    size_t capacity;
    explicit BorrowedEdgeString(const std::string& value)
        : data(value.c_str()), length(value.size()),
          capacity((size_t{1} << 63) | (value.size() + 1)) {}
};
static_assert(sizeof(BorrowedEdgeString) == 24);
static std::string_view EdgeStringView(const void* object) {
    if (!object) return {};
    const auto tag = ReadField<unsigned char>(object, 23);
    if (!(tag & 0x80))
        return {static_cast<const char*>(object), std::min<size_t>(tag, 22)};
    const auto count = ReadField<size_t>(object, 8);
    const auto* data = ReadField<const char*>(object, 0);
    return data && count <= 4096 ? std::string_view(data, count) : std::string_view{};
}
static bool IsSymbolFont(std::string_view family) {
    std::string lower(family);
    for (auto& c : lower) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    for (const char* token : {"symbol", "emoji", "icons", "mdl2", "marlett", "wingdings", "webdings"})
        if (lower.find(token) != std::string::npos) return true;
    return false;
}
using MatchUiFontFn = void* (*)(void*, bool, int, const void*, bool*);
static MatchUiFontFn g_matchUiFontOriginal;
static void* MatchUiFontHook(void* result, bool italic, int weight, const void* family, bool* success) {
    std::string selected;
    if (!g_stopping.load() && !IsSymbolFont(EdgeStringView(family))) {
        std::lock_guard<std::mutex> lock(g_browserStyleMutex);
        selected = g_browserFont;
    }
    if (selected.empty()) return g_matchUiFontOriginal(result, italic, weight, family, success);
    const BorrowedEdgeString replacement(selected);
    return g_matchUiFontOriginal(result, italic, weight, &replacement, success);
}

static std::wstring ReadTextSetting(const wchar_t* key) {
    const wchar_t* value = Wh_GetStringSetting(key);
    std::wstring result = value ? value : L"";
    if (value) Wh_FreeStringSetting(value);
    return result;
}
static std::wstring ParseGlyph(std::wstring text) {
    if (text.size() > 2 && (text[0] == L'U' || text[0] == L'u') && text[1] == L'+') {
        if (text.size() > 8) return {};
        unsigned value = 0;
        for (size_t i = 2; i < text.size(); ++i) {
            wchar_t c = text[i];
            unsigned digit = c >= L'0' && c <= L'9' ? c - L'0' :
                             c >= L'A' && c <= L'F' ? c - L'A' + 10 :
                             c >= L'a' && c <= L'f' ? c - L'a' + 10 : 16;
            if (digit > 15) return {};
            value = value * 16 + digit;
        }
        if (value < 0x20 || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) return {};
        if (value <= 0xffff) return std::wstring(1, static_cast<wchar_t>(value));
        value -= 0x10000;
        return {static_cast<wchar_t>(0xd800 + (value >> 10)), static_cast<wchar_t>(0xdc00 + (value & 1023))};
    }
    if (text.size() == 1 && text[0] >= 0x20 && !(text[0] >= 0xd800 && text[0] <= 0xdfff)) return text;
    if (text.size() == 2 && text[0] >= 0xd800 && text[0] <= 0xdbff && text[1] >= 0xdc00 && text[1] <= 0xdfff) return text;
    return {};
}
static void LoadBrowserStyle() {
    g_commonTabColor = Wh_GetIntSetting(L"tabs.appearance.common_color") != 0;
    auto font = ReadTextSetting(L"interface.font_family");
    std::string utf8;
    if (!font.empty() && font.size() < LF_FACESIZE) {
        int count = WideCharToMultiByte(CP_UTF8, 0, font.data(), static_cast<int>(font.size()), nullptr, 0, nullptr, nullptr);
        utf8.resize(count);
        WideCharToMultiByte(CP_UTF8, 0, font.data(), static_cast<int>(font.size()), utf8.data(), count, nullptr, nullptr);
    }
    std::lock_guard<std::mutex> lock(g_browserStyleMutex);
    g_browserFont = std::move(utf8);
    g_glyphFont = ReadTextSetting(L"interface.glyphs.font_family");
    const wchar_t* keys[] = {L"folder", L"new_tab", L"right", L"left", L"down", L"up"};
    for (size_t i = 0; i < std::size(keys); ++i)
        g_glyphs[i] = ParseGlyph(ReadTextSetting((std::wstring(L"interface.glyphs.") + keys[i]).c_str()));
    g_showMenuIcons = Wh_GetIntSetting(L"menus.icons.show_icons") != 0;
    g_showShortcuts = Wh_GetIntSetting(L"menus.text.show_shortcuts") != 0;
    g_showFavoriteIcons = Wh_GetIntSetting(L"favorites.appearance.show_icons") != 0;
    g_centerFavorites = Wh_GetIntSetting(L"favorites.layout.center") != 0;
    g_centerTabs = Wh_GetIntSetting(L"tabs.layout.center") != 0;
}

using ObjectCtorFn = void* (*)(void*);
static ObjectCtorFn g_emptyImageModel, g_emptyImageSkia;
static OpaqueObjectDtorFn g_imageModelDtor, g_bookmarkButtonDtorOriginal;
using SetMenuIconFn = void (*)(void*, const void*);
static SetMenuIconFn g_setMenuIconOriginal;
static void SetMenuIconHook(void* self, const void* icon) {
    if (g_showMenuIcons.load() || g_stopping.load()) {
        g_setMenuIconOriginal(self, icon);
        return;
    }
    OpaqueObjectStorage empty{};
    PrepareOpaqueObjectStorage(empty);
    g_emptyImageModel(empty.data);
    if (IsOpaqueObjectGuardIntact(empty)) {
        g_setMenuIconOriginal(self, empty.data);
        g_imageModelDtor(empty.data);
    } else g_setMenuIconOriginal(self, icon);
}
using ShortcutTextFn = bool (*)(const void*, const void*, void*);
static ShortcutTextFn g_shortcutTextOriginal;
static bool ShortcutTextHook(const void* config, const void* item, void* text) {
    return (g_showShortcuts.load() || g_stopping.load()) && g_shortcutTextOriginal(config, item, text);
}
static void BookmarkButtonDtorHook(void* self) {
    { std::lock_guard<std::mutex> lock(g_bookmarkButtonsMutex); g_bookmarkButtons.erase(self); }
    g_bookmarkButtonDtorOriginal(self);
}
using ButtonImageFn = void* (*)(const void*, void*, int);
static ButtonImageFn g_buttonImageOriginal;
static bool ClassIs(const void* view, const char* name);
static bool IsIconFreeFavorite(const void* self) {
    if (!self || g_showFavoriteIcons.load() || g_stopping.load() || g_isWebview) return false;
    // Folder/menu buttons must remain recognizable when website icons are hidden.
    if (ClassIs(self, "BookmarkFolderButton") || ClassIs(self, "BookmarkMenuButtonBase")) return false;
    bool bookmark = ClassIs(self, "BookmarkButton");
    if (!bookmark) {
        std::lock_guard<std::mutex> lock(g_bookmarkButtonsMutex);
        bookmark = g_bookmarkButtons.contains(const_cast<void*>(self));
    }
    if (!bookmark) return false;
    auto* parent = ReadField<const void*>(self, 0x198);
    for (int depth = 0; parent && depth < 4; ++depth) {
        if (ClassIs(parent, "BookmarkBarView")) return true;
        { std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);
          if (g_bookmarkBars.contains(const_cast<void*>(parent))) return true; }
        // Do not cross into a different button/menu ownership tree.
        if (ClassIs(parent, "MenuItemView") || ClassIs(parent, "SubmenuView")) return false;
        parent = ReadField<const void*>(parent, 0x198);
    }
    return false;
}
static bool IsFavoriteImageHidden(const void* image) {
    for (int depth = 0; image && depth < 4; ++depth) {
        if (IsIconFreeFavorite(image)) return true;
        image = ReadField<const void*>(image, 0x198);
    }
    return false;
}
static void (*g_favoriteImagePaintOriginal)(void*, void*);
static void (*g_favoriteImageLayerPaintOriginal)(void*, void*);
static void FavoriteImagePaintHook(void* self, void* canvas) {
    if (!IsFavoriteImageHidden(self)) g_favoriteImagePaintOriginal(self, canvas);
}
static void FavoriteImageLayerPaintHook(void* self, void* context) {
    // EdgeImageViewCanvasExtender stores the owning ImageView at +0x10.
    if (!IsFavoriteImageHidden(ReadField<const void*>(self, 0x10)))
        g_favoriteImageLayerPaintOriginal(self, context);
}
static void* ButtonImageHook(const void* self, void* result, int state) {
    if (IsIconFreeFavorite(self)) { g_emptyImageSkia(result); return result; }
    return g_buttonImageOriginal(self, result, state);
}

struct GlyphAddress { DWORD rva; GlyphKind kind; };
static const GlyphAddress kGlyphAddresses48[] = {
    {0x1276f840, GlyphKind::new_tab}, // Edge SMTC add-tab variant
    {0x1276f888, GlyphKind::new_tab}, // Edge SMTC add-tab variant
    {0x12015280, GlyphKind::new_tab}, // kAddTabIcon, EdgeNewTabButton default
    {0x124abbd8, GlyphKind::down}, // kArrowDropDownIcon
    {0x12767248, GlyphKind::up}, // kCaretUpOldIcon
    {0x12012f80, GlyphKind::new_tab}, // kNewTabRefreshOldIcon
    {0x12768338, GlyphKind::down}, // kExpandMoreOldIcon
    {0x12767210, GlyphKind::down}, // kCaretDownOldIcon
    {0x124abc18, GlyphKind::down}, // kArrowDropDownOldIcon
    {0x1276c438, GlyphKind::folder}, // kFolderTouchIcon
    {0x127687d0, GlyphKind::folder}, // kFolderOpenOldIcon
    {0x12770398, GlyphKind::folder}, // kEdgeSmtcFolderIcon
    {0x1276c570, GlyphKind::folder}, // kOpenFolderIcon
    {0x12768790, GlyphKind::folder}, // kFolderOpenIcon
    {0x127703e0, GlyphKind::folder}, // kEdgeSmtcFolderFilledIcon
    {0x127686d8, GlyphKind::folder}, // kFolderFlippableIcon
    {0x12017a18, GlyphKind::folder}, // kFolderIcon
    {0x12768658, GlyphKind::folder}, // kFolderChromeRefreshOldIcon
    {0x12768758, GlyphKind::folder}, // kFolderOldIcon
    {0x1276c3b0, GlyphKind::folder}, // kFolderEdgeIcon
    {0x12768698, GlyphKind::folder}, // kFolderFilledIcon
    {0x12018b48, GlyphKind::new_tab}, // kNewTabInGroupIcon
    {0x12770a58, GlyphKind::new_tab}, // kEdgeSmtcNewTabInGroupIcon
    {0x12770a10, GlyphKind::new_tab}, // kEdgeSmtcNewTabIcon
    {0x124ac148, GlyphKind::new_tab}, // kNewTabOldIcon
    {0x12011380, GlyphKind::right}, // kChevronRightChromeRefreshOldIcon
    {0x12011330, GlyphKind::right}, // kChevronRightIcon
    {0x12016d80, GlyphKind::right}, // kEdgePageInfoSubmenuArrowIcon
    {0x1276b1e0, GlyphKind::right}, // kSubmenuArrowChromeRefreshOldIcon
    {0x12771440, GlyphKind::right}, // kEdgeSmtcSidepaneChevronRightIcon
    {0x1276b230, GlyphKind::right}, // kSubmenuArrowOldIcon
    {0x12019908, GlyphKind::right}, // kSidepaneChevronRightIcon
    {0x120198c0, GlyphKind::left}, // kSidepaneChevronLeftIcon
    {0x127713f0, GlyphKind::left}, // kEdgeSmtcSidepaneChevronLeftIcon
    {0x1276fb30, GlyphKind::down}, // kEdgeSmtcChevronDownIcon
    {0x1201ab10, GlyphKind::down}, // kOverflowChevronIcon
    {0x120161b0, GlyphKind::down}, // kChevronDownIcon
    {0x1276feb8, GlyphKind::down}, // kEdgeSmtcDropdownChevronIcon
};
static const GlyphAddress kGlyphAddresses53[] = {
    {0x1279fcd0, GlyphKind::new_tab}, // Edge SMTC add-tab variant
    {0x1279fd18, GlyphKind::new_tab}, // Edge SMTC add-tab variant
    {0x12044b60, GlyphKind::new_tab}, // kAddTabIcon, EdgeNewTabButton default
    {0x124dbde8, GlyphKind::down}, // kArrowDropDownIcon
    {0x127976d8, GlyphKind::up}, // kCaretUpOldIcon
    {0x12042860, GlyphKind::new_tab}, // kNewTabRefreshOldIcon
    {0x127987c8, GlyphKind::down}, // kExpandMoreOldIcon
    {0x127976a0, GlyphKind::down}, // kCaretDownOldIcon
    {0x124dbe28, GlyphKind::down}, // kArrowDropDownOldIcon
    {0x1279c8c8, GlyphKind::folder}, // kFolderTouchIcon
    {0x12798c60, GlyphKind::folder}, // kFolderOpenOldIcon
    {0x127a0828, GlyphKind::folder}, // kEdgeSmtcFolderIcon
    {0x1279ca00, GlyphKind::folder}, // kOpenFolderIcon
    {0x12798c20, GlyphKind::folder}, // kFolderOpenIcon
    {0x127a0870, GlyphKind::folder}, // kEdgeSmtcFolderFilledIcon
    {0x12798b68, GlyphKind::folder}, // kFolderFlippableIcon
    {0x120472f8, GlyphKind::folder}, // kFolderIcon
    {0x12798ae8, GlyphKind::folder}, // kFolderChromeRefreshOldIcon
    {0x12798be8, GlyphKind::folder}, // kFolderOldIcon
    {0x1279c840, GlyphKind::folder}, // kFolderEdgeIcon
    {0x12798b28, GlyphKind::folder}, // kFolderFilledIcon
    {0x12048428, GlyphKind::new_tab}, // kNewTabInGroupIcon
    {0x127a0ee8, GlyphKind::new_tab}, // kEdgeSmtcNewTabInGroupIcon
    {0x127a0ea0, GlyphKind::new_tab}, // kEdgeSmtcNewTabIcon
    {0x124dc358, GlyphKind::new_tab}, // kNewTabOldIcon
    {0x12040c60, GlyphKind::right}, // kChevronRightChromeRefreshOldIcon
    {0x12040c10, GlyphKind::right}, // kChevronRightIcon
    {0x12046660, GlyphKind::right}, // kEdgePageInfoSubmenuArrowIcon
    {0x1279b670, GlyphKind::right}, // kSubmenuArrowChromeRefreshOldIcon
    {0x127a18d0, GlyphKind::right}, // kEdgeSmtcSidepaneChevronRightIcon
    {0x1279b6c0, GlyphKind::right}, // kSubmenuArrowOldIcon
    {0x120491e8, GlyphKind::right}, // kSidepaneChevronRightIcon
    {0x120491a0, GlyphKind::left}, // kSidepaneChevronLeftIcon
    {0x127a1880, GlyphKind::left}, // kEdgeSmtcSidepaneChevronLeftIcon
    {0x1279ffc0, GlyphKind::down}, // kEdgeSmtcChevronDownIcon
    {0x1204a3f0, GlyphKind::down}, // kOverflowChevronIcon
    {0x12045a90, GlyphKind::down}, // kChevronDownIcon
    {0x127a0348, GlyphKind::down}, // kEdgeSmtcDropdownChevronIcon
};
static std::span<const GlyphAddress> kGlyphAddresses = kGlyphAddresses48;
static bool g_build53 = false;
static const unsigned char* g_styleModule;
// Return addresses immediately after the shadow queries that gate popup
// clipping/radii. Shadow-selection queries deliberately aren't in this list.
// The exact build, CALL targets and following instructions are validated before
// installing the hook. This doesn't change SPI_GETDROPSHADOW for other callers.
struct PopupBranch { DWORD afterCall; DWORD target; unsigned char following[8]; bool radiusGetter; };
static constexpr PopupBranch kPopupBranches48[] = {
    {0x50c8d47, 0x318ea7f, {0x84,0xc0,0x74,0x48,0x48,0x89,0xd9,0xe8}, false},
    {0x52429b0, 0x318ea7f, {0x84,0xc0,0x74,0x28,0x80,0xbb,0xd0,0x01}, false},
    {0xaf75c80, 0x318ea7f, {0x84,0xc0,0x74,0x72,0x48,0x8b,0x07,0x48}, false},
    {0xaf762be, 0x318ea7f, {0x84,0xc0,0x74,0x0f,0x41,0x80,0xbc,0x24}, false},
    {0xaf763dc, 0x318ea7f, {0x84,0xc0,0x0f,0x84,0x23,0xff,0xff,0xff}, false},
    {0x50c8d53, 0x50c8eae, {0x31,0xc9,0x85,0xc0,0x0f,0x4f,0xc8,0xf3}, true},
    {0x52429c5, 0x50c8eae, {0x31,0xc9,0x85,0xc0,0x0f,0x4f,0xc8,0x0f}, true},
};
static constexpr PopupBranch kPopupBranches53[] = {
    {0x50efc27, 0x31ac4bf, {0x84,0xc0,0x74,0x48,0x48,0x89,0xd9,0xe8}, false},
    {0x526b380, 0x31ac4bf, {0x84,0xc0,0x74,0x28,0x80,0xbb,0xd0,0x01}, false},
    {0xafa3900, 0x31ac4bf, {0x84,0xc0,0x74,0x72,0x48,0x8b,0x07,0x48}, false},
    {0xafa3f3e, 0x31ac4bf, {0x84,0xc0,0x74,0x0f,0x41,0x80,0xbc,0x24}, false},
    {0xafa405c, 0x31ac4bf, {0x84,0xc0,0x0f,0x84,0x23,0xff,0xff,0xff}, false},
    {0x50efc33, 0x50efd8e, {0x31,0xc9,0x85,0xc0,0x0f,0x4f,0xc8,0xf3}, true},
    {0x526b395, 0x50efd8e, {0x31,0xc9,0x85,0xc0,0x0f,0x4f,0xc8,0x0f}, true},
};
static std::span<const PopupBranch> kPopupBranches = kPopupBranches48;
static bool IsPopupCornerCaller(uintptr_t caller, bool radiusGetter) {
    if (!g_styleModule || g_isWebview || g_stopping.load() || !(g_settings.load() & (1u << 16))) return false;
    const auto base = reinterpret_cast<uintptr_t>(g_styleModule);
    for (const auto& branch : kPopupBranches)
        if (branch.radiusGetter == radiusGetter && caller == base + branch.afterCall) return true;
    return false;
}
static ShadowsFn g_popupShadowsOriginal;
static bool PopupShadowsHook() {
    if (IsPopupCornerCaller(reinterpret_cast<uintptr_t>(__builtin_return_address(0)), false)) return true;
    return g_popupShadowsOriginal();
}
static int (*g_popupRadiusOriginal)(const void*);
static int PopupRadiusHook(const void* self) {
    if (IsPopupCornerCaller(reinterpret_cast<uintptr_t>(__builtin_return_address(0)), true)) {
        int configured = static_cast<int>(g_settings.load() & 0xff) - 1;
        if (configured >= 0) return configured;
    }
    return g_popupRadiusOriginal(self);
}
static bool ValidatePopupBranches(const unsigned char* base, const std::vector<CodeRange>& ranges) {
    for (const auto& branch : kPopupBranches) {
        const auto* call = base + branch.afterCall - 5;
        if (!Contains(ranges, call, 13) || call[0] != 0xe8 ||
            static_cast<int64_t>(branch.afterCall) + ReadField<int32_t>(call, 1) != branch.target ||
            std::memcmp(call + 5, branch.following, sizeof(branch.following)) != 0) return false;
    }
    // The public shadow helper is reached through this verified jump thunk.
    const auto* thunk = base + kPopupBranches[0].target;
    return Contains(ranges, thunk, 5) && thunk[0] == 0xe9 &&
           ReadField<int32_t>(thunk, 1) == 0;
}
static GlyphKind FindGlyph(const void* icon) {
    if (!g_styleModule) return GlyphKind::count;
    const uintptr_t address = reinterpret_cast<uintptr_t>(icon);
    const uintptr_t base = reinterpret_cast<uintptr_t>(g_styleModule);
    for (const auto& entry : kGlyphAddresses)
        if (address == base + entry.rva) return entry.kind;
    return GlyphKind::count;
}
using ImageRepCtorFn = void* (*)(void*, const void*, float);
using ImageFromRepFn = void* (*)(void*, const void*);
static ImageRepCtorFn g_imageRepCtor;
static ImageFromRepFn g_imageFromRep;
static OpaqueObjectDtorFn g_imageRepDtor;
// Draw in grayscale at 4x resolution, then retain the logical DIP dimensions
// in ImageSkiaRep. Original ARGB color (including disabled opacity) is retained.
// Default submenu arrows use a small, straight-sided outline rather than a
// hinted font glyph. Keeping the geometry symmetric avoids the uneven shoulder
// produced by GDI text rasterization at menu sizes. Custom glyphs stay custom.
static bool IsDefaultMenuChevron(GlyphKind kind, const std::wstring& glyph,
                                 const std::wstring& family, bool menu) {
    return menu && _wcsicmp(family.c_str(), L"Segoe Fluent Icons") == 0 &&
        ((kind == GlyphKind::right && glyph == L"\uE76C") ||
         (kind == GlyphKind::left && glyph == L"\uE76B"));
}
static UINT CurrentUiDpi();
static bool DrawMenuChevron(void* raw, int pixels, GlyphKind kind, double scale) {
    if (!raw || pixels < 1 || scale <= 0) return false;
    // Rasterize at the actual display scale, with subpixel coverage. GDI's
    // integer polygon vertices produced unequal arms after downsampling.
    const double stroke = 0.85 * scale;
    const double height = std::min(double(pixels) - 1, 8.0 * scale + 1.0);
    const double halfY = (height - stroke) / 2;
    const double halfX = (height * 0.5 - stroke) / 2;
    const double center = pixels / 2.0;
    auto distance = [](double x, double y, double ax, double ay, double bx, double by) {
        const double dx = bx - ax, dy = by - ay;
        const double t = std::clamp(((x-ax)*dx + (y-ay)*dy)/(dx*dx+dy*dy), 0.0, 1.0);
        return std::hypot(x-ax-t*dx, y-ay-t*dy);
    };
    auto* values = static_cast<uint32_t*>(raw);
    for (int y=0; y<pixels; ++y) for (int x=0; x<pixels; ++x) {
        unsigned coverage=0;
        for (int sy=0; sy<8; ++sy) for (int sx=0; sx<8; ++sx) {
            double px=x+(sx+0.5)/8-center, py=y+(sy+0.5)/8-center;
            if (kind == GlyphKind::left) px=-px;
            if (std::min(distance(px,py,-halfX,-halfY,halfX,0),
                         distance(px,py,halfX,0,-halfX,halfY)) <= stroke/2) ++coverage;
        }
        const unsigned gray=(coverage*255+32)/64;
        values[y*pixels+x] = gray * 0x010101;
    }
    return true;
}
static bool CreateGlyphImage(void* result, GlyphKind kind, int size, uint32_t color, bool menuChevron = false, bool tabClose = false) {
    if ((!tabClose && kind == GlyphKind::count) || size < 1 || size > 128 || g_stopping.load()) return false;
    std::wstring glyph, family;
    {
        std::lock_guard<std::mutex> lock(g_browserStyleMutex);
        if (tabClose) { glyph = L"x"; family = L"Segoe UI"; }
        else { glyph = g_glyphs[static_cast<size_t>(kind)]; family = g_glyphFont; }
    }
    if (glyph.empty() || family.empty() || family.size() >= LF_FACESIZE) return false;
    // Fluent chevrons have substantial em-box padding. Use a larger em and
    // center their ink bounds so the visible arrow matches native Windows menus.
    const bool chevron = kind == GlyphKind::right || kind == GlyphKind::left ||
                         kind == GlyphKind::down || kind == GlyphKind::up;
    const bool nativeMenuChevron = IsDefaultMenuChevron(kind, glyph, family, menuChevron);
    const double scale = nativeMenuChevron ? CurrentUiDpi() / 96.0 : 4.0;
    // One extra DIP provides room for the requested extra physical pixel.
    if (nativeMenuChevron) size = std::max(size, 9);
    const int pixels = int(std::ceil(size * scale));
    const int fontPixels = chevron && menuChevron ? pixels * 7 / 5 : pixels;
    HDC dc = CreateCompatibleDC(nullptr);
    if (!dc) return false;
    BITMAPINFO info{};
    info.bmiHeader = {sizeof(BITMAPINFOHEADER), pixels, -pixels, 1, 32, BI_RGB, 0, 0, 0, 0, 0};
    void* raw = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &raw, nullptr, 0);
    HFONT font = CreateFontW(-fontPixels, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, DEFAULT_PITCH, family.c_str());
    bool drawn = false;
    HICON icon = nullptr;
    if (bitmap && font && raw) {
        auto oldBitmap = SelectObject(dc, bitmap);
        auto oldFont = SelectObject(dc, font);
        wchar_t actualFace[LF_FACESIZE]{};
        GetTextFaceW(dc, LF_FACESIZE, actualFace);
        WORD indices[2]{};
        bool available = _wcsicmp(actualFace, family.c_str()) == 0 &&
            GetGlyphIndicesW(dc, glyph.data(), static_cast<int>(glyph.size()), indices, GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR;
        for (size_t i = 0; i < glyph.size(); ++i) available = available && indices[i] != 0xffff;
        if (available) {
            std::memset(raw, 0, pixels * pixels * 4);
            SetTextColor(dc, RGB(255, 255, 255)); SetBkMode(dc, TRANSPARENT);
            RECT bounds{0, 0, pixels, pixels};
            GLYPHMETRICS metrics{};
            MAT2 identity{{0, 1}, {0, 0}, {0, 0}, {0, 1}};
            if (tabClose) {
                // Match the reference's light X; independent of title font weight.
                const int extent = std::min(pixels - 4, int(8 * scale));
                const int first = (pixels - extent) / 2, last = first + extent;
                HPEN stroke = CreatePen(PS_SOLID, 2, RGB(255,255,255));
                if (stroke) {
                    auto previous = SelectObject(dc, stroke);
                    MoveToEx(dc, first, first, nullptr); LineTo(dc, last, last);
                    MoveToEx(dc, last, first, nullptr); LineTo(dc, first, last);
                    SelectObject(dc, previous); DeleteObject(stroke); drawn = true;
                }
            } else if (IsDefaultMenuChevron(kind, glyph, family, menuChevron)) {
                drawn = DrawMenuChevron(raw, pixels, kind, scale);
            } else if (chevron && glyph.size() == 1 &&
                GetGlyphOutlineW(dc, glyph[0], GGO_METRICS, &metrics, 0, nullptr, &identity) != GDI_ERROR &&
                metrics.gmBlackBoxX <= static_cast<UINT>(pixels) &&
                metrics.gmBlackBoxY <= static_cast<UINT>(pixels)) {
                SetTextAlign(dc, TA_LEFT | TA_BASELINE);
                const int x = (pixels - static_cast<int>(metrics.gmBlackBoxX)) / 2 - metrics.gmptGlyphOrigin.x;
                const int y = (pixels - static_cast<int>(metrics.gmBlackBoxY)) / 2 + metrics.gmptGlyphOrigin.y;
                drawn = TextOutW(dc, x, y, glyph.data(), 1) != 0;
            } else {
                drawn = DrawTextW(dc, glyph.c_str(), static_cast<int>(glyph.size()), &bounds,
                                 DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX) != 0;
            }
            GdiFlush();
            auto* values = static_cast<uint32_t*>(raw);
            for (int i = 0; i < pixels * pixels; ++i) {
                unsigned a = (values[i] & 255) * (color >> 24) / 255;
                values[i] = (a << 24) | ((((color >> 16) & 255) * a / 255) << 16) |
                            ((((color >> 8) & 255) * a / 255) << 8) | ((color & 255) * a / 255);
            }
        }
        SelectObject(dc, oldFont); SelectObject(dc, oldBitmap);
        if (drawn) {
            std::vector<unsigned char> maskBits(((pixels + 15) / 16) * 2 * pixels, 0);
            HBITMAP mask = CreateBitmap(pixels, pixels, 1, 1, maskBits.data());
            if (mask) {
                ICONINFO iconInfo{TRUE, 0, 0, mask, bitmap};
                icon = CreateIconIndirect(&iconInfo);
                DeleteObject(mask);
            }
        }
    }
    if (font) DeleteObject(font);
    if (bitmap) DeleteObject(bitmap);
    DeleteDC(dc);
    if (!icon) return false;
    OpaqueObjectStorage nativeBitmap{}, rep{};
    PrepareOpaqueObjectStorage(nativeBitmap); PrepareOpaqueObjectStorage(rep);
    g_CreateSkBitmapFromHICON(nativeBitmap.data, icon);
    DestroyIcon(icon);
    if (!IsOpaqueObjectGuardIntact(nativeBitmap)) return false;
    g_imageRepCtor(rep.data, nativeBitmap.data, static_cast<float>(scale));
    g_SkBitmapDtor(nativeBitmap.data);
    if (!IsOpaqueObjectGuardIntact(rep)) return false;
    g_imageFromRep(result, rep.data);
    g_imageRepDtor(rep.data);
    return true;
}
using VectorImageFn = void* (*)(void*, const void*);
static VectorImageFn g_vectorImageOriginal;
static bool IsMenuChevron(const void* icon) {
    const auto rva = reinterpret_cast<uintptr_t>(icon) - reinterpret_cast<uintptr_t>(g_styleModule);
    return g_build53 ? (rva == 0x1279b670 || rva == 0x1279b6c0 || rva == 0x12046660) : (rva == 0x1276b1e0 || rva == 0x1276b230 || rva == 0x12016d80);
}
static UINT CurrentUiDpi() {
    const HWND window = FindWindowForThread(GetCurrentThreadId());
    using DpiFn = UINT(WINAPI*)(HWND);
    static const auto getDpi = reinterpret_cast<DpiFn>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    const UINT dpi = window && getDpi ? getDpi(window) : 96;
    return dpi ? dpi : 96;
}
static void (*g_centerGlyphRectOriginal)(void*, const GfxSizeOpaque*);
static void* g_captionSizeCallGuard;
static GfxSizeOpaque ExplorerCaptionSize(GfxSizeOpaque size) {
    return {std::max(1, MulDiv(size.width, 4, 5)), std::max(1, MulDiv(size.height, 4, 5))};
}
static void CenterGlyphRectHook(void* rect, const GfxSizeOpaque* size) {
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0)) -
        reinterpret_cast<uintptr_t>(g_styleModule);
    if (!g_stopping.load() && !g_isWebview && g_build53 && caller == 0x2c88c23) {
        const auto smaller = ExplorerCaptionSize(*size);
        g_centerGlyphRectOriginal(rect, &smaller);
    } else g_centerGlyphRectOriginal(rect, size);
}
// Caption painters receive device-pixel coordinates after UndoDeviceScaleFactor.
// Use an Explorer-sized 8-DIP glyph box and a one-device-pixel outline instead
// of rounding the display scale to a thick two-pixel stroke.
using CaptionIconFn = void (*)(void*, void*, void*, void*);
static CaptionIconFn g_captionMinOriginal, g_captionCloseOriginal, g_captionRestoreOriginal;
static void (*g_captionMaxOriginal)(void*, const void*, void*, float);
static bool NormalizeCaptionFlags(void* flags) {
    if (g_stopping.load() || g_isWebview) return false;
    WriteField<float>(flags, 0x10, 1.0f);
    return true;
}
static void CaptionMinHook(void* self, void* canvas, void* rect, void* flags) {
    NormalizeCaptionFlags(flags); g_captionMinOriginal(self, canvas, rect, flags);
}
static void CaptionCloseHook(void* self, void* canvas, void* rect, void* flags) {
    NormalizeCaptionFlags(flags); g_captionCloseOriginal(self, canvas, rect, flags);
}
static void CaptionRestoreHook(void* self, void* canvas, void* rect, void* flags) {
    NormalizeCaptionFlags(flags); g_captionRestoreOriginal(self, canvas, rect, flags);
}
static void CaptionMaxHook(void* canvas, const void* rect, void* flags, float radius) {
    if (NormalizeCaptionFlags(flags)) radius = 1.0f;
    g_captionMaxOriginal(canvas, rect, flags, radius);
}
static std::atomic<int> g_addressRadius{-1}, g_addressGlyphSize{18};
static std::atomic<bool> g_addressHideBorder{false}, g_addressHideFocusOutline{false};
// Zero means native. Valid overrides are always opaque ARGB.
static std::atomic<uint32_t> g_addressBorderColor{0}, g_addressBackgroundColor{0};
static uint32_t ParseAddressColor(std::wstring value) {
    if (!value.empty() && value.front() == L'#') value.erase(0, 1);
    if (value.size() != 6) return 0;
    uint32_t color = 0;
    for (wchar_t c : value) {
        int digit = c >= L'0' && c <= L'9' ? c - L'0' :
            c >= L'a' && c <= L'f' ? c - L'a' + 10 : c >= L'A' && c <= L'F' ? c - L'A' + 10 : -1;
        if (digit < 0) return 0;
        color = (color << 4) | digit;
    }
    return color | 0xff000000;
}
static bool AddressAppearanceEnabled() {
    if (g_stopping.load() || g_isWebview) return false;
    HIGHCONTRASTW contrast{sizeof(contrast), 0, nullptr};
    return SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) &&
        !(contrast.dwFlags & HCF_HIGHCONTRASTON);
}
static int (*g_addressRadiusOriginal)(const void*);
static int AddressRadiusHook(const void* self) {
    const int requested = g_addressRadius.load();
    return AddressAppearanceEnabled() && requested >= 0 ? requested : g_addressRadiusOriginal(self);
}
// LocationBarHighlightPathGenerator returns optional<gfx::RRectF>.
// Preserve its computed bounds and rebuild through the native constructor so
// SkRRect's radius clamping and type classification remain consistent.
static void* (*g_addressFocusOriginal)(const void*, void*, const void*);
static void* (*g_addressRRectCtor)(void*, float, float, float, float, float, float);
static void* AddressFocusHook(const void* self, void* out, const void* bounds) {
    void* result = g_addressFocusOriginal(self, out, bounds);
    const int radius = g_addressRadius.load();
    if (radius >= 0 && AddressAppearanceEnabled() && ReadField<bool>(out, 0x34)) {
        const float left = ReadField<float>(out, 0), top = ReadField<float>(out, 4);
        const float width = ReadField<float>(out, 8) - left;
        const float height = ReadField<float>(out, 12) - top;
        g_addressRRectCtor(out, left, top, width, height, float(radius), float(radius));
    }
    return result;
}
static void* (*g_addressBackgroundOriginal)(const void*, void*, uint32_t, uint32_t, int, bool, bool);
static void* AddressBackgroundHook(const void* self, void* out, uint32_t fill, uint32_t border,
                                    int blend, bool antialias, bool inset) {
    if (AddressAppearanceEnabled()) {
        if (const auto color = g_addressBackgroundColor.load()) fill = color;
        if (g_addressHideBorder.load()) border = 0;
        else if (const auto color = g_addressBorderColor.load(); color && border) border = color;
    }
    return g_addressBackgroundOriginal(self, out, fill, border, blend, antialias, inset);
}
static int NativeIconSize(const void* icon, int size) {
    const auto rva = reinterpret_cast<uintptr_t>(icon) - reinterpret_cast<uintptr_t>(g_styleModule);
    const bool tabChevron = g_build53 ? (rva == 0x1279ffc0 || rva == 0x12045a90) :
        (rva == 0x1276fb30 || rva == 0x120161b0);
    if (g_stopping.load() || (FindGlyph(icon) != GlyphKind::new_tab && !tabChevron)) return size;
    // Both glyphs have internal whitespace: the previous 18px image yielded
    // 14px visible ink. Use a 24px image to obtain approximately 18px ink.
    return std::max(1, MulDiv(24, 96, CurrentUiDpi()));
}
static bool IsTabCloseIcon(const void* icon) {
    if (!g_build53 || g_isWebview || !g_styleModule) return false;
    const auto rva = reinterpret_cast<uintptr_t>(icon) - reinterpret_cast<uintptr_t>(g_styleModule);
    return rva == 0x12043b08 || rva == 0x12040e88;
}
static bool CreateSystemFolderImage(void* result, const void* icon) {
    if (g_stopping.load() || !g_useWindowsFolderIcon.load() || HasFolderGlyph() ||
        !g_copyImageSkia || FindGlyph(icon) != GlyphKind::folder) return false;
    CreateWindowsFolderImage();
    std::lock_guard<std::mutex> lock(g_windowsFolderImageMutex);
    if (!g_windowsFolderReady.load() || g_windowsFolderThreadId != GetCurrentThreadId()) return false;
    g_copyImageSkia(result, g_imageSkiaStorage.data);
    return true;
}
static void* VectorImageHook(void* result, const void* description) {
    const void* icon = ReadField<const void*>(description, 0);
    if (CreateSystemFolderImage(result, icon)) return result;
    const int size = NativeIconSize(icon, ReadField<int>(description, 8));
    if (CreateGlyphImage(result, FindGlyph(icon), size, ReadField<uint32_t>(description, 12), IsMenuChevron(icon), IsTabCloseIcon(icon))) return result;
    // Preserve the optional badge icon and its parameters as well.
    alignas(8) unsigned char adjusted[32];
    std::memcpy(adjusted, description, sizeof(adjusted));
    WriteField<int>(adjusted, 8, size);
    return g_vectorImageOriginal(result, adjusted);
}

using PaintVectorFn = void (*)(void*, const void*, int, uint32_t);
using DrawImageFn = void (*)(void*, const void*, int, int);
static PaintVectorFn g_paintVectorOriginal;
static DrawImageFn g_drawImage;
static void PaintVectorHook(void* canvas, const void* icon, int size, uint32_t color) {
    size = NativeIconSize(icon, size);
    OpaqueObjectStorage image{};
    PrepareOpaqueObjectStorage(image);
    if ((CreateSystemFolderImage(image.data, icon) ||
         CreateGlyphImage(image.data, FindGlyph(icon), size, color, IsMenuChevron(icon), IsTabCloseIcon(icon))) && IsOpaqueObjectGuardIntact(image)) {
        g_drawImage(canvas, image.data, 0, 0);
        g_ImageSkiaDtor(image.data);
        return;
    }
    g_paintVectorOriginal(canvas, icon, size, color);
}

// The active horizontal tab uses color 0xab9 in this verified Edge build
// (EdgeTabStyle::GetTabBackgroundColor, kActive, frame_active=true). This is
// resolved from Edge's palette on each call rather than fixing a gray RGB value.
static const void* (*g_frameBrowserView)(const void*);
static const void* (*g_styleGetWidget)(const void*);
static const void* (*g_styleGetColorProvider)(const void*);
static bool (*g_styleUsingDefaultTheme)(const void*);
static bool (*g_styleVerticalTabs)(const void*);
static bool (*g_styleWorkspace)(const void*);
static uint32_t (*g_styleGetColor)(const void*, int);
static uint32_t (*g_frameColorOriginal)(const void*, int);
static uint32_t (*g_tabBackgroundOriginal)(const void*, int, bool);
static bool IsBlackAgainstDarkGray(uint32_t original, uint32_t gray) {
    if (original != 0xff000000 && original != 0) return false;
    if ((gray >> 24) != 255) return false;
    const int r = (gray >> 16) & 255, g = (gray >> 8) & 255, b = gray & 255;
    return std::min({r, g, b}) >= 16 && std::max({r, g, b}) <= 96 &&
           std::max({r, g, b}) - std::min({r, g, b}) <= 8;
}
static uint32_t MatchDarkFrameColor(const void* view, const void* browserView, uint32_t original) {
    if (g_stopping.load() || g_isWebview || !browserView) return original;
    const bool common = g_commonTabColor.load();
    if (!common && original != 0xff000000 && original != 0) return original;
    if (g_styleVerticalTabs(browserView)) return original;
    // BrowserView's BrowserWindowInterface is the same field used by
    // BrowserWidget::UsingDefaultTheme and native workspace detection.
    const void* browser = ReadField<const void*>(browserView, 0x600);
    if (!browser || (!common && g_styleWorkspace(browser))) return original;
    const void* widget = g_styleGetWidget(view);
    if (!widget || (!common && !g_styleUsingDefaultTheme(widget))) return original;
    HIGHCONTRASTW contrast{sizeof(contrast), 0, nullptr};
    if (!SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) ||
        (contrast.dwFlags & HCF_HIGHCONTRASTON)) return original;
    const void* provider = g_styleGetColorProvider(view);
    if (!provider) return original;
    const uint32_t gray = g_styleGetColor(provider, 0xab9);
    if (common) return (gray >> 24) == 255 ? gray : original;
    return IsBlackAgainstDarkGray(original, gray) ? 0xff1d1d1d : original;
}
static uint32_t FrameColorHook(const void* self, int state) {
    const auto original = g_frameColorOriginal(self, state);
    return MatchDarkFrameColor(self, g_frameBrowserView(self), original);
}
static uint32_t TabBackgroundHook(const void* self, int state, bool active) {
    const auto original = g_tabBackgroundOriginal(self, state, active);
    if (state != 2 || g_stopping.load()) return original;
    const auto* tab = ReadField<const unsigned char*>(self, 0x10);
    if (!tab) return original;
    const void* view = tab + 0x18; // Tab's views::View base, verified in the original.
    const void* widget = g_styleGetWidget(view);
    if (!widget) return original;
    return MatchDarkFrameColor(view, ReadField<const void*>(widget, 0x3d0), original);
}

struct ViewRect { int x, y, width, height; };
using LayoutFn = void (*)(void*);
using SetBoundsFn = void (*)(void*, const ViewRect*);
static LayoutFn g_favoritesLayoutOriginal, g_tabStripLayoutOriginal, g_tabStripDtorOriginal;
static LayoutFn g_layoutView;
static SetBoundsFn g_setBounds;
static bool (*g_getVisible)(const void*);
static ToolbarActionViewCalculatePreferredSizeFn g_getPreferredSize;
static std::mutex g_tabStripsMutex;
static std::unordered_map<void*, DWORD> g_tabStrips;
static int CenterOffset(int start, int end, int occupiedStart, int occupiedEnd) {
    if (start < 0 || end <= start || end > 100000 || occupiedStart < start ||
        occupiedEnd > end || occupiedEnd <= occupiedStart) return 0;
    return start + (end - start - (occupiedEnd - occupiedStart)) / 2 - occupiedStart;
}
static LayoutFn g_favoriteUpdateImage;
static thread_local bool g_refreshingFavoriteImages = false;
static void RefreshFavoriteImages(void* view, int depth = 0) {
    if (!view || !g_favoriteUpdateImage || depth > 4 || g_refreshingFavoriteImages) return;
    if (IsIconFreeFavorite(view)) {
        g_refreshingFavoriteImages = true;
        g_favoriteUpdateImage(view);
        g_refreshingFavoriteImages = false;
        return;
    }
    auto** children = ReadField<void**>(view, 0x1a0);
    const size_t count = ReadField<size_t>(view, 0x1a8);
    if (!children || count > 1000) return;
    for (size_t i = 0; i < count; ++i) RefreshFavoriteImages(children[i], depth + 1);
}
static void FavoritesLayoutHook(void* self) {
    { std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex); g_bookmarkBars[self] = GetCurrentThreadId(); }
    // Refresh the actual image container before it reserves label space.
    // Paint suppression alone cannot remove a pre-existing composited image.
    RefreshFavoriteImages(self);
    g_favoritesLayoutOriginal(self);
    if (!g_centerFavorites.load() || g_stopping.load()) return;
    const auto count = ReadField<size_t>(self, 0x1a8);
    auto** children = ReadField<void**>(self, 0x1a0);
    if (!children || count > 10000) return;
    std::vector<void*> favorites;
    // BookmarkBarView::Layout starts at 6 DIP on this verified Edge build.
    // The previous 8-DIP lower bound rejected the normal first button at x=6.
    constexpr int kBarInset = 6;
    int first = 100000, last = 0, left = kBarInset,
        right = ReadField<int>(self, 0x1d0) - kBarInset;
    // Edge's bookmark records are 16 bytes; the first field is the button.
    // This excludes Other favorites, overflow and managed-policy controls.
    const auto favoriteCount = ReadField<size_t>(self, 0x678);
    const auto* records = ReadField<const unsigned char*>(self, 0x670);
    if (!records || favoriteCount > 10000) return;
    for (size_t i = 0; i < favoriteCount; ++i) {
        void* child = ReadField<void*>(records, i * 16);
        if (!child || !g_getVisible(child)) return; // Overflow: retain Edge layout.
        auto rect = ReadField<ViewRect>(child, 0x1c8);
        if (rect.width <= 0) continue;
        favorites.push_back(child);
        first = std::min(first, rect.x); last = std::max(last, rect.x + rect.width);
    }
    if (favorites.empty()) return;
    for (size_t i = 0; i < count; ++i) {
        void* child = children[i];
        if (std::find(favorites.begin(), favorites.end(), child) != favorites.end() || !g_getVisible(child)) continue;
        auto rect = ReadField<ViewRect>(child, 0x1c8);
        if (rect.width <= 0) continue;
        if (rect.x >= last) right = std::min(right, rect.x);
        else if (rect.x + rect.width <= first) left = std::max(left, rect.x + rect.width);
        // Full-bar backgrounds and overlay views can span the favorites. They
        // don't reserve horizontal space; only disjoint side controls do.
    }
    const int shift = CenterOffset(left, right, first, last);
    if (!shift) return;
    for (void* child : favorites) {
        auto rect = ReadField<ViewRect>(child, 0x1c8); rect.x += shift;
        g_setBounds(child, &rect);
    }
}
struct NativeStringView { const char* data; size_t size; };
static NativeStringView* (*g_viewClassName)(const void*, NativeStringView*);
static bool ClassIs(const void* view, const char* name) {
    if (!g_viewClassName || !view) return false;
    NativeStringView result{}; g_viewClassName(view, &result);
    const size_t length = std::strlen(name);
    return result.data && result.size == length && !std::memcmp(result.data, name, length);
}
static bool IsAddressGlyph(const void* view) {
    for (int i = 0; view && i < 10; ++i, view = ReadField<const void*>(view, 0x198)) {
        if (ClassIs(view, "LocationBarView") || ClassIs(view, "EdgeBackForwardButton") ||
            ClassIs(view, "BackForwardButton") || ClassIs(view, "ReloadButton")) return true;
    }
    return false;
}
static void (*g_focusRingPaintOriginal)(void*, void*);
static void AddressFocusPaintHook(void* self, void* canvas) {
    // Only the ring owned by the location bar, not its embedded buttons.
    if (g_addressHideFocusOutline.load() && AddressAppearanceEnabled() &&
        ClassIs(ReadField<const void*>(self, 0x198), "LocationBarView")) return;
    g_focusRingPaintOriginal(self, canvas);
}
struct GlyphInk { int width, height; ViewRect ink; };
static bool MeasureGlyphInk(const void* bitmap, GlyphInk& result) {
    // Exact-build SkBitmap: pixels +8, row bytes +16, ImageInfo +24.
    const auto* pixels = ReadField<const unsigned char*>(bitmap, 8);
    const size_t stride = ReadField<size_t>(bitmap, 16);
    const int width = ReadField<int>(bitmap, 40), height = ReadField<int>(bitmap, 44);
    const int colorType = ReadField<int>(bitmap, 32);
    if (!pixels || width <= 0 || height <= 0 || width > 512 || height > 512 ||
        (colorType != 4 && colorType != 6) || stride < size_t(width) * 4 || stride > 16384) return false;
    int left = width, top = height, right = -1, bottom = -1;
    unsigned maxAlpha = 0;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) maxAlpha = std::max(maxAlpha, unsigned(pixels[y * stride + x * 4 + 3]));
    if (!maxAlpha) return false;
    // Relative threshold keeps disabled (translucent) icons measurable too.
    const unsigned threshold = std::max(1u, maxAlpha / 16);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        if (pixels[y * stride + x * 4 + 3] < threshold) continue;
        left = std::min(left, x); top = std::min(top, y);
        right = std::max(right, x); bottom = std::max(bottom, y);
    }
    result = {width, height, {left, top, right - left + 1, bottom - top + 1}};
    return true;
}
static ViewRect SizeVisibleGlyph(ViewRect original, const GlyphInk& glyph, int pixels, UINT dpi) {
    const int longest = std::max(glyph.ink.width, glyph.ink.height);
    if (longest <= 0 || !dpi) return original;
    const double scale = double(pixels) * 96 / dpi / longest;
    ViewRect result = original;
    result.width = std::max(1, int(std::lround(glyph.width * scale)));
    result.height = std::max(1, int(std::lround(glyph.height * scale)));
    // Center the visible artwork rather than its potentially asymmetric padding.
    result.x = int(std::lround(original.x + original.width * 0.5 -
        (glyph.ink.x + glyph.ink.width * 0.5) * result.width / glyph.width));
    result.y = int(std::lround(original.y + original.height * 0.5 -
        (glyph.ink.y + glyph.ink.height * 0.5) * result.height / glyph.height));
    return result;
}
static void* (*g_addressGetImage)(const void*, void*, float);
static const void* (*g_addressGetRep)(const void*, float);
static const void* (*g_addressGetBitmap)(const void*);
static ViewRect* (*g_addressImageBoundsOriginal)(const void*, ViewRect*);
static ViewRect* AddressImageBoundsHook(const void* self, ViewRect* out) {
    auto* result = g_addressImageBoundsOriginal(self, out);
    // Suppress even ImageViews populated before their favorite acquired a parent.
    if (IsFavoriteImageHidden(self)) {
        out->width = out->height = 0;
        return result;
    }
    const int requested = g_addressGlyphSize.load();
    if (g_stopping.load() || g_isWebview || requested < 0 || !IsAddressGlyph(self)) return result;
    if (out->width <= 0 || out->height <= 0) return result;
    const UINT dpi = CurrentUiDpi();
    GlyphInk glyph{out->width, out->height, {0, 0, out->width, out->height}};
    if (g_addressGetImage && g_addressGetRep && g_addressGetBitmap) {
        // ImageSkia owns a refcounted storage pointer; native helpers own all
        // allocation/rasterization, and destruction balances that reference.
        void* image = nullptr;
        g_addressGetImage(self, &image, float(dpi) / 96);
        if (image) {
            const void* rep = g_addressGetRep(&image, float(dpi) / 96);
            if (rep) if (const void* bitmap = g_addressGetBitmap(rep)) MeasureGlyphInk(bitmap, glyph);
        }
        g_ImageSkiaDtor(&image);
    }
    *out = SizeVisibleGlyph(*out, glyph, requested, dpi);
    return result;
}

static bool ShouldHideToolbarButton(const void* view) {
    if (g_stopping.load() || g_isWebview) return false;
    return (g_hideExtensions.load() && (ClassIs(view, "EdgeExtensionsHubButton") || ClassIs(view, "EdgeExtensionsMenuButton"))) ||
           (g_hideProfile.load() && ClassIs(view, "EdgeAvatarToolbarButton"));
}
static std::mutex g_hiddenToolbarMutex;
static std::unordered_map<void*, bool> g_hiddenToolbarViews;
static void RememberToolbarVisibility(void* view, bool visible) {
    std::lock_guard<std::mutex> lock(g_hiddenToolbarMutex);
    g_hiddenToolbarViews[view] = visible;
}
struct TooltipInsets { int top, left, bottom, right; };
struct TooltipBorderInfo { void* border; TooltipInsets insets; };
static std::mutex g_tooltipBordersMutex;
static std::unordered_map<void*, TooltipBorderInfo> g_tooltipBorders;
static TooltipInsets* (*g_roundedInsetsOriginal)(const void*, TooltipInsets*);
static TooltipInsets* RoundedInsetsHook(const void* border, TooltipInsets* out) {
    { std::lock_guard<std::mutex> lock(g_tooltipBordersMutex);
      for (const auto& [view, info] : g_tooltipBorders) {
        if (info.border == border) { *out = info.insets; return out; }
      } }
    return g_roundedInsetsOriginal(border, out);
}
static LayoutFn g_viewDtorOriginal;
static void ViewDtorHook(void* self) {
    { std::lock_guard<std::mutex> lock(g_hiddenToolbarMutex); g_hiddenToolbarViews.erase(self); }
    { std::lock_guard<std::mutex> lock(g_tooltipBordersMutex); g_tooltipBorders.erase(self); }
    g_viewDtorOriginal(self);
}
static void HideToolbarDescendants(void* view, int depth = 0) {
    if (!view || depth > 5) return;
    if (ShouldHideToolbarButton(view)) {
        { std::lock_guard<std::mutex> lock(g_hiddenToolbarMutex);
          g_hiddenToolbarViews.try_emplace(view, g_getVisible(view)); }
        g_ViewSetVisibleOriginal(view, false); return;
    }
    bool restore = false, visible = false;
    { std::lock_guard<std::mutex> lock(g_hiddenToolbarMutex);
      auto it = g_hiddenToolbarViews.find(view);
      if (it != g_hiddenToolbarViews.end()) { restore = true; visible = it->second; g_hiddenToolbarViews.erase(it); } }
    if (restore) g_ViewSetVisibleOriginal(view, visible);
    auto** children = ReadField<void**>(view, 0x1a0);
    const size_t count = ReadField<size_t>(view, 0x1a8);
    if (!children || count > 1000) return;
    for (size_t i = 0; i < count; ++i) HideToolbarDescendants(children[i], depth + 1);
}
using DwmAttributeFn = HRESULT(WINAPI*)(HWND, DWORD, const void*, DWORD);
using DwmGetAttributeFn = HRESULT(WINAPI*)(HWND, DWORD, void*, DWORD);
static DwmAttributeFn g_dwmSetOriginal;
static DwmGetAttributeFn g_dwmGet;
static std::mutex g_frameBordersMutex;
static std::unordered_map<HWND, DWORD> g_frameBorderColors;
static bool IsEdgeMainWindow(HWND window) {
    if (g_isWebview) return false;
    DWORD process = 0; GetWindowThreadProcessId(window, &process);
    if (process != GetCurrentProcessId() || GetAncestor(window, GA_ROOT) != window || GetWindow(window, GW_OWNER)) return false;
    const auto style = GetWindowLongPtrW(window, GWL_STYLE);
    wchar_t name[64]{}; GetClassNameW(window, name, ARRAYSIZE(name));
    return (style & WS_CAPTION) == WS_CAPTION && (style & WS_THICKFRAME) &&
        wcscmp(name, L"Chrome_WidgetWin_1") == 0;
}
static bool HideOuterFrame() { return !g_stopping.load() && g_hideContentBorder.load() && !g_isWebview; }
static HRESULT WINAPI DwmSetHook(HWND window, DWORD attribute, const void* value, DWORD size) {
    if (attribute == 34 && size == sizeof(DWORD) && value && HideOuterFrame() && IsEdgeMainWindow(window)) {
        { std::lock_guard<std::mutex> lock(g_frameBordersMutex);
          g_frameBorderColors[window] = *static_cast<const DWORD*>(value); }
        const DWORD none = 0xfffffffe; // DWMWA_COLOR_NONE
        return g_dwmSetOriginal(window, attribute, &none, sizeof(none));
    }
    return g_dwmSetOriginal(window, attribute, value, size);
}
static void ApplyOuterFrame(HWND window) {
    if (!g_dwmSetOriginal || !g_dwmGet || !IsEdgeMainWindow(window)) return;
    std::lock_guard<std::mutex> lock(g_frameBordersMutex);
    auto it = g_frameBorderColors.find(window);
    if (HideOuterFrame()) {
        if (it == g_frameBorderColors.end()) {
            DWORD original = 0xffffffff;
            if (FAILED(g_dwmGet(window, 34, &original, sizeof(original)))) return;
            g_frameBorderColors.emplace(window, original);
        }
        const DWORD none = 0xfffffffe;
        g_dwmSetOriginal(window, 34, &none, sizeof(none));
    } else if (it != g_frameBorderColors.end()) {
        g_dwmSetOriginal(window, 34, &it->second, sizeof(it->second));
        g_frameBorderColors.erase(it);
    }
}
static void RefreshOuterFrames() {
    if (g_isWebview || !g_dwmSetOriginal) return;
    EnumWindows([](HWND window, LPARAM) -> BOOL { ApplyOuterFrame(window); return TRUE; }, 0);
    std::lock_guard<std::mutex> lock(g_frameBordersMutex);
    for (auto it = g_frameBorderColors.begin(); it != g_frameBorderColors.end();) {
        if (!IsEdgeMainWindow(it->first)) it = g_frameBorderColors.erase(it); else ++it;
    }
}
static LayoutFn g_toolbarLayoutOriginal;
static void ToolbarLayoutHook(void* self) {
    ApplyOuterFrame(FindWindowForThread(GetCurrentThreadId()));
    void* root = self;
    for (int i = 0; i < 20; ++i) {
        void* parent = ReadField<void*>(root, 0x198); if (!parent) break; root = parent;
    }
    HideToolbarDescendants(root);
    g_toolbarLayoutOriginal(self);
}
using MouseEventFn = void (*)(void*, const void*);
static MouseEventFn g_tabEnterOriginal, g_tabExitOriginal;
static void RefreshHoveredCloseButtons(void* parent, bool hovered) {
    if (g_closeButtonMode.load() != 1 || g_stopping.load()) return;
    std::vector<void*> buttons;
    { std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
      for (auto [button, owner] : g_tabCloseButtons)
        if (owner == GetCurrentThreadId() && ReadField<void*>(button, 0x198) == parent) buttons.push_back(button); }
    for (auto* button : buttons) {
        // Let native tab layout decide whether there is enough room to show it.
        g_ViewInvalidateLayout(parent, false);
        g_layoutView(parent);
        if (hovered) {
            // Native hover animation can leave the button hidden after layout.
            // Keep zero-sized/pinned controls hidden, but show usable bounds now.
            const auto bounds = ReadField<ViewRect>(button, 0x1c8);
            if (bounds.width > 0 && bounds.height > 0) {
                g_ViewSetVisibleOriginal(button, true);
                // Edge paints the X in a separate child canvas. Showing only the
                // outer button waits for its next paint to expose that child.
                if (ClassIs(button, "EdgeTabCloseButton")) {
                    void* glyph = ReadField<void*>(button, 0x738);
                    if (glyph) g_ViewSetVisibleOriginal(glyph, true);
                }
            }
        } else g_ViewSetVisibleOriginal(button, false);
    }
}
static void TabEnterHook(void* self, const void* event) {
    const void* previous = g_enteringTab; g_enteringTab = self;
    g_tabEnterOriginal(self, event); RefreshHoveredCloseButtons(self, true);
    g_enteringTab = previous;
}
static void TabExitHook(void* self, const void* event) {
    g_tabExitOriginal(self, event); RefreshHoveredCloseButtons(self, false);
}
static ToolbarActionViewCalculatePreferredSizeFn g_labelPreferredOriginal;
static GfxSizeOpaque* LabelPreferredHook(const void* self, GfxSizeOpaque* result, const void* available) {
    auto* returned = g_labelPreferredOriginal(self, result, available);
    if (!result || !IsIconFreeFavorite(self)) return returned;
    const void* label = ReadField<const void*>(self, 0x570);
    if (!label) return returned;
    GfxSizeOpaque text{}; alignas(16) unsigned char unlimited[16]{};
    g_getPreferredSize(label, &text, unlimited);
    // Keep native left inset, mirror it on the right, and discard icon/min-width slack.
    struct Insets { int top, left, bottom, right; } insets{};
    using InsetsFn = Insets* (*)(const void*, Insets*);
    auto table = ReadField<const unsigned char*>(self, 0);
    auto getInsets = ReadField<InsetsFn>(table, 0x60);
    getInsets(self, &insets);
    if (text.width > 0 && insets.left >= 0 && insets.left <= 64)
        result->width = std::min(result->width, text.width + 2 * insets.left);
    return returned;
}
// Position text after native child layout, not just by changing button width.
static void CenterFavoriteLabel(void* self) {
    if (!IsIconFreeFavorite(self)) return;
    void* label = ReadField<void*>(self, 0x570);
    if (!label) return;
    GfxSizeOpaque text{}; alignas(16) unsigned char unlimited[16]{};
    g_getPreferredSize(label, &text, unlimited);
    const int width = ReadField<int>(self, 0x1d0);
    auto bounds = ReadField<ViewRect>(label, 0x1c8);
    if (width <= 0 || text.width <= 0 || bounds.height <= 0) return;
    bounds.width = std::min(bounds.width, text.width);
    bounds.x = (width - bounds.width) / 2;
    g_setBounds(label, &bounds);
}
static LayoutFn g_viewLayoutOriginal;
static void ApplyActiveTitleStyle(void* view);
static void ViewLayoutHook(void* self) {
    if (IsIconFreeFavorite(self)) RefreshFavoriteImages(self);
    g_viewLayoutOriginal(self);
    CenterFavoriteLabel(self);
    ApplyActiveTitleStyle(self);
}
using RectGetterFn = ViewRect* (*)(const void*, ViewRect*);
static int (*g_tabAnchorPositionOriginal)(const void*);
static int TabAnchorPositionHook(const void* self) {
    const int original = g_tabAnchorPositionOriginal(self);
    // Horizontal anchors are TOP_LEFT/TOP_RIGHT; vertical tabs use side anchors.
    return g_centerTabs.load() && !g_stopping.load() && (original == 0 || original == 1) ? 8 : original;
}
using PaintFn = void (*)(void*, void*);
static PaintFn g_contentBorderPaintOriginal;
static void ContentBorderPaintHook(void* self, void* canvas) {
    if (!g_hideContentBorder.load() || g_stopping.load()) g_contentBorderPaintOriginal(self, canvas);
}
// App-layer spacing is independent of the thin painted content outline.
static TooltipInsets* (*g_contentMarginOriginal)(TooltipInsets*, void*);
static TooltipInsets* ContentMarginHook(TooltipInsets* out, void* browser) {
    auto* result = g_contentMarginOriginal(out, browser);
    if (g_hideContentBorder.load() && !g_stopping.load()) *out = {};
    return result;
}
static void (*g_contentRadiiOriginal)(void*, const void*);
static void ContentRadiiHook(void* self, const void* radii) {
    const float square[4] = {};
    g_contentRadiiOriginal(self, g_squareContentCorners.load() && !g_stopping.load() ? square : radii);
}
static void (*g_contentCornerVisibleOriginal)(void*, bool);
static void ContentCornerVisibleHook(void* self, bool visible) {
    g_contentCornerVisibleOriginal(self, g_squareContentCorners.load() && !g_stopping.load() ? false : visible);
}
static void (*g_contentCornerEnsureOriginal)(void*, uint64_t, bool);
static void ContentCornerEnsureHook(void* self, uint64_t radius, bool bottomOnly) {
    g_contentCornerEnsureOriginal(self, radius, bottomOnly);
    // Edge's separate overlay masks the webpage corners even when the container
    // background radius is zero. Suppress that mask as soon as it is created.
    if (g_squareContentCorners.load() && !g_stopping.load()) g_contentCornerVisibleOriginal(self, false);
}
// The corner mask contains its own cached one-pixel stroke, independent of
// EdgeContentsContainerBorder. Remove only that optional stroke in page masks.
static thread_local bool g_hidePageMaskStroke = false;
static void (*g_cornerBoundsOriginal)(void*, void*);
static void CornerBoundsHook(void* self, void* observed) {
    const bool previous = g_hidePageMaskStroke;
    const void* parent = ReadField<const void*>(self, 0x198);
    g_hidePageMaskStroke = !g_stopping.load() && g_hideContentBorder.load() &&
        parent && ClassIs(parent, "ContentsWebView");
    g_cornerBoundsOriginal(self, observed);
    g_hidePageMaskStroke = previous;
}
static void* (*g_cornerImageOriginal)(void*, uint32_t, uint64_t, const void*, int);
static void* CornerImageHook(void* out, uint32_t background, uint64_t border,
                             const void* shadows, int radius) {
    return g_cornerImageOriginal(out, background, g_hidePageMaskStroke ? 0 : border, shadows, radius);
}
static bool (*g_tabIsActive)(const void*);
static FontListOpaque* (*g_fontDerive)(const FontListOpaque*, FontListOpaque*, int, int, int);
static int (*g_fontStyle)(const FontListOpaque*), (*g_fontWeight)(const FontListOpaque*);
static void ApplyActiveTitleStyle(void* view) {
    if (!g_tabIsActive || !ClassIs(view, "EdgeTab")) return;
    const bool active = g_tabIsActive(static_cast<const unsigned char*>(view) - 0x18);
    const int mode = !g_stopping.load() && active ? g_activeTitleStyle.load() : 0;
    struct Pending { void* label; std::unique_ptr<OpaqueObjectStorage> font; };
    std::vector<Pending> pending;
    { std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
      for (auto& [label, info] : g_tabTitles) {
        if (g_tabCloseButtons.contains(label) || info.threadId != GetCurrentThreadId() || ReadField<void*>(label, 0x198) != view || info.appliedStyle == mode) continue;
        info.appliedStyle = mode;
        pending.push_back({label, CopyFontListToOwnedStorage(GetOwnedFontList(info.originalFontStorage), L"Active title")});
      } }
    for (auto& item : pending) {
        const auto* font = GetOwnedFontList(item.font);
        if (!font) continue;
        if (!mode) SetLabelFontForTargetSize(item.label, font, g_tabFontSize.load(), L"Tab title");
        else {
            OpaqueObjectStorage derived{}; PrepareOpaqueObjectStorage(derived);
            const int delta = g_tabFontSize.load() < 0 ? 0 : g_tabFontSize.load() - g_FontListGetFontSize(font);
            g_fontDerive(font, reinterpret_cast<FontListOpaque*>(derived.data), delta,
                         (g_fontStyle(font) & ~3) | (mode & 3), mode >> 2);
            if (IsOpaqueObjectGuardIntact(derived)) {
                g_LabelSetFontList(item.label, reinterpret_cast<const FontListOpaque*>(derived.data));
                g_FontListDtor(reinterpret_cast<FontListOpaque*>(derived.data));
            }
        }
        DestroyOwnedFontListStorage(std::move(item.font), L"Active title");
    }
}
static LayoutFn g_activeChangedOriginal;
static void ActiveChangedHook(void* self) {
    g_activeChangedOriginal(self);
    ApplyActiveTitleStyle(static_cast<unsigned char*>(self) + 0x18);
}
// Native Views objects own the background/border passed as a unique_ptr.
static void* (*g_roundedBackground)(void*, uint64_t, float, int);
static void* (*g_roundedBorder)(void*, int, float, const void*, uint64_t);
static void (*g_setBackground)(void*, void*);
static void (*g_setBorder)(void*, void*);
static thread_local bool g_paintingTooltipOutline = false;
static void (*g_roundedPaintOriginal)(void*, const void*, void*);
static void RoundedPaintHook(void* border, const void* view, void* canvas) {
    const bool previous = g_paintingTooltipOutline;
    { std::lock_guard<std::mutex> lock(g_tooltipBordersMutex);
      auto it = g_tooltipBorders.find(const_cast<void*>(view));
      g_paintingTooltipOutline = !g_stopping.load() && it != g_tooltipBorders.end() && it->second.border == border; }
    g_roundedPaintOriginal(border, view, canvas);
    g_paintingTooltipOutline = previous;
}
struct FloatRect { float x, y, width, height; };
static void (*g_drawRoundRectOriginal)(void*, const FloatRect*, float, const void*);
static void DrawRoundRectHook(void* canvas, const FloatRect* rect, float radius, const void* flags) {
    if (!g_paintingTooltipOutline) { g_drawRoundRectOriginal(canvas, rect, radius, flags); return; }
    const float stroke = 96.0f / CurrentUiDpi();
    const float outset = (1.0f - stroke) * 0.5f;
    const FloatRect adjusted{rect->x - outset, rect->y - outset, rect->width + 2 * outset, rect->height + 2 * outset};
    // Verified cc::PaintFlags layout; borrow its resources for this synchronous
    // call, without copying ownership or modifying the caller's flags.
    alignas(16) unsigned char adjustedFlags[0x48];
    std::memcpy(adjustedFlags, flags, sizeof(adjustedFlags));
    WriteField<float>(adjustedFlags, 0x10, stroke);
    g_drawRoundRectOriginal(canvas, &adjusted, radius + outset, adjustedFlags);
}
static LayoutFn g_tooltipThemeOriginal;
static void TooltipThemeHook(void* self) {
    g_tooltipThemeOriginal(self);
    if (g_stopping.load()) return;
    HIGHCONTRASTW contrast{sizeof(contrast), 0, nullptr};
    if (!SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) ||
        (contrast.dwFlags & HCF_HIGHCONTRASTON)) return;
    const void* provider = g_styleGetColorProvider(self);
    if (!provider) return;
    const uint32_t background = g_styleGetColor(provider, 0x1f9);
    const unsigned luminance = (((background >> 16) & 255) * 3 + ((background >> 8) & 255) * 6 + (background & 255)) / 10;
    // Windows-like subtle outline, while text/background retain Edge's theme palette.
    const uint32_t border = luminance < 128 ? 0xff1b1b1b : 0xffb8b8b8;
    const uint64_t bgVariant = uint64_t{background} | (uint64_t{1} << 32);
    const uint64_t borderVariant = uint64_t{border} | (uint64_t{1} << 32);
    void* bg = nullptr; void* outline = nullptr;
    g_roundedBackground(&bg, bgVariant, 6.0f, 0);
    g_setBackground(self, &bg);
    // The rounded-border inset moves BOTH its outline and its content. Keep the
    // outline at the outer edge; preserve the exact already-measured text insets.
    TooltipInsets insets{};
    using GetInsetsFn = TooltipInsets* (*)(const void*, TooltipInsets*);
    ReadField<GetInsetsFn>(ReadField<void*>(self, 0), 0x60)(self, &insets);
    const int paintInsets[4] = {};
    g_roundedBorder(&outline, 1, 6.0f, paintInsets, borderVariant);
    { std::lock_guard<std::mutex> lock(g_tooltipBordersMutex);
      g_tooltipBorders[self] = {outline, insets}; }
    g_setBorder(self, &outline);
}
static int WindowCenteredTabShift(int windowWidth, int regionX, int left, int right,
                                 int tabX, int tabWidth, int groupEnd) {
    const int groupWidth = groupEnd - tabX;
    if (windowWidth <= 0 || windowWidth > 100000 || tabWidth <= 0 ||
        groupWidth < tabWidth || right - left < groupWidth) return 0;
    const int target = (windowWidth - tabWidth) / 2 - regionX;
    // A clamped partial shift leaves a leading gap when the row fills up.
    // Center only while the entire group fits at the actual window midpoint.
    if (target < left || target + groupWidth > right) return 0;
    return target - tabX;
}
static LayoutFn g_regionLayoutOriginal;
static void RegionLayoutHook(void* self) {
    g_regionLayoutOriginal(self);
    if (!g_centerTabs.load() || g_stopping.load()) return;
    void* strip = ReadField<void*>(self, 0x5d8);
    void* newTab = ReadField<void*>(self, 0x5f0);
    if (!strip || !g_getVisible(strip)) return;
    auto rect = ReadField<ViewRect>(strip, 0x1c8);
    // Compressed/overflowing tabs must retain the native full-width layout.
    if (g_getPreferredSize) {
        GfxSizeOpaque preferred{}; alignas(16) unsigned char unlimited[16]{};
        g_getPreferredSize(strip, &preferred, unlimited);
        if (preferred.width > rect.width) return;
    }
    const int left = rect.x;
    int right = ReadField<int>(self, 0x1d0);
    int end = rect.x + rect.width;
    if (newTab && g_getVisible(newTab)) {
        auto r = ReadField<ViewRect>(newTab, 0x1c8); end = std::max(end, r.x + r.width);
    }
    auto** children = ReadField<void**>(self, 0x1a0);
    const size_t count = ReadField<size_t>(self, 0x1a8);
    if (!children || count > 1000) return;
    for (size_t i = 0; i < count; ++i) {
        void* child = children[i];
        if (child == strip || child == newTab || !g_getVisible(child) || ClassIs(child, "FrameGrabHandle")) continue;
        auto r = ReadField<ViewRect>(child, 0x1c8);
        if (r.width > 0 && r.x >= end) right = std::min(right, r.x);
    }
    // Convert the full root-view midpoint into this region's coordinates.
    // Caption controls and the new-tab button must not bias the tabs' midpoint.
    const void* root = self;
    int regionX = 0;
    for (int depth = 0; depth < 20; ++depth) {
        const void* parent = ReadField<const void*>(root, 0x198);
        if (!parent) break;
        regionX += ReadField<int>(root, 0x1c8);
        root = parent;
    }
    const int shift = WindowCenteredTabShift(ReadField<int>(root, 0x1d0), regionX,
                                            left, right, rect.x, rect.width, end);
    if (!shift) return;
    rect.x += shift; g_setBounds(strip, &rect);
    if (newTab && g_getVisible(newTab)) {
        auto r = ReadField<ViewRect>(newTab, 0x1c8); r.x += shift; g_setBounds(newTab, &r);
    }
    // The trailing drag surface represents empty space, not a fixed control.
    // Move its hit region too, otherwise it would overlap the centered tabs.
    for (size_t i = 0; i < count; ++i) {
        if (!ClassIs(children[i], "FrameGrabHandle")) continue;
        auto r = ReadField<ViewRect>(children[i], 0x1c8);
        r.x = end + shift; r.width = std::max(0, right - r.x);
        g_setBounds(children[i], &r);
    }
}
static thread_local bool g_centeringTabStrip = false;
static void TabStripLayoutHook(void* self) {
    if (g_centeringTabStrip) return;
    g_centeringTabStrip = true;
    g_tabStripLayoutOriginal(self);
    { std::lock_guard<std::mutex> lock(g_tabStripsMutex); g_tabStrips[self] = GetCurrentThreadId(); }
    g_centeringTabStrip = false;
}
static void TabStripDtorHook(void* self) {
    { std::lock_guard<std::mutex> lock(g_tabStripsMutex); g_tabStrips.erase(self); }
    g_tabStripDtorOriginal(self);
}
static void WINAPI RefreshBrowserLayoutOnThread(void*) {
    const DWORD thread = GetCurrentThreadId();
    std::vector<void*> bars, strips, buttons;
    { std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);
      for (auto [view, owner] : g_bookmarkBars) if (owner == thread) bars.push_back(view); }
    { std::lock_guard<std::mutex> lock(g_tabStripsMutex);
      for (auto [view, owner] : g_tabStrips) if (owner == thread) strips.push_back(view); }
    { std::lock_guard<std::mutex> lock(g_bookmarkButtonsMutex);
      for (auto [view, owner] : g_bookmarkButtons) if (owner == thread) buttons.push_back(view); }
    std::vector<void*> titleParents;
    { std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
      for (auto& [label, info] : g_tabTitles)
        if (info.threadId == thread) {
            void* parent = ReadField<void*>(label, 0x198);
            if (parent) titleParents.push_back(parent);
        } }
    for (void* parent : titleParents) ApplyActiveTitleStyle(parent);
    for (auto* button : buttons) {
        if (g_favoriteUpdateImage) g_favoriteUpdateImage(button);
        g_ViewPreferredSizeChanged(button);
    }
    for (auto* bar : bars) FavoritesLayoutHook(bar);
    for (auto* strip : strips) {
        TabStripLayoutHook(strip);
        void* parent = ReadField<void*>(strip, 0x198);
        if (parent && ClassIs(parent, "EdgeTabStripRegionView")) {
            g_ViewInvalidateLayout(parent, false);
            g_layoutView(parent);
        }
    }
    for (auto* bar : bars) {
        void* root = bar;
        for (int i = 0; i < 20; ++i) {
            void* parent = ReadField<void*>(root, 0x198); if (!parent) break; root = parent;
        }
        HideToolbarDescendants(root);
        g_ViewInvalidateLayout(root, false);
    }
}
static void RefreshBrowserLayout() {
    if (g_isWebview || !g_hooksActivated.load()) return;
    std::vector<DWORD> threads;
    { std::lock_guard<std::mutex> lock(g_bookmarkBarsMutex);
      for (auto [view, owner] : g_bookmarkBars) threads.push_back(owner); }
    { std::lock_guard<std::mutex> lock(g_tabStripsMutex);
      for (auto [view, owner] : g_tabStrips) threads.push_back(owner); }
    std::sort(threads.begin(), threads.end());
    threads.erase(std::unique(threads.begin(), threads.end()), threads.end());
    for (DWORD thread : threads) {
        if (thread == GetCurrentThreadId()) RefreshBrowserLayoutOnThread(nullptr);
        else if (HWND window = FindWindowForThread(thread)) RunFromWindowThread(window, RefreshBrowserLayoutOnThread, nullptr);
    }
}
static int ReadOptionalInt(const wchar_t* key, int minimum, int maximum) {
    const int value = Wh_GetIntSetting(key);
    return value == -1 ? -1 : std::clamp(value, minimum, maximum);
}
static void LoadSettings() {
    LoadBrowserStyle();
    g_hideContentBorder = Wh_GetIntSetting(L"content.hide_border") != 0;
    g_squareContentCorners = Wh_GetIntSetting(L"content.square_corners") != 0;
    const auto activeWeight = ReadTextSetting(L"tabs.appearance.active_title_weight");
    const int weight = activeWeight == L"light" ? 300 : activeWeight == L"semilight" ? 350 :
        activeWeight == L"semibold" ? 600 : activeWeight == L"bold" ? 700 : 400;
    g_activeTitleStyle = (weight << 2) |
        (Wh_GetIntSetting(L"tabs.appearance.active_title_italic") ? 1 : 0) |
        (Wh_GetIntSetting(L"tabs.appearance.active_title_underline") ? 2 : 0);
    { std::lock_guard<std::mutex> lock(g_tabObjectsMutex);
      for (auto& [label, info] : g_tabTitles) info.appliedStyle = -1; }
    g_bookmarkFontSize = ReadOptionalInt(L"favorites.appearance.font_size", 12, 48);
    g_useWindowsFolderIcon = Wh_GetIntSetting(L"favorites.appearance.windows_folder_icon") != 0;
    g_hideFavoriteEditorOnAdd = Wh_GetIntSetting(L"favorites.behavior.hide_editor_on_add") != 0;
    g_addressBarFontSize = ReadOptionalInt(L"address_bar.font_size", 12, 48);
    g_addressRadius = ReadOptionalInt(L"address_bar.appearance.corner_radius", 0, 32);
    g_addressHideFocusOutline = Wh_GetIntSetting(L"address_bar.appearance.hide_focus_outline") != 0;
    g_addressHideBorder = Wh_GetIntSetting(L"address_bar.appearance.hide_border") != 0;
    g_addressBorderColor = ParseAddressColor(ReadTextSetting(L"address_bar.appearance.border_color"));
    g_addressBackgroundColor = ParseAddressColor(ReadTextSetting(L"address_bar.appearance.background_color"));
    g_addressGlyphSize = ReadOptionalInt(L"address_bar.icons.size", 8, 48);
    g_tabFontSize = ReadOptionalInt(L"tabs.appearance.font_size", 12, 48);
    const auto closeMode = ReadTextSetting(L"tabs.layout.close_buttons");
    g_closeButtonMode = closeMode == L"hide" ? 2 : closeMode == L"hover" ? 1 : 0;
    g_tabCloseButtonsHidden = g_closeButtonMode.load() == 2;
    g_hideExtensions = Wh_GetIntSetting(L"toolbar.hide_extensions") != 0;
    g_hideProfile = Wh_GetIntSetting(L"toolbar.hide_profile") != 0;
    g_tabPreTitlePadding = ReadOptionalInt(L"tabs.layout.icon_title_spacing", 0, 32);
    g_extensionButtonWidth = ReadOptionalInt(L"extensions.button_width", 20, 64);
    g_menuFontSize = ReadOptionalInt(L"menus.text.font_size", 12, 48);
    g_hideMenuShadows = Wh_GetIntSetting(L"menus.shadows.hide_shadows") != 0;
    for (auto& field : g_menuFields) field.value = Wh_GetIntSetting(field.key);
}
static void ApplySettings() {
    RefreshOuterFrames();
    if (!g_hooksActivated.load()) return;
    ApplyFontToExistingBookmarkLabels();
    ApplyFontToExistingAddressBars();
    ApplyFolderIconToExistingBookmarkBars();
    ApplyTweaksToExistingTabs();
    ApplyWidthToExistingExtensionButtons();
    RefreshBrowserLayout();
}
static void WINAPI RestoreMenuConfig(void*) {
    if (g_configOriginal) MenuConfigHook();
}
static void Stop() {
    RefreshOuterFrames();
    g_bookmarkFontSize = -1;
    g_useWindowsFolderIcon = false;
    g_addressBarFontSize = -1;
    g_menuFontSize = -1;
    g_tabFontSize = -1;
    g_tabCloseButtonsHidden = false;
    g_tabPreTitlePadding = -1;
    g_extensionButtonWidth = -1;
    g_hideFavoriteEditorOnAdd = false;
    if (!g_hooksActivated.load()) return;
    RefreshBrowserLayout();
    ApplyFontToExistingBookmarkLabels(true);
    ApplyFontToExistingAddressBars(true);
    ApplyFolderIconToExistingBookmarkBars();
    ApplyTweaksToExistingTabs(true);
    ApplyWidthToExistingExtensionButtons();
    DestroyWindowsFolderImageOnOwningThread();
    DWORD thread;
    { std::lock_guard<std::mutex> lock(g_menuConfigMutex); thread = g_menuThread; }
    if (!thread) return;
    if (thread == GetCurrentThreadId()) {
        RestoreMenuConfig(nullptr);
        return;
    }
    if (HWND window = FindWindowForThread(thread);
        window && RunFromWindowThread(window, RestoreMenuConfig, nullptr)) return;
    // The last UI window may already have closed. These are plain scalar
    // fields in Edge's process-lifetime singleton; no UI method is needed.
    std::lock_guard<std::mutex> lock(g_menuConfigMutex);
    if (g_menuConfig) {
        for (const auto& field : g_menuFields)
            WriteMenuField(g_menuConfig, field, field.original);
    }
}

struct Binding {
    DWORD rva;
    unsigned char code[16];
    void** original;
    void* replacement;
    bool browserOnly;
};
static const Binding kBindings48[] = {
    {0x270262a, {0x48,0x8b,0x89,0x88,0x05,0x00,0x00,0x48,0x85,0xc9,0x0f,0x85,0x26,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_contentCornerVisibleOriginal), reinterpret_cast<void*>(ContentCornerVisibleHook), true},
    {0x2fec7c4, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x28,0x48,0x83,0xb9,0x88,0x05,0x00,0x00}, reinterpret_cast<void**>(&g_contentCornerEnsureOriginal), reinterpret_cast<void*>(ContentCornerEnsureHook), true},
    {0xaff9470, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x53,0x48,0x81,0xec,0xb8,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_roundedPaintOriginal), reinterpret_cast<void*>(RoundedPaintHook), true},
    {0x2c6c508, {0x56,0x57,0x53,0x48,0x83,0xec,0x50,0x0f,0x29,0x74,0x24,0x40,0x4c,0x89,0xce,0x0f}, reinterpret_cast<void**>(&g_drawRoundRectOriginal), reinterpret_cast<void*>(DrawRoundRectHook), true},
    {0x3356f40, {0x56,0x57,0x53,0x44,0x8b,0x41,0x10,0x43,0x8d,0x04,0x00,0xc1,0xf8,0x1f,0x41,0xba}, reinterpret_cast<void**>(&g_roundedInsetsOriginal), reinterpret_cast<void*>(RoundedInsetsHook), true},
    {0x2916ede, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x38}, reinterpret_cast<void**>(&g_contentMarginOriginal), reinterpret_cast<void*>(ContentMarginHook), true},
    {0xc693e7a, {0x56,0x57,0x48,0x83,0xec,0x38,0x48,0x89,0xd7,0x48,0x89,0xce,0x48,0x8b,0x05,0xb3}, reinterpret_cast<void**>(&g_contentRadiiOriginal), reinterpret_cast<void*>(ContentRadiiHook), true},
    {0xc4e0fd0, {0x48,0x83,0xec,0x28,0x48,0x8b,0x89,0x68,0x05,0x00,0x00,0x48,0x8b,0x01,0x48,0x8b}, reinterpret_cast<void**>(&g_tabAnchorPositionOriginal), reinterpret_cast<void*>(TabAnchorPositionHook), true},
    {0x3333940, {0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xb0,0x00,0x00,0x00,0x0f,0x29,0xb4}, reinterpret_cast<void**>(&g_contentBorderPaintOriginal), reinterpret_cast<void*>(ContentBorderPaintHook), true},
    {0x1040190, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x50,0x48,0x89,0xce,0x48,0x8b}, reinterpret_cast<void**>(&g_tabIsActive), nullptr, true},
    {0x2619a5c, {0x56,0x48,0x83,0xec,0x20,0x44,0x89,0xc8,0x48,0x89,0xd6,0x44,0x8b,0x4c,0x24,0x50}, reinterpret_cast<void**>(&g_fontDerive), nullptr, true},
    {0x2618516, {0x48,0x8b,0x09,0xe9,0x84,0x88,0xa3,0xfe,0x48,0x8b,0x09,0xe9,0x9c,0x88,0xa3,0xfe}, reinterpret_cast<void**>(&g_fontStyle), nullptr, true},
    {0x261851e, {0x48,0x8b,0x09,0xe9,0x9c,0x88,0xa3,0xfe,0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce}, reinterpret_cast<void**>(&g_fontWeight), nullptr, true},
    {0x3000b10, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x80,0xb9,0x28,0x09,0x00,0x00,0x00,0x75}, reinterpret_cast<void**>(&g_activeChangedOriginal), reinterpret_cast<void*>(ActiveChangedHook), true},
    {0xdc27ca0, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x48,0x89,0xce,0x48,0x8b,0x05,0x8f,0x43,0x0c}, reinterpret_cast<void**>(&g_tooltipThemeOriginal), reinterpret_cast<void*>(TooltipThemeHook), true},
    {0x29280a2, {0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xce,0xf3,0x41,0x0f,0x2a,0xc1,0xf3}, reinterpret_cast<void**>(&g_roundedBackground), nullptr, true},
    {0x2aefed7, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x50,0x0f,0x29}, reinterpret_cast<void**>(&g_roundedBorder), nullptr, true},
    {0x10a86bc, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xd6,0x48,0x89,0xcf,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_setBackground), nullptr, true},
    {0x10df600, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xd6,0x48,0x89}, reinterpret_cast<void**>(&g_setBorder), nullptr, true},
    {0x10235e0, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x58}, reinterpret_cast<void**>(&g_viewLayoutOriginal), reinterpret_cast<void*>(ViewLayoutHook), true},
    {0x104bd7a, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xd6,0x48,0x8b,0x81,0x00,0x01,0x00,0x00,0x48}, reinterpret_cast<void**>(&g_viewClassName), nullptr, true},
    {0x27cf804, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xce,0x48,0x8b,0x05,0x2b,0xc8,0x51}, reinterpret_cast<void**>(&g_isMouseHovered), nullptr, true},
    {0x28442c0, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x05,0x72,0x7d,0x4a,0x11,0x48,0x31}, reinterpret_cast<void**>(&g_toolbarLayoutOriginal), reinterpret_cast<void*>(ToolbarLayoutHook), true},
    {0x104f4ee, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x68}, reinterpret_cast<void**>(&g_viewDtorOriginal), reinterpret_cast<void*>(ViewDtorHook), true},
    {0xa372520, {0x48,0x83,0xc1,0xe8,0xe9,0x6b,0xff,0xff,0xff,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_tabEnterOriginal), reinterpret_cast<void*>(TabEnterHook), true},
    {0xa372530, {0x56,0x57,0x48,0x83,0xec,0x28,0x80,0xb9,0xa6,0x09,0x00,0x00,0x01,0x74,0x07,0x48}, reinterpret_cast<void**>(&g_tabExitOriginal), reinterpret_cast<void*>(TabExitHook), true},
    {0xfd9ce0, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x50,0x4c,0x89}, reinterpret_cast<void**>(&g_labelPreferredOriginal), reinterpret_cast<void*>(LabelPreferredHook), true},
    {0x2ac7540, {0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x05,0xef,0x4a,0x22}, reinterpret_cast<void**>(&g_regionLayoutOriginal), reinterpret_cast<void*>(RegionLayoutHook), true},
    {0xffc87a, {0x48,0x8b,0x89,0xb0,0x03,0x00,0x00,0x48,0x8d,0x81,0x08,0xfe,0xff,0xff,0x48,0x85}, reinterpret_cast<void**>(&g_frameBrowserView), nullptr, true},
    {0xfca8c0, {0x48,0x8b,0x81,0x90,0x01,0x00,0x00,0xc3,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_styleGetWidget), nullptr, true},
    {0x100bf40, {0x48,0x83,0xec,0x28,0x48,0x8b,0x01,0x48,0x8b,0x40,0x58,0xff,0x15,0xd7,0x8b,0x93}, reinterpret_cast<void**>(&g_styleGetColorProvider), nullptr, true},
    {0xc6977e0, {0x48,0x83,0xec,0x28,0x48,0x8b,0x81,0xd0,0x03,0x00,0x00,0x48,0x8b,0x80,0x00,0x06}, reinterpret_cast<void**>(&g_styleUsingDefaultTheme), nullptr, true},
    {0xfcd1a2, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x89,0x00,0x06,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_styleVerticalTabs), nullptr, true},
    {0xfce667, {0x48,0x85,0xc9,0x74,0x0d,0x80,0xb9,0x58,0x01,0x00,0x00,0x01,0x0f,0x84,0x45,0xbf}, reinterpret_cast<void**>(&g_styleWorkspace), nullptr, true},
    {0x100e2ea, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x53,0x48,0x83,0xec,0x28,0x89,0xd7,0x48}, reinterpret_cast<void**>(&g_styleGetColor), nullptr, true},
    {0x27d91a0, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x89,0xd3,0x48,0x89,0xcf,0x48,0x8b,0x05,0x8d}, reinterpret_cast<void**>(&g_frameColorOriginal), reinterpret_cast<void*>(FrameColorHook), true},
    {0x27c7110, {0x41,0x57,0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x48,0x44,0x89,0xc3,0x89}, reinterpret_cast<void**>(&g_tabBackgroundOriginal), reinterpret_cast<void*>(TabBackgroundHook), true},
    {0x318ea84, {0x56,0x48,0x83,0xec,0x30,0x48,0x8b,0x05,0xb0,0xd5,0xb5,0x10,0x48,0x31,0xe0,0x48}, reinterpret_cast<void**>(&g_popupShadowsOriginal), reinterpret_cast<void*>(PopupShadowsHook), true},
    {0x50c8eae, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x48,0x89,0xcf,0x48,0x8b,0x05,0x81,0x31,0xc2}, reinterpret_cast<void**>(&g_popupRadiusOriginal), reinterpret_cast<void*>(PopupRadiusHook), true},
    {0x26a2b63, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x60,0x4c,0x89}, reinterpret_cast<void**>(&g_matchUiFontOriginal), reinterpret_cast<void*>(MatchUiFontHook), true},
    {0x104d9ae, {0x56,0x57,0x48,0x83,0xec,0x28,0x48,0x89,0xce,0x31,0xff,0x48,0x89,0x39,0xc7,0x41}, reinterpret_cast<void**>(&g_emptyImageModel), nullptr, true},
    {0xfce550, {0x48,0x89,0xc8,0x48,0xc7,0x01,0x00,0x00,0x00,0x00,0xc3,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_emptyImageSkia), nullptr, true},
    {0x100c04e, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x0f,0xb6,0x41,0x20,0x85,0xc0,0x75,0x12}, reinterpret_cast<void**>(&g_imageModelDtor), nullptr, true},
    {0x2c800ea, {0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xd7,0x48,0x89,0xce,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_setMenuIconOriginal), reinterpret_cast<void*>(SetMenuIconHook), true},
    {0xafd348c, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x78,0x48,0x8b,0x05,0xa4,0x8b,0xd1,0x08}, reinterpret_cast<void**>(&g_shortcutTextOriginal), reinterpret_cast<void*>(ShortcutTextHook), true},
    {0x301f418, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8d,0x05,0x09,0x42,0x43,0x0e,0x48}, reinterpret_cast<void**>(&g_bookmarkButtonDtorOriginal), reinterpret_cast<void*>(BookmarkButtonDtorHook), true},
    {0x1083a70, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xb0,0x00,0x00}, reinterpret_cast<void**>(&g_buttonImageOriginal), reinterpret_cast<void*>(ButtonImageHook), true},
    {0x1083fc5, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x68}, reinterpret_cast<void**>(&g_vectorImageOriginal), reinterpret_cast<void*>(VectorImageHook), true},
    {0x2a5ab08, {0x56,0x57,0x53,0x48,0x81,0xec,0x10,0x02,0x00,0x00,0x0f,0x29,0xb4,0x24,0x00,0x02}, reinterpret_cast<void**>(&g_imageRepCtor), nullptr, true},
    {0x50a0106, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0xc7,0x01,0x00,0x00,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_imageFromRep), nullptr, true},
    {0x2853746, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x81,0xc1,0xc8,0x01,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_imageRepDtor), nullptr, true},
    {0x27a14ea, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x48,0x8b}, reinterpret_cast<void**>(&g_paintVectorOriginal), reinterpret_cast<void*>(PaintVectorHook), true},
    {0x29c2ac4, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0x80,0x00,0x00,0x00,0x44,0x89}, reinterpret_cast<void**>(&g_drawImage), nullptr, true},
    {0x2aeefa0, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x18}, reinterpret_cast<void**>(&g_favoritesLayoutOriginal), reinterpret_cast<void*>(FavoritesLayoutHook), true},
    {0x32f62f0, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x05,0x42,0x5d,0x9f,0x10,0x48,0x31}, reinterpret_cast<void**>(&g_tabStripLayoutOriginal), reinterpret_cast<void*>(TabStripLayoutHook), true},
    {0x3187072, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x28,0x48,0x89,0xce,0x48,0x8d,0x05,0x6b}, reinterpret_cast<void**>(&g_tabStripDtorOriginal), reinterpret_cast<void*>(TabStripDtorHook), true},
    {0x1006e5e, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xb8}, reinterpret_cast<void**>(&g_setBounds), nullptr, true},
    {0xfe90d6, {0x8a,0x81,0xd8,0x01,0x00,0x00,0xc3,0xcc,0xe9,0x61,0xa7,0x03,0x00,0xcc,0x56,0x57}, reinterpret_cast<void**>(&g_getVisible), nullptr, true},
    {0xfd9ea0, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xd6,0x80,0xb9,0xc4,0x01,0x00,0x00,0x01,0x74}, reinterpret_cast<void**>(&g_getPreferredSize), nullptr, true},
    {0xfe90de, {0xe9,0x61,0xa7,0x03,0x00,0xcc,0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x44,0x89,0xc3}, reinterpret_cast<void**>(&g_layoutView), nullptr, true},
    {0xfcccae, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x0a,0x48,0x89,0x0e,0x48,0x85}, reinterpret_cast<void**>(&g_FontListCopyCtor), nullptr, false},
    {0x261bd4c, {0x48,0x8b,0x09,0xe9,0xd0,0x4e,0xa3,0xfe,0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57}, reinterpret_cast<void**>(&g_FontListGetFontSize), nullptr, false},
    {0x1050d52, {0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x28,0x44,0x89,0xc7,0x48,0x89,0xd6,0x48,0x89}, reinterpret_cast<void**>(&g_FontListDeriveWithSizeDelta), nullptr, false},
    {0x10d4ae2, {0x48,0x8b,0x09,0x48,0x85,0xc9,0x0f,0x85,0xea,0x2f,0x24,0x04,0xc3,0xcc,0x56,0x48}, reinterpret_cast<void**>(&g_FontListDtor), nullptr, false},
    {0xafc2634, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xd6,0x48,0x89,0xcf,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_MenuItemGetFontListOriginal), reinterpret_cast<void*>(MenuItemGetFontListHook), false},
    {0x30b9440, {0x48,0x83,0xec,0x28,0x8b,0x05,0x1e,0xd9,0xe7,0x10,0x8b,0x0d,0xc0,0x1a,0xe0,0x10}, reinterpret_cast<void**>(&g_configOriginal), reinterpret_cast<void*>(MenuConfigHook), false},
    {0xc8249d0, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x53,0x48,0x81,0xec,0x80,0x00}, reinterpret_cast<void**>(&g_createBubbleBorderOriginal), reinterpret_cast<void*>(CreateBubbleBorderHook), false},
    {0x5242ade, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x44,0x89,0xc7,0x89,0xd3,0x48,0x89,0xce,0x48}, reinterpret_cast<void**>(&g_borderCtorOriginal), reinterpret_cast<void*>(BubbleBorderCtorHook), false},
    {0xc82298e, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x68}, reinterpret_cast<void**>(&g_menuHostOriginal), reinterpret_cast<void*>(MenuHostHook), false},
    {0x2b53136, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x68}, reinterpret_cast<void**>(&g_widgetInitOriginal), reinterpret_cast<void*>(WidgetInitHook), false},
    {0x75b2c30, {0x48,0x8b,0x81,0x48,0x03,0x00,0x00,0xc3,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_submenuGetItem), nullptr, false},
    {0x2d77ea4, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x88}, reinterpret_cast<void**>(&g_BookmarkButtonBaseCtorOriginal), reinterpret_cast<void*>(BookmarkButtonBaseCtorHook), true},
    {0x2d76f02, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x88}, reinterpret_cast<void**>(&g_BookmarkMenuButtonBaseCtorOriginal), reinterpret_cast<void*>(BookmarkMenuButtonBaseCtorHook), true},
    {0x2617a42, {0x56,0x57,0x53,0x48,0x83,0xec,0x50,0x48,0x89,0xce,0x48,0x8b,0x05,0xed,0x45,0x6d}, reinterpret_cast<void**>(&g_LabelButtonLabelCtorOriginal), reinterpret_cast<void*>(LabelButtonLabelCtorHook), true},
    {0x25f5d70, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_LabelButtonLabelDeletingDtorOriginal), reinterpret_cast<void*>(LabelButtonLabelDeletingDtorHook), true},
    {0x1048fd8, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x58,0x44,0x89,0xc6,0x89,0xd7,0x48,0x89}, reinterpret_cast<void**>(&g_TypographyGetFontOriginal), reinterpret_cast<void*>(TypographyGetFontHook), true},
    {0x2cb48b0, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x89,0x98,0x03,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_LabelSetFontList), nullptr, true},
    {0x2a23f36, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x4c,0x89}, reinterpret_cast<void**>(&g_GetBookmarkFolderIconOriginal), reinterpret_cast<void*>(GetBookmarkFolderIconHook), true},
    {0xb354430, {0x56,0x57,0x48,0x83,0xec,0x78,0x48,0x89,0xce,0x48,0x8b,0x05,0x00,0x7c,0x99,0x08}, reinterpret_cast<void**>(&g_CreateSkBitmapFromHICON), nullptr, true},
    {0x2929750, {0x56,0x57,0x48,0x81,0xec,0x28,0x02,0x00,0x00,0x48,0x89,0xce,0x48,0x8b,0x05,0xdd}, reinterpret_cast<void**>(&g_ImageSkiaCreateFrom1xBitmap), nullptr, true},
    {0x11d6e30, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x49,0x18,0x48,0x85,0xc9,0x74}, reinterpret_cast<void**>(&g_SkBitmapDtor), nullptr, true},
    {0x1080636, {0x48,0x8b,0x09,0x48,0x85,0xc9,0x74,0x06,0xf0,0xff,0x49,0x08,0x74,0x01,0xc3,0xba}, reinterpret_cast<void**>(&g_ImageSkiaDtor), nullptr, true},
    {0xff1b86, {0x56,0x57,0x48,0x83,0xec,0x38,0x48,0x89,0xce,0x48,0x8b,0x05,0xaa,0xa4,0xcf,0x12}, reinterpret_cast<void**>(&g_ImageModelFromImageSkia), nullptr, true},
    {0x2a238f0, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x50,0x01,0x00}, reinterpret_cast<void**>(&g_BookmarkBarViewUpdateAppearanceForThemeOriginal), reinterpret_cast<void*>(BookmarkBarViewUpdateAppearanceForThemeHook), true},
    {0x31a1c60, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_BookmarkBarViewDeletingDtorOriginal), reinterpret_cast<void*>(BookmarkBarViewDeletingDtorHook), true},
    {0x2fd066e, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xe8}, reinterpret_cast<void**>(&g_OmniboxViewViewsCtorOriginal), reinterpret_cast<void*>(OmniboxViewViewsCtorHook), true},
    {0x2fe5f70, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_OmniboxViewViewsDtorOriginal), reinterpret_cast<void*>(OmniboxViewViewsDtorHook), true},
    {0x2fd13e0, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x78}, reinterpret_cast<void**>(&g_TextfieldCtorOriginal), reinterpret_cast<void*>(TextfieldCtorHook), true},
    {0x2fd0f82, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x81,0xc0,0x03,0x00,0x00,0x48}, reinterpret_cast<void**>(&g_TextfieldSetFontListOriginal), nullptr, true},
    {0x292ea00, {0x56,0x57,0x48,0x83,0xec,0x68,0x48,0x89,0xce,0x48,0x8b,0x05,0x30,0xd6,0x3b,0x11}, reinterpret_cast<void**>(&g_TabTitleCtorOriginal), reinterpret_cast<void*>(TabTitleCtorHook), true},
    {0x32e0910, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x6c,0x55,0x31,0xff}, reinterpret_cast<void**>(&g_TabTitleDtorOriginal), reinterpret_cast<void*>(TabTitleDtorHook), true},
    {0x292f77a, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x53,0x48,0x81,0xec,0xc0,0x00}, reinterpret_cast<void**>(&g_TabCloseButtonCtorOriginal), reinterpret_cast<void*>(TabCloseButtonCtorHook), true},
    {0x304da7e, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8d,0x05,0x63,0x18,0x3b,0x0e,0x48}, reinterpret_cast<void**>(&g_TabCloseButtonDtorOriginal), reinterpret_cast<void*>(TabCloseButtonDtorHook), true},
    {0x10213a0, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x48,0x89,0xd3,0x48,0x89,0xce,0x48,0x8b}, reinterpret_cast<void**>(&g_ViewSetVisibleOriginal), reinterpret_cast<void*>(ViewSetVisibleHook), true},
    {0xfe9b40, {0x41,0x57,0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x38,0x48,0x8b,0x05,0xed}, reinterpret_cast<void**>(&g_ViewInvalidateLayout), nullptr, true},
    {0xff2590, {0x56,0x48,0x83,0xec,0x30,0x48,0x89,0xce,0x48,0x8b,0x05,0xa1,0x9a,0xcf,0x12,0x48}, reinterpret_cast<void**>(&g_ViewPreferredSizeChanged), nullptr, true},
    {0xfe0d0b, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x89,0xcb,0x48,0x8b,0x05,0x25,0xb3,0xd0,0x12}, reinterpret_cast<void**>(&g_GetLayoutConstantOriginal), reinterpret_cast<void*>(GetLayoutConstantHook), true},
    {0xa0bb372, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x78}, reinterpret_cast<void**>(&g_ToolbarActionViewCtorOriginal), reinterpret_cast<void*>(ToolbarActionViewCtorHook), true},
    {0xa0bbef0, {0x48,0x8b,0x89,0x08,0x07,0x00,0x00,0xe9,0x04,0x4b,0x85,0xf8,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_ToolbarActionViewCalculatePreferredSizeOriginal), reinterpret_cast<void*>(ToolbarActionViewCalculatePreferredSizeHook), true},
    {0xa0bc640, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0xf0,0xf4,0xff,0xff}, reinterpret_cast<void**>(&g_ToolbarActionViewDeletingDtorOriginal), reinterpret_cast<void*>(ToolbarActionViewDeletingDtorHook), true},
    {0xa0bb8b0, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0x10,0x01,0x00,0x00,0x0f,0x29}, reinterpret_cast<void**>(&g_ToolbarActionViewUpdateState), nullptr, true},
    {0x2e1c0aa, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xd8}, reinterpret_cast<void**>(&g_ExtensionsToolbarDesktopCtorOriginal), reinterpret_cast<void*>(ExtensionsToolbarDesktopCtorHook), true},
    {0x3160e90, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_ExtensionsToolbarDesktopDeletingDtorOriginal), reinterpret_cast<void*>(ExtensionsToolbarDesktopDeletingDtorHook), true},
    {0xa0c11e8, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x98}, reinterpret_cast<void**>(&g_bookmarkFlowOriginal), reinterpret_cast<void*>(BookmarkFlowHook), true},
};
static const Binding kBindings53[] = {
    {0x10df370, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x01,0x48,0x8b,0x40,0x50,0xff}, reinterpret_cast<void**>(&g_favoriteUpdateImage), nullptr, true},
    {0x30f4670, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0x80,0x00,0x00,0x00,0x0f,0x29}, reinterpret_cast<void**>(&g_favoriteImagePaintOriginal), reinterpret_cast<void*>(FavoriteImagePaintHook), true},
    {0x29d6fe0, {0x56,0x57,0x53,0x48,0x81,0xec,0x60,0x08,0x00,0x00,0x48,0x89,0xd7,0x48,0x89,0xce}, reinterpret_cast<void**>(&g_favoriteImageLayerPaintOriginal), reinterpret_cast<void*>(FavoriteImageLayerPaintHook), true},
    {0x2c88dd0, {0x56,0x57,0x48,0x83,0xec,0x38,0x48,0x89,0xce,0x48,0x8b,0x05,0x60,0x52,0x09,0x11}, reinterpret_cast<void**>(&g_centerGlyphRectOriginal), reinterpret_cast<void*>(CenterGlyphRectHook), true},
    {0x2c88c1e, {0xe8,0xad,0x01,0x00,0x00,0x45,0x0f,0x29,0x4e,0x30,0x45,0x0f,0x29,0x4e,0x20,0x45}, reinterpret_cast<void**>(&g_captionSizeCallGuard), nullptr, true},
    {0x29d7508, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xa0,0x02,0x00}, reinterpret_cast<void**>(&g_addressGetImage), nullptr, true},
    {0x28473c0, {0x56,0x57,0x48,0x83,0xec,0x58,0x0f,0x29,0x74,0x24,0x40,0x0f,0x28,0xf1,0x48,0x89}, reinterpret_cast<void**>(&g_addressGetRep), nullptr, true},
    {0x267dcac, {0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0xf8,0x06,0x00,0x00,0x48,0x89,0xce,0x48}, reinterpret_cast<void**>(&g_addressGetBitmap), nullptr, true},
    {0x3107d70, {0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xd6,0x48,0x89,0xcf,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_focusRingPaintOriginal), reinterpret_cast<void*>(AddressFocusPaintHook), true},
    {0x2c890d0, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0xe0,0x01,0x00,0x00,0x4c,0x89}, reinterpret_cast<void**>(&g_captionCloseOriginal), reinterpret_cast<void*>(CaptionCloseHook), true},
    {0x2c88ed0, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x38,0x4c,0x89,0xce,0x4c,0x89,0xc7,0x48}, reinterpret_cast<void**>(&g_captionMinOriginal), reinterpret_cast<void*>(CaptionMinHook), true},
    {0xa29e450, {0x41,0x57,0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x38,0x01,0x00,0x00,0x0f}, reinterpret_cast<void**>(&g_captionRestoreOriginal), reinterpret_cast<void*>(CaptionRestoreHook), true},
    {0x2c88f90, {0x56,0x57,0x48,0x83,0xec,0x68,0x0f,0x29,0x74,0x24,0x50,0x0f,0x28,0xf3,0x4c,0x89}, reinterpret_cast<void**>(&g_captionMaxOriginal), reinterpret_cast<void*>(CaptionMaxHook), true},
    {0x9dff9f0, {0x56,0x57,0x53,0x48,0x83,0xec,0x70,0x4c,0x89,0xc3,0x48,0x89,0xd6,0x48,0x89,0xcf}, reinterpret_cast<void**>(&g_addressFocusOriginal), reinterpret_cast<void*>(AddressFocusHook), true},
    {0x12ec61a, {0x56,0x48,0x81,0xec,0x80,0x00,0x00,0x00,0x44,0x0f,0x29,0x4c,0x24,0x70,0x44,0x0f}, reinterpret_cast<void**>(&g_addressRRectCtor), nullptr, true},
    {0x29b4d16, {0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xce,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_addressRadiusOriginal), reinterpret_cast<void*>(AddressRadiusHook), true},
    {0x29b4bfc, {0x41,0x57,0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x68,0x44,0x89,0xcb,0x44}, reinterpret_cast<void**>(&g_addressBackgroundOriginal), reinterpret_cast<void*>(AddressBackgroundHook), true},
    {0x101b22c, {0x48,0x89,0xc8,0x48,0x8b,0x0a,0x48,0x89,0x08,0x48,0x85,0xc9,0x74,0x16,0xba,0x01}, reinterpret_cast<void**>(&g_copyImageSkia), nullptr, true},
    {0x29d741e, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x48,0x89,0xd6,0x48,0x89,0xcf,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_addressImageBoundsOriginal), reinterpret_cast<void*>(AddressImageBoundsHook), true},
    {0x2d5609c, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xe8}, reinterpret_cast<void**>(&g_cornerBoundsOriginal), reinterpret_cast<void*>(CornerBoundsHook), true},
    {0x2d56d22, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xa8}, reinterpret_cast<void**>(&g_cornerImageOriginal), reinterpret_cast<void*>(CornerImageHook), true},
    {0x2702d5a, {0x48,0x8b,0x89,0x88,0x05,0x00,0x00,0x48,0x85,0xc9,0x0f,0x85,0x26,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_contentCornerVisibleOriginal), reinterpret_cast<void*>(ContentCornerVisibleHook), true},
    {0x3000a54, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x28,0x48,0x83,0xb9,0x88,0x05,0x00,0x00}, reinterpret_cast<void**>(&g_contentCornerEnsureOriginal), reinterpret_cast<void*>(ContentCornerEnsureHook), true},
    {0xb026eb0, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x53,0x48,0x81,0xec,0xb8,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_roundedPaintOriginal), reinterpret_cast<void*>(RoundedPaintHook), true},
    {0x2c89048, {0x56,0x57,0x53,0x48,0x83,0xec,0x50,0x0f,0x29,0x74,0x24,0x40,0x4c,0x89,0xce,0x0f}, reinterpret_cast<void**>(&g_drawRoundRectOriginal), reinterpret_cast<void*>(DrawRoundRectHook), true},
    {0x3397920, {0x56,0x57,0x53,0x44,0x8b,0x41,0x10,0x43,0x8d,0x04,0x00,0xc1,0xf8,0x1f,0x41,0xba}, reinterpret_cast<void**>(&g_roundedInsetsOriginal), reinterpret_cast<void*>(RoundedInsetsHook), true},
    {0x292bf5e, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x38}, reinterpret_cast<void**>(&g_contentMarginOriginal), reinterpret_cast<void*>(ContentMarginHook), true},
    {0xc6c0dda, {0x56,0x57,0x48,0x83,0xec,0x38,0x48,0x89,0xd7,0x48,0x89,0xce,0x48,0x8b,0x05,0x53}, reinterpret_cast<void**>(&g_contentRadiiOriginal), reinterpret_cast<void*>(ContentRadiiHook), true},
    {0xc50e5e0, {0x48,0x83,0xec,0x28,0x48,0x8b,0x89,0x68,0x05,0x00,0x00,0x48,0x8b,0x01,0x48,0x8b}, reinterpret_cast<void**>(&g_tabAnchorPositionOriginal), reinterpret_cast<void*>(TabAnchorPositionHook), true},
    {0x3367ba0, {0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xb0,0x00,0x00,0x00,0x0f,0x29,0xb4}, reinterpret_cast<void**>(&g_contentBorderPaintOriginal), reinterpret_cast<void*>(ContentBorderPaintHook), true},
    {0x1069c60, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x50,0x48,0x89,0xce,0x48,0x8b}, reinterpret_cast<void**>(&g_tabIsActive), nullptr, true},
    {0x263f2fc, {0x56,0x48,0x83,0xec,0x20,0x44,0x89,0xc8,0x48,0x89,0xd6,0x44,0x8b,0x4c,0x24,0x50}, reinterpret_cast<void**>(&g_fontDerive), nullptr, true},
    {0x263ddb6, {0x48,0x8b,0x09,0xe9,0x24,0xb0,0x9f,0xfe,0x48,0x8b,0x09,0xe9,0x3c,0xb0,0x9f,0xfe}, reinterpret_cast<void**>(&g_fontStyle), nullptr, true},
    {0x263ddbe, {0x48,0x8b,0x09,0xe9,0x3c,0xb0,0x9f,0xfe,0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce}, reinterpret_cast<void**>(&g_fontWeight), nullptr, true},
    {0x3015010, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x80,0xb9,0x28,0x09,0x00,0x00,0x00,0x75}, reinterpret_cast<void**>(&g_activeChangedOriginal), reinterpret_cast<void*>(ActiveChangedHook), true},
    {0xdc53980, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x48,0x89,0xce,0x48,0x8b,0x05,0xaf,0xa6,0x0c}, reinterpret_cast<void**>(&g_tooltipThemeOriginal), reinterpret_cast<void*>(TooltipThemeHook), true},
    {0x293d1b2, {0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xce,0xf3,0x41,0x0f,0x2a,0xc1,0xf3}, reinterpret_cast<void**>(&g_roundedBackground), nullptr, true},
    {0x2b13ee7, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x50,0x0f,0x29}, reinterpret_cast<void**>(&g_roundedBorder), nullptr, true},
    {0x10df50c, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xd6,0x48,0x89,0xcf,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_setBackground), nullptr, true},
    {0x110a060, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xd6,0x48,0x89}, reinterpret_cast<void**>(&g_setBorder), nullptr, true},
    {0x1053e70, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x58}, reinterpret_cast<void**>(&g_viewLayoutOriginal), reinterpret_cast<void*>(ViewLayoutHook), true},
    {0x107844a, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xd6,0x48,0x8b,0x81,0x00,0x01,0x00,0x00,0x48}, reinterpret_cast<void**>(&g_viewClassName), nullptr, true},
    {0x27ea034, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xce,0x48,0x8b,0x05,0xfb,0x3f,0x53}, reinterpret_cast<void**>(&g_isMouseHovered), nullptr, true},
    {0x2838490, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x05,0xa2,0x5b,0x4e,0x11,0x48,0x31}, reinterpret_cast<void**>(&g_toolbarLayoutOriginal), reinterpret_cast<void*>(ToolbarLayoutHook), true},
    {0x107bb80, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x58}, reinterpret_cast<void**>(&g_viewDtorOriginal), reinterpret_cast<void*>(ViewDtorHook), true},
    {0xa39ff80, {0x48,0x83,0xc1,0xe8,0xe9,0x6b,0xff,0xff,0xff,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_tabEnterOriginal), reinterpret_cast<void*>(TabEnterHook), true},
    {0xa39ff90, {0x56,0x57,0x48,0x83,0xec,0x28,0x80,0xb9,0xa6,0x09,0x00,0x00,0x01,0x74,0x07,0x48}, reinterpret_cast<void**>(&g_tabExitOriginal), reinterpret_cast<void*>(TabExitHook), true},
    {0x10051a0, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x50,0x4c,0x89}, reinterpret_cast<void**>(&g_labelPreferredOriginal), reinterpret_cast<void*>(LabelPreferredHook), true},
    {0x2aeec90, {0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x05,0x9f,0xf3,0x22}, reinterpret_cast<void**>(&g_regionLayoutOriginal), reinterpret_cast<void*>(RegionLayoutHook), true},
    {0x1025dba, {0x48,0x8b,0x89,0xb0,0x03,0x00,0x00,0x48,0x8d,0x81,0x08,0xfe,0xff,0xff,0x48,0x85}, reinterpret_cast<void**>(&g_frameBrowserView), nullptr, true},
    {0xff5a80, {0x48,0x8b,0x81,0x90,0x01,0x00,0x00,0xc3,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_styleGetWidget), nullptr, true},
    {0x103bbe0, {0x48,0x83,0xec,0x28,0x48,0x8b,0x01,0x48,0x8b,0x40,0x58,0xff,0x15,0x97,0xbe,0x93}, reinterpret_cast<void**>(&g_styleGetColorProvider), nullptr, true},
    {0xc6c4740, {0x48,0x83,0xec,0x28,0x48,0x8b,0x81,0xd0,0x03,0x00,0x00,0x48,0x8b,0x80,0x00,0x06}, reinterpret_cast<void**>(&g_styleUsingDefaultTheme), nullptr, true},
    {0xff7fe2, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x89,0x00,0x06,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_styleVerticalTabs), nullptr, true},
    {0xff94a7, {0x48,0x85,0xc9,0x74,0x0d,0x80,0xb9,0x58,0x01,0x00,0x00,0x01,0x0f,0x84,0xc2,0x31}, reinterpret_cast<void**>(&g_styleWorkspace), nullptr, true},
    {0x103df8a, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x53,0x48,0x83,0xec,0x28,0x89,0xd7,0x48}, reinterpret_cast<void**>(&g_styleGetColor), nullptr, true},
    {0x27f3ea0, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x89,0xd3,0x48,0x89,0xcf,0x48,0x8b,0x05,0x8d}, reinterpret_cast<void**>(&g_frameColorOriginal), reinterpret_cast<void*>(FrameColorHook), true},
    {0x27e2090, {0x41,0x57,0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x48,0x44,0x89,0xc3,0x89}, reinterpret_cast<void**>(&g_tabBackgroundOriginal), reinterpret_cast<void*>(TabBackgroundHook), true},
    {0x31ac4c4, {0x56,0x48,0x83,0xec,0x30,0x48,0x8b,0x05,0x70,0x1b,0xb7,0x10,0x48,0x31,0xe0,0x48}, reinterpret_cast<void**>(&g_popupShadowsOriginal), reinterpret_cast<void*>(PopupShadowsHook), true},
    {0x50efd8e, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x48,0x89,0xcf,0x48,0x8b,0x05,0xa1,0xe2,0xc2}, reinterpret_cast<void**>(&g_popupRadiusOriginal), reinterpret_cast<void*>(PopupRadiusHook), true},
    {0x26ed003, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x60,0x4c,0x89}, reinterpret_cast<void**>(&g_matchUiFontOriginal), reinterpret_cast<void*>(MatchUiFontHook), true},
    {0x1079d5e, {0x56,0x57,0x48,0x83,0xec,0x28,0x48,0x89,0xce,0x31,0xff,0x48,0x89,0x39,0xc7,0x41}, reinterpret_cast<void**>(&g_emptyImageModel), nullptr, true},
    {0xff9390, {0x48,0x89,0xc8,0x48,0xc7,0x01,0x00,0x00,0x00,0x00,0xc3,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_emptyImageSkia), nullptr, true},
    {0x103bcee, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x0f,0xb6,0x41,0x20,0x85,0xc0,0x75,0x12}, reinterpret_cast<void**>(&g_imageModelDtor), nullptr, true},
    {0x2c9751a, {0x56,0x57,0x53,0x48,0x83,0xec,0x60,0x48,0x89,0xd7,0x48,0x89,0xce,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_setMenuIconOriginal), reinterpret_cast<void*>(SetMenuIconHook), true},
    {0xb000edc, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x78,0x48,0x8b,0x05,0x54,0xd1,0xd1,0x08}, reinterpret_cast<void**>(&g_shortcutTextOriginal), reinterpret_cast<void*>(ShortcutTextHook), true},
    {0x3030698, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8d,0x05,0x19,0x15,0x45,0x0e,0x48}, reinterpret_cast<void**>(&g_bookmarkButtonDtorOriginal), reinterpret_cast<void*>(BookmarkButtonDtorHook), true},
    {0x10ae1d0, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xb0,0x00,0x00}, reinterpret_cast<void**>(&g_buttonImageOriginal), reinterpret_cast<void*>(ButtonImageHook), true},
    {0x10ae725, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x68}, reinterpret_cast<void**>(&g_vectorImageOriginal), reinterpret_cast<void*>(VectorImageHook), true},
    {0x2a7b128, {0x56,0x57,0x53,0x48,0x81,0xec,0x10,0x02,0x00,0x00,0x0f,0x29,0xb4,0x24,0x00,0x02}, reinterpret_cast<void**>(&g_imageRepCtor), nullptr, true},
    {0x50c6996, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0xc7,0x01,0x00,0x00,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_imageFromRep), nullptr, true},
    {0x2847906, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x81,0xc1,0xc8,0x01,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_imageRepDtor), nullptr, true},
    {0x27bc8aa, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x48,0x8b}, reinterpret_cast<void**>(&g_paintVectorOriginal), reinterpret_cast<void*>(PaintVectorHook), true},
    {0x29d7384, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0x80,0x00,0x00,0x00,0x44,0x89}, reinterpret_cast<void**>(&g_drawImage), nullptr, true},
    {0x2b12fb0, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x18}, reinterpret_cast<void**>(&g_favoritesLayoutOriginal), reinterpret_cast<void*>(FavoritesLayoutHook), true},
    {0x3329ae0, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x05,0x52,0x45,0x9f,0x10,0x48,0x31}, reinterpret_cast<void**>(&g_tabStripLayoutOriginal), reinterpret_cast<void*>(TabStripLayoutHook), true},
    {0x31a4a22, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x28,0x48,0x89,0xce,0x48,0x8d,0x05,0x4b}, reinterpret_cast<void**>(&g_tabStripDtorOriginal), reinterpret_cast<void*>(TabStripDtorHook), true},
    {0x10361de, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xb8}, reinterpret_cast<void**>(&g_setBounds), nullptr, true},
    {0x10125d6, {0x8a,0x81,0xd8,0x01,0x00,0x00,0xc3,0xcc,0xe9,0xf1,0x1a,0x04,0x00,0xcc,0x56,0x57}, reinterpret_cast<void**>(&g_getVisible), nullptr, true},
    {0x1005360, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xd6,0x80,0xb9,0xc4,0x01,0x00,0x00,0x01,0x74}, reinterpret_cast<void**>(&g_getPreferredSize), nullptr, true},
    {0x10125de, {0xe9,0xf1,0x1a,0x04,0x00,0xcc,0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x44,0x89,0xc3}, reinterpret_cast<void**>(&g_layoutView), nullptr, true},
    {0xffc9de, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x0a,0x48,0x89,0x0e,0x48,0x85}, reinterpret_cast<void**>(&g_FontListCopyCtor), nullptr, false},
    {0x26208b4, {0x48,0x8b,0x09,0xe9,0xa8,0x83,0xa1,0xfe,0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57}, reinterpret_cast<void**>(&g_FontListGetFontSize), nullptr, false},
    {0x1038d92, {0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x28,0x44,0x89,0xc7,0x48,0x89,0xd6,0x48,0x89}, reinterpret_cast<void**>(&g_FontListDeriveWithSizeDelta), nullptr, false},
    {0x10fe442, {0x48,0x8b,0x09,0x48,0x85,0xc9,0x0f,0x85,0x06,0x26,0x24,0x04,0xc3,0xcc,0x56,0x48}, reinterpret_cast<void**>(&g_FontListDtor), nullptr, false},
    {0xaff0084, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x48,0x89,0xd6,0x48,0x89,0xcf,0x48,0x8b,0x05}, reinterpret_cast<void**>(&g_MenuItemGetFontListOriginal), reinterpret_cast<void*>(MenuItemGetFontListHook), false},
    {0x30d0680, {0x48,0x83,0xec,0x28,0x8b,0x05,0x1e,0x89,0xe9,0x10,0x8b,0x0d,0x80,0xca,0xe1,0x10}, reinterpret_cast<void**>(&g_configOriginal), reinterpret_cast<void*>(MenuConfigHook), false},
    {0xc851b10, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x53,0x48,0x81,0xec,0x80,0x00}, reinterpret_cast<void**>(&g_createBubbleBorderOriginal), reinterpret_cast<void*>(CreateBubbleBorderHook), false},
    {0x526b4ae, {0x56,0x57,0x53,0x48,0x83,0xec,0x30,0x44,0x89,0xc7,0x89,0xd3,0x48,0x89,0xce,0x48}, reinterpret_cast<void**>(&g_borderCtorOriginal), reinterpret_cast<void*>(BubbleBorderCtorHook), false},
    {0xc84face, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x68}, reinterpret_cast<void**>(&g_menuHostOriginal), reinterpret_cast<void*>(MenuHostHook), false},
    {0x2b76ac6, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x68}, reinterpret_cast<void**>(&g_widgetInitOriginal), reinterpret_cast<void*>(WidgetInitHook), false},
    {0x75dedb0, {0x48,0x8b,0x81,0x48,0x03,0x00,0x00,0xc3,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_submenuGetItem), nullptr, false},
    {0x2d93644, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x88}, reinterpret_cast<void**>(&g_BookmarkButtonBaseCtorOriginal), reinterpret_cast<void*>(BookmarkButtonBaseCtorHook), true},
    {0x2d926a2, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x88}, reinterpret_cast<void**>(&g_BookmarkMenuButtonBaseCtorOriginal), reinterpret_cast<void*>(BookmarkMenuButtonBaseCtorHook), true},
    {0x263d2e2, {0x56,0x57,0x53,0x48,0x83,0xec,0x50,0x48,0x89,0xce,0x48,0x8b,0x05,0x4d,0x0d,0x6e}, reinterpret_cast<void**>(&g_LabelButtonLabelCtorOriginal), reinterpret_cast<void*>(LabelButtonLabelCtorHook), true},
    {0x2624b00, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_LabelButtonLabelDeletingDtorOriginal), reinterpret_cast<void*>(LabelButtonLabelDeletingDtorHook), true},
    {0x10756a0, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x58,0x44,0x89,0xc6,0x89,0xd7,0x48,0x89}, reinterpret_cast<void**>(&g_TypographyGetFontOriginal), reinterpret_cast<void*>(TypographyGetFontHook), true},
    {0x2cce0f0, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x89,0x98,0x03,0x00,0x00,0xe8}, reinterpret_cast<void**>(&g_LabelSetFontList), nullptr, true},
    {0x2a3fc26, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x40,0x4c,0x89}, reinterpret_cast<void**>(&g_GetBookmarkFolderIconOriginal), reinterpret_cast<void*>(GetBookmarkFolderIconHook), true},
    {0xb381b70, {0x56,0x57,0x48,0x83,0xec,0x78,0x48,0x89,0xce,0x48,0x8b,0x05,0xc0,0xc4,0x99,0x08}, reinterpret_cast<void**>(&g_CreateSkBitmapFromHICON), nullptr, true},
    {0x293e860, {0x56,0x57,0x48,0x81,0xec,0x28,0x02,0x00,0x00,0x48,0x89,0xce,0x48,0x8b,0x05,0xcd}, reinterpret_cast<void**>(&g_ImageSkiaCreateFrom1xBitmap), nullptr, true},
    {0x12053e0, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x49,0x18,0x48,0x85,0xc9,0x74}, reinterpret_cast<void**>(&g_SkBitmapDtor), nullptr, true},
    {0x10aad96, {0x48,0x8b,0x09,0x48,0x85,0xc9,0x74,0x06,0xf0,0xff,0x49,0x08,0x74,0x01,0xc3,0xba}, reinterpret_cast<void**>(&g_ImageSkiaDtor), nullptr, true},
    {0x101b086, {0x56,0x57,0x48,0x83,0xec,0x38,0x48,0x89,0xce,0x48,0x8b,0x05,0xaa,0x2f,0xd0,0x12}, reinterpret_cast<void**>(&g_ImageModelFromImageSkia), nullptr, true},
    {0x2a3f5e0, {0x41,0x57,0x41,0x56,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x50,0x01,0x00}, reinterpret_cast<void**>(&g_BookmarkBarViewUpdateAppearanceForThemeOriginal), reinterpret_cast<void*>(BookmarkBarViewUpdateAppearanceForThemeHook), true},
    {0x31bf780, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_BookmarkBarViewDeletingDtorOriginal), reinterpret_cast<void*>(BookmarkBarViewDeletingDtorHook), true},
    {0x2fe83fe, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xe8}, reinterpret_cast<void**>(&g_OmniboxViewViewsCtorOriginal), reinterpret_cast<void*>(OmniboxViewViewsCtorHook), true},
    {0x2ffa200, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_OmniboxViewViewsDtorOriginal), reinterpret_cast<void*>(OmniboxViewViewsDtorHook), true},
    {0x2fe9170, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x78}, reinterpret_cast<void**>(&g_TextfieldCtorOriginal), reinterpret_cast<void*>(TextfieldCtorHook), true},
    {0x2fe8d12, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8b,0x81,0xc0,0x03,0x00,0x00,0x48}, reinterpret_cast<void**>(&g_TextfieldSetFontListOriginal), nullptr, true},
    {0x2943b10, {0x56,0x57,0x48,0x83,0xec,0x68,0x48,0x89,0xce,0x48,0x8b,0x05,0x20,0xa5,0x3d,0x11}, reinterpret_cast<void**>(&g_TabTitleCtorOriginal), reinterpret_cast<void*>(TabTitleCtorHook), true},
    {0x3313fe0, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x2c,0x0c,0x31,0xff}, reinterpret_cast<void**>(&g_TabTitleDtorOriginal), reinterpret_cast<void*>(TabTitleDtorHook), true},
    {0x294488a, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x53,0x48,0x81,0xec,0xc0,0x00}, reinterpret_cast<void**>(&g_TabCloseButtonCtorOriginal), reinterpret_cast<void*>(TabCloseButtonCtorHook), true},
    {0x306169e, {0x56,0x48,0x83,0xec,0x20,0x48,0x89,0xce,0x48,0x8d,0x05,0xd3,0xc1,0x3c,0x0e,0x48}, reinterpret_cast<void**>(&g_TabCloseButtonDtorOriginal), reinterpret_cast<void*>(TabCloseButtonDtorHook), true},
    {0x1050dd0, {0x41,0x56,0x56,0x57,0x53,0x48,0x83,0xec,0x48,0x89,0xd3,0x48,0x89,0xce,0x48,0x8b}, reinterpret_cast<void**>(&g_ViewSetVisibleOriginal), reinterpret_cast<void*>(ViewSetVisibleHook), true},
    {0x1014060, {0x41,0x57,0x41,0x56,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x38,0x48,0x8b,0x05,0xcd}, reinterpret_cast<void**>(&g_ViewInvalidateLayout), nullptr, true},
    {0x101ba90, {0x56,0x48,0x83,0xec,0x30,0x48,0x89,0xce,0x48,0x8b,0x05,0xa1,0x25,0xd0,0x12,0x48}, reinterpret_cast<void**>(&g_ViewPreferredSizeChanged), nullptr, true},
    {0x100bf6c, {0x56,0x57,0x53,0x48,0x83,0xec,0x40,0x89,0xcb,0x48,0x8b,0x05,0xc4,0x20,0xd1,0x12}, reinterpret_cast<void**>(&g_GetLayoutConstantOriginal), reinterpret_cast<void*>(GetLayoutConstantHook), true},
    {0xa0e86c2, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x83,0xec,0x78}, reinterpret_cast<void**>(&g_ToolbarActionViewCtorOriginal), reinterpret_cast<void*>(ToolbarActionViewCtorHook), true},
    {0xa0e9240, {0x48,0x8b,0x89,0x08,0x07,0x00,0x00,0xe9,0x54,0x62,0x83,0xf8,0xcc,0xcc,0xcc,0xcc}, reinterpret_cast<void**>(&g_ToolbarActionViewCalculatePreferredSizeOriginal), reinterpret_cast<void*>(ToolbarActionViewCalculatePreferredSizeHook), true},
    {0xa0e9990, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0xf0,0xf4,0xff,0xff}, reinterpret_cast<void**>(&g_ToolbarActionViewDeletingDtorOriginal), reinterpret_cast<void*>(ToolbarActionViewDeletingDtorHook), true},
    {0xa0e8c00, {0x41,0x57,0x41,0x56,0x56,0x57,0x53,0x48,0x81,0xec,0x10,0x01,0x00,0x00,0x0f,0x29}, reinterpret_cast<void**>(&g_ToolbarActionViewUpdateState), nullptr, true},
    {0x2e3736a, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0xd8}, reinterpret_cast<void**>(&g_ExtensionsToolbarDesktopCtorOriginal), reinterpret_cast<void*>(ExtensionsToolbarDesktopCtorHook), true},
    {0x317e740, {0x56,0x57,0x48,0x83,0xec,0x28,0x89,0xd7,0x48,0x89,0xce,0xe8,0x28,0x00,0x00,0x00}, reinterpret_cast<void**>(&g_ExtensionsToolbarDesktopDeletingDtorOriginal), reinterpret_cast<void*>(ExtensionsToolbarDesktopDeletingDtorHook), true},
    {0xa0ee538, {0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x56,0x57,0x55,0x53,0x48,0x81,0xec,0x98}, reinterpret_cast<void**>(&g_bookmarkFlowOriginal), reinterpret_cast<void*>(BookmarkFlowHook), true},
};
static std::span<const Binding> kBindings = kBindings48;
static constexpr unsigned char kCodeViewIdentity[] = {0x52,0x53,0x44,0x53,0x48,0x7f,0xa0,0x98,0x5b,0x06,0x45,0x9d,0x4c,0x4c,0x44,0x20,0x50,0x44,0x42,0x2e,0x01,0x00,0x00,0x00};
static constexpr unsigned char kCodeViewIdentity53[] = {0x52,0x53,0x44,0x53,0xa4,0xea,0xfb,0xf2,0xc3,0x26,0x30,0x06,0x4c,0x4c,0x44,0x20,0x50,0x44,0x42,0x2e,0x01,0x00,0x00,0x00};
static bool MatchesBuild(const unsigned char* base, const IMAGE_NT_HEADERS64* nt) {
    const auto debug = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    const auto size = nt->OptionalHeader.SizeOfImage;
    if (debug.VirtualAddress > size || debug.Size > size - debug.VirtualAddress) return false;
    for (size_t pos = 0; pos + sizeof(IMAGE_DEBUG_DIRECTORY) <= debug.Size;
         pos += sizeof(IMAGE_DEBUG_DIRECTORY)) {
        auto entry = ReadField<IMAGE_DEBUG_DIRECTORY>(base, debug.VirtualAddress + pos);
        if (entry.Type != IMAGE_DEBUG_TYPE_CODEVIEW || entry.SizeOfData < sizeof(kCodeViewIdentity) ||
            entry.AddressOfRawData > size || sizeof(kCodeViewIdentity) > size - entry.AddressOfRawData) continue;
        const bool oldBuild = !std::memcmp(base + entry.AddressOfRawData, kCodeViewIdentity, sizeof(kCodeViewIdentity));
        const bool newBuild = !std::memcmp(base + entry.AddressOfRawData, kCodeViewIdentity53, sizeof(kCodeViewIdentity53));
        if (oldBuild || newBuild) {
            g_build53 = newBuild;
            kBindings = newBuild ? std::span<const Binding>(kBindings53) : std::span<const Binding>(kBindings48);
            kGlyphAddresses = newBuild ? std::span<const GlyphAddress>(kGlyphAddresses53) : std::span<const GlyphAddress>(kGlyphAddresses48);
            kPopupBranches = newBuild ? std::span<const PopupBranch>(kPopupBranches53) : std::span<const PopupBranch>(kPopupBranches48);
            return true;
        }
    }
    return false;
}
static bool ValidateBindings(const unsigned char* base, const std::vector<CodeRange>& ranges,
                             bool webview) {
    for (const auto& binding : kBindings) {
        if (webview && binding.browserOnly) continue;
        if (!Contains(ranges, base + binding.rva, sizeof(binding.code)) ||
            std::memcmp(base + binding.rva, binding.code, sizeof(binding.code))) {
            Wh_Log(L"Extended UI compatibility check failed at RVA 0x%X", binding.rva);
            return false;
        }
    }
    return true;
}
static bool Install(HMODULE module, const IMAGE_NT_HEADERS64* nt,
                    const std::vector<CodeRange>& ranges, bool webview) {
    auto* base = reinterpret_cast<unsigned char*>(module);
    if (!MatchesBuild(base, nt)) {
        Wh_Log(L"Extended UI tweaks require Edge/WebView2 154.0.4258.48 or .53; using corner fix only");
        return true;
    }
    if (!ValidateBindings(base, ranges, webview)) return false;
    if (!webview && !ValidatePopupBranches(base, ranges)) {
        Wh_Log(L"Popup corner call-site validation failed; refusing extended hooks");
        return false;
    }
    g_styleModule = base;
    for (const auto& binding : kBindings) {
        if (webview && binding.browserOnly) continue;
        if (!binding.replacement) *binding.original = base + binding.rva;
        else if (!Wh_SetFunctionHook(base + binding.rva, binding.replacement, binding.original)) return false;
    }
    if (!webview) {
        HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
        if (dwm) {
            g_dwmGet = reinterpret_cast<DwmGetAttributeFn>(GetProcAddress(dwm, "DwmGetWindowAttribute"));
            auto set = GetProcAddress(dwm, "DwmSetWindowAttribute");
            if (set && !Wh_SetFunctionHook(reinterpret_cast<void*>(set), reinterpret_cast<void*>(DwmSetHook),
                    reinterpret_cast<void**>(&g_dwmSetOriginal))) return false;
        }
    }
    g_addressBarFontHooksReady = !webview;
    g_tabFontHooksReady = !webview;
    g_tabCloseHooksReady = !webview;
    g_extensionTrackingReady = !webview;
    g_extensionContainerTrackingReady = !webview;
    g_bookmarkFolderLiveUpdateReady = !webview;
    g_hooksActivated = true;
    Wh_Log(L"Extended Edge UI hooks installed (%ls)", webview ? L"Windhawk menus" : L"browser and menus");
    return true;
}
} // namespace Native
void LoadExtraSettings() { Native::LoadSettings(); }

bool Install(HMODULE module) {
    if (g_stopping.load() || g_installStarted.exchange(true)) {
        return g_installed.load();
    }
    const auto* base = reinterpret_cast<const unsigned char*>(module);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) return false;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) return false;

    std::vector<CodeRange> ranges;
    const auto* section = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; i++, section++) {
        if (!(section->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
            !(section->Characteristics & IMAGE_SCN_MEM_READ)) continue;
        const size_t rva = section->VirtualAddress;
        const size_t size = section->Misc.VirtualSize;
        if (rva > nt->OptionalHeader.SizeOfImage ||
            size > nt->OptionalHeader.SizeOfImage - rva) return false;
        ranges.push_back({base + rva, size});
    }

    const unsigned char* match = nullptr;
    for (const auto& range : ranges) {
        if (range.size < sizeof(kRadiusCode)) continue;
        std::string_view bytes(reinterpret_cast<const char*>(range.data), range.size);
        std::string_view prefix(reinterpret_cast<const char*>(kRadiusCode), kCalls[0]);
        for (size_t pos = bytes.find(prefix); pos != std::string_view::npos;
             pos = bytes.find(prefix, pos + 1)) {
            if (sizeof(kRadiusCode) > range.size - pos || !MatchesRadius(range.data + pos)) continue;
            if (match) {
                Wh_Log(L"Unsupported Edge build: multiple radius function matches");
                return false;
            }
            match = range.data + pos;
        }
    }
    if (!match || !ValidateHelpers(match, ranges)) {
        Wh_Log(L"Unsupported Edge build: menu radius signature/helpers did not match; no changes applied");
        return false;
    }
    DWORD64 imageBase = 0;
    const auto* entry = RtlLookupFunctionEntry(reinterpret_cast<DWORD64>(match), &imageBase, nullptr);
    if (!entry || imageBase + entry->BeginAddress != reinterpret_cast<DWORD64>(match) ||
        entry->EndAddress - entry->BeginAddress != sizeof(kRadiusCode)) {
        Wh_Log(L"Unsupported Edge build: radius function boundary validation failed");
        return false;
    }
    if (!Wh_SetFunctionHook(const_cast<unsigned char*>(match),
                           reinterpret_cast<void*>(RadiusHook),
                           reinterpret_cast<void**>(&g_originalRadius))) return false;
    if (!Native::Install(module, nt, ranges, g_isWebview)) return false;
    g_installed.store(true);
    Wh_Log(L"Menu radius hook installed at msedge.dll+0x%llX",
           static_cast<unsigned long long>(match - base));
    return true;
}

HMODULE WINAPI LoadLibraryHook(LPCWSTR path, HANDLE file, DWORD flags) {
    HMODULE module = g_originalLoadLibraryExW(path, file, flags);
    if (!module || g_stopping.load() || g_installStarted.load() || !path) return module;
    const wchar_t* name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    if (_wcsicmp(name, L"msedge.dll") == 0 &&
        module == GetModuleHandleW(L"msedge.dll") && Install(module)) {
        Wh_ApplyHookOperations();
    }
    return module;
}

}  // namespace

BOOL Wh_ModInit() {
    // Enforce process scope even when Windhawk's advanced inclusion rules are
    // overridden. Do this before reading settings or registering any hook.
    wchar_t executable[32768];
    const DWORD length = GetModuleFileNameW(nullptr, executable, ARRAYSIZE(executable));
    if (!length || length >= ARRAYSIZE(executable)) return FALSE;
    const wchar_t* filename = wcsrchr(executable, L'\\');
    filename = filename ? filename + 1 : executable;
    const bool webview = _wcsicmp(filename, L"msedgewebview2.exe") == 0;
    if (_wcsicmp(filename, L"msedge.exe") != 0 && !webview) return FALSE;

    int argc = 0;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return FALSE;
    const bool browserProcess = IsSupportedProcess(filename, argc, argv,
                                                   webview && HasWindhawkUiParent());
    LocalFree(argv);
    if (!browserProcess) return FALSE;
    g_isWebview = webview;
    LoadSettings();
    if (auto module = GetModuleHandleW(L"msedge.dll")) return Install(module);
    auto kernelBase = GetModuleHandleW(L"kernelbase.dll");
    auto load = kernelBase ? GetProcAddress(kernelBase, "LoadLibraryExW") : nullptr;
    return load && Wh_SetFunctionHook(reinterpret_cast<void*>(load),
                                     reinterpret_cast<void*>(LoadLibraryHook),
                                     reinterpret_cast<void**>(&g_originalLoadLibraryExW));
}

void Wh_ModAfterInit() {
    Native::RefreshOuterFrames();
    // Cover a load between the initial module check and loader-hook activation.
    if (auto module = GetModuleHandleW(L"msedge.dll"); module && !g_installStarted.load()) {
        if (Install(module)) Wh_ApplyHookOperations();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Native::ApplySettings();
}

void Wh_ModBeforeUninit() {
    g_stopping.store(true);
    Native::Stop();
}
