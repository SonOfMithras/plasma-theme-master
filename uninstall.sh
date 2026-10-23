#!/bin/bash
# =============================================================================
# plasma-theme-master uninstaller
# Removes system binaries, service files, system data, and optionally user
# configurations and generated third-party application templates.
# =============================================================================

set -uo pipefail

APP_NAME="plasma-theme-master"

# Dynamic path discovery
BINARY_PATH=$(command -v $APP_NAME 2>/dev/null || echo "/usr/bin/$APP_NAME")

# Derive the installation prefix (e.g., /usr/bin/app -> /usr)
PREFIX=$(dirname "$(dirname "$BINARY_PATH")")

# Handle cases where the binary might be in /bin instead of /usr/bin
if [ "$PREFIX" = "/" ]; then
    PREFIX=""
fi

DESKTOP_FILE="$PREFIX/share/applications/$APP_NAME.desktop"
SERVICE_FILE="$HOME/.config/systemd/user/$APP_NAME.service"
CONFIG_FILE="$HOME/.config/plasma-theme-masterrc"
TOML_CONFIG="$HOME/.config/plasma-theme-master/config.toml"
DATA_DIR="$HOME/.local/share/plasma-theme-master"
SYSTEM_DATA_DIR="$PREFIX/share/plasma-theme-master"

# Helper binaries installed alongside the main binary
HELPERS=(
    "$PREFIX/bin/plasma-theme-master-helper-vscode"
    "$PREFIX/bin/plasma-theme-master-helper-zed"
    "$PREFIX/bin/plasma-theme-master-helper-firefox"
    "$PREFIX/bin/plasma-theme-master-helper-konsole"
    "$PREFIX/bin/plasma-theme-master-helper-kitty"
    "$PREFIX/bin/plasma-theme-master-helper-gtk4-reload"
    "$PREFIX/bin/plasma-theme-master-clean-templates"
    "$PREFIX/bin/plasma-theme-master-uninstall"
)

AUTO_YES=false
CLEAN_TEMPLATES=""
REMOVE_CONFIG=""

for arg in "$@"; do
    case "$arg" in
        -y|--yes)
            AUTO_YES=true
            ;;
        --all)
            AUTO_YES=true
            CLEAN_TEMPLATES=true
            REMOVE_CONFIG=true
            ;;
        --clean-templates)
            CLEAN_TEMPLATES=true
            ;;
        --keep-templates)
            CLEAN_TEMPLATES=false
            ;;
        --remove-config)
            REMOVE_CONFIG=true
            ;;
        --keep-config)
            REMOVE_CONFIG=false
            ;;
        -h|--help)
            echo "Usage: $(basename "$0") [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -y, --yes            Assume yes to prompts (cleans templates, keeps user config unless --all)"
            echo "  --all                Clean templates and remove all configuration/logs"
            echo "  --clean-templates    Remove generated templates without prompting"
            echo "  --keep-templates     Keep generated templates without prompting"
            echo "  --remove-config      Remove user configuration without prompting"
            echo "  --keep-config        Keep user configuration without prompting"
            echo "  -h, --help           Show this help message"
            exit 0
            ;;
    esac
done

clean_templates_func() {
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

    if [ -x "$script_dir/clean-templates.sh" ]; then
        "$script_dir/clean-templates.sh"
    elif command -v plasma-theme-master-clean-templates >/dev/null 2>&1; then
        plasma-theme-master-clean-templates
    elif [ -x "$PREFIX/bin/plasma-theme-master-clean-templates" ]; then
        "$PREFIX/bin/plasma-theme-master-clean-templates"
    else
        echo "Running built-in template cleanup..."
        remove_f() { [ -f "$1" ] || [ -L "$1" ] && rm -f "$1" && echo "  ✔ Removed: $1"; }
        restore_b() { [ -f "$2" ] && cp -f "$2" "$1" && rm -f "$2" && echo "  ✔ Restored backup: $1"; }

        # 1. Kitty
        remove_f "$HOME/.config/kitty/themes/ptm.conf"
        remove_f "$HOME/.config/kitty/themes/noctalia.conf"
        remove_f "$HOME/.config/kitty/dark-theme.auto.conf"
        remove_f "$HOME/.config/kitty/light-theme.auto.conf"
        [ -f "$HOME/.config/kitty/kitty.conf" ] && sed -i '/include.*ptm\.conf/d; /include.*noctalia\.conf/d' "$HOME/.config/kitty/kitty.conf"
        restore_b "$HOME/.config/kitty/kitty.conf" "$HOME/.config/kitty/kitty.conf.ptm.bak"
        pkill -USR1 -x kitty 2>/dev/null || true

        # 2. Alacritty
        remove_f "$HOME/.config/alacritty/themes/ptm.toml"
        remove_f "$HOME/.config/alacritty/themes/noctalia.toml"
        remove_f "$HOME/.config/alacritty/ptm.toml"
        [ -f "$HOME/.config/alacritty/alacritty.toml" ] && sed -i '/ptm\.toml/d; /noctalia\.toml/d' "$HOME/.config/alacritty/alacritty.toml"
        restore_b "$HOME/.config/alacritty/alacritty.toml" "$HOME/.config/alacritty/alacritty.toml.ptm.bak"

        # 3. Foot
        remove_f "$HOME/.config/foot/themes/ptm"
        remove_f "$HOME/.config/foot/themes/ptm.ini"
        remove_f "$HOME/.config/foot/themes/noctalia"
        remove_f "$HOME/.config/foot/themes/noctalia.ini"
        remove_f "$HOME/.config/foot/ptm.ini"
        [ -f "$HOME/.config/foot/foot.ini" ] && sed -i '/include=.*themes\/ptm/d; /include=.*themes\/noctalia/d; /include=.*ptm\.ini/d; /include=.*noctalia\.ini/d' "$HOME/.config/foot/foot.ini"
        restore_b "$HOME/.config/foot/foot.ini" "$HOME/.config/foot/foot.ini.ptm.bak"

        # 4. Ghostty
        remove_f "$HOME/.config/ghostty/themes/ptm"
        remove_f "$HOME/.config/ghostty/themes/noctalia"
        [ -f "$HOME/.config/ghostty/config" ] && sed -i '/theme = ptm/d; /theme = noctalia/d' "$HOME/.config/ghostty/config"
        restore_b "$HOME/.config/ghostty/config" "$HOME/.config/ghostty/config.ptm.bak"

        # 5. Btop & Cava
        remove_f "$HOME/.config/btop/themes/ptm.theme"
        remove_f "$HOME/.config/btop/themes/noctalia.theme"
        remove_f "$HOME/.config/btop/themes/plasma-theme-master.theme"
        restore_b "$HOME/.config/btop/btop.conf" "$HOME/.config/btop/btop.conf.ptm.bak"
        remove_f "$HOME/.config/cava/themes/ptm"
        remove_f "$HOME/.config/cava/themes/noctalia"
        restore_b "$HOME/.config/cava/config" "$HOME/.config/cava/config.ptm.bak"

        # 6. Fastfetch & Fzf
        remove_f "$HOME/.config/fastfetch/ptm.jsonc"
        remove_f "$HOME/.config/fastfetch/noctalia.jsonc"
        remove_f "$HOME/.config/fzf/ptm.sh"
        remove_f "$HOME/.config/fzf/noctalia.sh"

        # 7. Starship
        remove_f "$HOME/.config/starship/ptm-palette.toml"
        remove_f "$HOME/.config/starship/ptm.toml"
        remove_f "$HOME/.config/starship/noctalia.toml"
        [ -f "$HOME/.config/starship.toml" ] && sed -i '/palette = "ptm"/d; /palette = "noctalia"/d' "$HOME/.config/starship.toml"
        restore_b "$HOME/.config/starship.toml" "$HOME/.config/starship.toml.ptm.bak"

        # 8. Helix & Wezterm & Zed & Bat
        remove_f "$HOME/.config/helix/themes/ptm.toml"
        remove_f "$HOME/.config/helix/themes/noctalia.toml"
        restore_b "$HOME/.config/helix/config.toml" "$HOME/.config/helix/config.toml.ptm.bak"
        remove_f "$HOME/.config/wezterm/colors/PTM.toml"
        remove_f "$HOME/.config/wezterm/colors/ptm.toml"
        remove_f "$HOME/.config/wezterm/colors/noctalia.toml"
        remove_f "$HOME/.config/zed/themes/plasma-master.json"
        remove_f "$HOME/.config/zed/themes/ptm.json"
        remove_f "$HOME/.config/zed/themes/noctalia.json"
        remove_f "$HOME/.config/bat/themes/ptm.tmTheme"
        remove_f "$HOME/.config/bat/themes/noctalia.tmTheme"
        command -v bat >/dev/null 2>&1 && bat cache --build >/dev/null 2>&1 || true

        # 9. Zellij & Yazi & Zathura & Superfile & Tmux & Micro & Hyprtoolkit
        remove_f "$HOME/.config/zellij/themes/ptm.kdl"
        remove_f "$HOME/.config/zellij/themes/noctalia.kdl"
        [ -d "$HOME/.config/yazi/flavors/ptm.yazi" ] && rm -rf "$HOME/.config/yazi/flavors/ptm.yazi"
        [ -d "$HOME/.config/yazi/flavors/noctalia.yazi" ] && rm -rf "$HOME/.config/yazi/flavors/noctalia.yazi"
        remove_f "$HOME/.config/zathura/ptmrc"
        remove_f "$HOME/.var/app/org.pwmt.zathura/config/zathura/ptmrc"
        remove_f "$HOME/.config/superfile/theme/ptm.toml"
        remove_f "$HOME/.config/tmux/themes/ptm.conf"
        remove_f "$HOME/.config/micro/colorschemes/ptm.micro"
        remove_f "$HOME/.config/hypr/hyprtoolkit.conf"

        # 10. Vicinae & Tauon & SpeedCrunch
        remove_f "$HOME/.local/share/vicinae/themes/ptm.toml"
        remove_f "$HOME/.local/share/TauonMusicBox/theme/PTM.ttheme"
        remove_f "$HOME/.local/share/SpeedCrunch/color-schemes/PTM.json"

        # 11. VSCode / VSCodium / Antigravity IDE
        rm -rf "$HOME/.vscode/extensions/ptm.ptmtheme"* 2>/dev/null || true
        rm -rf "$HOME/.vscode/extensions/noctalia"* 2>/dev/null || true
        rm -rf "$HOME/.vscode-oss/extensions/ptm.ptmtheme"* 2>/dev/null || true
        rm -rf "$HOME/.vscode-oss/extensions/noctalia"* 2>/dev/null || true
        rm -rf "$HOME/.antigravity-ide/extensions/ptm.ptmtheme"* 2>/dev/null || true
        restore_b "$HOME/.config/Code/User/settings.json" "$HOME/.config/Code/User/settings.json.ptm.bak"
        restore_b "$HOME/.config/VSCodium/User/settings.json" "$HOME/.config/VSCodium/User/settings.json.ptm.bak"
        restore_b "$HOME/.config/Antigravity IDE/User/settings.json" "$HOME/.config/Antigravity IDE/User/settings.json.ptm.bak"

        # 12. Obsidian snippets
        local obsidian_conf="$HOME/.config/obsidian/obsidian.json"
        if [ -f "$obsidian_conf" ]; then
            grep -o '"path": *"[^"]*"' "$obsidian_conf" | cut -d'"' -f4 | while read -r vault; do
                remove_f "$vault/.obsidian/snippets/ptm.css"
                remove_f "$vault/.obsidian/snippets/noctalia.css"
            done
        fi

        # 13. Zen Browser & Firefox
        rm -f "$HOME/.zen/"*"/chrome/zen-userChrome.css" 2>/dev/null || true
        rm -f "$HOME/.zen/"*"/chrome/zen-userContent.css" 2>/dev/null || true
        rm -f "$HOME/.mozilla/firefox/"*"/chrome/ptm-userChrome.css" 2>/dev/null || true
        rm -rf "$HOME/.config/plasma-theme-master/zen-browser" 2>/dev/null || true

        # 14. Plasma color schemes
        remove_f "$HOME/.local/share/color-schemes/ptm.colors"
        remove_f "$HOME/.local/share/color-schemes/noctalia.colors"

        # 14b. GTK 3 & 4 Theme Sync
        remove_f "$HOME/.config/gtk-3.0/ptm.css"
        [ -f "$HOME/.config/gtk-3.0/gtk.css" ] && sed -i '/@import.*ptm\.css/d' "$HOME/.config/gtk-3.0/gtk.css"
        remove_f "$HOME/.config/gtk-4.0/ptm.css"
        [ -f "$HOME/.config/gtk-4.0/gtk.css" ] && sed -i '/@import.*ptm\.css/d' "$HOME/.config/gtk-4.0/gtk.css"

        # 15. PTM Template Cache
        [ -d "$HOME/.cache/ptm" ] && rm -rf "$HOME/.cache/ptm" && echo "  ✔ Removed cache: $HOME/.cache/ptm"

        # 16. Scan & restore any remaining .ptm.bak or .noctalia.bak in ~/.config and ~/.local
        find "$HOME/.config" "$HOME/.local" -maxdepth 4 -type f \( -name "*.ptm.bak" -o -name "*.noctalia.bak" \) 2>/dev/null | while read -r bak; do
            local orig="${bak%.ptm.bak}"
            orig="${orig%.noctalia.bak}"
            restore_b "$orig" "$bak"
        done
        echo "  ✔ Template cleanup complete."
    fi
}

echo "=== Uninstalling $APP_NAME ==="

# 1. Stop and disable service
echo "Stopping service..."
systemctl --user disable --now $APP_NAME.service 2>/dev/null || true
if [ -f "$SERVICE_FILE" ]; then
    rm -f "$SERVICE_FILE"
    systemctl --user daemon-reload 2>/dev/null || true
    echo "Service removed."
fi

# 2. Clean up generated application templates and restore original configs
echo ""
if [ -z "$CLEAN_TEMPLATES" ]; then
    if [ "$AUTO_YES" = true ]; then
        CLEAN_TEMPLATES=true
    else
        read -p "Do you want to clean up generated application templates and restore original configs? [Y/n] " tmpl_response
        tmpl_response=${tmpl_response:-Y}
        if [[ "$tmpl_response" =~ ^[Yy]$ ]]; then
            CLEAN_TEMPLATES=true
        else
            CLEAN_TEMPLATES=false
        fi
    fi
fi

if [ "$CLEAN_TEMPLATES" = true ]; then
    echo "Cleaning up application templates and restoring backups..."
    clean_templates_func
else
    echo "Application templates retained."
fi

# 3. Prompt for user config/data
echo ""
if [ -z "$REMOVE_CONFIG" ]; then
    if [ "$AUTO_YES" = true ]; then
        REMOVE_CONFIG=false
    else
        read -p "Do you want to remove user configuration and logs? [y/N] " cfg_response
        if [[ "$cfg_response" =~ ^[Yy]$ ]]; then
            REMOVE_CONFIG=true
        else
            REMOVE_CONFIG=false
        fi
    fi
fi

if [ "$REMOVE_CONFIG" = true ]; then
    if [ -f "$CONFIG_FILE" ]; then
        rm -f "$CONFIG_FILE"
        echo "Removed: $CONFIG_FILE"
    fi
    if [ -f "$TOML_CONFIG" ]; then
        rm -f "$TOML_CONFIG"
        rmdir --ignore-fail-on-non-empty "$HOME/.config/plasma-theme-master" 2>/dev/null || true
        echo "Removed: $TOML_CONFIG"
    fi
    if [ -d "$DATA_DIR" ]; then
        rm -rf "$DATA_DIR"
        echo "Removed data directory: $DATA_DIR"
    fi
else
    echo "User configuration retained."
    if [ -f "$TOML_CONFIG" ]; then
        echo "  config.toml preserved at: $TOML_CONFIG"
    fi
fi

# 4. Remove system files (requires sudo)
echo ""
echo "Removing system files (requires sudo)..."
if [ -f "$BINARY_PATH" ]; then
    sudo rm -f "$BINARY_PATH"
    echo "Removed binary: $BINARY_PATH"
else
    echo "Binary not found at $BINARY_PATH"
fi

if [ -f "$DESKTOP_FILE" ]; then
    sudo rm -f "$DESKTOP_FILE"
    echo "Removed desktop file."
else
    echo "Desktop file not found at $DESKTOP_FILE"
fi

# 5. Remove helper binaries
echo "Removing helper binaries..."
for helper in "${HELPERS[@]}"; do
    if [ -f "$helper" ]; then
        sudo rm -f "$helper"
        echo "  Removed: $helper"
    fi
done

# 6. Remove system data directory (templates, default config, palettes)
if [ -d "$SYSTEM_DATA_DIR" ]; then
    sudo rm -rf "$SYSTEM_DATA_DIR"
    echo "Removed system data: $SYSTEM_DATA_DIR"
fi

echo ""
echo "=== Uninstallation Complete ==="
