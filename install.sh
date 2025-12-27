#!/bin/bash
set -e

APP_NAME="plasma-theme-master"
SERVICE_NAME="plasma-theme-master.service"

echo "=== Installing $APP_NAME ==="

# 1. Stop existing service if running
echo "Stopping existing service..."
systemctl --user disable --now $SERVICE_NAME 2>/dev/null || true

# 2. Clean previous build
# 2. Configure Build
if [ -d "build" ]; then
    echo "Build directory exists. Using incremental build."
else
    echo "Creating build directory..."
    mkdir build
fi
cd build

# 3. Configure & Build
echo "Building..."
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. Install Binary & Desktop File
echo "Installing to system (requires sudo)..."
sudo make install

# 5. Install Systemd Service (User level)
echo "Configuring systemd user service..."
mkdir -p ~/.config/systemd/user
cp ../$SERVICE_NAME ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now $SERVICE_NAME

# 6. Cleanup
cd ..
echo "Cleaning up build artifacts..."
rm -rf build

echo "=== Installation Complete ==="
echo "Run '$APP_NAME status' to verify."
