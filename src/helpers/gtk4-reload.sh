#!/usr/bin/env bash
# gtk4-reload.sh
# Forces a live refresh of GTK4 themes by toggling GNOME desktop color-scheme via gsettings.

sleep 1

current=$(gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null || echo "")

if [ "$current" = "'prefer-dark'" ]; then
    gsettings set org.gnome.desktop.interface color-scheme 'prefer-light'
    gsettings set org.gnome.desktop.interface color-scheme 'prefer-dark'
else
    gsettings set org.gnome.desktop.interface color-scheme 'prefer-dark'
    gsettings set org.gnome.desktop.interface color-scheme 'prefer-light'
fi
