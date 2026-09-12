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
