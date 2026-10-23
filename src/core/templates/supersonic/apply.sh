#!/usr/bin/env bash
set -euo pipefail

flatpak_id="io.github.dweymouth.supersonic"

reload_native() {
    command -v supersonic-desktop >/dev/null 2>&1 || return 0

    local socket_path
    if [ -n "${XDG_RUNTIME_DIR:-}" ]; then
        socket_path="$XDG_RUNTIME_DIR/supersonic.sock"
    else
        socket_path="/tmp/supersonic-$(id -u).sock"
    fi

    # The CLI exits with an error if no instance is listening. Avoid launching
    # it when Supersonic is closed; the new theme loads on its next start.
    [ -S "$socket_path" ] || return 0
    supersonic-desktop --reload-theme >/dev/null 2>&1 || true
}

flatpak_is_running() {
    local application
    while IFS= read -r application; do
        [ "$application" = "$flatpak_id" ] && return 0
    done < <(flatpak ps --columns=application 2>/dev/null)
    return 1
}

reload_flatpak() {
    command -v flatpak >/dev/null 2>&1 || return 0
    flatpak_is_running || return 0
    flatpak run "$flatpak_id" --reload-theme >/dev/null 2>&1 || true
}

case "${1:-}" in
    native)
        reload_native
        ;;
    flatpak)
        reload_flatpak
        ;;
    *)
        echo "Usage: $0 {native|flatpak}" >&2
        exit 2
        ;;
esac
