"""Applies PTM's palette to Blender's UI chrome, run headless via `blender --background --python`.

Only structural chrome is themed: panel backgrounds, headers, widget fills/outlines/text.
Workflow-meaningful colors (mesh selection states, node categories, sequencer strip types,
syntax highlighting) are deliberately left at Blender's own defaults, since users rely on those
as functional conventions, not decoration.
"""

import bpy

COLORS = {
    "surface": "{{colors.surface.default.hex}}",
    "on_surface": "{{colors.on_surface.default.hex}}",
    "surface_variant": "{{colors.surface_variant.default.hex}}",
    "on_surface_variant": "{{colors.on_surface_variant.default.hex}}",
    "surface_container": "{{colors.surface_container.default.hex}}",
    "surface_container_low": "{{colors.surface_container_low.default.hex}}",
    "surface_container_high": "{{colors.surface_container_high.default.hex}}",
    "outline": "{{colors.outline.default.hex}}",
    "outline_variant": "{{colors.outline_variant.default.hex}}",
    "primary": "{{colors.primary.default.hex}}",
    "on_primary": "{{colors.on_primary.default.hex}}",
    "primary_container": "{{colors.primary_container.default.hex}}",
    "on_primary_container": "{{colors.on_primary_container.default.hex}}",
    "secondary": "{{colors.secondary.default.hex}}",
    "secondary_container": "{{colors.secondary_container.default.hex}}",
    "on_secondary_container": "{{colors.on_secondary_container.default.hex}}",
    "tertiary": "{{colors.tertiary.default.hex}}",
    "tertiary_container": "{{colors.tertiary_container.default.hex}}",
    "on_tertiary_container": "{{colors.on_tertiary_container.default.hex}}",
    "error": "{{colors.error.default.hex}}",
    "on_error": "{{colors.on_error.default.hex}}",
    "error_container": "{{colors.error_container.default.hex}}",
}


def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip("#")
    return tuple(int(hex_str[i : i + 2], 16) / 255.0 for i in (0, 2, 4))


RGB = {name: hex_to_rgb(value) for name, value in COLORS.items()}


def classify(path):
    if path.endswith(".space.back"):
        return "surface"
    if path.endswith(".space.header"):
        return "surface_container"
    if path.endswith(".space.header_text"):
        return "on_surface"
    if path.endswith(".space.header_text_hi"):
        return "primary"
    if path.endswith(".space.text"):
        return "on_surface"
    if path.endswith(".space.text_hi"):
        return "primary"
    if path.endswith(".space.title"):
        return "on_surface_variant"
    if path.endswith(".space.tab_active"):
        return "surface_container_high"
    if path.endswith(".space.tab_back"):
        return "surface_container_low"
    if path.endswith(".space.tab_outline"):
        return "outline"
    if path.endswith(".space.gradients.gradient"):
        return "surface"
    if path.endswith(".space.gradients.high_gradient"):
        return "surface_container"
    if path.endswith("row_alternate"):
        return "surface_container_low"

    if ".wcol_" in path:
        suffix = path.rsplit(".", 1)[-1]
        wcol_map = {
            "inner": "surface_container",
            "inner_sel": "primary_container",
            "inner_key": "tertiary_container",
            "inner_key_sel": "tertiary_container",
            "inner_anim": "secondary_container",
            "inner_anim_sel": "secondary_container",
            "inner_driven": "tertiary",
            "inner_driven_sel": "tertiary",
            "inner_changed": "primary",
            "inner_changed_sel": "primary",
            "inner_overridden": "error_container",
            "inner_overridden_sel": "error_container",
            "outline": "outline",
            "outline_sel": "primary",
            "item": "on_surface_variant",
            "text": "on_surface",
            "text_sel": "on_primary",
            "error": "error",
            "warning": "tertiary",
            "success": "secondary",
            "info": "primary",
        }
        if suffix in wcol_map:
            return wcol_map[suffix]

    if path.startswith("regions."):
        if path.endswith(".back") or path.endswith(".header_back"):
            return "surface_container"
        if path.endswith(".tab_back"):
            return "surface_container_low"
        if path.endswith(".text"):
            return "on_surface"
        if path.endswith(".text_selected"):
            return "primary"

    direct = {
        "text_editor.line_numbers": "on_surface_variant",
        "text_editor.line_numbers_background": "surface_container_low",
        "text_editor.cursor": "primary",
        "text_editor.selected_text": "primary_container",
        "image_editor.metadatabg": "surface_container",
        "image_editor.metadatatext": "on_surface",
        "sequence_editor.metadatabg": "surface_container",
        "sequence_editor.metadatatext": "on_surface",
        "sequence_editor.preview_back": "surface",
        "clip_editor.metadatabg": "surface_container",
        "clip_editor.metadatatext": "on_surface",
        "node_editor.node_outline": "outline",
        "user_interface.widget_text_cursor": "primary",
        "user_interface.editor_border": "outline",
        "user_interface.editor_outline": "outline_variant",
        "user_interface.editor_outline_active": "primary",
        "user_interface.panel_header": "surface_container",
        "user_interface.panel_back": "surface_container_low",
        "user_interface.panel_outline": "outline",
        "user_interface.panel_title": "on_surface",
        "user_interface.panel_text": "on_surface",
        "user_interface.panel_active": "primary",
        "outliner.selected_highlight": "primary_container",
        "outliner.active": "primary",
        "outliner.selected_object": "on_primary_container",
        "outliner.active_object": "on_primary",
        "outliner.match": "tertiary_container",
        "info.info_error_text": "error",
        "info.info_warning_text": "tertiary",
        "info.info_info_text": "on_surface",
        "info.info_property_text": "secondary",
        "info.info_operator_text": "primary",
        "info.info_debug_text": "outline",
        "info.info_selected": "primary_container",
        "info.info_selected_text": "on_primary_container",
        "console.line_output": "on_surface",
        "console.line_input": "primary",
        "console.line_info": "secondary",
        "console.line_error": "error",
        "console.select": "primary_container",
        "console.cursor": "primary",
        "file_browser.selected_file": "primary_container",
        "preferences.match": "tertiary_container",
        "properties.match": "tertiary_container",
    }
    if path in direct:
        return direct[path]

    return None


def walk_and_apply(obj, path="", depth=0):
    if depth > 4:
        return 0
    applied = 0
    for prop in obj.bl_rna.properties:
        if prop.identifier == "rna_type":
            continue
        try:
            val = getattr(obj, prop.identifier)
        except Exception:
            continue
        full = f"{path}.{prop.identifier}" if path else prop.identifier
        if prop.type == "POINTER" and val is not None:
            applied += walk_and_apply(val, full, depth + 1)
        elif prop.type == "FLOAT" and getattr(prop, "array_length", 0) in (3, 4):
            role = classify(full)
            if role is None:
                continue
            rgb = RGB[role]
            current = list(val)
            current[0], current[1], current[2] = rgb
            setattr(obj, prop.identifier, current)
            applied += 1
    return applied


theme = bpy.context.preferences.themes[0]
count = walk_and_apply(theme)
print(f"ptm-theme: applied {count} properties")
bpy.ops.wm.save_userpref()
