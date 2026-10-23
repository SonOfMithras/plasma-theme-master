# Strata

Renders the active PTM palette as a Strata custom theme
(`ptm.toml`, shown in Strata as "PTM").

Background, surface, text, accent, danger, borders and dim text follow the
base tokens; code-preview colors follow the terminal palette. No reload hook
is needed: the file is re-read the next time Strata starts.

Note: Strata v1.0.0 reads custom themes at startup, so restart Strata after a
palette change to see it. (If it was already running before this file existed
for the first time, restart once so it picks the theme up at all.)
