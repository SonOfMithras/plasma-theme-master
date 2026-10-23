![tmux dark and light Material themes](screenshot.png)

## Options

Inactive tabs use their own background by default. To make them inherit the status bar background, add this before the PTM `source-file` line in your tmux config:

```tmux
set -g @ptm_inactive_tabs_use_background off
```
