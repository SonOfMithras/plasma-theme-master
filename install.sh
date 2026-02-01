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
        
        DEPENDENCIES="build-essential cmake extra-cmake-modules qt6-base-dev qt6-declarative-dev libkf6config-dev libkf6coreaddons-dev"
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
        
        # Check standard dependencies
        # Updated to use 'kconfig' and 'kcoreaddons' as requested
        ARCH_DEPENDENCIES="base-devel cmake extra-cmake-modules qt6-base qt6-declarative kconfig kcoreaddons"
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

        # Check for libadwaita-without-adwaita
        if ! pacman -Qi libadwaita-without-adwaita >/dev/null 2>&1; then
             print_warning "libadwaita-without-adwaita is missing (recommended for GTK4 theming)."
             if ask_confirm "Do you want to install it from AUR?"; then
                 if command -v yay >/dev/null 2>&1; then
                     print_info "Installing using yay..."
                     yay -S libadwaita-without-adwaita
                 elif command -v paru >/dev/null 2>&1; then
                     print_info "Installing using paru..."
                     paru -S libadwaita-without-adwaita
                 else
                     print_error "No AUR helper (yay/paru) found. Please install 'libadwaita-without-adwaita' manually."
                 fi
             fi
        fi
    fi
}

check_dependencies

print_header "Installing $APP_NAME"

# 1. Stop existing service if running
print_info "Stopping existing service..."
systemctl --user disable --now $SERVICE_NAME 2>/dev/null || true

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

# 6. Cleanup
cd ..
print_info "Cleaning up build artifacts..."
rm -rf build

print_header "Installation Complete"
print_success "Run '$APP_NAME status' to verify."
