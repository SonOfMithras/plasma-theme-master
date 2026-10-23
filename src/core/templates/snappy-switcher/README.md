# snappy-switcher

Generate a PTM theme for snappy-switcher's cards, text, borders, and badges.

Enable **snappy-switcher** in **PTM Settings → Templates**. The theme is written to `$XDG_CONFIG_HOME/snappy-switcher/themes/ptm.ini`.

## Hook behavior and prerequisites

Install snappy-switcher with `snappy-install-config` and `snappy-switcher` available on `PATH`. The Bash hook also uses `mktemp`, `sed`, `cmp`, `cat`, and `rm`.

**Path caveat:** the hook looks for `snappy-switcher/config.ini` under `${XDG_CACHE_HOME:-$HOME/.config}`, not under `XDG_CONFIG_HOME`. If that file is missing, it runs `snappy-install-config`. Ensure your configuration and theme lookup match these paths, especially with custom XDG directories.

The hook replaces the exact text `name = snappy-slate.ini` with `name = ptm.ini`. If you already use another theme name, change that selection manually. Back up `config.ini` first; no backup is created. The hook writes through the existing file only when changed, then runs `snappy-switcher quit` and starts `snappy-switcher --daemon` in the background.
