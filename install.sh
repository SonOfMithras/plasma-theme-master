#!/bin/bash
set -e

APP_NAME="plasma-theme-master"
SERVICE_NAME="plasma-theme-master.service"

# Function to check and install dependencies on Debian-based systems
check_dependencies() {
    if command -v apt >/dev/null 2>&1; then
        echo "Detected Debian-based system (apt found)."
        DEPENDENCIES="build-essential cmake extra-cmake-modules qt6-base-dev qt6-declarative-dev libkf6config-dev libkf6coreaddons-dev"
        MISSING_DEPS=""

        for dep in $DEPENDENCIES; do
            if ! dpkg-query -W -f='${Status}' "$dep" 2>/dev/null | grep -q "install ok installed"; then
                MISSING_DEPS="$MISSING_DEPS $dep"
            fi
        done

        if [ -n "$MISSING_DEPS" ]; then
            echo "The following dependencies are missing: $MISSING_DEPS"
            read -p "Do you want to install them now? (y/N) " response
            if [[ "$response" =~ ^[Yy]$ ]]; then
                echo "Installing dependencies (requires sudo)..."
                sudo apt update
                sudo apt install -y $MISSING_DEPS
            else
                echo "Warning: Build may fail without these dependencies."
            fi
        else
            echo "All dependencies appear to be installed."
        fi
    fi
}

check_dependencies

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
