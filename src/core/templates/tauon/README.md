# Tauon

Generate a PTM theme for Tauon Music Box, covering panels, playlists, menus, controls, and dialogs.

Enable **Tauon** in **PTM Settings → Templates**. The theme is written to `$XDG_DATA_HOME/TauonMusicBox/theme/PTM.ttheme` (normally `~/.local/share/TauonMusicBox/theme/PTM.ttheme`). Select the generated **PTM** theme in Tauon's theme controls.

The manifest targets this user-data path only; installations using another data directory need the generated theme placed in their own theme directory. No hook selects the theme or reloads Tauon. The file is overwritten on regeneration, so keep custom edits elsewhere.
