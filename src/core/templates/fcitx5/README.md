# Fcitx5

[Fcitx5](https://github.com/fcitx/fcitx5) is a lightweight input method framework. This template
renders a classic UI theme that follows the PTM palette for the candidate list and context
menu.

Tray icon colors stay in `classicui.conf` and are not part of the theme file — that matches how
Fcitx5 separates global UI settings from per-theme colors.

Tested against Fcitx5 5.1.x.

## Apply hook (`apply.sh`)

PTM runs `apply.sh` after every theme render. It does not touch the rendered `theme.conf`;
that file is written by the template engine. The hook only activates the theme and asks Fcitx5 to
pick up the new colors.

### 1. Select the theme

Target file: `$XDG_CONFIG_HOME/fcitx5/conf/classicui.conf` (created if the directory is missing).

| Situation | Behaviour |
| --- | --- |
| File does not exist | Write `Theme=ptm` as the only line |
| `Theme=` line already set to `ptm` | No change |
| `Theme=` line set to something else | Replace that line with `Theme=ptm` |
| File exists but has no `Theme=` line | Append `Theme=ptm` on a new line |

Other keys in `classicui.conf` (fonts, tray colors, vertical candidate list, etc.) are left
untouched.

### 2. Hot-reload the UI

If Fcitx5 is running, reload only the `classicui` addon over the session D-Bus:

```bash
busctl --user call org.fcitx.Fcitx5 /controller org.fcitx.Fcitx.Controller1 ReloadAddonConfig s classicui
```

This is Fcitx5’s own controller API — not KDE-specific — and works on any desktop where Fcitx5
runs. Reloading the addon updates candidate-list and menu colors without restarting the IME, so
in-progress preedit is not cleared.

If Fcitx5 is not running, or D-Bus is unavailable, the call fails silently. The theme file and
`classicui.conf` are still updated; colors apply the next time Fcitx5 starts.

### Idempotency

Running the hook twice with the same theme does not append duplicate `Theme=` lines or rewrite
`classicui.conf` when it already points at `ptm`. The D-Bus reload is safe to repeat.
