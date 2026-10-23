# herdr

This template keeps Herdr's custom UI colors in sync with PTM while preserving the rest of your configuration.

Herdr needs an existing config file before the template is enabled. If you do not have one yet:

```sh
config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/herdr"
mkdir -p "$config_dir"
herdr --default-config >"$config_dir/config.toml"
```

The apply hook updates the supported color keys in `[theme.custom]` and leaves all other settings and custom keys intact.
It also reloads a running Herdr server when possible. A path set through `HERDR_CONFIG_PATH` is honored.
