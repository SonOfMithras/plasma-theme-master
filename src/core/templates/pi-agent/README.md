# Pi Agent

Generates a **ptm** theme for Pi Agent's interactive UI, Markdown, syntax highlighting and tool output.

Enable **Pi Agent** in **PTM Settings -> Templates**. The theme is written to `~/.pi/agent/themes/ptm.json`.

The Bash post-hook selects the theme in `~/.pi/agent/settings.json`, creating the directory and a minimal settings file if absent. For an existing file it uses `grep` and `sed` to replace a string-valued `theme` entry or insert one after the first line's opening brace. Keep settings in a conventional JSON-object layout: this is a text edit, not a JSON-aware merge.

Back up existing settings and theme customizations first; no backup is made. Changed settings are written through existing symlinks. The hook uses standard shell utilities and does not invoke Pi Agent or send a reload command.
