# MuseScore 4

Colors MuseScore 4's dark and light UI themes and score-canvas backgrounds with the PTM palette.

## Setup

Install MuseScore 4, then enable **MuseScore4** in **PTM Settings -> Templates**. The generated fragment is written to `$XDG_CONFIG_HOME/MuseScore/ptm.ini`.

## Configuration changes

The Bash post-hook creates `$XDG_CONFIG_HOME/MuseScore/MuseScore4.ini` if needed and inserts the fragment into its `[ui]` section (creating that section if absent). When the section already exists, it removes existing `application\themes` and `canvas\background` settings before inserting the generated values. Other settings are retained; writes follow existing symlinks.

Back up `MuseScore4.ini` before enabling: the hook does not make a backup and replaces existing theme and canvas customizations. It uses standard shell utilities including `awk`, `grep`, `mktemp` and `cmp`. There is no application reload command in the hook.
