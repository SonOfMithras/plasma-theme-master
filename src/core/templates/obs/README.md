# OBS Studio

Generates the **Matugen** OBS Studio theme using PTM colors. Some semantic colors remain fixed, and the theme metadata marks it as dark.

Enable **OBS** in **PTM Settings -> Templates**. The output is `$XDG_CONFIG_HOME/obs-studio/themes/matugen.obt`. Use an OBS Studio version that supports `.obt` themes and select **Matugen** in OBS's theme settings manually.

Back up an existing file at that path before enabling. No hook changes OBS settings or triggers a reload, and no Flatpak-specific output is supplied.
