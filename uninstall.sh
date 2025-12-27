#!/bin/bash

APP_NAME="plasma-theme-master"
BINARY_PATH="/usr/bin/$APP_NAME"
DESKTOP_FILE="/usr/share/applications/$APP_NAME.desktop"
SERVICE_FILE="$HOME/.config/systemd/user/$APP_NAME.service"
CONFIG_FILE="$HOME/.config/plasma-theme-masterrc"
DATA_DIR="$HOME/.local/share/plasma-theme-master"

echo "=== Uninstalling $APP_NAME ==="

# 1. Stop and remove service
echo "Stopping service..."
systemctl --user disable --now $APP_NAME.service 2>/dev/null || true
if [ -f "$SERVICE_FILE" ]; then
    rm "$SERVICE_FILE"
    systemctl --user daemon-reload
    echo "Service removed."
fi

# 2. Remove Binary and Desktop File
echo "Removing system files (requires sudo)..."
if [ -f "$BINARY_PATH" ]; then
    sudo rm "$BINARY_PATH"
else
    echo "Binary not found at $BINARY_PATH"
fi

if [ -f "$DESKTOP_FILE" ]; then
    sudo rm "$DESKTOP_FILE"
else
     echo "Desktop file not found at $DESKTOP_FILE"
fi

# 3. Prompt for Config/Data
echo ""
read -p "Do you want to remove configuration and logs? [y/N] " response
if [[ "$response" =~ ^[Yy]$ ]]; then
    if [ -f "$CONFIG_FILE" ]; then
        rm -f "$CONFIG_FILE"
        echo "Configuration removed."
    fi
    if [ -d "$DATA_DIR" ]; then
        rm -rf "$DATA_DIR"
        echo "Data directory removed."
    fi
else
    echo "Configuration and data retained."
fi

echo "=== Uninstallation Complete ==="
 add service files and desktop entries here later

echo "Uninstalled."
