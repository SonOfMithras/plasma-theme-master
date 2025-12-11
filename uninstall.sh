#!/bin/bash

# Plasma Theme Master - Uninstall Script

INSTALL_DIR="/opt/plasma-theme-master"
BIN_PATH="/usr/local/bin/plasma-theme-master"
DESKTOP_PATH="/usr/share/applications/plasma-theme-master.desktop"

# Check for root
if [ "$EUID" -ne 0 ]; then 
  echo "Please run as root (sudo ./uninstall.sh)"
  exit 1
fi

echo "Uninstalling Plasma Theme Master..."

if [ -d "$INSTALL_DIR" ]; then
    rm -rf "$INSTALL_DIR"
    echo "Removed $INSTALL_DIR"
fi

if [ -f "$BIN_PATH" ]; then
    rm "$BIN_PATH"
    echo "Removed $BIN_PATH"
fi

if [ -f "$DESKTOP_PATH" ]; then
    rm "$DESKTOP_PATH"
    echo "Removed $DESKTOP_PATH"
fi

echo "Uninstallation complete."
