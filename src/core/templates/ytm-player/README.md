# ytm-player

Themes [ytm-player](https://pypi.org/project/ytm-player/), a Textual-based terminal YouTube Music
client. Install with `pip install ytm-player` or `pipx install ytm-player`.

`ytm-player` reads `~/.config/ytm-player/theme.toml` independently of its own `config.toml`
(source: `ytm_player/ui/theme.py`'s `ThemeColors._apply_toml_overrides`), so this template writes
straight to that file. No merge into the user's own config is needed, and no `apply.sh` either.

**Upstream fix submitted, not yet merged: cursor and selection highlights previously did not
follow `theme.toml`.** Every field this template writes (`primary`, `selected_item`, and so on) is
applied correctly, and widgets that read `ThemeColors` directly (lyrics, progress bar) have always
themed correctly. What did not follow the theme was the DataTable cursor (track list) and other
Textual built-in cursor highlights, which stayed on ytm-player's hardcoded default red regardless
of `theme.toml`. Root cause, confirmed live: those highlights are Textual CSS variables
(`$block-cursor-background`, `$block-cursor-blurred-background`) that Textual's `ColorSystem`
derives once from the app's registered `Theme.primary` (`YTM_DARK.primary = "#ff0000"` in
`ytm_player/ui/theme.py`) at theme-registration time. `get_css_variables()` in `_app.py` patched
the exposed variables dict with `theme.toml` overrides afterward, but only for the fields it
explicitly remapped, and never rebuilt Textual's derived token set from the new `primary`, so any
widget CSS referencing those pre-derived tokens stayed on the original red. The same investigation
also found the album-art placeholder's note glyph going near-invisible on light or pastel primary
colors, for a related reason.

Filed as [peternaame-boop/ytm-player#122](https://github.com/peternaame-boop/ytm-player/issues/122),
fixed in [peternaame-boop/ytm-player#123](https://github.com/peternaame-boop/ytm-player/pull/123)
(re-registers the active `Theme` with the resolved colors instead of only patching the exposed
variables dict, plus a luminance-based contrast fix for the placeholder glyph). Once that PR
merges and ships in a release, every color this template writes, including cursor and selection
highlights, will be fully theme-reactive with no remaining gaps.

Tested against ytm-player 2.0.0.
