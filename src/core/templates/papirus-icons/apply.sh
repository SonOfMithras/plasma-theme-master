#!/usr/bin/env bash
set -euo pipefail

{
  COLOR_FILE="${1:-$(dirname "$0")/colors-final}"
  if [[ ! -f "$COLOR_FILE" ]]; then
    COLOR_FILE="${XDG_CACHE_HOME:-$HOME/.cache}/ptm/papirus-icons/colors-final"
  fi
  [[ -f "$COLOR_FILE" ]] || exit 0

  # 1. Read the file instantly into RAM
  mapfile -t lines < "$COLOR_FILE"

  # 2. Extract and clean the target color
  TARGET="${lines[0]//[# ]/}"
  [[ ${#TARGET} -ge 6 ]] || exit 0
  TARGET=${TARGET:0:6}

  TR=$((16#${TARGET:0:2}))
  TG=$((16#${TARGET:2:2}))
  TB=$((16#${TARGET:4:2}))

  # 3. Extract the mapping array
  MAPPING="${lines[${#lines[@]}-1]}"

  # 3b. Derive dark/light from the surface (background) color's luminance, and point GTK & KDE icon theme at the matching Papirus variant.
  SURFACE="${lines[1]//[# ]/}"
  if [[ ${#SURFACE} -ge 6 ]]; then
    SURFACE=${SURFACE:0:6}
    SR=$((16#${SURFACE:0:2})); SG=$((16#${SURFACE:2:2})); SB=$((16#${SURFACE:4:2}))
    LUMA=$(( (SR*299 + SG*587 + SB*114) / 1000 ))
    ICON_VARIANT="Papirus-Dark"; (( LUMA >= 128 )) && ICON_VARIANT="Papirus-Light"
    command -v gsettings >/dev/null 2>&1 && \
      gsettings set org.gnome.desktop.interface icon-theme "$ICON_VARIANT" 2>/dev/null || true

    CURRENT_KDE_ICON="$(kreadconfig6 --file kdeglobals --group Icons --key Theme 2>/dev/null || true)"
    if [[ "$CURRENT_KDE_ICON" =~ Papirus ]]; then
      kwriteconfig6 --file kdeglobals --group Icons --key Theme "$ICON_VARIANT" 2>/dev/null || true
    fi
  fi

  # 4. Math calculation (HSV-based)
  closest=$(
    awk -v r="$TR" -v g="$TG" -v b="$TB" -v m="$MAPPING" '
    function rgb2hsv(r,g,b, mx,mn,d,h,s,v,t) {
      r/=255; g/=255; b/=255
      mx = (r>g)?(r>b?r:b):(g>b?g:b)
      mn = (r<g)?(r<b?r:b):(g<b?g:b)
      v = mx
      d = mx - mn
      if (d == 0) { s = 0; h = 0 }
      else {
        s = d / mx
        if (mx == r) {
          t = (g - b) / d
          if (t < 0) t += 6
          h = 60 * t
        } else if (mx == g) {
          h = 60 * (((b - r) / d) + 2)
        } else {
          h = 60 * (((r - g) / d) + 4)
        }
      }
      return h SUBSEP s SUBSEP v
    }
    BEGIN {
      # weights: hue dominates, saturation moderate, value (brightness) least
      WH = 10
      WS = 1
      WV = 0.3

      split(rgb2hsv(r,g,b), tgt, SUBSEP)
      th = tgt[1]; ts = tgt[2]; tv = tgt[3]

      n = split(m, arr)
      for (i = 1; i <= n; i++) {
        split(arr[i], p, ":")
        cr = strtonum("0x" substr(p[2],1,2))
        cg = strtonum("0x" substr(p[2],3,2))
        cb = strtonum("0x" substr(p[2],5,2))

        split(rgb2hsv(cr,cg,cb), c, SUBSEP)
        ch = c[1]; cs = c[2]; cv = c[3]

        dh = th - ch
        if (dh < 0) dh = -dh
        if (dh > 180) dh = 360 - dh
        dh /= 180

        ds = ts - cs
        dv = tv - cv

        # scale hue term by saturation product so greys/low-sat colors
        # do not get distorted by an undefined/meaningless hue
        d = WH * (ts*cs) * dh*dh + WS * ds*ds + WV * dv*dv

        if (min == "" || d < min) {
          min = d
          name = p[1]
        }
      }
      print name
    }
  ')

  # 5/6. Recolor every installed Papirus variant, not just the base one so Papirus-Dark and Papirus-Light stay in sync too
  [[ -n "$closest" ]] || { echo "Error: no matching folder color found" 1>&2; exit 0; }

  # Locate the system Papirus install via XDG_DATA_DIRS (covers NixOS, Guix
  # and custom prefixes), falling back to the classic FHS locations.
  dirs=()
  IFS=: read -ra dirs <<< "${XDG_DATA_DIRS:-}"
  dirs+=(/usr/local/share /usr/share)

  for variant in Papirus Papirus-Dark Papirus-Light; do
    src=""
    for dir in "${dirs[@]}"; do
      if [[ -n "$dir" && -d "$dir/icons/$variant" ]]; then
        # Resolve symlinks so we copy the real theme, not a link into a
        # read-only store (e.g. /run/current-system/sw on NixOS).
        src="$(readlink -f "$dir/icons/$variant")"
        break
      fi
    done
    [[ -n "$src" ]] || continue

    dest="$HOME/.local/share/icons/$variant"
    if [[ ! -d "$dest" ]]; then
      mkdir -p "$HOME/.local/share/icons"
      cp -r "$src" "$dest"
      # Files copied from a read-only store keep their read-only mode.
      chmod -R u+w "$dest"
    fi
    "$(dirname "$0")/papirus-folders" -t "$variant" -C "$closest" \
      || echo "Error: papirus-folders failed for $variant" 1>&2
    gtk-update-icon-cache -f -q "$dest" 2>/dev/null || true
    touch "$dest" 2>/dev/null || true
  done

  # Notify KWin & Plasma
  qdbus6 org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
  qdbus6 org.kde.plasmashell /PlasmaShell refreshCurrentShell >/dev/null 2>&1 || true
  qdbus6 org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.refreshCurrentShell >/dev/null 2>&1 || true
}
