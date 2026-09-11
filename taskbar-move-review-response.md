# Taskbar Move review response

Changes apply to version 1.0.1. The existing Asteski author metadata is retained.

## Required findings

1. **Callback lifetime — addressed.** Each injected Click delegate is tracked
   using a weak item reference, event token, and owning thread ID. Both normal
   and toggle menu items use the same IMenuFlyoutItem Click contract.
   `Wh_ModBeforeUninit` prevents further registrations and marshals revocation
   to each owning UI thread with a synchronous, thread-specific message hook.
   Native `remove_Click` HRESULTs are checked instead of being discarded by the
   projected removal method. Active handlers finish before unloading. If a
   thread cannot be reached or removal fails, the module is retained until
   Explorer exits rather than leaving a delegate pointing to unloaded code.

2. **Missing position setting — addressed.** Only Move taskbar is conditional.
   Taskbar items and its three entries are built independently. A missing
   registry value no longer disables an otherwise available setting: values
   come from the native handler's GetValue, including Windows defaults.

3. **Private message compatibility — addressed.** All direct `0x5CA` messages
   and registry writes have been removed from the mod. Position uses
   `SystemSettings.Desktop.Taskbar.DesktopTaskbarLoSetting`, whose selection
   order is Bottom, Top, Left, Right. Search also uses its native handler's
   selection indexes, with a separate saved `LastVisibleSearchSelection` value
   so old raw registry modes cannot be mistaken for selection indexes.
   Availability and policy checks apply to every control. These are private
   Windows Settings interfaces, not a newly claimed public API contract.

4. **Attribution — addressed; license agreement — pending.** The README credits
   [Taskbar Restart Explorer by Mgrmjp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-restart-explorer.wh.cpp)
   and acknowledges the callback-lifetime pattern in
   [Taskbar Icon Separators](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-icon-separators.wh.cpp).
   The user confirmed no licensing agreement has been obtained yet and asked
   to continue preparing the mod. No agreement or `@license` declaration has
   been invented. This remains to resolve with the upstream author before
   presenting the submission as fully cleared for distribution.

5. **Screenshot — pending.** A real screenshot/GIF of the menus has not been
   supplied. No fabricated screenshot, placeholder, or nonexistent hosted
   image link was added. The README image should use the supplied review's
   allowed i.imgur.com or raw.githubusercontent.com host once an image exists.

6. **Persistent settings — documented.** Choices are explicit user actions
   performed through Windows Settings handlers. The README continues to state
   that unloading does not revert the user's chosen Windows settings.

## Additional cleanup

- Removed unused tags, name guards, and unnecessary cast exception handling.
- Constructed separators adjacent to their append operations; no leading
  separator is produced when movement is unavailable.
- Kept the public appbar query and documented why it uses the actual edge
  instead of the saved preference when other taskbar mods are active.
- Removed private-message magic numbers entirely.
- Retained Windows version logging, but removed the redundant minimum-build
  gate in favor of actual symbol and native-handler availability.
- Used one native activation per option for both value and availability;
  the same object is retained for that item's click handling.
- Checked IsUpdating before verifying persistence. Pending updates are not
  reported as failed, and the UI thread is not blocked waiting for them.
- Localization settings and visual testing with other menu/position mods are
  deferred. The menu labels and requested position glyphs are unchanged.

## Validation

- Built the final x64 DLL with the installed Windhawk compiler, warnings enabled.
- Tested native position selections 0, 1, 2, 3 against live Explorer:
  ABM_GETTASKBARPOS reported Bottom, Top, Left, Right respectively. Restored Top.
- Standalone regression tests used actual Windows XAML MenuFlyoutItem and
  ToggleMenuFlyoutItem instances with the mod's extracted lifecycle code:
  both delegate types were released while the items remained alive; new
  registration was blocked during unload; expired registrations were pruned;
  and cleanup invoked from a background thread ran on the owning UI thread.
- A menu-construction regression test with unavailable setting handlers kept
  Taskbar items, all three toggles, and the trailing separator, without a
  leading separator or Move taskbar entry.
- Tests are in `.codex-build/test-menu-review.cpp` and
  `.codex-build/probe-native-location.cpp`. These standalone tests do not
  substitute for a live install/unload test of the complete mod in Explorer.
- The updated mod has not been installed, published, or submitted for another
  review by the assistant.
