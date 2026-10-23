# Community templates

<p align="center">
  <img src="https://assets.ptm.dev/ptm-logo.svg?v=2" alt="PTM Logo" style="width: 192px" />
</p>

---

Templates keep your other apps in sync with the [PTM](https://github.com/ptm-dev/ptm) palette.
When the theme changes, PTM renders each enabled template with the new colors and writes the result into that
app's config, so your terminal, launcher, editor and browser follow along.

This repo is the **community** template source. Merged templates are served from `https://api.ptm.dev/templates`
and show up in Settings -> Templates, where users enable the ones they want. **PRs are welcome.**

## Layout

One top-level directory per app, named after the template id:

```
fuzzel/
  template.toml           # the manifest: catalog entry, and what gets rendered where
  fuzzel.conf             # the input file, written in PTM's template syntax
  apply.sh                # optional hook, run after rendering
  README.md               # required, for app-specific setup notes
```

## The manifest

```toml
[catalog.fuzzel]            # the id users enable; must match the directory name
name     = "Fuzzel"         # display name in Settings
category = "launcher"

[templates.fuzzel]          # one or more render entries
input_path  = "fuzzel.conf"                              # a file in this directory
output_path = "$XDG_CONFIG_HOME/fuzzel/themes/ptm"  # where the rendered file lands
post_hook   = "bash '{{ config_dir }}/apply.sh'"         # optional, runs after the write
```

`category` must be one of `ai`, `audio`, `browser`, `chat`, `compositor`, `editor`, `gaming`, `launcher`, `system`,
`terminal`, `misc`. Prefer an existing bucket over inventing a category of one; keep `misc` for true one-offs.

An app can declare **several entries**. That is how `discord/` renders three styles across nine Discord clients: one
`[templates.<entry>]` table per output, all under a single `[catalog.discord]`.

Beyond the basics:

| Key | What it does |
| --- | --- |
| `output_path` | A single path, or an array of paths to write the same render to. |
| `output_path_dynamic` | A command whose stdout is the output path, for apps whose config location has to be discovered at apply time (see `obsidian/`). |
| `input_path_modes` | `{ dark = "...", light = "..." }` when dark and light need different source files. |
| `input_path_dynamic` | A command whose stdout is the input path. |
| `pre_hook` / `post_hook` | Shell run before or after the write. |
| `undo_hook` | Shell run when the user disables the template (or all community templates), to remove what `post_hook` added to the app's config. Retried on the next theme change until it exits 0. |
| `post_action` | Named ptm built-in after the write (not shell). Currently: `firefox-theme`. |
| `requires_path` | Skip the entry unless this path exists, so one template can ship entries for apps a user may not have. |

Output paths expand a leading `$XDG_CONFIG_HOME`, `$XDG_DATA_HOME`, `$XDG_STATE_HOME`, `$XDG_CACHE_HOME`, or `~`.

**`{{ config_dir }}` is your template's own directory.** On a user's machine your files are cached in a folder of their
own, and `{{ config_dir }}` points at it, so a hook names a file you ship directly: `bash '{{ config_dir }}/apply.sh'`.
It is not the repo root, and not the app's config directory. CI rejects a hook that reaches for a file the template
does not ship.

## Writing the template body

The input file is the app's own config, with PTM's template syntax wherever a color goes:

```css
window {
  background-color: {{ colors.surface.default.hex }};
  color: {{ colors.on_surface.default.hex }};
}
```

The full reference is in the docs: [Templates](https://docs.ptm.dev/v5/theming/templates/) covers the syntax, every
color token, filters (`| darken(0.2)`, `| hex_stripped`), loops and conditionals.
[App theming](https://docs.ptm.dev/v5/theming/app-theming/) covers how templates get applied.

## Hooks run as the user

A `pre_hook`, `post_hook`, or `undo_hook` is shell that runs on the user's machine, as them; pre and post hooks run every
time the theme changes. Rendering a file is inert; a hook is not. So:

- Keep `apply.sh` short and readable. A reviewer has to be able to see everything it does.
- Make it **idempotent**. It runs on every theme change, so it must never append the same line twice. See
  `walker/apply.sh` for the pattern: check first, rewrite in place, append only as a last resort.
- Fail loudly when the app's config is missing (non-zero exit, message on stderr) instead of creating something the user
  never asked for.
- Download nothing, and execute nothing you did not ship in this directory.
- Touch only that app's own config.
- If `post_hook` edits the app's config (an include, a theme selection), ship an `undo_hook` that reverts exactly that
  edit and is a no-op when there is nothing to revert. A failed undo is retried, so it must be idempotent too.

## Test it before you PR

You do not need a fork of the shell. Point a **user template** at your working copy in `~/.config/ptm/config.toml`:

```toml
[theme.templates.user.my_app]
input_path  = "~/dev/community-templates/my_app/my_app.css"
output_path = "$XDG_CONFIG_HOME/my_app/theme.css"
post_hook   = "bash '~/dev/community-templates/my_app/apply.sh'"
```

Confirm it is picked up:

```sh
ptm theme --list-templates
```

Then change your theme, or re-apply the current one, and check the app updates. Test **dark and light**, and apply twice
to prove the hook is idempotent.

One wrinkle: a *user* template's `{{ config_dir }}` is your PTM config directory, not the template folder, which is
why the hook above spells out a full path. Once merged, the community copy resolves `{{ config_dir }}` to its own cached
folder, so the manifest you submit uses `{{ config_dir }}/apply.sh`.

## Submitting

Open a PR against `main`. CI validates every manifest on each push: the catalog id matches the directory, the category is
known, every `input_path` exists, every entry writes somewhere, hooks only reference files you ship, and every template
has a non-empty `README.md` (exact filename).

- **One app per PR.**
- Include a `README.md` explaining what is themed, prerequisites, setup, generated config paths, and how changes take
  effect. Document hook side effects and overwrite caveats where applicable; say explicitly when no manual setup is needed.
- Include a screenshot of the app with the theme applied.
- Say which version of the app you tested against.
- Do not commit rendered output or your own personal app config.

Maintainers read hooks line by line before merging.

## Help

- [Documentation](https://docs.ptm.dev)
- [Discord](https://discord.ptm.dev)
- Bugs in PTM itself, rather than in a template, belong in
  [ptm](https://github.com/ptm-dev/ptm/issues).
