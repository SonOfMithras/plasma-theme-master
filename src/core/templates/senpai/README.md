# senpai

[senpai](https://sr.ht/~delthas/senpai/) is a terminal IRC client made for bouncers.

Its config format (`scfg`) has no include directive, so it can't read a separate generated theme
file the way apps like kitty or bat can. `apply.sh` renders the theme to its own file
(`themes/ptm.scfg`) and merges the `colors { }` block from it directly into the user's real
`senpai.scfg`, replacing only that block and leaving everything else (server address, nickname,
highlight keywords, etc.) untouched. Idempotent: running it twice with the same colors leaves the
file's content and mtime unchanged.

senpai's own `colors { }` schema only exposes `prompt`, `unread`, `status`, and `nicks` (see
`senpai(5)`), so that's all this template can drive. Everything else in the UI (the main
background, borders, the buffer-list selection highlight) follows the terminal's own ANSI-16
palette, so it stays correctly themed as long as the terminal itself is PTM-themed too.

Tested against senpai 0.5.0.
