# Yapfast — PTM Community Template

Applies PTM's active palette to [Yapfast](https://github.com/linuxmobile/yapfast),
the native Discord client, as a theme called **PTM**.

## What gets themed

Yapfast has no themes directory: it stores its themes in the `custom_themes` array
of `layout.json`. This template renders a complete custom theme and merges it into
that file, preserving every other setting. The rendered palette is cached at:

```
$XDG_CACHE_HOME/ptm/yapfast/theme.json
```

and the apply hook writes to the first existing path among:

- `$YAPFAST_LAYOUT`, if set
- `$XDG_DATA_HOME/yapfast/layout.json` (normally `~/.local/share/yapfast/layout.json`)

The theme is created once and updated in place on later palette changes. It becomes
the selected theme, so the next time Yapfast draws its window it uses the new
colours. Every colour Yapfast knows is written, so the theme does not depend on
the fallback base once applied.

Dark and light PTM modes are both supported: the window's `dark` flag and all
surfaces, text, accent, status and hover colours follow the active mode.

## Setup

None. Install Yapfast, make sure it has created `layout.json` (starting it once is
enough), then enable the template in PTM under **Templates**.

## Restarting Yapfast

Yapfast keeps its layout in memory and writes the whole file when settings change.
If Yapfast is already running while the palette is applied, **restart it** before
touching its settings; otherwise its next save can replace the merged theme with
the in-memory copy. The apply hook prints a warning when it sees a running Yapfast
process.

## Notes

- Only `custom_themes` and the selected `theme` key in `layout.json` are changed.
- The hook is idempotent: applying the same palette twice produces the same file.
- The file is written atomically, so an interrupted apply leaves the previous layout
  intact.
- Tested against Yapfast 0.1.0.
