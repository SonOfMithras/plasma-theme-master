#!/usr/bin/env bash
# Runs on every theme change (post_hook). Converts the rendered Theme_Colors.xcu's
# hex color values to the decimal integers LibreOffice's ColorScheme schema
# actually requires, assembles a fresh .oxt from this directory's static files
# plus the converted colors, and installs it via unopkg.
set -euo pipefail

CONFIG_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RENDERED="${XDG_STATE_HOME:-$HOME/.local/state}/ptm/libreoffice-theme-staging/Theme_Colors.xcu"
BUILD_DIR="$CONFIG_DIR/build"
OXT_PATH="$BUILD_DIR/ptm-theme.oxt"

if [ ! -f "$RENDERED" ]; then
  echo "ptm libreoffice: rendered Theme_Colors.xcu not found at $RENDERED" >&2
  exit 1
fi

rm -rf "$BUILD_DIR/pkg"
mkdir -p "$BUILD_DIR/pkg/META-INF"

# Convert every 6-hex-digit <value>...</value> to its decimal equivalent.
# ColorScheme's schema stores colors as base-10 integers, not hex strings.
python3 - "$RENDERED" "$BUILD_DIR/pkg/Theme_Colors.xcu" << 'PYEOF'
import re
import sys

src, dst = sys.argv[1], sys.argv[2]
with open(src) as f:
    content = f.read()


def hex_to_dec(match):
    return f"<value>{int(match.group(1), 16)}</value>"


converted = re.sub(r"<value>([0-9a-fA-F]{6})</value>", hex_to_dec, content)
with open(dst, "w") as f:
    f.write(converted)
PYEOF

cp "$CONFIG_DIR/Paths.xcu" "$BUILD_DIR/pkg/Paths.xcu"
cp "$CONFIG_DIR/description.xml" "$BUILD_DIR/pkg/description.xml"
cp "$CONFIG_DIR/pkg-description.en" "$BUILD_DIR/pkg/pkg-description.en"

# Write the package manifest inline rather than copying it from the template directory.
# The template distribution pipeline only ships top-level files, so the
# META-INF/manifest.xml that used to live here was never delivered to users and
# the previous `cp` aborted under set -euo pipefail before the .oxt was built.
# The manifest is static and small, so generating it here keeps the template
# self-contained and removes the only subdirectory dependency.
cat > "$BUILD_DIR/pkg/META-INF/manifest.xml" <<'MANIFEST'
<?xml version="1.0" encoding="UTF-8"?>
<manifest:manifest xmlns:manifest="http://openoffice.org/2001/manifest">
  <manifest:file-entry manifest:full-path="Theme_Colors.xcu" manifest:media-type="application/vnd.sun.star.configuration-data"/>
  <manifest:file-entry manifest:full-path="Paths.xcu" manifest:media-type="application/vnd.sun.star.configuration-data"/>
</manifest:manifest>
MANIFEST

rm -f "$OXT_PATH"
(cd "$BUILD_DIR/pkg" && zip -qr "$OXT_PATH" .)

# unopkg add --force while LibreOffice is running has been confirmed live to
# silently corrupt the install (it can remove the old registration before
# failing to add the new one, leaving nothing installed at all). Skip the
# install step entirely rather than risk that -- the .oxt is still rebuilt
# above so the next run (once LibreOffice is closed) installs the current
# colors, and a restart is already required to see any change regardless.
if pgrep -x soffice.bin >/dev/null 2>&1; then
  # The .oxt above is already rebuilt with the current colors, only the
  # install step is skipped here. A restart alone does NOT pick up anything
  # new, the previously-installed extension is still what's registered.
  # Close LibreOffice, then re-run the theme apply (or wait for the next
  # theme change) to actually install this build.
  echo "ptm libreoffice: LibreOffice is running, skipping extension install this run (installing while it's open has been confirmed to corrupt the existing install). Close LibreOffice, then re-apply the theme to actually install these colors, a restart alone will not pick them up." >&2
  exit 0
fi

EXT_ID="dev.ptm.libreoffice.theme"
INSTALLED_ANY=0

# `add --force` alone has been confirmed live to leave the extension in a
# broken state (a stale cache index pointing at a deleted temp dir, `list`
# then reports nothing installed at all, and every later reinstall fails
# with "file opening ... NOT_EXISTING" until the cache is cleared by hand).
# Explicitly removing the old registration first, ignoring failure since it
# may not be installed yet, avoids relying on --force to do that atomically.

# Install to every LibreOffice found, not just whichever `unopkg` a plain
# PATH lookup happens to resolve first. A machine with both a native
# LibreOffice (providing `unopkg` on PATH) and the Flatpak (the tested
# target) installed would otherwise only get themed on whichever one won
# that lookup, silently leaving the other one unthemed.
#
# Native detection here also covers the official TDF `.deb`/`.rpm` layout,
# which installs into /opt/libreoffice<major>.<minor>/ and does NOT place
# `unopkg` on PATH. `command -v unopkg` alone misses it, so fall back to the
# known install dirs before concluding there is no native install.
UNOPKG="$(command -v unopkg || true)"
if [ -z "$UNOPKG" ]; then
  for candidate in /opt/libreoffice*/program/unopkg \
                   /usr/lib/libreoffice/program/unopkg \
                   /usr/lib64/libreoffice/program/unopkg; do
    if [ -x "$candidate" ]; then
      UNOPKG="$candidate"
      break
    fi
  done
fi

if [ -n "$UNOPKG" ]; then
  "$UNOPKG" remove "$EXT_ID" >/dev/null 2>&1 || true
  if "$UNOPKG" add --force "$OXT_PATH" 2>&1; then
    INSTALLED_ANY=1
  else
    echo "ptm libreoffice: native unopkg install failed ($UNOPKG)" >&2
  fi
fi

if flatpak info org.libreoffice.LibreOffice >/dev/null 2>&1; then
  flatpak run --command=/app/libreoffice/program/unopkg org.libreoffice.LibreOffice remove "$EXT_ID" >/dev/null 2>&1 || true
  if flatpak run --command=/app/libreoffice/program/unopkg org.libreoffice.LibreOffice add --force "$OXT_PATH" 2>&1; then
    INSTALLED_ANY=1
  else
    echo "ptm libreoffice: Flatpak unopkg install failed" >&2
  fi
fi

if [ "$INSTALLED_ANY" -eq 0 ]; then
  echo "ptm libreoffice: no LibreOffice installation found (neither native unopkg on PATH nor the org.libreoffice.LibreOffice Flatpak)" >&2
fi
