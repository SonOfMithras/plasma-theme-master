# Rofi

A PTM Rasi theme for Rofi, with rounded panels, highlighted selections, and an eight-line list.

Enable **Rofi** in **PTM Settings → Templates** to generate `$XDG_CONFIG_HOME/rofi/ptm.rasi` (normally `~/.config/rofi/ptm.rasi`). Then select it explicitly, for example:

```sh
rofi -show drun -theme "${XDG_CONFIG_HOME:-$HOME/.config}/rofi/ptm.rasi"
```

Alternatively, reference the generated file from your Rofi configuration. The template has no hook: it does not change your existing configuration or reload a running launcher. Changes are available to subsequent launches using this theme. Keep customizations outside the generated file, which is overwritten on regeneration.
