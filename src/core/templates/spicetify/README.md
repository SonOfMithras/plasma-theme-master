# Spicetify — Colorful and Comfy

A PTM v5 color template for the [Colorful](https://github.com/sanoojes/spicetify-colorful) and [Comfy](https://github.com/Comfy-Themes/Spicetify) Spicetify themes.

Generates the theme's `color.ini` from your active PTM palette and reapplies Spicetify automatically whenever your theme changes.

## Requirements

- [Spicetify](https://spicetify.app/docs/getting-started) installed and working
- The **[Colorful](https://github.com/sanoojes/spicetify-colorful)** theme **MANUALLY** installed at `~/.config/spicetify/Themes/Colorful`, or the **[Comfy](https://github.com/Comfy-Themes/Spicetify)** theme **MANUALLY** installed at `~/.config/spicetify/Themes/Comfy`

## What it does

- `spicetify_colorful.ini` — maps PTM's palette to Colorful's `color.ini` keys, as the `ptm` color scheme.
- `spicetify.ini` — maps PTM's palette to Comfy's `color.ini` keys, as the `Comfy` color scheme.
- `template.toml` — tells PTM where to write the generated files (`$XDG_CONFIG_HOME/spicetify/Themes/Colorful/color.ini` and `$XDG_CONFIG_HOME/spicetify/Themes/Comfy/color.ini`) and what to run afterward.
- `apply.sh` — runs `spicetify apply -q --no-restart` so Spotify picks up the new colors without a manual step.

## Installing Comfy

Comfy must be installed manually (see [its instructions](https://github.com/Comfy-Themes/Spicetify?tab=readme-ov-file#-manual-installation)):

```sh
cd "$(spicetify config-dir)/Themes"
git clone https://github.com/Comfy-Themes/Spicetify.git ComfyRepo
mv ComfyRepo/Comfy ./
rm -rf ComfyRepo

spicetify config current_theme Comfy color_scheme Comfy
spicetify config inject_css 1 replace_colors 1 overwrite_assets 1 inject_theme_js 1
spicetify apply
```

Then enable **Spicetify** in **PTM Settings -> Templates**. If the colors do not show up, run `spicetify apply` or restart Spotify.

## Manual Installation

1. Add this block to your `config.toml`:

   ```toml
   [theme.templates.user.spicetify]
   input_path = "spicetify_colorful.ini"
   output_path = "$XDG_CONFIG_HOME/spicetify/Themes/Colorful/color.ini"
   post_hook = "spicetify apply -q --no-restart"
   ```

2. Place `spicetify_colorful.ini` in the same directory as your `config.toml`, since `input_path` is resolved relative to it.

## Applying Colorful

To apply the Colorful theme run the commands below:

```sh
spicetify config current_theme Colorful color_scheme ptm
spicetify config inject_css 1 replace_colors 1 overwrite_assets 1 inject_theme_js 1
spicetify apply
```

## Notes

- `apply.sh` runs `spicetify apply --no-restart`, so Spotify won't be force-closed.
  If the theme is already applied, hot-reload it with `Ctrl+Shift+R` instead.
- This template is limited to color values only. Any CSS inconsistencies,
  layout issues, or other formatting concerns are the responsibility of the
  Colorful or Comfy theme itself and should be reported upstream.
