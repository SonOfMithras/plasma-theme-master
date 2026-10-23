# Zathura

This template renders the color values into `ptmrc`, next to your real `zathurarc`. Zathura
does not load `ptmrc` on its own; add one line to your own `zathurarc` so it does:

```
include ptmrc
```

## Native install

`zathurarc` lives at `~/.config/zathura/zathurarc`. Add the `include` line there.

## Flatpak install (`org.pwmt.zathura`)

Flatpak's sandbox keeps its own isolated config directory and cannot see `~/.config/zathura` by
default, so the `zathura-flatpak` entry in `template.toml` renders a second copy of
`ptmrc` straight into it:

```
~/.var/app/org.pwmt.zathura/config/zathura/ptmrc
```

Add the same `include ptmrc` line to `~/.var/app/org.pwmt.zathura/config/zathura/zathurarc`.
No `flatpak override` is needed. That directory is real, writable host storage even though the
running app is sandboxed; only reads from inside the sandbox are restricted, and this template
writes from the host side.

Both entries are skipped automatically if the matching install is not present
(`requires_path`), so enabling this template is safe regardless of which install you use, or
whether you have both.
