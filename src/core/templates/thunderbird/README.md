# Thunderbird

Keeps [Thunderbird](https://www.thunderbird.net/) in sync with the PTM palette.

![Thunderbird with the PTM theme, dark palette](screenshot.png)

![Thunderbird with the PTM theme, light palette](light-screenshot.png)

## What it themes

PTM renders its palette into Thunderbird's chrome through `userChrome.css`.
The template overrides Thunderbird's layout variables, which cover the toolbars,
folder pane, message list, message header and cards in one place:

- backgrounds, text and separators (`--layout-background-*`, `--layout-color-*`,
  `--layout-border-*`)
- selected folder and message rows (`--selected-item-*`)
- the accent color (`--color-accent-primary`)

It also trims the chrome for tiling setups: the classic menu bar is hidden and
the tab bar, unified toolbar and list rows are made more compact.

On top of the palette it applies Material 3 shape and surfaces: pill buttons and
inputs, 12dp cards and panels, filled tonal buttons (`secondary_container`) and
message cards one tonal step above the page. The Spaces rail gets a circular
active indicator behind the icon, the tab strip softer corners, and the search
field a filled container.

Separators follow the same rule: the structural hairlines between panes, the tab
bar underline and the toolbar separators go transparent, so regions are divided
by tonal surface steps instead. Card outlines keep a subtle `outline_variant`
border.

The chrome also leans on Firefox's Nova redesign: tabs are floating pills with
a soft gradient on the active tab, and the three panes become rounded cards on a
dimmer frame, so content is inset rather than edge to edge.

Message bodies and the compose window are not themed. They render in their own
documents outside the reach of `userChrome.css`, and PTM only ships a
`userChrome` template, not a `userContent` one.

## Setup

1. Enable **Thunderbird** in PTM under *Settings -> Templates* (community
   templates), or from `config.toml`:

   ```toml
   [theme.templates]
   community_ids = ["thunderbird"]
   ```

2. Apply a theme (change the wallpaper/palette, or re-apply the current one).
   `apply.sh` writes the rendered file and wires it into every profile.

3. Restart Thunderbird once. `userChrome.css` is only read on startup.

## How the wiring works

`apply.sh` looks for `prefs.js` in the profiles under `~/.thunderbird`, the
Flatpak path and the Snap path and under `'XDG_CONFIG_HOME'/thunderbird`. 
It follows symlinks, so profiles managed as symlinks are found too. 
In each profile it:

- prepends `@import "<cache>/ptm.css";` to `chrome/userChrome.css`
  (creating the file if needed),
- ensures
  `user_pref("toolkit.legacyUserProfileCustomizations.stylesheets", true);`
  in `user.js`, correcting a stale `false`.

Both steps are idempotent: re-applying never duplicates a line. Anything that
cannot be written, for example profiles managed by Home Manager or another Nix
module (read-only symlinks into the store), is reported on stderr and skipped;
other profiles are still wired. In that case set the preference and the
`@import` from your system configuration instead.

If no Thunderbird directory exists at all the hook exits silently, so it is a
no-op on machines without Thunderbird. If a Thunderbird directory exists but no
profile with `prefs.js` is found, the hook exits non-zero with a message on
stderr.

## Uninstall

Remove the import line from `chrome/userChrome.css`, the
`toolkit.legacyUserProfileCustomizations.stylesheets` line from `user.js`, and
disable the template in PTM.

## Switching the palette while Thunderbird is running

`userChrome.css` is a startup stylesheet: Thunderbird loads it, and the
`@import` it points at, when a window is created and does not re-read it
cleanly afterwards. Changing the palette while Thunderbird is running can
therefore leave the chrome half-restyled until it is restarted.

PTM also renders the imported file in place while Thunderbird may be
reading it, and that write is not atomic, so a running instance can catch a
truncated file. Restart Thunderbird after a theme change.
