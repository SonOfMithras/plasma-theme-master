# LibreOffice

Tested against LibreOffice 26.2.4.2 (Flatpak, `org.libreoffice.LibreOffice`).

## Why this ships as an extension, not a rendered config file

LibreOffice's main window chrome (menus, toolbars, buttons, dialogs) is drawn by its own widget
framework, not read from `gtk.css`. Confirmed live: an unmistakable
`window { background-color: #ff0000; }` override in
`~/.var/app/org.libreoffice.LibreOffice/config/gtk-3.0/gtk.css` had zero effect, even with the
GTK3 VCL plugin explicitly forced (`SAL_USE_VCLPLUGIN=gtk3`). LibreOffice's real theming
mechanism is its own **ColorScheme** system (Tools > Options > LibreOffice > Application Colors),
distributed as an installable `.oxt` extension package, the same format real community themes
like [libre-nord](https://github.com/gomesluiz/libre-nord) use.

## The property list is confirmed against LibreOffice's own source, not guessed

Every `ColorScheme` property name and its real meaning is confirmed against
[`officecfg/registry/schema/org/openoffice/Office/UI.xcs`](https://github.com/LibreOffice/core/blob/master/officecfg/registry/schema/org/openoffice/Office/UI.xcs)
in the LibreOffice/core repository, not a partial third-party example. This mattered in practice:
`FieldColor`'s own schema description is "Allows to overwrite the system field color", it
controls input/dropdown widget backgrounds, not document-embedded field shading the way its name
first suggests. An earlier pass mapped it to a literal document-shading color by analogy with a
third-party reference theme, and dropdown boxes, the Options sidebar, and text fields stayed on
a stock dark color as a result, exactly the kind of "confirmed working" that only turns out to
be partially true once someone actually looks closely. Fixed by mapping `FieldColor` to a real
PTM surface role, then cross-checking the *entire* property list against the schema (several
other properties, `DisabledColor`, `ActiveTextColor`, `InactiveColor`, and their pairs, were
missing from the theme entirely and are now set too).

Colors below are hex placeholders in the template source; `Theme_Colors.xcu` uses this schema's
own decimal-integer format, not hex, so `apply.sh` converts every value at render time (plain
`int(hex, 16)`, confirmed against the Document Foundation's own blog post examples).

## Document canvas and BASIC editor colors stay literal, deliberately

Same reasoning as darktable's neutral photo-viewing background: `DocColor`, `FontColor`,
`DocBoundaries`, `CalcGrid`, and the BASIC IDE syntax-highlighting colors are kept as sensible
literal light/dark values regardless of how dark the surrounding chrome is. A document is still
meant to look like a normal page (or be printed), and code syntax highlighting needs
distinguishable, conventional colors (keywords, strings, comments), not four or five UI accent
roles stretched over a dozen token types. These are functional, not chrome.

## Real gotchas found building this, not theoretical

- **The scheme has to be selected manually, once.** Installing the extension registers "PTM"
  as an available option in the Scheme dropdown, it does not auto-select it. Confirmed live: same
  behavior class as darktable's own theme dropdown. Go to Tools > Options > LibreOffice >
  Application Colors, pick "PTM" from the Scheme dropdown, click Apply/OK. After that one
  manual step, every later theme change re-renders and reinstalls the extension automatically
  through PTM's own template pipeline, no further manual selection needed.
- **A restart is needed to see updated colors**, same as Heroic's own documented limitation.
  Confirmed live: switching PTM's theme mode while LibreOffice was already open did not
  change the running window at all; closing and reopening it did. This is not a bug in the
  template, extensions are loaded at LibreOffice startup.
- **`unopkg add --force` alone is not reliable for reinstalling the same extension identifier.**
  Confirmed live, the hard way: after several reinstalls it left the extension in a broken state,
  `unopkg list` reported nothing installed at all, while a stale binary cache index
  (`uno_packages/cache/uno_packages.pmap`) still pointed at a deleted temp directory, causing
  every subsequent install attempt to fail with "file opening ... NOT_EXISTING" until the whole
  cache directory was cleared by hand. `apply.sh` now explicitly `unopkg remove`s the extension
  identifier before every `add`, ignoring failure since it may not be installed yet, rather than
  relying on `--force` to handle that atomically.
- **`unopkg add` while LibreOffice is running can corrupt the install the same way.** `apply.sh`
  checks for a running `soffice.bin` process first and skips the install step entirely if found,
  rather than risk it, the `.oxt` is still rebuilt with the current colors either way, so the next
  render (once LibreOffice is closed) or the next manual re-apply installs them, and a restart is
  already required regardless.
- **`unopkg` is not on the sandbox's `PATH`** inside the Flatpak. Confirmed live:
  `flatpak run --command=unopkg ...` fails with "No such file or directory", the full path
  `/app/libreoffice/program/unopkg` is required.
- **If LibreOffice was open at the exact moment the theme changed, the install-skip above has no
  automatic retry.** The `.oxt` still gets rebuilt with the current colors every render, but
  since the install step is skipped while LibreOffice is running, closing LibreOffice alone is
  not enough, nothing re-triggers the install afterward on its own. Confirmed live: after
  changing the palette while LibreOffice was open, colors stayed stale even after closing and
  reopening it, until `apply.sh` was run again by hand with LibreOffice closed. In practice this
  means a color change made while LibreOffice happens to be open may need the theme re-applied
  once more (`ptm msg templates-apply`) after closing it, not just a restart.

## Native install

Same `unopkg` mechanism, invoked directly. `apply.sh` resolves `unopkg` from `PATH` first, then
falls back to the official TDF install layout (`/opt/libreoffice*/program/unopkg`,
`/usr/lib/libreoffice/program/unopkg`, `/usr/lib64/libreoffice/program/unopkg`) so the official
`.deb`/`.rpm` packages, which do **not** put `unopkg` on `PATH`, are detected too. If no native
`unopkg` is found it falls back to the Flatpak `flatpak run --command=` form.

**Verified live** against LibreOffice 26.2.4.2 from the official TDF `.deb` packages on
Kali Linux (amd64), including a clean-PATH run where `command -v unopkg` returned nothing and
`/opt/libreoffice26.2/program/unopkg` was picked up by the fallback; the extension installed
and registered correctly.

## Flatpak install (`org.libreoffice.LibreOffice`)

Extension cache lives at
`~/.var/app/org.libreoffice.LibreOffice/config/libreoffice/4/user/uno_packages/`. No
`flatpak override` needed.

## Status

Verified live against LibreOffice 26.2.4.2 (Flatpak) through PTM's own real render pipeline,
`community_ids` + local `community-templates/libreoffice/`, both dark and light mode, confirmed
after a restart in each case. Native install path is implemented but not live-tested.
