# Brave

Packed Chromium theme for Brave chrome (tabs, toolbar, frame, NTP, omnibox).

Tested against Brave 151.1.93.136 and PTM v5.0.0 on Linux.

## Enable

1. Enable this template in PTM (Settings → Templates, or a user-template pointer).
2. First time only — load the unpacked theme:
   1. Open `brave://extensions`
   2. Enable Developer mode
   3. Load unpacked → `~/.local/share/ptm/brave-theme`
   4. Confirm Appearance uses the "PTM" theme
3. After palette changes: `brave://extensions` → Developer mode → Update (or restart Brave).

## What the hook does

`apply.sh` writes solid PNGs next to the rendered `manifest.json`. Color-only
Chrome themes often leave the frame and toolbar unpainted on Linux.

If Brave is **stopped**, the hook also sets `browser.theme.color_scheme` and
`browser.theme.user_color` (Material You seed from the palette primary). It
never writes Preferences while Brave is running.

```sh
bash apply.sh dark    # or light
bash apply.sh backup  # one-time original Preferences snapshot
bash apply.sh restore # Brave must be stopped
```

## Limits

- `brave://settings` checkboxes and Leo accents are not in Chromium's
  overwriteable theme table. A packed theme cannot recolor those.
- `user_color` is a seed; Leo may still ignore it.

## Restore

Original Preferences are snapshotted once under
`$XDG_DATA_HOME/ptm/brave-theme/backups/`. Restore only with Brave quit.
