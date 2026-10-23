# Starship

Adds a `ptm` palette for the [Starship](https://starship.rs) prompt and selects it. The palette maps the terminal
colours (`blue`, `red`, ...) plus Catppuccin-compatible names (`rosewater`, `mauve`, ...), so prompts written against
either keep working.

## Setup

Install Starship, then enable **starship** in **PTM Settings -> Templates**. No manual setup is needed.

The template renders the palette to `$XDG_CACHE_HOME/ptm/starship-palette.toml`; the post-hook copies it into your
Starship config.

## The post-hook

`apply.sh` edits the config Starship reads, found in this order:

1. `STARSHIP_CONFIG` in PTM's own environment.
2. `STARSHIP_CONFIG` in the systemd user environment (`systemctl --user show-environment`).
3. `STARSHIP_CONFIG` in the environment of one of your running processes, read from `/proc`.
4. `$XDG_CONFIG_HOME/starship.toml` (`~/.config/starship.toml` by default).

PTM's daemon does not inherit variables exported only from `.bashrc` or `.zshrc`. If your config lives elsewhere,
export `STARSHIP_CONFIG` in your login environment (for example `~/.config/environment.d/`) so step 2 finds it.

In that file the hook:

- sets `palette = "ptm"` at the top, or right after a `"$schema"` line, replacing any other `palette =` line;
- writes the palette between `# >>> PTM STARSHIP PALETTE >>>` and `# <<< PTM STARSHIP PALETTE <<<`,
  replacing the previous block.

A missing config is created with only those two parts. The file is only rewritten when something changes, and symlinked
configs are written through rather than replaced. No backup is made.

## Disabling

Disabling the template runs `undo.sh`, which removes the marked block and the `palette = "ptm"` line and deletes the
cached palette. Select another palette yourself if you had one before.

## Notes

- **Read-only configs are not supported.** A `starship.toml` managed by Home Manager is a symlink into the Nix store and
  cannot be edited. Point `STARSHIP_CONFIG` at a writable file instead, or add the palette to your declarative config.
- Earlier PTM versions shipped this as a built-in template with the same markers. Enabling this one takes over the
  existing block; if you do not enable it, delete the block and the `palette = "ptm"` line by hand.
