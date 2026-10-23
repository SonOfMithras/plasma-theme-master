# Fuzzel

Generates PTM colors for the Fuzzel launcher, including translucent background and selection colors.

Enable **Fuzzel** in **PTM Settings -> Templates**. The theme is written to `$XDG_CONFIG_HOME/fuzzel/themes/ptm` (`~/.config/fuzzel/themes/ptm` by default).

The Bash post-hook creates or updates `$XDG_CONFIG_HOME/fuzzel/fuzzel.ini` to include the generated theme using an absolute `include=.../fuzzel/themes/ptm` line. If the exact include is absent, existing lines beginning with `include=` and containing `themes` are replaced; otherwise the include is appended. Other settings are retained.

Install Fuzzel and back up `fuzzel.ini` first: the hook changes it without creating a backup. No launcher reload command is included.
