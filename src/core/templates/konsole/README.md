# konsole

[PTM](https://github.com/ptm-dev/ptm) color scheme for
[Konsole](https://konsole.kde.org/), KDE's terminal emulator.

The template generates a `.colorscheme` file with the 16 ANSI colors (normal + bright),
background, and foreground.

## How to enable the theme in Konsole

1. **Apply the template** with PTM so that `ptm.colorscheme` is generated
   in `~/.local/share/konsole/`.

2. **Open Konsole** and navigate to:
   **Settings → Edit Current Profile… → Appearance** (_Color scheme and font_ tab).

3. In the color scheme list, find **PTM** and select it.

4. Click **Apply** and then **OK**.

> [!TIP]
> If the scheme does not appear in the list after applying the template for the very first time,
> close and reopen Konsole so it indexes new schemes in `~/.local/share/konsole/`.

## Generated file path

| Installation | Path |
| --- | --- |
| Native | `~/.local/share/konsole/ptm.colorscheme` |

Konsole automatically discovers `.colorscheme` files placed in
`$XDG_DATA_HOME/konsole/` (defaults to `~/.local/share/konsole/`).

## Preserving user settings (opacity, blur, wallpaper)

The template ships with standard defaults for the `[General]` section (`Opacity=1`, `Blur=false`, `Description=PTM`), ensuring the scheme always has a valid name and works out of the box.

When you customize opacity, background blur, or wallpaper in Konsole's Color Scheme editor, the included hooks preserve your preferences across theme re-applies:

- **`pre-apply.sh`** (pre-hook): extracts and backs up your customized `[General]` section before PTM writes fresh colors.
- **`apply.sh`** (post-hook): restores your saved `[General]` settings into the newly generated scheme and ensures `Description=PTM` is preserved.

## Live reload without restarting

The **`apply.sh`** post-hook automatically reloads active Konsole sessions when PTM generates new colors:

- Evicts Konsole's in-memory scheme cache by cycling through a lightweight transition scheme using terminal escape sequences (`OSC 50`).
- Reloads the updated colorscheme file directly from disk into all open tabs and split views in real time, without requiring D-Bus permissions or restarting Konsole.

## Testing locally

To test this template before it is merged into the catalog, configure it as a **user template** in `~/.config/ptm/config.toml`:

```toml
[theme.templates.user.konsole]
input_path  = "/path/to/community-templates/konsole/ptm.colorscheme"
output_path = "$XDG_DATA_HOME/konsole/ptm.colorscheme"
pre_hook    = "bash /path/to/community-templates/konsole/pre-apply.sh"
post_hook   = "bash /path/to/community-templates/konsole/apply.sh"
```

Then reload or apply a theme with PTM.

## Notes

- **Faint** colors are assigned the same values as the normal colors, and **Intense** colors use the corresponding bright variant, following the standard Konsole scheme convention.

Tested against Konsole 24+.
