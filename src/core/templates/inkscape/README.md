# Inkscape

Inkscape ships its own personal-override CSS hook, `ui/user.css`, empty by default and imported
automatically ("This is a CSS override that users can provide to override Inkscape's and theme
styles"). No extra setup needed once this file is in place.

## Why this isn't a simple `@define-color` swap like GIMP

Inkscape's default active theme, `Minwaita-Inkscape` (its own bundled GTK3 theme fork, not the
system's real Adwaita), hardcodes literal hex colors throughout instead of routing through shared
`@define-color` roles: confirmed 763 literal hex values vs only 35 named-color uses across its
~6800-line `gtk-3.0/gtk.css`/`gtk-dark.css`. Redefining GTK's standard `theme_*` names alone (the
approach that fully worked for GIMP) only covers the narrow parts of the UI that actually reference
those names; the bulk of the visible chrome does not.

`user.css` does load at a higher GTK style-provider priority than the active theme (confirmed
against Inkscape's own `src/ui/themes.cpp` source), so it reliably wins wherever it directly
targets a selector. The fix is identifying the *right* selectors, not fighting priority.

## Selectors used, and how each was confirmed (not guessed)

Every selector below was found by cross-referencing Inkscape's real source
(gitlab.com/inkscape/inkscape) for exact widget names/IDs, then confirmed live with a targeted
solid-color CSS probe and a screenshot, never assumed from the theme file alone.

- `menubar`, `scrolledwindow`, `.background`: root window chrome.
- `notebook header` / `notebook header tabs tab:checked`: docked panel (Layers, Fill & Stroke,
  etc.) tab strip. Checked state uses a bottom `box-shadow` accent bar on a dark background,
  matching the ptm-dev/community-templates#58 GIMP template's pattern, not a full fill.
- `#NotebookPage`: docked panel body content.
- `button.square-button` (+ `:hover`, `:checked`, `:checked image`): the vertical toolbox's tool
  icons. Confirmed via `share/ui/toolbar-tool.ui`: each tool is a `GtkRadioButton.square-button`
  inside a `GtkFlowBox` inside a `GtkScrolledWindow`. This one **does** use a full accent fill with
  a dark icon/text override (kept deliberately, matches GIMP's own toolbox treatment) rather than
  the box-shadow style used elsewhere.
- `#SelectToolbar`, `#NodeToolbar`, ... (one ID per tool): the horizontal tool-options bar.
  Confirmed via `src/ui/toolbar/toolbars.cpp`: each tool's toolbar widget gets
  `set_name(toolName + "Toolbar")`. Every tool from `toolbars.cpp`'s own registry is listed.
- `entry` / `entry:focus`: number/text entry fields (X, Y, W, H, etc.).
- `#DesktopStatusBar`: the bottom status bar.
- `#\33 DBoxToolbar`: the 3D Box tool's options bar. CSS identifiers can't start with a digit,
  so this needs the CSS escape for a leading "3" (`\33` is the escaped hex code point), confirmed
  via `toolbars.cpp`'s own tool-name registry (`"3DBox"` + `"Toolbar"` suffix).
- `.view` / `.view:selected`: the Preferences dialog's category sidebar, a `GtkTreeView`, not a
  `GtkListBox` (same widget type as the GIMP template's own Preferences sidebar). Base background
  and the selected-row left-edge accent bar both needed separate rules.
- `menu menuitem:hover` / `menuitem:selected`: right-click context menus and menubar dropdowns.
  Needs the `menu` ancestor in the selector to match Minwaita-Inkscape's own specificity; a bare
  `menuitem:hover` silently loses that fight.
- `button.flat:checked` (+ `label` variants): Preferences > Color Selector's picker toggle
  buttons. A real contrast bug (near-white text on a yellow `:checked` background), same class of
  issue as the GIMP toolbox and the separate `pywalfox#171` sidebar-highlight fix.
- `scale trough` / `scale fill` (plain, not just `:backdrop`): the actual widget `InkScale::on_draw`
  delegates its background painting to (`src/ui/widget/ink-spinscale.cpp`), used by the Blur/Opacity
  sliders. The plain (focused-window) state was the real remaining gap for a long stretch of this
  work, since every screenshot taken to verify it was itself in `:backdrop` state.
- `notebook > stack:not(:only-child)`: the docked-panel notebook's own content-stack background,
  which shows through as a visible gray strip between a dock panel and the command toolbar when
  more than one stack child exists. Confirmed by exact pixel-color match against the theme's own
  values, after two wrong guesses (`paned > separator`, `scrollbar.left/.right` border).
- `:backdrop` variants throughout: the window is unfocused far more often in daily use than
  expected (any dialog taking focus, alt-tabbing), and Minwaita-Inkscape defines a fully separate
  stock-colored rule set for that state that the plain (focused) selectors above don't cover.

## Known gap

Inkscape's canvas "desk" (pasteboard) color is a real user preference stored as an XML attribute
in `preferences.xml`, not reachable via `user.css` at all, since the canvas is a custom-drawn
widget, not GTK chrome. A `post_hook`-based fix (patching the XML directly, same architecture as
the Blender template's own `post_hook`) was built and verified working, then intentionally left
out of this PR as out of scope for this template's first pass. Revisit separately if wanted later.

## Requires Inkscape's own "Minwaita-Inkscape" base theme

Preferences > Interface > Theming > "Change GTK theme" must stay on the default
`Minwaita-Inkscape` (do **not** pick "Adwaita" from that dropdown: it does not correspond to a
real loadable theme folder for Inkscape and silently falls back to GTK's compiled-in default,
which ignores most of this override; confirmed by direct testing, this looked like "nothing themed
at all" until root-caused).

## Native install

`user.css` lives at `~/.config/inkscape/ui/user.css`.

## Flatpak install (`org.inkscape.Inkscape`)

Same file, inside the sandbox's own isolated config:
`~/.var/app/org.inkscape.Inkscape/config/inkscape/ui/user.css`. No `flatpak override` needed.

## Status

Verified live against Inkscape 1.4.4 (Flatpak), no CSS parse errors: menu bar, toolbox, tool-options
bar, docked panel tabs and body, status bar, Preferences dialog (sidebar navigation and Color
Selector), right-click context menus and menubar dropdowns, scrollbars, the docked-panel/toolbar
boundary, and the systemic `:backdrop` (unfocused-window) state across all of the above. Screenshots
above are from a live themed session on this machine, not a synthetic test palette.
