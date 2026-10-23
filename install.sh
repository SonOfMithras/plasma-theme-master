#!/bin/bash
set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # No Color

APP_NAME="plasma-theme-master"
SERVICE_NAME="plasma-theme-master.service"

# Helper Functions
print_header() {
    echo -e "${BLUE}${BOLD}=== $1 ===${NC}"
}

print_success() {
    echo -e "${GREEN}✔ $1${NC}"
}

print_error() {
    echo -e "${RED}✖ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BOLD}ℹ $1${NC}"
}

ask_confirm() {
    local prompt="$1"
    local response
    echo -e -n "${YELLOW}$prompt [y/N]: ${NC}"
    read -r response
    [[ "$response" =~ ^[Yy]$ ]]
}

# Function to check and install dependencies on Debian-based systems
check_dependencies() {
    if command -v apt >/dev/null 2>&1; then
        print_info "Detected Debian-based system (apt found)."

        # Check for Kvantum Manager
        if ! command -v kvantummanager >/dev/null 2>&1; then
            print_warning "Kvantum Manager is not installed."
            if ask_confirm "Would you like to install Kvantum Manager and themes?"; then
                print_info "Installing Kvantum Manager..."
                sudo add-apt-repository -y ppa:papirus/papirus
                sudo apt update
                sudo apt install -y qt6-style-kvantum qt6-style-kvantum-themes
                print_success "Kvantum Manager installed."
            fi
        fi
        
        DEPENDENCIES="build-essential cmake extra-cmake-modules qt6-base-dev libkf6config-dev libkf6coreaddons-dev libkf6colorscheme-dev pipx"
        MISSING_DEPS=""

        for dep in $DEPENDENCIES; do
            if ! dpkg-query -W -f='${Status}' "$dep" 2>/dev/null | grep -q "install ok installed"; then
                MISSING_DEPS="$MISSING_DEPS $dep"
            fi
        done

        if [ -n "$MISSING_DEPS" ]; then
            print_warning "The following dependencies are missing: $MISSING_DEPS"
            if ask_confirm "Do you want to install them now?"; then
                print_info "Installing dependencies (requires sudo)..."
                sudo apt update
                sudo apt install -y $MISSING_DEPS
                print_success "Dependencies installed."
            else
                print_warning "Build may fail without these dependencies."
            fi
        else
            print_success "All dependencies appear to be installed."
        fi
    elif command -v pacman >/dev/null 2>&1; then
        print_info "Detected Arch-based system (pacman found)."
        
        # Check ARCH dependencies
        ARCH_DEPENDENCIES="base-devel cmake extra-cmake-modules qt6-base kconfig kcoreaddons kcolorscheme python-pipx"
        MISSING_ARCH_DEPS=""
        
        for dep in $ARCH_DEPENDENCIES; do
            if ! pacman -Qi $dep >/dev/null 2>&1; then
                MISSING_ARCH_DEPS="$MISSING_ARCH_DEPS $dep"
            fi
        done
        
        if [ -n "$MISSING_ARCH_DEPS" ]; then
             print_warning "The following dependencies are missing: $MISSING_ARCH_DEPS"
             if ask_confirm "Do you want to install them now?"; then
                 print_info "Installing dependencies (requires sudo)..."
                 sudo pacman -S --needed $MISSING_ARCH_DEPS
                 print_success "Dependencies installed."
             else
                 print_warning "Build may fail without these dependencies."
             fi
        else
            print_success "All standard dependencies appear to be installed."
        fi

        # Check for Kvantum Manager
        if ! command -v kvantummanager >/dev/null 2>&1; then
             print_warning "Kvantum Manager is not installed."
             if ask_confirm "Would you like to install Kvantum Manager?"; then
                 print_info "Installing Kvantum Manager..."
                 sudo pacman -S --needed kvantum
                 print_success "Kvantum Manager installed."
             fi
        fi

    elif command -v dnf >/dev/null 2>&1; then
        print_info "Detected Fedora-based system (dnf found)."
        
        FEDORA_DEPENDENCIES="gcc-c++ cmake extra-cmake-modules qt6-qtbase-devel kf6-kconfig-devel kf6-kcoreaddons-devel kf6-kcolorscheme-devel pipx"
        MISSING_FEDORA_DEPS=""
        
        for dep in $FEDORA_DEPENDENCIES; do
            if ! dnf info installed "$dep" >/dev/null 2>&1; then
                MISSING_FEDORA_DEPS="$MISSING_FEDORA_DEPS $dep"
            fi
        done
        
        if [ -n "$MISSING_FEDORA_DEPS" ]; then
             print_warning "The following dependencies are missing: $MISSING_FEDORA_DEPS"
             if ask_confirm "Do you want to install them now?"; then
                 print_info "Installing dependencies (requires sudo)..."
                 sudo dnf install -y $MISSING_FEDORA_DEPS
                 print_success "Dependencies installed."
             else
                 print_warning "Build may fail without these dependencies."
             fi
        else
            print_success "All standard dependencies appear to be installed."
        fi

        # Check for Kvantum Manager
        if ! command -v kvantummanager >/dev/null 2>&1; then
             print_warning "Kvantum Manager is not installed."
             if ask_confirm "Would you like to install Kvantum Manager (available in standard repo)?"; then
                 print_info "Installing Kvantum Manager..."
                 sudo dnf install -y kvantum
                 print_success "Kvantum Manager installed."
             fi
        fi
    fi
}

# Function to detect installed version <= 2.0.7 and clean/migrate legacy templates & config
migrate_legacy_install() {
    local installed_ver=""
    if command -v "$APP_NAME" >/dev/null 2>&1; then
        installed_ver=$("$APP_NAME" --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+(\.[0-9]+)?' | head -n1 || true)
    fi

    local is_legacy=false

    if [ -n "$installed_ver" ]; then
        local major=$(echo "$installed_ver" | cut -d. -f1)
        local minor=$(echo "$installed_ver" | cut -d. -f2)
        local patch=$(echo "$installed_ver" | cut -d. -f3)
        patch=${patch:-0}
        # If version <= 2.0.7
        if [ "$major" -lt 2 ] 2>/dev/null || { [ "$major" -eq 2 ] && [ "$minor" -eq 0 ] && [ "$patch" -le 7 ]; } 2>/dev/null; then
            is_legacy=true
        fi
    fi

    local user_cfg="$HOME/.config/plasma-theme-master/config.toml"
    # Also detect filesystem markers from <= 2.0.7 even if binary was uninstalled or version was unknown
    if [ -d "$HOME/.config/plasma-theme-master/zen-browser" ] || \
       [ -f "/usr/share/plasma-theme-master/templates/kitty_dark.tpl" ] || \
       [ -f "/usr/share/plasma-theme-master/templates/btop.tpl" ] || \
       ( [ -f "$user_cfg" ] && grep -qE '^\[templates\.(kitty|btop|konsole|obsidian|zed|zen_browser|vicinae|millennium|palette)' "$user_cfg" 2>/dev/null ); then
        is_legacy=true
    fi

    if [ "$is_legacy" = true ]; then
        print_header "Legacy Installation Detected (Version <= 2.0.7)"
        if [ -n "$installed_ver" ]; then
            print_info "Installed version: $installed_ver"
        fi
        print_info "Cleaning legacy template files, cache, and conflicting configuration entries..."

        # 1. Clean generated application templates & cache
        local script_dir
        script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
        if [ -f "$script_dir/clean-templates.sh" ]; then
            bash "$script_dir/clean-templates.sh" || print_warning "Template cleanup completed with warnings."
        fi

        # 2. Clean obsolete root template files from system directory
        if [ -d "/usr/share/plasma-theme-master/templates" ]; then
            print_info "Removing obsolete legacy template files from /usr/share/plasma-theme-master/templates..."
            sudo rm -f /usr/share/plasma-theme-master/templates/*.tpl 2>/dev/null || true
            sudo rm -f /usr/share/plasma-theme-master/templates/README.md 2>/dev/null || true
            sudo rm -rf /usr/share/plasma-theme-master/templates/betterdiscord 2>/dev/null || true
            sudo rm -rf /usr/share/plasma-theme-master/templates/vencord 2>/dev/null || true
            sudo rm -rf /usr/share/plasma-theme-master/templates/builtin 2>/dev/null || true
        fi

        # 3. Clean legacy zen-browser config directory
        if [ -d "$HOME/.config/plasma-theme-master/zen-browser" ]; then
            rm -rf "$HOME/.config/plasma-theme-master/zen-browser"
            print_info "Removed legacy zen-browser directory."
        fi

        # 4. Backup & migrate ~/.config/plasma-theme-master/config.toml
        if [ -f "$user_cfg" ]; then
            print_info "Backing up user config to $user_cfg.2.0.7.bak..."
            cp "$user_cfg" "$user_cfg.2.0.7.bak"

            python3 -c "
import re, sys

cfg_file = '''$user_cfg'''
try:
    with open(cfg_file, 'r', encoding='utf-8') as f:
        content = f.read()

    legacy_sections = {
        'kitty_light', 'kitty_dark', 'btop', 'vicinae', 'firefox',
        'zen_browser_chrome', 'zen_browser_content', 'obsidian',
        'betterdiscord', 'vencord', 'zed', 'vscode', 'konsole', 'millennium', 'palette'
    }
    section_app_map = {
        'kitty_light': 'kitty', 'kitty_dark': 'kitty', 'btop': 'btop',
        'vicinae': 'vicinae', 'firefox': 'firefox',
        'zen_browser_chrome': 'zen-browser', 'zen_browser_content': 'zen-browser',
        'obsidian': 'obsidian', 'betterdiscord': 'discord', 'vencord': 'discord',
        'zed': 'zed', 'vscode': 'vscode', 'konsole': 'konsole', 'millennium': 'steam'
    }

    lines = content.splitlines(keepends=True)
    new_lines = []
    in_legacy = False
    cur_legacy = ''
    enabled_legacy_apps = set()
    found_palette_current = ''

    for line in lines:
        m_sec = re.match(r'^\s*\[templates\.([a-zA-Z0-9_\-]+)\]', line)
        m_any = re.match(r'^\s*\[', line)
        if m_sec:
            name = m_sec.group(1)
            if name in legacy_sections:
                in_legacy = True
                cur_legacy = name
                continue
            else:
                in_legacy = False
                cur_legacy = ''
        elif m_any:
            in_legacy = False
            cur_legacy = ''

        if in_legacy:
            if cur_legacy == 'palette':
                m_pal = re.search(r'current\s*=\s*[\x27\"]([^\x27\"]+)[\x27\"]', line)
                if m_pal:
                    found_palette_current = m_pal.group(1)
            elif re.search(r'enabled\s*=\s*true', line, re.IGNORECASE):
                app = section_app_map.get(cur_legacy)
                if app:
                    enabled_legacy_apps.add(app)
            continue

        new_lines.append(line)

    cleaned = ''.join(new_lines)

    # Ensure [palette] section exists if found in legacy [templates.palette]
    if found_palette_current and not re.search(r'^\s*\[palette\]', cleaned, re.MULTILINE):
        cleaned += f'\n[palette]\ncurrent = \"{found_palette_current}\"\n'

    # Ensure enabled apps are recorded under [app_templates.<app>]
    for app in sorted(enabled_legacy_apps):
        pattern = rf'^\s*\[app_templates\.{re.escape(app)}\]'
        if not re.search(pattern, cleaned, re.MULTILINE):
            cleaned += f'\n[app_templates.{app}]\nenabled = true\n'

    with open(cfg_file, 'w', encoding='utf-8') as f:
        f.write(cleaned)
    print('✔ Successfully pruned legacy [templates.*] sections from config.toml.')
except Exception as e:
    print(f'⚠ Error during config migration: {e}', file=sys.stderr)
" 2>/dev/null || true
            print_success "Config migration and cleanup completed."
        fi
    fi
}

check_dependencies

print_header "Installing $APP_NAME"

# 1. Stop existing service if running
print_info "Stopping existing service..."
systemctl --user disable --now $SERVICE_NAME 2>/dev/null || true

# 1b. Check for version <= 2.0.7 and clean/migrate legacy templates and config
migrate_legacy_install

# 2. Configure Build
if [ -d "build" ]; then
    print_info "Build directory exists. Using incremental build."
else
    print_info "Creating build directory..."
    mkdir build
fi
cd build

# 3. Configure & Build
print_info "Building..."
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. Install Binary & Desktop File
print_info "Installing to system (requires sudo)..."
sudo make install

# 5. Install Systemd Service (User level)
print_info "Configuring systemd user service..."
mkdir -p ~/.config/systemd/user
cp ../$SERVICE_NAME ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable $SERVICE_NAME
print_info "Restarting service to apply changes..."
systemctl --user restart $SERVICE_NAME

# 6. First-run: copy default config.toml if not already present
USER_CONFIG="$HOME/.config/plasma-theme-master/config.toml"
SYSTEM_DEFAULT="/usr/share/plasma-theme-master/config.toml.default"
if [ ! -f "$USER_CONFIG" ] && [ -f "$SYSTEM_DEFAULT" ]; then
    print_info "Installing default config.toml to $USER_CONFIG..."
    mkdir -p "$(dirname "$USER_CONFIG")"
    cp "$SYSTEM_DEFAULT" "$USER_CONFIG"
    print_success "config.toml installed. Edit it to enable apps and set paths."
elif [ -f "$USER_CONFIG" ]; then
    print_info "Existing config.toml found at $USER_CONFIG — skipping (your settings preserved)."
fi

# 7. Cleanup
cd ..
print_info "Cleaning up build artifacts..."
rm -rf build

print_header "Installation Complete"
print_success "Run '$APP_NAME status' to verify."
print_success "Edit ~/.config/plasma-theme-master/config.toml to enable universal theming."
