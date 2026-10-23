# fzf

Generates shell snippets that add PTM colors to `FZF_DEFAULT_OPTS`.

Enable the appropriate **fzf** entries in **PTM Settings -> Templates**:

- Bash: `$XDG_CONFIG_HOME/fzf/themes/ptm.sh`
- Fish: `$XDG_CONFIG_HOME/fzf/themes/ptm.fish`

`$XDG_CONFIG_HOME` defaults to `~/.config`. Install fzf and source the appropriate generated file after your other fzf options. For example, with the default paths:

```bash
# Add to ~/.bashrc
source "$HOME/.config/fzf/themes/ptm.sh"
```

```fish
# Add to ~/.config/fish/config.fish
source "$HOME/.config/fzf/themes/ptm.fish"
```

Adjust the paths if you use a custom config directory. There is no hook to edit your shell startup files or reload an existing shell; source the file yourself to apply newly generated colors.

Both snippets append color options instead of replacing existing options. Repeated sourcing adds another copy. The Fish snippet uses `set -Ux`, so it persists the exported value as a universal variable; repeated shell startups can also accumulate color options. Keep a copy of your original `FZF_DEFAULT_OPTS` if you need to restore it.
