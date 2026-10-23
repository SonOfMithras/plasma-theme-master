#!/usr/bin/env bash
set -euo pipefail

data_dir="${XDG_DATA_HOME:-$HOME/.local/share}/konsole"

rm -f "$data_dir/ptm.colorscheme"
rm -f "$data_dir/ptm.colorscheme.general.bak"
rm -f "$data_dir/PlasmaMaster.colorscheme"
rm -f "$data_dir/PlasmaMaster.profile"
rm -f "$data_dir/PTM.profile"
