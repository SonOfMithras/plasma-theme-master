# Yazi

Generates a PTM flavor for Yazi's interface, file icons and syntax-highlighted previews. The flavor uses glyph icons, so use a font that contains those glyphs, such as a Nerd Font, for complete icon display.

## Setup

Install Yazi, then enable **Yazi** in **PTM Settings -> Templates**. The post-hook selects the generated flavor for both dark and light modes automatically.

## Output paths

- `$XDG_CONFIG_HOME/yazi/flavors/ptm.yazi/flavor.toml`
- `$XDG_CONFIG_HOME/yazi/flavors/ptm.yazi/tmtheme.xml`

## Configuration changes

The Bash hook creates `$XDG_CONFIG_HOME/yazi/theme.toml` if needed, or updates its `[flavor]` section to contain:

```toml
[flavor]
dark = "ptm"
light = "ptm"
```

Existing dark/light flavor choices are replaced; missing keys or the section itself are added. Other settings are retained. The hook uses `awk`, `mktemp` and `cmp`, writes through symlinks, and makes no backup. Back up an existing `theme.toml` before enabling the template. Existing explicit theme overrides remain in place.

The hook does not restart Yazi or send a reload command. With `XDG_CONFIG_HOME` unset, the hook uses `~/.config`.
