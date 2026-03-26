#!/bin/bash

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
    "$PREFIX/bin/plasma-theme-master-uninstall"
)

echo "=== Uninstalling $APP_NAME ==="

# 1. Stop and remove service
echo "Stopping service..."
systemctl --user disable --now $APP_NAME.service 2>/dev/null || true
if [ -f "$SERVICE_FILE" ]; then
    rm "$SERVICE_FILE"
    systemctl --user daemon-reload
    echo "Service removed."
fi

# 2. Remove main binary and desktop file
echo "Removing system files (requires sudo)..."
if [ -f "$BINARY_PATH" ]; then
    sudo rm "$BINARY_PATH"
    echo "Removed binary: $BINARY_PATH"
else
    echo "Binary not found at $BINARY_PATH"
fi

if [ -f "$DESKTOP_FILE" ]; then
    sudo rm "$DESKTOP_FILE"
    echo "Removed desktop file."
else
    echo "Desktop file not found at $DESKTOP_FILE"
fi

# 3. Remove helper binaries
echo "Removing helper binaries..."
for helper in "${HELPERS[@]}"; do
    if [ -f "$helper" ]; then
        sudo rm "$helper"
        echo "  Removed: $helper"
    fi
done

# 4. Remove system data directory (templates, default config)
if [ -d "$SYSTEM_DATA_DIR" ]; then
    sudo rm -rf "$SYSTEM_DATA_DIR"
    echo "Removed system data: $SYSTEM_DATA_DIR"
fi

# 5. Prompt for user config/data
echo ""
read -p "Do you want to remove user configuration and logs? [y/N] " response
if [[ "$response" =~ ^[Yy]$ ]]; then
    if [ -f "$CONFIG_FILE" ]; then
        rm -f "$CONFIG_FILE"
        echo "Removed: $CONFIG_FILE"
    fi
    if [ -f "$TOML_CONFIG" ]; then
        rm -f "$TOML_CONFIG"
        # Remove config dir if now empty
        rmdir --ignore-fail-on-non-empty "$HOME/.config/plasma-theme-master" 2>/dev/null
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

echo ""
echo "=== Uninstallation Complete ==="
