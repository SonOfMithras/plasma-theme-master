/* GTK4 & Libadwaita Color Overrides */

@define-color accent_color {{ACCENT}};
@define-color accent_bg_color {{ACCENT}};
@define-color accent_fg_color {{WINDOW_BG}};

@define-color window_bg_color {{WINDOW_BG}};
@define-color window_fg_color {{WINDOW_FG}};

@define-color view_bg_color {{VIEW_BG}};
@define-color view_fg_color {{VIEW_FG}};

@define-color headerbar_bg_color {{TITLEBAR_BG}};
@define-color headerbar_fg_color {{TITLEBAR_FG}};
@define-color headerbar_border_color {{TITLEBAR_BG}};
@define-color headerbar_backdrop_color {{TITLEBAR_BG}};
@define-color headerbar_shade_color rgba(0, 0, 0, 0.07);

@define-color card_bg_color {{BUTTON_BG}};
@define-color card_fg_color {{BUTTON_FG}};
@define-color card_border_color {{BUTTON_BG}};

@define-color popover_bg_color {{VIEW_BG}};
@define-color popover_fg_color {{VIEW_FG}};

@define-color sidebar_bg_color {{WINDOW_BG_DARK}};
@define-color sidebar_fg_color {{WINDOW_FG}};
@define-color sidebar_backdrop_color {{WINDOW_BG_DARK}};
@define-color sidebar_shade_color rgba(0, 0, 0, 0.07);

@define-color dialog_bg_color {{WINDOW_BG}};
@define-color dialog_fg_color {{WINDOW_FG}};

@define-color scrollbar_outline_color {{WINDOW_BG}};

@define-color success_color {{SUCCESS}};
@define-color success_bg_color {{SUCCESS}};
@define-color success_fg_color {{WINDOW_BG}};

@define-color warning_color {{WARNING}};
@define-color warning_bg_color {{WARNING}};
@define-color warning_fg_color {{WINDOW_BG}};

@define-color error_color {{ERROR}};
@define-color error_bg_color {{ERROR}};
@define-color error_fg_color {{WINDOW_BG}};

@define-color destructive_color {{ERROR}};
@define-color destructive_bg_color {{ERROR}};
@define-color destructive_fg_color {{WINDOW_BG}};

/* Classic GTK theme variables for compatibility */
@define-color theme_bg_color {{WINDOW_BG}};
@define-color theme_fg_color {{WINDOW_FG}};
@define-color theme_base_color {{VIEW_BG}};
@define-color theme_text_color {{VIEW_FG}};
@define-color theme_selected_bg_color {{ACCENT}};
@define-color theme_selected_fg_color {{WINDOW_BG}};

@define-color theme_unfocused_bg_color {{WINDOW_BG}};
@define-color theme_unfocused_fg_color {{WINDOW_FG}};
@define-color theme_unfocused_base_color {{VIEW_BG}};
@define-color theme_unfocused_text_color {{VIEW_FG}};
@define-color theme_unfocused_selected_bg_color {{ACCENT}};
@define-color theme_unfocused_selected_fg_color {{WINDOW_BG}};

@define-color insensitive_bg_color {{WINDOW_BG_DARK}};
@define-color insensitive_fg_color {{VIEW_FG_MUTED}};
@define-color insensitive_base_color {{VIEW_BG}};
@define-color insensitive_base_fg_color {{VIEW_FG_MUTED}};
@define-color insensitive_selected_bg_color {{WINDOW_BG_DARK}};
@define-color insensitive_selected_fg_color {{VIEW_FG_MUTED}};

@define-color borders {{WINDOW_BG_DARK}};
@define-color unfocused_borders {{WINDOW_BG_DARK}};

/* CSS Variable Custom Properties */
:root {
  --accent-color: {{ACCENT}};
  --accent-bg-color: {{ACCENT}};
  --accent-fg-color: {{WINDOW_BG}};
  --window-bg-color: {{WINDOW_BG}};
  --window-fg-color: {{WINDOW_FG}};
  --view-bg-color: {{VIEW_BG}};
  --view-fg-color: {{VIEW_FG}};
  --headerbar-bg-color: {{TITLEBAR_BG}};
  --headerbar-fg-color: {{TITLEBAR_FG}};
  --card-bg-color: {{BUTTON_BG}};
  --card-fg-color: {{BUTTON_FG}};
  --popover-bg-color: {{VIEW_BG}};
  --popover-fg-color: {{VIEW_FG}};
  --dialog-bg-color: {{WINDOW_BG}};
  --dialog-fg-color: {{WINDOW_FG}};
  --sidebar-bg-color: {{WINDOW_BG_DARK}};
  --sidebar-fg-color: {{WINDOW_FG}};
}
