// ==WindhawkMod==
// @id              taskbar-menu-bar
// @name            Taskbar Menu Bar
// @description     Adds custom taskbar buttons, quick actions, and dropdown menus to Windows 11.
// @version         0.1.0
// @author          Asteski
// @include         explorer.exe
// @compilerOptions -lruntimeobject -lversion -luuid -luser32 -lwindowsapp -lshell32 -lgdi32 -lshlwapi -lwindowscodecs -ldwmapi -lshcore -lksuser
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Menu Bar

Adds customizable buttons and dropdown menus to the Windows 11 taskbar. Use it
to recreate legacy taskbar toolbars, build quick launch buttons, or create a
compact menu bar for folders, commands, shortcuts, and web links.

The mod focuses on horizontal Windows 11 taskbars. Buttons can be inserted near
Start/Search/Task View/Widgets or overlaid at the left, center, or right edge of
the taskbar area.

---

## Quick start

| What to do |
|------------|
| Open this mod's Windhawk settings. |
| Add or edit entries under **Buttons**. |
| Choose `button` for a direct action or `menu` for a dropdown. |
| Set each top-level item's label, icon, action/menu items, and display mode. |
| Save settings; Explorer rebuilds the taskbar toolbar automatically. |

---

## Item types

| Type | Where used | Behavior |
|------|------------|----------|
| `button` | Top-level taskbar item or menu entry | Runs its configured action when clicked. |
| `menu` | Top-level taskbar item or nested menu entry | Opens a dropdown or submenu. |
| `separatorAfter` | Menu entries | Adds a separator after the entry. |

Menu nesting is capped at three levels total: taskbar button, menu item, and
submenu item.

---

## Action formats

| Format | Example | Description |
|--------|---------|-------------|
| File/app path | `C:\Path\To\App.exe` | Opens a program, file, or folder. |
| Path with arguments | `C:\Path\To\App.exe --flag` | Opens a program with arguments. |
| `cmd:` | `cmd:explorer` | Runs a command through Command Prompt. |
| `shell:` | `shell:Downloads` | Opens a Windows Shell folder or Shell item. |
| `powershell:` / `ps:` | `powershell:Start-Process notepad` | Runs a PowerShell command. |
| `key:` | `key:Ctrl+Shift+Esc` | Sends a keyboard shortcut. |
| `web:` | `web:https://example.com` | Opens a web URL in the default browser. |

---

## Icon formats

| Type | Example | Notes |
|------|---------|-------|
| Fluent glyph | `E80F` | Segoe Fluent Icons hex code, without `\u`. |
| EXE icon | `C:\Windows\explorer.exe` | Uses the file's embedded icon. |
| DLL icon | `C:\Windows\System32\shell32.dll,3` | Optional comma index selects an icon resource. |
| ICO file | `C:\Icons\home.ico` | Uses the icon file directly. |

---

## Layout and behavior

| Setting | Behavior |
|---------|----------|
| `displayMode` | Per top-level item: `text`, `icon`, or `both`. |
| `dynamic` width | Each button sizes to its own content. |
| `static` width | All visible buttons match the widest rendered button. |
| `custom` width | All visible buttons use `customButtonWidth`. |
| `enableTooltips` | Enables or disables all tooltips created by this mod. |

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- MenuBarSettings:
  - placement: "taskbar_right_start"
    $name: Placement
    $description: >-
      Where the toolbar is inserted. Start/Search/Task View/Widgets placements
      reserve space next to the selected taskbar element. Edge placements overlay
      the taskbar area without moving native taskbar content.
    $options:
    - "taskbar_left_edge": "Left edge (Overlay)"
    - "taskbar_center_edge": "Center (Overlay)"
    - "taskbar_right_edge": "Right edge (Overlay)"
    - "taskbar_left_start": "Left of Start button"
    - "taskbar_right_start": "Right of Start button"
    - "taskbar_after_search_left": "Left of Search button"
    - "taskbar_after_search_right": "Right of Search button"
    - "taskbar_after_taskview_left": "Left of Task View button"
    - "taskbar_after_taskview_right": "Right of Task View button"
    - "taskbar_after_widgets_left": "Left of Widgets button"
    - "taskbar_after_widgets_right": "Right of Widgets button"
  - buttonContentAlignment: "center"
    $name: Button content alignment
    $description: Aligns each button's label/icon content inside its hit target.
    $options:
    - left: Left
    - center: Center
    - right: Right
  - buttonWidthMode: "dynamic"
    $name: Button width
    $description: >-
      Dynamic sizes each button to its content. Static makes every button as
      wide as the widest visible button. Custom uses the exact pixel width below.
    $options:
    - dynamic: Dynamic
    - static: Static
    - custom: Custom
  - customButtonWidth: 164
    $name: Custom button width (px)
    $description: Used only when Button width is set to Custom.
  - buttonSpacing: 2
    $name: Spacing between buttons
    $description: Horizontal gap in pixels between visible taskbar buttons.
  - iconLabelSpacing: 6
    $name: Spacing between icon and label
    $description: Gap in pixels between a button icon and its text label.
  - buttonOffset: "2,0"
    $name: Button offset (horizontal,vertical)
    $description: Moves the whole toolbar by the given X,Y pixel offset.
  - buttonPadding: "10,5"
    $name: Button padding (horizontal,vertical)
    $description: Inner padding around each button's label/icon content.
  - iconSize: 16
    $name: Icon size
    $description: Icon size in pixels for top-level taskbar buttons.
  - textSize: 12
    $name: Font size
    $description: Text size in pixels for top-level taskbar button labels.
  - enableTooltips: true
    $name: Enable tooltips
    $description: Enables tooltip popups for taskbar buttons and dropdown menu entries.
  $name: Menu Bar

- ButtonsSettings:
    - buttons:
        - - label: ""
            $name: Name
          - type: button
            $name: Item type
            $options:
            - button: Button
            - menu: Dropdown menu
          - icon: "shell32.dll,-1001"
            $name: Icon
            $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
          - action: "winver.exe"
            $name: Action
          - tooltip: "About Windows"
            $name: Tooltip
          - state: "normal"
            $name: State
            $options:
            - normal: Normal
            - disabled: Disabled
            - hidden: Hidden
          - displayMode: "icon"
            $name: Display mode
            $options:
            - text: Label only
            - both: Icon and label
            - icon: Icon only
          - subItems:
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: ""
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: ""
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: ""
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: ""
                    $name: Action
                  - tooltip: ""
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            $name: Menu items
        - - label: "Home"
            $name: Name
          - type: menu
            $name: Item type
            $options:
            - button: Button
            - menu: Dropdown menu
          - icon: "shell32.dll,-269"
            $name: Icon
            $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
          - action: ""
            $name: Action
          - tooltip: "Open home folders"
            $name: Tooltip
          - state: "normal"
            $name: State
            $options:
            - normal: Normal
            - disabled: Disabled
            - hidden: Hidden
          - displayMode: "both"
            $name: Display mode
            $options:
            - text: Label only
            - both: Icon and label
            - icon: Icon only
          - subItems:
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Desktop"
                $name: Name
              - icon: "shell32.dll,-35"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:Desktop"
                $name: Action
              - tooltip: "Open Desktop"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Documents"
                $name: Name
              - icon: "shell32.dll,-235"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:Personal"
                $name: Action
              - tooltip: "Open Documents"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Downloads"
                $name: Name
              - icon: "imageres.dll,-184"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:Downloads"
                $name: Action
              - tooltip: "Open Downloads"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Favorites"
                $name: Name
              - icon: "shell32.dll,-44"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:Favorites"
                $name: Action
              - tooltip: "Open Favorites"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Pictures"
                $name: Name
              - icon: "shell32.dll,-236"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:My Pictures"
                $name: Action
              - tooltip: "Open Pictures"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Music"
                $name: Name
              - icon: "shell32.dll,-237"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:My Music"
                $name: Action
              - tooltip: "Open Music"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            - - type: button
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Videos"
                $name: Name
              - icon: "shell32.dll,-238"
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: "shell:My Video"
                $name: Action
              - tooltip: "Open Videos"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
            $name: Menu items
        - - label: "Settings"
            $name: Name
          - type: menu
            $name: Item type
            $options:
            - button: Button
            - menu: Dropdown menu
          - icon: ""
            $name: Icon
            $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
          - action: ""
            $name: Action
          - tooltip: "Open Windows Settings"
            $name: Tooltip
          - state: "normal"
            $name: State
            $options:
            - normal: Normal
            - disabled: Disabled
            - hidden: Hidden
          - displayMode: "both"
            $name: Display mode
            $options:
            - text: Label only
            - both: Icon and label
            - icon: Icon only
          - subItems:
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "System"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Display, sound, power, storage"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Display"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:display"
                    $name: Action
                  - tooltip: "Open Display settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Sound"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:sound"
                    $name: Action
                  - tooltip: "Open Sound settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Notifications"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:notifications"
                    $name: Action
                  - tooltip: "Open Notifications settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Power & battery"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:powersleep"
                    $name: Action
                  - tooltip: "Open Power & battery settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Storage"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:storagesense"
                    $name: Action
                  - tooltip: "Open Storage settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Bluetooth & devices"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Bluetooth, printers, mouse, touchpad"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Bluetooth"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:bluetooth"
                    $name: Action
                  - tooltip: "Open Bluetooth settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Devices"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:connecteddevices"
                    $name: Action
                  - tooltip: "Open Devices settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Printers & scanners"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:printers"
                    $name: Action
                  - tooltip: "Open Printers & scanners settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Mouse"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:mousetouchpad"
                    $name: Action
                  - tooltip: "Open Mouse settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Touchpad"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:devices-touchpad"
                    $name: Action
                  - tooltip: "Open Touchpad settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Network & internet"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Wi-Fi, ethernet, VPN, proxy"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Wi-Fi"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:network-wifi"
                    $name: Action
                  - tooltip: "Open Wi-Fi settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Ethernet"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:network-ethernet"
                    $name: Action
                  - tooltip: "Open Ethernet settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "VPN"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:network-vpn"
                    $name: Action
                  - tooltip: "Open VPN settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Proxy"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:network-proxy"
                    $name: Action
                  - tooltip: "Open Proxy settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Advanced network settings"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:network-advancedsettings"
                    $name: Action
                  - tooltip: "Open Advanced network settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Personalization"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Background, colors, themes, taskbar"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Background"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:personalization-background"
                    $name: Action
                  - tooltip: "Open Background settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Colors"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:colors"
                    $name: Action
                  - tooltip: "Open Colors settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Themes"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:themes"
                    $name: Action
                  - tooltip: "Open Themes settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Lock screen"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:lockscreen"
                    $name: Action
                  - tooltip: "Open Lock screen settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Taskbar"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:taskbar"
                    $name: Action
                  - tooltip: "Open Taskbar settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Apps"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Installed, default, startup apps"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Installed apps"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:appsfeatures"
                    $name: Action
                  - tooltip: "Open Installed apps settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Default apps"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:defaultapps"
                    $name: Action
                  - tooltip: "Open Default apps settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Startup apps"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:startupapps"
                    $name: Action
                  - tooltip: "Open Startup apps settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Optional features"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:optionalfeatures"
                    $name: Action
                  - tooltip: "Open Optional features settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Accounts"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Your info, sign-in, family"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Your info"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:yourinfo"
                    $name: Action
                  - tooltip: "Open Your info settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Email & accounts"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:emailandaccounts"
                    $name: Action
                  - tooltip: "Open Email & accounts settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Sign-in options"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:signinoptions"
                    $name: Action
                  - tooltip: "Open Sign-in options settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Family"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:family-group"
                    $name: Action
                  - tooltip: "Open Family settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Time & language"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Date, language, typing, speech"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Date & time"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:dateandtime"
                    $name: Action
                  - tooltip: "Open Date & time settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Language & region"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:regionlanguage"
                    $name: Action
                  - tooltip: "Open Language & region settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Typing"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:typing"
                    $name: Action
                  - tooltip: "Open Typing settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Speech"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:speech"
                    $name: Action
                  - tooltip: "Open Speech settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Gaming"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Game Bar, captures, Game Mode"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Game Bar"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:gaming-gamebar"
                    $name: Action
                  - tooltip: "Open Game Bar settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Captures"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:gaming-gamedvr"
                    $name: Action
                  - tooltip: "Open Captures settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Game Mode"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:gaming-gamemode"
                    $name: Action
                  - tooltip: "Open Game Mode settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Accessibility"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Visual effects, pointer, cursor, narrator"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Visual effects"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:easeofaccess-visualeffects"
                    $name: Action
                  - tooltip: "Open Visual effects settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Text size"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:easeofaccess-display"
                    $name: Action
                  - tooltip: "Open Text size settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Mouse pointer"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:easeofaccess-mousepointer"
                    $name: Action
                  - tooltip: "Open Mouse pointer settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Text cursor"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:easeofaccess-cursor"
                    $name: Action
                  - tooltip: "Open Text cursor settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Narrator"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:easeofaccess-narrator"
                    $name: Action
                  - tooltip: "Open Narrator settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Privacy & security"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Security and app permissions"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Windows Security"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:windowsdefender"
                    $name: Action
                  - tooltip: "Open Windows Security settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Privacy dashboard"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:privacy"
                    $name: Action
                  - tooltip: "Open Privacy dashboard"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Location"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:privacy-location"
                    $name: Action
                  - tooltip: "Open Location privacy settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Camera"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:privacy-webcam"
                    $name: Action
                  - tooltip: "Open Camera privacy settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Microphone"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:privacy-microphone"
                    $name: Action
                  - tooltip: "Open Microphone privacy settings"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            - - type: menu
                $name: Item type
                $options:
                - button: Menu item
                - menu: Submenu
              - label: "Windows Update"
                $name: Name
              - icon: ""
                $name: Icon
                $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
              - action: ""
                $name: Action
              - tooltip: "Updates, history, advanced options"
                $name: Tooltip
              - separatorAfter: false
                $name: Separator after
              - subItems:
                - - label: "Windows Update"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:windowsupdate"
                    $name: Action
                  - tooltip: "Open Windows Update"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Update history"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:windowsupdate-history"
                    $name: Action
                  - tooltip: "Open Update history"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Advanced options"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:windowsupdate-options"
                    $name: Action
                  - tooltip: "Open Windows Update advanced options"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                - - label: "Delivery Optimization"
                    $name: Name
                  - icon: ""
                    $name: Icon
                    $description: Segoe Fluent glyph or .exe, .dll, or .ico path (optional ,index).
                  - action: "ms-settings:delivery-optimization"
                    $name: Action
                  - tooltip: "Open Delivery Optimization"
                    $name: Tooltip
                  - separatorAfter: false
                    $name: Separator after
                $name: Submenu items
            $name: Menu items
  $name: Buttons
  $description: >-
    Add top-level taskbar items here. Use type=button for a direct click action,
    or type=menu for a dropdown. Existing simple items without type are treated
    as buttons. File icons can point to .exe, .dll, or .ico files.
*/
// ==/WindhawkModSettings==

#define INITGUID
#include <windhawk_utils.h>

#include <windows.h>
#include <shellapi.h>
#include <wincodec.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>

#include <algorithm>
#include <cmath>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Markup;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;

struct MenuBarButton {
    bool isMenu = false;
    std::wstring label;
    std::wstring iconRaw;
    std::wstring action;
    std::wstring tooltip;
    std::wstring state;
    std::wstring displayMode;
    bool separatorAfter = false;
    std::vector<MenuBarButton> subItems;
};

struct Settings {
    std::wstring placement      = L"taskbar_right_start";
    std::wstring buttonContentAlignment = L"center";
    std::wstring buttonWidthMode = L"dynamic";
    bool         enableTooltips = true;
    int          customButtonWidth = 80;
    int          buttonSpacing  = 8;
    int          iconLabelSpacing = 6;
    int          buttonOffsetX  = 0;
    int          buttonOffsetY  = 0;
    int          buttonPaddingX  = 8;
    int          buttonPaddingY  = 4;
    int          iconSize       = 16;
    int          textSize       = 13;
};

static Settings g_settings;
static std::vector<MenuBarButton> g_buttons;
static std::mutex g_buttonsMutex;
static HWND g_taskbarWnd = nullptr;
static bool g_unloading = false;
static bool g_applyingSettings = false;
static const int kOverlayColumn = -1;

using WindowThreadProc = void(*)(void*);
using TrayUI_StartTaskbar_t = void(WINAPI*)(void*);
using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void*, void*);
using TaskbarHost_FrameHeight_t = int(WINAPI*)(void*);
using Std_Ref_Decref_t = void(WINAPI*)(void*);

static TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original = nullptr;
static CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original = nullptr;
static TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original = nullptr;
static Std_Ref_Decref_t Std_Ref_Decref_Original = nullptr;
static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static Grid g_menuBarRoot = nullptr;
static FrameworkElement g_menuBarHost = nullptr;
static Grid g_injectionParent = nullptr;
static FrameworkElement g_menuBarAnchor = nullptr;
static Grid g_layoutUpdatedSource = nullptr;
static event_token g_layoutUpdatedToken{};
static bool g_layoutUpdatedAttached = false;
static FrameworkElement g_trackedElement = nullptr;
static Thickness g_trackedElementOriginalMargin{};
static bool g_hasTrackedElementOriginalMargin = false;
static std::wstring g_trackPosition;
static int g_placementKind = 0;
static double g_lastMenuBarX = -1.0;
static double g_lastMenuBarY = -1.0;
static bool g_hasLastMenuBarPlacement = false;
static std::atomic<bool> g_retryScheduled{false};

static std::wstring Trim(std::wstring s) {
    auto isWs = [](wchar_t c) { return iswspace(c) != 0; };
    while (!s.empty() && isWs(s.front())) s.erase(s.begin());
    while (!s.empty() && isWs(s.back())) s.pop_back();
    return s;
}

static std::wstring ToUpper(std::wstring s) {
    for (auto& ch : s) ch = (wchar_t)towupper(ch);
    return s;
}

static bool StartsWithCI(const std::wstring& s, const wchar_t* prefix) {
    size_t n = wcslen(prefix);
    return s.size() >= n && _wcsnicmp(s.c_str(), prefix, n) == 0;
}

static bool SplitCommandAndArguments(const std::wstring& raw, std::wstring& file, std::wstring& args) {
    std::wstring s = Trim(raw);
    if (s.empty()) return false;

    if (s.front() == L'"') {
        size_t end = s.find(L'"', 1);
        if (end == std::wstring::npos || end == 1) return false;
        file = s.substr(1, end - 1);
        size_t pos = end + 1;
        while (pos < s.size() && iswspace(s[pos])) ++pos;
        args = s.substr(pos);
        return !file.empty();
    }

    size_t pos = 0;
    while (pos < s.size() && !iswspace(s[pos])) ++pos;
    file = s.substr(0, pos);
    while (pos < s.size() && iswspace(s[pos])) ++pos;
    args = s.substr(pos);
    return !file.empty();
}

static bool LaunchDefaultAction(const std::wstring& raw) {
    std::wstring file;
    std::wstring args;
    if (!SplitCommandAndArguments(raw, file, args)) return false;

    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_DOENVSUBST;
    sei.lpVerb = L"open";
    sei.lpFile = file.c_str();
    sei.lpParameters = args.empty() ? nullptr : args.c_str();
    sei.nShow = SW_SHOWNORMAL;
    return ShellExecuteExW(&sei) != FALSE;
}

static bool ParseIntPair(const wchar_t* key, const wchar_t* def, int& a, int& b) {
    PCWSTR p = Wh_GetStringSetting(key);
    std::wstring s = p ? p : def;
    if (p) Wh_FreeStringSetting(p);

    for (auto& ch : s) {
        if (ch == L',') ch = L' ';
    }

    int x = 0, y = 0;
    if (swscanf_s(s.c_str(), L"%d %d", &x, &y) == 2) {
        a = x;
        b = y;
        return true;
    }
    if (swscanf_s(def, L"%d %d", &x, &y) == 2) {
        a = x;
        b = y;
    }
    return false;
}

static std::wstring GetStringSetting(const wchar_t* key, const wchar_t* def = L"") {
    PCWSTR p = Wh_GetStringSetting(key);
    std::wstring s = p ? p : def;
    if (p) Wh_FreeStringSetting(p);
    return s;
}

static int GetIntSetting(const wchar_t* key, int minValue, int maxValue, int defValue) {
    int v = Wh_GetIntSetting(key);
    if (v < minValue || v > maxValue) return defValue;
    return v;
}

static bool TryParseShortcut(std::wstring_view shortcut, UINT* modifiersOut, UINT* vkOut) {
    auto trim = [](std::wstring_view s) {
        size_t start = 0;
        while (start < s.size() && iswspace(s[start])) start++;
        size_t end = s.size();
        while (end > start && iswspace(s[end - 1])) end--;
        return s.substr(start, end - start);
    };

    std::unordered_map<std::wstring, UINT> modifiersMap = {
        {L"ALT", MOD_ALT},
        {L"CTRL", MOD_CONTROL},
        {L"CONTROL", MOD_CONTROL},
        {L"SHIFT", MOD_SHIFT},
        {L"WIN", MOD_WIN},
    };

    std::unordered_map<std::wstring, UINT> vkMap = {
        {L"TAB", VK_TAB},
        {L"ENTER", VK_RETURN},
        {L"RETURN", VK_RETURN},
        {L"SPACE", VK_SPACE},
        {L"ESC", VK_ESCAPE},
        {L"ESCAPE", VK_ESCAPE},
        {L"BACKSPACE", VK_BACK},
        {L"HOME", VK_HOME},
        {L"END", VK_END},
        {L"PAGEUP", VK_PRIOR},
        {L"PAGEDOWN", VK_NEXT},
        {L"INSERT", VK_INSERT},
        {L"DELETE", VK_DELETE},
        {L"LEFT", VK_LEFT},
        {L"RIGHT", VK_RIGHT},
        {L"UP", VK_UP},
        {L"DOWN", VK_DOWN},
        {L"NUMLOCK", VK_NUMLOCK},
        {L"VOLUMEMUTE", VK_VOLUME_MUTE},
        {L"VOLUMEUP", VK_VOLUME_UP},
        {L"VOLUMEDOWN", VK_VOLUME_DOWN},
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
        std::wstring_view part = plus == std::wstring_view::npos
            ? shortcut.substr(start)
            : shortcut.substr(start, plus - start);
        part = trim(part);
        if (part.empty()) return false;

        std::wstring token(part);
        token = ToUpper(std::move(token));

        auto modIt = modifiersMap.find(token);
        if (modIt != modifiersMap.end()) {
            modifiers |= modIt->second;
        } else {
            if (vk != 0) return false;

            if (token.size() == 1 && token[0] >= L'A' && token[0] <= L'Z') {
                vk = (UINT)token[0];
            } else if (token.size() == 1 && token[0] >= L'0' && token[0] <= L'9') {
                vk = (UINT)token[0];
            } else if (token.size() >= 2 && token[0] == L'F') {
                int fn = _wtoi(token.c_str() + 1);
                if (fn >= 1 && fn <= 24) {
                    vk = VK_F1 + (UINT)(fn - 1);
                } else {
                    return false;
                }
            } else if (token.rfind(L"NUMPAD", 0) == 0 && token.size() == 7 &&
                       token[6] >= L'0' && token[6] <= L'9') {
                vk = VK_NUMPAD0 + (UINT)(token[6] - L'0');
            } else {
                auto vkIt = vkMap.find(token);
                if (vkIt != vkMap.end()) {
                    vk = vkIt->second;
                } else {
                    try {
                        size_t pos = 0;
                        unsigned long parsed = std::stoul(token, &pos, 0);
                        if (pos != token.size() || parsed == 0 || parsed > 0xFF) return false;
                        vk = (UINT)parsed;
                    } catch (...) {
                        return false;
                    }
                }
            }
        }

        if (plus == std::wstring_view::npos) break;
        start = plus + 1;
    }

    if (vk == 0) return false;
    *modifiersOut = modifiers;
    *vkOut = vk;
    return true;
}

static bool SendShortcut(UINT modifiers, UINT vk) {
    std::vector<INPUT> inputs;
    inputs.reserve(10);

    auto addKey = [&inputs](WORD key, DWORD flags) {
        INPUT in = {};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = key;
        in.ki.dwFlags = flags;
        inputs.push_back(in);
    };

    if (modifiers & MOD_CONTROL) addKey(VK_CONTROL, 0);
    if (modifiers & MOD_SHIFT) addKey(VK_SHIFT, 0);
    if (modifiers & MOD_ALT) addKey(VK_MENU, 0);
    if (modifiers & MOD_WIN) addKey(VK_LWIN, 0);

    addKey((WORD)vk, 0);
    addKey((WORD)vk, KEYEVENTF_KEYUP);

    if (modifiers & MOD_WIN) addKey(VK_LWIN, KEYEVENTF_KEYUP);
    if (modifiers & MOD_ALT) addKey(VK_MENU, KEYEVENTF_KEYUP);
    if (modifiers & MOD_SHIFT) addKey(VK_SHIFT, KEYEVENTF_KEYUP);
    if (modifiers & MOD_CONTROL) addKey(VK_CONTROL, KEYEVENTF_KEYUP);

    UINT sent = SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
    return sent == inputs.size();
}

static void ExecuteAction(const std::wstring& raw) {
    if (raw.empty()) return;

    std::thread([raw]() {
        const std::wstring& a = raw;

        if (StartsWithCI(a, L"web:")) {
            ShellExecuteW(nullptr, L"open", a.substr(4).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            return;
        }
        if (StartsWithCI(a, L"ms-settings:")) {
            ShellExecuteW(nullptr, L"open", a.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            return;
        }
        if (StartsWithCI(a, L"cmd:")) {
            std::wstring arg = L"/C " + a.substr(4);
            ShellExecuteW(nullptr, L"open", L"cmd.exe", arg.c_str(), nullptr, SW_HIDE);
            return;
        }
        if (StartsWithCI(a, L"powershell:") || StartsWithCI(a, L"ps:")) {
            std::wstring script = StartsWithCI(a, L"powershell:") ? a.substr(11) : a.substr(3);
            std::wstring arg = L"-NoProfile -ExecutionPolicy Bypass -Command " + script;
            ShellExecuteW(nullptr, L"open", L"powershell.exe", arg.c_str(), nullptr, SW_HIDE);
            return;
        }
        if (StartsWithCI(a, L"shell:")) {
            HINSTANCE result = ShellExecuteW(nullptr, L"open", a.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            if ((INT_PTR)result > 32) {
                return;
            }

            // Compatibility for older configs that used shell: as a PowerShell prefix.
            std::wstring arg = L"-NoProfile -ExecutionPolicy Bypass -Command " + a.substr(6);
            ShellExecuteW(nullptr, L"open", L"powershell.exe", arg.c_str(), nullptr, SW_HIDE);
            return;
        }
        if (StartsWithCI(a, L"key:") || StartsWithCI(a, L"hotkey:")) {
            std::wstring shortcut = StartsWithCI(a, L"key:") ? a.substr(4) : a.substr(7);
            UINT modifiers = 0;
            UINT vk = 0;
            if (!TryParseShortcut(shortcut, &modifiers, &vk)) {
                Wh_Log(L"Invalid shortcut action: %s", shortcut.c_str());
                return;
            }
            if (!SendShortcut(modifiers, vk)) {
                Wh_Log(L"Failed to send shortcut: %s", shortcut.c_str());
            }
            return;
        }
        if (!LaunchDefaultAction(a)) {
            Wh_Log(L"Failed to launch default action: %s", a.c_str());
        }
    }).detach();
}

static SolidColorBrush GetTextBrush() {
    try {
        auto resources = Application::Current().Resources();
        for (const auto& key : {L"SystemControlForegroundBaseHighBrush", L"TextFillColorPrimaryBrush"}) {
            auto value = resources.Lookup(box_value(key));
            if (auto brush = value.try_as<SolidColorBrush>()) {
                return brush;
            }
        }
    } catch (...) {}
    return SolidColorBrush(Colors::White());
}

static void ApplyEmptyXamlStyle(FrameworkElement const& element, const wchar_t* typeNameString);
static FrameworkElement FindChildByName(FrameworkElement const& root, std::wstring_view name, int depth);

static bool LooksLikeIconPath(const std::wstring& iconRaw) {
    if (iconRaw.find(L'\\') != std::wstring::npos ||
        iconRaw.find(L'/') != std::wstring::npos ||
        iconRaw.find(L':') != std::wstring::npos) {
        return true;
    }

    std::wstring lower = iconRaw;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t ch) {
        return (wchar_t)towlower(ch);
    });
    size_t comma = lower.rfind(L',');
    if (comma != std::wstring::npos) lower.resize(comma);
    return lower.size() >= 4 &&
           (lower.compare(lower.size() - 4, 4, L".exe") == 0 ||
            lower.compare(lower.size() - 4, 4, L".dll") == 0 ||
            lower.compare(lower.size() - 4, 4, L".ico") == 0);
}

static std::wstring ExpandEnvironmentVariables(const std::wstring& value) {
    DWORD length = ExpandEnvironmentStringsW(value.c_str(), nullptr, 0);
    if (length == 0) return value;

    std::wstring expanded(length, L'\0');
    if (ExpandEnvironmentStringsW(value.c_str(), expanded.data(), length) == 0) {
        return value;
    }
    expanded.resize(length - 1);
    return expanded;
}

static HICON LoadIconFromFile(const std::wstring& iconRaw) {
    std::wstring path = ExpandEnvironmentVariables(iconRaw);
    int iconIndex = 0;
    size_t comma = path.rfind(L',');
    if (comma != std::wstring::npos && comma + 1 < path.size()) {
        const wchar_t* indexText = path.c_str() + comma + 1;
        wchar_t* end = nullptr;
        long parsed = wcstol(indexText, &end, 10);
        if (end && *end == L'\0') {
            iconIndex = (int)parsed;
            path.resize(comma);
        }
    }

    HICON icon = nullptr;
    if (ExtractIconExW(path.c_str(), iconIndex, &icon, nullptr, 1) && icon) {
        return icon;
    }

    SHFILEINFOW fileInfo{};
    if (SHGetFileInfoW(path.c_str(), 0, &fileInfo, sizeof(fileInfo),
                       SHGFI_ICON | SHGFI_LARGEICON)) {
        return fileInfo.hIcon;
    }

    Wh_Log(L"Couldn't load an icon from %s", iconRaw.c_str());
    return nullptr;
}

struct DecodedIcon {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;

    bool empty() const { return pixels.empty(); }
};

static bool ReadBitmapPixels(HBITMAP bitmap, DecodedIcon* decoded) {
    BITMAP details{};
    if (!GetObject(bitmap, sizeof(details), &details) || details.bmWidth <= 0 ||
        details.bmHeight <= 0) {
        return false;
    }

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = details.bmWidth;
    info.bmiHeader.biHeight = -details.bmHeight;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    std::vector<uint8_t> pixels((size_t)details.bmWidth * details.bmHeight * 4);
    HDC dc = CreateCompatibleDC(nullptr);
    if (!dc) return false;
    bool succeeded = GetDIBits(dc, bitmap, 0, details.bmHeight, pixels.data(),
                               &info, DIB_RGB_COLORS) != 0;
    DeleteDC(dc);
    if (!succeeded) return false;

    decoded->width = details.bmWidth;
    decoded->height = details.bmHeight;
    decoded->pixels = std::move(pixels);
    return true;
}

static bool HasNoAlphaChannel(const std::vector<uint8_t>& pixels) {
    for (size_t i = 3; i < pixels.size(); i += 4) {
        if (pixels[i]) return false;
    }
    return true;
}

static void MakeOpaque(std::vector<uint8_t>& pixels) {
    for (size_t i = 3; i < pixels.size(); i += 4) pixels[i] = 255;
}

static bool DecodeIcon(HICON icon, DecodedIcon* decoded) {
    ICONINFO info{};
    if (!GetIconInfo(icon, &info)) return false;

    bool succeeded = false;
    if (info.hbmColor && ReadBitmapPixels(info.hbmColor, decoded)) {
        if (HasNoAlphaChannel(decoded->pixels)) {
            MakeOpaque(decoded->pixels);
        } else {
            // XAML expects premultiplied alpha.
            for (size_t i = 0; i < decoded->pixels.size(); i += 4) {
                uint8_t alpha = decoded->pixels[i + 3];
                if (alpha != 255) {
                    decoded->pixels[i] = decoded->pixels[i] * alpha / 255;
                    decoded->pixels[i + 1] = decoded->pixels[i + 1] * alpha / 255;
                    decoded->pixels[i + 2] = decoded->pixels[i + 2] * alpha / 255;
                }
            }
        }
        succeeded = true;
    }

    if (info.hbmColor) DeleteObject(info.hbmColor);
    if (info.hbmMask) DeleteObject(info.hbmMask);
    return succeeded;
}

static std::mutex g_fileIconCacheMutex;
static std::unordered_map<std::wstring, std::shared_ptr<DecodedIcon>> g_fileIconCache;

static std::shared_ptr<DecodedIcon> GetFileIcon(const std::wstring& iconRaw) {
    std::lock_guard<std::mutex> lock(g_fileIconCacheMutex);
    auto found = g_fileIconCache.find(iconRaw);
    if (found != g_fileIconCache.end()) return found->second;

    auto decoded = std::make_shared<DecodedIcon>();
    if (HICON icon = LoadIconFromFile(iconRaw)) {
        DecodeIcon(icon, decoded.get());
        DestroyIcon(icon);
    }
    return g_fileIconCache.emplace(iconRaw, decoded).first->second;
}

static ImageSource CreateImageSource(const DecodedIcon& decoded) {
    if (decoded.empty()) return nullptr;
    try {
        WriteableBitmap bitmap(decoded.width, decoded.height);
        memcpy(bitmap.PixelBuffer().data(), decoded.pixels.data(), decoded.pixels.size());
        bitmap.Invalidate();
        return bitmap;
    } catch (...) {
        return nullptr;
    }
}

static bool WritePngIconFile(const std::wstring& filePath, const DecodedIcon& decoded) {
    if (decoded.empty()) return false;

    try {
        // DecodedIcon uses premultiplied pixels for WriteableBitmap. PNG flyout
        // icons need conventional BGRA pixels or transparent pixels turn black.
        std::vector<uint8_t> pixels = decoded.pixels;
        for (size_t i = 0; i < pixels.size(); i += 4) {
            const uint8_t alpha = pixels[i + 3];
            if (alpha != 0 && alpha != 255) {
                pixels[i] = (uint8_t)std::min(255u, pixels[i] * 255u / alpha);
                pixels[i + 1] = (uint8_t)std::min(255u, pixels[i + 1] * 255u / alpha);
                pixels[i + 2] = (uint8_t)std::min(255u, pixels[i + 2] * 255u / alpha);
            }
        }

        com_ptr<IWICImagingFactory> factory;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                    CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARGS(factory.put())))) {
            return false;
        }

        com_ptr<IWICStream> stream;
        if (FAILED(factory->CreateStream(stream.put())) ||
            FAILED(stream->InitializeFromFilename(filePath.c_str(), GENERIC_WRITE))) {
            return false;
        }

        com_ptr<IWICBitmapEncoder> encoder;
        if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr,
                                          encoder.put())) ||
            FAILED(encoder->Initialize(stream.get(), WICBitmapEncoderNoCache))) {
            return false;
        }

        com_ptr<IWICBitmapFrameEncode> frame;
        if (FAILED(encoder->CreateNewFrame(frame.put(), nullptr)) ||
            FAILED(frame->Initialize(nullptr)) ||
            FAILED(frame->SetSize(decoded.width, decoded.height))) {
            return false;
        }

        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        if (FAILED(frame->SetPixelFormat(&format)) ||
            format != GUID_WICPixelFormat32bppBGRA) {
            return false;
        }

        UINT stride = decoded.width * 4;
        UINT bufferSize = stride * decoded.height;
        if (FAILED(frame->WritePixels(decoded.height, stride, bufferSize,
                                      const_cast<BYTE*>(decoded.pixels.data()))) ||
            FAILED(frame->Commit()) ||
            FAILED(encoder->Commit())) {
            return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

static std::wstring GetFileIconBitmapUri(const std::wstring& iconRaw) {
    auto decoded = GetFileIcon(iconRaw);
    if (!decoded || decoded->empty()) return {};

    // BitmapIcon only accepts a URI. Persist extracted pixels as PNG so alpha
    // stays transparent in menu/flyout icons.
    uint64_t hash = 1469598103934665603ULL;
    for (wchar_t ch : iconRaw) {
        hash ^= (uint64_t)towlower(ch);
        hash *= 1099511628211ULL;
    }

    WCHAR tempPath[MAX_PATH];
    DWORD tempLength = GetTempPathW(ARRAYSIZE(tempPath), tempPath);
    if (tempLength == 0 || tempLength >= ARRAYSIZE(tempPath)) return {};

    std::wstring directory(tempPath);
    directory += L"WindhawkCustomTaskbarToolbars";
    CreateDirectoryW(directory.c_str(), nullptr);

    WCHAR fileName[64];
    // Version the cache name so a failed/partial older conversion is never
    // reused after the menu is rebuilt.
    swprintf_s(fileName, L"\\icon-v2-%016llX.png", (unsigned long long)hash);
    std::wstring filePath = directory + fileName;

    if (GetFileAttributesW(filePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        if (!WritePngIconFile(filePath, *decoded)) {
            DeleteFileW(filePath.c_str());
            return {};
        }
    }

    std::replace(filePath.begin(), filePath.end(), L'\\', L'/');
    return L"file:///" + filePath;
}

static FrameworkElement MakeGlyphIcon(const std::wstring& iconRaw, double size) {
    try {
        WCHAR glyph = 0xE700;
        if (!iconRaw.empty()) {
            glyph = (WCHAR)wcstoul(iconRaw.c_str(), nullptr, 16);
            if (glyph == 0) glyph = 0xE700;
        }

        TextBlock tb;
        tb.Name(L"TaskbarMenuBarButtonIcon");
        tb.Tag(box_value(hstring(L"TaskbarMenuBarButtonIcon")));
        tb.Text(hstring(std::wstring(1, glyph)));
        tb.FontFamily(FontFamily(L"Segoe Fluent Icons"));
        tb.FontSize(size);
        tb.Foreground(GetTextBrush());
        tb.HorizontalAlignment(HorizontalAlignment::Center);
        tb.VerticalAlignment(VerticalAlignment::Center);
        return tb;
    } catch (...) {
        return nullptr;
    }
}

static FrameworkElement MakeIconElement(const std::wstring& iconRaw, double size) {
    if (iconRaw.empty()) return nullptr;
    if (LooksLikeIconPath(iconRaw)) {
        if (auto source = CreateImageSource(*GetFileIcon(iconRaw))) {
            Image image;
            image.Name(L"TaskbarMenuBarButtonIcon");
            image.Tag(box_value(hstring(L"TaskbarMenuBarButtonIcon")));
            image.Source(source);
            image.Width(size);
            image.Height(size);
            image.HorizontalAlignment(HorizontalAlignment::Center);
            image.VerticalAlignment(VerticalAlignment::Center);
            return image;
        }
        return nullptr;
    }
    return MakeGlyphIcon(iconRaw, size);
}

static IconElement MakeMenuIcon(const std::wstring& iconRaw) {
    if (iconRaw.empty()) return nullptr;

    try {
        if (LooksLikeIconPath(iconRaw)) {
            if (auto uri = GetFileIconBitmapUri(iconRaw); !uri.empty()) {
                BitmapIcon icon;
                icon.UriSource(Uri(uri));
                icon.ShowAsMonochrome(false);
                return icon;
            }
            return nullptr;
        }

        WCHAR glyph = (WCHAR)wcstoul(iconRaw.c_str(), nullptr, 16);
        if (glyph == 0) return nullptr;

        FontIcon icon;
        icon.Glyph(hstring(std::wstring(1, glyph)));
        icon.FontFamily(FontFamily(L"Segoe Fluent Icons"));
        icon.FontSize((double)g_settings.iconSize);
        icon.Foreground(GetTextBrush());
        return icon;
    } catch (...) {
        return nullptr;
    }
}

static void AppendMenuEntries(const std::vector<MenuBarButton>& items,
                              Windows::Foundation::Collections::IVector<MenuFlyoutItemBase> const& target);

static void SetOptionalToolTip(DependencyObject const& element,
                               const std::wstring& text,
                               const std::wstring& fallback = L"") {
    if (!g_settings.enableTooltips || !element) return;

    const std::wstring& tooltip = text.empty() ? fallback : text;
    if (tooltip.empty()) return;

    try {
        ToolTipService::SetToolTip(element, box_value(hstring(tooltip)));
    } catch (...) {}
}

static MenuFlyoutItemBase CreateMenuEntry(const MenuBarButton& item) {
    if (item.isMenu && !item.subItems.empty()) {
        MenuFlyoutSubItem subMenu;
        subMenu.Text(hstring(item.label));
        if (auto icon = MakeMenuIcon(item.iconRaw)) {
            subMenu.Icon(icon);
        }
        SetOptionalToolTip(subMenu, item.tooltip);
        AppendMenuEntries(item.subItems, subMenu.Items());
        return subMenu;
    }

    MenuFlyoutItem menuItem;
    menuItem.Text(hstring(item.label));
    if (auto icon = MakeMenuIcon(item.iconRaw)) {
        menuItem.Icon(icon);
    }
    SetOptionalToolTip(menuItem, item.tooltip);
    menuItem.Click([action = item.action](auto const&, auto const&) {
        ExecuteAction(action);
    });
    return menuItem;
}

static void AppendMenuEntries(const std::vector<MenuBarButton>& items,
                              Windows::Foundation::Collections::IVector<MenuFlyoutItemBase> const& target) {
    for (const auto& item : items) {
        if (item.label.empty() && item.iconRaw.empty() && item.action.empty() && item.subItems.empty()) {
            continue;
        }

        target.Append(CreateMenuEntry(item));
        if (item.separatorAfter) {
            target.Append(MenuFlyoutSeparator());
        }
    }
}

static std::wstring ResolveDisplayMode(const MenuBarButton& item) {
    if (item.displayMode == L"text" || item.displayMode == L"both" || item.displayMode == L"icon") {
        return item.displayMode;
    }
    // Old configurations did not require this setting per button.
    return L"both";
}

static bool IsButtonHidden(const MenuBarButton& item) {
    return item.state == L"hidden";
}

static bool IsButtonDisabled(const MenuBarButton& item) {
    return item.state == L"disabled";
}

static HorizontalAlignment ResolveButtonContentAlignment() {
    if (g_settings.buttonContentAlignment == L"left") return HorizontalAlignment::Left;
    if (g_settings.buttonContentAlignment == L"right") return HorizontalAlignment::Right;
    return HorizontalAlignment::Center;
}

static void ApplyButtonContentAlignment(FrameworkElement const& element, HorizontalAlignment alignment) {
    if (!element) return;

    try {
        element.HorizontalAlignment(alignment);

        if (auto tb = element.try_as<TextBlock>()) {
            if (alignment == HorizontalAlignment::Left) {
                tb.TextAlignment(TextAlignment::Left);
            } else if (alignment == HorizontalAlignment::Right) {
                tb.TextAlignment(TextAlignment::Right);
            } else {
                tb.TextAlignment(TextAlignment::Center);
            }
        }
    } catch (...) {}
}

static double GetButtonContentMinWidth() {
    if (g_settings.buttonContentAlignment == L"center") {
        return 0.0;
    }

    return std::max(64.0, (double)g_settings.iconSize + (double)g_settings.buttonPaddingX * 2.0 + 24.0);
}

static FrameworkElement MakeButtonContent(const MenuBarButton& item, const std::wstring& displayMode) {
    bool showText = (displayMode == L"text" || displayMode == L"both");
    bool showIcon = (displayMode == L"icon" || displayMode == L"both");
    bool hasText = !item.label.empty();
    bool hasIcon = !item.iconRaw.empty();

    if (displayMode == L"both" && (showIcon || showText)) {
        if (hasIcon && hasText) {
            StackPanel panel;
            panel.Name(L"TaskbarMenuBarButtonContent");
            panel.Tag(box_value(hstring(L"TaskbarMenuBarButtonContent")));
            ApplyEmptyXamlStyle(panel, L"Windows.UI.Xaml.Controls.StackPanel");
            panel.Orientation(Orientation::Horizontal);

            auto icon = MakeIconElement(item.iconRaw, (double)g_settings.iconSize);
            if (icon) {
                if (auto fe = icon.try_as<FrameworkElement>()) {
                    fe.Margin({0, 0, (double)g_settings.iconLabelSpacing, 0});
                }
                panel.Children().Append(icon);
            } else {
                TextBlock spacer;
                spacer.Name(L"TaskbarMenuBarButtonIcon");
                spacer.Tag(box_value(hstring(L"TaskbarMenuBarButtonIcon")));
                spacer.Text(L"");
                spacer.Margin({0, 0, (double)g_settings.iconLabelSpacing, 0});
                panel.Children().Append(spacer);
            }

            TextBlock text;
            text.Name(L"TaskbarMenuBarButtonLabel");
            text.Tag(box_value(hstring(L"TaskbarMenuBarButtonLabel")));
            text.Text(hstring(item.label));
            text.FontSize((double)g_settings.textSize);
            text.Foreground(GetTextBrush());
            text.TextWrapping(TextWrapping::NoWrap);
            text.TextTrimming(TextTrimming::CharacterEllipsis);
            text.VerticalAlignment(VerticalAlignment::Center);
            panel.Children().Append(text);
            ApplyButtonContentAlignment(panel, ResolveButtonContentAlignment());
            return panel;
        }

        if (hasIcon) {
            auto icon = MakeIconElement(item.iconRaw, (double)g_settings.iconSize);
            if (auto fe = icon.try_as<FrameworkElement>()) {
                ApplyButtonContentAlignment(fe, ResolveButtonContentAlignment());
            }
            return icon;
        }

        if (hasText) {
            TextBlock text;
            text.Name(L"TaskbarMenuBarButtonLabel");
            text.Tag(box_value(hstring(L"TaskbarMenuBarButtonLabel")));
            text.Text(hstring(item.label));
            text.FontSize((double)g_settings.textSize);
            text.Foreground(GetTextBrush());
            text.TextWrapping(TextWrapping::NoWrap);
            text.TextTrimming(TextTrimming::CharacterEllipsis);
            text.VerticalAlignment(VerticalAlignment::Center);
            ApplyButtonContentAlignment(text, ResolveButtonContentAlignment());
            return text;
        }
    }

    if (showIcon && !showText) {
        if (hasIcon) {
            auto icon = MakeIconElement(item.iconRaw, (double)g_settings.iconSize);
            if (auto fe = icon.try_as<FrameworkElement>()) {
                ApplyButtonContentAlignment(fe, ResolveButtonContentAlignment());
            }
            return icon;
        }
        auto glyph = MakeGlyphIcon(L"E700", (double)g_settings.iconSize);
        if (auto fe = glyph.try_as<FrameworkElement>()) {
            ApplyButtonContentAlignment(fe, ResolveButtonContentAlignment());
        }
        return glyph;
    }

    TextBlock text;
    text.Name(L"TaskbarMenuBarButtonLabel");
    text.Tag(box_value(hstring(L"TaskbarMenuBarButtonLabel")));
    text.Text(hstring(item.label.empty() ? L"Button" : item.label));
    text.FontSize((double)g_settings.textSize);
    text.Foreground(GetTextBrush());
    text.TextWrapping(TextWrapping::NoWrap);
    text.TextTrimming(TextTrimming::CharacterEllipsis);
    text.VerticalAlignment(VerticalAlignment::Center);
    ApplyButtonContentAlignment(text, ResolveButtonContentAlignment());
    return text;
}

static Style CreateTaskbarMenuBarHitTargetStyle() {
    static const wchar_t kStyleXaml[] = LR"XAML(
<Style TargetType="Button"
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
  <Setter Property="Background" Value="#00FFFFFF"/>
  <Setter Property="BorderBrush" Value="Transparent"/>
  <Setter Property="BorderThickness" Value="0"/>
  <Setter Property="Padding" Value="0"/>
  <Setter Property="UseSystemFocusVisuals" Value="False"/>
  <Setter Property="Template">
    <Setter.Value>
      <ControlTemplate TargetType="Button">
        <Grid Background="#00FFFFFF">
          <ContentPresenter Content="{TemplateBinding Content}"
                            ContentTemplate="{TemplateBinding ContentTemplate}"
                            HorizontalAlignment="{TemplateBinding HorizontalContentAlignment}"
                            VerticalAlignment="{TemplateBinding VerticalContentAlignment}"
                            IsHitTestVisible="False"/>
        </Grid>
      </ControlTemplate>
    </Setter.Value>
  </Setter>
</Style>)XAML";

    try {
        return XamlReader::Load(hstring(kStyleXaml)).as<Style>();
    } catch (...) {
        return nullptr;
    }
}

static bool TaskbarMenusOpenDownward() {
    RECT taskbarRect{};
    if (!g_taskbarWnd || !GetWindowRect(g_taskbarWnd, &taskbarRect)) {
        return false;
    }

    MONITORINFO monitorInfo{sizeof(monitorInfo)};
    HMONITOR monitor = MonitorFromWindow(g_taskbarWnd, MONITOR_DEFAULTTONEAREST);
    if (!monitor || !GetMonitorInfoW(monitor, &monitorInfo)) {
        return false;
    }

    const LONG taskbarCenterY = (taskbarRect.top + taskbarRect.bottom) / 2;
    const LONG monitorCenterY = (monitorInfo.rcMonitor.top + monitorInfo.rcMonitor.bottom) / 2;
    return taskbarCenterY < monitorCenterY;
}

static Brush MakeTaskbarVisualStateBackground(hstring const& stateName) {
    if (stateName == L"Pressed") {
        return SolidColorBrush(Color{0x38, 0xFF, 0xFF, 0xFF});
    }
    if (stateName == L"PointerOver") {
        return SolidColorBrush(Color{0x24, 0xFF, 0xFF, 0xFF});
    }
    return SolidColorBrush(Colors::Transparent());
}

static void EnableTaskbarStylerButtonStates(Button const& hitTarget,
                                            Border const& highlightBackground,
                                            UIElement const& animatedIcon) {
    if (!hitTarget || !highlightBackground) return;

    auto isPressed = std::make_shared<bool>(false);
    auto isHovered = std::make_shared<bool>(false);

    auto updateState = [hitTarget, highlightBackground, animatedIcon,
                        isPressed, isHovered]() {
        try {
            hstring stateName = L"Normal";
            if (!hitTarget.IsEnabled()) {
                stateName = L"Disabled";
            } else if (*isPressed) {
                stateName = L"Pressed";
            } else if (*isHovered) {
                stateName = L"PointerOver";
            }

            double scale = stateName == L"Pressed" ? 0.92 : 1.0;

            highlightBackground.Background(MakeTaskbarVisualStateBackground(stateName));

            if (animatedIcon) {
                auto transform = animatedIcon.RenderTransform().try_as<ScaleTransform>();
                if (!transform) {
                    transform = ScaleTransform();
                    animatedIcon.RenderTransform(transform);
                    animatedIcon.RenderTransformOrigin({0.5, 0.5});
                }
                transform.ScaleX(scale);
                transform.ScaleY(scale);
            }
        } catch (...) {}
    };

    hitTarget.PointerEntered(PointerEventHandler([isHovered, updateState](
        IInspectable const&, PointerRoutedEventArgs const&) {
        *isHovered = true;
        updateState();
    }));

    hitTarget.PointerExited(PointerEventHandler([isPressed, isHovered, updateState](
        IInspectable const&, PointerRoutedEventArgs const&) {
        *isPressed = false;
        *isHovered = false;
        updateState();
    }));

    auto pressedHandler = PointerEventHandler([isPressed, updateState](
        IInspectable const& sender, PointerRoutedEventArgs const& e) {
        if (auto elem = sender.try_as<UIElement>()) {
            bool animatePress = false;
            try {
                auto properties = e.GetCurrentPoint(elem).Properties();
                animatePress = properties.IsLeftButtonPressed() ||
                               properties.IsRightButtonPressed();
            } catch (...) {}
            if (!animatePress) {
                return;
            }
            elem.CapturePointer(e.Pointer());
        }
        *isPressed = true;
        updateState();
    });

    auto releasedHandler = PointerEventHandler([isPressed, isHovered, updateState](
        IInspectable const& sender, PointerRoutedEventArgs const& e) {
        bool actuallyHovered = false;
        if (auto elem = sender.try_as<UIElement>()) {
            elem.ReleasePointerCapture(e.Pointer());
            try {
                auto size = elem.RenderSize();
                auto pos = e.GetCurrentPoint(elem).Position();
                actuallyHovered =
                    pos.X >= 0 && pos.X <= size.Width &&
                    pos.Y >= 0 && pos.Y <= size.Height;
            } catch (...) {}
        }

        *isPressed = false;
        *isHovered = actuallyHovered;
        updateState();
    });

    auto canceledHandler = PointerEventHandler([isPressed, isHovered, updateState](
        IInspectable const&, PointerRoutedEventArgs const&) {
        *isPressed = false;
        *isHovered = false;
        updateState();
    });

    auto captureLostHandler = PointerEventHandler([isPressed, isHovered, updateState](
        IInspectable const&, PointerRoutedEventArgs const&) {
        *isPressed = false;
        *isHovered = false;
        updateState();
    });

    hitTarget.AddHandler(UIElement::PointerPressedEvent(), box_value(pressedHandler), true);
    hitTarget.AddHandler(UIElement::PointerReleasedEvent(), box_value(releasedHandler), true);
    hitTarget.AddHandler(UIElement::PointerCanceledEvent(), box_value(canceledHandler), true);
    hitTarget.AddHandler(UIElement::PointerCaptureLostEvent(), box_value(captureLostHandler), true);

    hitTarget.Loaded(RoutedEventHandler([updateState](
        IInspectable const&, RoutedEventArgs const&) {
        updateState();
    }));

    updateState();
}

static void ApplyEmptyXamlStyle(FrameworkElement const& element, const wchar_t* typeNameString) {
    if (!element || !typeNameString) return;

    try {
        winrt::Windows::UI::Xaml::Interop::TypeName typeName;
        typeName.Name = typeNameString;
        typeName.Kind = winrt::Windows::UI::Xaml::Interop::TypeKind::Metadata;

        Style style(typeName);
        element.Style(style);
    } catch (...) {}
}

enum class PlacementKind {
    TaskbarLeftEdge = 1,
    TaskbarCenterEdge = 2,
    TaskbarRightEdge = 3,
    TaskbarLeftStart = 4,
    TaskbarRightStart = 5,
    TaskbarAfterSearchLeft = 6,
    TaskbarAfterSearchRight = 7,
    TaskbarAfterTaskViewLeft = 8,
    TaskbarAfterTaskViewRight = 9,
    TaskbarAfterWidgetsLeft = 10,
    TaskbarAfterWidgetsRight = 11,
};

struct PlacementResult {
    Grid parent = nullptr;
    FrameworkElement anchor = nullptr;
    PlacementKind kind = PlacementKind::TaskbarRightStart;
    int insertColumn = kOverlayColumn;
};

static bool RunFromWindowThread(HWND hWnd, WindowThreadProc proc, void* param) {
    static const UINT kMsg = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

    struct Payload {
        WindowThreadProc proc;
        void* param;
    };

    DWORD tid = GetWindowThreadProcessId(hWnd, nullptr);
    if (!tid) return false;

    if (tid == GetCurrentThreadId()) {
        proc(param);
        return true;
    }

    HHOOK hook = SetWindowsHookExW(
        WH_CALLWNDPROC,
        [](int code, WPARAM wParam, LPARAM lParam) CALLBACK -> LRESULT {
            if (code == HC_ACTION) {
                auto* cwp = reinterpret_cast<const CWPSTRUCT*>(lParam);
                static const UINT kM = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
                if (cwp->message == kM) {
                    auto* p = reinterpret_cast<Payload*>(cwp->lParam);
                    p->proc(p->param);
                }
            }
            return CallNextHookEx(nullptr, code, wParam, lParam);
        },
        nullptr,
        tid);

    if (!hook) return false;

    Payload pay{proc, param};
    SendMessageW(hWnd, kMsg, 0, reinterpret_cast<LPARAM>(&pay));
    UnhookWindowsHookEx(hook);
    return true;
}

static FrameworkElement FindChildByName(FrameworkElement const& root, std::wstring_view name, int depth = 32) {
    if (!root || depth == 0) return nullptr;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (child.Name() == name) return child;
        if (auto found = FindChildByName(child, name, depth - 1)) return found;
    }
    return nullptr;
}

static bool ContainsMenuFlyoutPresenter(DependencyObject const& root, int depth = 16) {
    if (!root || depth == 0) return false;
    if (root.try_as<MenuFlyoutPresenter>()) return true;

    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        if (ContainsMenuFlyoutPresenter(VisualTreeHelper::GetChild(root, i), depth - 1)) {
            return true;
        }
    }
    return false;
}

static void OffsetRootMenuFlyout(MenuFlyout const& flyout) {
    if (!flyout) return;

    try {
        auto target = flyout.Target();
        auto xamlRoot = target ? target.XamlRoot() : nullptr;
        if (!xamlRoot) return;

        // This runs only for the root MenuFlyout's Opened event. Submenus use
        // their normal Popup and therefore retain Windows' original alignment.
        auto popups = VisualTreeHelper::GetOpenPopupsForXamlRoot(xamlRoot);
        for (uint32_t i = popups.Size(); i > 0; --i) {
            auto popup = popups.GetAt(i - 1);
            if (!popup || !ContainsMenuFlyoutPresenter(popup.Child())) continue;

            // Windows provides about 2 px by default; 9 px more yields the
            // requested 11 px total gap without making it feel loose.
            const double offset = TaskbarMenusOpenDownward() ? 9.0 : -9.0;
            popup.VerticalOffset(popup.VerticalOffset() + offset);
            return;
        }
    } catch (...) {}
}

static const wchar_t* const kStartButtonNames[] = {
    L"StartButton",
    L"StartMenuButton",
    L"StartMenuLaunchButton",
    L"LaunchListButton",
};

static Grid FindTaskbarRootGrid(FrameworkElement const& root) {
    FrameworkElement taskbarFrame = nullptr;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto c = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (c) {
            auto className = winrt::get_class_name(c);
            if (className == L"Taskbar.TaskbarFrame") {
                taskbarFrame = c;
                break;
            }
        }
    }

    if (!taskbarFrame) return nullptr;
    auto rootGrid = FindChildByName(taskbarFrame, L"RootGrid");
    return rootGrid ? rootGrid.try_as<Grid>() : nullptr;
}

static FrameworkElement FindElementInRepeater(FrameworkElement const& repeater, const wchar_t* const* names, int nameCount) {
    if (!repeater) return nullptr;

    int childCount = VisualTreeHelper::GetChildrenCount(repeater);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(repeater, i).try_as<FrameworkElement>();
        if (!child) continue;

        for (int j = 0; j < nameCount; j++) {
            if (child.Name() == names[j]) return child;
        }
    }

    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(repeater, i).try_as<FrameworkElement>();
        if (!child) continue;

        int subChildCount = VisualTreeHelper::GetChildrenCount(child);
        for (int k = 0; k < subChildCount; k++) {
            auto subChild = VisualTreeHelper::GetChild(child, k).try_as<FrameworkElement>();
            if (!subChild) continue;

            for (int j = 0; j < nameCount; j++) {
                if (subChild.Name() == names[j]) return subChild;
            }
        }
    }

    return nullptr;
}

static FrameworkElement FindElementByClassName(FrameworkElement const& parent, const wchar_t* className) {
    if (!parent) return nullptr;

    int childCount = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;

        if (winrt::get_class_name(child) == className) return child;
    }

    return nullptr;
}

static FrameworkElement FindNthElementByClassName(FrameworkElement const& parent, const wchar_t* className, int index) {
    if (!parent) return nullptr;

    int foundCount = 0;
    int childCount = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;

        if (winrt::get_class_name(child) == className) {
            if (foundCount == index) return child;
            foundCount++;
        }
    }

    return nullptr;
}

static FrameworkElement FindChildByClassName(FrameworkElement const& parent, const wchar_t* className, int depth = 32) {
    if (!parent || depth <= 0) return nullptr;

    int childCount = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;

        if (winrt::get_class_name(child) == className) return child;
        if (auto found = FindChildByClassName(child, className, depth - 1)) return found;
    }

    return nullptr;
}

static HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows([](HWND hWnd, LPARAM lp) CALLBACK -> BOOL {
        DWORD pid = 0;
        wchar_t cls[32] = {};
        if (GetWindowThreadProcessId(hWnd, &pid) && pid == GetCurrentProcessId() &&
            GetClassNameW(hWnd, cls, ARRAYSIZE(cls)) &&
            _wcsicmp(cls, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(lp) = hWnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    return result;
}

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;

    void* taskBand = (void*)GetWindowLongPtrW(hTaskSwWnd, 0);
    void* taskBandForTaskListWndSite = taskBand;
    for (int i = 0; *(void**)taskBandForTaskListWndSite != CTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) return nullptr;
        taskBandForTaskListWndSite = (void**)taskBandForTaskListWndSite + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForTaskListWndSite, taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) return nullptr;

    size_t taskbarElementIUnknownOffset = 0x10;
#if defined(_M_X64) || defined(__x86_64__)
    {
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
            b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F) {
            taskbarElementIUnknownOffset = b[7];
        }
    }
#elif defined(_M_ARM64) || defined(__aarch64__)
    {
        const DWORD* p = (const DWORD*)TaskbarHost_FrameHeight_Original;
        if (p[0] == 0xD503237F && (p[1] & 0xFFC07FFF) == 0xA9807BFD &&
            p[2] == 0x910003FD && (p[3] & 0xFFF00FE0) == 0xF8400C00) {
            taskbarElementIUnknownOffset = (p[3] >> 12) & 0xFF;
        }
    }
#endif

    auto* taskbarElementIUnknown = *(::IUnknown**)((BYTE*)taskbarHostSharedPtr[0] + taskbarElementIUnknownOffset);
    FrameworkElement taskbarElement{nullptr};
    taskbarElementIUnknown->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(taskbarElement));

    auto result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;
    if (taskbarHostSharedPtr[1] && Std_Ref_Decref_Original) Std_Ref_Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

static std::vector<FrameworkElement> GetMenuBarItemHosts() {
    std::vector<FrameworkElement> hosts;
    if (!g_menuBarRoot) return hosts;

    try {
        auto panel = FindChildByName(g_menuBarRoot, L"TaskbarMenuBarPanel").try_as<Panel>();
        if (!panel) return hosts;

        auto children = panel.Children();
        for (uint32_t i = 0; i < children.Size(); ++i) {
            if (auto child = children.GetAt(i).try_as<FrameworkElement>()) {
                hosts.push_back(child);
            }
        }
    } catch (...) {}

    return hosts;
}

static FrameworkElement GetMenuBarVisualButton(FrameworkElement const& host) {
    if (!host) return nullptr;
    if (host.Name() == L"TaskbarMenuBarButton") {
        return host;
    }
    return FindChildByName(host, L"TaskbarMenuBarButton").try_as<FrameworkElement>();
}

static void UpdateButtonWidths() {
    if (!g_menuBarRoot) return;
    if (g_settings.buttonWidthMode != L"static" &&
        g_settings.buttonWidthMode != L"custom") {
        return;
    }

    try {
        auto hosts = GetMenuBarItemHosts();
        if (hosts.empty()) return;

        double targetWidth = 0.0;
        if (g_settings.buttonWidthMode == L"custom") {
            targetWidth = (double)std::max(1, g_settings.customButtonWidth);
        } else {
            for (auto const& host : hosts) {
                targetWidth = std::max(targetWidth, host.ActualWidth());
            }
        }

        if (targetWidth <= 0.5) return;

        for (auto const& host : hosts) {
            double targetMinWidth = std::min(host.MinWidth(), targetWidth);
            if (std::abs(host.MinWidth() - targetMinWidth) > 0.5) {
                host.MinWidth(targetMinWidth);
            }
            double currentWidth = host.Width();
            if (std::isnan(currentWidth) || std::abs(currentWidth - targetWidth) > 0.5) {
                host.Width(targetWidth);
            }

            if (auto visualButton = GetMenuBarVisualButton(host)) {
                double visualWidth = visualButton.Width();
                if (std::isnan(visualWidth) || std::abs(visualWidth - targetWidth) > 0.5) {
                    visualButton.Width(targetWidth);
                }
            }
        }
    } catch (...) {}
}

static void UpdateButtonHitTargetHeight() {
    if (!g_menuBarRoot || !g_injectionParent) return;

    double taskbarHeight = std::max(1.0, g_injectionParent.ActualHeight());
    double visualButtonHeight = std::max(24.0, taskbarHeight - 4.0);

    try {
        g_menuBarRoot.MinHeight(taskbarHeight);
        g_menuBarRoot.Height(taskbarHeight);

        if (auto host = g_menuBarHost.try_as<FrameworkElement>()) {
            host.MinHeight(taskbarHeight);
        }

        if (auto panel = FindChildByName(g_menuBarRoot, L"TaskbarMenuBarPanel").try_as<FrameworkElement>()) {
            panel.MinHeight(taskbarHeight);
            panel.Height(taskbarHeight);
        }

        auto hosts = GetMenuBarItemHosts();
        for (auto const& host : hosts) {
            host.MinHeight(taskbarHeight);
            host.Height(taskbarHeight);
            if (auto visualButton = GetMenuBarVisualButton(host)) {
                visualButton.MinHeight(visualButtonHeight);
                visualButton.Height(visualButtonHeight);
            }
        }
    } catch (...) {}
}

static Grid BuildMenuBar() {
    const double initialTaskbarHeight =
        g_injectionParent ? std::max(0.0, g_injectionParent.ActualHeight()) : 0.0;
    const double initialVisualButtonHeight =
        initialTaskbarHeight > 1.0 ? std::max(24.0, initialTaskbarHeight - 4.0) : 0.0;

    Grid root;
    root.Name(L"TaskbarMenuBarHost");
    root.Tag(box_value(hstring(L"TaskbarMenuBarHost")));
    ApplyEmptyXamlStyle(root, L"Windows.UI.Xaml.Controls.Grid");
    root.VerticalAlignment(VerticalAlignment::Stretch);
    root.HorizontalAlignment(HorizontalAlignment::Left);
    root.IsHitTestVisible(true);
    root.Background(SolidColorBrush(Colors::Transparent()));
    root.MinWidth(1.0);
    root.MinHeight(1.0);
    if (initialTaskbarHeight > 1.0) {
        root.MinHeight(initialTaskbarHeight);
        root.Height(initialTaskbarHeight);
    }

    std::vector<MenuBarButton> items;
    {
        std::lock_guard<std::mutex> lk(g_buttonsMutex);
        items = g_buttons;
    }

    std::vector<MenuBarButton> visibleItems;
    visibleItems.reserve(items.size());
    for (const auto& item : items) {
        if (IsButtonHidden(item)) continue;
        if (item.label.empty() && item.iconRaw.empty() && item.action.empty() && item.subItems.empty()) continue;
        visibleItems.push_back(item);
    }

    if (visibleItems.empty()) return nullptr;

    auto buildButton = [&](const MenuBarButton& item, size_t index) -> FrameworkElement {
        Button hitTarget;
        hitTarget.Name(L"TaskbarMenuBarHitTarget");
        hitTarget.Tag(box_value(hstring(L"TaskbarMenuBarHitTarget")));
        hitTarget.VerticalAlignment(VerticalAlignment::Stretch);
        hitTarget.HorizontalAlignment(HorizontalAlignment::Center);
        hitTarget.HorizontalContentAlignment(HorizontalAlignment::Center);
        hitTarget.VerticalContentAlignment(VerticalAlignment::Center);
        hitTarget.MinWidth(GetButtonContentMinWidth());
        hitTarget.Padding({0, 0, 0, 0});
        if (initialTaskbarHeight > 1.0) {
            hitTarget.MinHeight(initialTaskbarHeight);
            hitTarget.Height(initialTaskbarHeight);
        }
        if (auto style = CreateTaskbarMenuBarHitTargetStyle()) {
            hitTarget.Style(style);
        } else {
            hitTarget.Background(SolidColorBrush(Colors::Transparent()));
            hitTarget.BorderThickness({0, 0, 0, 0});
            hitTarget.UseSystemFocusVisuals(false);
        }

        Grid visualButton;
        visualButton.Name(L"TaskbarMenuBarButton");
        visualButton.Tag(box_value(hstring(L"TaskbarMenuBarButton")));
        visualButton.VerticalAlignment(VerticalAlignment::Center);
        visualButton.HorizontalAlignment(HorizontalAlignment::Center);
        visualButton.MinWidth(GetButtonContentMinWidth());
        visualButton.IsHitTestVisible(false);
        if (initialVisualButtonHeight > 1.0) {
            visualButton.MinHeight(initialVisualButtonHeight);
            visualButton.Height(initialVisualButtonHeight);
        }

        auto content = MakeButtonContent(item, ResolveDisplayMode(item));
        if (content) {
            content.Margin({(double)g_settings.buttonPaddingX, (double)g_settings.buttonPaddingY,
                            (double)g_settings.buttonPaddingX, (double)g_settings.buttonPaddingY});
        }

        Border highlightBackground;
        highlightBackground.Name(L"TaskbarMenuBarButtonHighlightBackground");
        highlightBackground.Background(SolidColorBrush(Colors::Transparent()));
        highlightBackground.BorderBrush(SolidColorBrush(Colors::Transparent()));
        highlightBackground.BorderThickness({0, 0, 0, 0});
        highlightBackground.CornerRadius({4, 4, 4, 4});
        highlightBackground.HorizontalAlignment(HorizontalAlignment::Stretch);
        highlightBackground.VerticalAlignment(VerticalAlignment::Stretch);
        highlightBackground.IsHitTestVisible(false);
        visualButton.Children().Append(highlightBackground);

        Grid contentSlot;
        contentSlot.Name(L"TaskbarMenuBarButtonContentSlot");
        contentSlot.Tag(box_value(hstring(L"TaskbarMenuBarButtonContentSlot")));
        contentSlot.HorizontalAlignment(HorizontalAlignment::Stretch);
        contentSlot.VerticalAlignment(VerticalAlignment::Stretch);
        contentSlot.IsHitTestVisible(false);
        if (content) {
            contentSlot.Children().Append(content);
        }
        visualButton.Children().Append(contentSlot);
        auto animatedIcon = FindChildByName(contentSlot, L"TaskbarMenuBarButtonIcon", 8)
                                .try_as<UIElement>();

        MenuFlyout menu = nullptr;
        if (IsButtonDisabled(item)) {
            hitTarget.IsEnabled(false);
            visualButton.Opacity(0.55);
        }
        EnableTaskbarStylerButtonStates(hitTarget, highlightBackground, animatedIcon);

        if (item.isMenu) {
            menu = MenuFlyout();
            menu.Placement(Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::TopEdgeAlignedLeft);

            // Menu contents and their icons are created only when needed.
            menu.Opening([subItems = item.subItems](auto const& sender, auto const&) {
                auto flyout = sender.template try_as<MenuFlyout>();
                if (flyout && flyout.Items().Size() == 0) {
                    AppendMenuEntries(subItems, flyout.Items());
                }
            });
            menu.Opened([](auto const& sender, auto const&) {
                OffsetRootMenuFlyout(sender.template try_as<MenuFlyout>());
            });
            hitTarget.Click([menu, hitTarget](auto const&, auto const&) {
                if (!menu.IsOpen()) {
                    menu.ShowAt(hitTarget);
                }
            });
        } else {
            hitTarget.Click([action = item.action](auto const&, auto const&) {
                ExecuteAction(action);
            });
        }
        SetOptionalToolTip(hitTarget, item.tooltip, item.label);
        hitTarget.Content(visualButton);
        return hitTarget;
    };

    StackPanel panel;
    panel.Name(L"TaskbarMenuBarPanel");
    panel.Tag(box_value(hstring(L"TaskbarMenuBarPanel")));
    ApplyEmptyXamlStyle(panel, L"Windows.UI.Xaml.Controls.StackPanel");
    panel.Orientation(Orientation::Horizontal);
    panel.VerticalAlignment(VerticalAlignment::Stretch);
    panel.HorizontalAlignment(HorizontalAlignment::Left);
    panel.Margin({(double)g_settings.buttonOffsetX, (double)g_settings.buttonOffsetY, 0, 0});
    if (initialTaskbarHeight > 1.0) {
        panel.MinHeight(initialTaskbarHeight);
        panel.Height(initialTaskbarHeight);
    }

    for (size_t i = 0; i < visibleItems.size(); ++i) {
        auto btn = buildButton(visibleItems[i], i);

        if (i + 1 < visibleItems.size()) {
            btn.Margin({0, 0, (double)g_settings.buttonSpacing, 0});
        }

        panel.Children().Append(btn);
    }

    root.Children().Append(panel);
    return root;
}

static bool IsTaskbarTrackingPlacement(PlacementKind kind) {
    return kind >= PlacementKind::TaskbarLeftStart && kind <= PlacementKind::TaskbarAfterWidgetsRight;
}

static PlacementResult ResolvePlacement(FrameworkElement const& root) {
    auto rootGrid = FindTaskbarRootGrid(root);
    auto placement = g_settings.placement;

    if (!rootGrid) return {};

    if (placement == L"taskbar_left_edge") {
        return {rootGrid, rootGrid, PlacementKind::TaskbarLeftEdge, kOverlayColumn};
    }
    if (placement == L"taskbar_center_edge") {
        return {rootGrid, rootGrid, PlacementKind::TaskbarCenterEdge, kOverlayColumn};
    }
    if (placement == L"taskbar_right_edge") {
        return {rootGrid, rootGrid, PlacementKind::TaskbarRightEdge, kOverlayColumn};
    }

    auto repeater = FindChildByName(rootGrid, L"TaskbarFrameRepeater");
    if (placement == L"taskbar_left_start") {
        auto target = FindElementInRepeater(repeater, kStartButtonNames, ARRAYSIZE(kStartButtonNames));
        return {rootGrid, target, target ? PlacementKind::TaskbarLeftStart : PlacementKind::TaskbarLeftEdge, kOverlayColumn};
    }
    if (placement == L"taskbar_right_start") {
        auto target = FindElementInRepeater(repeater, kStartButtonNames, ARRAYSIZE(kStartButtonNames));
        return {rootGrid, target, target ? PlacementKind::TaskbarRightStart : PlacementKind::TaskbarLeftEdge, kOverlayColumn};
    }
    if (placement == L"taskbar_after_search_left" || placement == L"taskbar_after_search_right") {
        auto target = FindElementByClassName(repeater, L"Taskbar.TaskbarExtensionElement");
        if (!target) return {rootGrid, rootGrid, PlacementKind::TaskbarLeftEdge, kOverlayColumn};
        return {rootGrid, target,
                placement == L"taskbar_after_search_left" ? PlacementKind::TaskbarAfterSearchLeft : PlacementKind::TaskbarAfterSearchRight,
                kOverlayColumn};
    }
    if (placement == L"taskbar_after_taskview_left" || placement == L"taskbar_after_taskview_right") {
        auto target = FindNthElementByClassName(repeater, L"Taskbar.ExperienceToggleButton", 1);
        if (!target) return {rootGrid, rootGrid, PlacementKind::TaskbarLeftEdge, kOverlayColumn};
        return {rootGrid, target,
                placement == L"taskbar_after_taskview_left" ? PlacementKind::TaskbarAfterTaskViewLeft : PlacementKind::TaskbarAfterTaskViewRight,
                kOverlayColumn};
    }
    if (placement == L"taskbar_after_widgets_left" || placement == L"taskbar_after_widgets_right") {
        auto target = FindChildByName(repeater, L"AugmentedEntryPointButton");
        if (!target) target = FindChildByClassName(repeater, L"Taskbar.AugmentedEntryPointButton");
        if (!target) return {rootGrid, rootGrid, PlacementKind::TaskbarLeftEdge, kOverlayColumn};
        return {rootGrid, target,
                placement == L"taskbar_after_widgets_left" ? PlacementKind::TaskbarAfterWidgetsLeft : PlacementKind::TaskbarAfterWidgetsRight,
                kOverlayColumn};
    }

    auto startButton = FindElementInRepeater(repeater, kStartButtonNames, ARRAYSIZE(kStartButtonNames));
    return {rootGrid, startButton, startButton ? PlacementKind::TaskbarRightStart : PlacementKind::TaskbarLeftEdge, kOverlayColumn};
}

static void RemoveMenuBar() {
    try {
        if (g_layoutUpdatedAttached && g_layoutUpdatedSource) {
            try { g_layoutUpdatedSource.LayoutUpdated(g_layoutUpdatedToken); } catch (...) {}
        }
        g_layoutUpdatedAttached = false;
        g_layoutUpdatedSource = nullptr;

        if (g_trackedElement) {
            try {
                if (g_hasTrackedElementOriginalMargin) {
                    g_trackedElement.Margin(g_trackedElementOriginalMargin);
                }
            } catch (...) {}
            g_trackedElement = nullptr;
        }
        g_hasTrackedElementOriginalMargin = false;
        g_trackPosition.clear();

        if (g_injectionParent && g_menuBarHost) {
            auto children = g_injectionParent.Children();
            for (uint32_t i = 0; i < children.Size(); ++i) {
                auto child = children.GetAt(i).try_as<FrameworkElement>();
                if (child && (child == g_menuBarHost || child.Name() == L"TaskbarMenuBarHost")) {
                    children.RemoveAt(i);
                    break;
                }
            }
        }

    } catch (...) {}

    g_menuBarRoot = nullptr;
    g_menuBarHost = nullptr;
    g_injectionParent = nullptr;
    g_menuBarAnchor = nullptr;
    g_trackedElement = nullptr;
    g_hasTrackedElementOriginalMargin = false;
    g_trackPosition.clear();
    g_placementKind = 0;
    g_hasLastMenuBarPlacement = false;
    g_lastMenuBarX = -1.0;
    g_lastMenuBarY = -1.0;
}

static bool IsLeftTrackingPlacement(PlacementKind kind) {
    return kind == PlacementKind::TaskbarLeftStart ||
           kind == PlacementKind::TaskbarAfterSearchLeft ||
           kind == PlacementKind::TaskbarAfterTaskViewLeft ||
           kind == PlacementKind::TaskbarAfterWidgetsLeft;
}

static bool IsWidgetsTrackingPlacement(PlacementKind kind) {
    return kind == PlacementKind::TaskbarAfterWidgetsLeft ||
           kind == PlacementKind::TaskbarAfterWidgetsRight;
}

static double GetTrackedPlacementGap(PlacementKind kind) {
    return IsWidgetsTrackingPlacement(kind) ? 8.0 : 0.0;
}

static bool AccumulateVisibleDescendantBounds(FrameworkElement const& root,
                                              FrameworkElement const& relativeTo,
                                              int depth,
                                              double rootWidth,
                                              double& left,
                                              double& right,
                                              double& top,
                                              double& bottom,
                                              bool& found) {
    if (!root || !relativeTo || depth <= 0) return found;

    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (!child) continue;

        try {
            if (child.Visibility() == Visibility::Visible &&
                child.ActualWidth() > 1.0 &&
                child.ActualHeight() > 1.0) {
                std::wstring className(winrt::get_class_name(child).c_str());
                bool wideLayoutContainer =
                    child.ActualWidth() >= rootWidth - 2.0 &&
                    (className.find(L"Grid") != std::wstring::npos ||
                     className.find(L"Panel") != std::wstring::npos ||
                     className.find(L"Presenter") != std::wstring::npos ||
                     className.find(L"Border") != std::wstring::npos);

                if (!wideLayoutContainer) {
                    auto transform = child.TransformToVisual(relativeTo);
                    auto point = transform.TransformPoint({0, 0});
                    double childLeft = point.X;
                    double childRight = point.X + child.ActualWidth();
                    double childTop = point.Y;
                    double childBottom = point.Y + child.ActualHeight();

                    if (!found) {
                        left = childLeft;
                        right = childRight;
                        top = childTop;
                        bottom = childBottom;
                        found = true;
                    } else {
                        left = std::min(left, childLeft);
                        right = std::max(right, childRight);
                        top = std::min(top, childTop);
                        bottom = std::max(bottom, childBottom);
                    }
                }
            }
        } catch (...) {}

        AccumulateVisibleDescendantBounds(child, relativeTo, depth - 1, rootWidth,
                                          left, right, top, bottom, found);
    }

    return found;
}

static bool TryGetWidgetVisualBounds(FrameworkElement const& anchor,
                                     FrameworkElement const& relativeTo,
                                     double& left,
                                     double& right,
                                     double& top,
                                     double& height) {
    if (!anchor || !relativeTo) return false;

    try {
        double boundsLeft = 0.0;
        double boundsRight = 0.0;
        double boundsTop = 0.0;
        double boundsBottom = 0.0;
        bool found = false;
        AccumulateVisibleDescendantBounds(anchor, relativeTo, 8,
                                          std::max(1.0, anchor.ActualWidth()),
                                          boundsLeft, boundsRight,
                                          boundsTop, boundsBottom, found);
        if (!found || boundsRight <= boundsLeft || boundsBottom <= boundsTop) {
            return false;
        }

        left = boundsLeft;
        right = boundsRight;
        top = boundsTop;
        height = boundsBottom - boundsTop;
        return true;
    } catch (...) {
        return false;
    }
}

static void UpdateTrackedReservation(double barWidth) {
    if (!g_trackedElement) return;

    auto kind = static_cast<PlacementKind>(g_placementKind);
    if (!IsTaskbarTrackingPlacement(kind)) return;

    try {
        double desiredGap = std::max(0.0, barWidth + GetTrackedPlacementGap(kind));
        auto margin = g_hasTrackedElementOriginalMargin ? g_trackedElementOriginalMargin : g_trackedElement.Margin();

        if (IsLeftTrackingPlacement(kind)) {
            margin.Left += desiredGap;
        } else {
            margin.Right += desiredGap;
        }

        auto current = g_trackedElement.Margin();
        if (std::abs(current.Left - margin.Left) > 0.5 ||
            std::abs(current.Right - margin.Right) > 0.5 ||
            std::abs(current.Top - margin.Top) > 0.5 ||
            std::abs(current.Bottom - margin.Bottom) > 0.5) {
            g_trackedElement.Margin(margin);
        }
    } catch (...) {
        g_trackedElement = nullptr;
        g_hasTrackedElementOriginalMargin = false;
        g_trackPosition.clear();
    }
}

static void UpdateMenuBarPlacement() {
    if (!g_menuBarRoot || !g_injectionParent || !g_menuBarHost) return;

    try {
        auto parent = g_injectionParent;
        auto bar = g_menuBarRoot;
        auto anchor = g_menuBarAnchor;
        auto host = g_menuBarHost;
        if (!parent || !bar || !host) return;

        UpdateButtonWidths();
        UpdateButtonHitTargetHeight();
        try { bar.UpdateLayout(); } catch (...) {}

        double barW = std::max(1.0, bar.ActualWidth());
        double barH = std::max(1.0, bar.ActualHeight());
        UpdateTrackedReservation(barW);

        double parentW = std::max(1.0, parent.ActualWidth());
        double parentH = std::max(1.0, parent.ActualHeight());
        double x = 0.0;
        double y = std::max(0.0, (parentH - barH) / 2.0);
        auto kind = static_cast<PlacementKind>(g_placementKind);
        const double gap = IsTaskbarTrackingPlacement(kind) ? GetTrackedPlacementGap(kind) : 6.0;

        switch (kind) {
            case PlacementKind::TaskbarLeftEdge:
                x = 0.0;
                break;
            case PlacementKind::TaskbarCenterEdge:
                x = std::max(0.0, (parentW - barW) / 2.0);
                break;
            case PlacementKind::TaskbarRightEdge:
                x = std::max(0.0, parentW - barW);
                break;
            case PlacementKind::TaskbarLeftStart:
            case PlacementKind::TaskbarRightStart:
            case PlacementKind::TaskbarAfterSearchLeft:
            case PlacementKind::TaskbarAfterSearchRight:
            case PlacementKind::TaskbarAfterTaskViewLeft:
            case PlacementKind::TaskbarAfterTaskViewRight:
            case PlacementKind::TaskbarAfterWidgetsLeft:
            case PlacementKind::TaskbarAfterWidgetsRight:
                if (anchor) {
                    auto transform = anchor.TransformToVisual(parent);
                    auto point = transform.TransformPoint({0, 0});
                    double anchorW = std::max(1.0, anchor.ActualWidth());
                    double anchorH = std::max(1.0, anchor.ActualHeight());
                    double anchorLeft = point.X;
                    double anchorRight = point.X + anchorW;
                    double anchorTop = point.Y;
                    double anchorHeight = anchorH;
                    if (IsWidgetsTrackingPlacement(kind)) {
                        double visualLeft = 0.0;
                        double visualRight = 0.0;
                        double visualTop = 0.0;
                        double visualHeight = 0.0;
                        if (TryGetWidgetVisualBounds(anchor, parent, visualLeft, visualRight,
                                                     visualTop, visualHeight)) {
                            anchorLeft = visualLeft;
                            anchorRight = visualRight;
                            anchorTop = visualTop;
                            anchorHeight = visualHeight;
                        }
                    }
                    y = std::max(0.0, anchorTop + (anchorHeight - barH) / 2.0);
                    if (IsLeftTrackingPlacement(kind)) {
                        x = std::max(0.0, anchorLeft - barW - gap);
                    } else {
                        x = std::max(0.0, anchorRight + gap);
                    }
                }
                break;
            default:
                break;
        }

        if (!g_hasLastMenuBarPlacement ||
            std::abs(g_lastMenuBarX - x) > 0.5 ||
            std::abs(g_lastMenuBarY - y) > 0.5) {
            Canvas::SetLeft(bar, x);
            Canvas::SetTop(bar, y);
            g_lastMenuBarX = x;
            g_lastMenuBarY = y;
            g_hasLastMenuBarPlacement = true;
        }
    } catch (...) {}
}

static bool InjectMenuBar() {
    if (g_unloading) return false;
    if (g_buttons.empty()) return false;

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return false;
    g_taskbarWnd = hWnd;

    auto xamlRoot = GetTaskbarXamlRoot(hWnd);
    if (!xamlRoot) return false;

    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return false;

    auto placement = ResolvePlacement(root);
    if (!placement.parent) return false;
    auto parent = placement.parent;
    g_injectionParent = parent;
    g_menuBarAnchor = placement.anchor;
    g_trackedElement = nullptr;
    g_hasTrackedElementOriginalMargin = false;
    g_trackPosition.clear();

    Grid bar = BuildMenuBar();
    if (!bar) return false;

    FrameworkElement host = nullptr;
    auto kind = placement.kind;

    Canvas canvasHost;
    canvasHost.Name(L"TaskbarMenuBarCanvasHost");
    canvasHost.Tag(box_value(hstring(L"TaskbarMenuBarCanvasHost")));
    ApplyEmptyXamlStyle(canvasHost, L"Windows.UI.Xaml.Controls.Canvas");
    canvasHost.HorizontalAlignment(HorizontalAlignment::Stretch);
    canvasHost.VerticalAlignment(VerticalAlignment::Stretch);
    canvasHost.IsHitTestVisible(true);
    Canvas::SetZIndex(canvasHost, 1000);
    canvasHost.Children().Append(bar);

    parent.Children().Append(canvasHost);
    host = canvasHost;

    if (IsTaskbarTrackingPlacement(kind) && placement.anchor) {
        g_trackedElement = placement.anchor;
        g_trackedElementOriginalMargin = placement.anchor.Margin();
        g_hasTrackedElementOriginalMargin = true;
        g_trackPosition = IsLeftTrackingPlacement(kind) ? L"left" : L"right";
    }

    g_menuBarRoot = bar;
    g_menuBarHost = host;
    g_placementKind = (int)kind;

    if (g_layoutUpdatedAttached && g_layoutUpdatedSource) {
        try { g_layoutUpdatedSource.LayoutUpdated(g_layoutUpdatedToken); } catch (...) {}
    }
    g_layoutUpdatedSource = parent;
    g_layoutUpdatedToken = parent.LayoutUpdated([](auto const&, auto const&) {
        UpdateMenuBarPlacement();
    });
    g_layoutUpdatedAttached = true;

    try {
        parent.UpdateLayout();
    } catch (...) {}
    UpdateMenuBarPlacement();
    return true;
}

static bool TryApplySettings() {
    if (g_unloading || g_applyingSettings) return false;
    g_applyingSettings = true;
    bool ok = false;
    try {
        RemoveMenuBar();
        ok = InjectMenuBar();
    } catch (...) {
        ok = false;
    }
    g_applyingSettings = false;
    return ok;
}

static void ApplySettingsWithRetry(HWND hWnd) {
    if (g_retryScheduled.exchange(true)) return;
    std::thread([hWnd]() {
        struct ResetGuard {
            ~ResetGuard() { g_retryScheduled.store(false); }
        } guard;

        for (int i = 0; i < 20 && !g_unloading; ++i) {
            bool ok = false;
            RunFromWindowThread(hWnd, [](void* param) {
                *reinterpret_cast<bool*>(param) = TryApplySettings();
            }, &ok);
            if (ok) return;
            Sleep(100);
        }
    }).detach();
}

// Levels: taskbar items (0) -> menu items (1) -> submenu items (2).
constexpr int kMaxMenuDepth = 2;

static MenuBarButton LoadMenuBarButton(const wchar_t* prefix, int depth, bool* isEmpty) {
    WCHAR key[256];
    MenuBarButton item;

    swprintf_s(key, L"%s.label", prefix);
    item.label = GetStringSetting(key);
    swprintf_s(key, L"%s.icon", prefix);
    item.iconRaw = GetStringSetting(key);
    swprintf_s(key, L"%s.action", prefix);
    item.action = GetStringSetting(key);
    swprintf_s(key, L"%s.tooltip", prefix);
    item.tooltip = GetStringSetting(key);
    swprintf_s(key, L"%s.separatorAfter", prefix);
    item.separatorAfter = GetStringSetting(key, L"false") == L"true" ||
                          Wh_GetIntSetting(key) != 0;

    if (depth == 0) {
        swprintf_s(key, L"%s.state", prefix);
        item.state = GetStringSetting(key, L"normal");
        if (item.state != L"normal" && item.state != L"disabled" && item.state != L"hidden") {
            item.state = L"normal";
        }

        swprintf_s(key, L"%s.displayMode", prefix);
        item.displayMode = GetStringSetting(key, L"both");
    }

    if (depth < kMaxMenuDepth) {
        swprintf_s(key, L"%s.type", prefix);
        item.isMenu = GetStringSetting(key, L"button") == L"menu";

        for (int i = 0; i <= 127; ++i) {
            WCHAR subPrefix[256];
            swprintf_s(subPrefix, L"%s.subItems[%d]", prefix, i);

            bool subEmpty = false;
            auto subItem = LoadMenuBarButton(subPrefix, depth + 1, &subEmpty);
            if (subEmpty) {
                if (i >= (int)item.subItems.size() + 2) break;
                continue;
            }
            item.subItems.push_back(std::move(subItem));
        }
    }

    *isEmpty = item.label.empty() && item.iconRaw.empty() && item.action.empty() &&
               item.tooltip.empty() && item.subItems.empty();
    return item;
}

static void LoadSettings() {
    g_settings.placement = GetStringSetting(L"MenuBarSettings.placement", L"taskbar_right_start");
    g_settings.buttonContentAlignment = GetStringSetting(L"MenuBarSettings.buttonContentAlignment", L"center");
    g_settings.enableTooltips = GetIntSetting(L"MenuBarSettings.enableTooltips", 0, 1, 1) != 0;
    g_settings.buttonWidthMode = GetStringSetting(L"MenuBarSettings.buttonWidthMode", L"dynamic");
    if (g_settings.buttonWidthMode != L"dynamic" &&
        g_settings.buttonWidthMode != L"static" &&
        g_settings.buttonWidthMode != L"custom") {
        g_settings.buttonWidthMode = L"dynamic";
    }
    g_settings.customButtonWidth = GetIntSetting(L"MenuBarSettings.customButtonWidth", 1, 1024, 80);
    g_settings.buttonSpacing = GetIntSetting(L"MenuBarSettings.buttonSpacing", 0, 64, 8);
    g_settings.iconLabelSpacing = GetIntSetting(L"MenuBarSettings.iconLabelSpacing", 0, 64, 6);
    g_settings.iconSize = GetIntSetting(L"MenuBarSettings.iconSize", 8, 48, 16);
    g_settings.textSize = GetIntSetting(L"MenuBarSettings.textSize", 8, 32, 13);
    g_settings.buttonPaddingX = 8;
    g_settings.buttonPaddingY = 4;
    g_settings.buttonOffsetX = 0;
    g_settings.buttonOffsetY = 0;
    ParseIntPair(L"MenuBarSettings.buttonOffset", L"0,0", g_settings.buttonOffsetX, g_settings.buttonOffsetY);
    ParseIntPair(L"MenuBarSettings.buttonPadding", L"8,4", g_settings.buttonPaddingX, g_settings.buttonPaddingY);

    std::vector<MenuBarButton> newButtons;
    for (int i = 0; i <= 127; ++i) {
        WCHAR prefix[256];
        swprintf_s(prefix, L"ButtonsSettings.buttons[%d]", i);

        bool isEmpty = false;
        auto item = LoadMenuBarButton(prefix, 0, &isEmpty);
        if (isEmpty) {
            if (i >= (int)newButtons.size() + 2) break;
            continue;
        }

        if (item.label.empty()) item.label = L"Button";
        newButtons.push_back(std::move(item));
    }

    {
        std::lock_guard<std::mutex> lk(g_buttonsMutex);
        g_buttons = std::move(newButtons);
    }
}

static void WINAPI TrayUI_StartTaskbar_Hook(void* pThis) {
    TrayUI_StartTaskbar_Original(pThis);
    if (g_unloading) return;

    g_taskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!g_taskbarWnd) return;

    g_menuBarRoot = nullptr;
    g_injectionParent = nullptr;
    g_menuBarAnchor = nullptr;
    g_trackedElement = nullptr;
    g_hasTrackedElementOriginalMargin = false;
    g_trackPosition.clear();

    ApplySettingsWithRetry(g_taskbarWnd);
}

void WINAPI EntryPoint_Hook() {
    Wh_Log(L">");
    ExitThread(0);
}

BOOL HookTaskbarDllSymbols() {
    static const wchar_t* const kCandidates[] = { L"taskbar.dll" };
    HMODULE h = nullptr;
    for (auto* name : kCandidates) {
        h = LoadLibraryExW(name, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (h) break;
    }
    if (!h) return FALSE;

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"}, &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"}, &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"}, &TaskbarHost_FrameHeight_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"}, &Std_Ref_Decref_Original},
        {{LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"}, &TrayUI_StartTaskbar_Original, TrayUI_StartTaskbar_Hook},
    };

    if (!WindhawkUtils::HookSymbols(h, hooks, ARRAYSIZE(hooks))) {
        return FALSE;
    }

    return TRUE;
}

BOOL Wh_ModInit() {
    g_unloading = false;
    g_applyingSettings = false;
    g_retryScheduled = false;
    g_taskbarWnd = nullptr;
    g_menuBarRoot = nullptr;
    g_injectionParent = nullptr;
    g_menuBarAnchor = nullptr;
    g_menuBarHost = nullptr;
    g_trackedElement = nullptr;
    g_hasTrackedElementOriginalMargin = false;
    g_trackPosition.clear();
    g_placementKind = 0;
    g_layoutUpdatedSource = nullptr;
    g_layoutUpdatedAttached = false;

    LoadSettings();
    return HookTaskbarDllSymbols();
}

void Wh_ModAfterInit() {
    g_taskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!g_taskbarWnd) return;

    ApplySettingsWithRetry(g_taskbarWnd);
}

void Wh_ModSettingsChanged() {
    if (g_unloading) return;
    LoadSettings();

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    g_taskbarWnd = hWnd;

    ApplySettingsWithRetry(hWnd);
}

void Wh_ModUninit() {
    g_unloading = true;
    g_retryScheduled = false;
    if (g_taskbarWnd) {
        RunFromWindowThread(g_taskbarWnd, [](void*) {
            RemoveMenuBar();
        }, nullptr);
    }
}
