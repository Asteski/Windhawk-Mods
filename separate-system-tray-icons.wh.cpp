// ==WindhawkMod==
// @id              separate-system-tray-icons
// @name            Separate System Tray Icons
// @description     Adds native-looking Bluetooth, network, and sound buttons to the Windows 11 taskbar tray.
// @version         0.3.0
// @author          Asteski
// @github          https://www.github.com/Asteski
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lshell32 -lole32 -loleaut32 -lruntimeobject -luuid -liphlpapi -lwlanapi -lbthprops
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Separate System Tray Icons

Experimental v2 of the separate Quick Settings tray controls.

Unlike the legacy `Shell_NotifyIcon` version, this mod injects real XAML
`FontIcon` elements into the Windows 11 taskbar tray. That means icons are drawn
as XAML text/vector glyphs instead of rasterized HICON bitmaps.

Buttons:

- Bluetooth -> `ms-controlcenter:bluetooth`
- Network -> `ms-availablenetworks:`
- Sound -> Quick Settings, sound output picker, or Volume Mixer

Sound supports:

- mouse wheel: unmute first, then volume up/down
- middle click: mute toggle

Disable the older `separate-quick-settings-tray-icons` mod while testing this
one, otherwise you'll see both legacy tray icons and XAML tray icons.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- soundClickAction: quick_settings
  $name: Sound click action
  $description: "What happens when the separated sound icon is clicked."
  $options:
  - quick_settings: Open Quick Settings
  - sound_output: Open sound output picker
  - sndvol: Open Volume Mixer
- groupedButtonMode: hidden
  $name: Original grouped button
  $description: "What to do with the original grouped Quick Settings tray button."
  $options:
  - hidden: Hide
  - native: Keep native
  - compact: Replace with single icon
- compactGroupedButtonGlyph: E91C
  $name: Single grouped button glyph
  $description: "Segoe Fluent Icons glyph used when the original grouped button mode is single icon. Example: E91C"
- groupedButtonAction: "ms-controlcenter:"
  $name: Grouped button replacement action
  $description: "Action used by the single icon replacement mode. Supports URI/path, cmd:, shell:, key:/hotkey:, web:, ms-settings:, and ~known-folder actions."
- showBluetoothButton: true
  $name: Show Bluetooth button
- showNetworkButton: true
  $name: Show network button
- showSoundButton: true
  $name: Show sound button
- showCurrentlyPlayingInSoundTooltip: true
  $name: Show currently playing in sound tooltip
- soundIconFollowsOutputDevice: false
  $name: Sound icon follows output device
  $description: "When enabled, the sound icon uses an output-device glyph for headphones, speakers, display audio, etc. Muted/unavailable audio still uses the normal mute glyph."
- buttonOrder: bluetooth,network,sound,quick_settings
  $name: Button order
  $description: "Comma-separated order: bluetooth, network, sound, quick_settings. Hidden and unavailable buttons are skipped; missing visible buttons are appended."
- contextMenuFramework: winui
  $name: Context menu framework
  $description: "Framework used for right-click menus on the Bluetooth, network, and sound buttons. WinUI follows the taskbar theme; Win32 uses the classic native popup menu."
  $options:
  - winui: WinUI
  - win32: Win32
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <netlistmgr.h>
#include <propsys.h>
#include <iphlpapi.h>
#include <iprtrmib.h>
#include <wlanapi.h>
#include <bluetoothapis.h>

#include <atomic>
#include <chrono>
#include <cwchar>
#include <cwctype>
#include <cstring>
#include <new>
#include <unordered_map>
#include <string>
#include <thread>
#include <vector>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/base.h>

namespace wf = winrt::Windows::Foundation;
namespace wfc = winrt::Windows::Foundation::Collections;
namespace wmc = winrt::Windows::Media::Control;
namespace wu = winrt::Windows::UI;
namespace wui = winrt::Windows::UI::Input;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxa = winrt::Windows::UI::Xaml::Automation;
namespace wuc = winrt::Windows::UI::Xaml::Controls;
namespace wucp = winrt::Windows::UI::Xaml::Controls::Primitives;
namespace wuxi = winrt::Windows::UI::Xaml::Input;
namespace wuxmk = winrt::Windows::UI::Xaml::Markup;
namespace wuxm = winrt::Windows::UI::Xaml::Media;


struct Settings {
    std::wstring soundClickAction = L"quick_settings";
    std::wstring groupedButtonMode = L"hidden";
    std::wstring compactGroupedButtonGlyph = L"E91C";
    std::wstring groupedButtonAction = L"ms-controlcenter:";
    bool showBluetoothButton = true;
    bool showNetworkButton = true;
    bool showSoundButton = true;
    bool showCurrentlyPlayingInSoundTooltip = true;
    bool soundIconFollowsOutputDevice = false;
    std::wstring buttonOrder = L"bluetooth,network,sound,quick_settings";
    std::wstring contextMenuFramework = L"winui";
};

enum class NetworkKind {
    Disconnected,
    Ethernet,
    Wifi,
    WifiConnecting,
    WifiDisconnected,
    WifiDisabled,
};

struct NetworkState {
    NetworkKind kind = NetworkKind::Disconnected;
    ULONG signal = 0;
    std::wstring name;
    bool internetAccess = false;
};

struct SoundState {
    bool available = false;
    bool muted = false;
    float volume = 0.0f;
    std::wstring outputName;
    EndpointFormFactor outputFormFactor = UnknownFormFactor;
};

static void UpdateDynamicXamlIcons();

using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void*, void*);
using TaskbarHost_FrameHeight_t = int(WINAPI*)(void*);
using Std_Ref_Decref_t = void(WINAPI*)(void*);
using TrayUI_StartTaskbar_t = void(WINAPI*)(void*);

static Settings g_settings;
static HWND g_taskbarWnd = nullptr;
static wux::FrameworkElement g_bluetoothButton{nullptr};
static wux::FrameworkElement g_networkButton{nullptr};
static wux::FrameworkElement g_soundButton{nullptr};
static wux::FrameworkElement g_compactGroupedButton{nullptr};
struct IconLayers {
    wuc::Grid host{nullptr};
    wuc::FontIcon underlay{nullptr};
    wuc::FontIcon primary{nullptr};
    wuc::FontIcon overlay{nullptr};
};

static IconLayers g_bluetoothIcon;
static IconLayers g_networkIcon;
static IconLayers g_soundIcon;
static IconLayers g_compactGroupedIcon;

struct NativeMirrorSource {
    wuc::FontIcon icon{nullptr};
    wuc::TextBlock textBlock{nullptr};
    std::wstring tooltip;
    std::wstring name;
    std::wstring automationName;
    std::wstring path;
};

struct MediaTooltipInfo {
    bool hasMedia = false;
    bool paused = false;
    std::wstring appName;
    std::wstring title;
    std::wstring artist;
};

static NativeMirrorSource g_nativeNetworkSource;
static NativeMirrorSource g_nativeSoundSource;
static bool g_nativeMirrorSourcesResolved = false;
static int g_nativeMirrorDiagnosticCount = 0;
static ULONGLONG g_nextNativeMirrorRetryTick = 0;
static std::wstring g_bluetoothTooltipCache;
static std::wstring g_networkTooltipCache;
static std::wstring g_soundTooltipCache;
// The taskbar host ignores the placement properties of ToolTipService for
// injected controls and falls back to mouse-relative placement. Keep one
// XAML Popup for our three controls instead, positioned from the taskbar edge.
static wucp::Popup g_fixedTrayTooltipPopup{nullptr};
static wuc::Border g_fixedTrayTooltipBorder{nullptr};
static wuc::TextBlock g_fixedTrayTooltipText{nullptr};
static wux::FrameworkElement g_fixedTrayTooltipTarget{nullptr};
static bool g_fixedTrayTooltipOpened = false;
static wuc::MenuFlyout g_activeTrayContextFlyout{nullptr};
static wuc::Panel g_trayPanel{nullptr};
static wux::FrameworkElement g_trayControlCenterButton{nullptr};
static wux::FrameworkElement g_originalGroupedButton{nullptr};
static wux::Style g_nativeGroupedButtonStyle{nullptr};
static wux::Style g_nativeNotifyIconStyle{nullptr};
static wux::Visibility g_originalGroupedVisibility = wux::Visibility::Visible;
static double g_originalGroupedWidth = NAN;
static double g_originalGroupedMinWidth = 0;
static double g_originalGroupedMaxWidth = INFINITY;
static double g_trayButtonWidth = 32;
static double g_trayButtonHeight = 32;
static int g_notifyMetricDiagnosticCount = 0;
static wux::DispatcherTimer g_updateTimer{nullptr};
static wux::DispatcherTimer g_retryTimer{nullptr};
static wux::DispatcherTimer g_metricRefreshTimer{nullptr};
static int g_retryCount = 0;
static bool g_unloading = false;
static bool g_dumpedTree = false;
static int g_wifiConnectingFrame = 0;
static SRWLOCK g_mediaTooltipLock = SRWLOCK_INIT;
static MediaTooltipInfo g_mediaTooltipInfo;
static std::atomic<ULONGLONG> g_lastMediaTooltipQueryTick{0};
static std::atomic<bool> g_mediaTooltipQueryInProgress{false};
static bool g_metricRefreshPending = false;
static int g_metricRefreshSettlePasses = 0;
static wux::FrameworkElement g_sizeRefreshTrayElement{nullptr};
static wux::FrameworkElement g_sizeRefreshControlCenterButton{nullptr};
static winrt::event_token g_sizeRefreshTrayToken{};
static winrt::event_token g_sizeRefreshControlCenterToken{};

static CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original = nullptr;
static TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original = nullptr;
static Std_Ref_Decref_t Std_Ref_Decref_Original = nullptr;
static TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original = nullptr;
static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static std::wstring GetStringSettingWithDefault(PCWSTR name,
                                                PCWSTR fallback) {
    PCWSTR value = Wh_GetStringSetting(name);
    std::wstring result = value && *value ? value : fallback;
    Wh_FreeStringSetting(value);
    return result;
}

static void LoadSettings() {
    g_settings.soundClickAction =
        GetStringSettingWithDefault(L"soundClickAction", L"quick_settings");
    g_settings.groupedButtonMode =
        GetStringSettingWithDefault(L"groupedButtonMode", L"");
    if (g_settings.groupedButtonMode.empty()) {
        g_settings.groupedButtonMode =
            Wh_GetIntSetting(L"hideOriginalGroupedButton") != 0 ? L"hidden"
                                                                : L"native";
    }
    g_settings.compactGroupedButtonGlyph =
        GetStringSettingWithDefault(L"compactGroupedButtonGlyph", L"E91C");
    g_settings.groupedButtonAction =
        GetStringSettingWithDefault(L"groupedButtonAction",
                                    L"ms-controlcenter:");
    g_settings.showBluetoothButton =
        Wh_GetIntSetting(L"showBluetoothButton") != 0;
    g_settings.showNetworkButton =
        Wh_GetIntSetting(L"showNetworkButton") != 0;
    g_settings.showSoundButton =
        Wh_GetIntSetting(L"showSoundButton") != 0;
    g_settings.showCurrentlyPlayingInSoundTooltip =
        Wh_GetIntSetting(L"showCurrentlyPlayingInSoundTooltip") != 0;
    g_settings.soundIconFollowsOutputDevice =
        Wh_GetIntSetting(L"soundIconFollowsOutputDevice") != 0;
    g_settings.buttonOrder =
        GetStringSettingWithDefault(L"buttonOrder",
                                    L"bluetooth,network,sound,quick_settings");
    g_settings.contextMenuFramework =
        GetStringSettingWithDefault(L"contextMenuFramework", L"winui");
    Wh_Log(L"Tray button settings: bluetooth=%d network=%d sound=%d order=[%s].",
           g_settings.showBluetoothButton, g_settings.showNetworkButton,
           g_settings.showSoundButton, g_settings.buttonOrder.c_str());
}

static bool IsSystemLightTheme() {
    DWORD value = 0;
    DWORD size = sizeof(value);
    LONG ret = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    return ret == ERROR_SUCCESS && value != 0;
}

static wuxm::Brush MakeIconBrush() {
    wu::Color color{};
    color.A = 255;
    if (IsSystemLightTheme()) {
        color.R = 0;
        color.G = 0;
        color.B = 0;
    } else {
        color.R = 255;
        color.G = 255;
        color.B = 255;
    }

    return wuxm::SolidColorBrush(color);
}

static wuxm::Brush MakeUnderlayBrush() {
    wu::Color color{};
    color.A = 255;
    const BYTE channel = IsSystemLightTheme() ? 0xC4 : 0x49;
    color.R = channel;
    color.G = channel;
    color.B = channel;
    return wuxm::SolidColorBrush(color);
}

static winrt::hstring GlyphFromHexSetting(std::wstring const& setting,
                                          wchar_t fallbackGlyph) {
    wchar_t const* text = setting.c_str();
    while (*text == L' ' || *text == L'\t' || *text == L'#') {
        ++text;
    }
    if ((text[0] == L'0') && (text[1] == L'x' || text[1] == L'X')) {
        text += 2;
    }

    wchar_t* end = nullptr;
    unsigned long value = wcstoul(text, &end, 16);
    if (end == text || value == 0 || value > 0xFFFF) {
        value = static_cast<unsigned long>(fallbackGlyph);
    }

    wchar_t glyph[2] = {static_cast<wchar_t>(value), 0};
    return winrt::hstring(glyph);
}

static bool GroupedButtonModeIs(PCWSTR mode) {
    return _wcsicmp(g_settings.groupedButtonMode.c_str(), mode) == 0;
}

static std::wstring ToLower(std::wstring s) {
    for (auto& c : s) {
        c = static_cast<wchar_t>(towlower(c));
    }
    return s;
}

static std::wstring Trim(std::wstring value) {
    size_t start = 0;
    while (start < value.size() && iswspace(value[start])) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && iswspace(value[end - 1])) {
        --end;
    }

    return value.substr(start, end - start);
}

static bool ContainsCI(std::wstring const& value, PCWSTR needle) {
    return ToLower(value).find(ToLower(needle)) != std::wstring::npos;
}

static bool StartsWithCI(std::wstring const& value, PCWSTR prefix) {
    const size_t length = wcslen(prefix);
    return value.size() >= length &&
           _wcsnicmp(value.c_str(), prefix, length) == 0;
}

static bool EndsWithCI(std::wstring const& value, PCWSTR suffix) {
    const size_t length = wcslen(suffix);
    return value.size() >= length &&
           _wcsicmp(value.c_str() + value.size() - length, suffix) == 0;
}

static std::wstring StripQuotes(std::wstring const& value) {
    if (value.size() >= 2 && value.front() == L'"' &&
        value.back() == L'"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

static bool GetKnownFolderPath(PCWSTR name, std::wstring& out) {
    KNOWNFOLDERID folderId{};
    std::wstring lower = ToLower(name);
    if (lower == L"downloads") {
        folderId = FOLDERID_Downloads;
    } else if (lower == L"documents" || lower == L"personal") {
        folderId = FOLDERID_Documents;
    } else if (lower == L"music") {
        folderId = FOLDERID_Music;
    } else if (lower == L"pictures") {
        folderId = FOLDERID_Pictures;
    } else if (lower == L"videos") {
        folderId = FOLDERID_Videos;
    } else if (lower == L"desktop") {
        folderId = FOLDERID_Desktop;
    } else if (lower == L"profile" || lower == L"home") {
        folderId = FOLDERID_Profile;
    } else {
        return false;
    }

    PWSTR raw = nullptr;
    bool ok = SUCCEEDED(SHGetKnownFolderPath(folderId, 0, nullptr, &raw)) &&
              raw;
    if (ok) {
        out = raw;
    }
    if (raw) {
        CoTaskMemFree(raw);
    }
    return ok;
}

static bool TryParseShortcut(std::wstring_view shortcut,
                             UINT* modifiersOut,
                             UINT* vkOut) {
    auto trim = [](std::wstring_view s) {
        size_t start = 0;
        while (start < s.size() && iswspace(s[start])) {
            ++start;
        }

        size_t end = s.size();
        while (end > start && iswspace(s[end - 1])) {
            --end;
        }

        return s.substr(start, end - start);
    };

    std::unordered_map<std::wstring, UINT> modifiersMap = {
        {L"ALT", MOD_ALT},         {L"CTRL", MOD_CONTROL},
        {L"CONTROL", MOD_CONTROL}, {L"SHIFT", MOD_SHIFT},
        {L"WIN", MOD_WIN},
    };
    std::unordered_map<std::wstring, UINT> vkMap = {
        {L"TAB", VK_TAB},             {L"ENTER", VK_RETURN},
        {L"RETURN", VK_RETURN},       {L"SPACE", VK_SPACE},
        {L"ESC", VK_ESCAPE},          {L"ESCAPE", VK_ESCAPE},
        {L"BACKSPACE", VK_BACK},      {L"HOME", VK_HOME},
        {L"END", VK_END},             {L"PAGEUP", VK_PRIOR},
        {L"PAGEDOWN", VK_NEXT},       {L"INSERT", VK_INSERT},
        {L"DELETE", VK_DELETE},       {L"LEFT", VK_LEFT},
        {L"RIGHT", VK_RIGHT},         {L"UP", VK_UP},
        {L"DOWN", VK_DOWN},           {L"VOLUMEMUTE", VK_VOLUME_MUTE},
        {L"VOLUMEUP", VK_VOLUME_UP},  {L"VOLUMEDOWN", VK_VOLUME_DOWN},
        {L"MEDIAPLAYPAUSE", VK_MEDIA_PLAY_PAUSE},
        {L"MEDIANEXT", VK_MEDIA_NEXT_TRACK},
        {L"MEDIAPREV", VK_MEDIA_PREV_TRACK},
        {L"MEDIASTOP", VK_MEDIA_STOP},
    };

    UINT modifiers = 0;
    UINT vk = 0;
    size_t start = 0;
    while (start <= shortcut.size()) {
        size_t plus = shortcut.find(L'+', start);
        std::wstring_view part =
            plus == std::wstring_view::npos
                ? shortcut.substr(start)
                : shortcut.substr(start, plus - start);
        part = trim(part);
        if (part.empty()) {
            return false;
        }

        std::wstring token(part);
        for (auto& ch : token) {
            ch = static_cast<wchar_t>(towupper(ch));
        }

        auto modIt = modifiersMap.find(token);
        if (modIt != modifiersMap.end()) {
            modifiers |= modIt->second;
        } else {
            if (vk != 0) {
                return false;
            }

            if (token.size() == 1 &&
                ((token[0] >= L'A' && token[0] <= L'Z') ||
                 (token[0] >= L'0' && token[0] <= L'9'))) {
                vk = static_cast<UINT>(token[0]);
            } else if (token.size() >= 2 && token[0] == L'F') {
                int fn = _wtoi(token.c_str() + 1);
                if (fn < 1 || fn > 24) {
                    return false;
                }
                vk = VK_F1 + static_cast<UINT>(fn - 1);
            } else {
                auto vkIt = vkMap.find(token);
                if (vkIt != vkMap.end()) {
                    vk = vkIt->second;
                } else {
                    size_t pos = 0;
                    try {
                        unsigned long parsed = std::stoul(token, &pos, 0);
                        if (pos != token.size() || parsed == 0 ||
                            parsed > 0xFF) {
                            return false;
                        }
                        vk = static_cast<UINT>(parsed);
                    } catch (...) {
                        return false;
                    }
                }
            }
        }

        if (plus == std::wstring_view::npos) {
            break;
        }
        start = plus + 1;
    }

    if (vk == 0) {
        return false;
    }

    *modifiersOut = modifiers;
    *vkOut = vk;
    return true;
}

static bool SendShortcut(UINT modifiers, UINT vk) {
    std::vector<INPUT> inputs;
    inputs.reserve(10);

    auto addKey = [&inputs](WORD key, DWORD flags) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        input.ki.dwFlags = flags;
        inputs.push_back(input);
    };

    if (modifiers & MOD_CONTROL) addKey(VK_CONTROL, 0);
    if (modifiers & MOD_SHIFT) addKey(VK_SHIFT, 0);
    if (modifiers & MOD_ALT) addKey(VK_MENU, 0);
    if (modifiers & MOD_WIN) addKey(VK_LWIN, 0);

    addKey(static_cast<WORD>(vk), 0);
    addKey(static_cast<WORD>(vk), KEYEVENTF_KEYUP);

    if (modifiers & MOD_WIN) addKey(VK_LWIN, KEYEVENTF_KEYUP);
    if (modifiers & MOD_ALT) addKey(VK_MENU, KEYEVENTF_KEYUP);
    if (modifiers & MOD_SHIFT) addKey(VK_SHIFT, KEYEVENTF_KEYUP);
    if (modifiers & MOD_CONTROL) addKey(VK_CONTROL, KEYEVENTF_KEYUP);

    UINT sent = SendInput(static_cast<UINT>(inputs.size()), inputs.data(),
                          sizeof(INPUT));
    return sent == inputs.size();
}

static DWORD WINAPI ExecuteActionThreadProc(void* param) {
    auto* raw = static_cast<std::wstring*>(param);
    std::wstring action = raw ? *raw : L"";
    delete raw;

    if (action.empty()) {
        return 0;
    }

    if (StartsWithCI(action, L"web:")) {
        ShellExecuteW(nullptr, L"open", action.substr(4).c_str(), nullptr,
                      nullptr, SW_SHOWNORMAL);
        return 0;
    }
    if (StartsWithCI(action, L"ms-settings:") ||
        StartsWithCI(action, L"ms-controlcenter:") ||
        StartsWithCI(action, L"ms-availablenetworks:") ||
        StartsWithCI(action, L"ms-actioncenter:")) {
        ShellExecuteW(nullptr, L"open", action.c_str(), nullptr, nullptr,
                      SW_SHOWNORMAL);
        return 0;
    }
    if (StartsWithCI(action, L"cmd:")) {
        std::wstring args = L"/C " + action.substr(4);
        ShellExecuteW(nullptr, L"open", L"cmd.exe", args.c_str(), nullptr,
                      SW_HIDE);
        return 0;
    }
    if (StartsWithCI(action, L"shell:")) {
        std::wstring args =
            L"-NoProfile -ExecutionPolicy Bypass -Command " +
            action.substr(6);
        ShellExecuteW(nullptr, L"open", L"powershell.exe", args.c_str(),
                      nullptr, SW_HIDE);
        return 0;
    }
    if (StartsWithCI(action, L"key:") || StartsWithCI(action, L"hotkey:")) {
        std::wstring shortcut =
            StartsWithCI(action, L"key:") ? action.substr(4)
                                           : action.substr(7);
        UINT modifiers = 0;
        UINT vk = 0;
        if (!TryParseShortcut(shortcut, &modifiers, &vk)) {
            Wh_Log(L"Invalid replacement button shortcut action: %s",
                   shortcut.c_str());
            return 0;
        }
        if (!SendShortcut(modifiers, vk)) {
            Wh_Log(L"Failed to send replacement button shortcut: %s",
                   shortcut.c_str());
        }
        return 0;
    }
    if (!action.empty() && action.front() == L'~') {
        std::wstring target = action.substr(1);
        std::wstring resolved;
        if (GetKnownFolderPath(target.c_str(), resolved)) {
            ShellExecuteW(nullptr, L"open", resolved.c_str(), nullptr, nullptr,
                          SW_SHOWNORMAL);
            return 0;
        }
        wchar_t buffer[MAX_PATH * 4]{};
        if (SearchPathW(nullptr, target.c_str(), nullptr, ARRAYSIZE(buffer),
                        buffer, nullptr)) {
            ShellExecuteW(nullptr, L"open", buffer, nullptr, nullptr,
                          SW_SHOWNORMAL);
            return 0;
        }
        Wh_Log(L"Replacement button ~search target not found: %s",
               target.c_str());
        return 0;
    }

    std::wstring path = StripQuotes(action);
    ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr,
                  SW_SHOWNORMAL);
    return 0;
}

static void ExecuteAction(std::wstring const& action) {
    if (action.empty()) {
        return;
    }

    auto* heapAction = new (std::nothrow) std::wstring(action);
    if (!heapAction) {
        return;
    }

    HANDLE thread =
        CreateThread(nullptr, 0, ExecuteActionThreadProc, heapAction, 0,
                     nullptr);
    if (thread) {
        CloseHandle(thread);
    } else {
        delete heapAction;
    }
}

static void LaunchUri(PCWSTR uri) {
    HINSTANCE result =
        ShellExecuteW(nullptr, L"open", uri, nullptr, nullptr, SW_SHOWNORMAL);
    Wh_Log(L"ShellExecuteW(%s) returned %p", uri, result);
}

static void OpenBluetooth() {
    LaunchUri(L"ms-controlcenter:bluetooth");
}

static void OpenNetwork() {
    LaunchUri(L"ms-availablenetworks:");
}

static void OpenSoundOutput() {
    LaunchUri(L"ms-actioncenter:controlcenter/volume");
}

struct WindowSearchByPid {
    DWORD pid = 0;
    HWND hwnd = nullptr;
};

static BOOL CALLBACK FindVisibleWindowByPidProc(HWND hwnd, LPARAM lParam) {
    auto* search = reinterpret_cast<WindowSearchByPid*>(lParam);
    if (!search || !IsWindowVisible(hwnd) || IsIconic(hwnd)) {
        return TRUE;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != search->pid) {
        return TRUE;
    }

    RECT rect{};
    if (!GetWindowRect(hwnd, &rect) ||
        rect.right - rect.left < 100 || rect.bottom - rect.top < 100) {
        return TRUE;
    }

    search->hwnd = hwnd;
    return FALSE;
}

static HWND FindVisibleWindowByPid(DWORD pid) {
    WindowSearchByPid search{};
    search.pid = pid;
    EnumWindows(FindVisibleWindowByPidProc, reinterpret_cast<LPARAM>(&search));
    return search.hwnd;
}

static int ClampInt(int value, int minValue, int maxValue) {
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

static void PositionWindowNearTaskbar(HWND hwnd, PCWSTR label) {
    if (!hwnd) {
        return;
    }

    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!taskbar) {
        Wh_Log(L"%s placement: Shell_TrayWnd not found.", label);
        return;
    }

    RECT taskbarRect{};
    RECT mixerRect{};
    if (!GetWindowRect(taskbar, &taskbarRect) ||
        !GetWindowRect(hwnd, &mixerRect)) {
        Wh_Log(L"%s placement: failed to read window rectangles.", label);
        return;
    }

    HMONITOR monitor = MonitorFromWindow(taskbar, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        Wh_Log(L"sndvol placement: GetMonitorInfoW failed.");
        return;
    }

    const int gap = 10;
    const int mixerWidth = mixerRect.right - mixerRect.left;
    const int mixerHeight = mixerRect.bottom - mixerRect.top;
    const int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    const int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
    const int taskbarWidth = taskbarRect.right - taskbarRect.left;
    const int taskbarHeight = taskbarRect.bottom - taskbarRect.top;

    enum class TaskbarEdge {
        Top,
        Bottom,
        Left,
        Right,
    };

    TaskbarEdge edge = TaskbarEdge::Bottom;
    if (taskbarWidth >= taskbarHeight) {
        const int distanceToTop =
            abs(taskbarRect.top - monitorInfo.rcMonitor.top);
        const int distanceToBottom =
            abs(monitorInfo.rcMonitor.bottom - taskbarRect.bottom);
        edge = distanceToTop <= distanceToBottom ? TaskbarEdge::Top
                                                 : TaskbarEdge::Bottom;
    } else {
        const int distanceToLeft =
            abs(taskbarRect.left - monitorInfo.rcMonitor.left);
        const int distanceToRight =
            abs(monitorInfo.rcMonitor.right - taskbarRect.right);
        edge = distanceToLeft <= distanceToRight ? TaskbarEdge::Left
                                                 : TaskbarEdge::Right;
    }

    int x = taskbarRect.right - mixerWidth - gap;
    int y = taskbarRect.bottom + gap;

    switch (edge) {
        case TaskbarEdge::Top:
            x = taskbarRect.right - mixerWidth - gap;
            y = taskbarRect.bottom + gap;
            break;
        case TaskbarEdge::Bottom:
            x = taskbarRect.right - mixerWidth - gap;
            y = taskbarRect.top - mixerHeight - gap;
            break;
        case TaskbarEdge::Left:
            x = taskbarRect.right + gap;
            y = taskbarRect.bottom - mixerHeight - gap;
            break;
        case TaskbarEdge::Right:
            x = taskbarRect.left - mixerWidth - gap;
            y = taskbarRect.bottom - mixerHeight - gap;
            break;
    }

    x = ClampInt(x, monitorInfo.rcWork.left + gap,
                 monitorInfo.rcWork.right - mixerWidth - gap);
    y = ClampInt(y, monitorInfo.rcWork.top + gap,
                 monitorInfo.rcWork.bottom - mixerHeight - gap);

    SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0,
                 SWP_NOSIZE | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
    Wh_Log(L"%s placement: moved hwnd=%p to %d,%d near taskbar rect=%ld,%ld,%ld,%ld monitor=%ldx%ld.",
           label, hwnd, x, y, taskbarRect.left, taskbarRect.top,
           taskbarRect.right, taskbarRect.bottom, monitorWidth, monitorHeight);
}

struct PositionNearTaskbarRequest {
    HANDLE process = nullptr;
    PCWSTR label = L"window";
};

static DWORD WINAPI PositionWindowNearTaskbarThreadProc(void* param) {
    auto* request = static_cast<PositionNearTaskbarRequest*>(param);
    HANDLE process = request ? request->process : nullptr;
    PCWSTR label = request ? request->label : L"window";
    delete request;
    if (!process) {
        return 0;
    }

    DWORD pid = GetProcessId(process);
    WaitForInputIdle(process, 2000);

    HWND hwnd = nullptr;
    for (int i = 0; i < 40 && !hwnd; ++i) {
        hwnd = FindVisibleWindowByPid(pid);
        if (!hwnd) {
            Sleep(50);
        }
    }

    if (hwnd) {
        PositionWindowNearTaskbar(hwnd, label);
    } else {
        Wh_Log(L"%s placement: no visible top-level window found for pid=%lu.",
               label, pid);
    }

    CloseHandle(process);
    return 0;
}

static void OpenVolumeMixer() {
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"open";
    sei.lpFile = L"sndvol.exe";
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        Wh_Log(L"ShellExecuteExW(sndvol.exe) failed: %lu", GetLastError());
        return;
    }

    Wh_Log(L"ShellExecuteExW(sndvol.exe) process=%p", sei.hProcess);
    if (sei.hProcess) {
        auto* request = new (std::nothrow) PositionNearTaskbarRequest{
            sei.hProcess, L"sndvol"};
        HANDLE thread = request ? CreateThread(
                                     nullptr, 0,
                                     PositionWindowNearTaskbarThreadProc,
                                     request, 0, nullptr)
                                : nullptr;
        if (thread) {
            CloseHandle(thread);
        } else {
            Wh_Log(L"sndvol placement: CreateThread failed: %lu",
                   GetLastError());
            delete request;
            CloseHandle(sei.hProcess);
        }
    }
}

static void OpenControlPanelWindow(PCWSTR parameters, PCWSTR label) {
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"open";
    sei.lpFile = L"control.exe";
    sei.lpParameters = parameters;
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        Wh_Log(L"ShellExecuteExW(control.exe %s) failed: %lu", parameters,
               GetLastError());
        return;
    }

    if (sei.hProcess) {
        auto* request = new (std::nothrow) PositionNearTaskbarRequest{
            sei.hProcess, label};
        HANDLE thread = request ? CreateThread(
                                     nullptr, 0,
                                     PositionWindowNearTaskbarThreadProc,
                                     request, 0, nullptr)
                                : nullptr;
        if (thread) {
            CloseHandle(thread);
        } else {
            Wh_Log(L"%s placement: CreateThread failed: %lu", label,
                   GetLastError());
            delete request;
            CloseHandle(sei.hProcess);
        }
    }
}

static void OpenLegacySoundSettings() {
    OpenControlPanelWindow(L"/name Microsoft.Sound", L"legacy sound settings");
}

static void OpenSpeakerSetup() {
    OpenControlPanelWindow(L"mmsys.cpl,,0", L"speaker setup");
}

static void OpenSoundsControlPanel() {
    OpenControlPanelWindow(L"mmsys.cpl,,2", L"sounds control panel");
}

static void OpenBluetoothOptions() {
    OpenControlPanelWindow(L"bthprops.cpl", L"Bluetooth options");
}

static void OpenBluetoothFileTransfer(bool send) {
    ShellExecuteW(nullptr, L"open", L"fsquirt.exe",
                  send ? L"-send" : L"-receive", nullptr, SW_SHOWNORMAL);
}

static void OpenSound() {
    if (_wcsicmp(g_settings.soundClickAction.c_str(), L"sound_output") == 0) {
        OpenSoundOutput();
        return;
    }

    if (_wcsicmp(g_settings.soundClickAction.c_str(), L"sndvol") == 0) {
        OpenVolumeMixer();
        return;
    }

    LaunchUri(L"ms-controlcenter:");
}

static bool GetDefaultEndpointVolume(IAudioEndpointVolume** outVolume,
                                     bool* outCoInitialized) {
    *outVolume = nullptr;
    *outCoInitialized = false;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool coInitialized = SUCCEEDED(hr);
    *outCoInitialized = coInitialized;
    if (hr == RPC_E_CHANGED_MODE) {
        hr = S_OK;
    }
    if (FAILED(hr)) {
        return false;
    }

    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void**>(&enumerator));
    if (FAILED(hr) || !enumerator) {
        if (coInitialized) {
            CoUninitialize();
        }
        return false;
    }

    IMMDevice* device = nullptr;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    enumerator->Release();
    if (FAILED(hr) || !device) {
        if (coInitialized) {
            CoUninitialize();
        }
        return false;
    }

    hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                          reinterpret_cast<void**>(outVolume));
    device->Release();
    if (FAILED(hr) || !*outVolume) {
        if (coInitialized) {
            CoUninitialize();
        }
        return false;
    }

    return true;
}

static std::wstring GetAudioEndpointFriendlyName(IMMDevice* device) {
    if (!device) {
        return {};
    }

    IPropertyStore* properties = nullptr;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &properties);
    if (FAILED(hr) || !properties) {
        return {};
    }

    PROPVARIANT value;
    PropVariantInit(&value);
    std::wstring name;
    if (SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &value)) &&
        value.vt == VT_LPWSTR && value.pwszVal) {
        name = value.pwszVal;
    }

    PropVariantClear(&value);
    properties->Release();
    return name;
}

static constexpr PROPERTYKEY kAudioEndpointFormFactorKey = {
    {0x1da5d803,
     0xd492,
     0x4edd,
     {0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e}},
    0};

static EndpointFormFactor GetAudioEndpointFormFactor(IMMDevice* device) {
    if (!device) {
        return UnknownFormFactor;
    }

    IPropertyStore* properties = nullptr;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &properties);
    if (FAILED(hr) || !properties) {
        return UnknownFormFactor;
    }

    PROPVARIANT value;
    PropVariantInit(&value);
    EndpointFormFactor formFactor = UnknownFormFactor;
    if (SUCCEEDED(properties->GetValue(kAudioEndpointFormFactorKey,
                                       &value))) {
        if (value.vt == VT_UI4 || value.vt == VT_UINT) {
            formFactor = static_cast<EndpointFormFactor>(value.ulVal);
        } else if (value.vt == VT_I4 || value.vt == VT_INT) {
            formFactor = static_cast<EndpointFormFactor>(value.lVal);
        }
    }

    PropVariantClear(&value);
    properties->Release();
    return formFactor;
}

static SoundState GetSoundState() {
    SoundState state{};

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool coInitialized = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) {
        hr = S_OK;
    }
    if (FAILED(hr)) {
        return state;
    }

    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void**>(&enumerator));
    if (FAILED(hr) || !enumerator) {
        if (coInitialized) {
            CoUninitialize();
        }
        return state;
    }

    IMMDevice* device = nullptr;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    enumerator->Release();
    if (FAILED(hr) || !device) {
        if (coInitialized) {
            CoUninitialize();
        }
        return state;
    }

    state.outputName = GetAudioEndpointFriendlyName(device);
    state.outputFormFactor = GetAudioEndpointFormFactor(device);

    IAudioEndpointVolume* volume = nullptr;
    hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                          reinterpret_cast<void**>(&volume));
    device->Release();
    if (FAILED(hr) || !volume) {
        if (coInitialized) {
            CoUninitialize();
        }
        return state;
    }

    BOOL muted = FALSE;
    float level = 0.0f;
    if (SUCCEEDED(volume->GetMute(&muted)) &&
        SUCCEEDED(volume->GetMasterVolumeLevelScalar(&level))) {
        state.available = true;
        state.muted = muted != FALSE;
        if (level < 0.0f) {
            level = 0.0f;
        } else if (level > 1.0f) {
            level = 1.0f;
        }
        state.volume = level;
    }

    volume->Release();
    if (coInitialized) {
        CoUninitialize();
    }
    return state;
}

static void StepDefaultEndpointVolume(bool up) {
    IAudioEndpointVolume* volume = nullptr;
    bool coInitialized = false;
    if (!GetDefaultEndpointVolume(&volume, &coInitialized)) {
        Wh_Log(L"Sound wheel: default endpoint not available.");
        return;
    }

    BOOL muted = FALSE;
    HRESULT hr = volume->GetMute(&muted);
    if (SUCCEEDED(hr) && muted) {
        hr = volume->SetMute(FALSE, nullptr);
    }

    if (SUCCEEDED(hr)) {
        hr = up ? volume->VolumeStepUp(nullptr)
                : volume->VolumeStepDown(nullptr);
    }
    Wh_Log(L"Sound wheel: VolumeStep%s returned 0x%08X", up ? L"Up" : L"Down",
           hr);

    volume->Release();
    if (coInitialized) {
        CoUninitialize();
    }
}

static void ToggleDefaultEndpointMute() {
    IAudioEndpointVolume* volume = nullptr;
    bool coInitialized = false;
    if (!GetDefaultEndpointVolume(&volume, &coInitialized)) {
        Wh_Log(L"Sound middle-click mute: default endpoint not available.");
        return;
    }

    BOOL muted = FALSE;
    HRESULT hr = volume->GetMute(&muted);
    if (SUCCEEDED(hr)) {
        hr = volume->SetMute(!muted, nullptr);
    }
    Wh_Log(L"Sound middle-click mute toggle returned 0x%08X", hr);

    volume->Release();
    if (coInitialized) {
        CoUninitialize();
    }
}

struct AudioOutputEndpoint {
    std::wstring id;
    std::wstring name;
    bool isDefault = false;
};

static std::vector<AudioOutputEndpoint> GetActiveAudioOutputEndpoints() {
    std::vector<AudioOutputEndpoint> result;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool coInitialized = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) {
        hr = S_OK;
    }
    if (FAILED(hr)) {
        return result;
    }

    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void**>(&enumerator));
    if (FAILED(hr) || !enumerator) {
        if (coInitialized) CoUninitialize();
        return result;
    }

    std::wstring defaultId;
    IMMDevice* defaultDevice = nullptr;
    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole,
                                                      &defaultDevice)) &&
        defaultDevice) {
        LPWSTR id = nullptr;
        if (SUCCEEDED(defaultDevice->GetId(&id)) && id) {
            defaultId = id;
            CoTaskMemFree(id);
        }
        defaultDevice->Release();
    }

    IMMDeviceCollection* devices = nullptr;
    if (SUCCEEDED(enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE,
                                                  &devices)) &&
        devices) {
        UINT count = 0;
        devices->GetCount(&count);
        for (UINT index = 0; index < count; ++index) {
            IMMDevice* device = nullptr;
            if (FAILED(devices->Item(index, &device)) || !device) {
                continue;
            }

            LPWSTR id = nullptr;
            if (SUCCEEDED(device->GetId(&id)) && id) {
                AudioOutputEndpoint endpoint;
                endpoint.id = id;
                endpoint.name = GetAudioEndpointFriendlyName(device);
                endpoint.isDefault = endpoint.id == defaultId;
                if (endpoint.name.empty()) {
                    endpoint.name = L"Audio output";
                }
                result.push_back(std::move(endpoint));
                CoTaskMemFree(id);
            }
            device->Release();
        }
        devices->Release();
    }

    enumerator->Release();
    if (coInitialized) CoUninitialize();
    return result;
}

// The endpoint-selection API is undocumented but stable across modern
// Windows releases. It is what many volume-control utilities use to change
// the default playback device without opening Settings.
struct IPolicyConfigVista : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR, WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT,
                                                       WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, WAVEFORMATEX*,
                                                       WAVEFORMATEX*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT,
                                                           INT64*, INT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, INT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY&,
                                                        PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY&,
                                                        const PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(PCWSTR, ERole) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT) = 0;
};

struct IPolicyConfig : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR, WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT,
                                                       WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, WAVEFORMATEX*,
                                                       WAVEFORMATEX*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT,
                                                           INT64*, INT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, INT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY&,
                                                        PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY&,
                                                        const PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(PCWSTR, ERole) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT) = 0;
};

template <typename TPolicy>
static HRESULT SetDefaultAudioOutputWithPolicy(TPolicy* policy,
                                               std::wstring const& id) {
    if (!policy) {
        return E_POINTER;
    }

    HRESULT hr = policy->SetDefaultEndpoint(id.c_str(), eConsole);
    HRESULT hrMultimedia = policy->SetDefaultEndpoint(id.c_str(), eMultimedia);
    HRESULT hrCommunications =
        policy->SetDefaultEndpoint(id.c_str(), eCommunications);
    if (FAILED(hr)) {
        return hr;
    }
    if (FAILED(hrMultimedia)) {
        return hrMultimedia;
    }
    return hrCommunications;
}

static void SetDefaultAudioOutput(std::wstring const& id) {
    if (id.empty()) {
        return;
    }

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool coInitialized = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) hr = S_OK;
    if (FAILED(hr)) return;

    HRESULT finalHr = E_NOINTERFACE;

    const CLSID clsidPolicyConfig =
        {0x870AF99C, 0x171D, 0x4F9E,
         {0xAF, 0x0D, 0xE6, 0x3D, 0xF4, 0x0C, 0x2B, 0xC9}};
    const IID iidPolicyConfig =
        {0xF8679F50, 0x850A, 0x41CF,
         {0x9C, 0x72, 0x43, 0x0F, 0x29, 0x02, 0x90, 0xC8}};
    IPolicyConfig* policy = nullptr;
    hr = CoCreateInstance(clsidPolicyConfig, nullptr, CLSCTX_ALL,
                          iidPolicyConfig,
                          reinterpret_cast<void**>(&policy));
    if (SUCCEEDED(hr) && policy) {
        finalHr = SetDefaultAudioOutputWithPolicy(policy, id);
        Wh_Log(L"Set default audio output with IPolicyConfig [%s]: 0x%08X",
               id.c_str(), finalHr);
        policy->Release();
    } else {
        Wh_Log(L"CoCreateInstance(IPolicyConfig) failed: 0x%08X", hr);
    }

    if (FAILED(finalHr)) {
        const CLSID clsidPolicyConfigVista =
        {0x294935CE, 0xF637, 0x4E7C,
         {0xA4, 0x1B, 0xAB, 0x25, 0x5B, 0xE8, 0x4F, 0xC6}};
        const IID iidPolicyConfigVista =
        {0x568B9108, 0x44BF, 0x40B4,
         {0x90, 0x06, 0x86, 0xAF, 0xE5, 0xB5, 0xA6, 0x20}};
        IPolicyConfigVista* vistaPolicy = nullptr;
        hr = CoCreateInstance(clsidPolicyConfigVista, nullptr, CLSCTX_ALL,
                              iidPolicyConfigVista,
                              reinterpret_cast<void**>(&vistaPolicy));
        if (SUCCEEDED(hr) && vistaPolicy) {
            finalHr = SetDefaultAudioOutputWithPolicy(vistaPolicy, id);
            Wh_Log(L"Set default audio output with IPolicyConfigVista [%s]: 0x%08X",
                   id.c_str(), finalHr);
            vistaPolicy->Release();
        } else {
            Wh_Log(L"CoCreateInstance(IPolicyConfigVista) failed: 0x%08X", hr);
        }
    }

    Wh_Log(L"Set default audio output final [%s]: 0x%08X", id.c_str(),
           finalHr);
    if (coInitialized) CoUninitialize();
    UpdateDynamicXamlIcons();
}

static bool ContainsAsciiInsensitive(char const* text,
                                    size_t textLength,
                                    char const* needle) {
    if (!text || !needle || !*needle) {
        return false;
    }

    const size_t needleLength = strlen(needle);
    if (needleLength > textLength) {
        return false;
    }

    for (size_t i = 0; i + needleLength <= textLength; ++i) {
        bool match = true;
        for (size_t j = 0; j < needleLength; ++j) {
            char a = text[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z') {
                a = static_cast<char>(a - 'A' + 'a');
            }
            if (b >= 'A' && b <= 'Z') {
                b = static_cast<char>(b - 'A' + 'a');
            }
            if (a != b) {
                match = false;
                break;
            }
        }

        if (match) {
            return true;
        }
    }

    return false;
}

static bool IsLikelyRealEthernet(MIB_IFROW const& row) {
    if (row.dwType != IF_TYPE_ETHERNET_CSMACD || row.dwPhysAddrLen == 0) {
        return false;
    }

    char const* description =
        reinterpret_cast<char const*>(row.bDescr);
    const size_t length = row.dwDescrLen;
    PCSTR virtualMarkers[] = {
        "bluetooth", "hyper-v", "loopback", "npcap", "pseudo",
        "tailscale", "tap",     "tun",     "virtual", "virtualbox",
        "vmware",    "vpn",     "wireguard", "wintun", "zerotier",
    };

    for (PCSTR marker : virtualMarkers) {
        if (ContainsAsciiInsensitive(description, length, marker)) {
            return false;
        }
    }

    return true;
}

static bool IsBluetoothAvailable() {
    BLUETOOTH_FIND_RADIO_PARAMS params{};
    params.dwSize = sizeof(params);

    HANDLE radio = nullptr;
    HBLUETOOTH_RADIO_FIND find = BluetoothFindFirstRadio(&params, &radio);
    if (!find) {
        return false;
    }

    if (radio) {
        CloseHandle(radio);
    }
    BluetoothFindRadioClose(find);
    return true;
}

static bool HasInternetAccess() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool coInitialized = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) {
        hr = S_OK;
    }
    if (FAILED(hr)) {
        return false;
    }

    INetworkListManager* networkListManager = nullptr;
    hr = CoCreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_ALL,
                          IID_PPV_ARGS(&networkListManager));
    if (FAILED(hr) || !networkListManager) {
        if (coInitialized) {
            CoUninitialize();
        }
        return false;
    }

    NLM_CONNECTIVITY connectivity = NLM_CONNECTIVITY_DISCONNECTED;
    bool hasInternet =
        SUCCEEDED(networkListManager->GetConnectivity(&connectivity)) &&
        ((connectivity & NLM_CONNECTIVITY_IPV4_INTERNET) ||
         (connectivity & NLM_CONNECTIVITY_IPV6_INTERNET));

    networkListManager->Release();
    if (coInitialized) {
        CoUninitialize();
    }
    return hasInternet;
}

static std::wstring Dot11SsidToString(DOT11_SSID const& ssid) {
    if (ssid.uSSIDLength == 0) {
        return {};
    }

    char const* bytes = reinterpret_cast<char const*>(ssid.ucSSID);
    int byteCount = static_cast<int>(ssid.uSSIDLength);
    int wideCount =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, byteCount,
                            nullptr, 0);
    UINT codePage = CP_UTF8;
    DWORD flags = MB_ERR_INVALID_CHARS;
    if (wideCount <= 0) {
        codePage = CP_ACP;
        flags = 0;
        wideCount =
            MultiByteToWideChar(codePage, flags, bytes, byteCount, nullptr, 0);
    }

    if (wideCount <= 0) {
        return {};
    }

    std::wstring result(static_cast<size_t>(wideCount), L'\0');
    MultiByteToWideChar(codePage, flags, bytes, byteCount, result.data(),
                        wideCount);
    return result;
}

static NetworkState GetNetworkState() {
    NetworkState state{};
    state.internetAccess = HasInternetAccess();
    bool sawWifi = false;
    bool sawEnabledWifi = false;

    HANDLE wlan = nullptr;
    DWORD negotiatedVersion = 0;
    if (WlanOpenHandle(2, nullptr, &negotiatedVersion, &wlan) == ERROR_SUCCESS) {
        PWLAN_INTERFACE_INFO_LIST interfaces = nullptr;
        if (WlanEnumInterfaces(wlan, nullptr, &interfaces) == ERROR_SUCCESS &&
            interfaces) {
            for (DWORD i = 0; i < interfaces->dwNumberOfItems; ++i) {
                auto const& iface = interfaces->InterfaceInfo[i];
                sawWifi = true;

                if (iface.isState != wlan_interface_state_not_ready) {
                    sawEnabledWifi = true;
                }

                if (iface.isState == wlan_interface_state_associating ||
                    iface.isState == wlan_interface_state_discovering ||
                    iface.isState == wlan_interface_state_authenticating) {
                    state.kind = NetworkKind::WifiConnecting;
                }

                if (iface.isState != wlan_interface_state_connected) {
                    continue;
                }

                DWORD dataSize = 0;
                WLAN_OPCODE_VALUE_TYPE opcode{};
                PWLAN_CONNECTION_ATTRIBUTES attrs = nullptr;
                if (WlanQueryInterface(
                        wlan, &iface.InterfaceGuid,
                        wlan_intf_opcode_current_connection, nullptr,
                        &dataSize, reinterpret_cast<PVOID*>(&attrs),
                        &opcode) == ERROR_SUCCESS &&
                    attrs) {
                    state.kind = NetworkKind::Wifi;
                    state.signal =
                        attrs->wlanAssociationAttributes.wlanSignalQuality;
                    state.name = Dot11SsidToString(
                        attrs->wlanAssociationAttributes.dot11Ssid);
                    WlanFreeMemory(attrs);
                    break;
                }
            }
            WlanFreeMemory(interfaces);
        }
        WlanCloseHandle(wlan, nullptr);
    }

    if (state.kind == NetworkKind::Wifi) {
        return state;
    }

    ULONG size = 0;
    DWORD ret = GetIfTable(nullptr, &size, FALSE);
    if (ret != ERROR_INSUFFICIENT_BUFFER || !size) {
        if (sawWifi) {
            state.kind =
                sawEnabledWifi ? NetworkKind::WifiDisconnected
                               : NetworkKind::WifiDisabled;
        }
        return state;
    }

    auto* table =
        static_cast<MIB_IFTABLE*>(HeapAlloc(GetProcessHeap(), 0, size));
    if (!table) {
        return state;
    }

    ret = GetIfTable(table, &size, FALSE);
    if (ret == NO_ERROR) {
        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
            MIB_IFROW const& row = table->table[i];
            if (row.dwOperStatus != IF_OPER_STATUS_OPERATIONAL ||
                row.dwType == IF_TYPE_SOFTWARE_LOOPBACK) {
                continue;
            }

            if (IsLikelyRealEthernet(row)) {
                state.kind = NetworkKind::Ethernet;
                break;
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, table);
    if (state.kind == NetworkKind::Ethernet ||
        state.kind == NetworkKind::WifiConnecting) {
        return state;
    }

    if (sawWifi) {
        state.kind =
            sawEnabledWifi ? NetworkKind::WifiDisconnected
                           : NetworkKind::WifiDisabled;
    }

    return state;
}

static BOOL CALLBACK FindTaskbarWndProc(HWND hwnd, LPARAM lp) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != GetCurrentProcessId()) {
        return TRUE;
    }

    WCHAR className[64]{};
    if (GetClassNameW(hwnd, className, ARRAYSIZE(className)) &&
        _wcsicmp(className, L"Shell_TrayWnd") == 0) {
        *reinterpret_cast<HWND*>(lp) = hwnd;
        return FALSE;
    }

    return TRUE;
}

static HWND FindCurrentProcessTaskbarWnd() {
    HWND hwnd = nullptr;
    EnumWindows(FindTaskbarWndProc, reinterpret_cast<LPARAM>(&hwnd));
    return hwnd;
}

static wux::XamlRoot GetTaskbarXamlRoot(HWND taskbarWnd) {
    if (!taskbarWnd || !CTaskBand_ITaskListWndSite_vftable ||
        !CTaskBand_GetTaskbarHost_Original || !TaskbarHost_FrameHeight_Original) {
        return nullptr;
    }

    HWND taskSwWnd = reinterpret_cast<HWND>(
        GetPropW(taskbarWnd, L"TaskbandHWND"));
    if (!taskSwWnd) {
        Wh_Log(L"GetTaskbarXamlRoot: TaskbandHWND not found.");
        return nullptr;
    }

    void* taskBand = reinterpret_cast<void*>(GetWindowLongPtrW(taskSwWnd, 0));
    if (!taskBand) {
        Wh_Log(L"GetTaskbarXamlRoot: taskBand pointer not found.");
        return nullptr;
    }

    void* taskBandForTaskListWndSite = taskBand;
    for (int i = 0; *reinterpret_cast<void**>(taskBandForTaskListWndSite) !=
                    CTaskBand_ITaskListWndSite_vftable;
         ++i) {
        if (i == 20) {
            Wh_Log(L"GetTaskbarXamlRoot: ITaskListWndSite vftable not found.");
            return nullptr;
        }
        taskBandForTaskListWndSite =
            reinterpret_cast<void**>(taskBandForTaskListWndSite) + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForTaskListWndSite,
                                      taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) {
        Wh_Log(L"GetTaskbarXamlRoot: TaskbarHost shared_ptr is empty.");
        return nullptr;
    }

    size_t taskbarElementIUnknownOffset = 0x10;
    const BYTE* b = reinterpret_cast<const BYTE*>(
        TaskbarHost_FrameHeight_Original);
    if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
        b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F) {
        taskbarElementIUnknownOffset = b[7];
    } else {
        Wh_Log(L"GetTaskbarXamlRoot: unsupported FrameHeight pattern; using "
               L"fallback offset 0x10.");
    }

    auto* taskbarElementIUnknown =
        *reinterpret_cast<IUnknown**>(
            reinterpret_cast<BYTE*>(taskbarHostSharedPtr[0]) +
            taskbarElementIUnknownOffset);

    wux::FrameworkElement taskbarElement{nullptr};
    if (taskbarElementIUnknown) {
        taskbarElementIUnknown->QueryInterface(
            winrt::guid_of<wux::FrameworkElement>(),
            winrt::put_abi(taskbarElement));
    }

    auto result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;

    if (taskbarHostSharedPtr[1] && Std_Ref_Decref_Original) {
        Std_Ref_Decref_Original(taskbarHostSharedPtr[1]);
    }

    return result;
}

static bool ClassNameMatches(wf::IInspectable const& object, PCWSTR expected) {
    if (!object || !expected) {
        return false;
    }

    std::wstring className = winrt::get_class_name(object).c_str();
    return _wcsicmp(className.c_str(), expected) == 0;
}

static wux::FrameworkElement FindChildByName(wux::DependencyObject const& root,
                                             PCWSTR name) {
    if (!root || !name) {
        return nullptr;
    }

    std::vector<wux::DependencyObject> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        if (auto element = current.try_as<wux::FrameworkElement>()) {
            if (_wcsicmp(element.Name().c_str(), name) == 0) {
                return element;
            }
        }

        int childCount = wuxm::VisualTreeHelper::GetChildrenCount(current);
        for (int i = childCount - 1; i >= 0; --i) {
            auto child = wuxm::VisualTreeHelper::GetChild(current, i);
            if (child) {
                stack.push_back(child);
            }
        }
    }

    return nullptr;
}

static void DumpXamlTree(wux::DependencyObject const& root,
                         int depth = 0,
                         int maxDepth = 6) {
    if (!root || depth > maxDepth) {
        return;
    }

    std::wstring indent(depth * 2, L' ');
    auto element = root.try_as<wux::FrameworkElement>();
    Wh_Log(L"%s%s#%s", indent.c_str(),
           winrt::get_class_name(root).c_str(),
           element ? element.Name().c_str() : L"");

    int childCount = 0;
    try {
        childCount = wuxm::VisualTreeHelper::GetChildrenCount(root);
    } catch (...) {
        Wh_Log(L"%s  <children unavailable: 0x%08X>", indent.c_str(),
               winrt::to_hresult());
        return;
    }

    for (int i = 0; i < childCount; ++i) {
        DumpXamlTree(wuxm::VisualTreeHelper::GetChild(root, i), depth + 1,
                     maxDepth);
    }
}

static std::wstring GetElementSelector(wux::DependencyObject const& object) {
    std::wstring selector = winrt::get_class_name(object).c_str();
    if (auto element = object.try_as<wux::FrameworkElement>()) {
        if (!element.Name().empty()) {
            selector += L"#";
            selector += element.Name().c_str();
        }
    }
    return selector;
}

static void DumpInjectedButtonVisualPaths(wux::DependencyObject const& root,
                                          std::wstring const& path,
                                          int depth = 0,
                                          int maxDepth = 5) {
    if (!root || depth > maxDepth) {
        return;
    }

    std::wstring currentPath =
        path.empty() ? GetElementSelector(root)
                     : path + L" > " + GetElementSelector(root);
    Wh_Log(L"Injected tray visual path: %s", currentPath.c_str());

    int childCount = 0;
    try {
        childCount = wuxm::VisualTreeHelper::GetChildrenCount(root);
    } catch (...) {
        return;
    }

    for (int i = 0; i < childCount; ++i) {
        DumpInjectedButtonVisualPaths(
            wuxm::VisualTreeHelper::GetChild(root, i), currentPath, depth + 1,
            maxDepth);
    }
}

static void DumpInjectedButtonDiagnostics(wux::FrameworkElement const& button) {
    if (!button) {
        return;
    }

    Wh_Log(L"Injected tray button diagnostics for %s#%s.",
           winrt::get_class_name(button).c_str(), button.Name().c_str());
    DumpInjectedButtonVisualPaths(button, L"", 0, 5);
}

static void LogVisualStateGroups(wux::FrameworkElement const& root) {
    if (!root) {
        return;
    }

    try {
        std::vector<wux::DependencyObject> stack;
        stack.push_back(root);

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();

            if (auto element = current.try_as<wux::FrameworkElement>()) {
                auto groups = wux::VisualStateManager::GetVisualStateGroups(
                    element);
                if (groups.Size() > 0) {
                    for (uint32_t i = 0; i < groups.Size(); ++i) {
                        auto group = groups.GetAt(i);
                        std::wstring states;
                        auto groupStates = group.States();
                        for (uint32_t j = 0; j < groupStates.Size(); ++j) {
                            if (!states.empty()) {
                                states += L",";
                            }
                            states += groupStates.GetAt(j).Name().c_str();
                        }

                        Wh_Log(L"Injected tray visual states on %s#%s: group=%s states=[%s].",
                               winrt::get_class_name(element).c_str(),
                               element.Name().c_str(), group.Name().c_str(),
                               states.c_str());
                    }
                }
            }

            int childCount = 0;
            try {
                childCount = wuxm::VisualTreeHelper::GetChildrenCount(current);
            } catch (...) {
                childCount = 0;
            }
            for (int i = 0; i < childCount; ++i) {
                auto child = wuxm::VisualTreeHelper::GetChild(current, i);
                if (child) {
                    stack.push_back(child);
                }
            }
        }
    } catch (...) {
        Wh_Log(L"LogVisualStateGroups failed for %s#%s: 0x%08X",
               winrt::get_class_name(root).c_str(), root.Name().c_str(),
               winrt::to_hresult());
    }
}

static double GetTargetHighlightHeight() {
    if (g_trayButtonHeight <= 34) {
        return 28;
    }
    if (g_trayButtonHeight >= 44) {
        return 40;
    }
    const double fallback = g_trayButtonHeight - 4;
    return fallback > 0 ? fallback : 0;
}

static void ApplyHoverBackgroundMetrics(wux::FrameworkElement const& button) {
    if (!button) {
        return;
    }

    try {
        auto background =
            FindChildByName(button, L"BackgroundBorder").try_as<wux::FrameworkElement>();
        if (!background) {
            return;
        }

        const double targetHeight = GetTargetHighlightHeight();
        background.ClearValue(wux::FrameworkElement::HeightProperty());
        background.ClearValue(wux::FrameworkElement::MaxHeightProperty());
        background.ClearValue(wux::FrameworkElement::MinHeightProperty());
        background.Height(targetHeight);
        background.MaxHeight(targetHeight);
        background.VerticalAlignment(wux::VerticalAlignment::Center);

        if (auto uiElement = background.try_as<wux::UIElement>()) {
            uiElement.InvalidateMeasure();
            uiElement.InvalidateArrange();
        }

        Wh_Log(L"Applied hover BackgroundBorder height %.1f for %s#%s at tray height %.1f.",
               targetHeight, winrt::get_class_name(button).c_str(),
               button.Name().c_str(), g_trayButtonHeight);
    } catch (...) {
        Wh_Log(L"ApplyHoverBackgroundMetrics failed for %s#%s: 0x%08X",
               winrt::get_class_name(button).c_str(), button.Name().c_str(),
               winrt::to_hresult());
    }
}

static bool GoToInjectedButtonState(wux::FrameworkElement const& button,
                                    PCWSTR stateName) {
    if (!button || !stateName) {
        return false;
    }

    try {
        bool changed = wux::VisualStateManager::GoToState(
            button.try_as<wuc::Control>(), stateName, true);
        static int logCount = 0;
        if (!changed && logCount < 24) {
            ++logCount;
            Wh_Log(L"VisualStateManager::GoToState(%s#%s, %s) -> 0.",
                   winrt::get_class_name(button).c_str(),
                   button.Name().c_str(), stateName);
        }
        return changed;
    } catch (...) {
        static int logCount = 0;
        if (logCount < 24) {
            ++logCount;
            Wh_Log(L"VisualStateManager::GoToState(%s#%s, %s) failed: 0x%08X.",
                   winrt::get_class_name(button).c_str(),
                   button.Name().c_str(), stateName, winrt::to_hresult());
        }
        return false;
    }
}

static wux::FrameworkElement FindAncestorFrameworkElement(
    wux::DependencyObject const& start) {
    auto current = start;
    while (current) {
        current = wuxm::VisualTreeHelper::GetParent(current);
        if (auto element = current.try_as<wux::FrameworkElement>()) {
            return element;
        }
    }

    return nullptr;
}

static wux::FrameworkElement FindControlCenterButton(
    wux::DependencyObject const& root) {
    if (!root) {
        return nullptr;
    }

    std::vector<wux::DependencyObject> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        if (auto element = current.try_as<wux::FrameworkElement>()) {
            if (_wcsicmp(element.Name().c_str(), L"ControlCenterButton") == 0 &&
                ClassNameMatches(element, L"SystemTray.OmniButton")) {
                return element;
            }
        }

        int childCount = wuxm::VisualTreeHelper::GetChildrenCount(current);
        for (int i = childCount - 1; i >= 0; --i) {
            auto child = wuxm::VisualTreeHelper::GetChild(current, i);
            if (child) {
                stack.push_back(child);
            }
        }
    }

    return nullptr;
}

static bool IsInjectedElement(wux::FrameworkElement const& element) {
    if (!element) {
        return false;
    }

    PCWSTR name = element.Name().c_str();
    return wcsncmp(name, L"SeparateQuickSettingsXaml", 25) == 0;
}

static std::wstring InspectableToText(wf::IInspectable const& value) {
    if (!value) {
        return {};
    }

    try {
        auto text = winrt::unbox_value<winrt::hstring>(value);
        return text.c_str();
    } catch (...) {
    }

    try {
        if (auto tooltip = value.try_as<wuc::ToolTip>()) {
            return InspectableToText(tooltip.Content());
        }
    } catch (...) {
    }

    try {
        if (auto element = value.try_as<wux::FrameworkElement>()) {
            auto name = element.Name();
            if (!name.empty()) {
                return name.c_str();
            }
        }
    } catch (...) {
    }

    return {};
}

static std::wstring GetElementToolTipText(wux::FrameworkElement const& element) {
    if (!element) {
        return {};
    }

    try {
        return InspectableToText(wuc::ToolTipService::GetToolTip(element));
    } catch (...) {
        return {};
    }
}

static std::wstring GetAutomationName(wux::FrameworkElement const& element) {
    if (!element) {
        return {};
    }

    try {
        return wuxa::AutomationProperties::GetName(element).c_str();
    } catch (...) {
        return {};
    }
}

static bool GlyphEquals(winrt::hstring const& glyph, wchar_t codepoint) {
    return glyph.size() == 1 && glyph[0] == codepoint;
}

static bool IsKnownNetworkGlyph(winrt::hstring const& glyph) {
    static const wchar_t glyphs[] = {
        L'\xE701', L'\xE872', L'\xE873', L'\xE874', L'\xE839',
        L'\xF384', L'\xE871', L'\xEAA5', L'\xEAA8',
    };

    for (wchar_t candidate : glyphs) {
        if (GlyphEquals(glyph, candidate)) {
            return true;
        }
    }
    return false;
}

static bool IsKnownSoundGlyph(winrt::hstring const& glyph) {
    static const wchar_t glyphs[] = {
        L'\xE74F', L'\xE992', L'\xE993', L'\xE994', L'\xE767', L'\xEBC5',
    };

    for (wchar_t candidate : glyphs) {
        if (GlyphEquals(glyph, candidate)) {
            return true;
        }
    }
    return false;
}

static winrt::hstring GetNativeSourceGlyph(NativeMirrorSource const& source) {
    if (source.icon) {
        return source.icon.Glyph();
    }
    if (source.textBlock) {
        return source.textBlock.Text();
    }
    return L"";
}

static std::wstring GlyphToHex(winrt::hstring const& glyph) {
    if (glyph.empty()) {
        return L"";
    }

    wchar_t buffer[16]{};
    swprintf_s(buffer, L"%04X", static_cast<unsigned>(glyph[0]));
    return buffer;
}

struct NativeGlyphCandidate {
    wuc::FontIcon icon{nullptr};
    wuc::TextBlock textBlock{nullptr};
    std::wstring glyphHex;
    std::wstring sourceType;
    std::wstring tooltip;
    std::wstring name;
    std::wstring automationName;
    std::wstring path;
    int networkScore = 0;
    int soundScore = 0;
};

static wux::DependencyObject NativeCandidateObject(
    NativeGlyphCandidate const& candidate) {
    if (candidate.icon) {
        return candidate.icon.try_as<wux::DependencyObject>();
    }
    if (candidate.textBlock) {
        return candidate.textBlock.try_as<wux::DependencyObject>();
    }
    return nullptr;
}

static std::wstring BuildNativeCandidatePath(wux::DependencyObject const& start) {
    std::vector<std::wstring> parts;
    auto current = start;

    while (current) {
        if (auto element = current.try_as<wux::FrameworkElement>()) {
            std::wstring part = winrt::get_class_name(element).c_str();
            auto name = element.Name();
            if (!name.empty()) {
                part += L"#";
                part += name.c_str();
            }
            parts.push_back(part);

            if (element == g_trayControlCenterButton) {
                break;
            }
        }

        current = wuxm::VisualTreeHelper::GetParent(current);
    }

    std::wstring path;
    for (size_t i = parts.size(); i > 0; --i) {
        if (!path.empty()) {
            path += L" > ";
        }
        path += parts[i - 1];
    }
    return path;
}

static NativeGlyphCandidate InspectNativeGlyphCandidate(
    wuc::FontIcon const& icon) {
    NativeGlyphCandidate candidate;
    candidate.icon = icon;
    candidate.glyphHex = GlyphToHex(icon.Glyph());
    candidate.sourceType = L"FontIcon";
    candidate.path = BuildNativeCandidatePath(icon);

    auto current = icon.try_as<wux::DependencyObject>();
    auto glyph = icon.Glyph();
    if (IsKnownNetworkGlyph(glyph)) {
        candidate.networkScore += 35;
    }
    if (IsKnownSoundGlyph(glyph)) {
        candidate.soundScore += 35;
    }
    if (glyph.size() != 1) {
        candidate.networkScore -= 10;
        candidate.soundScore -= 10;
    }
    
    while (current) {
        if (auto element = current.try_as<wux::FrameworkElement>()) {
            if (IsInjectedElement(element)) {
                candidate.networkScore = -1000;
                candidate.soundScore = -1000;
                return candidate;
            }

            auto name = element.Name();
            if (candidate.name.empty() && !name.empty()) {
                candidate.name = name.c_str();
            }
            if (candidate.automationName.empty()) {
                candidate.automationName = GetAutomationName(element);
            }
            if (candidate.tooltip.empty()) {
                candidate.tooltip = GetElementToolTipText(element);
            }

            if (element == g_trayControlCenterButton) {
                break;
            }
        }

        current = wuxm::VisualTreeHelper::GetParent(current);
    }

    std::wstring haystack = ToLower(candidate.tooltip + L" " +
                                   candidate.name + L" " +
                                   candidate.automationName + L" " +
                                   candidate.path + L" " +
                                   candidate.glyphHex);

    if (haystack.find(L"network") != std::wstring::npos ||
        haystack.find(L"wi-fi") != std::wstring::npos ||
        haystack.find(L"wifi") != std::wstring::npos ||
        haystack.find(L"internet") != std::wstring::npos ||
        haystack.find(L"ethernet") != std::wstring::npos ||
        haystack.find(L"airplane") != std::wstring::npos) {
        candidate.networkScore += 100;
    }

    if (haystack.find(L"volume") != std::wstring::npos ||
        haystack.find(L"sound") != std::wstring::npos ||
        haystack.find(L"audio") != std::wstring::npos ||
        haystack.find(L"speaker") != std::wstring::npos) {
        candidate.soundScore += 100;
    }

    return candidate;
}

static NativeGlyphCandidate InspectNativeTextBlockCandidate(
    wuc::TextBlock const& textBlock) {
    NativeGlyphCandidate candidate;
    candidate.textBlock = textBlock;
    candidate.glyphHex = GlyphToHex(textBlock.Text());
    candidate.sourceType = L"TextBlock";
    candidate.path = BuildNativeCandidatePath(textBlock);

    auto current = textBlock.try_as<wux::DependencyObject>();
    auto glyph = textBlock.Text();
    if (IsKnownNetworkGlyph(glyph)) {
        candidate.networkScore += 45;
    }
    if (IsKnownSoundGlyph(glyph)) {
        candidate.soundScore += 45;
    }
    if (glyph.size() != 1) {
        candidate.networkScore -= 25;
        candidate.soundScore -= 25;
    }

    while (current) {
        if (auto element = current.try_as<wux::FrameworkElement>()) {
            if (IsInjectedElement(element)) {
                candidate.networkScore = -1000;
                candidate.soundScore = -1000;
                return candidate;
            }

            auto name = element.Name();
            if (candidate.name.empty() && !name.empty()) {
                candidate.name = name.c_str();
            }
            if (candidate.automationName.empty()) {
                candidate.automationName = GetAutomationName(element);
            }
            if (candidate.tooltip.empty()) {
                candidate.tooltip = GetElementToolTipText(element);
            }

            if (element == g_trayControlCenterButton) {
                break;
            }
        }

        current = wuxm::VisualTreeHelper::GetParent(current);
    }

    std::wstring haystack = ToLower(candidate.tooltip + L" " +
                                   candidate.name + L" " +
                                   candidate.automationName + L" " +
                                   candidate.path + L" " +
                                   candidate.glyphHex);

    if (haystack.find(L"network") != std::wstring::npos ||
        haystack.find(L"wi-fi") != std::wstring::npos ||
        haystack.find(L"wifi") != std::wstring::npos ||
        haystack.find(L"internet") != std::wstring::npos ||
        haystack.find(L"ethernet") != std::wstring::npos ||
        haystack.find(L"airplane") != std::wstring::npos) {
        candidate.networkScore += 100;
    }
    if (haystack.find(L"volume") != std::wstring::npos ||
        haystack.find(L"sound") != std::wstring::npos ||
        haystack.find(L"audio") != std::wstring::npos ||
        haystack.find(L"speaker") != std::wstring::npos) {
        candidate.soundScore += 100;
    }

    return candidate;
}

static void ResolveNativeMirrorSources() {
    g_nativeMirrorSourcesResolved = true;
    g_nativeNetworkSource = {};
    g_nativeSoundSource = {};

    if (!g_trayControlCenterButton) {
        return;
    }

    std::vector<NativeGlyphCandidate> candidates;
    std::vector<wux::DependencyObject> stack;
    stack.push_back(g_trayControlCenterButton);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        if (auto element = current.try_as<wux::FrameworkElement>()) {
            if (IsInjectedElement(element)) {
                continue;
            }
        }

        if (auto icon = current.try_as<wuc::FontIcon>()) {
            auto glyph = icon.Glyph();
            if (!glyph.empty()) {
                auto candidate = InspectNativeGlyphCandidate(icon);
                if (candidate.networkScore > -1000) {
                    candidates.push_back(candidate);
                }
            }
        }

        if (auto textBlock = current.try_as<wuc::TextBlock>()) {
            auto glyph = textBlock.Text();
            if (!glyph.empty() && glyph.size() <= 2) {
                auto candidate = InspectNativeTextBlockCandidate(textBlock);
                if (candidate.networkScore > -1000) {
                    candidates.push_back(candidate);
                }
            }
        }

        int childCount = 0;
        try {
            childCount = wuxm::VisualTreeHelper::GetChildrenCount(current);
        } catch (...) {
            childCount = 0;
        }

        for (int i = childCount - 1; i >= 0; --i) {
            auto child = wuxm::VisualTreeHelper::GetChild(current, i);
            if (child) {
                stack.push_back(child);
            }
        }
    }

    NativeGlyphCandidate* bestNetwork = nullptr;
    NativeGlyphCandidate* bestSound = nullptr;
    for (auto& candidate : candidates) {
        if (!bestNetwork || candidate.networkScore > bestNetwork->networkScore) {
            bestNetwork = &candidate;
        }
        if (!bestSound || candidate.soundScore > bestSound->soundScore) {
            bestSound = &candidate;
        }
    }

    const bool logDiagnostics = g_nativeMirrorDiagnosticCount < 8;
    if (logDiagnostics) {
        ++g_nativeMirrorDiagnosticCount;
        Wh_Log(L"Native mirror scan found %u glyph candidate(s) under ControlCenterButton.",
               static_cast<unsigned>(candidates.size()));
        for (size_t i = 0; i < candidates.size() && i < 32; ++i) {
            auto const& c = candidates[i];
            Wh_Log(L"Native mirror candidate[%u]: type=%s glyph=%s networkScore=%d soundScore=%d name=[%s] automation=[%s] tooltip=[%s] path=[%s]",
                   static_cast<unsigned>(i), c.sourceType.c_str(),
                   c.glyphHex.c_str(), c.networkScore, c.soundScore,
                   c.name.c_str(),
                   c.automationName.c_str(), c.tooltip.c_str(),
                   c.path.c_str());
        }
    }

    if (bestNetwork && bestNetwork->networkScore >= 35) {
        g_nativeNetworkSource.icon = bestNetwork->icon;
        g_nativeNetworkSource.textBlock = bestNetwork->textBlock;
        g_nativeNetworkSource.tooltip = bestNetwork->tooltip;
        g_nativeNetworkSource.name = bestNetwork->name;
        g_nativeNetworkSource.automationName = bestNetwork->automationName;
        g_nativeNetworkSource.path = bestNetwork->path;
        if (logDiagnostics) {
            Wh_Log(L"Native mirror selected network source: type=%s glyph=%s score=%d tooltip=[%s] path=[%s]",
                   bestNetwork->sourceType.c_str(), bestNetwork->glyphHex.c_str(),
                   bestNetwork->networkScore,
                   bestNetwork->tooltip.c_str(), bestNetwork->path.c_str());
        }
    }

    if (bestSound && bestSound->soundScore >= 35 &&
        (!bestNetwork || NativeCandidateObject(*bestSound) !=
                             NativeCandidateObject(*bestNetwork) ||
         bestSound->soundScore > bestNetwork->networkScore)) {
        g_nativeSoundSource.icon = bestSound->icon;
        g_nativeSoundSource.textBlock = bestSound->textBlock;
        g_nativeSoundSource.tooltip = bestSound->tooltip;
        g_nativeSoundSource.name = bestSound->name;
        g_nativeSoundSource.automationName = bestSound->automationName;
        g_nativeSoundSource.path = bestSound->path;
        if (logDiagnostics) {
            Wh_Log(L"Native mirror selected sound source: type=%s glyph=%s score=%d tooltip=[%s] path=[%s]",
                   bestSound->sourceType.c_str(), bestSound->glyphHex.c_str(),
                   bestSound->soundScore,
                   bestSound->tooltip.c_str(), bestSound->path.c_str());
        }
    }
}

// The taskbar's ToolTipService ignores placement settings for injected
// controls on this build and always chooses a mouse-relative anchor. A popup
// is the only XAML surface whose offsets the host respects. It is still drawn
// in-process with XAML and is deliberately non-interactive.
static void HideFixedTrayTooltip() {
    try {
        if (g_fixedTrayTooltipPopup) {
            g_fixedTrayTooltipPopup.IsOpen(false);
        }
    } catch (...) {
    }
    g_fixedTrayTooltipOpened = false;
    g_fixedTrayTooltipTarget = nullptr;
}

static bool GetTaskbarGeometry(RECT* taskbarRect, RECT* hostRect,
                               bool* horizontal, bool* nearFirstEdge) {
    HWND taskbar = g_taskbarWnd ? g_taskbarWnd
                                : FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!taskbar || !GetWindowRect(taskbar, taskbarRect) ||
        !GetWindowRect(taskbar, hostRect)) {
        return false;
    }

    HMONITOR monitor = MonitorFromWindow(taskbar, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!monitor || !GetMonitorInfoW(monitor, &monitorInfo)) {
        return false;
    }

    const int width = taskbarRect->right - taskbarRect->left;
    const int height = taskbarRect->bottom - taskbarRect->top;
    *horizontal = width >= height;
    if (*horizontal) {
        *nearFirstEdge = abs(taskbarRect->top - monitorInfo.rcMonitor.top) <=
                         abs(monitorInfo.rcMonitor.bottom - taskbarRect->bottom);
    } else {
        *nearFirstEdge = abs(taskbarRect->left - monitorInfo.rcMonitor.left) <=
                         abs(monitorInfo.rcMonitor.right - taskbarRect->right);
    }
    return true;
}

static void EnsureFixedTrayTooltip() {
    if (g_fixedTrayTooltipPopup) {
        return;
    }

    const bool light = IsSystemLightTheme();
    wu::Color background{};
    background.A = 255;
    background.R = light ? 0xF9 : 0x2C;
    background.G = light ? 0xF9 : 0x2C;
    background.B = light ? 0xF9 : 0x2C;
    wu::Color foreground{};
    foreground.A = 255;
    foreground.R = light ? 0x1A : 0xF5;
    foreground.G = light ? 0x1A : 0xF5;
    foreground.B = light ? 0x1A : 0xF5;

    g_fixedTrayTooltipText = wuc::TextBlock();
    g_fixedTrayTooltipText.FontSize(12);
    g_fixedTrayTooltipText.Foreground(wuxm::SolidColorBrush(foreground));
    g_fixedTrayTooltipText.IsHitTestVisible(false);

    g_fixedTrayTooltipBorder = wuc::Border();
    g_fixedTrayTooltipBorder.Background(wuxm::SolidColorBrush(background));
    g_fixedTrayTooltipBorder.BorderThickness({1, 1, 1, 1});
    g_fixedTrayTooltipBorder.BorderBrush(
        wuxm::SolidColorBrush(light ? wu::Color{255, 224, 224, 224}
                               : wu::Color{255, 70, 70, 70}));
    g_fixedTrayTooltipBorder.CornerRadius({4, 4, 4, 4});
    g_fixedTrayTooltipBorder.Padding({10, 7, 10, 7});
    g_fixedTrayTooltipBorder.IsHitTestVisible(false);
    g_fixedTrayTooltipBorder.Child(g_fixedTrayTooltipText);

    g_fixedTrayTooltipPopup = wucp::Popup();
    g_fixedTrayTooltipPopup.Child(g_fixedTrayTooltipBorder);
    g_fixedTrayTooltipPopup.IsLightDismissEnabled(false);
    g_fixedTrayTooltipPopup.Opened([](wf::IInspectable const&,
                                      wf::IInspectable const&) {
        g_fixedTrayTooltipOpened = true;
        Wh_Log(L"Fixed tray tooltip popup opened for %s#%s.",
               g_fixedTrayTooltipTarget
                   ? winrt::get_class_name(g_fixedTrayTooltipTarget).c_str()
                   : L"(none)",
               g_fixedTrayTooltipTarget
                   ? g_fixedTrayTooltipTarget.Name().c_str()
                   : L"");
        // The stock tooltip is retained as a fallback until this event proves
        // the Popup was attached to Shell's XAML popup root.
        if (g_fixedTrayTooltipTarget) {
            wuc::ToolTipService::SetToolTip(g_fixedTrayTooltipTarget, nullptr);
        }
    });
    if (auto popup3 = g_fixedTrayTooltipPopup.try_as<wucp::IPopup3>()) {
        popup3.ShouldConstrainToRootBounds(false);
    }
}

static double GetTaskbarDpiScale(HWND taskbar) {
    using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
    auto getDpiForWindow = reinterpret_cast<GetDpiForWindow_t>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    const UINT dpi = (taskbar && getDpiForWindow) ? getDpiForWindow(taskbar)
                                                   : 96;
    return dpi ? static_cast<double>(dpi) / 96.0 : 1.0;
}

static void ShowFixedTrayTooltip(wux::FrameworkElement const& targetButton,
                                 std::wstring const& tooltip) {
    if (!targetButton || tooltip.empty()) {
        return;
    }

    try {
        EnsureFixedTrayTooltip();
        g_fixedTrayTooltipText.Text(tooltip);
        g_fixedTrayTooltipBorder.Measure({640.0f, 320.0f});
        const wf::Size desired = g_fixedTrayTooltipBorder.DesiredSize();

        RECT taskbarRect{};
        RECT hostRect{};
        bool horizontal = true;
        bool nearFirstEdge = true;
        if (!GetTaskbarGeometry(&taskbarRect, &hostRect, &horizontal,
                                &nearFirstEdge)) {
            return;
        }

        HWND taskbar = g_taskbarWnd ? g_taskbarWnd
                                    : FindWindowW(L"Shell_TrayWnd", nullptr);
        const double scale = GetTaskbarDpiScale(taskbar);
        const auto transform = targetButton.TransformToVisual(nullptr);
        const wf::Point target = transform.TransformPoint({0.0f, 0.0f});
        constexpr double kGap = 12.0;
        double x = target.X + (targetButton.ActualWidth() - desired.Width) / 2.0;
        double y = target.Y + (targetButton.ActualHeight() - desired.Height) / 2.0;

        if (horizontal) {
            const double taskbarEdge = nearFirstEdge
                                           ? (taskbarRect.bottom - hostRect.top) / scale
                                           : (taskbarRect.top - hostRect.top) / scale;
            y = nearFirstEdge ? taskbarEdge + kGap
                              : taskbarEdge - desired.Height - kGap;
        } else {
            const double taskbarEdge = nearFirstEdge
                                           ? (taskbarRect.right - hostRect.left) / scale
                                           : (taskbarRect.left - hostRect.left) / scale;
            x = nearFirstEdge ? taskbarEdge + kGap
                              : taskbarEdge - desired.Width - kGap;
        }

        g_fixedTrayTooltipPopup.HorizontalOffset(x);
        g_fixedTrayTooltipPopup.VerticalOffset(y);
        g_fixedTrayTooltipTarget = targetButton;
        g_fixedTrayTooltipOpened = false;
        g_fixedTrayTooltipPopup.IsOpen(true);
        Wh_Log(L"Requested fixed tray tooltip for %s#%s at %.1f,%.1f.",
               winrt::get_class_name(targetButton).c_str(),
               targetButton.Name().c_str(), x, y);
    } catch (...) {
        Wh_Log(L"ShowFixedTrayTooltip failed for %s#%s: 0x%08X",
               winrt::get_class_name(targetButton).c_str(),
               targetButton.Name().c_str(), winrt::to_hresult());
    }
}

// Shell chooses its own mouse-relative geometry while it opens a tooltip.
// Applying these values in ToolTip::Opened runs after that Shell step, while
// retaining the native tooltip template and behaviour.
static void ApplyNativeTrayToolTipPlacement(
    wuc::ToolTip const& tip, wux::FrameworkElement const& targetButton) {
    if (!tip || !targetButton) {
        return;
    }

    RECT taskbarRect{};
    RECT hostRect{};
    bool horizontal = true;
    bool nearFirstEdge = true;
    if (!GetTaskbarGeometry(&taskbarRect, &hostRect, &horizontal,
                            &nearFirstEdge)) {
        return;
    }

    const auto placement = horizontal
                               ? (nearFirstEdge ? wucp::PlacementMode::Bottom
                                                : wucp::PlacementMode::Top)
                               : (nearFirstEdge ? wucp::PlacementMode::Right
                                                : wucp::PlacementMode::Left);
    constexpr double kGap = 12.0;
    wuc::ToolTipService::SetPlacementTarget(targetButton, targetButton);
    wuc::ToolTipService::SetPlacement(targetButton, placement);
    tip.PlacementTarget(targetButton);
    tip.Placement(placement);
    tip.HorizontalOffset(horizontal ? 0.0 : (nearFirstEdge ? kGap : -kGap));
    tip.VerticalOffset(horizontal ? (nearFirstEdge ? kGap : -kGap) : 0.0);
    tip.PlacementRect(winrt::box_value(wf::Rect{
                             0.0f, 0.0f,
                             static_cast<float>(targetButton.ActualWidth()),
                             static_cast<float>(targetButton.ActualHeight())})
                          .as<wf::IReference<wf::Rect>>());
}

static void SetTrayToolTip(wux::FrameworkElement const& targetButton,
                           std::wstring const& tooltip) {
    if (!targetButton || tooltip.empty()) {
        return;
    }

    try {
        // Keep the stock tooltip. Shell rejects a standalone XAML Popup in
        // explorer.exe (0x8000FFFF), so this is the supported host surface.
        wuc::ToolTip tip;
        tip.Content(winrt::box_value(tooltip));
        ApplyNativeTrayToolTipPlacement(tip, targetButton);
        tip.Opened([targetButton](wf::IInspectable const& sender,
                                 wux::RoutedEventArgs const&) {
            if (auto openedTip = sender.try_as<wuc::ToolTip>()) {
                ApplyNativeTrayToolTipPlacement(openedTip, targetButton);
            }
        });
        wuc::ToolTipService::SetToolTip(targetButton, tip);
        wuxa::AutomationProperties::SetName(targetButton, tooltip);
    } catch (...) {
        Wh_Log(L"SetTrayToolTip failed for %s#%s: 0x%08X",
               winrt::get_class_name(targetButton).c_str(),
               targetButton.Name().c_str(), winrt::to_hresult());
    }
}

static void SetCachedTrayToolTip(wux::FrameworkElement const& targetButton,
                                 std::wstring& cache,
                                 std::wstring const& tooltip) {
    if (!targetButton || tooltip.empty()) {
        return;
    }

    try {
        if (cache == tooltip) {
            return;
        }
        cache = tooltip;
        auto existing = wuc::ToolTipService::GetToolTip(targetButton)
                            .try_as<wuc::ToolTip>();
        if (existing) {
            // Replacing the ToolTip object closes it. Updating its live
            // content preserves the open tooltip while the sound wheel moves
            // the volume, exactly like the native tray sound control.
            existing.Content(winrt::box_value(tooltip));
        } else {
            SetTrayToolTip(targetButton, tooltip);
        }
        wuxa::AutomationProperties::SetName(targetButton, tooltip);
        if (g_fixedTrayTooltipTarget == targetButton &&
            g_fixedTrayTooltipPopup && g_fixedTrayTooltipPopup.IsOpen()) {
            ShowFixedTrayTooltip(targetButton, tooltip);
        }
        Wh_Log(L"Updated tray tooltip for %s#%s: [%s]",
               winrt::get_class_name(targetButton).c_str(),
               targetButton.Name().c_str(), tooltip.c_str());
    } catch (...) {
        Wh_Log(L"Updating fixed tooltip failed for %s#%s: 0x%08X",
               winrt::get_class_name(targetButton).c_str(),
               targetButton.Name().c_str(), winrt::to_hresult());
    }
}

static bool ApplyNativeMirrorSource(NativeMirrorSource& source,
                                    IconLayers const& targetIcon,
                                    wux::FrameworkElement const& targetButton,
                                    std::wstring& tooltipCache,
                                    wuxm::Brush const& primaryBrush) {
    if (!source.icon && !source.textBlock && g_nativeMirrorSourcesResolved) {
        ULONGLONG now = GetTickCount64();
        if (now < g_nextNativeMirrorRetryTick) {
            return false;
        }

        g_nextNativeMirrorRetryTick = now + 5000;
        g_nativeMirrorSourcesResolved = false;
    }

    if (!g_nativeMirrorSourcesResolved) {
        ResolveNativeMirrorSources();
    }

    if ((!source.icon && !source.textBlock) || !targetIcon.primary) {
        return false;
    }

    try {
        auto glyph = GetNativeSourceGlyph(source);
        if (glyph.empty()) {
            return false;
        }

        targetIcon.primary.Glyph(glyph);
        targetIcon.primary.Foreground(primaryBrush);

        auto sourceElement =
            source.icon ? source.icon.try_as<wux::FrameworkElement>()
                        : source.textBlock.try_as<wux::FrameworkElement>();
        std::wstring tooltip = GetElementToolTipText(sourceElement);
        if (tooltip.empty()) {
            tooltip = source.tooltip;
        }
        SetCachedTrayToolTip(targetButton, tooltipCache, tooltip);

        return true;
    } catch (...) {
        Wh_Log(L"Native mirror source became unavailable; falling back: 0x%08X",
               winrt::to_hresult());
        source = {};
        g_nativeMirrorSourcesResolved = false;
        return false;
    }
}

static void RemoveInjectedControls(wuc::Panel const& parent) {
    if (!parent) {
        return;
    }

    HideFixedTrayTooltip();

    auto children = parent.Children();
    for (uint32_t i = children.Size(); i > 0; --i) {
        uint32_t index = i - 1;
        auto element = children.GetAt(index).try_as<wux::FrameworkElement>();
        if (IsInjectedElement(element)) {
            children.RemoveAt(index);
        }
    }

    g_bluetoothButton = nullptr;
    g_networkButton = nullptr;
    g_soundButton = nullptr;
    g_compactGroupedButton = nullptr;
    g_bluetoothIcon = {};
    g_networkIcon = {};
    g_soundIcon = {};
    g_compactGroupedIcon = {};
    g_nativeMirrorSourcesResolved = false;
    g_nextNativeMirrorRetryTick = 0;
    g_nativeNetworkSource = {};
    g_nativeSoundSource = {};
    g_bluetoothTooltipCache.clear();
    g_networkTooltipCache.clear();
    g_soundTooltipCache.clear();
}

static void RestoreOriginalGroupedButton() {
    if (g_originalGroupedButton) {
        g_originalGroupedButton.Visibility(g_originalGroupedVisibility);
        g_originalGroupedButton.Width(g_originalGroupedWidth);
        g_originalGroupedButton.MinWidth(g_originalGroupedMinWidth);
        g_originalGroupedButton.MaxWidth(g_originalGroupedMaxWidth);
    }
}

static void HideOriginalGroupedButton(wux::FrameworkElement const& button) {
    if (!button) {
        return;
    }

    if (!g_originalGroupedButton || g_originalGroupedButton != button) {
        g_originalGroupedButton = button;
        g_originalGroupedVisibility = button.Visibility();
        g_originalGroupedWidth = button.Width();
        g_originalGroupedMinWidth = button.MinWidth();
        g_originalGroupedMaxWidth = button.MaxWidth();

        if (auto control = button.try_as<wuc::Control>()) {
            g_nativeGroupedButtonStyle = control.Style();
            Wh_Log(L"Captured grouped button class=%s style=%p.",
                   winrt::get_class_name(button).c_str(),
                   winrt::get_abi(g_nativeGroupedButtonStyle));
        } else {
            Wh_Log(L"Grouped button is not projected as Windows.UI.Xaml.Controls.Control: %s.",
                   winrt::get_class_name(button).c_str());
        }
    }

    button.Visibility(wux::Visibility::Collapsed);
    button.Width(0);
    button.MinWidth(0);
    button.MaxWidth(0);
}

static bool IsUsableTrayButtonDimension(double value) {
    // The normal 48-pixel taskbar reports 32x48 XAML units on this build;
    // the compact taskbar reports smaller values.  Both are valid tray
    // hit-target sizes, so do not reject the default height as a fallback.
    return value == value && value >= 18 && value <= 64;
}

static bool TryCaptureTrayButtonMetricsFromElement(
    wux::FrameworkElement const& reference) {
    if (!reference || IsInjectedElement(reference)) {
        return false;
    }

    double width = reference.ActualWidth();
    double height = reference.ActualHeight();
    bool usedDeclaredSize = false;
    if (!IsUsableTrayButtonDimension(width) ||
        !IsUsableTrayButtonDimension(height)) {
        // During taskbar/XAML rebuilds the icon can exist in the visual tree
        // before its first measure pass.  Its declared Width/Height is still
        // the correct live taskbar metric and becomes available earlier.
        width = reference.Width();
        height = reference.Height();
        usedDeclaredSize = true;
    }
    if (!IsUsableTrayButtonDimension(width) ||
        !IsUsableTrayButtonDimension(height)) {
        return false;
    }

    g_trayButtonWidth = width;
    g_trayButtonHeight = height;
    Wh_Log(L"Captured single tray button metrics from %s#%s: %.1fx%.1f%s.",
           winrt::get_class_name(reference).c_str(), reference.Name().c_str(),
           g_trayButtonWidth, g_trayButtonHeight,
           usedDeclaredSize ? L" (declared)" : L"");
    return true;
}

// The direct children of SystemTrayFrameGrid are layout stacks.  Their
// dimensions describe the stack, not the individual tray hit target.  Walk
// the visual subtree and prefer the real NotifyIconView used by Windows for
// ordinary tray icons.  This keeps our OmniButtons on the same live size when
// the taskbar switches between its normal and small metrics.
static bool TryCaptureNativeNotifyIconMetrics(
    wux::DependencyObject const& root) {
    if (!root) {
        return false;
    }

    std::vector<wux::DependencyObject> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        auto element = current.try_as<wux::FrameworkElement>();
        if (element && !IsInjectedElement(element)) {
            const auto className = winrt::get_class_name(current);
            const auto name = element.Name();
            const bool isNotifyIcon =
                _wcsicmp(className.c_str(), L"SystemTray.NotifyIconView") == 0 ||
                _wcsicmp(name.c_str(), L"NotifyItemIcon") == 0;
            if (isNotifyIcon && g_notifyMetricDiagnosticCount < 8) {
                Wh_Log(L"NotifyIcon candidate %s#%s actual=%.1fx%.1f declared=%.1fx%.1f.",
                       className.c_str(), name.c_str(), element.ActualWidth(),
                       element.ActualHeight(), element.Width(), element.Height());
                ++g_notifyMetricDiagnosticCount;
            }
            if (isNotifyIcon && TryCaptureTrayButtonMetricsFromElement(element)) {
                if (auto control = element.try_as<wuc::Control>()) {
                    try {
                        auto style = control.Style();
                        if (style) {
                            g_nativeNotifyIconStyle = style;
                            Wh_Log(L"Captured live NotifyIconView style from %s#%s.",
                                   className.c_str(), name.c_str());
                        }
                    } catch (...) {
                        Wh_Log(L"Reading NotifyIconView style failed: 0x%08X",
                               winrt::to_hresult());
                    }
                }
                Wh_Log(L"Using individual native tray icon metrics from %s#%s.",
                       className.c_str(), name.c_str());
                return true;
            }
        }

        const int count = wuxm::VisualTreeHelper::GetChildrenCount(current);
        for (int i = count - 1; i >= 0; --i) {
            auto child = wuxm::VisualTreeHelper::GetChild(current, i);
            if (child) {
                stack.push_back(child);
            }
        }
    }

    return false;
}

static void ApplyTrayButtonMetrics(wux::FrameworkElement const& element);

static wux::FrameworkElement TryCreateNativeNotifyIcon(PCWSTR name) {
    try {
        auto loaded = wuxmk::XamlReader::Load(
            LR"(<SystemTray:NotifyIconView xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" xmlns:SystemTray="using:SystemTray" HorizontalAlignment="Center" VerticalAlignment="Center"/>)");
        auto element = loaded.try_as<wux::FrameworkElement>();
        if (!element) {
            Wh_Log(L"XamlReader loaded NotifyIconView, but it is not FrameworkElement: %s.",
                   winrt::get_class_name(loaded).c_str());
            return nullptr;
        }

        element.Name(name);
        if (auto control = element.try_as<wuc::Control>()) {
            if (g_nativeNotifyIconStyle) {
                try {
                    control.Style(g_nativeNotifyIconStyle);
                    Wh_Log(L"Applied live NotifyIconView style to %s#%s.",
                           winrt::get_class_name(element).c_str(), name);
                } catch (winrt::hresult_error const& e) {
                    Wh_Log(L"Applying NotifyIconView style failed: 0x%08X %s",
                           e.code(), e.message().c_str());
                }
            }
        }

        ApplyTrayButtonMetrics(element);
        Wh_Log(L"Created native tray element through XamlReader: %s#%s",
               winrt::get_class_name(element).c_str(), name);
        return element;
    } catch (winrt::hresult_error const& e) {
        Wh_Log(L"XamlReader SystemTray.NotifyIconView creation failed: 0x%08X %s",
               e.code(), e.message().c_str());
    } catch (...) {
        Wh_Log(L"XamlReader SystemTray.NotifyIconView creation failed: 0x%08X",
               winrt::to_hresult());
    }

    return nullptr;
}

static void CaptureTrayButtonMetricsFromPanel(
    wuc::Panel const& parentPanel,
    wux::FrameworkElement const& controlCenterButton) {
    if (!parentPanel) {
        return;
    }

    auto children = parentPanel.Children();
    uint32_t controlCenterIndex = children.Size();
    if (controlCenterButton) {
        children.IndexOf(controlCenterButton.as<wux::UIElement>(),
                         controlCenterIndex);
    }

    // Prefer a real individual tray icon anywhere below the frame.  The
    // ordinary notification-area stacks are present in both taskbar sizes,
    // and their NotifyIconView dimensions are the most faithful hit-target
    // reference available to us.
    if (TryCaptureNativeNotifyIconMetrics(parentPanel)) {
        return;
    }

    // Adjacent containers (clock, chevron, empty app-icon stacks) are not
    // individual tray buttons. Retain the last valid metrics if none exist.

    Wh_Log(L"No nearby single tray button metrics found; using fallback %.1fx%.1f.",
           g_trayButtonWidth, g_trayButtonHeight);
}

static void ApplyTrayButtonMetrics(wux::FrameworkElement const& element) {
    if (!element) {
        return;
    }

    try {
        element.ClearValue(wux::FrameworkElement::WidthProperty());
        element.ClearValue(wux::FrameworkElement::HeightProperty());
        element.ClearValue(wux::FrameworkElement::MaxWidthProperty());
        element.ClearValue(wux::FrameworkElement::MinWidthProperty());
        element.ClearValue(wux::FrameworkElement::MaxHeightProperty());
        element.ClearValue(wux::FrameworkElement::MinHeightProperty());
    } catch (...) {
        Wh_Log(L"ApplyTrayButtonMetrics: ClearValue failed for %s#%s: 0x%08X",
               winrt::get_class_name(element).c_str(), element.Name().c_str(),
               winrt::to_hresult());
    }

    element.Width(g_trayButtonWidth);
    element.Height(g_trayButtonHeight);
    element.MinWidth(0);
    element.MaxWidth(g_trayButtonWidth);
    element.MinHeight(0);
    element.MaxHeight(g_trayButtonHeight);
    element.HorizontalAlignment(wux::HorizontalAlignment::Center);
    element.VerticalAlignment(wux::VerticalAlignment::Center);

    if (auto uiElement = element.try_as<wux::UIElement>()) {
        uiElement.InvalidateMeasure();
        uiElement.InvalidateArrange();
    }
}

static bool ApplyXamlButtons();

static void ScheduleMetricRefresh() {
    if (g_unloading || g_metricRefreshPending) {
        return;
    }

    g_metricRefreshPending = true;
    g_metricRefreshSettlePasses = 0;
    if (!g_metricRefreshTimer) {
        g_metricRefreshTimer = wux::DispatcherTimer();
        g_metricRefreshTimer.Interval(std::chrono::milliseconds(250));
        g_metricRefreshTimer.Tick([](wf::IInspectable const&,
                                     wf::IInspectable const&) {
            if (g_metricRefreshTimer) {
                g_metricRefreshTimer.Stop();
            }
            g_metricRefreshPending = false;
            ++g_metricRefreshSettlePasses;
            Wh_Log(L"Running delayed tray metric refresh.");
            ApplyXamlButtons();
            ApplyHoverBackgroundMetrics(g_bluetoothButton);
            ApplyHoverBackgroundMetrics(g_networkButton);
            ApplyHoverBackgroundMetrics(g_soundButton);
            ApplyHoverBackgroundMetrics(g_compactGroupedButton);
            if (g_trayPanel) {
                g_trayPanel.InvalidateMeasure();
                g_trayPanel.InvalidateArrange();
                g_trayPanel.UpdateLayout();
            }
            if (g_metricRefreshSettlePasses < 3) {
                g_metricRefreshPending = true;
                g_metricRefreshTimer.Start();
            }
        });
    }
    g_metricRefreshTimer.Start();
}

static void RefreshInjectedButtonMetrics() {
    if (!g_trayPanel || !g_trayControlCenterButton) {
        return;
    }

    const double oldWidth = g_trayButtonWidth;
    const double oldHeight = g_trayButtonHeight;
    CaptureTrayButtonMetricsFromPanel(g_trayPanel,
                                      g_trayControlCenterButton);
    if (oldWidth == g_trayButtonWidth && oldHeight == g_trayButtonHeight) {
        return;
    }

    Wh_Log(L"Tray metrics changed %.1fx%.1f -> %.1fx%.1f; updating injected "
           L"buttons.", oldWidth, oldHeight, g_trayButtonWidth,
           g_trayButtonHeight);
    ApplyTrayButtonMetrics(g_bluetoothButton);
    ApplyTrayButtonMetrics(g_networkButton);
    ApplyTrayButtonMetrics(g_soundButton);
    ApplyTrayButtonMetrics(g_compactGroupedButton);
    ApplyHoverBackgroundMetrics(g_bluetoothButton);
    ApplyHoverBackgroundMetrics(g_networkButton);
    ApplyHoverBackgroundMetrics(g_soundButton);
    ApplyHoverBackgroundMetrics(g_compactGroupedButton);
    if (g_trayPanel) {
        g_trayPanel.InvalidateMeasure();
        g_trayPanel.InvalidateArrange();
        g_trayPanel.UpdateLayout();
    }
    // Explorer updates the taskbar size and the private tray template in
    // separate layout passes.  Recreate our injected controls after the native
    // tray has settled so BackgroundBorder gets measured against the new size.
    ScheduleMetricRefresh();
}

static void AttachTaskbarSizeRefreshHandlers(
    wux::FrameworkElement const& trayElement,
    wux::FrameworkElement const& controlCenterButton) {
    auto handler = [](wf::IInspectable const& sender,
                      wux::SizeChangedEventArgs const& args) {
        const auto oldSize = args.PreviousSize();
        const auto newSize = args.NewSize();
        if (oldSize.Width == newSize.Width && oldSize.Height == newSize.Height) {
            return;
        }

        auto element = sender.try_as<wux::FrameworkElement>();
        Wh_Log(L"Taskbar tray size changed on %s#%s %.1fx%.1f -> %.1fx%.1f; scheduling metric refresh.",
               element ? winrt::get_class_name(element).c_str() : L"",
               element ? element.Name().c_str() : L"", oldSize.Width,
               oldSize.Height, newSize.Width, newSize.Height);
        ScheduleMetricRefresh();
    };

    try {
        if (g_sizeRefreshTrayElement) {
            g_sizeRefreshTrayElement.SizeChanged(g_sizeRefreshTrayToken);
            g_sizeRefreshTrayElement = nullptr;
            g_sizeRefreshTrayToken = {};
        }
        if (g_sizeRefreshControlCenterButton) {
            g_sizeRefreshControlCenterButton.SizeChanged(
                g_sizeRefreshControlCenterToken);
            g_sizeRefreshControlCenterButton = nullptr;
            g_sizeRefreshControlCenterToken = {};
        }
        if (trayElement) {
            g_sizeRefreshTrayToken = trayElement.SizeChanged(handler);
            g_sizeRefreshTrayElement = trayElement;
        }
        if (controlCenterButton) {
            g_sizeRefreshControlCenterToken =
                controlCenterButton.SizeChanged(handler);
            g_sizeRefreshControlCenterButton = controlCenterButton;
        }
    } catch (...) {
        Wh_Log(L"AttachTaskbarSizeRefreshHandlers failed: 0x%08X",
               winrt::to_hresult());
    }
}

static void UpdateDynamicXamlIcons();

static bool RefreshTaskbarLayoutIfRebuilt() {
    HWND taskbarWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!taskbarWnd) {
        return false;
    }

    auto xamlRoot = GetTaskbarXamlRoot(taskbarWnd);
    if (!xamlRoot) {
        return false;
    }

    auto root = xamlRoot.Content().try_as<wux::FrameworkElement>();
    if (!root) {
        return false;
    }

    auto currentButton = FindControlCenterButton(root);
    auto currentParent = currentButton
                             ? FindAncestorFrameworkElement(currentButton)
                             : nullptr;
    auto currentPanel = currentParent.try_as<wuc::Panel>();
    if (!currentPanel || !currentButton) {
        return false;
    }

    if (currentPanel == g_trayPanel &&
        currentButton == g_trayControlCenterButton) {
        return false;
    }

    Wh_Log(L"Detected taskbar tray XAML rebuild; reinjecting separated buttons.");
    g_taskbarWnd = taskbarWnd;
    return ApplyXamlButtons();
}

static void EnsureUpdateTimer() {
    if (g_updateTimer) {
        return;
    }

    g_updateTimer = wux::DispatcherTimer();
    g_updateTimer.Interval(std::chrono::milliseconds(500));
    g_updateTimer.Tick([](wf::IInspectable const&, wf::IInspectable const&) {
        UpdateDynamicXamlIcons();
        if (!RefreshTaskbarLayoutIfRebuilt()) {
            RefreshInjectedButtonMetrics();
        }
        if (!GroupedButtonModeIs(L"native") && g_originalGroupedButton) {
            HideOriginalGroupedButton(g_originalGroupedButton);
        }
    });
    g_updateTimer.Start();
}

static winrt::hstring GetNetworkGlyph(NetworkState const& state) {
    switch (state.kind) {
        case NetworkKind::Ethernet:
            return L"\xE839";
        case NetworkKind::Wifi:
            if (state.signal <= 25) {
                return L"\xE872";
            }
            if (state.signal <= 50) {
                return L"\xE873";
            }
            if (state.signal <= 75) {
                return L"\xE874";
            }
            return L"\xE701";
        case NetworkKind::WifiConnecting: {
            static PCWSTR frames[] = {L"\xE873", L"\xEAA5", L"\xEAA8"};
            winrt::hstring glyph = frames[g_wifiConnectingFrame %
                                          ARRAYSIZE(frames)];
            ++g_wifiConnectingFrame;
            return glyph;
        }
        case NetworkKind::WifiDisconnected:
            return L"\xF384";
        case NetworkKind::WifiDisabled:
            return L"\xF384";
        case NetworkKind::Disconnected:
        default:
            return L"\xF384";
    }
}

static winrt::hstring GetSoundOutputDeviceGlyph(SoundState const& state) {
    if (ContainsCI(state.outputName, L"buds") ||
        ContainsCI(state.outputName, L"earbuds")) {
        return L"\xF4C0";
    }

    switch (state.outputFormFactor) {
        case Headphones:
        case Headset:
        case Handset:
            return L"\xE7F6";
        case DigitalAudioDisplayDevice:
            return L"\xE7F3";
        case Speakers:
        case LineLevel:
        case UnknownDigitalPassthrough:
        case SPDIF:
            return L"\xE7F5";
        default:
            return L"\xE767";
    }
}

static winrt::hstring GetSoundVolumeGlyph(SoundState const& state) {
    if (state.volume <= 0.001f) {
        return L"\xE992";
    }
    if (state.volume < 0.34f) {
        return L"\xE993";
    }
    if (state.volume < 0.67f) {
        return L"\xE994";
    }
    return L"\xE767";
}

static winrt::hstring GetSoundGlyph(SoundState const& state) {
    if (!state.available || state.muted) {
        return L"\xE74F";
    }

    if (g_settings.soundIconFollowsOutputDevice) {
        return GetSoundOutputDeviceGlyph(state);
    }

    return GetSoundVolumeGlyph(state);
}

static std::wstring GetNetworkTooltip(NetworkState const& state) {
    auto accessLine = [&state]() {
        return state.internetAccess ? L"Internet access" : L"No internet access";
    };

    switch (state.kind) {
        case NetworkKind::Ethernet:
            return std::wstring(L"Ethernet\n") + accessLine();
        case NetworkKind::Wifi: {
            std::wstring name = state.name.empty() ? L"Wi-Fi" : state.name;
            wchar_t buffer[256]{};
            swprintf_s(buffer, L"%s: %lu%%\n%s", name.c_str(), state.signal,
                       accessLine());
            return buffer;
        }
        case NetworkKind::WifiConnecting: {
            if (!state.name.empty()) {
                return std::wstring(L"Connecting to ") + state.name + L"\n" +
                       accessLine();
            }
            return std::wstring(L"Connecting to Wi-Fi\n") + accessLine();
        }
        case NetworkKind::WifiDisconnected:
            return L"No internet access\nNo connections available";
        case NetworkKind::WifiDisabled:
            return L"No internet access\nNo connections available";
        case NetworkKind::Disconnected:
        default:
            return L"No internet access";
    }
}

namespace bt = winrt::Windows::Devices::Bluetooth;
namespace de = winrt::Windows::Devices::Enumeration;
static wf::IAsyncOperation<de::DeviceInformationCollection> g_btQueries[2]{nullptr, nullptr};
static std::wstring g_btConnectedNames;
static size_t g_btConnectedCount = 0;
static ULONGLONG g_btQueryTick = 0;

static std::wstring GetBluetoothTooltip(bool available) {
    if (!available) {
        for (auto& query : g_btQueries) {
            if (query) query.Cancel();
            query = nullptr;
        }
        g_btConnectedNames.clear();
        g_btConnectedCount = 0;
        g_btQueryTick = 0;
        return L"Bluetooth is off";
    }
    try {
        if (g_btQueries[0] && g_btQueries[1] &&
            g_btQueries[0].Status() != wf::AsyncStatus::Started &&
            g_btQueries[1].Status() != wf::AsyncStatus::Started) {
            std::unordered_map<std::wstring, std::wstring> devices;
            for (auto& query : g_btQueries) {
                if (query.Status() == wf::AsyncStatus::Completed) {
                    for (auto const& device : query.GetResults()) {
                        auto props = device.Properties();
                        auto connected = props.TryLookup(L"System.Devices.Aep.IsConnected");
                        if (!connected || !winrt::unbox_value_or<bool>(connected, false)) continue;
                        auto address = props.TryLookup(L"System.Devices.Aep.DeviceAddress");
                        std::wstring key = address ? winrt::unbox_value_or<winrt::hstring>(address, L"").c_str() : L"";
                        if (key.empty()) key = device.Id().c_str();
                        devices[ToLower(key)] = device.Name().c_str();
                    }
                } else {
                    Wh_Log(L"Bluetooth connected-endpoint query failed: 0x%08X", query.ErrorCode().value);
                }
                query = nullptr;
            }
            g_btConnectedNames.clear();
            g_btConnectedCount = devices.size();
            for (auto const& entry : devices) {
                std::wstring name = entry.second;
                for (auto& character : name) {
                    if (character == L'\r' || character == L'\n' ||
                        character == L'\u2028' || character == L'\u2029') {
                        character = L' ';
                    }
                }
                name = Trim(name);
                g_btConnectedNames += L"\n- " +
                    (name.empty() ? std::wstring(L"Bluetooth device") : name);
            }
        }
        if (!g_btQueries[0] && GetTickCount64() - g_btQueryTick >= 1000) {
            auto properties = winrt::single_threaded_vector<winrt::hstring>({
                L"System.Devices.Aep.IsConnected", L"System.Devices.Aep.DeviceAddress"});
            g_btQueries[0] = de::DeviceInformation::FindAllAsync(
                bt::BluetoothDevice::GetDeviceSelectorFromConnectionStatus(bt::BluetoothConnectionStatus::Connected),
                properties, de::DeviceInformationKind::AssociationEndpoint);
            g_btQueries[1] = de::DeviceInformation::FindAllAsync(
                bt::BluetoothLEDevice::GetDeviceSelectorFromConnectionStatus(bt::BluetoothConnectionStatus::Connected),
                properties, de::DeviceInformationKind::AssociationEndpoint);
            g_btQueryTick = GetTickCount64();
        }
    } catch (...) {
        for (auto& query : g_btQueries) query = nullptr;
        g_btConnectedNames.clear();
        g_btConnectedCount = 0;
        g_btQueryTick = GetTickCount64();
        Wh_Log(L"Bluetooth tooltip query failed: 0x%08X", winrt::to_hresult());
    }
    return g_btConnectedNames.empty()
        ? L"Bluetooth\n\nNo devices connected"
        : L"Bluetooth\n\nConnected devices (" + std::to_wstring(g_btConnectedCount) + L"):" + g_btConnectedNames;
}

static std::wstring GetFriendlyMediaAppName(std::wstring appId) {
    appId = Trim(appId);
    if (appId.empty()) {
        return L"Media app";
    }

    std::wstring lower = ToLower(appId);
    if (lower.find(L"spotify") != std::wstring::npos) return L"Spotify";
    if (lower.find(L"chrome") != std::wstring::npos) return L"Google Chrome";
    if (lower.find(L"msedge") != std::wstring::npos ||
        lower.find(L"microsoftedge") != std::wstring::npos) {
        return L"Microsoft Edge";
    }
    if (lower.find(L"firefox") != std::wstring::npos) return L"Firefox";
    if (lower.find(L"vlc") != std::wstring::npos) return L"VLC";
    if (lower.find(L"zunemusic") != std::wstring::npos ||
        lower.find(L"mediaplayer") != std::wstring::npos) {
        return L"Media Player";
    }

    size_t bang = appId.find(L'!');
    if (bang != std::wstring::npos) {
        appId.resize(bang);
    }

    size_t slash = appId.find_last_of(L"\\/");
    if (slash != std::wstring::npos && slash + 1 < appId.size()) {
        appId = appId.substr(slash + 1);
    }

    if (EndsWithCI(appId, L".exe")) {
        appId.resize(appId.size() - 4);
    }

    size_t underscore = appId.find(L'_');
    if (underscore != std::wstring::npos) {
        appId.resize(underscore);
    }

    size_t dot = appId.find_last_of(L'.');
    if (dot != std::wstring::npos && dot + 1 < appId.size()) {
        appId = appId.substr(dot + 1);
    }

    appId = Trim(appId);
    if (!appId.empty()) {
        appId[0] = static_cast<wchar_t>(towupper(appId[0]));
    }
    return appId.empty() ? L"Media app" : appId;
}

static void StoreMediaTooltipInfo(MediaTooltipInfo const& info) {
    AcquireSRWLockExclusive(&g_mediaTooltipLock);
    g_mediaTooltipInfo = info;
    ReleaseSRWLockExclusive(&g_mediaTooltipLock);
}

static MediaTooltipInfo LoadMediaTooltipInfo() {
    AcquireSRWLockShared(&g_mediaTooltipLock);
    MediaTooltipInfo info = g_mediaTooltipInfo;
    ReleaseSRWLockShared(&g_mediaTooltipLock);
    return info;
}

static MediaTooltipInfo GetCurrentlyPlayingMediaInfo() {
    if (!g_settings.showCurrentlyPlayingInSoundTooltip) {
        return {};
    }

    const ULONGLONG now = GetTickCount64();
    const ULONGLONG previous = g_lastMediaTooltipQueryTick.load();
    if ((!previous || now - previous >= 1000) &&
        !g_mediaTooltipQueryInProgress.exchange(true)) {
        g_lastMediaTooltipQueryTick.store(now);
        std::thread([]() {
            MediaTooltipInfo info{};
            try {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);
                auto manager =
                    wmc::GlobalSystemMediaTransportControlsSessionManager::
                        RequestAsync()
                            .get();
                auto session = manager ? manager.GetCurrentSession() : nullptr;
                if (session) {
                    auto playbackInfo = session.GetPlaybackInfo();
                    const auto status =
                        playbackInfo
                            ? playbackInfo.PlaybackStatus()
                            : wmc::GlobalSystemMediaTransportControlsSessionPlaybackStatus::
                                  Closed;
                    if (status ==
                            wmc::GlobalSystemMediaTransportControlsSessionPlaybackStatus::
                                Playing ||
                        status ==
                            wmc::GlobalSystemMediaTransportControlsSessionPlaybackStatus::
                                Paused) {
                        auto properties =
                            session.TryGetMediaPropertiesAsync().get();
                        info.title = Trim(properties.Title().c_str());
                        info.artist = Trim(properties.Artist().c_str());
                        if (info.artist.empty()) {
                            info.artist = Trim(properties.AlbumArtist().c_str());
                        }
                        if (!info.title.empty() || !info.artist.empty()) {
                            info.hasMedia = true;
                            info.paused =
                                status ==
                                wmc::GlobalSystemMediaTransportControlsSessionPlaybackStatus::
                                    Paused;
                            info.appName = GetFriendlyMediaAppName(
                                session.SourceAppUserModelId().c_str());
                        }
                    }
                }
            } catch (...) {
                Wh_Log(L"Currently playing tooltip query failed: 0x%08X",
                       winrt::to_hresult());
            }
            StoreMediaTooltipInfo(info);
            g_mediaTooltipQueryInProgress.store(false);
        }).detach();
    }

    return LoadMediaTooltipInfo();
}

static std::wstring GetMediaTooltipTrackText(MediaTooltipInfo const& media) {
    if (!media.artist.empty() && !media.title.empty()) {
        return media.artist + L" - " + media.title;
    }

    if (!media.title.empty()) {
        return media.title;
    }

    if (!media.artist.empty()) {
        return media.artist;
    }

    return L"Unknown";
}

static std::wstring GetSoundTooltip(SoundState const& state) {
    if (!state.available) {
        return L"No audio output device";
    }

    int percent = static_cast<int>(state.volume * 100.0f + 0.5f);
    if (percent < 0) {
        percent = 0;
    } else if (percent > 100) {
        percent = 100;
    }

    wchar_t buffer[256]{};
    std::wstring name =
        state.outputName.empty() ? L"Volume" : state.outputName;
    std::wstring tooltip;
    if (state.muted) {
        swprintf_s(buffer, L"%s: Muted", name.c_str());
    } else {
        swprintf_s(buffer, L"%s: %d%%", name.c_str(), percent);
    }
    tooltip = buffer;

    MediaTooltipInfo media = GetCurrentlyPlayingMediaInfo();
    if (media.hasMedia) {
        tooltip += media.paused ? L"\n\nCurrently playing (paused):\n"
                                : L"\n\nCurrently playing:\n";
        tooltip += GetMediaTooltipTrackText(media);
        if (!media.appName.empty()) {
            tooltip += L"\n\nSource: ";
            tooltip += media.appName;
        }
    }

    return tooltip;
}

static void UpdateDynamicXamlIcons() {
    try {
        auto primaryBrush = MakeIconBrush();
        auto underlayBrush = MakeUnderlayBrush();

        if (g_bluetoothIcon.primary) {
            const bool bluetoothAvailable = IsBluetoothAvailable();
            if (g_bluetoothButton) {
                g_bluetoothButton.Visibility(wux::Visibility::Visible);
                g_bluetoothButton.Opacity(1.0);
            }
            g_bluetoothIcon.primary.Visibility(wux::Visibility::Visible);
            g_bluetoothIcon.primary.Opacity(1.0);
            g_bluetoothIcon.primary.Glyph(L"\xE702");
            g_bluetoothIcon.primary.Foreground(bluetoothAvailable
                                                   ? primaryBrush
                                                   : underlayBrush);
            SetCachedTrayToolTip(
                g_bluetoothButton, g_bluetoothTooltipCache,
                GetBluetoothTooltip(bluetoothAvailable));
            if (g_bluetoothIcon.underlay) {
                g_bluetoothIcon.underlay.Visibility(wux::Visibility::Collapsed);
            }
            if (g_bluetoothIcon.overlay) {
                g_bluetoothIcon.overlay.Glyph(L"\u2715");
                g_bluetoothIcon.overlay.FontFamily(wuxm::FontFamily(L"Segoe UI"));
                g_bluetoothIcon.overlay.FontSize(12);
                g_bluetoothIcon.overlay.FontWeight({200}); // ExtraLight
                g_bluetoothIcon.overlay.Foreground(primaryBrush);
                g_bluetoothIcon.overlay.Visibility(
                    bluetoothAvailable ? wux::Visibility::Collapsed
                                       : wux::Visibility::Visible);
            }
        }

        if (g_networkIcon.primary) {
            NetworkState state = GetNetworkState();
            g_networkIcon.primary.Glyph(GetNetworkGlyph(state));
            g_networkIcon.primary.Foreground(primaryBrush);
            SetCachedTrayToolTip(g_networkButton, g_networkTooltipCache,
                                 GetNetworkTooltip(state));
            if (g_networkIcon.underlay) {
                g_networkIcon.underlay.Glyph(L"\xE701");
                g_networkIcon.underlay.Foreground(underlayBrush);
                g_networkIcon.underlay.Visibility(
                    state.kind == NetworkKind::Wifi ||
                            state.kind == NetworkKind::WifiConnecting
                        ? wux::Visibility::Visible
                        : wux::Visibility::Collapsed);
            }
            if (g_networkIcon.overlay) {
                g_networkIcon.overlay.Visibility(wux::Visibility::Collapsed);
            }
        }

        if (g_soundIcon.primary) {
            SoundState state = GetSoundState();
            const bool useOutputDeviceGlyph =
                g_settings.soundIconFollowsOutputDevice && state.available;
            const bool layerMuteOverOutput =
                useOutputDeviceGlyph && state.muted;
            g_soundIcon.primary.Glyph(layerMuteOverOutput
                                          ? GetSoundOutputDeviceGlyph(state)
                                          : GetSoundGlyph(state));
            g_soundIcon.primary.Foreground(layerMuteOverOutput ? underlayBrush
                                                               : primaryBrush);
            SetCachedTrayToolTip(g_soundButton, g_soundTooltipCache,
                                 GetSoundTooltip(state));
            if (g_soundIcon.underlay) {
                g_soundIcon.underlay.Glyph(L"\xEBC5");
                g_soundIcon.underlay.Foreground(underlayBrush);
                g_soundIcon.underlay.Visibility(
                    state.available && !state.muted && !useOutputDeviceGlyph
                        ? wux::Visibility::Visible
                        : wux::Visibility::Collapsed);
            }
            if (g_soundIcon.overlay) {
                g_soundIcon.overlay.Glyph(L"\xE74F");
                g_soundIcon.overlay.Foreground(primaryBrush);
                g_soundIcon.overlay.Visibility(
                    layerMuteOverOutput ? wux::Visibility::Visible
                                        : wux::Visibility::Collapsed);
            }
        }
    } catch (...) {
        Wh_Log(L"UpdateDynamicXamlIcons error: 0x%08X", winrt::to_hresult());
    }
}

enum class ButtonKind {
    Bluetooth,
    Network,
    Sound,
    QuickSettings,
};

static PCWSTR ButtonKindName(ButtonKind kind) {
    switch (kind) {
        case ButtonKind::Bluetooth:
            return L"bluetooth";
        case ButtonKind::Network:
            return L"network";
        case ButtonKind::Sound:
            return L"sound";
        case ButtonKind::QuickSettings:
            return L"quick_settings";
        default:
            return L"unknown";
    }
}

enum class TrayContextCommand : UINT {
    BluetoothAddDevice = 1,
    BluetoothAllowDevice,
    BluetoothDevices,
    BluetoothSendFile,
    BluetoothReceiveFile,
    BluetoothPan,
    BluetoothSettings,
    NetworkAvailable,
    NetworkWifiSettings,
    NetworkSettings,
    NetworkAirplaneMode,
    SoundSettings,
    SoundMixer,
    SoundSpeakerSetup,
    SoundSounds,
    SoundTroubleshoot,
};

struct TrayContextMenuItem {
    PCWSTR text;
    TrayContextCommand command;
};

static std::vector<TrayContextMenuItem> GetTrayContextMenuItems(
    ButtonKind kind) {
    switch (kind) {
        case ButtonKind::Bluetooth:
        case ButtonKind::Sound:
            // These menus contain separators and submenus, so they are built
            // by the framework-specific functions below.
            return {};
        case ButtonKind::Network:
            return {
                {L"Show available networks", TrayContextCommand::NetworkAvailable},
                {L"Wi-Fi settings", TrayContextCommand::NetworkWifiSettings},
                {L"Network & Internet settings", TrayContextCommand::NetworkSettings},
                {L"Airplane mode settings", TrayContextCommand::NetworkAirplaneMode},
            };
        default:
            return {};
    }
}

static void ExecuteTrayContextCommand(TrayContextCommand command) {
    switch (command) {
        case TrayContextCommand::BluetoothAddDevice:
            ExecuteAction(L"ms-settings:bluetooth");
            break;
        case TrayContextCommand::BluetoothAllowDevice:
            OpenBluetoothOptions();
            break;
        case TrayContextCommand::BluetoothDevices:
            OpenBluetooth();
            break;
        case TrayContextCommand::BluetoothSendFile:
            OpenBluetoothFileTransfer(true);
            break;
        case TrayContextCommand::BluetoothReceiveFile:
            OpenBluetoothFileTransfer(false);
            break;
        case TrayContextCommand::BluetoothPan:
            ExecuteAction(L"ms-settings:network");
            break;
        case TrayContextCommand::BluetoothSettings:
            ExecuteAction(L"ms-settings:bluetooth");
            break;
        case TrayContextCommand::NetworkAvailable:
            OpenNetwork();
            break;
        case TrayContextCommand::NetworkWifiSettings:
            ExecuteAction(L"ms-settings:network-wifi");
            break;
        case TrayContextCommand::NetworkSettings:
            ExecuteAction(L"ms-settings:network");
            break;
        case TrayContextCommand::NetworkAirplaneMode:
            ExecuteAction(L"ms-settings:network-airplanemode");
            break;
        case TrayContextCommand::SoundSettings:
            ExecuteAction(L"ms-settings:sound");
            break;
        case TrayContextCommand::SoundMixer:
            OpenVolumeMixer();
            break;
        case TrayContextCommand::SoundSpeakerSetup:
            OpenSpeakerSetup();
            break;
        case TrayContextCommand::SoundSounds:
            OpenSoundsControlPanel();
            break;
        case TrayContextCommand::SoundTroubleshoot:
            ExecuteAction(L"ms-settings:troubleshoot");
            break;
        default:
            break;
    }
}

static void AppendWin32ContextItem(HMENU menu, PCWSTR text,
                                   TrayContextCommand command) {
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(command), text);
}

static void AppendWin32BluetoothContextMenu(HMENU menu) {
    AppendWin32ContextItem(menu, L"Add a Bluetooth Device",
                           TrayContextCommand::BluetoothAddDevice);
    // This entry is present in the legacy Bluetooth menu, but Windows 11 no
    // longer exposes its old inbound-connection toggle. Keep it disabled
    // rather than pretending that the legacy Options page changes it.
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"Allow a Device to Connect");
    AppendWin32ContextItem(menu, L"Show Bluetooth Devices",
                           TrayContextCommand::BluetoothDevices);
    SetMenuDefaultItem(menu, static_cast<UINT>(TrayContextCommand::BluetoothDevices),
                       FALSE);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendWin32ContextItem(menu, L"Send a File",
                           TrayContextCommand::BluetoothSendFile);
    AppendWin32ContextItem(menu, L"Receive a File",
                           TrayContextCommand::BluetoothReceiveFile);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendWin32ContextItem(menu, L"Join a Personal Area Network",
                           TrayContextCommand::BluetoothPan);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendWin32ContextItem(menu, L"Open Settings",
                           TrayContextCommand::BluetoothSettings);
}

constexpr UINT kAudioOutputMenuIdFirst = 0x7000;

static void AppendWin32SoundContextMenu(
    HMENU menu, std::vector<AudioOutputEndpoint> const& outputs) {
    AppendWin32ContextItem(menu, L"Open Sound settings",
                           TrayContextCommand::SoundSettings);
    AppendWin32ContextItem(menu, L"Open Volume mixer",
                           TrayContextCommand::SoundMixer);

    HMENU outputMenu = CreatePopupMenu();
    if (outputs.empty()) {
        AppendMenuW(outputMenu, MF_STRING | MF_GRAYED, 0,
                    L"No output devices");
    } else {
        for (size_t i = 0; i < outputs.size(); ++i) {
            const UINT flags = MF_STRING | (outputs[i].isDefault ? MF_CHECKED : 0);
            AppendMenuW(outputMenu, flags, kAudioOutputMenuIdFirst +
                                            static_cast<UINT>(i),
                        outputs[i].name.c_str());
        }
    }
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(outputMenu),
                L"Output device");

    HMENU speakerSetupMenu = CreatePopupMenu();
    AppendWin32ContextItem(speakerSetupMenu, L"Configure speakers",
                           TrayContextCommand::SoundSpeakerSetup);
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(speakerSetupMenu),
                L"Speaker setup (Stereo)");
    AppendWin32ContextItem(menu, L"Sounds", TrayContextCommand::SoundSounds);
    AppendWin32ContextItem(menu, L"Troubleshoot sound problems",
                           TrayContextCommand::SoundTroubleshoot);
}

static void ShowWin32TrayContextMenu(ButtonKind kind) {
    HMENU menu = CreatePopupMenu();
    if (!menu) {
        Wh_Log(L"CreatePopupMenu failed: %lu", GetLastError());
        return;
    }

    std::vector<AudioOutputEndpoint> audioOutputs;
    if (kind == ButtonKind::Bluetooth) {
        AppendWin32BluetoothContextMenu(menu);
    } else if (kind == ButtonKind::Sound) {
        audioOutputs = GetActiveAudioOutputEndpoints();
        AppendWin32SoundContextMenu(menu, audioOutputs);
    } else {
        for (auto const& item : GetTrayContextMenuItems(kind)) {
            AppendWin32ContextItem(menu, item.text, item.command);
        }
    }

    POINT point{};
    GetCursorPos(&point);
    HWND owner = g_taskbarWnd ? g_taskbarWnd : FindWindowW(L"Shell_TrayWnd", nullptr);
    if (owner) {
        SetForegroundWindow(owner);
    }
    const UINT selected = TrackPopupMenuEx(
        menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, point.x, point.y,
        owner, nullptr);
    DestroyMenu(menu);

    if (selected >= kAudioOutputMenuIdFirst &&
        selected < kAudioOutputMenuIdFirst + audioOutputs.size()) {
        SetDefaultAudioOutput(
            audioOutputs[selected - kAudioOutputMenuIdFirst].id);
    } else if (selected) {
        ExecuteTrayContextCommand(static_cast<TrayContextCommand>(selected));
    }
}

static void AppendWinUiContextItem(
    wfc::IVector<wuc::MenuFlyoutItemBase> const& items, PCWSTR text,
    TrayContextCommand command) {
    wuc::MenuFlyoutItem item;
    item.Text(text);
    item.Click([command](wf::IInspectable const&,
                         wux::RoutedEventArgs const&) {
        ExecuteTrayContextCommand(command);
    });
    items.Append(item);
}

static void AppendWinUiSeparator(
    wfc::IVector<wuc::MenuFlyoutItemBase> const& items) {
    items.Append(wuc::MenuFlyoutSeparator());
}

static void AppendWinUiBluetoothContextMenu(wuc::MenuFlyout const& flyout) {
    auto items = flyout.Items();
    AppendWinUiContextItem(items, L"Add a Bluetooth Device",
                           TrayContextCommand::BluetoothAddDevice);
    wuc::MenuFlyoutItem allowDevice;
    allowDevice.Text(L"Allow a Device to Connect");
    allowDevice.IsEnabled(false);
    items.Append(allowDevice);
    AppendWinUiContextItem(items, L"Show Bluetooth Devices",
                           TrayContextCommand::BluetoothDevices);
    AppendWinUiSeparator(items);
    AppendWinUiContextItem(items, L"Send a File",
                           TrayContextCommand::BluetoothSendFile);
    AppendWinUiContextItem(items, L"Receive a File",
                           TrayContextCommand::BluetoothReceiveFile);
    AppendWinUiSeparator(items);
    AppendWinUiContextItem(items, L"Join a Personal Area Network",
                           TrayContextCommand::BluetoothPan);
    AppendWinUiSeparator(items);
    AppendWinUiContextItem(items, L"Open Settings",
                           TrayContextCommand::BluetoothSettings);
}

static void AppendWinUiSoundContextMenu(wuc::MenuFlyout const& flyout) {
    auto items = flyout.Items();
    AppendWinUiContextItem(items, L"Open Sound settings",
                           TrayContextCommand::SoundSettings);
    AppendWinUiContextItem(items, L"Open Volume mixer",
                           TrayContextCommand::SoundMixer);

    wuc::MenuFlyoutSubItem outputSubmenu;
    outputSubmenu.Text(L"Output device");
    auto outputs = GetActiveAudioOutputEndpoints();
    if (outputs.empty()) {
        wuc::MenuFlyoutItem noOutput;
        noOutput.Text(L"No output devices");
        noOutput.IsEnabled(false);
        outputSubmenu.Items().Append(noOutput);
    } else {
        for (auto const& output : outputs) {
            wuc::ToggleMenuFlyoutItem outputItem;
            outputItem.Text(output.name);
            outputItem.IsChecked(output.isDefault);
            outputItem.Click([id = output.id](wf::IInspectable const&,
                                              wux::RoutedEventArgs const&) {
                SetDefaultAudioOutput(id);
            });
            outputSubmenu.Items().Append(outputItem);
        }
    }
    items.Append(outputSubmenu);

    wuc::MenuFlyoutSubItem speakerSetupSubmenu;
    speakerSetupSubmenu.Text(L"Speaker setup (Stereo)");
    AppendWinUiContextItem(speakerSetupSubmenu.Items(), L"Configure speakers",
                           TrayContextCommand::SoundSpeakerSetup);
    items.Append(speakerSetupSubmenu);
    AppendWinUiContextItem(items, L"Sounds", TrayContextCommand::SoundSounds);
    AppendWinUiContextItem(items, L"Troubleshoot sound problems",
                           TrayContextCommand::SoundTroubleshoot);
}

static void PositionTrayMenuPopup(wux::XamlRoot const& root,
                                 wuc::MenuFlyout const& flyout) {
    RECT bar{}, host{};
    bool horizontal{}, first{};
    if (!root || !GetTaskbarGeometry(&bar, &host, &horizontal, &first)) return;
    MONITORINFO mi{sizeof(mi)};
    if (!GetMonitorInfoW(MonitorFromRect(&bar, MONITOR_DEFAULTTONEAREST), &mi)) return;
    const double scale = root.RasterizationScale();
    for (auto const& popup : wuxm::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
        auto presenter = popup.Child().try_as<wuc::MenuFlyoutPresenter>();
        if (!presenter || presenter.Items().Size() == 0 || flyout.Items().Size() == 0 ||
            presenter.Items().GetAt(0) != flyout.Items().GetAt(0)) continue;
        presenter.Name(L"SeparateTrayContextMenuPresenter");
        presenter.UpdateLayout();
        auto origin = presenter.TransformToVisual(root.Content()).TransformPoint({0, 0});
        const double width = presenter.ActualWidth() * scale;
        const double height = presenter.ActualHeight() * scale;
        if (width <= 0 || height <= 0) continue;
        double x = host.left + origin.X * scale;
        double y = host.top + origin.Y * scale;
        const double oldX = x, oldY = y;
        if (horizontal) {
            // Tray menus open inward from the screen's tray corner, with
            // their right edge inset from the monitor rather than the icon.
            x = mi.rcMonitor.right - 12.0 - width;
            y = first ? bar.bottom + 12.0 : bar.top - 12.0 - height;
        } else {
            x = first ? bar.right + 12.0 : bar.left - 12.0 - width;
            y = mi.rcMonitor.bottom - 12.0 - height;
        }
        x = (std::max)(mi.rcMonitor.left + 12.0,
                       (std::min)(x, mi.rcMonitor.right - 12.0 - width));
        y = (std::max)(mi.rcMonitor.top + 12.0,
                       (std::min)(y, mi.rcMonitor.bottom - 12.0 - height));
        popup.HorizontalOffset(popup.HorizontalOffset() + (x - oldX) / scale);
        popup.VerticalOffset(popup.VerticalOffset() + (y - oldY) / scale);
    }
}

static void ShowWinUiFlyoutNearTaskbar(wuc::MenuFlyout const& flyout,
                                       wux::FrameworkElement const& target) {
    RECT taskbarRect{};
    RECT hostRect{};
    bool horizontal = true;
    bool nearFirstEdge = true;
    if (!GetTaskbarGeometry(&taskbarRect, &hostRect, &horizontal,
                            &nearFirstEdge)) {
        flyout.ShowAt(target);
        return;
    }

    constexpr float kGap = 12.0f;
    wf::Point point{
        static_cast<float>(target.ActualWidth() / 2.0),
        static_cast<float>(target.ActualHeight() / 2.0)};
    wucp::FlyoutPlacementMode placement = wucp::FlyoutPlacementMode::Auto;

    if (horizontal) {
        if (nearFirstEdge) {
            point.Y = static_cast<float>(target.ActualHeight()) + kGap;
            placement = wucp::FlyoutPlacementMode::Bottom;
        } else {
            point.Y = -kGap;
            placement = wucp::FlyoutPlacementMode::Top;
        }
    } else {
        if (nearFirstEdge) {
            point.X = static_cast<float>(target.ActualWidth()) + kGap;
            placement = wucp::FlyoutPlacementMode::Right;
        } else {
            point.X = -kGap;
            placement = wucp::FlyoutPlacementMode::Left;
        }
    }

    wucp::FlyoutShowOptions options;
    options.Position(winrt::box_value(point).as<wf::IReference<wf::Point>>());
    options.Placement(placement);
    options.ShowMode(wucp::FlyoutShowMode::Standard);
    // Position the measured native presenter, rather than relying on ShowAt's
    // anchor, whose final placement includes framework offsets and clamping.
    auto weakRoot = winrt::make_weak(target.XamlRoot());
    flyout.Opened([weakRoot](auto const& sender, auto const&) {
        try {
            if (auto root = weakRoot.get()) PositionTrayMenuPopup(root, sender.template as<wuc::MenuFlyout>());
        } catch (...) {
            Wh_Log(L"Menu placement failed: 0x%08X", winrt::to_hresult());
        }
    });
    flyout.ShowAt(target, options);
}

static bool ShowWinUiTrayContextMenu(wux::FrameworkElement const& target,
                                     ButtonKind kind) {
    if (!target) {
        return false;
    }

    try {
        auto flyout = wuc::MenuFlyout();
        if (kind == ButtonKind::Bluetooth) {
            AppendWinUiBluetoothContextMenu(flyout);
        } else if (kind == ButtonKind::Sound) {
            AppendWinUiSoundContextMenu(flyout);
        } else {
            for (auto const& item : GetTrayContextMenuItems(kind)) {
                AppendWinUiContextItem(flyout.Items(), item.text, item.command);
            }
        }

        g_activeTrayContextFlyout = flyout;
        ShowWinUiFlyoutNearTaskbar(flyout, target);
        return true;
    } catch (...) {
        Wh_Log(L"WinUI context menu failed for %s: 0x%08X",
               ButtonKindName(kind), winrt::to_hresult());
        return false;
    }
}

static void ShowTrayContextMenu(wux::FrameworkElement const& target,
                                ButtonKind kind) {
    const bool useWin32 =
        _wcsicmp(g_settings.contextMenuFramework.c_str(), L"win32") == 0;
    if (useWin32 || !ShowWinUiTrayContextMenu(target, kind)) {
        ShowWin32TrayContextMenu(kind);
    }
}

static std::wstring const& TooltipCacheForButtonKind(ButtonKind kind) {
    static const std::wstring kQuickSettingsTooltip = L"Quick Settings";
    static const std::wstring kEmptyTooltip;
    switch (kind) {
        case ButtonKind::Bluetooth:
            return g_bluetoothTooltipCache;
        case ButtonKind::Network:
            return g_networkTooltipCache;
        case ButtonKind::Sound:
            return g_soundTooltipCache;
        case ButtonKind::QuickSettings:
            return kQuickSettingsTooltip;
        default:
            return kEmptyTooltip;
    }
}

static bool TryParseButtonKind(std::wstring const& rawToken,
                               ButtonKind* kind) {
    std::wstring token = ToLower(Trim(rawToken));
    if (token == L"bluetooth" || token == L"bt") {
        *kind = ButtonKind::Bluetooth;
        return true;
    }
    if (token == L"network" || token == L"wifi" || token == L"wi-fi") {
        *kind = ButtonKind::Network;
        return true;
    }
    if (token == L"sound" || token == L"audio" || token == L"volume") {
        *kind = ButtonKind::Sound;
        return true;
    }
    if (token == L"quick_settings" || token == L"quicksettings" ||
        token == L"control_center" || token == L"controlcenter" ||
        token == L"grouped") {
        *kind = ButtonKind::QuickSettings;
        return true;
    }
    return false;
}

static bool IsButtonKindVisible(ButtonKind kind) {
    switch (kind) {
        case ButtonKind::Bluetooth:
            return g_settings.showBluetoothButton;
        case ButtonKind::Network:
            return g_settings.showNetworkButton;
        case ButtonKind::Sound:
            return g_settings.showSoundButton;
        case ButtonKind::QuickSettings:
            return GroupedButtonModeIs(L"native") ||
                   GroupedButtonModeIs(L"compact");
        default:
            return false;
    }
}

static void AppendOrderedButtonKind(std::vector<ButtonKind>& order,
                                    bool used[],
                                    ButtonKind kind) {
    const size_t index = static_cast<size_t>(kind);
    if (index >= 4 || used[index] || !IsButtonKindVisible(kind)) {
        return;
    }

    used[index] = true;
    order.push_back(kind);
}

static std::vector<ButtonKind> GetVisibleButtonOrder() {
    std::vector<ButtonKind> order;
    bool used[4]{};

    size_t start = 0;
    while (start <= g_settings.buttonOrder.size()) {
        size_t comma = g_settings.buttonOrder.find(L',', start);
        std::wstring token = g_settings.buttonOrder.substr(
            start, comma == std::wstring::npos ? std::wstring::npos
                                               : comma - start);

        ButtonKind kind{};
        if (TryParseButtonKind(token, &kind)) {
            AppendOrderedButtonKind(order, used, kind);
        } else if (!Trim(token).empty()) {
            Wh_Log(L"Ignoring unknown buttonOrder entry: [%s].",
                   Trim(token).c_str());
        }

        if (comma == std::wstring::npos) {
            break;
        }
        start = comma + 1;
    }

    AppendOrderedButtonKind(order, used, ButtonKind::Bluetooth);
    AppendOrderedButtonKind(order, used, ButtonKind::Network);
    AppendOrderedButtonKind(order, used, ButtonKind::Sound);
    AppendOrderedButtonKind(order, used, ButtonKind::QuickSettings);

    std::wstring orderLog;
    for (auto kind : order) {
        if (!orderLog.empty()) {
            orderLog += L",";
        }
        orderLog += ButtonKindName(kind);
    }
    Wh_Log(L"Resolved visible tray button order: %s", orderLog.c_str());

    return order;
}

static int g_lastOpenedFlyoutButton = -1;
static ULONGLONG g_lastOpenedFlyoutTick = 0;
constexpr ULONGLONG kFlyoutToggleLifetimeMs = 5 * 60 * 1000;

struct ShellFlyoutSearch {
    HWND hwnd = nullptr;
};

static bool IsShellFlyoutHostProcess(DWORD processId) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                 processId);
    if (!process) {
        return false;
    }

    wchar_t path[MAX_PATH]{};
    DWORD length = ARRAYSIZE(path);
    const bool queried = QueryFullProcessImageNameW(process, 0, path, &length);
    CloseHandle(process);
    if (!queried) {
        return false;
    }

    PCWSTR fileName = wcsrchr(path, L'\\');
    fileName = fileName ? fileName + 1 : path;
    return _wcsicmp(fileName, L"ShellHost.exe") == 0 ||
           _wcsicmp(fileName, L"ShellExperienceHost.exe") == 0;
}

static BOOL CALLBACK FindShellFlyoutWindowProc(HWND hwnd, LPARAM lParam) {
    auto* search = reinterpret_cast<ShellFlyoutSearch*>(lParam);
    if (!search || !IsWindowVisible(hwnd) || IsIconic(hwnd)) {
        return TRUE;
    }

    wchar_t className[128]{};
    wchar_t title[256]{};
    GetClassNameW(hwnd, className, ARRAYSIZE(className));
    GetWindowTextW(hwnd, title, ARRAYSIZE(title));

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    if (!processId || !IsShellFlyoutHostProcess(processId)) {
        return TRUE;
    }

    RECT rect{};
    GetWindowRect(hwnd, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width < 100 || height < 100) {
        return TRUE;
    }

    Wh_Log(L"Visible shell flyout candidate hwnd=%p pid=%lu class=%s title=[%s] rect=%ld,%ld %ldx%ld.",
           hwnd, processId, className, title, rect.left, rect.top, width,
           height);
    search->hwnd = hwnd;
    return FALSE;
}

static HWND FindVisibleShellFlyoutWindow() {
    ShellFlyoutSearch search;
    EnumWindows(FindShellFlyoutWindowProc,
                reinterpret_cast<LPARAM>(&search));
    return search.hwnd;
}

static void SendEscapeToDismissFlyout(HWND flyoutWindow) {
    // Escape is how the native system flyouts dismiss themselves.  Only send
    // it after confirming that the matching shell flyout is visibly open.
    if (flyoutWindow) {
        SetForegroundWindow(flyoutWindow);
    }

    INPUT inputs[2]{};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_ESCAPE;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_ESCAPE;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

static bool SoundUsesQuickSettings() {
    return _wcsicmp(g_settings.soundClickAction.c_str(), L"sndvol") != 0;
}

static bool HandleTrayButtonClick(ButtonKind kind) {
    const int buttonIndex = static_cast<int>(kind);
    const ULONGLONG now = GetTickCount64();
    const bool canToggle = kind != ButtonKind::Sound || SoundUsesQuickSettings();

    if (canToggle && g_lastOpenedFlyoutButton == buttonIndex &&
        now - g_lastOpenedFlyoutTick <= kFlyoutToggleLifetimeMs) {
        if (HWND flyout = FindVisibleShellFlyoutWindow()) {
            Wh_Log(L"Repeated %d tray click: dismissing visible shell flyout %p.",
                   buttonIndex, flyout);
            SendEscapeToDismissFlyout(flyout);
            g_lastOpenedFlyoutButton = -1;
            g_lastOpenedFlyoutTick = 0;
            return true;
        }

        // The panel was dismissed independently, so this click should open it
        // again rather than treating a stale last-click record as a toggle.
        g_lastOpenedFlyoutButton = -1;
        g_lastOpenedFlyoutTick = 0;
    }

    if (kind == ButtonKind::Bluetooth) {
        OpenBluetooth();
    } else if (kind == ButtonKind::Network) {
        OpenNetwork();
    } else if (kind == ButtonKind::QuickSettings) {
        ExecuteAction(g_settings.groupedButtonAction);
    } else {
        OpenSound();
    }

    if (canToggle) {
        g_lastOpenedFlyoutButton = buttonIndex;
        g_lastOpenedFlyoutTick = now;
    } else {
        g_lastOpenedFlyoutButton = -1;
        g_lastOpenedFlyoutTick = 0;
    }
    return true;
}

// PointerPressed is handled for a middle click, but the private tray control
// can still raise Tapped afterwards. Keep a short per-button suppression
// window so middle-click never falls through to the normal click action.
static ULONGLONG g_suppressTapUntil[4]{};

static size_t ButtonKindIndex(ButtonKind kind) {
    return static_cast<size_t>(kind);
}

static void SuppressMiddleClickTap(ButtonKind kind) {
    g_suppressTapUntil[ButtonKindIndex(kind)] = GetTickCount64() + 750;
}

static bool ConsumeSuppressedTap(ButtonKind kind) {
    auto& until = g_suppressTapUntil[ButtonKindIndex(kind)];
    if (until && GetTickCount64() <= until) {
        until = 0;
        return true;
    }
    return false;
}

static wuc::FontIcon CreateTrayFontIcon(PCWSTR glyph, wuxm::Brush const& brush) {
    wuc::FontIcon icon;
    icon.FontFamily(wuxm::FontFamily(L"Segoe Fluent Icons"));
    icon.FontSize(16);
    icon.Glyph(glyph);
    icon.Foreground(brush);
    icon.HorizontalAlignment(wux::HorizontalAlignment::Center);
    icon.VerticalAlignment(wux::VerticalAlignment::Center);
    return icon;
}

static IconLayers CreateTrayIconLayers(PCWSTR primaryGlyph) {
    IconLayers layers;

    wuc::Grid host;
    host.Name(L"SeparateTrayIconLayers");
    host.HorizontalAlignment(wux::HorizontalAlignment::Center);
    host.VerticalAlignment(wux::VerticalAlignment::Center);
    host.Width(16);
    host.Height(16);

    layers.host = host;
    layers.underlay = CreateTrayFontIcon(L"", MakeUnderlayBrush());
    layers.primary = CreateTrayFontIcon(primaryGlyph, MakeIconBrush());
    layers.overlay = CreateTrayFontIcon(L"", MakeIconBrush());
    layers.underlay.Name(L"SeparateTrayIconUnderlay");
    layers.primary.Name(L"SeparateTrayIconPrimary");
    layers.overlay.Name(L"SeparateTrayIconOverlay");

    layers.underlay.Visibility(wux::Visibility::Collapsed);
    layers.overlay.Visibility(wux::Visibility::Collapsed);

    host.Children().Append(layers.underlay);
    host.Children().Append(layers.primary);
    host.Children().Append(layers.overlay);

    return layers;
}

static wux::FrameworkElement TryCreateNativeOmniButton(PCWSTR name) {
    try {
        auto loaded = wuxmk::XamlReader::Load(
            LR"(<SystemTray:OmniButton xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" xmlns:SystemTray="using:SystemTray" MinWidth="0" HorizontalAlignment="Center" VerticalAlignment="Center" HorizontalContentAlignment="Center" VerticalContentAlignment="Center"/>)");
        auto element = loaded.try_as<wux::FrameworkElement>();
        if (!element) {
            Wh_Log(L"XamlReader loaded object, but it is not FrameworkElement: %s.",
                   winrt::get_class_name(loaded).c_str());
            return nullptr;
        }

        element.Name(name);

        // XamlReader can construct the private SystemTray.OmniButton type, but
        // does not automatically give it the taskbar instance's style. Use the
        // live ControlCenterButton style first: it is the matching control
        // type and owns the BackgroundBorder/hover template that Taskbar
        // Styler can target.
        if (auto control = element.try_as<wuc::Control>()) {
            bool styleApplied = false;
            if (g_nativeGroupedButtonStyle) {
                try {
                    control.Style(g_nativeGroupedButtonStyle);
                    styleApplied = true;
                    Wh_Log(L"Applied the live ControlCenterButton style to OmniButton#%s.",
                           name);
                } catch (winrt::hresult_error const& e) {
                    Wh_Log(L"ControlCenterButton style is incompatible with OmniButton#%s: 0x%08X %s.",
                           name, e.code(), e.message().c_str());
                } catch (...) {
                    Wh_Log(L"ControlCenterButton style is incompatible with OmniButton#%s: 0x%08X.",
                           name, winrt::to_hresult());
                }
            }
            if (!styleApplied && g_nativeNotifyIconStyle) {
                try {
                    control.Style(g_nativeNotifyIconStyle);
                    styleApplied = true;
                    Wh_Log(L"Applied the live NotifyIconView style to OmniButton#%s.",
                           name);
                } catch (winrt::hresult_error const& e) {
                    Wh_Log(L"NotifyIconView style is incompatible with OmniButton#%s: 0x%08X %s.",
                           name, e.code(), e.message().c_str());
                } catch (...) {
                    Wh_Log(L"NotifyIconView style is incompatible with OmniButton#%s: 0x%08X.",
                           name, winrt::to_hresult());
                }
            }
            if (!styleApplied) {
                Wh_Log(L"Native OmniButton#%s was created before a compatible tray style was captured.",
                       name);
            }
        } else {
            Wh_Log(L"Native tray element %s#%s is not projected as Control; cannot apply its live style.",
                   winrt::get_class_name(element).c_str(), name);
        }

        ApplyTrayButtonMetrics(element);
        Wh_Log(L"Created native tray element through XamlReader: %s#%s",
               winrt::get_class_name(element).c_str(), element.Name().c_str());
        return element;
    } catch (winrt::hresult_error const& e) {
        Wh_Log(L"XamlReader SystemTray.OmniButton creation failed: 0x%08X %s",
               e.code(), e.message().c_str());
    } catch (...) {
        Wh_Log(L"XamlReader SystemTray.OmniButton creation failed: 0x%08X",
               winrt::to_hresult());
    }

    return nullptr;
}

static wux::FrameworkElement CreateFallbackButton(PCWSTR name) {
    wuc::Button button;
    button.Name(name);
    button.HorizontalAlignment(wux::HorizontalAlignment::Center);
    button.VerticalAlignment(wux::VerticalAlignment::Center);
    button.HorizontalContentAlignment(wux::HorizontalAlignment::Center);
    button.VerticalContentAlignment(wux::VerticalAlignment::Center);

    if (g_nativeGroupedButtonStyle) {
        try {
            button.Style(g_nativeGroupedButtonStyle);
            Wh_Log(L"Applied captured grouped-button style to fallback Button#%s.",
                   name);
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"Applying captured grouped-button style to Button#%s failed: 0x%08X %s",
                   name, e.code(), e.message().c_str());
            button.Padding({0, 0, 0, 0});
            button.BorderThickness({0, 0, 0, 0});
            button.Background(wuxm::SolidColorBrush(wu::Colors::Transparent()));
        } catch (...) {
            Wh_Log(L"Applying captured grouped-button style to Button#%s failed: 0x%08X",
                   name, winrt::to_hresult());
            button.Padding({0, 0, 0, 0});
            button.BorderThickness({0, 0, 0, 0});
            button.Background(wuxm::SolidColorBrush(wu::Colors::Transparent()));
        }
    } else {
        button.Padding({0, 0, 0, 0});
        button.BorderThickness({0, 0, 0, 0});
        button.Background(wuxm::SolidColorBrush(wu::Colors::Transparent()));
        Wh_Log(L"No captured grouped-button style; using plain fallback Button#%s.",
               name);
    }

    ApplyTrayButtonMetrics(button);
    return button;
}

static bool SetElementIcon(wux::FrameworkElement const& element,
                           wux::UIElement const& icon) {
    if (auto contentControl = element.try_as<wuc::ContentControl>()) {
        contentControl.Content(icon);
        Wh_Log(L"Set icon through ContentControl on %s#%s.",
               winrt::get_class_name(element).c_str(), element.Name().c_str());
        return true;
    }

    if (auto itemsControl = element.try_as<wuc::ItemsControl>()) {
        itemsControl.Items().Append(icon);
        Wh_Log(L"Set icon through ItemsControl.Items on %s#%s.",
               winrt::get_class_name(element).c_str(), element.Name().c_str());
        return true;
    }

    Wh_Log(L"Cannot set icon content on %s#%s: not ContentControl.",
           winrt::get_class_name(element).c_str(), element.Name().c_str());
    return false;
}

// OmniButton's stock template is designed around the grouped Quick Settings
// content: an ItemsPresenter containing a horizontal stack of several entries.
// A button made with XamlReader gets the same template, but its generated item
// host keeps the grouped layout's left bias. Centre the *template's item host*,
// leaving the OmniButton, its BackgroundBorder, and all of its native visual
// states untouched.
static void CenterNativeOmniButtonItemHost(wux::FrameworkElement const& button) {
    try {
        std::vector<wux::DependencyObject> stack;
        stack.push_back(button);
        int centeredHosts = 0;

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();

            auto element = current.try_as<wux::FrameworkElement>();
            if (element && current != button) {
                const auto className = winrt::get_class_name(current);
                if (_wcsicmp(className.c_str(),
                             L"Windows.UI.Xaml.Controls.ItemsPresenter") == 0 ||
                    _wcsicmp(className.c_str(),
                             L"Windows.UI.Xaml.Controls.StackPanel") == 0) {
                    element.HorizontalAlignment(wux::HorizontalAlignment::Center);
                    element.VerticalAlignment(wux::VerticalAlignment::Center);
                    ++centeredHosts;
                }

                if (auto content = current.try_as<wuc::ContentControl>()) {
                    content.HorizontalContentAlignment(wux::HorizontalAlignment::Center);
                    content.VerticalContentAlignment(wux::VerticalAlignment::Center);
                }
            }

            const int count = wuxm::VisualTreeHelper::GetChildrenCount(current);
            for (int i = 0; i < count; ++i) {
                auto child = wuxm::VisualTreeHelper::GetChild(current, i);
                if (child) {
                    stack.push_back(child);
                }
            }
        }

        Wh_Log(L"Centered %d native OmniButton item-host element(s) for %s#%s.",
               centeredHosts, winrt::get_class_name(button).c_str(),
               button.Name().c_str());
    } catch (...) {
        Wh_Log(L"CenterNativeOmniButtonItemHost failed for %s#%s: 0x%08X",
               winrt::get_class_name(button).c_str(), button.Name().c_str(),
               winrt::to_hresult());
    }
}

static void AttachTrayButtonHandlers(wux::FrameworkElement const& element,
                                     ButtonKind kind) {
    auto uiElement = element.as<wux::UIElement>();

    uiElement.RightTapped(
        [kind](wf::IInspectable const& sender,
               wuxi::RightTappedRoutedEventArgs const& args) {
            auto element = sender.try_as<wux::FrameworkElement>();
            if (element) {
                ShowTrayContextMenu(element, kind);
            }
            args.Handled(true);
        });

    uiElement.Tapped([kind](wf::IInspectable const&,
                            wuxi::TappedRoutedEventArgs const& args) {
        if (ConsumeSuppressedTap(kind)) {
            args.Handled(true);
            return;
        }

        HandleTrayButtonClick(kind);
        args.Handled(true);
    });

    uiElement.PointerPressed(
        [kind](wf::IInspectable const& sender,
               wuxi::PointerRoutedEventArgs const& args) {
            auto element = sender.try_as<wux::UIElement>();
            auto point = args.GetCurrentPoint(element);
            if (!point.Properties().IsMiddleButtonPressed()) {
                return;
            }

            SuppressMiddleClickTap(kind);
            if (kind == ButtonKind::Sound) {
                ToggleDefaultEndpointMute();
                UpdateDynamicXamlIcons();
            }
            args.Handled(true);
        });

    if (kind == ButtonKind::Sound) {
        uiElement.PointerWheelChanged(
            [](wf::IInspectable const& sender,
               wuxi::PointerRoutedEventArgs const& args) {
                auto element = sender.try_as<wux::UIElement>();
                auto point = args.GetCurrentPoint(element);
                int delta = point.Properties().MouseWheelDelta();
                if (delta != 0) {
                    StepDefaultEndpointVolume(delta > 0);
                    UpdateDynamicXamlIcons();
                    args.Handled(true);
                }
            });
    }
}

static wux::FrameworkElement CreateTrayButton(ButtonKind kind,
                                              PCWSTR glyph,
                                              PCWSTR name,
                                              PCWSTR tooltip) {
    // OmniButton is the private tray control that accepts injected XAML
    // content on this build while keeping the native pointer-over chrome.
    auto element = TryCreateNativeOmniButton(name);
    bool nativeElement = static_cast<bool>(element);
    if (!element) {
        element = CreateFallbackButton(name);
    }

    SetTrayToolTip(element, tooltip);

    IconLayers icon = CreateTrayIconLayers(glyph);
    bool iconSet = SetElementIcon(element, icon.host);
    if (!iconSet && nativeElement) {
        if (!iconSet) {
            Wh_Log(L"Native tray element#%s has no public content/items "
                   L"surface; falling back to styled public Button.", name);
            element = CreateFallbackButton(name);
            icon = CreateTrayIconLayers(glyph);
            iconSet = SetElementIcon(element, icon.host);
        }
        SetTrayToolTip(element, tooltip);
    }

    if (kind == ButtonKind::Bluetooth) {
        g_bluetoothIcon = icon;
    } else if (kind == ButtonKind::Network) {
        g_networkIcon = icon;
    } else if (kind == ButtonKind::Sound) {
        g_soundIcon = icon;
    } else if (kind == ButtonKind::QuickSettings) {
        g_compactGroupedIcon = icon;
    }

    AttachTrayButtonHandlers(element, kind);

    if (nativeElement &&
        _wcsicmp(winrt::get_class_name(element).c_str(),
                 L"SystemTray.OmniButton") == 0) {
        // The private control's visual tree exists only once it is in the live
        // taskbar. Do this on Loaded rather than offsetting the FontIcon itself.
        element.Loaded([](wf::IInspectable const& sender,
                          wux::RoutedEventArgs const&) {
            if (auto button = sender.try_as<wux::FrameworkElement>()) {
                ApplyTrayButtonMetrics(button);
                CenterNativeOmniButtonItemHost(button);
                ApplyHoverBackgroundMetrics(button);
                DumpInjectedButtonDiagnostics(button);
                LogVisualStateGroups(button);
            }
        });
    }

    return element;
}

struct OrderedTrayButtons {
    std::vector<wux::FrameworkElement> all;
    std::vector<wux::FrameworkElement> beforeNativeQuickSettings;
    std::vector<wux::FrameworkElement> afterNativeQuickSettings;
};

static wux::FrameworkElement ButtonElementForKind(ButtonKind kind) {
    switch (kind) {
        case ButtonKind::Bluetooth:
            return g_bluetoothButton;
        case ButtonKind::Network:
            return g_networkButton;
        case ButtonKind::Sound:
            return g_soundButton;
        case ButtonKind::QuickSettings:
            return g_compactGroupedButton;
        default:
            return nullptr;
    }
}

static OrderedTrayButtons CreateTrayButtons() {
    OrderedTrayButtons buttons;

    if (g_settings.showBluetoothButton) {
        g_bluetoothButton =
            CreateTrayButton(ButtonKind::Bluetooth, L"\xE702",
                             L"SeparateQuickSettingsXamlBluetooth",
                             L"Bluetooth");
        Wh_Log(L"Bluetooth tray button created: element=%p icon=%p.",
               winrt::get_abi(g_bluetoothButton),
               winrt::get_abi(g_bluetoothIcon.primary));
    } else {
        g_bluetoothButton = nullptr;
    }

    if (g_settings.showNetworkButton) {
        g_networkButton =
            CreateTrayButton(ButtonKind::Network, L"\xE701",
                             L"SeparateQuickSettingsXamlNetwork",
                             L"Network");
    } else {
        g_networkButton = nullptr;
    }

    if (g_settings.showSoundButton) {
        g_soundButton = CreateTrayButton(ButtonKind::Sound, L"\xE767",
                                         L"SeparateQuickSettingsXamlSound",
                                         L"Sound");
    } else {
        g_soundButton = nullptr;
    }

    if (GroupedButtonModeIs(L"compact")) {
        auto compactGlyph =
            GlyphFromHexSetting(g_settings.compactGroupedButtonGlyph, L'\xE91C');
        g_compactGroupedButton =
            CreateTrayButton(ButtonKind::QuickSettings, compactGlyph.c_str(),
                             L"SeparateQuickSettingsXamlQuickSettings",
                             L"Quick Settings");
    } else {
        g_compactGroupedButton = nullptr;
    }

    bool nativeQuickSettingsSeen = false;
    for (auto kind : GetVisibleButtonOrder()) {
        if (kind == ButtonKind::QuickSettings) {
            if (GroupedButtonModeIs(L"native")) {
                nativeQuickSettingsSeen = true;
            } else if (auto compact = ButtonElementForKind(kind)) {
                buttons.all.push_back(compact);
            }
            continue;
        }

        auto element = ButtonElementForKind(kind);
        if (!element) {
            continue;
        }

        buttons.all.push_back(element);
        if (GroupedButtonModeIs(L"native")) {
            if (nativeQuickSettingsSeen) {
                buttons.afterNativeQuickSettings.push_back(element);
            } else {
                buttons.beforeNativeQuickSettings.push_back(element);
            }
        }
    }

    return buttons;
}

static bool TryInjectBesideControlCenterButton(wux::FrameworkElement const& root) {
    auto controlCenterButton = FindControlCenterButton(root);
    if (!controlCenterButton) {
        Wh_Log(L"TryInjectBesideControlCenterButton: ControlCenterButton not "
               L"found from root.");
        return false;
    }

    Wh_Log(L"TryInjectBesideControlCenterButton: ControlCenterButton found.");

    auto parentElement = FindAncestorFrameworkElement(controlCenterButton);
    if (!parentElement) {
        Wh_Log(L"TryInjectBesideControlCenterButton: parent not found.");
        return false;
    }

    if (auto parentPanel = parentElement.try_as<wuc::Panel>()) {
        RemoveInjectedControls(parentPanel);
        g_trayPanel = parentPanel;
        g_trayControlCenterButton = controlCenterButton;
        CaptureTrayButtonMetricsFromPanel(parentPanel, controlCenterButton);
        AttachTaskbarSizeRefreshHandlers(parentElement, controlCenterButton);

        if (!GroupedButtonModeIs(L"native")) {
            HideOriginalGroupedButton(controlCenterButton);
        } else {
            RestoreOriginalGroupedButton();
        }

        auto children = parentPanel.Children();
        uint32_t insertIndex = children.Size();
        uint32_t controlCenterIndex = 0;
        if (children.IndexOf(controlCenterButton.as<wux::UIElement>(),
                             controlCenterIndex)) {
            insertIndex = controlCenterIndex;
        }

        auto buttons = CreateTrayButtons();
        if (GroupedButtonModeIs(L"native")) {
            for (uint32_t i = 0; i < buttons.beforeNativeQuickSettings.size();
                 ++i) {
                children.InsertAt(
                    insertIndex + i,
                    buttons.beforeNativeQuickSettings[i].as<wux::UIElement>());
            }

            uint32_t afterIndex = insertIndex +
                                  static_cast<uint32_t>(
                                      buttons.beforeNativeQuickSettings.size()) +
                                  1;
            for (uint32_t i = 0; i < buttons.afterNativeQuickSettings.size();
                 ++i) {
                children.InsertAt(
                    afterIndex + i,
                    buttons.afterNativeQuickSettings[i].as<wux::UIElement>());
            }
        } else {
            for (uint32_t i = 0; i < buttons.all.size(); ++i) {
                children.InsertAt(insertIndex + i,
                                  buttons.all[i].as<wux::UIElement>());
            }
        }

        UpdateDynamicXamlIcons();
        EnsureUpdateTimer();
        Wh_Log(L"TryInjectBesideControlCenterButton: injected into parent "
               L"panel %s#%s at index=%u.",
               winrt::get_class_name(parentElement).c_str(),
               parentElement.Name().c_str(), insertIndex);
        return true;
    }

    Wh_Log(L"TryInjectBesideControlCenterButton: parent is not a Panel: %s#%s",
           winrt::get_class_name(parentElement).c_str(),
           parentElement.Name().c_str());
    return false;
}

static void InsertGridTrayButtons(wuc::Grid const& trayGrid,
                                  std::vector<wux::FrameworkElement> const& buttons,
                                  int insertCol) {
    const int buttonCount = static_cast<int>(buttons.size());
    if (!trayGrid || buttonCount <= 0) {
        return;
    }

    for (int i = 0; i < buttonCount; ++i) {
        wuc::ColumnDefinition column;
        column.Width({1.0, wux::GridUnitType::Auto});
        if (insertCol + i >=
            static_cast<int>(trayGrid.ColumnDefinitions().Size())) {
            trayGrid.ColumnDefinitions().Append(column);
        } else {
            trayGrid.ColumnDefinitions().InsertAt(insertCol + i, column);
        }
    }

    for (uint32_t i = 0; i < trayGrid.Children().Size(); ++i) {
        auto child =
            trayGrid.Children().GetAt(i).try_as<wux::FrameworkElement>();
        if (!child || IsInjectedElement(child)) {
            continue;
        }

        int childCol = wuc::Grid::GetColumn(child);
        if (childCol >= insertCol) {
            wuc::Grid::SetColumn(child, childCol + buttonCount);
        }
    }

    for (uint32_t i = 0; i < buttons.size(); ++i) {
        wuc::Grid::SetColumn(buttons[i], insertCol + static_cast<int>(i));
        trayGrid.Children().Append(buttons[i]);
    }

    Wh_Log(L"Inserted %d separated tray button(s) at SystemTrayFrameGrid column %d.",
           buttonCount, insertCol);
}

static bool ApplyXamlButtons() {
    if (g_unloading) {
        return false;
    }

    HWND taskbarWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!taskbarWnd) {
        Wh_Log(L"ApplyXamlButtons: taskbar window not found.");
        return false;
    }
    g_taskbarWnd = taskbarWnd;
    Wh_Log(L"ApplyXamlButtons: taskbar window=%p", taskbarWnd);

    auto xamlRoot = GetTaskbarXamlRoot(taskbarWnd);
    if (!xamlRoot) {
        Wh_Log(L"ApplyXamlButtons: XamlRoot not found.");
        return false;
    }
    Wh_Log(L"ApplyXamlButtons: XamlRoot found.");

    auto root = xamlRoot.Content().try_as<wux::FrameworkElement>();
    if (!root) {
        Wh_Log(L"ApplyXamlButtons: root FrameworkElement not found.");
        return false;
    }
    Wh_Log(L"ApplyXamlButtons: root=%s#%s",
           winrt::get_class_name(root).c_str(), root.Name().c_str());

    auto trayFrame = FindChildByName(root, L"SystemTrayFrameGrid");
    auto trayGrid = trayFrame.try_as<wuc::Grid>();
    if (!trayGrid) {
        Wh_Log(L"ApplyXamlButtons: SystemTrayFrameGrid not found.");
        if (!g_dumpedTree) {
            g_dumpedTree = true;
            Wh_Log(L"ApplyXamlButtons: dumping XAML tree because "
                   L"SystemTrayFrameGrid was not found.");
            DumpXamlTree(root, 0, 7);
        }
        if (TryInjectBesideControlCenterButton(root)) {
            return true;
        }
        return false;
    }
    Wh_Log(L"ApplyXamlButtons: SystemTrayFrameGrid found.");

    RemoveInjectedControls(trayGrid);

    auto controlCenterButton = FindControlCenterButton(trayGrid);
    if (controlCenterButton) {
        Wh_Log(L"ApplyXamlButtons: ControlCenterButton found, column=%d.",
               wuc::Grid::GetColumn(controlCenterButton));
    } else {
        Wh_Log(L"ApplyXamlButtons: ControlCenterButton not found; injecting at "
               L"end of SystemTrayFrameGrid.");
    }

    CaptureTrayButtonMetricsFromPanel(trayGrid, controlCenterButton);
    g_trayPanel = trayGrid;
    g_trayControlCenterButton = controlCenterButton;
    AttachTaskbarSizeRefreshHandlers(trayGrid, controlCenterButton);

    if (!GroupedButtonModeIs(L"native")) {
        HideOriginalGroupedButton(controlCenterButton);
    } else {
        RestoreOriginalGroupedButton();
    }

    int insertCol = static_cast<int>(trayGrid.ColumnDefinitions().Size());
    if (controlCenterButton) {
        insertCol = wuc::Grid::GetColumn(controlCenterButton);
        if (insertCol < 0) {
            insertCol = static_cast<int>(trayGrid.ColumnDefinitions().Size());
        }
    }

    auto buttons = CreateTrayButtons();
    if (GroupedButtonModeIs(L"native")) {
        InsertGridTrayButtons(trayGrid, buttons.beforeNativeQuickSettings,
                              insertCol);
        const int nativeQuickSettingsCol =
            insertCol +
            static_cast<int>(buttons.beforeNativeQuickSettings.size());
        InsertGridTrayButtons(trayGrid, buttons.afterNativeQuickSettings,
                              nativeQuickSettingsCol + 1);
    } else {
        InsertGridTrayButtons(trayGrid, buttons.all, insertCol);
    }

    EnsureUpdateTimer();

    UpdateDynamicXamlIcons();
    Wh_Log(L"ApplyXamlButtons: injected native XAML tray buttons.");
    return true;
}

static void ApplyXamlButtonsWithRetry() {
    if (g_unloading) {
        return;
    }

    if (ApplyXamlButtons()) {
        g_retryCount = 0;
        if (g_retryTimer) {
            g_retryTimer.Stop();
            g_retryTimer = nullptr;
        }
        return;
    }

    if (++g_retryCount > 50) {
        Wh_Log(L"ApplyXamlButtonsWithRetry: giving up after %d attempts.",
               g_retryCount);
        if (g_retryTimer) {
            g_retryTimer.Stop();
            g_retryTimer = nullptr;
        }
        return;
    }

    if (!g_retryTimer) {
        g_retryTimer = wux::DispatcherTimer();
        g_retryTimer.Interval(std::chrono::milliseconds(100));
        g_retryTimer.Tick([](wf::IInspectable const&,
                             wf::IInspectable const&) {
            ApplyXamlButtonsWithRetry();
        });
        g_retryTimer.Start();
        Wh_Log(L"ApplyXamlButtonsWithRetry: retry timer started.");
    }
}

static void RemoveXamlButtons() {
    for (auto& query : g_btQueries) {
        if (query) {
            try { query.Cancel(); } catch (...) {}
            query = nullptr;
        }
    }
    g_btConnectedNames.clear();
    g_btConnectedCount = 0;
    g_btQueryTick = 0;
    try {
        HideFixedTrayTooltip();
        g_fixedTrayTooltipPopup = nullptr;
        g_fixedTrayTooltipBorder = nullptr;
        g_fixedTrayTooltipText = nullptr;
        if (g_updateTimer) {
            g_updateTimer.Stop();
            g_updateTimer = nullptr;
        }
        if (g_retryTimer) {
            g_retryTimer.Stop();
            g_retryTimer = nullptr;
        }
        if (g_metricRefreshTimer) {
            g_metricRefreshTimer.Stop();
            g_metricRefreshTimer = nullptr;
        }
        g_retryCount = 0;
        g_metricRefreshPending = false;
        g_metricRefreshSettlePasses = 0;
        if (g_sizeRefreshTrayElement) {
            g_sizeRefreshTrayElement.SizeChanged(g_sizeRefreshTrayToken);
            g_sizeRefreshTrayElement = nullptr;
            g_sizeRefreshTrayToken = {};
        }
        if (g_sizeRefreshControlCenterButton) {
            g_sizeRefreshControlCenterButton.SizeChanged(
                g_sizeRefreshControlCenterToken);
            g_sizeRefreshControlCenterButton = nullptr;
            g_sizeRefreshControlCenterToken = {};
        }

        HWND taskbarWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
        auto xamlRoot = taskbarWnd ? GetTaskbarXamlRoot(taskbarWnd) : nullptr;
        auto root = xamlRoot ? xamlRoot.Content().try_as<wux::FrameworkElement>()
                             : nullptr;
        auto trayFrame = root ? FindChildByName(root, L"SystemTrayFrameGrid")
                              : nullptr;
        if (auto trayGrid = trayFrame.try_as<wuc::Grid>()) {
            RemoveInjectedControls(trayGrid);
        } else if (root) {
            auto controlCenterButton = FindControlCenterButton(root);
            auto parentElement = controlCenterButton
                                     ? FindAncestorFrameworkElement(
                                           controlCenterButton)
                                     : nullptr;
            if (auto parentPanel = parentElement.try_as<wuc::Panel>()) {
                RemoveInjectedControls(parentPanel);
            }
        }

        RestoreOriginalGroupedButton();
        g_bluetoothButton = nullptr;
        g_networkButton = nullptr;
        g_soundButton = nullptr;
        g_bluetoothIcon = {};
        g_networkIcon = {};
        g_soundIcon = {};
        g_originalGroupedButton = nullptr;
    } catch (...) {
        Wh_Log(L"RemoveXamlButtons error: 0x%08X", winrt::to_hresult());
    }
}

using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID);
static UINT g_runFromWindowThreadRegisteredMsg = 0;

static void WINAPI ApplyXamlButtonsProc(PVOID) {
    ApplyXamlButtonsWithRetry();
}

static void WINAPI RemoveXamlButtonsProc(PVOID) {
    RemoveXamlButtons();
}

static void WINAPI ReapplyXamlButtonsProc(PVOID) {
    RemoveXamlButtons();
    ApplyXamlButtonsWithRetry();
}

static LRESULT CALLBACK RunFromWindowThreadHookProc(int code,
                                                    WPARAM wParam,
                                                    LPARAM lParam) {
    if (code == HC_ACTION) {
        const CWPSTRUCT* cwp = reinterpret_cast<const CWPSTRUCT*>(lParam);
        if (cwp && cwp->message == g_runFromWindowThreadRegisteredMsg) {
            struct Param {
                RunFromWindowThreadProc_t proc;
                PVOID procParam;
            };

            auto* param = reinterpret_cast<Param*>(cwp->lParam);
            param->proc(param->procParam);
        }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}

static bool RunFromWindowThread(HWND hwnd,
                                RunFromWindowThreadProc_t proc,
                                PVOID procParam) {
    if (!hwnd) {
        return false;
    }

    if (!g_runFromWindowThreadRegisteredMsg) {
        g_runFromWindowThreadRegisteredMsg =
            RegisterWindowMessageW(L"Windhawk_RunFromWindowThread_"
                                   L"separate-quick-settings-tray-icons-xaml");
    }

    DWORD threadId = GetWindowThreadProcessId(hwnd, nullptr);
    if (!threadId) {
        return false;
    }

    if (threadId == GetCurrentThreadId()) {
        proc(procParam);
        return true;
    }

    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC,
                                   RunFromWindowThreadHookProc, nullptr,
                                   threadId);
    if (!hook) {
        Wh_Log(L"RunFromWindowThread: SetWindowsHookEx failed: %lu",
               GetLastError());
        return false;
    }

    struct Param {
        RunFromWindowThreadProc_t proc;
        PVOID procParam;
    } param{proc, procParam};

    SendMessageW(hwnd, g_runFromWindowThreadRegisteredMsg, 0,
                 reinterpret_cast<LPARAM>(&param));
    UnhookWindowsHookEx(hook);
    return true;
}

static void ApplyFromTaskbarThread() {
    g_taskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!g_taskbarWnd) {
        Wh_Log(L"ApplyFromTaskbarThread: Shell_TrayWnd not found.");
        return;
    }

    RunFromWindowThread(g_taskbarWnd, ApplyXamlButtonsProc, nullptr);
}

static void WINAPI TrayUI_StartTaskbar_Hook(void* self) {
    TrayUI_StartTaskbar_Original(self);
    ApplyFromTaskbarThread();
}

static bool HookTaskbarDllSymbols() {
    HMODULE taskbarModule =
        LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!taskbarModule) {
        Wh_Log(L"HookTaskbarDllSymbols: taskbar.dll not loaded.");
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK taskbarHooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
         &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
         &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
         &TaskbarHost_FrameHeight_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
         &Std_Ref_Decref_Original},
        {{LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"},
         &TrayUI_StartTaskbar_Original,
         TrayUI_StartTaskbar_Hook},
    };

    if (!WindhawkUtils::HookSymbols(taskbarModule, taskbarHooks,
                                    ARRAYSIZE(taskbarHooks))) {
        Wh_Log(L"HookTaskbarDllSymbols: failed to resolve taskbar symbols.");
        return false;
    }

    Wh_Log(L"HookTaskbarDllSymbols: resolved taskbar XAML symbols.");
    return true;
}

BOOL Wh_ModInit() {
    LoadSettings();

    if (!HookTaskbarDllSymbols()) {
        return FALSE;
    }

    Wh_Log(L"Separate Quick Settings Tray Icons XAML loaded.");
    return TRUE;
}

void Wh_ModAfterInit() {
    ApplyFromTaskbarThread();
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    g_taskbarWnd = FindCurrentProcessTaskbarWnd();
    if (g_taskbarWnd) {
        RunFromWindowThread(g_taskbarWnd, ReapplyXamlButtonsProc, nullptr);
    }
}

void Wh_ModUninit() {
    g_unloading = true;
    g_taskbarWnd = FindCurrentProcessTaskbarWnd();
    if (g_taskbarWnd) {
        RunFromWindowThread(g_taskbarWnd, RemoveXamlButtonsProc, nullptr);
    }
}
