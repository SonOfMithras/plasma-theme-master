#!/usr/bin/env bash
# Gruvbox Plus folder icons — dynamic folder color for PTM.
set -euo pipefail
shopt -s nullglob

COLOR_FILE="$(cd "$(dirname "$0")" && pwd)/colors-final"
DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"
VARIANTS=(Gruvbox-Plus-Dark Gruvbox-Plus-Light)
BASE=blue BASE_FRONT=83a598 BASE_BACK=458588
GLYPH_CONTRAST=1.3 # contrast ratio between the glyph and the folder (higher = stronger glyph)

msg() { printf 'gruvbox-plus-icons: %s\n' "$*" >&2; }

[[ -f "$COLOR_FILE" ]] || exit 0
mapfile -t lines <"$COLOR_FILE"

# Ensure the color file has all required lines
((${#lines[@]} >= 3)) || exit 0

FRONT="${lines[0]//[# ]/}"
BACK="${lines[1]//[# ]/}"
SURFACE="${lines[2]//[# ]/}"

for c in "$FRONT" "$BACK" "$SURFACE"; do
  [[ "$c" =~ ^[0-9a-fA-F]{6}$ ]] || exit 0
done

FRONT="${FRONT,,}"
BACK="${BACK,,}"

# Same hue as the folder, darker (or lighter when it is too dark to go darker)
# until the WCAG contrast ratio against it is GLYPH_CONTRAST. Works in linear light.
glyph_color() {
  awk -v hex="$1" -v r="$GLYPH_CONTRAST" '
    function lin(h, i,  v) {
      v = index("0123456789abcdef", substr(hex, i, 1)) - 1
      v = (v * 16 + index("0123456789abcdef", substr(hex, i + 1, 1)) - 1) / 255
      return (v <= 0.04045) ? v / 12.92 : ((v + 0.055) / 1.055) ^ 2.4
    }
    function enc(c) {
      if (c < 0) c = 0; if (c > 1) c = 1
      c = (c <= 0.0031308) ? c * 12.92 : 1.055 * c ^ (1 / 2.4) - 0.055
      return sprintf("%02x", int(c * 255 + 0.5))
    }
    BEGIN {
      c[0] = lin(hex, 1); c[1] = lin(hex, 3); c[2] = lin(hex, 5)
      y = 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]
      yd = (y + 0.05) / r - 0.05
      if (yd >= 0 && y > 0) {
        for (i = 0; i < 3; i++) c[i] *= yd / y
      } else {
        t = (r * (y + 0.05) - 0.05 - y) / (1 - y)
        for (i = 0; i < 3; i++) c[i] += t * (1 - c[i])
      }
      printf "%s%s%s", enc(c[0]), enc(c[1]), enc(c[2])
    }'
}
GLYPH="$(glyph_color "$FRONT")"

find_user_dir() {
  local d
  for d in "$DATA_HOME/icons/$1" "$HOME/.icons/$1"; do
    [[ -f "$d/index.theme" ]] && { printf '%s' "$d"; return 0; }
  done
  return 1
}

find_system_dir() {
  local IFS=: d
  for d in ${XDG_DATA_DIRS:-/usr/local/share:/usr/share} /usr/local/share /usr/share; do
    [[ -f "$d/icons/$1/index.theme" ]] && { printf '%s' "$d/icons/$1"; return 0; }
  done
  return 1
}

# folder-blue.svg -> folder.svg, folder-blue-docs.svg -> folder-docs.svg, ...
# Recolor icons in parallel across available CPU cores for maximum performance.
recolor() { # <places/scalable dir>
  local dir="$1" f name alias tmp changed_flag="$dir/.changed_flag"
  [[ -f "$dir/folder-$BASE.svg" ]] || { msg "folder-$BASE.svg not found in $dir"; return 1; }

  rm -f "$changed_flag"

  for f in "$dir"/*-$BASE.svg "$dir"/*-$BASE-*.svg; do
    [[ -f "$f" ]] || continue
    (
      name="${f##*/}"
      if [[ "$name" == "bookmarks-$BASE.svg" ]]; then
        alias="folder-bookmark.svg"
      else
        alias="${name/-$BASE/}"
      fi

      tmp="$dir/.$alias.tmp"

      # The glyph is the back-colored element next to the glyph's highlight/shadow
      # (opacity .1 before it, opacity .05 after it); everything else is the folder body.
      sed -E -z \
        -e "s,fill=\"#$BASE_BACK\"(/?>[[:space:]]*<path[^>]*fill=\"#282828\" opacity=\"\\.05\"),fill=\"@G@\"\\1,g" \
        -e "s,(opacity=\"\\.1\"/?>[[:space:]]*<path[^>]*)fill=\"#$BASE_BACK\",\\1fill=\"@G@\",g" \
        -e "s,#$BASE_BACK,@B@,g" -e "s,#$BASE_FRONT,@F@,g" \
        -e "s,@B@,#$BACK,g" -e "s,@F@,#$FRONT,g" -e "s,@G@,#$GLYPH,g" \
        "$f" >"$tmp"

      if cmp -s "$tmp" "$dir/$alias"; then
        rm -f "$tmp"
      else
        mv -f "$tmp" "$dir/$alias"
        touch "$changed_flag"
      fi
    ) &
  done
  wait

  if [[ -f "$changed_flag" ]]; then
    rm -f "$changed_flag"
    echo "changed"
  fi
  return 0
}

installed=()
status=0
for variant in "${VARIANTS[@]}"; do
  if ! dir="$(find_user_dir "$variant")"; then
    sys="$(find_system_dir "$variant")" || continue
    dir="$DATA_HOME/icons/$variant"
    mkdir -p "$DATA_HOME/icons"
    cp -a --reflink=auto "$sys" "$dir"
  fi

  scalable="$dir/places/scalable"
  [[ -w "$scalable" ]] || { msg "$scalable is not writable"; status=1; continue; }

  result="$(recolor "$scalable")" || { status=1; continue; }

  if [[ -n "$result" ]] && command -v gtk-update-icon-cache >/dev/null; then
    gtk-update-icon-cache -f -q "$dir" 2>/dev/null || true
  fi

  installed+=("$variant")
done

if ((${#installed[@]} == 0)); then
  msg "Gruvbox Plus icon theme is not installed"
  exit 1
fi

# Follow dark/light mode preference if gsettings is available
if command -v gsettings >/dev/null; then
  current="$(gsettings get org.gnome.desktop.interface icon-theme 2>/dev/null || true)"
  if [[ "$current" == *Gruvbox-Plus* ]]; then
    luma=$(((0x${SURFACE:0:2} * 299 + 0x${SURFACE:2:2} * 587 + 0x${SURFACE:4:2} * 114) / 1000))
    want=Gruvbox-Plus-Dark
    ((luma >= 128)) && want=Gruvbox-Plus-Light
    if [[ " ${installed[*]} " == *" $want "* && "$current" != "'$want'" ]]; then
      gsettings set org.gnome.desktop.interface icon-theme "$want" 2>/dev/null || true
    fi
  fi
fi

exit "$status"
