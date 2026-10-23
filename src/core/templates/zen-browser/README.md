# Zen Browser

Applies PTM colors to Zen's browser interface and internal pages through `userChrome.css` and `userContent.css`. Both generated stylesheets contain dark and light palettes selected by `prefers-color-scheme`; this is not a general website theme.

## Setup

1. Install and launch Zen at least once to create a profile.
2. Back up each profile's `chrome/userChrome.css`, `chrome/userContent.css` and `user.js` if they exist.
3. Enable **Zen Browser** in **PTM Settings -> Templates**.
4. In Zen, open **Settings → Appearance** and set **Website Appearance** to **Auto**, so internal pages follow the PTM dark/light mode.
5. Restart Zen. It does not live-reload, so a restart may also be needed after color changes.

The hook only discovers profiles with a `prefs.js` file exactly one profile-directory level below either `$XDG_CONFIG_HOME/zen` (normally `~/.config/zen`) or `~/.zen`. It does nothing when those roots are absent. Other layouts, including sandbox-specific profile locations, are not searched; they need manual imports and preference setup or an adapted hook.

## Generated files

- `$XDG_CACHE_HOME/ptm/zen-browser/zen-userChrome.css`
- `$XDG_CACHE_HOME/ptm/zen-browser/zen-userContent.css`

The hook uses `~/.cache` when `XDG_CACHE_HOME` is unset. Profile CSS imports these files by absolute path, so keep the generated cache files available.

## Profile changes

For every discovered profile, the Bash hook creates the `chrome` directory and these files as necessary:

- `chrome/userChrome.css`: prepends an `@import` of the generated chrome stylesheet.
- `chrome/userContent.css`: prepends an `@import` of the generated content stylesheet.
- `user.js`: sets `devtools.chrome.enabled` and `toolkit.legacyUserProfileCustomizations.stylesheets` to `true`.

Existing lines referencing the corresponding generated stylesheet are removed before its import is prepended. Existing lines mentioning either preference are removed from `user.js` before the enabled values are appended. Other content is retained. The hook writes through existing symlinks and creates no backups; review these changes if you already maintain custom CSS or preferences.

Both template entries run this hook synchronously to prevent concurrent profile edits. It requires Bash and standard utilities including `find`, `sed`, `mktemp` and `cmp`. It does not restart Zen or send a live-reload command.

The shipped content stylesheet marks its `about:logins` and `about:protections` sections as no longer working; coverage of those pages is not guaranteed.

## Troubleshooting

- **Some colors look wrong:** Zen mods can override the template colors. Disable all mods under **Settings → Zen Mods** and restart Zen to check. For example, [Better Tab Indicators](https://zen-browser.app/mods/664c54f9-d97d-410b-a479-23dd8a08a628/) can make the selected-tab highlight barely visible; disable it or set a custom color in its settings (see [#51](https://github.com/ptm-dev/community-templates/issues/51)).
- **Text is unreadable in dark or light mode:** make the system GTK color scheme (`prefer-dark` or `prefer-light`) match the PTM mode, for example by also enabling PTM's built-in GTK template.

## Removing the theme

Disabling the template does not undo the profile changes. For each profile, remove these lines from `chrome/userChrome.css` and `chrome/userContent.css` respectively, with your own cache path:

```css
@import "/home/you/.cache/ptm/zen-browser/zen-userChrome.css";
@import "/home/you/.cache/ptm/zen-browser/zen-userContent.css";
```

Remove these preferences from `user.js` if nothing else needs them, then restart Zen:

```js
user_pref("devtools.chrome.enabled", true);
user_pref("toolkit.legacyUserProfileCustomizations.stylesheets", true);
```
