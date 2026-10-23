fzf_theme_opts="\
--color=bg+:{{colors.surface_variant.default.hex}}
--color=bg:{{colors.surface.default.hex}}
--color=spinner:{{colors.terminal_cursor.default.hex}}
--color=hl:{{colors.error.default.hex}}
--color=fg:{{colors.terminal_foreground.default.hex}}
--color=header:{{colors.error.default.hex}}
--color=info:{{colors.primary.default.hex}}
--color=pointer:{{colors.terminal_cursor.default.hex}}
--color=marker:{{colors.on_surface_variant.default.hex}}
--color=fg+:{{colors.on_surface.default.hex}}
--color=prompt:{{colors.primary.default.hex}}
--color=hl+:{{colors.error.default.hex}}
--color=selected-bg:{{colors.terminal_normal_black.default.hex}}
--color=border:{{colors.terminal_selection_bg.default.hex}}
--color=label:{{colors.on_surface.default.hex}}"

export FZF_DEFAULT_OPTS="${FZF_DEFAULT_OPTS:+$FZF_DEFAULT_OPTS
}$fzf_theme_opts"
