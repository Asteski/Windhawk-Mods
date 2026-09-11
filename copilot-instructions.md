Windhawk developer suggested two things to change in regards to Simple Window Switcher mod, here are his comments and specific code lines he reffered to:

1. line 337: "Don't modify the registry, use APIs such as Wh_SetIntValue and Wh_SetStringValue instead."

2. line 8: "See https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process". Below I pasted the page content under mentioned URL:

# Mods as tools: Running mods in a dedicated process
Michael Maltsev edited this page on Mar 29 · 5 revisions

# The Problem
Some mods are injected into explorer.exe and run as part of it simply because it's convenient, even though they don't actually need Windhawk's injection and hooking capabilities. This approach is suboptimal for several reasons:

Multiple explorer.exe processes: There can be more than one explorer.exe process when the "launch folder windows in separate process" option is enabled, or for other reasons.
Shell stability risk: A mistake or instability in the mod affects the entire Windows shell.
Unnecessary coupling: These mods could function as separate standalone tools.

# Examples of Such Mods
The following mods fall into this category:

- Auto Theme Switcher - Automatically switches between light and dark appearance/wallpapers/themes based on custom hours or sunset to sunrise
- Internet Status Indicator - Real-time network connectivity monitoring with visual indicators as a system tray icon
- Virtual Desktop Helper - Go to a specific virtual desktop and move the active window to a specific virtual desktop
- Taskbar Music Lounge - A native-style music ticker with media controls
All of these mods could be separate tools - they don't use and don't need Windhawk's injection and hooking capabilities.

# The Solution
Currently, Windhawk doesn't have built-in functionality for running mods as standalone tools. However, there is a code snippet that can be added to a mod to make it run as part of a separate windhawk.exe process instead of being injected into explorer.exe.

# How to Implement
- Add the code below at the end of your mod code
- Rename your callbacks: Replace Wh_ModInit, Wh_ModSettingsChanged, Wh_ModUninit in your code with WhTool_* equivalents:
- Wh_ModInit to WhTool_ModInit
- Wh_ModSettingsChanged to WhTool_ModSettingsChanged
- Wh_ModUninit to WhTool_ModUninit
- Change the target process: Update from explorer.exe to windhawk.exe
- Code Snippet (click to expand)
- How It Works
- When the mod is first loaded in windhawk.exe, it detects that it's the "launcher" instance
- The launcher spawns a new windhawk.exe process with the -tool-mod "mod-id" argument
- The new process runs the mod's logic via the WhTool_* callbacks
- A mutex prevents multiple instances of the same tool mod from running
- The tool process runs independently, isolating any potential issues from affecting the shell
- Note: In some cases, the target can be changed from windhawk.exe to explorer.exe or another process, in case running in a specific process is required, for example for special capabilities.