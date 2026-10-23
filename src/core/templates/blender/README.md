# Blender

Blender's theme system isn't a CSS file, it's roughly 570 individual color properties on
`bpy.context.preferences.themes[0]`, saved into a binary `userpref.blend`, not a plain-text
config. So this template doesn't just render a file, `input_path` is a Python script (with
`{{colors.role.default.hex}}` placeholders substituted the normal way), and `post_hook` runs it
through Blender itself in headless mode, which applies the colors via the real `bpy` API and
saves preferences directly. No GUI window opens, `--background` mode is fully invisible.

## What gets themed, and what deliberately doesn't

Only structural chrome: panel backgrounds, headers, tab strips, widget fills/outlines/text,
editor borders, list backgrounds, metadata overlay boxes. Around 345 of the ~570 properties.

The rest (about 225) are left at Blender's own defaults on purpose: mesh-editing selection
states (vertex/edge/face colors), node category colors, sequencer strip-type colors, syntax
highlighting, keyframe types, axis/gizmo colors. These are workflow conventions users rely on
for actual 3D work, not decoration, recoloring them to an arbitrary palette would hurt
usability rather than improve consistency. Same reasoning already applied to terminal ANSI
colors and Blender's own axis-color convention (X=red, Y=green, Z=blue) elsewhere.

## Native install

Runs `blender --background --python ...` directly. No setup beyond having `blender` on `PATH`.

## Flatpak install (`org.blender.Blender`)

Blender's Flatpak has `filesystems=host` (full host filesystem access), broader than most
sandboxed apps here. `$XDG_CONFIG_HOME` inside the sandbox is still remapped to
`~/.var/app/org.blender.Blender/config` regardless, same as every other Flatpak app, but that
doesn't matter for this template: the rendered script lives at a plain host path
(`~/.cache/ptm/blender_theme_rendered.py`), and Blender's `host` grant means it can read
that path directly by its real location, no `flatpak override` or `.var/app`-local copy needed
at all.

## Testing

Tested against Blender 5.2.0 LTS (Flatpak). Verified via a full audit: captured Blender's true
factory-default theme values (`--factory-startup`) and diffed against the post-apply state
property by property, confirming exactly which of the ~570 were intentionally changed versus
intentionally left alone, not just spot-checked visually.
