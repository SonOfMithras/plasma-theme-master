#!/usr/bin/env bash
set -euo pipefail

config_file="${XDG_CONFIG_HOME:-$HOME/.config}/halloy/config.toml"

if [ ! -f "$config_file" ]; then
  echo "Error: halloy config file not found at $config_file" >&2
  exit 1
fi

write_if_changed() {
  local target="$1" tmp="$2"
  if ! cmp -s "$target" "$tmp"; then
    cat "$tmp" >"$target"
  fi
  rm -f "$tmp"
}

# `theme` is a root key: it must appear before the first [section] header,
# so all matching/rewriting below is confined to that root region.
if awk '/^\[/ { exit 1 } /^theme[[:space:]]*=[[:space:]]*"ptm"[[:space:]]*(#.*)?$/ { found = 1 } END { exit !found }' "$config_file"; then
  : # already set
elif awk '/^\[/ { exit 1 } /^theme[[:space:]]*=/ { found = 1 } END { exit !found }' "$config_file"; then
  # A root-level theme key exists with a different value: replace it in place.
  tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
  awk '
    /^\[/ { in_section = 1 }
    !in_section && /^theme[[:space:]]*=/ { print "theme = \"ptm\""; next }
    { print }
  ' "$config_file" >"$tmp_file"
  write_if_changed "$config_file" "$tmp_file"
else
  # No root-level theme key: insert one before the first section header,
  # or append at the end if the file has no sections at all.
  tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
  awk '
    /^\[/ && !inserted { print "theme = \"ptm\""; print ""; inserted = 1 }
    { print }
    END { if (!inserted) print "theme = \"ptm\"" }
  ' "$config_file" >"$tmp_file"
  write_if_changed "$config_file" "$tmp_file"
fi

pkill -SIGUSR1 halloy >/dev/null 2>&1 || true
