# Firefox

Synchronizes KDE Plasma desktop theme colors into Mozilla Firefox and derivatives using `userChrome.css`.

## How it works

1. Renders `userChrome.css` with active palette colors.
2. Invokes `plasma-theme-master-helper-firefox` to discover active Firefox profiles and inject `plasma-colors.css` into `chrome/`.
3. Adds `@import "plasma-colors.css";` to `chrome/userChrome.css` if not already present.

## Note
Ensure `toolkit.legacyUserProfileCustomizations.stylesheets` is set to `true` in `about:config` in Firefox.
