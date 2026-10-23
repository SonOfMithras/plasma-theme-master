# Glow

![Glow using the PTM theme](screenshot.png)

Tested with Glow 3.0.0.

Use an absolute custom-style path: Glow 3.0 does not expand `$...` or `~` when
passing a configured style to Glamour. Set `GLOW_STYLE` in your shell
environment so the shell expands the path before Glow reads it:

```sh
export GLOW_STYLE="${XDG_CONFIG_HOME:-$HOME/.config}/glow/ptm.json"
```
