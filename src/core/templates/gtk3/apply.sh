#!/usr/bin/env bash
set -euo pipefail

GTK_IMPORT='@import url("ptm.css");'

theme_exists() {
    local name="$1"
    local -a paths=(
        "$HOME/.themes"
        "${XDG_DATA_HOME:-$HOME/.local/share}/themes"
        /usr/share/themes
        /usr/local/share/themes
    )
    if [ -n "${XDG_DATA_DIRS:-}" ]; then
        IFS=':' read -ra xdg_dirs <<< "$XDG_DATA_DIRS"
        for dir in "${xdg_dirs[@]}"; do
            [ -n "$dir" ] && paths+=("$dir/themes")
        done
    fi
    for base in "${paths[@]}"; do
        [ -d "$base/$name" ] && return 0
    done
    return 1
}

ensure_gtk_css_import() {
    local gtk_css="$1" colors_file="$2" label="$3"

    if [ ! -f "$colors_file" ]; then
        return 0
    fi

    if [ -e "$gtk_css" ] || [ -L "$gtk_css" ]; then
        local content
        content=$(cat "$gtk_css")
        if [[ "$content" == *"ptm.css"* ]] && [[ "$content" == *"@import"* ]]; then
            return 0
        fi
        local target="$gtk_css"
        if [ -L "$gtk_css" ]; then
            local resolved
            resolved=$(readlink -f "$gtk_css")
            if [ -w "$resolved" ]; then
                target="$resolved"
            else
                rm "$gtk_css"
            fi
        fi
        printf '%s\n\n%s\n' "$content" "$GTK_IMPORT" > "$target"
        echo "Appended $label ptm.css import to $(basename "$gtk_css")"
    else
        printf '%s\n' "$GTK_IMPORT" > "$gtk_css"
        echo "Created $label $(basename "$gtk_css") with ptm.css import"
    fi
}

sync_gtk_theme() {
    local mode="$1"
    local current_theme=""

    if command -v gsettings >/dev/null \
            && [[ "$(gsettings list-schemas 2>/dev/null || true)" == *"org.gnome.desktop.interface"* ]]; then
        current_theme=$(gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null | tr -d "'" || true)
    fi

    # Sync GNOME portal color-scheme to prefer-dark or prefer-light matching active mode
    local target_cs="prefer-dark"
    [ "$mode" = "light" ] && target_cs="prefer-light"

    if command -v gsettings >/dev/null \
            && [[ "$(gsettings list-schemas 2>/dev/null || true)" == *"org.gnome.desktop.interface"* ]]; then
        gsettings set org.gnome.desktop.interface color-scheme "$target_cs" 2>/dev/null || true
    elif command -v dconf >/dev/null; then
        dconf write /org/gnome/desktop/interface/color-scheme "'$target_cs'" 2>/dev/null || true
    fi

    local target_theme
    [ "$mode" = "light" ] && target_theme="adw-gtk3" || target_theme="adw-gtk3-dark"

    if ! theme_exists "$target_theme"; then
        return 0
    fi

    # Only sync gtk-theme variant if current theme is an adw theme or empty
    if [[ "$current_theme" == *"adw"* ]] || [ -z "$current_theme" ]; then
        if command -v gsettings >/dev/null \
                && [[ "$(gsettings list-schemas 2>/dev/null || true)" == *"org.gnome.desktop.interface"* ]]; then
            gsettings set org.gnome.desktop.interface gtk-theme "$target_theme" 2>/dev/null || true
        elif command -v dconf >/dev/null; then
            dconf write /org/gnome/desktop/interface/gtk-theme "'$target_theme'" 2>/dev/null || true
        fi
    fi
}

main() {
    local mode="${1:-dark}"
    if [ "$mode" != "dark" ] && [ "$mode" != "light" ]; then
        mode="dark"
    fi

    local config_dir="${XDG_CONFIG_HOME:-$HOME/.config}"
    mkdir -p "$config_dir/gtk-3.0" "$config_dir/gtk-4.0"

    ensure_gtk_css_import "$config_dir/gtk-3.0/gtk.css" "$config_dir/gtk-3.0/ptm.css" "GTK3"
    ensure_gtk_css_import "$config_dir/gtk-4.0/gtk.css" "$config_dir/gtk-4.0/ptm.css" "GTK4"

    sync_gtk_theme "$mode"
}

main "$@"
