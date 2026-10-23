# fastfetch

Themes [fastfetch](https://github.com/fastfetch-cli/fastfetch) — sets `display.color.keys`,
`display.color.title`, `display.percent.color.{green,yellow,red}` and `logo.color.{1,2}`.

fastfetch's config (`~/.config/fastfetch/config.jsonc`) has no include/merge directive, so
`apply.sh` merges the rendered color object into the user's real config.jsonc directly (`jq -s
'.[0] * .[1]'`), preserving everything else in it (modules list, custom module config, etc.).

**Known limitation: strict JSON only, not full JSONC.** `jq` (used for the merge) only parses
strict JSON — `//`/`/* */` comments and trailing commas, both valid JSONC, will make `apply.sh`
fail. It fails loudly with a clear message in that case rather than silently corrupting the file
or applying nothing with no explanation. If your `config.jsonc` uses comments, remove them to use
this template (fastfetch's own `--gen-config`/`--gen-config-force` output has none by default).
Also fails loudly (rather than auto-creating one) if `config.jsonc` doesn't exist at all yet —
run fastfetch once first to generate a default config.

**Idempotency note**: the merge is guarded by comparing only the `logo`/`display` sub-objects
semantically (`jq -S`), not the whole file's raw bytes — a naive whole-file `cmp` after a `jq`
merge will never actually match hand-formatted JSON (`jq` re-serializes with its own canonical
formatting), which silently defeats the guard and rewrites the file on every apply regardless of
whether colors changed. Verified idempotent: running `apply.sh` twice with unchanged colors
leaves the file's mtime and content completely untouched.

**Known upstream bug (worked around here): `logo.color` silently drops pastel/light colors.**
Confirmed live against fastfetch 2.66.0: a `logo.color.N` value is dropped with no error — the
logo falls back to its default palette — whenever its truecolor SGR string (`38;2;R;G;B`) is
exactly 16 characters, i.e. every one of R/G/B is a 3-digit number (`>=100`). That covers a large
share of real accent colors, including most pastel/light theme colors. `display.color.keys`/
`title`/`percent` are unaffected — only `logo.color.N` goes through the broken code path.
Isolated by testing the exact byte-length trigger directly (both via `--logo-color-N` and via this
same JSON key) with fully saturated colors of the same hue (worked) vs. desaturated/light colors
producing a 16-char string (failed), including a control test with a non-pastel color that still
had all three channels `>=100` (also failed) — confirming it's the string length, not the hue or
lightness itself.

`apply.sh` routes around it: whenever a `logo.color` value would trip the bug, the single
*smallest* RGB channel is nudged down to 99 — the minimal change that drops the SGR string below
16 characters, and since it's already the channel closest to the 100 boundary, it's the smallest
possible perceptual shift (e.g. `#fff59b` → `#fff563`). An earlier version of this fix quantized
to the nearest ANSI-256 cube color instead, but the 216-color cube only has 6 steps per channel,
so a near-white pastel like `#fff59b` snapped to `#ffffaf` — visually just white on a small ascii
logo. The channel-nudge keeps full 24-bit truecolor and stays close to the real color.

Tested against fastfetch 2.66.0.
