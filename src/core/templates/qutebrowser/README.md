# qutebrowser

Matugen template that themes qutebrowser (completion menu, hints, tabs, statusbar, prompts, messages, downloads bar) to match your current PTM color scheme.

> Note: Some palettes might not work well or as intended. Please change the color scheme or try out a different template then. Else you can go to `~/.config/qutebrowser/ptm/template.py` and tweak the colors yourself.

## Setup

**If you're using PTM's community-templates catalog:** just enable the `qutebrowser` template from the catalog — the repo (including `template.py` and `reload.sh`) is cloned to `~/.local/state/ptm/community-templates/` automatically, so `templates.toml` and the hook path already resolve correctly. Skip to step 3 once you're confirmed :)

**Manual / standalone setup:**

1. Copy `template.py` and `reload.sh` into your qutebrowser config folder:
```bash
   mkdir -p ~/.config/qutebrowser/ptm
   cp template.py ~/.config/qutebrowser/ptm/template.py
   cp reload.sh ~/.config/qutebrowser/ptm/reload.sh
```

2. Register it with matugen by adding this to your PTM user templates config (`~/.config/ptm/templates.toml`):
```toml
   [theme.templates.user.qutebrowser]
   input_path  = "~/.config/qutebrowser/ptm/template.py"
   output_path = "~/.config/qutebrowser/ptm/colors.py"
   post_hook   = "sh ~/.config/qutebrowser/ptm/reload.sh"
```
   (Point `post_hook` at wherever you actually placed `reload.sh`. If installing via the catalog, resolve the path dynamically instead of hardcoding it, since the clone location has moved between PTM versions:
   `post_hook = "sh \"${XDG_STATE_HOME:-$HOME/.local/state}/ptm/community-templates/qutebrowser/reload.sh\""`)

3. Tell qutebrowser to load the generated file by adding this line near the top of your `~/.config/qutebrowser/config.py`:
```python
   config.source('ptm/colors.py')
```

4. Trigger a theme regeneration (change wallpaper, or re-select the current one) so matugen renders `template.py` into `colors.py` for the first time.

That's it — no manual reload needed after the first run. The `post_hook` first checks whether qutebrowser is already running (`pgrep -f qutebrowser`, which matches the full command line rather than just the short process name — some installs launch qutebrowser through a wrapper, which `pgrep -x` can miss); only if it finds a running instance does it send `:config-source` over qutebrowser's IPC socket to reload the new colors. If qutebrowser isn't open, the hook does nothing and won't launch a new window — so theme changes never interrupt you with an unwanted qutebrowser window popping up. Thanks to @VuiMuich for pointing out this issue :)

Your color scheme should now update automatically every time PTM's theme regenerates, but only while qutebrowser is actually open.

> **Tip:** If you use light/dark mode switching too (which also fires a reload script on qutebrowser), the same IPC command can end up stealing window focus depending on your window manager/compositor. The fix, per a qutebrowser maintainer in [discussion #6212](https://github.com/qutebrowser/qutebrowser/discussions/6212), is to set `new_instance_open_target` to `tab-silent` in your `config.py`, so sending IPC commands to a running instance doesn't raise/focus the window:
> ```python
> c.new_instance_open_target = 'tab-silent'
> ```
> Note this is still somewhat WM/compositor-dependent — it's the known workaround, not a guaranteed fix on every setup. Thanks to @VuiMuich for flagging this one too :)

### `reload.sh`

```sh
#!/bin/sh
COLORS_FILE="${XDG_CONFIG_HOME:-$HOME/.config}/qutebrowser/ptm/colors.py"

# Skip if colors.py hasn't changed since the last time this script ran
[ "$COLORS_FILE" -nt "$0" ] || exit 0
touch "$0"

pgrep -f qutebrowser >/dev/null && qutebrowser :config-source
```

Make sure it's executable after copying or editing:
```bash
chmod +x ~/.config/qutebrowser/ptm/reload.sh
```

## Why a helper function is in the template

A few qutebrowser settings (hint label backgrounds, tab bar background, etc.) need an `rgba()` string for transparency, not a bare hex value. matugen only outputs hex, so a small conversion helper is defined directly inside the template:

```python
def hex_to_rgba(hex_color, alpha):
    """Convert a #rrggbb hex string to a qutebrowser-compatible rgba() string."""
    hex_color = hex_color.lstrip('#')
    r = int(hex_color[0:2], 16)
    g = int(hex_color[2:4], 16)
    b = int(hex_color[4:6], 16)
    return 'rgba({}, {}, {}, {})'.format(r, g, b, alpha)
```

This has to live in `template.py` itself, not in the generated `colors.py`. Since `colors.py` is fully overwritten on every theme regeneration, adding the function only to the generated file means it disappears the next time the theme changes — and qutebrowser will throw `NameError: name 'hex_to_rgba' is not defined` on the next launch, since a later line in the file calls it. Keeping it in the template ensures it survives every regeneration.

## Notes

- Requires qutebrowser's `config.py` to exist and call `config.load_autoconfig(True)` (the default) so both the generated colors and your own settings load together.
- Tested with qutebrowser v3.7.0 / QtWebEngine 6.11.

> **Tip:** `:config-source` re-runs your entire `config.py` from top to bottom, not just the colors. If `config.load_autoconfig(True)` is called near the *top* of your `config.py`, any settings you toggle at runtime (`:config-cycle`, `:set`, etc. — saved to `autoconfig.yml`) will get overwritten by whatever hardcoded values come after it in the file, resetting things like dark mode or tab position back to your defaults every time the theme regenerates. Move `config.load_autoconfig(True)` to the **bottom** of `config.py`, after all your other settings, so your current session state loads last and wins.

> Also if you're using Niri, you might have to put this in your `rules.kdl` as well... (DISCLAIMER: won't work in `niri 26.04` version, to check your Niri version use `niri --version`)
```kdl
// --- QuteBrowser ---
window-rule {
match app-id="org.qutebrowser.qutebrowser"
focus-on-xdg-activate false
}
```
