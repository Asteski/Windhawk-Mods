### Submission review

_Note: This review was done by Claude, and then refined manually. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

I'm not sure about 2, but there's indeed some contradiction in the readme.

---

A few things keep the mod from working as advertised, plus a stability concern. The tool-mod conversion looks good.

**1. The `toggleProtectedFiles` setting has no effect — it's hardcoded.** `LoadSettings()` (lines 202‑206) never reads the value; it just assigns `true`:

```cpp
void LoadSettings() {
    g_settings.toggleProtectedFiles = true;   // always true, ignores the UI
}
```

So the declared setting is dead — turning it off in the UI does nothing. Read it from Windhawk:

```cpp
void LoadSettings() {
    g_settings.toggleProtectedFiles = Wh_GetIntSetting(L"toggleProtectedFiles");
}
```

**2. The toggle doesn't actually take effect without restarting Explorer.** The mod writes the `Hidden`/`ShowSuperHidden` registry values directly (lines 143‑184) and then tries to refresh with `WM_SETTINGCHANGE` + an undocumented `WM_COMMAND 41504` (lines 209‑230). That doesn't make Explorer re-read the setting — which is exactly why the README has to say *"Explorer process must be restarted for changes to take effect"* (line 45), contradicting the "applied immediately" claim above it. Use the documented shell API instead, which updates the shell's in-memory state and notifies open windows so the change applies live:

```cpp
SHELLSTATE ss{};
SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS | SSF_SHOWSUPERHIDDEN, FALSE); // read
DWORD mask = SSF_SHOWALLOBJECTS;
ss.fShowAllObjects = !ss.fShowAllObjects;
if (g_settings.toggleProtectedFiles) {
    ss.fShowSuperHidden = !ss.fShowSuperHidden;
    mask |= SSF_SHOWSUPERHIDDEN;
}
SHGetSetSettings(&ss, mask, TRUE); // write + update the live shell state
SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
```

This replaces all four registry helpers and `RefreshAllExplorerWindows()`, removes the `WM_COMMAND 41504` magic number, and lets you drop the "must restart Explorer" line from the README. (`<shlobj.h>` is already included.)

**3. Don't do the toggle work inside the `WH_KEYBOARD_LL` callback.** Low-level keyboard hooks run synchronously on the hook thread and block *all* system input until they return. On Ctrl+H the callback currently does registry I/O, a `HWND_BROADCAST` `SendNotifyMessage`, and `FindWindowEx` enumeration loops (lines 255‑270) — all of which stall the input queue. Post the work to the message loop you already run on the hook thread (or a worker thread) and return `1` immediately. The established pattern is in [explorer-up-new-window.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/explorer-up-new-window.wh.cpp#L833-L836), whose keyboard hook just kicks off a worker thread and returns. For this mod, a `PostThreadMessageW(g_hookThreadId, WM_APP + 1, 0, 0)` handled in `HookThreadProc`'s loop is enough.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **Dead desktop-detection code.** `CONTEXT_DESKTOP`, the `Progman`/`WorkerW` checks and the `GetShellWindow`/`IsChild` fallback (lines 76‑80, 113‑123) are computed but never used — the hook only acts on `CONTEXT_EXPLORER` (line 255). Either handle the desktop (arguably a place you'd want this too) or drop the unused branches.
- **`GetModuleHandle(nullptr)` for the LL hook** (line 282): low-level hooks run in-process, so the module handle is ignored — passing `nullptr` is the idiomatic value, matching the reference mod above.
- **Ctrl+H detection** (line 246) fires even when other modifiers are held (Ctrl+Shift+H, Ctrl+Alt+H). Consider requiring Ctrl-only if you want to avoid clobbering combos other tools use.
- **`g_modEnabled`** is always `true` for the mod's lifetime, so the check in the hook is redundant — harmless, but removable.
- **README visual:** the effect is visible (files appearing/disappearing), so a short GIF would make the mod page clearer. Only `i.imgur.com` / `raw.githubusercontent.com` are allowed image hosts.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations about the feature itself.

- **Per-keystroke cost.** The global LL keyboard hook runs `GetForegroundWindow` + `GetClassNameW` on every keydown system-wide. It's lightweight, and it's the right trade-off here: `RegisterHotKey` would be global and would steal Ctrl+H from browsers and other apps, whereas the LL hook lets you scope the shortcut to focused Explorer windows and swallow it only there (returning `1`). Just noting the cost — no change needed.
- **Persistence on disable.** The toggle changes a persistent, user-facing Windows setting (the same value Explorer's own "Show hidden files" writes), so it doesn't auto-revert when the mod is disabled. That's inherent to what the mod does and is reasonable, but worth being aware of relative to Windhawk's usual "effects disappear when disabled" expectation.
- No overlap found with existing mods — `hide-dotfiles-explorer`, `dot-hide`, etc. address a different problem (hiding dotfiles), not toggling the hidden-files setting via a hotkey.

</p>
</details>