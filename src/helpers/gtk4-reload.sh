#!/usr/bin/env bash
# gtk4-reload.sh
# Forces a live refresh of GTK4 themes by toggling GNOME desktop color-scheme via gsettings.

LOCKFILE="/tmp/ptm_gtk4_reload.lock"

# Rate limit: do not run more often than once every 4 seconds
if [ -f "$LOCKFILE" ]; then
    last_run=$(stat -c %Y "$LOCKFILE" 2>/dev/null || echo 0)
    now=$(date +%s)
    if [ $((now - last_run)) -lt 4 ]; then
        exit 0
    fi
fi
touch "$LOCKFILE" 2>/dev/null || true

sleep 0.4

sanitize_gtk_colors() {
    local css="$1"
    [ -f "$css" ] || return 0
    sed -i \
        -e 's/@define-color theme_unfocused_bg_color_breeze .*/@define-color theme_unfocused_bg_color_breeze @theme_bg_color_breeze;/' \
        -e 's/@define-color theme_unfocused_base_color_breeze .*/@define-color theme_unfocused_base_color_breeze @theme_base_color_breeze;/' \
        -e 's/@define-color theme_unfocused_fg_color_breeze .*/@define-color theme_unfocused_fg_color_breeze @theme_fg_color_breeze;/' \
        -e 's/@define-color theme_unfocused_text_color_breeze .*/@define-color theme_unfocused_text_color_breeze @theme_text_color_breeze;/' \
        -e 's/@define-color theme_unfocused_selected_bg_color_breeze .*/@define-color theme_unfocused_selected_bg_color_breeze @theme_selected_bg_color_breeze;/' \
        -e 's/@define-color theme_unfocused_selected_bg_color_alt_breeze .*/@define-color theme_unfocused_selected_bg_color_alt_breeze @theme_selected_bg_color_breeze;/' \
        -e 's/@define-color theme_unfocused_selected_fg_color_breeze .*/@define-color theme_unfocused_selected_fg_color_breeze @theme_selected_fg_color_breeze;/' \
        -e 's/@define-color theme_unfocused_view_bg_color_breeze .*/@define-color theme_unfocused_view_bg_color_breeze @content_view_bg_breeze;/' \
        -e 's/@define-color theme_unfocused_view_text_color_breeze .*/@define-color theme_unfocused_view_text_color_breeze @theme_text_color_breeze;/' \
        -e 's/@define-color theme_header_background_backdrop_breeze .*/@define-color theme_header_background_backdrop_breeze @theme_header_background_breeze;/' \
        -e 's/@define-color theme_header_foreground_backdrop_breeze .*/@define-color theme_header_foreground_backdrop_breeze @theme_header_foreground_breeze;/' \
        -e 's/@define-color theme_header_foreground_insensitive_backdrop_breeze .*/@define-color theme_header_foreground_insensitive_backdrop_breeze @theme_header_foreground_insensitive_breeze;/' \
        -e 's/@define-color theme_button_background_backdrop_breeze .*/@define-color theme_button_background_backdrop_breeze @theme_button_background_normal_breeze;/' \
        -e 's/@define-color theme_button_foreground_backdrop_breeze .*/@define-color theme_button_foreground_backdrop_breeze @theme_button_foreground_normal_breeze;/' \
        -e 's/@define-color theme_button_foreground_active_backdrop_breeze .*/@define-color theme_button_foreground_active_backdrop_breeze @theme_button_foreground_active_breeze;/' \
        -e 's/@define-color theme_button_decoration_focus_backdrop_breeze .*/@define-color theme_button_decoration_focus_backdrop_breeze @theme_button_decoration_focus_breeze;/' \
        -e 's/@define-color theme_button_decoration_hover_backdrop_breeze .*/@define-color theme_button_decoration_hover_backdrop_breeze @theme_button_decoration_hover_breeze;/' \
        -e 's/@define-color theme_titlebar_background_backdrop_breeze .*/@define-color theme_titlebar_background_backdrop_breeze @theme_titlebar_background_breeze;/' \
        -e 's/@define-color theme_titlebar_foreground_backdrop_breeze .*/@define-color theme_titlebar_foreground_backdrop_breeze @theme_titlebar_foreground_breeze;/' \
        -e 's/@define-color theme_titlebar_foreground_insensitive_backdrop_breeze .*/@define-color theme_titlebar_foreground_insensitive_backdrop_breeze @theme_titlebar_foreground_insensitive_breeze;/' \
        -e 's/@define-color unfocused_borders_breeze .*/@define-color unfocused_borders_breeze @borders_breeze;/' \
        -e 's/@define-color error_color_backdrop_breeze .*/@define-color error_color_backdrop_breeze @error_color_breeze;/' \
        -e 's/@define-color success_color_backdrop_breeze .*/@define-color success_color_backdrop_breeze @success_color_breeze;/' \
        -e 's/@define-color warning_color_backdrop_breeze .*/@define-color warning_color_backdrop_breeze @warning_color_breeze;/' \
        "$css"
}

sanitize_gtk_colors "$HOME/.config/gtk-4.0/colors.css"
sanitize_gtk_colors "$HOME/.config/gtk-3.0/colors.css"

mode="${1:-}"
if [ -z "$mode" ]; then
    gtk_theme=$(gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null | tr -d "'" || echo "")
    if [[ "$gtk_theme" =~ [Dd]ark ]]; then
        mode="dark"
    else
        mode="light"
    fi
fi

target_cs="prefer-dark"
[ "$mode" = "light" ] && target_cs="prefer-light"

if command -v gsettings >/dev/null 2>&1; then
    gsettings set org.gnome.desktop.interface color-scheme "$target_cs" 2>/dev/null || true
elif command -v dconf >/dev/null 2>&1; then
    dconf write /org/gnome/desktop/interface/color-scheme "'$target_cs'" 2>/dev/null || true
fi
