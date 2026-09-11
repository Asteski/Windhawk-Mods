# Separate System Tray Icons: styling targets

Version 0.2.2 adds stable names for the glyph layers. The button names are
unchanged. The normal creation path uses `SystemTray.OmniButton`; the mod logs
the actual class if it has to use a fallback on a different Windows build.

| Target | Purpose |
| --- | --- |
| `SystemTray.OmniButton#SeparateQuickSettingsXamlBluetooth` | Bluetooth button, hit target and native template |
| `SystemTray.OmniButton#SeparateQuickSettingsXamlNetwork` | Network button |
| `SystemTray.OmniButton#SeparateQuickSettingsXamlSound` | Sound button, including wheel and middle-click handling |
| `SystemTray.OmniButton#SeparateQuickSettingsXamlQuickSettings` | Optional compact replacement for the grouped button |
| `Grid#SeparateTrayIconLayers` | Centered 16-by-16 icon host inside each injected button |
| `FontIcon#SeparateTrayIconPrimary` | Main status/device glyph |
| `FontIcon#SeparateTrayIconUnderlay` | Secondary grey glyph, when applicable |
| `FontIcon#SeparateTrayIconOverlay` | Disabled Bluetooth or muted sound overlay, when applicable |
| `MenuFlyoutPresenter#SeparateTrayContextMenuPresenter` | Native XAML context menu presenter, named when it opens |

The generated native button template also contains `Border#BackgroundBorder`
on the supported taskbar build. It supplies the hover highlight; this is an
existing template element, not a new custom-drawn border. Use the visual-tree
paths logged by the mod to qualify it beneath a particular button. Template
intermediate elements can vary by Windows build; `>` requires the actual direct
parent/child structure, so do not put it directly after the button unless the
tree confirms that relationship.

Taskbar Styler examples for the named glyph elements:

```yaml
controlStyles:
  - target: FontIcon#SeparateTrayIconPrimary
    styles:
      - FontSize=16
  - target: FontIcon#SeparateTrayIconOverlay
    styles:
      - FontSize=14
```

Glyph, foreground, visibility, and button dimensions are updated by the mod.
Styles on those properties can be overwritten during status/layout refreshes.
Names expose elements for selection; they do not guarantee every external
styler observes dynamically created elements. In particular, the menu presenter
is named after creation. Battery has no injected target.
