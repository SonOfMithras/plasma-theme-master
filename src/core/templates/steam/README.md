# Steam

Generate PTM CSS color variables for Steam's **Material-Theme** skin. This is not a standalone skin for unmodified Steam.

## Setup

The skin is loaded by [Millennium](https://github.com/SteamClientHomebrew/Millennium), an open-source Steam client modding framework, and uses [Material-Theme](https://github.com/kuska1/Material-Theme).

1. Install Millennium by following [its installation page](https://docs.steambrew.app/users/getting-started/installation) for your distribution.
2. Copy the theme ID from [Material-Theme on steambrew.app](https://steambrew.app/theme/ipYjqODds05KMcvh7QJn?id=ipYjqODds05KMcvh7QJn) and install it from Millennium's theme settings. It shows up installed but not yet enabled:

   ![Material-Theme installed but disabled](millennium-installed.png)

3. Click **Use** to enable it, then open the drop-down on the right to configure it:

   ![Material-Theme configuration drop-down](millennium-configure.png)

4. In the **Color** drop-down, select **Matugen**:

   ![Matugen color option](millennium-matugen.png)

5. Enable **Steam** in **PTM Settings → Templates**.
6. Regenerate the palette (change the wallpaper or toggle dark/light mode), then fully restart Steam (**Steam → Exit**).

## Output

The template writes directly to:

```text
~/.steam/steam/steamui/skins/Material-Theme/css/main/colors/matugen.css
```

The stylesheet identifies the color scheme as `Matugen` and defines `--md-sys-color-*` variables from the PTM palette. Ensure your Material-Theme setup loads this color file; other Steam installation paths are not targeted by this manifest.

The generated file is overwritten on regeneration, so back up any existing custom file at that path. There is no post-hook to install the skin, select a color scheme, or reload Steam; restart Steam to pick up new colors.
