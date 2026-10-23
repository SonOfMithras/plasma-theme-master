# Antigravity

Generates dark and light custom theme seeds for Antigravity.

Enable **Antigravity** in **PTM Settings -> Templates**. The generated file is `~/.gemini/config/theme.json`.

## Setup and hook

Install `jq`, which the Bash post-hook uses to merge the generated `userSettings.customThemeSeedsDark` and `userSettings.customThemeSeedsLight` into `~/.gemini/config/config.json`. Other settings, including an existing `themeMode`, are retained. If the config does not exist, the hook copies the generated file, which also sets `themeMode` to `THEME_MODE_INHERIT`.

Back up `config.json` before enabling: the hook writes it without making a backup. No application reload command is included.
