# darktable

Tested against darktable 5.6.0 (Flatpak, `org.darktable.Darktable`).

## Why this isn't a simple `@define-color` swap

darktable ships its own separate theme-file system (Preferences > general > theme dropdown),
completely independent of the generic `~/.config/gtk-3.0/gtk.css` path. Themes live as real GTK3
CSS files under `themes/`, selected internally, not something PTM's built-in `gtk3`/`gtk4`
templates touch at all.

The base `darktable.css` defines around 106 named colors, most of them derived from a 21-stop
`grey_00`..`grey_100` scale via alias, `shade()`, or `alpha()`. Following the same pattern
darktable's own "elegant" variant themes use, this template imports the base theme, then
redefines the colors that need to change:

```css
@import url("/app/share/darktable/themes/darktable.css");
@define-color grey_15 {{ colors.on_primary.default.hex }};
```

`@define-color` chains resolve at parse time, not live. Redefining only the `grey_XX` anchors
after the import would not retroactively update `bg_color`, `fg_color`, and everything else that
already aliased `grey_XX` inside `darktable.css` itself, since those captured the old value the
moment they were defined. So this template re-executes darktable's entire ~106-line color block
in the same order as the original, with only the `grey_XX` anchors changed, everything else keeps
its original `shade()`/`alpha()`/alias formula, which then correctly recomputes off the new greys
within this file's own top-to-bottom cascade.

The grey scale uses only 8 real PTM role colors, stepped rather than smoothly interpolated.
A smooth 21-stop gradient would require inventing in-between tones with no single matching
color role. Each named color still colors a distinct flat UI region, not a continuous gradient surface,
so the step boundaries aren't visible in practice.

Deliberately left untouched: `graph_red`/`graph_green`/`graph_blue` (the RGB histogram channel
lines), `colorlabel_red`/`green`/`blue`/`yellow`/`purple` (the actual photo color-tagging
feature), and `brush_cursor`/`brush_trace` (the mask-editing overlay, drawn over arbitrary photo
content). These are functional and semantic, not chrome, recoloring them would work against the
app's own purpose rather than just changing how it looks.

## The `@import` path must be absolute

Confirmed against [darktable-org/darktable#10914](https://github.com/darktable-org/darktable/issues/10914):
a theme file placed in the user themes directory cannot resolve a relative `darktable.css`
import, it only searches within its own directory, never the system themes directory where the
real `darktable.css` lives. Some community themes work around this with a `themes` symlink
placed next to the custom file; using the real absolute path directly is simpler and does not
depend on a manual symlink surviving reinstalls. This is why native and Flatpak need separate
entries: the absolute system-themes path differs (`/usr/share/darktable/themes/` vs
`/app/share/darktable/themes/`).

## A CSS comment containing a literal `/*` breaks everything after it

Not specific to darktable, a general GTK CSS parser behavior, but easy to trip on by accident in
a long descriptive header comment. `/* darktable ships themes/*.css files */` reads fine as
prose, but the literal `/*` inside `themes/*.css` opens a second, unterminated comment block
inside the first one, and GTK's parser rejects the whole file rather than the one line. Confirmed
live via darktable's own debug log (`-d all`): `dt_gui_load_theme: error parsing combined CSS
... '/*' in comment block`.

## Preferences don't apply on process restart alone

Editing `darktablerc`'s `ui_last/theme` key directly and relaunching does not trigger a reload,
that key only reflects the last value shown in the Preferences dropdown, it isn't read-and-applied
on startup. Selecting the theme in Preferences > general and clicking "save CSS and apply" while
the app is running is what actually invokes the CSS loader.

## Native install

`themes/ptm.css` lives at `~/.config/darktable/themes/ptm.css`.

## Flatpak install (`org.darktable.Darktable`)

Same relative layout, inside the sandbox's own isolated config:
`~/.var/app/org.darktable.Darktable/config/darktable/themes/ptm.css`. No `flatpak override`
needed.

## Status

Verified live against darktable 5.6.0 (Flatpak) through PTM's own real render pipeline
(`community_ids` + local `community-templates/darktable/`, not a manually placed test copy), no
CSS parse errors, confirmed via darktable's own debug log. Screenshot above is from a live themed
session on this machine, not a synthetic test palette.
