# lsd

Adds a PTM colour theme for [lsd](https://github.com/lsd-rs/lsd) (LSDeluxe): user, group, permissions, dates,
sizes, inodes, links, tree edges and Git status.

![lsd with its default colours (top) and the PTM theme (bottom), in light mode](screenshot.png)

Tested with lsd 1.2.0.

## Setup

Install lsd, then enable **lsd** in **PTM Settings -> Templates**. No manual setup is needed.

The template writes `$XDG_CONFIG_HOME/lsd/colors.yaml` (`~/.config/lsd/colors.yaml` by default). PTM creates the
directory if it does not exist.

## The post-hook

lsd only reads `colors.yaml` when its config selects the custom colour theme. After each render, `apply.sh` makes sure
lsd's config contains:

```yaml
color:
  theme: custom
```

It edits the config file lsd actually reads: the first `config.yaml` or `config.yml` found in `~/.config/lsd`, then
`$XDG_CONFIG_HOME/lsd`, the same order lsd searches.

- **No config file:** lsd never creates one itself, so the hook creates `$XDG_CONFIG_HOME/lsd/config.yaml` containing
  only the lines above.
- **`theme` already set:** the value is replaced with `custom`, discarding any comment on that line. A config that
  already selects `custom` is left untouched.
- **`color:` block without `theme`:** `theme: custom` is added as the first entry in the block, at the block's existing
  indentation.
- **No `color:` block:** one is appended to the end of the file.
- **Inline `color:` value** (for example `color: { when: auto }`): the hook changes nothing and exits with an error.
  Rewrite it as a block by hand.

All other settings are left as they were. The file is only rewritten when something changes, and symlinked configs are
written through rather than replaced. No backup is made; copy your config first if you want one.

The hook exits with an error if `lsd` is not on `PATH`.

## Applying changes

lsd reads its theme on every run, so the next `lsd` invocation after a theme change uses the new colours. Nothing needs
reloading.

## Notes

- **An existing `colors.yaml` is overwritten** on every theme change, and any colour theme you had selected is replaced
  by `custom`. Back up your own lsd colours before enabling the template.
- **File and directory names are not themed by this template.** lsd colours names from `LS_COLORS`, not from
  `colors.yaml`. The default `LS_COLORS` use the terminal's ANSI palette, so names follow PTM if your terminal is
  themed too.
- If `$XDG_CONFIG_HOME` is not `~/.config` and you also have a `~/.config/lsd/colors.yaml` or `colors.yml`, lsd reads
  that file instead of the one this template writes. Remove it.
- The `attributes` section (Windows only) is left at lsd's defaults.
