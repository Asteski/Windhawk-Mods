# Separate System Tray Icons — Handoff

## Project

Windhawk mod source:

`mods/separate-quick-settings-tray-icons-xaml.wh.cpp`

The mod targets `explorer.exe` and creates separate XAML tray buttons for sound, Bluetooth, network, and Control Center. Current source version is `0.5.0`.

## Current intended behavior

- Sound, Bluetooth, network, and Control Center are injected as native-looking XAML tray buttons.
- Default order is:
  `sound,bluetooth,network,controlcenter,battery`
- Default sound action is the sound output picker.
- The original grouped Control Center button should be hidden on devices without a battery.
- On devices with a battery, the native grouped host is temporarily kept visible so its native battery content can remain Windows-owned.
- Battery visibility is controlled by `showBatteryButton`.
- Control Center settings are:
  - `showControlCenterButton`
  - `controlCenterGlyph` (default `F4C3`)
  - `controlCenterAction` (default `ms-controlcenter:`)

## Important current problem

The battery is exposed as `SystemTray.BatteryIconContent` inside the original grouped `ControlCenterButton`.

Attempts to detach/rehost it caused Explorer restart loops and non-interactive rendering. The current source therefore keeps the native grouped host in place when a battery target is found and collapses non-battery siblings. This restores native battery rendering but can interfere with hover highlights on the injected buttons because the grouped host may retain excess layout width or template hit-testing.

The latest attempted mitigation constrains the grouped host width to the measured battery content width. This still needs testing on the laptop.

On a laptop without a battery, the grouped host must be hidden so its network/sound children do not reappear.

## Existing features to preserve

- Bluetooth click: `ms-controlcenter:bluetooth`
- Network click: `ms-availablenetworks:`
- Sound click modes: Quick Settings, sound output picker, or `sndvol.exe`
- Sound wheel volume control; scrolling unmutes first
- Sound middle-click mute
- Dynamic network, sound, Bluetooth glyphs and theme-aware underlays
- Fixed-position tooltips relative to taskbar edge
- Current media information in sound tooltip
- WinUI/Win32 context-menu framework setting
- Bluetooth, network, sound context menus and actions
- Taskbar-size/highlight handling
- Stable injected names:
  - `SeparateQuickSettingsXamlBluetooth`
  - `SeparateQuickSettingsXamlNetwork`
  - `SeparateQuickSettingsXamlSound`
  - `SeparateQuickSettingsXamlControlCenter`

## Battery-related functions

- `FindNativeBatteryButton` searches for class `SystemTray.BatteryIconContent`.
- `KeepOnlyNativeBattery` collapses grouped siblings that do not contain the battery content.
- `ShowNativeBatteryHost` restores the grouped host visibility and constrains its width.
- The dedicated custom battery rehosting path was disabled because it caused Explorer restart loops and broke interaction.

## Build / syntax check

Run from the repository root:

```powershell
& 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -fsyntax-only -x c++ -std=c++20 -DWH_MOD -DWH_MOD_ID='L"separate-quick-settings-tray-icons-xaml"' -DUNICODE -D_UNICODE -I'C:\Program Files\Windhawk\Compiler\include' -include windhawk_api.h '.\mods\separate-quick-settings-tray-icons-xaml.wh.cpp'
```

The syntax check currently passes.

## Laptop test procedure

1. Copy or clone the repository to the laptop.
2. Install/enable the mod from the source file in Windhawk.
3. Restart Explorer once after changing the mod.
4. Capture Windhawk/DbgView logs during initialization.
5. Test both laptop states:
   - battery present and percentage enabled;
   - battery unavailable/removed.
6. Verify:
   - battery appearance matches native Windows;
   - battery is clickable;
   - no Explorer restart loop;
   - sound/Bluetooth/network highlights still work;
   - grouped network/sound icons do not reappear.

## Next likely debugging step

If highlights remain broken on the laptop, log the grouped host's `ActualWidth`, `ActualHeight`, `Visibility`, `IsHitTestVisible`, Grid column, and visual-tree path for `SystemTray.BatteryIconContent` and its nearest `OmniButton`/panel ancestors. The key design decision is whether the native battery can be isolated by collapsing only the grouped content presenters while keeping the native battery host clickable.

## Current laptop implementation (v0.5.3)

The active source is `mods/separate-quick-settings-tray-icons-xaml.wh.cpp`,
mod ID `separate-system-tray-icons` (installed as `local@separate-system-tray-icons`).
The root-level source is older. Earlier battery-width notes above are historical:
the native battery host now uses automatic content sizing, and the refresh timer
must never hide that host while `showBatteryButton` is enabled and a battery exists.
Capture the grouped host/style on the battery path as well as the hidden-host path.

The four injected buttons use 24-unit widths for 12-unit glyphs and 28 for
16-unit glyphs. Native battery and chevron widths are not overridden. Prefer
live XamlRoot height for size classification; HWND/DPI calculations are a fallback.
`retainDefaultGlyphSize` defaults to false and forces 16-unit glyphs when enabled.

### Restoring optional Win32 context menus

WinUI is now the only reachable context-menu framework. The setting and the
Win32 fallback were removed from `ShowTrayContextMenu`. Legacy stored
`contextMenuFramework` values have no effect. Win32 construction and command
helpers, including `ShowWin32TrayContextMenu`, remain in the source but are dormant.

To restore the choice:
1. Add `contextMenuFramework: winui` to the Windhawk settings block with `winui`
   and `win32` options.
2. Add `std::wstring contextMenuFramework = L"winui"` to `Settings` and load it
   with `GetStringSettingWithDefault` in `LoadSettings`.
3. In `ShowTrayContextMenu`, dispatch to `ShowWin32TrayContextMenu(kind)` for
   `win32`, otherwise call `ShowWinUiTrayContextMenu(target, kind)`. Restore an
   automatic Win32 fallback only if explicitly desired; it is currently absent.
4. Compile and test all menu commands, both taskbar edges, DPI scales, and
   sound-output submenus. WinUI button-left alignment is in `PositionTrayMenuPopup`.

## v0.5.4 rollback
Removed retainDefaultGlyphSize and automatic glyph-dependent widths at user request due to instability. Injected glyphs are fixed at 16; widths again follow native tray metrics with a 28-unit fallback. Battery and chevron glyph sizing is Windows-owned. Battery content-based width, native mute glyph, left-aligned WinUI menus, and permanent WinUI are retained. The v0.5.3 sizing notes above are superseded.

## v0.5.5 battery-only styling
Added keepBatteryGlyphSize (default false): forces only native BatteryIconContent glyphs to FontSize 16; percentage text and chevron are untouched. No taskbar-size detection or injected-button resizing was reintroduced. The exact ControlCenterButton > Grid > ContentPresenter#ContentPresenter > ItemsPresenter > StackPanel path always receives Spacing=0 while the mod is active, regardless of battery visibility. Native property overrides are restored when the mod unloads or settings are reapplied.

## v0.5.6 context menus
Control Center now has Taskbar settings above System settings, with distinct ms-settings:taskbar and ms-settings: actions. Menu edge/taskbar gaps use 12 XAML units multiplied by XamlRoot.RasterizationScale rather than 12 physical pixels; left-button alignment and monitor clamping remain.
BatteryIconContent receives a removable RightTapped handler that displays a WinUI Power mode submenu with separate Plugged in and On battery submenus, followed by a separator and Power and sleep settings. Native battery rendering/left click remain Windows-owned. Uses dynamically resolved PowerGet/SetUserConfiguredACPowerMode and DCPowerMode APIs from powrprof.dll. Unsupported or unreadable state is disabled. API setters only run on user selection. Validate native-event routing and visual gap matching on the laptop.

## v0.5.8 battery menu routing
The content RightTapped handler and handled-events host handler did not expose the extended menu in user testing. Added guarded TrackPopupMenu/TrackPopupMenuEx interception for the English one-item Power and sleep settings menu on the taskbar UI thread. This path is runtime-matched, but whether the laptop invokes it still needs verification. Other menus pass through. Perform speed test now uses glyph F42F.
