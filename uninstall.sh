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

# 0. Stop/Disable Service if running
if [ -n "$SUDO_USER" ]; then
    REAL_USER="$SUDO_USER"
    echo "Attempting to stop service for user: $REAL_USER..."
    
    # Try robust machinectl method first
    if systemctl --user --machine="${REAL_USER}@.host" disable --now plasma-theme-master.service 2>/dev/null; then
        echo "Service stopped and disabled."
    else
        # Fallback method
        USER_ID=$(id -u "$REAL_USER")
        export XDG_RUNTIME_DIR="/run/user/$USER_ID"
        export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$USER_ID/bus"
        
        if sudo -E -u "$REAL_USER" env "XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR" "DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS" systemctl --user disable --now plasma-theme-master.service 2>/dev/null; then
             echo "Service stopped via fallback."
        else
             echo "Warning: Could not stop service automatically. Custom user settings or environment?"
        fi
    fi
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

if [ -f "/usr/lib/systemd/user/plasma-theme-master.service" ]; then
    rm "/usr/lib/systemd/user/plasma-theme-master.service"
    echo "Removed systemd service file"
fi

# Optional: Remove user configuration and logs
# Optional: Remove user configuration and logs
if [ -n "$SUDO_USER" ]; then
    # Get user home directory reliably
    USER_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
    CONFIG_DIR="$USER_HOME/.config/plasma-theme-master"
    DATA_DIR="$USER_HOME/.local/share/plasma-theme-master"
    
    CLEANUP_ACTION=false
    CLEANUP_PROMPT=true

    if [ "$1" == "--clean" ]; then
        CLEANUP_ACTION=true
        CLEANUP_PROMPT=false
    elif [ "$1" == "--keep" ]; then
        CLEANUP_ACTION=false
        CLEANUP_PROMPT=false
    fi
    
    if [ -d "$CONFIG_DIR" ] || [ -d "$DATA_DIR" ]; then
        if [ "$CLEANUP_PROMPT" == true ]; then
            echo ""
            read -p "Do you want to clean up user configuration and logs? [y/N] " -n 1 -r
            echo ""
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                CLEANUP_ACTION=true
            fi
        fi

        if [ "$CLEANUP_ACTION" == true ]; then
            if [ -d "$CONFIG_DIR" ]; then
                rm -rf "$CONFIG_DIR"
                echo "Removed configuration directory: $CONFIG_DIR"
            fi
            if [ -d "$DATA_DIR" ]; then
                rm -rf "$DATA_DIR"
                echo "Removed data/log directory: $DATA_DIR"
            fi
        else
            echo "User data preserved."
        fi
    fi
fi

echo "Uninstallation complete."
