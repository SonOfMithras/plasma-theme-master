# Oh My Posh

Recolor an existing Oh My Posh prompt using PTM's 16 terminal colors plus
terminal foreground and background. Prompt layout, segments, and Go expressions
come from the user's imported configuration; there is no bundled prompt theme.

PTM supplies curated terminal colors for named palettes and generates them
for wallpaper palettes. The hook uses those colors directly. It does not need
an adapter for every OMP theme or depend on the terminal emulator being themed.

OMP themes define their own palette names and can also use literal colors inside
segments. A fixed palette template would only work for themes sharing those
names. The importer and hook handle those differences while retaining the
user's prompt; the tests cover the transformations and file ownership guards.

Requires Python 3 on Linux and Oh My Posh. Tested with PTM 5.2.0 and
Oh My Posh 30.9.0.

## Setup from a checkout

The examples assume this repository is at `~/community-templates`. Adjust that
path if your checkout is elsewhere.

1. From a terminal where OMP is initialized, import its current configuration:

   ```sh
   cd ~/community-templates
   python3 oh-my-posh/apply.py import
   ```

   The importer reads `$POSH_THEME`. If it is unset, pass the original config:

   ```sh
   python3 oh-my-posh/apply.py import --config /path/to/original.omp.json
   ```

   JSON, YAML, TOML, and inheritance are normalized through OMP's `config export`.
   This creates `~/.config/oh-my-posh/ptm-source.omp.json`, an editable copy
   with the original colors. The original file is not modified.

2. Register a user template in `~/.config/ptm/config.toml`:

   ```toml
   [theme.templates.user.oh_my_posh]
   input_path = "~/community-templates/oh-my-posh/palette.json"
   output_path = "$XDG_CONFIG_HOME/oh-my-posh/ptm-palette.json"
   post_hook = "python3 \"$HOME/community-templates/oh-my-posh/apply.py\" apply"
   hook_async = false
   ```

   For a community catalog installation, `template.toml` provides this
   registration. Run the import command using `apply.py` in the downloaded
   template directory before applying it. The hook reports an error if no
   source config has been imported yet.

3. Render the current palette and apply it:

   ```sh
   ptm msg config-reload
   ptm msg templates-apply
   ```

4. Point the existing OMP initialization at the generated file. For Zsh:

   ```sh
   eval "$(oh-my-posh init zsh --config "${XDG_CONFIG_HOME:-$HOME/.config}/oh-my-posh/ptm.omp.json")"
   ```

   Replace the existing initialization line, then open a terminal. For another
   shell, keep its initialization syntax and change only the config path.

PTM's desktop process does not normally inherit a terminal's `$POSH_THEME`,
so importing is a one-time action from the user's shell. Later palette changes
run the local hook automatically. The hook reads the imported source each time,
writes the generated prompt atomically, and skips identical output.

The hook records its generated output's checksum in
`ptm-output-state.json`. It refuses an existing unmanaged prompt, a prompt
edited after generation, or a symlink at either output/state path. An unchanged
reapply preserves both files' timestamps. Interrupted updates can resume from
either complete prompt version, and concurrent hook runs are serialized.

All OMP integration files honor `$XDG_CONFIG_HOME`. `--directory /some/path`
before the subcommand can override their directory, for example when testing.
The apply hook is offline and uses only Python's standard library; it does not
invoke OMP, source shell profiles, or run prompt segments.

## Color mapping

- ANSI names and indices 0–15 use the corresponding PTM terminal slot.
- Hex colors and other ANSI indices are classified by hue and lightness.
  Reds map to red slots, greens to green slots, and so on. Neutral shades use
  foreground, background, or gray slots.
- Arbitrary palette names work because their values are recolored. Palette
  references and runtime palette selectors are retained.
- Foreground/background fields, conditional color outputs, gradients, shade
  functions, and inline color tags in prompt templates are supported.
- Neutral text on a statically known background is adjusted to a more readable
  PTM neutral when its contrast is below 4.5:1. This chooses among supplied
  colors; it cannot guarantee a threshold if the palette itself lacks contrast.

The classification is a heuristic: similar source colors can share a terminal
slot. It cannot infer that an arbitrary brand color means "warning", or recover
the intention of every theme. Go code that computes a color, for example
`{{ printf "#%06x" .Code }}`, is preserved. Colors embedded inside Go string
literals, custom variables, or unrecognized option fields may keep their
original shades. Dynamic foreground/background pairs are not contrast-adjusted.

Inspect the mapping without writing the prompt:

```sh
python3 oh-my-posh/apply.py apply --dry-run
```

For an ambiguous color, create `~/.config/oh-my-posh/ptm-map.json`:

```json
{
  "#abcdef": "green",
  "#b7bdf8": "lightMagenta"
}
```

Keys are source color values; values are OMP ANSI color names (`red`, `lightRed`,
`darkGray`, etc.) or `foreground`/`background`. Run `ptm msg templates-apply`
after editing the map or the imported source.

## Change or restore the prompt

Edit `ptm-source.omp.json` to change the imported layout. To capture a new
theme or edits made to the original config, re-import explicitly:

```sh
python3 oh-my-posh/apply.py import --config /path/to/original.omp.json --replace
ptm msg templates-apply
```

The importer refuses its own generated output to prevent repeated recoloring.
To restore the original theme, point OMP's initialization back to its original
config and disable this PTM template. The generated copy is separate.

### Existing or edited output

If applying reports that `ptm.omp.json` is unmanaged or was edited, inspect
that file first and put any changes you want to keep in
`ptm-source.omp.json`. Move the conflicting output to a backup, then apply
again. For example, from the OMP integration directory:

```sh
mv -i ptm.omp.json ptm.omp.json.backup
ptm msg templates-apply
```

This also applies to output generated by an older version without an ownership
record. There is no automatic takeover or force-overwrite option. Keep
`ptm-output-state.json` alongside the generated prompt; if that record is
missing, malformed, or unrelated, move both files aside before regenerating.
The hook never modifies the saved backup.

## Validation

From the repository root:

```sh
python3 -B .github/workflows/validate-templates.py
python3 -B oh-my-posh/test_apply.py
```

Tests cover preservation of Go expressions and non-color data, palette variants,
indexed colors, gradient percentages, inline colors, contrast, import guards,
atomic output, output collisions, edited files, symlinks, interrupted updates,
concurrent hooks, and repeated application. Integration checks have also exercised
dark/light palettes with Catppuccin, Atomic, Jandedobbeleer, and Powerlevel10k
Rainbow, plus imports in all three OMP config formats.
