# Separate System Tray Icons — Handoff

## Current state — 2026-09-13

This section overrides the historical notes below.

2026-09-14 Windhawk automated-review remediation: replaced self-pinning worker
threads with tracked joinable workers, and the status-event worker is joined
before unload completes. Teardown now stops native resources even when the
taskbar-thread dispatch fails; the refresh window receives an owner-thread
destroy message. XAML/WinRT global references that are explicitly cleared on
the taskbar thread use clang::no_destroy to prevent Explorer process-shutdown
destructors from releasing them off-thread. Refresh-window class registration
now rejects stale classes and unregisters on failed window creation. Grid
column insertions and shifted native columns are recorded and restored.
Repeated clicks close the selected flyout with WM_CLOSE rather than sending a
global Escape. Energy-saver preference reads are event-invalidated, removing
the periodic Settings-worker creation. x64/ARM64 -Werror builds and helper
tests passed; disable/enable retained both Explorer PIDs.

Underlay fix implemented after the native inspection: MakeUnderlayBrush reads
the original grouped button's foreground, with TextFillColorPrimaryBrush fallback.
Underlay FontIcons use Opacity=0.2 without mutating the shared native brush.
Disabled Bluetooth's base glyph applies the same opacity while its overlay stays
fully opaque. Opaque grey underlay colors were removed. Main glyphs are unchanged.

2026-09-14 native underlay investigation: temporary diagnostic build traversed
the original ControlCenterButton XAML tree on Explorer's UI thread. Native
SystemTray.TextIconContent > Grid#ContainerGrid >
SystemTray.AdaptiveTextBlock#Underlay has Opacity=0.2; its InnerTextBlock has
Opacity=1 and SolidColorBrush ARGB #E4000000, brush opacity 1 in current light
theme. Base uses the same brush at element opacity 1. Thus native underlays
are translucent, not solid grey. Trashpanda similarly creates underlays with
TextFillColorPrimaryBrush and Opacity=0.20. Our MakeUnderlayBrush still uses
opaque #C4C4C4/#494949; no production behavior change was made during this
investigation. Raw dump: .codex-build/tray-test/native-brushes.txt. Restore source
snapshot: .codex-build/tray-test/before-brush-probe.wh.cpp.

Energy saver tooltip text follows the context menu's AlwaysOn preference only,
not the OR-combined glyph state. Windows' SystemStatusFlag was observed as 1
after the user disabled the setting; it no longer overrides the tooltip's off
state. Unknown preference reads hide the text. Battery glyph behavior unchanged.

Battery tooltip formatting: remaining time uses `h` / `m`; active energy saver text is appended after a blank line as `Energy saver is enabled`, instead of on the battery status line. Existing charging suppression is preserved.

Bluetooth tooltip refresh fix: installed/enabled 1.0.0_363578. Device-change invalidation now preserves the previous completed snapshot during asynchronous enumeration; failed endpoint queries also preserve it, and completed results publish atomically (including real empty lists). Live test sent two device-change messages to the mod's refresh window and sampled 30 times over 15 seconds: zero false empty tooltips, iPhone 48% preserved, Explorer PID 1220 unchanged. Helper tests and compilation passed; editor synced.

Audit validation: build 0.6.0_697213 passed extracted-helper tests, full compilation, and four disable/enable checks plus Battery visibility reapply with Explorer PID 1220 preserved. The user subsequently reported all their manual tests looked good. Historical crash causality remains unproven.

Release 1.0.0 adds Bluetooth battery percentages from Windows' peripheral battery property. Async device-node enumeration joins readings to connected classic/LE endpoints by container ID, including headset audio components. Missing or invalid readings are omitted; duplicate component readings use the lowest valid percentage. No direct GATT connection is made. Live accessible tooltip validation showed `Adams iPhone (48%)`, matching Windows' reported property. Full compile and helper tests passed, including absent/invalid/0/100 battery readings and missing container identities. Custom dropdown labels now say Custom action; Sound has a settings description and prose README controls, and the dedicated Battery README section was removed.

- Active source: `mods/separate-quick-settings-tray-icons-xaml.wh.cpp`; version 1.0.0; installed id `local@separate-system-tray-icons`.
- Primary taskbar only. The previously implemented per-taskbar refactor is absent from the current checkout; do not claim multi-monitor support.
- Sound, Bluetooth, Network, Control Center and Battery are independent injected buttons, all orderable. Native grouped button is hidden and restored on disable. Battery follows Windows percentage settings and has its own size/action settings.
- Event subscriptions cover audio, network, Bluetooth, power and registry settings, with a five-second state fallback. UI refreshes are queued to an owned window on the taskbar thread. Fast layout checks remain.
- Lifetime audit: owned background-worker launcher retains the DLL; worker diagnostics avoid Windhawk calls after unload; media COM apartment is balanced. XAML UI/timer events are tracked for explicit revocation; tooltip references are weak. Cleanup closes menus and releases cached XAML/style references on the UI thread.
- Removed 498 lines of unreachable native-glyph mirroring code after checking references. Native style capture remains. Status glyph/visibility/opacity/foreground setters now avoid unchanged assignments; native hiding also avoids repeated property writes.
- Historical Explorer crashes during replacement have not been conclusively attributed. The audit fixes concrete lifetime hazards, but passing tests cannot prove the historical crash is eliminated.
- Regression procedure and current validation results: [testing checklist](docs/separate-system-tray-icons-testing.md).

## Historical notes (may describe reverted implementations)

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

## v0.5.9 observed native battery menu
Direct window/accessibility inspection identified Xaml_WindowedPopupClass on Explorer's taskbar thread, containing MenuFlyout and the single Power and sleep settings MenuFlyoutItem. Removed unsuccessful RightTapped and TrackPopupMenu interception. Extend native ContextFlyout when exposed, otherwise extend the matching open MenuFlyoutPresenter through its item source. The 500ms refresh can add a brief delay on the latter path. Preserve native settings action and menu placement; restore decorations on unload. Verification of visible Power mode entry is pending.

## v0.5.10 battery menu refinement
Power mode contains the three choices directly, using the AC/DC state read when constructed. Added a taskbar-thread EVENT_OBJECT_SHOW in-context callback for Xaml_WindowedPopupClass to prepare native menus without the polling delay; the periodic scan remains a fallback. Native presenter-source decoration uses an explicitly constructed settings item with gear icon; direct native-flyout decoration saves/restores its icon. Timing and icon appearance need laptop confirmation.

## v0.5.11 battery menu prepared before opening
Removed EVENT_OBJECT_SHOW and live presenter ItemsSource edits. Hook both Windows.UI.Xaml IFlyoutBase ShowAt entry points using their WinRT interface ABI slots (IFlyoutBase slot 14, IFlyoutBase5 slot 12). Native one-item battery menus are decorated before forwarding to ShowAt; reused menus refresh the active AC/DC power-mode choices before opening. Keep native settings action, explicitly provide gear icon, and let XAML measure/animate the complete menu. Runtime animation and native call-path confirmation remain pending.
## v0.5.12 battery menu and fixed native ordering
User confirmed the v0.5.11 pre-ShowAt menu animation works. Power mode now uses Segoe Fluent Icons EC48. Enable/Disable Energy saver is directly below it, before the existing separator and native settings action. The new item is tracked and removed together with other decorations when reopening/restoring menus.

Energy saver uses the Windows Settings handler, not the current battery-saver status or a power-plan/policy registry edit: dynamically load System32 SettingsHandlers_OneCore_BatterySaver.dll, call its GetSetting(HSTRING, IInspectable**) export with SystemSettings_PowerAndBattery_EnergySaverAlwaysOn, query ISettingItem {40c037cc-d8bf-489e-8697-d66baa3221bf}, and GetValue/SetValue("Value") with a boxed Boolean (ABI slots 13/14). This is a private Windows interface; unknown exports/interfaces or unreadable values disable the item. Setter failures are reported. Read state again at click time and each menu opening. A standalone probe on this PC read false, set true and read true, restored false, and independently reread false successfully.

Battery is removed from buttonOrder defaults/description and from injected ordering. Legacy battery tokens are ignored. Native battery panel/grid repositioning and the disabled rehosting block are removed; native host sizing and spacing overrides remain. Do not restore battery ordering. Syntax check passed; visual menu verification remains separate from the handler test.
Windhawk CLI compilation/install succeeded; v0.5.12 is enabled and its DLL was observed loaded in Explorer. Installed source and editor workspace were synchronized.

## v0.5.13 default tray icon sizing
Energy saver now uses Segoe Fluent Icons E8BE. The former battery-only toggle is displayed as Keep default tray icon size with default/small taskbar wording. Retain the stored keepBatteryGlyphSize key for upgrade compatibility; internal field is keepDefaultTrayIconSize. RefreshNativeTrayStyling traverses only SystemTrayFrameGrid and overrides FontIcon and all-PUA TextBlock font sizes to 16, plus native ImageIconContent/NotifyIconView image width/height to 16. This includes chevron, status and notification icons appearing dynamically. Percentage/date/clock text and button dimensions are untouched. Existing native-property tracking restores overrides on settings reapply/unload; the always-zero Control Center content spacing remains independent of the toggle. No 24/28 button-width switching was reintroduced. Syntax check passed; small-taskbar appearance still requires visual confirmation.
Windhawk compilation/install succeeded. v0.5.13 was enabled and its DLL confirmed loaded in Explorer; installed and editor sources match the workspace.

## RC version 0.5.0
Version reset from 0.5.13 to 0.5.0 at user request; this contains all preceding fixes. Added Battery percentage below Energy saver as a ToggleMenuFlyoutItem with a fixed label and native checkmark. It uses the same Settings handler/ISettingItem ABI with SystemSettings_PowerAndBattery_BatteryPercentageToggle. Reads on opening and clicking, restores the checkmark on failure, and tracks/removes the added item on menu reuse/unload. Energy saver's existing Enable/Disable wording is unchanged. A real-user-session probe successfully toggled percentage from true to false and restored true; a fresh read confirmed restoration. Sandbox writes returned access denied, so actual desktop testing is required. Existing native automatic battery width remains in place.
Syntax check and Windhawk compilation/install passed; version 0.5.0 is enabled, loaded in Explorer, and synchronized with the editor. Menu checkmark appearance still needs visual confirmation.
RC menu alignment fix: Battery percentage uses a regular MenuFlyoutItem with E73E in its Icon slot when checked (empty glyph when unchecked), avoiding ToggleMenuFlyoutItem extra check-column width. Automation ItemStatus exposes Checked/Unchecked. The Windows preference operation is unchanged. Version remains 0.5.0.
Battery percentage checkmark refinement: E73E font size is 12 to match the native submenu checkmark, with its existing 16-by-16 icon slot retained. Other menu glyphs are unchanged; version remains 0.5.0.
RC settings organization: nested visibility (Sound, Bluetooth, Network, Control Center, Battery), controlCenter (glyph/action), and sound (click action/media tooltip/output device) groups; then default tray icon size and button order at top level. Runtime reads use corresponding dotted keys. Existing installed preferences were copied to the grouped keys via Windhawk CLI; old flat keys are ignored. Version remains 0.5.0. Compilation and settings validation passed; updated DLL enabled and loaded, editor source synchronized.
Additional 0.5.0 options: Bluetooth group controls connected-device tooltip detail and disabled-state glyph styling (both true by default); Network group controls Wi-Fi signal detail (true) and speed-test HTTP(S) URL (existing speedtest.net/run default). Control Center has Default/Custom action selection and a custom action field retaining the existing action parser. Default always opens ms-controlcenter:. Disabled tooltip detail cancels/resets Bluetooth device queries. Syntax and whitespace checks passed.
v0.6.0 reorganizes settings in order Sound, Bluetooth, Network, Control Center, Toggle buttons visibility, then global options. Button order is now an ordered array (buttonOrder[0..3]) with four fixed options, rendering as a draggable list in Windhawk; legacy comma-separated buttonOrder is read only if no array is present. Battery is intentionally absent and remains native. Bluetooth unavailable indicator wording is generalized. Renamed requested settings labels. Syntax and diff checks passed before installation.

REVERTED: v0.6.0 shared-flyout battery highlight patch (DLL 0.6.0_452265) caused an Explorer restart loop in user testing. Disabled the mod immediately and removed the whole patch: routed delegates, AddHandler/RemoveHandler calls, native hover handlers, background opacity tracking, and injected-click suppression. Exact crash cause is not yet diagnosed; do not reuse that event-hook approach without investigation. Restored build 0.6.0_149730 compiled, enabled, and loaded in Explorer; editor source synchronized. Shared battery highlighting remains an unresolved cosmetic issue. Compile success and initial DLL loading do not prove runtime safety.

Energy saver intermittent freeze mitigation (v0.6.0): previous menu construction and Click called private Windows Settings APIs synchronously on the taskbar thread. Moved EnergySaverAlwaysOn GetSetting/GetValue/SetValue and COM object lifetime to a serialized STA worker. Menu uses atomic cached state, disabled while unknown/busy; background reads refresh at most every two seconds. A click racing a read is queued. Worker holds a module reference until FreeLibraryAndExitThread and uses no Windhawk/XAML calls, so mod teardown doesn't wait for a stuck Settings call or unload its running code. Percentage uses the existing independent path. This removes a confirmed UI-thread blocking risk; the intermittent Explorer restart's exact cause and whether this fully resolves it still need runtime confirmation.

## Experimental separate battery branch
Event-driven stage (single-taskbar checkout): native audio endpoint/device callbacks, IP-interface/WLAN notifications, Bluetooth radio-handle notifications, AC/percentage power notifications, and registry-change waits for Explorer Advanced + Control/Power now queue UI-thread refreshes. A dedicated MTA worker owns subscription setup/cleanup and retains the DLL until it exits; stop is asynchronous to avoid blocking Explorer. Notification callbacks only queue messages; unregister operations never hold the refresh lock. Routine state fallback is 5 seconds; 500 ms layout checks and Wi-Fi connecting glyph animation remain. Registry changes invalidate the energy-saver read timestamp; changed asynchronous reads request a repaint. Worker periodically rebinds Bluetooth handles after adapter changes. Failed subscriptions leave fallback polling available.

Validation: syntax/full compile passed (SDK winsock include-order warning). Build 0.6.0_204417 registered all seven groups, diagnostic StatusEventSources=127 on its message window. A Windows.UI.Xaml.dll crash occurred during replacement in prior Explorer PID 11428 at 16:44:54 on 2026-09-13; causal attribution remains unresolved. New Explorer PID 1220 survived a separate disable/enable cycle. Disable removed the message-only window, and the worker/module was released; editor synchronized. Actual device reconnects, power transitions, and external volume changes remain to be exercised; subscription success alone is not end-to-end validation. Multi-monitor support is still not present in this checkout.
Action refresh stage: added a message-only window created on the taskbar UI thread. Volume/mute/output-device changes and completed radio/energy-saver/percentage operations post a coalesced refresh, followed by one 150 ms settle pass. Radio changes invalidate Bluetooth device queries on the UI thread. Removed the airplane-mode worker's direct XAML refresh, retained its module until worker exit, and made unloading atomic. Teardown disconnects producers before destroying the window/timer. Current checkout remains single-taskbar: this does NOT restore cross-monitor state/refresher support. Compile passed; build 0.6.0_688141 refresh window and Shell_TrayWnd both belonged to Explorer PID 11428, UI thread 7768. Disable removed the refresh window; Explorer PID remained unchanged. Actual setting changes were not made during validation.
First-stage runtime validation: build 0.6.0_966237 compiled, enabled, and loaded; Explorer PID 20020 was unchanged through installation. UI Automation confirmed each visible injected button (Sound, Bluetooth, Network, Battery) is keyboard-focusable and exposes its dynamic name plus keyboard help. Editor synchronized. Control Center was not visible in this configuration; actual key activation, focus visuals, and Narrator output remain unverified.
First input/accessibility stage: current workspace was back on the single-taskbar implementation when this task began; the earlier per-taskbar refactor was not restored. Added per-button input state for wheel accumulation (120 units per notch, signed remainder, discard remainder after 1.5 seconds idle, ignore horizontal wheel). Custom volume steps apply the complete notch count in one scalar update; system default repeats native steps. Added tab stops/system focus visuals, Enter activation, Space release activation, Menu/Shift+F10 context menus, and accessible keyboard help. Existing dynamic accessible names remain. Middle-click suppression array expanded to five entries. Battery tooltip now distinguishes charging, plugged-in/not-charging and fully charged/plugged-in, with energy saver separate. Extracted actual wheel accumulator into a standalone C++ test: partial deltas, positive/negative multi-notch, direction cancellation, and idle expiry passed. Live keyboard/screen-reader testing remains required.
Added Sound > Volume scroll step, adapted from the supplied fork: system default (0), 2%, 5%, or 10% per existing wheel step. Custom values use endpoint scalar volume, clamp to 0–100%, preserve unmute-first behavior, and fall back to native stepping if reading current volume fails. Invalid setting values use system default. README updated; no live audio level changes made during validation.
Energy saver appearance specified by user: use SysBatt F8D0 outline with regular charge-level fill F8D1-F8DA in exact #EAA300 (opaque), replacing the fork's saver-specific glyphs and theme caution brush. Charging retains priority. This resolves the previously unspecified replacement mapping.
Energy saver priority correction: saver glyph/color now applies only when not charging; charging retains its existing green charging layers. User also reports saver glyph resembles low battery. Mapping matches the supplied fork (F849 base, F84A-F851 fills), but referenced comparison images were absent from that message; replacement mapping remains unresolved pending images.
Battery is now an orderable independent button: added battery to the draggable list defaults/options, read five array entries, and parse battery into ButtonKind::Battery. Existing four-entry orders still append missing Battery at the end; duplicate/hidden entries use existing filtering. This supersedes all earlier instructions to keep Battery out of ordering.
Latest padding reference: increased stable icon-only width to 36 DIPs (31 for small glyph), and shifted visible battery outline +2 DIPs within that button to correct its left-biased appearance. Percentage mode has no glyph translation and retains its existing automatic width. Supersedes earlier 32/27 widths. Screenshot-based adjustment, not yet visually confirmed after installation.
Further clipping correction: natural glyph width alone did not resolve clipping in user testing. Removed the extra 6-DIP content margins in fixed-width icon-only mode so they no longer compete with the native OmniButton template inset. Percentage mode retains its existing margins. Await visual confirmation; do not treat preceding clipping attempts as verified fixes.
Clipping follow-up: user screenshot showed the battery outline clipped after fixed glyph-slot sizing. Removed the inner 20/15-DIP host width and restored natural glyph measurement; kept stable outer icon-only button widths and height refresh. Visual confirmation remains pending.
User confirmed height refresh works but reported continuous width changes with percentage off. Icon-only battery now has stable width 32 DIPs (27 with small glyph), with a stable 20/15-DIP glyph slot. Percentage-on width remains automatic. Refresh and metric paths use the same width rule; side margins update only when their value changes. Height/highlight refresh is retained. Runtime stability still needs user confirmation.
Battery sizing/settings follow-up: icon-only content now uses 6-DIP side margins (8 with percentage). Battery uses the same live tray-height and BackgroundBorder refresh path as the other injected buttons, including delayed settle passes, while retaining automatic width. Removed global Keep default tray icon size and its native-icon traversal; always-zero native panel spacing remains. Added battery.smallGlyph (Use small battery icon), default false: battery layers 16 or 12, percentage typography unaffected. These supersede earlier native-height/template preservation notes. Compilation/runtime loading do not confirm visual parity or both taskbar sizes.
Screenshot alignment correction: removed both horizontal render offsets to restore the full 4-DIP battery/text gap; reduced percentage Y offset from -2 to -1 DIP because the previous scaled shift placed text too high. These supersede the previous offset values below. Visual parity is still subject to user confirmation.
Battery alignment follow-up: RenderTransform shifts percentage -1 X/-2 Y and the whole battery glyph host +1 X in XAML logical pixels, preserving measured button geometry. Energy saver coloring now honors both SYSTEM_POWER_STATUS.SystemStatusFlag and the asynchronously refreshed Always use energy saver cache, including AC. No synchronous Settings calls added to rendering. Live visual confirmation of color/offsets remains pending.
Percentage typography correction: the explicit increase from 14 to 16 made the label visibly larger (confirmed by user). Percentage now uses CaptionTextBlockStyle, as the reference fork uses for labels, with a 12-DIP fallback when the style is unavailable. Battery icon glyphs remain 16; label sizing is separate from icon sizing. Exact visual matching remains to be confirmed.
User subsequently confirmed percentage toggling works. Battery sizing follow-up: explicitly set battery glyph layers and percentage text to 16, add 8-DIP horizontal content margins (matching the reference fork's battery layout), inherit native grouped-button height constraints, and leave the native OmniButton highlight template untouched instead of applying compact notification-button highlight height. Width remains automatic when percentage visibility changes. Syntax and whitespace checks passed; exact visual parity with the old battery still requires live visual confirmation.
On sep-battery-button, v0.6.0 replaces the native battery presentation with an independent SystemTray.OmniButton#SeparateQuickSettingsXamlBattery. SysBatt Fluent Icons base/fill glyph mapping is adapted from the supplied trashpanda fork. Charge, charging, energy saver, low/critical battery colors and tooltip update through GetSystemPowerStatus. The original ControlCenterButton is collapsed with zero width and restored on unload; native battery extraction/visibility helpers and all native battery ShowAt hooks/decorations are removed. The new button uses the existing direct WinUI menu builder (Power mode, Energy saver, Battery percentage check, separator, Power and sleep settings), and Battery settings add Open Control Center/Custom action selection. Battery is appended after the four ordered injected buttons.

Battery content uses natural width, SysBatt font layers, and SeparateTrayBatteryPercentage text; never run the four-button fixed width on it. Windows percentage preference is read every refresh from HKCU Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced / IsBatteryPercentageEnabled. Percentage writes now run through the serialized Settings worker as operation 2, independently of energy saver support; operation 1 toggles energy saver and 0 refreshes it. Both pending toggle types can be retained. No Settings COM calls remain in percentage menu construction or Click.

Runtime verification found the independent battery automation peer and live charge tooltip. During diagnosis Explorer consistently read percentage=0 while standalone registry/Settings probes reported 1, even with the same user SID/key path. The raw registry diagnostic did not resolve that discrepancy and was removed; user confirmation of the actual Settings toggle is pending. Do not claim percentage visibility/width switching is verified until both states are observed in the live taskbar. Temporary probes are under .codex-build/tray-test only.
