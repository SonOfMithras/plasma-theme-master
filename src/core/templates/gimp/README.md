# GIMP

GIMP has its own personal-override CSS hook, separate from the auto-generated `theme.css` that
ships with the app (which is regenerated on every theme change and explicitly says not to edit
it by hand). This template writes to that personal file directly: `gimp.css`, imported
automatically if present, no extra setup needed once the template is enabled.

## What gets themed, and the two things that needed extra care

GIMP's own theme only defines 17 named colors (`fg-color`, `bg-color`, `selected-color`, and so
on), reused across `common.css`/`common-dark.css`. Most of the UI follows from setting those.
Two things needed more than a plain 1:1 color swap:

- **Context menus and other `GtkPopoverMenu` content had no background rule at all** in GIMP's
  own theme (only narrow button/list-row styling), so they fell through to GTK's stock light
  default. Fixed with explicit `popover`/`popover contents` rules.
- **`selected-color`/`extreme-selected-color` are always paired with white text** in GIMP's own
  CSS, with no dark-text variant the way PTM's other templates use `on_primary` on top of a
  `primary`-colored background. Mapping them straight to PTM's yellow accent made text
  unreadable everywhere that yellow shows up (menu hovers, selected list rows, the Preferences
  sidebar). Fixed by keeping the yellow and adding an explicit dark-text override for every
  selector GIMP's `common.css` actually uses those two variables as a background for (checked via
  grep, not guessed).
- A few widgets, confirmed just the Preferences dialog's own page title bar, don't use GIMP's
  custom variables at all. GIMP's own source comment on that exact element admits it "doesn't
  have any more specific way to style it," meaning it falls back to standard GTK/Adwaita
  variable names (`@theme_bg_color`, `@theme_selected_bg_color`, etc.) instead. Those are set
  too, as a safety net, since the system `gtk.css` those would normally come from never loads
  inside a Flatpak sandbox.

## Requires GIMP's own "Default" base theme

GIMP has two selectable base themes (Preferences > Interface > Theme): `Default` and `System`.
This template is built against `Default` (`themes/Default/gimp-dark.css`), the one this personal
override's own 17 variables and extra selectors are checked against. `System` imports a
different, much smaller CSS file with no guarantee it uses the same variable names, not tested
or supported here. If colors look wrong after enabling this template, check Preferences first.

## Native install

`gimp.css` lives at `~/.config/GIMP/3.2/gimp.css`.

## Flatpak install (`org.gimp.GIMP`)

Same file, inside the sandbox's own isolated config: `~/.var/app/org.gimp.GIMP/config/GIMP/3.2/
gimp.css`. No `flatpak override` needed, GIMP reads its own config from inside its sandbox
regardless of any host `~/.config` access.

## Testing

Tested against GIMP 3.2.4 (Flatpak). Verified visually across three passes: main window and
toolbox (`screenshot.png`), right-click context menus, and the Preferences dialog specifically
(`screenshot-preferences.png`), the hardest case since it touches nearly every one of GIMP's 17
variables plus the GTK fallback ones.
