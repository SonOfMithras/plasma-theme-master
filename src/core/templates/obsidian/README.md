# Obsidian — PTM Community Template

Applies ptm's generated color palette to [Obsidian](https://obsidian.md) as a CSS snippet.
Both dark and light mode variants are generated, and Obsidian will use whichever matches its
current appearance setting.

## How it works

This template renders a CSS snippet (`ptm.css`) into each discovered Obsidian vault's
`snippets/` directory. The snippet overrides Obsidian's CSS custom properties to match
ptm's active palette — backgrounds, text, accents, headings, links, tags, graph view,
and more.

The apply script automatically enables the snippet in each vault's `appearance.json`.

## Vault discovery

The apply script searches `$HOME` (up to 4 directories deep) for `.obsidian/` folders.
If your vault lives deeper or on an external drive, you can add a user template override
in your ptm config with an explicit `output_path`:

```toml
[theme.templates.user.obsidian_extra]
input_path  = "obsidian/obsidian.css"
output_path = "/mnt/data/MyVault/.obsidian/snippets/ptm.css"
```

## Requirements

- Obsidian (any recent version with CSS snippet support)
- `python3` (for enabling the snippet in `appearance.json`)

## Setup

1. Enable **Obsidian** in **PTM Settings -> Templates**.
2. In Obsidian, open **Settings → Appearance** and set **Base color scheme** to **Adapt to system**, so the
   dark or light variant follows PTM's mode.
3. Check that the `ptm` snippet is on under **Settings → Appearance → CSS Snippets**.

## Notes

- The snippet layers on top of any installed Obsidian theme — it overrides color
  variables without removing your base theme.
- Obsidian hot-reloads CSS snippets from disk on most platforms. If colors don't
  update after a wallpaper change, toggle the snippet off/on in
  Settings → Appearance → CSS Snippets.
