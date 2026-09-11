### Submission review

_Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

The mod works by polling the Start menu's XAML tree on a timer and force-collapsing / rebinding elements. The core idea is fine, but a few things affect stability and users of the mod:

**1. `Wh_ModSettingsChanged` force-terminates the whole process (`ExitProcess(0)`).** Every settings tweak kills `StartMenuExperienceHost.exe` (and tears down every other Windhawk mod loaded into it), then relies on Windows to restart it. The `LoadSettings()` call right before `ExitProcess(0)` is dead code — the process dies before it's used. The mod already has full apply (`InstallXamlTraversal`) and restore (`UninstallXamlTraversal`) machinery, so it should re-apply live instead of killing the process, e.g. marshal to the UI thread and do `UninstallXamlTraversal(); LoadSettings(); InstallXamlTraversal();`. `windows-11-start-menu-styler.wh.cpp` handles the same class of tree mutation cleanly in `Wh_ModSettingsChanged` by reverting/re-applying rather than exiting.

```cpp
void Wh_ModSettingsChanged() {
    HWND coreWnd = GetCoreWnd();
    if (coreWnd) {
        RunFromWindowThread(coreWnd, [](PVOID) {
            UninstallXamlTraversal();
            LoadSettings();
            InstallXamlTraversal();
        }, nullptr);
    }
}
```

**2. Continuous 250 ms full visual-tree polling.** `g_xamlTraversalTimer` walks the entire visual tree (plus open popups) every 250 ms forever, calling `winrt::get_class_name` / name comparisons on every `FrameworkElement`, even while the Start menu is closed — a persistent CPU cost in a shell-adjacent process. It also produces a visible flash of the un-modified menu (headers, grouping, gaps) on each open, because collapsing lags the menu appearing by up to 250 ms.

Would it be possible to be more event-driven? e.g. process the XAML tree on window open, perhaps after mouse click, etc.

**3. `hideCategoryViewOption` matches the localized text "Category".** `IsCategoryText` compares the flyout item text against `L"Category"`, so on any non-English Windows UI language the menu item is never found and stays visible (the `SemanticZoom` disabling still works, so the behavior is partial/inconsistent). User-facing text is localized — match by element name / type rather than the visible string, or otherwise handle localization.

**4. Process-shutdown crash potential from global winrt objects with non-trivial destructors.** `g_xamlTraversalTimer`, `g_scrollBarHideTimer`, `g_scrollBarFadeTimer` (all `wux::DispatcherTimer`) and the global `std::vector`s that hold winrt refs (`g_flattenedSources`, `g_keepCollapsedElements`, …) run their destructors at `DLL_PROCESS_DETACH`. On a normal Start-menu restart (sign-out, update, crash) — and on the `ExitProcess(0)` path above, which you hit on *every* settings change — `Wh_ModUninit` is **not** called, so the CRT releases these XAML/COM objects during process shutdown from an arbitrary thread, which can crash the host. Prefer holding this state behind a heap pointer that's intentionally leaked at shutdown (trivial destructor) and keep the real teardown in `Wh_ModUninit`. See the note in the Windhawk docs about globals with non-trivial destructors at `DLL_PROCESS_DETACH`.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **Use `WindhawkUtils::StringSetting`** (RAII) instead of the raw `Wh_GetStringSetting` + `Wh_FreeStringSetting` pairs in `LoadSettings` (`headerText`, `scrollBarMode`).
- **`CollapseElement` is heavier than it needs to be.** Setting `Visibility(Collapsed)` removes an element from layout; additionally zeroing `Width`/`Height`/`MinWidth`/`MaxWidth`/`Margin`/alignments/`Opacity`/`IsHitTestVisible` **and** registering three `RegisterPropertyChangedCallback`s per element to keep re-forcing them is a lot of machinery to maintain and revert. If `Collapsed` alone is insufficient for a specific container (e.g. a sticky `GridViewHeaderItem`), scope the extra work to that case rather than applying it to everything.
- **`headerText` has no visible effect under the defaults**: `hideTopLevelHeader` defaults to `true`, which collapses `AllListHeadingText`, so the header text you set is applied to an element that's hidden. Consider only applying/showing the setting when the header is visible.
- `g_xamlTraversalInstalled = false;` at the end of `Wh_ModUninit` is redundant — `UninstallXamlTraversal` already resets it.
- `$name: Change Header Text` uses title case; the rest of the settings use sentence case (e.g. "Hide top header").

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations about the feature behavior itself.

- **The scroll-bar "show while scrolling" fade is a hand-rolled reimplementation** (a 200 ms hide timer + a 50 ms timer decrementing opacity by 0.2) of behavior XAML's `ScrollViewer` already provides via its auto-hide scrollbars. If the native `Auto` visibility gives an acceptable result, it would be simpler and avoid the extra timers.

</p>
</details>