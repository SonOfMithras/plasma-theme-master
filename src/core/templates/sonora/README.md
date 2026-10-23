# Sonora

Themes [Sonora](https://github.com/sonorahq/sonora) using the active PTM palette.

The template supports both native installations and the
`io.github.nolight132.sonora` Flatpak. It writes the palette to
`themes/ptm.json` in Sonora's config folder, where it shows up as PTM
in Settings > Appearance, and selects it in `settings.json`. Sonora watches both
files and applies the new colors while running.

Selecting the theme turns off Sonora's adaptive theme, which would otherwise
tint the colors from the playing cover.

Launch Sonora once before enabling the template so its configuration directory
and `settings.json` exist. The hook requires `jq`. Sonora 0.41.0 or newer is
needed to load theme files.
