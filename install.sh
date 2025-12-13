#!/bin/bash

# Plasma Theme Master - Install Script
# Installs to /opt/plasma-theme-master and integrates with KDE Plasma.

# PKG_ROOT allows installing to a temporary directory for packaging (e.g. .deb creation)
PKG_ROOT="${PKG_ROOT:-}"

INSTALL_DIR="${PKG_ROOT}/opt/plasma-theme-master"
BIN_PATH="${PKG_ROOT}/usr/local/bin/plasma-theme-master"
DESKTOP_PATH="${PKG_ROOT}/usr/share/applications/plasma-theme-master.desktop"

# Check for root
# Check for root (unless we are packaging into a custom root)
if [ -z "$PKG_ROOT" ] && [ "$EUID" -ne 0 ]; then 
  echo "Please run as root (sudo ./install.sh)"
  exit 1
fi

echo "Installing Plasma Theme Master..."

# 1. Create Directory and Copy Files
# Clean up potential leftovers from previous installs to ensure a clean update
if [ -d "$INSTALL_DIR" ]; then
    echo "removing old files from $INSTALL_DIR..."
    rm -rf "$INSTALL_DIR"
fi

echo "Creating directory $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"
cp -r src "$INSTALL_DIR/"
cp requirements.txt "$INSTALL_DIR/"
cp uninstall.sh "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/uninstall.sh"

# 2. Set Permissions
chmod -R 755 "$INSTALL_DIR"

# 3. Create Wrapper Script
echo "Creating executable wrapper at $BIN_PATH..."
cat <<EOF > "$BIN_PATH"
#!/bin/bash
# Wrapper for Plasma Theme Master
cd "$INSTALL_DIR"
python3 src/main.py "\$@"
EOF

chmod +x "$BIN_PATH"

# 4. Create Desktop Entry
echo "Creating desktop entry at $DESKTOP_PATH..."
cat <<EOF > "$DESKTOP_PATH"
[Desktop Entry]
Name=Plasma Theme Master
Comment=Manage Kvantum Day/Night cycles and Edit Global Themes
Exec=$BIN_PATH
Icon=preferences-desktop-theme
Type=Application
Categories=Settings;DesktopSettings;Qt;
Terminal=false
EOF

# 5. Install Systemd Service
SERVICE_DEST="${PKG_ROOT}/usr/lib/systemd/user/plasma-theme-master.service"
echo "Installing systemd service to $SERVICE_DEST..."
# Ensure dir exists (it should on systemd distros)
mkdir -p "${PKG_ROOT}/usr/lib/systemd/user"
cp plasma-theme-master.service "$SERVICE_DEST"
chmod 644 "$SERVICE_DEST"

# 6. Interactive Service Enable
echo ""
read -p "Do you want to enable the background service now? [y/N] " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -n "$SUDO_USER" ]; then
        REAL_USER="$SUDO_USER"
        USER_ID=$(id -u "$REAL_USER")
        echo "Enabling service for user: $REAL_USER (UID: $USER_ID)..."
        
        # We need XDG_RUNTIME_DIR for systemctl --user to work from sudo
        if systemctl --user --machine="${REAL_USER}@.host" enable --now plasma-theme-master.service; then
            echo "Service enabled and started successfully!"
        else
            # Fallback for systems where machinectl/systemd-container logic might fail slightly or differ
            echo "Attempting fallback method..."
            export XDG_RUNTIME_DIR="/run/user/$USER_ID"
            # Getting DBUS address is tricky, usually at /run/user/UID/bus
            export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$USER_ID/bus"
            
            if sudo -E -u "$REAL_USER" env "XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR" "DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS" systemctl --user enable --now plasma-theme-master.service; then
                 echo "Service enabled via fallback!"
            else
                echo "Failed to enable service automatically."
                echo "Please run this manually as your normal user:"
                echo "  systemctl --user enable --now plasma-theme-master.service"
            fi
        fi
    else
        echo "Warning: Could not detect original user (SUDO_USER not set)."
        echo "Please enable the service manually as your normal user:"
        echo "  systemctl --user enable --now plasma-theme-master.service"
    fi
else
    echo "Skipping service enable."
    echo "To enable later, run:"
    echo "  systemctl --user enable --now plasma-theme-master.service"
fi
echo ""

# 7. Install Dependencies (Optional/Best Effort)
# We assume python3-pyside6 is best installed via system package manager, 
# but for a generic script we might suggest user action or try pip if desired.
# ideally we don't mess with system pip.
echo "--------------------------------------------------------"
echo "Installation complete!"
echo ""
echo "NOTE: This tool requires PySide6."
echo "If not installed system-wide, please run:"
echo "  sudo apt install python3-pyside6  (Debian/Ubuntu)"
echo "  sudo pacman -S python-pyside6     (Arch)"
echo "  OR pip install PySide6 --break-system-packages (Not Recommended)"
echo ""
echo "You can launch 'Plasma Theme Master' from your menu."
