# Zed

Generates a PTM theme family for Zed and Gram, including interface and syntax colors in four variants:

- `PTM Dark`
- `PTM Light`
- `PTM Dark Transparent`
- `PTM Light Transparent`

## Setup

Install Zed or Gram, enable **Zed** in **PTM Settings -> Templates**, then select a generated PTM variant in your editor's theme selector. The file contains both dark and light palettes rather than only the active mode.

## Output paths

- Zed: `$XDG_CONFIG_HOME/zed/themes/ptm.json`
- Gram: `$XDG_CONFIG_HOME/gram/themes/ptm.json`

Both outputs are configured by this template. Back up any hand-written files at these paths before enabling it. No hooks change editor settings or request a reload.
