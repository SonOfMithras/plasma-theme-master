#!/usr/bin/env bash
# =============================================================================
# clean-templates.sh
# Removes all generated application template files, themes, and snippet links
# created by Plasma Theme Master (PTM) and Noctalia across the system,
# restoring any original backup files (*.ptm.bak / *.noctalia.bak).
# =============================================================================
set -uo pipefail

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}=== Cleaning Plasma Theme Master & Noctalia Application Templates ===${NC}"

remove_file() {
    local f="$1"
    if [ -f "$f" ] || [ -L "$f" ]; then
        rm -f "$f"
        echo -e "${GREEN}✔ Removed:${NC} $f"
    fi
}

remove_dir_if_empty() {
    local d="$1"
    if [ -d "$d" ]; then
        rmdir --ignore-fail-on-non-empty "$d" 2>/dev/null || true
    fi
}

restore_backup() {
    local orig="$1"
    local bak="$2"
    if [ -f "$bak" ]; then
        cp -f "$bak" "$orig"
        rm -f "$bak"
        echo -e "${GREEN}✔ Restored backup:${NC} $orig (from $(basename "$bak"))"
    fi
}

# 1. Kitty
echo -e "\n${YELLOW}Checking Kitty...${NC}"
remove_file "$HOME/.config/kitty/themes/ptm.conf"
remove_file "$HOME/.config/kitty/themes/noctalia.conf"
remove_file "$HOME/.config/kitty/dark-theme.auto.conf"
remove_file "$HOME/.config/kitty/light-theme.auto.conf"
remove_file "$HOME/.config/kitty/current-theme.conf"
remove_file "$HOME/.config/kitty/no-preference-theme.auto.conf"
if [ -f "$HOME/.config/kitty/kitty.conf" ]; then
    sed -i '/include.*ptm\.conf/d' "$HOME/.config/kitty/kitty.conf"
    sed -i '/include.*noctalia\.conf/d' "$HOME/.config/kitty/kitty.conf"
    sed -i '/include.*current-theme\.conf/d' "$HOME/.config/kitty/kitty.conf"
    sed -i '/include.*plasma-colors\.conf/d' "$HOME/.config/kitty/kitty.conf"
    echo -e "${GREEN}✔ Cleaned:${NC} $HOME/.config/kitty/kitty.conf include lines"
fi
restore_backup "$HOME/.config/kitty/kitty.conf" "$HOME/.config/kitty/kitty.conf.ptm.bak"
pkill -USR1 -x kitty 2>/dev/null || true

# 2. Alacritty
echo -e "\n${YELLOW}Checking Alacritty...${NC}"
remove_file "$HOME/.config/alacritty/themes/ptm.toml"
remove_file "$HOME/.config/alacritty/themes/noctalia.toml"
remove_file "$HOME/.config/alacritty/ptm.toml"
if [ -f "$HOME/.config/alacritty/alacritty.toml" ]; then
    sed -i '/ptm\.toml/d' "$HOME/.config/alacritty/alacritty.toml"
    sed -i '/noctalia\.toml/d' "$HOME/.config/alacritty/alacritty.toml"
fi
restore_backup "$HOME/.config/alacritty/alacritty.toml" "$HOME/.config/alacritty/alacritty.toml.ptm.bak"

# 3. Foot
echo -e "\n${YELLOW}Checking Foot...${NC}"
remove_file "$HOME/.config/foot/themes/ptm"
remove_file "$HOME/.config/foot/themes/ptm.ini"
remove_file "$HOME/.config/foot/themes/noctalia"
remove_file "$HOME/.config/foot/themes/noctalia.ini"
remove_file "$HOME/.config/foot/ptm.ini"
if [ -f "$HOME/.config/foot/foot.ini" ]; then
    sed -i '/include=.*themes\/ptm/d' "$HOME/.config/foot/foot.ini"
    sed -i '/include=.*themes\/noctalia/d' "$HOME/.config/foot/foot.ini"
    sed -i '/include=.*ptm\.ini/d' "$HOME/.config/foot/foot.ini"
    sed -i '/include=.*noctalia\.ini/d' "$HOME/.config/foot/foot.ini"
fi
restore_backup "$HOME/.config/foot/foot.ini" "$HOME/.config/foot/foot.ini.ptm.bak"

# 4. Ghostty
echo -e "\n${YELLOW}Checking Ghostty...${NC}"
remove_file "$HOME/.config/ghostty/themes/ptm"
remove_file "$HOME/.config/ghostty/themes/noctalia"
if [ -f "$HOME/.config/ghostty/config" ]; then
    sed -i '/theme = ptm/d' "$HOME/.config/ghostty/config"
    sed -i '/theme = noctalia/d' "$HOME/.config/ghostty/config"
fi
restore_backup "$HOME/.config/ghostty/config" "$HOME/.config/ghostty/config.ptm.bak"

# 5. Konsole
echo -e "\n${YELLOW}Checking Konsole...${NC}"
remove_file "${XDG_DATA_HOME:-$HOME/.local/share}/konsole/ptm.colorscheme"
remove_file "${XDG_DATA_HOME:-$HOME/.local/share}/konsole/ptm.colorscheme.general.bak"
remove_file "${XDG_DATA_HOME:-$HOME/.local/share}/konsole/noctalia.colorscheme"
remove_file "${XDG_DATA_HOME:-$HOME/.local/share}/konsole/PlasmaMaster.colorscheme"
remove_file "${XDG_DATA_HOME:-$HOME/.local/share}/konsole/PlasmaMaster.profile"
remove_file "${XDG_DATA_HOME:-$HOME/.local/share}/konsole/PTM.profile"

# 6. Btop
echo -e "\n${YELLOW}Checking Btop...${NC}"
remove_file "$HOME/.config/btop/themes/ptm.theme"
remove_file "$HOME/.config/btop/themes/noctalia.theme"
remove_file "$HOME/.config/btop/themes/plasma-theme-master.theme"
restore_backup "$HOME/.config/btop/btop.conf" "$HOME/.config/btop/btop.conf.ptm.bak"

# 6. Cava
echo -e "\n${YELLOW}Checking Cava...${NC}"
remove_file "$HOME/.config/cava/themes/ptm"
remove_file "$HOME/.config/cava/themes/noctalia"
restore_backup "$HOME/.config/cava/config" "$HOME/.config/cava/config.ptm.bak"

# 7. Fastfetch
echo -e "\n${YELLOW}Checking Fastfetch...${NC}"
remove_file "$HOME/.config/fastfetch/ptm.jsonc"
remove_file "$HOME/.config/fastfetch/noctalia.jsonc"

# 8. Fzf
echo -e "\n${YELLOW}Checking Fzf...${NC}"
remove_file "$HOME/.config/fzf/ptm.sh"
remove_file "$HOME/.config/fzf/noctalia.sh"

# 9. Starship
echo -e "\n${YELLOW}Checking Starship...${NC}"
remove_file "$HOME/.config/starship/ptm-palette.toml"
remove_file "$HOME/.config/starship/ptm.toml"
remove_file "$HOME/.config/starship/noctalia.toml"
if [ -f "$HOME/.config/starship.toml" ]; then
    sed -i '/palette = "ptm"/d' "$HOME/.config/starship.toml"
    sed -i '/palette = "noctalia"/d' "$HOME/.config/starship.toml"
fi
restore_backup "$HOME/.config/starship.toml" "$HOME/.config/starship.toml.ptm.bak"

# 10. Helix
echo -e "\n${YELLOW}Checking Helix...${NC}"
remove_file "$HOME/.config/helix/themes/ptm.toml"
remove_file "$HOME/.config/helix/themes/noctalia.toml"
restore_backup "$HOME/.config/helix/config.toml" "$HOME/.config/helix/config.toml.ptm.bak"

# 11. Wezterm
echo -e "\n${YELLOW}Checking Wezterm...${NC}"
remove_file "$HOME/.config/wezterm/colors/PTM.toml"
remove_file "$HOME/.config/wezterm/colors/ptm.toml"
remove_file "$HOME/.config/wezterm/colors/noctalia.toml"

# 12. Zed
echo -e "\n${YELLOW}Checking Zed...${NC}"
remove_file "$HOME/.config/zed/themes/plasma-master.json"
remove_file "$HOME/.config/zed/themes/ptm.json"
remove_file "$HOME/.config/zed/themes/noctalia.json"

# 13. Bat
echo -e "\n${YELLOW}Checking Bat...${NC}"
remove_file "$HOME/.config/bat/themes/ptm.tmTheme"
remove_file "$HOME/.config/bat/themes/noctalia.tmTheme"
if command -v bat >/dev/null 2>&1; then
    bat cache --build >/dev/null 2>&1 || true
fi

# 14. Zellij
echo -e "\n${YELLOW}Checking Zellij...${NC}"
remove_file "$HOME/.config/zellij/themes/ptm.kdl"
remove_file "$HOME/.config/zellij/themes/noctalia.kdl"

# 15. Yazi
echo -e "\n${YELLOW}Checking Yazi...${NC}"
if [ -d "$HOME/.config/yazi/flavors/ptm.yazi" ]; then
    rm -rf "$HOME/.config/yazi/flavors/ptm.yazi"
    echo -e "${GREEN}✔ Removed:${NC} $HOME/.config/yazi/flavors/ptm.yazi"
fi
if [ -d "$HOME/.config/yazi/flavors/noctalia.yazi" ]; then
    rm -rf "$HOME/.config/yazi/flavors/noctalia.yazi"
    echo -e "${GREEN}✔ Removed:${NC} $HOME/.config/yazi/flavors/noctalia.yazi"
fi

# 16. Zathura & Superfile & Tmux & Micro & Hyprtoolkit
echo -e "\n${YELLOW}Checking Zathura, Superfile, Tmux, Micro, Hyprtoolkit...${NC}"
remove_file "$HOME/.config/zathura/ptmrc"
remove_file "$HOME/.var/app/org.pwmt.zathura/config/zathura/ptmrc"
remove_file "$HOME/.config/superfile/theme/ptm.toml"
remove_file "$HOME/.config/tmux/themes/ptm.conf"
remove_file "$HOME/.config/micro/colorschemes/ptm.micro"
remove_file "$HOME/.config/hypr/hyprtoolkit.conf"

# 17. Vicinae & Tauon & SpeedCrunch
echo -e "\n${YELLOW}Checking Vicinae, Tauon, SpeedCrunch...${NC}"
remove_file "$HOME/.local/share/vicinae/themes/ptm.toml"
remove_file "$HOME/.local/share/vicinae/themes/plasma-theme-master.toml"
remove_file "$HOME/.local/share/TauonMusicBox/theme/PTM.ttheme"
remove_file "$HOME/.local/share/SpeedCrunch/color-schemes/PTM.json"

# 17b. Discord Clients (BetterDiscord, Vencord, Vesktop, etc.)
echo -e "\n${YELLOW}Checking Discord clients (BetterDiscord, Vencord, Vesktop)...${NC}"
for ddir in "$HOME/.config/BetterDiscord" "$HOME/.config/Vencord" "$HOME/.config/vesktop" "$HOME/.config/webcord" "$HOME/.config/legcord" "$HOME/.config/equibop" "$HOME/.config/Equicord" "$HOME/.var/app/dev.vencord.Vesktop/config/vesktop" "$HOME/.var/app/com.discordapp.Discord/config/Vencord"; do
    remove_file "$ddir/themes/PlasmaMaster.theme.css"
    remove_file "$ddir/themes/ptm.theme.css"
    remove_file "$ddir/themes/ptm-material.theme.css"
    remove_file "$ddir/themes/discord-system24.css"
    remove_file "$ddir/themes/noctalia.theme.css"
done

# 18. VSCode / VSCodium / Antigravity / Cursor / Windsurf extensions & settings
echo -e "\n${YELLOW}Checking VSCode & editors...${NC}"
rm -rf "$HOME/.vscode/extensions/ptm.ptmtheme"* 2>/dev/null || true
rm -rf "$HOME/.vscode/extensions/plasma-theme-master.plasma-theme-master"* 2>/dev/null || true
rm -rf "$HOME/.vscode/extensions/noctalia"* 2>/dev/null || true
rm -rf "$HOME/.vscode-oss/extensions/ptm.ptmtheme"* 2>/dev/null || true
rm -rf "$HOME/.vscode-oss/extensions/plasma-theme-master.plasma-theme-master"* 2>/dev/null || true
rm -rf "$HOME/.vscode-oss/extensions/noctalia"* 2>/dev/null || true
rm -rf "$HOME/.antigravity-ide/extensions/ptm.ptmtheme"* 2>/dev/null || true
rm -rf "$HOME/.antigravity-ide/extensions/plasma-theme-master.plasma-theme-master"* 2>/dev/null || true
rm -rf "$HOME/.cursor/extensions/ptm.ptmtheme"* 2>/dev/null || true
rm -rf "$HOME/.cursor/extensions/plasma-theme-master.plasma-theme-master"* 2>/dev/null || true
rm -rf "$HOME/.windsurf/extensions/ptm.ptmtheme"* 2>/dev/null || true
rm -rf "$HOME/.windsurf/extensions/plasma-theme-master.plasma-theme-master"* 2>/dev/null || true

restore_backup "$HOME/.config/Code/User/settings.json" "$HOME/.config/Code/User/settings.json.ptm.bak"
restore_backup "$HOME/.config/Code - OSS/User/settings.json" "$HOME/.config/Code - OSS/User/settings.json.ptm.bak"
restore_backup "$HOME/.config/VSCodium/User/settings.json" "$HOME/.config/VSCodium/User/settings.json.ptm.bak"
restore_backup "$HOME/.config/Antigravity IDE/User/settings.json" "$HOME/.config/Antigravity IDE/User/settings.json.ptm.bak"
restore_backup "$HOME/.config/Antigravity/User/settings.json" "$HOME/.config/Antigravity/User/settings.json.ptm.bak"
restore_backup "$HOME/.config/Cursor/User/settings.json" "$HOME/.config/Cursor/User/settings.json.ptm.bak"
restore_backup "$HOME/.config/Windsurf/User/settings.json" "$HOME/.config/Windsurf/User/settings.json.ptm.bak"

# 19. Obsidian Vaults
echo -e "\n${YELLOW}Checking Obsidian snippets...${NC}"
OBSIDIAN_CONF="$HOME/.config/obsidian/obsidian.json"
if [ -f "$OBSIDIAN_CONF" ]; then
    grep -o '"path": *"[^"]*"' "$OBSIDIAN_CONF" | cut -d'"' -f4 | while read -r vault; do
        if [ -d "$vault/.obsidian/snippets" ]; then
            remove_file "$vault/.obsidian/snippets/ptm.css"
            remove_file "$vault/.obsidian/snippets/noctalia.css"
        fi
    done
fi

# 20. Zen Browser & Firefox
echo -e "\n${YELLOW}Checking Zen Browser & Firefox...${NC}"
rm -rf "$HOME/.cache/ptm/zen-browser" 2>/dev/null || true
rm -rf "$HOME/.config/plasma-theme-master/zen-browser" 2>/dev/null || true
for zdir in "$HOME/.config/zen" "$HOME/.zen" "$HOME/.var/app/app.zen_browser.zen/.zen" "$HOME/.var/app/io.github.zen_browser.zen/.zen"; do
    if [ -d "$zdir" ]; then
        for uc in "$zdir"/*"/chrome/userChrome.css"; do
            [ -f "$uc" ] && sed -i -E '/(zen-browser\/(zen-)?userChrome\.css|plasma-theme-master\/zen-browser|plasma-colors\.css|ptm-userChrome|noctalia)/d' "$uc"
        done
        for uc in "$zdir"/*"/chrome/userContent.css"; do
            [ -f "$uc" ] && sed -i -E '/(zen-browser\/(zen-)?userContent\.css|plasma-theme-master\/zen-browser|plasma-colors\.css|ptm-userContent|noctalia)/d' "$uc"
        done
    fi
done
rm -f "$HOME/.mozilla/firefox/"*"/chrome/plasma-colors.css" 2>/dev/null || true
rm -f "$HOME/.mozilla/firefox/"*"/chrome/ptm-userChrome.css" 2>/dev/null || true
for uc in "$HOME/.mozilla/firefox/"*"/chrome/userChrome.css"; do
    if [ -f "$uc" ]; then
        sed -i -E '/(@import.*plasma-colors\.css|@import.*ptm-userChrome\.css)/d' "$uc"
    fi
done

# 21. Plasma color schemes
echo -e "\n${YELLOW}Checking KDE Plasma Color Schemes...${NC}"
remove_file "$HOME/.local/share/color-schemes/ptm.colors"
remove_file "$HOME/.local/share/color-schemes/noctalia.colors"

# 21b. GTK 3 & 4 Theme Sync
echo -e "\n${YELLOW}Checking GTK 3 & 4...${NC}"
remove_file "$HOME/.config/gtk-3.0/ptm.css"
if [ -f "$HOME/.config/gtk-3.0/gtk.css" ]; then
    sed -i '/@import.*ptm\.css/d' "$HOME/.config/gtk-3.0/gtk.css"
fi
remove_file "$HOME/.config/gtk-4.0/ptm.css"
if [ -f "$HOME/.config/gtk-4.0/gtk.css" ]; then
    sed -i '/@import.*ptm\.css/d' "$HOME/.config/gtk-4.0/gtk.css"
fi

# 22. Steam & Millennium Themes
echo -e "\n${YELLOW}Checking Steam / Millennium theme files...${NC}"
for p in "$HOME/.local/share/Steam" "$HOME/.steam/steam"; do
    restore_backup "$p/millennium/themes/Material-Theme/css/main/colors/matugen.css" "$p/millennium/themes/Material-Theme/css/main/colors/matugen.css.ptm.bak"
    restore_backup "$p/steamui/skins/Material-Theme/css/main/colors/matugen.css" "$p/steamui/skins/Material-Theme/css/main/colors/matugen.css.ptm.bak"
done

# 23. Icon Themes (WhiteSur pristine restore & Papirus)
echo -e "\n${YELLOW}Checking Icon Themes (WhiteSur / Papirus)...${NC}"
if [ -d "$HOME/.local/share/icons/.whitesur-pristine" ]; then
    for tdir in "$HOME/.local/share/icons/.whitesur-pristine"/*; do
        [ -d "$tdir" ] || continue
        tname="$(basename "$tdir")"
        if [ -d "$HOME/.local/share/icons/$tname" ]; then
            cp -r "$tdir/places" "$HOME/.local/share/icons/$tname/" 2>/dev/null || true
            [ -d "$tdir/places@2x" ] && cp -r "$tdir/places@2x" "$HOME/.local/share/icons/$tname/" 2>/dev/null || true
            echo -e "${GREEN}✔ Restored pristine icons:${NC} $tname"
            gtk-update-icon-cache -f -q "$HOME/.local/share/icons/$tname" 2>/dev/null || true
        fi
    done
    rm -rf "$HOME/.local/share/icons/.whitesur-pristine"
fi

# 24. PTM Template Cache
echo -e "\n${YELLOW}Checking PTM Template Cache...${NC}"
if [ -d "$HOME/.cache/ptm" ]; then
    rm -rf "$HOME/.cache/ptm"
    echo -e "${GREEN}✔ Removed cache:${NC} $HOME/.cache/ptm"
fi

# 25. Auto-restore any remaining .ptm.bak or .noctalia.bak files in ~/.config and ~/.local
echo -e "\n${YELLOW}Scanning for remaining backup files...${NC}"
find "$HOME/.config" "$HOME/.local" -maxdepth 4 -type f \( -name "*.ptm.bak" -o -name "*.noctalia.bak" \) 2>/dev/null | while read -r bak; do
    orig="${bak%.ptm.bak}"
    orig="${orig%.noctalia.bak}"
    restore_backup "$orig" "$bak"
done

echo -e "\n${GREEN}✔ Clean-up complete! All template files and links removed.${NC}"
