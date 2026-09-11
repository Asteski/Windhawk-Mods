# Taskbar Move implementation notes

The newest implementation is described in `taskbar-move-review-response.md`.
The sections below record earlier investigations, including the private-message
approach that has now been replaced with native Windows Settings handlers.

Verified on Windows 11 build 26200.9278, 2026-09-08.

The native Settings position control stores a DWORD named `TaskbarLocation`
under `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced`.
Values follow the appbar edge constants: Left = 0, Top = 1, Right = 2,
Bottom = 3. Settings clicks confirmed Left and Right; native movement tests
confirmed all four values.

Microsoft public symbols and disassembly of the installed
`SettingsHandlers_DesktopTaskbar.dll` identify the setter as
`DesktopTaskbarSettingsSingleton::Location`. It writes the setting, then calls
`SendMessageW` with message 1482 (`0x5CA`), wParam 6, and lParam containing the
edge. Sending that message to `Shell_TrayWnd` applies the change immediately.
Writing the registry and broadcasting `WM_SETTINGCHANGE` with `TraySettings`
or `TaskbarLocation` did not apply it.

The mod uses this same registry/message sequence, with `SendMessageTimeoutW`
to bound waits on an unresponsive recipient. This is a private Windows message,
so future Windows versions may require an update. It requires the existing
native position feature; it does not implement vertical taskbar layout itself.

Validation:

- Compiled the x64 mod with the installed Windhawk compiler and import library.
- Executed the registry/message movement sequence on the live Explorer taskbar.
  `ABM_GETTASKBARPOS` confirmed Top -> Left -> Right -> Bottom -> Top, including
  the corresponding screen rectangles. The original Top position was restored.
- Menu construction contains one entry for each of the other three edges and
  guards against recursive append-hook injection while constructing the submenu.
- The user subsequently installed version 1.0 and confirmed that it works.

The original `taskbar-restart-explorer.wh.cpp` is unchanged.

## Version 1.1

Adds a single size action offering the opposite of the saved `TaskbarSize`
(0 = Default, 1 = Small), and a Taskbar items submenu with Search, Task View,
and Widgets checkmarks. The submenu is rebuilt each time the menu opens.

Size, Task View, and Widgets use the native
`SystemSettings.Desktop.Taskbar.DesktopTaskbar{Size,TaskView,Da}Setting`
activation classes and `SystemSettings.DataModel.ISettingItem` interface
(`40c037cc-d8bf-489e-8697-d66baa3221bf`). The ABI was checked against Microsoft
public symbols and the installed DLL vtables. Size takes a boxed Int32;
Task View and Widgets take a boxed Boolean. Availability and policy state
come from that interface.

Search retains the actual registry mode in Windhawk's persistent mod storage.
The Settings Search handler's Value is a dropdown index with a dynamic mapping,
so the mod instead writes `SearchboxTaskbarMode` and sends taskbar message
`0x5CA`, wParam 102, lParam mode. The Task View handler's corresponding command
is 103. These command values were verified from disassembly.

Validation and remaining limitations:

- Version 1.1 compiles with the x64 Windhawk compiler with warnings enabled.
- External probes verified Size, Search, and Task View persistence and restored
  the original values (Small; all three items hidden).
- The external size tests did not change the reported taskbar rectangle.
  Immediate resizing still needs to be verified when called inside Explorer.
- External Widgets registry writes still returned Access denied after retries.
  The native handler returned success without persisting the new Widgets value;
  the mod therefore verifies persistence and restores the checkmark on failure.
  The user confirmed Widgets can be changed directly in Windows Settings.
- Attempting to install a temporary Explorer diagnostic through the CLI failed
  because `C:\ProgramData\Windhawk\Engine\Mods` was not writable. It was not
  installed. Version 1.1 has not been live-tested or installed by the assistant.

## Version 1.1.1

Removed the size action and its setting definition at the user's request.
A separator now sits between Move taskbar and Taskbar items. The existing
separator after Taskbar items remains. The size investigations above describe
version 1.1 only; the current mod does not change taskbar size.
