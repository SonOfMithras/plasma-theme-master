# Sidra

Applies the active PTM color palette to [Sidra](https://github.com/wimpysworld/sidra)'s custom theme.

## Setup

In Sidra, open **Settings → Style → Custom Theme** and select **Custom Theme** once.

No manual creation of `custom-theme.json` is required. PTM renders the template to:

```text
$XDG_CONFIG_HOME/Sidra/custom-theme.json
```

If `XDG_CONFIG_HOME` is not set, this resolves to:

```text
~/.config/Sidra/custom-theme.json
```

The generated theme follows PTM's active palette and is updated automatically when the PTM theme changes.

Only the `dark` palette is generated. Sidra uses this palette for both modes when no separate `light` palette is provided.
