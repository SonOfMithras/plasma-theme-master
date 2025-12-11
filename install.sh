#!/bin/bash

# Plasma Theme Master - Install Script
# Installs to /opt/plasma-theme-master and integrates with KDE Plasma.

INSTALL_DIR="/opt/plasma-theme-master"
BIN_PATH="/usr/local/bin/plasma-theme-master"
DESKTOP_PATH="/usr/share/applications/plasma-theme-master.desktop"

# Check for root
if [ "$EUID" -ne 0 ]; then 
  echo "Please run as root (sudo ./install.sh)"
  exit 1
fi

echo "Installing Plasma Theme Master..."

# 1. Create Directory and Copy Files
echo "Creating directory $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"
cp -r src "$INSTALL_DIR/"
cp requirements.txt "$INSTALL_DIR/"

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

# 5. Install Dependencies (Optional/Best Effort)
# We assume python3-pyside6 is best installed via system package manager, 
# but for a generic script we might suggest user action or try pip if desired.
# ideally we don't mess with system pip.
echo "--------------------------------------------------------"
echo "Installation structure complete!"
echo ""
echo "NOTE: This tool requires PySide6."
echo "If not installed system-wide, please run:"
echo "  sudo apt install python3-pyside6  (Debian/Ubuntu)"
echo "  sudo pacman -S python-pyside6     (Arch)"
echo "  OR pip install PySide6 --break-system-packages (Not Recommended)"
echo ""
echo "You can now launch 'Plasma Theme Master' from your menu."
