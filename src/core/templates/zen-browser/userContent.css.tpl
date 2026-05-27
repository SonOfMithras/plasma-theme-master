* {
  --base:           {{MD_SYS_COLOR_SURFACE_CONTAINER_LOWEST}};
  --surface:        {{MD_SYS_COLOR_SURFACE_CONTAINER_LOW}};
  --overlay:        {{MD_SYS_COLOR_SURFACE_CONTAINER}};
  --muted:          {{MD_SYS_COLOR_SURFACE_CONTAINER_HIGH}};
  --subtle:         {{MD_SYS_COLOR_SURFACE_CONTAINER_HIGHEST}};
  --text:           {{MD_SYS_COLOR_ON_SURFACE}};
  --secondary:      {{MD_SYS_COLOR_SECONDARY}};
  --primary:        {{MD_SYS_COLOR_PRIMARY}};
  --on_primary:     {{MD_SYS_COLOR_ON_PRIMARY}};
}

/* Common variables affecting all pages */
@-moz-document url-prefix("about:") {
  :root {
    --in-content-page-color: var(--secondary) !important;
    --color-accent-primary: var(--primary) !important;
    background-color: var(--on_primary) !important;
    --in-content-page-background: var(--overlay) !important;
  }

  /* Sync with theme */
  .main-heading,
  .main-search,
  #content,
  .sticky-container,
  #search-container {
    background-color: var(--base) !important;
    background: var(--base) !important;
  }

  moz-toggle {
    --toggle-background-color: var(--muted) !important;
    --toggle-background-color-pressed: var(--primary) !important;
    --toggle-background-color-hover: var(--muted) !important;
    --toggle-background-color-active: var(--muted) !important;
    --toggle-background-color-pressed-hover: var(--primary) !important;
    --toggle-background-color-pressed-active: var(--primary) !important;
    --toggle-border-color: transparent !important;
    --toggle-dot-background-color: var(--text) !important;
    --toggle-dot-background-color-hover: var(--text) !important;
    --toggle-dot-background-color-active: var(--text) !important;
    --toggle-dot-background-color-on-pressed: var(--primary) !important;
    --toggle-dot-margin: 3px !important;
  }

  /* Input fields styling */
  moz-input-search,
  moz-input-text,
  input[type="text"],
  input[type="search"],
  input:not([type="checkbox"]):not([type="radio"]) {
    background-color: var(--muted) !important;
    color: var(--text) !important;
    border-color: var(--subtle) !important;
    border-radius: 8px !important;
    --input-text-background-color: var(--text) !important;
    --input-text-color: var(--text) !important;
    --input-text-border-color: var(--subtle) !important;
    --input-text-border-radius: 8px !important;
  }

  /* Message Bar styling */
  moz-message-bar {
    --message-bar-background-color: var(--overlay) !important;
    --message-bar-border-color: var(--subtle) !important;
    --message-bar-icon-color: var(--primary) !important;
    background-color: var(--overlay) !important;
  }
  
  select#input {
    background: var(--subtle) !important;
    color: var(--text) !important;
  }
  
  .toggle-button {
    background-color: var(--muted) !important;
    border-color: transparent !important;
  }

  .toggle-button::before {
    background-color: var(--text) !important;
    margin: 3px !important;
  }

  .toggle-button[aria-pressed="true"],
  .toggle-button[checked],
  .toggle-button:checked {
    background-color: var(--primary) !important;
    border-color: transparent !important;
  }

  .toggle-button[aria-pressed="true"]::before,
  .toggle-button[checked]::before,
  .toggle-button:checked::before {
    background-color: var(--on_primary) !important;
  }
}

/* Variables and styles specific to about:newtab and about:home */
@-moz-document url("about:newtab"), url("about:home") {
  :root {
    --newtab-background-color: var(--overlay) !important;
    --newtab-background-color-secondary: var(--muted) !important;
    --newtab-element-hover-color: var(--muted) !important;
    --newtab-text-primary-color: var(--text) !important;
    --newtab-wordmark-color: var(--text) !important;
    --newtab-primary-action-background: var(--primary) !important;
  }

  .icon, .card-outer:is(:hover, :focus, .active):not(.placeholder) .card-title {
    color: var(--primary) !important;
  }
}

/* Variables and styles specific to about:preferences */
@-moz-document url-prefix("about:preferences") {
  :root {
    --zen-colors-tertiary: var(--surface) !important;
    --in-content-text-color: var(--text) !important;
    --link-color: var(--secondary) !important;
    --zen-colors-primary: var(--muted) !important;
    --in-content-box-background: var(--muted) !important;
    --zen-primary-color: var(--primary) !important;
    
    /* Cover dialogs that use default in-content variables */
    --in-content-page-background: var(--base) !important;
    --in-content-dialog-background: var(--base) !important;
  }
  
  div.container {
    background: var(--surface) !important;
    color: var(--text) !important;
  }
  
  div.picker-item.image-item {
    background: var(--overlay) !important;
    color: var(--text) !important;
  }

  button,
  groupbox menulist {
    background: var(--muted) !important;
    color: var(--text) !important;
  }

  moz-card, .main-content {
    background-color: var(--base) !important;
  }

  li.status-ok::before {
    -moz-context-properties: fill !important;
    fill: var(--primary) !important;
    color: var(--primary) !important;
  }

  .text-content.has-icon > img.icon {
    fill: var(--primary) !important;
    stroke: var(--primary) !important;
  }

	vbox.navigation, vbox#updateSettingsContainer.info-box-container {
		background: var(--overlay) !important;
		color: var(--text) !important;
	}

  /* Container tabs selected row */
  richlistitem[selected="true"] {
    background-color: var(--primary) !important;
    color: var(--base) !important;
  }

  /* Only set child text color for non-button elements */
  richlistitem[selected="true"] *:not(button):not(button *) {
    color: var(--base) !important;
  }

  /* Hover states for Settings and Remove buttons */
  richlistitem button:hover {
    background-color: var(--subtle) !important;
  }
  
  /* Sub-dialog popup frame (this wrapper is in the about:preferences document) */
  dialog,
  dialog#containers-dialog,
  .dialogBox,
  .dialog-content-box {
    background-color: var(--base) !important;
    border: 1px solid var(--subtle) !important;
    border-radius: 8px !important;
  }
  
  /* Sub-dialog Close (X) button hover effect */
  button.dialogClose {
    border-radius: 4px !important;
    transition: background-color 0.2s ease !important;
  }
  button.dialogClose:hover {
    background-color: var(--subtle) !important;
  }

  /* Back arrow button */
  .back-button,
  .subcategory-back-button,
  .page-back-button,
  .navigation-back-button,
  moz-button[ghost="true"] button,
  moz-button[type="ghost"] button,
  button:has(img[src*="arrow-left.svg"]) {
    appearance: none !important;
    background-color: transparent !important;
    border: none !important;
    box-shadow: none !important;
    list-style-image: url("chrome://global/skin/icons/arrow-left.svg") !important;
    color: var(--text) !important;
    -moz-context-properties: fill, fill-opacity !important;
    fill: var(--text) !important;
    width: 32px !important;
    height: 32px !important;
    padding: 0 !important;
    display: flex !important;
    justify-content: center !important;
    align-items: center !important;
  }

  /* Reset inner elements of moz-button back buttons */
  moz-button.back-button::part(button),
  moz-button.subcategory-back-button::part(button),
  moz-button.page-back-button::part(button),
  moz-button.navigation-back-button::part(button),
  moz-button[ghost="true"]::part(button),
  moz-button[type="ghost"]::part(button) {
    background-color: transparent !important;
    border: none !important;
    box-shadow: none !important;
  }

  .back-button .button-icon,
  .back-button .toolbarbutton-icon,
  .subcategory-back-button .button-icon,
  .subcategory-back-button .toolbarbutton-icon,
  .page-back-button .button-icon,
  .page-back-button .toolbarbutton-icon,
  .navigation-back-button .button-icon,
  .navigation-back-button .toolbarbutton-icon {
    -moz-context-properties: fill, fill-opacity !important;
    fill: var(--text) !important;
    color: var(--text) !important;
    margin: 0 !important;
  }

  .back-button:hover,
  .subcategory-back-button:hover,
  .page-back-button:hover,
  .navigation-back-button:hover,
  moz-button.back-button:hover button,
  moz-button.subcategory-back-button:hover button,
  moz-button.page-back-button:hover button,
  moz-button.navigation-back-button:hover button,
  moz-button[ghost="true"]:hover button,
  moz-button[type="ghost"]:hover button,
  moz-button.back-button:hover::part(button),
  moz-button.subcategory-back-button:hover::part(button),
  moz-button.page-back-button:hover::part(button),
  moz-button.navigation-back-button:hover::part(button),
  moz-button[ghost="true"]:hover::part(button),
  moz-button[type="ghost"]:hover::part(button),
  button:has(img[src*="arrow-left.svg"]):hover {
    background-color: var(--subtle) !important;
    border-radius: 4px !important;
  }
}

/* --- All Preferences Sub-Dialogs (Containers, Fonts, Connection, Language, etc.) --- */
@-moz-document url-prefix("chrome://browser/content/preferences/dialogs/"),
               url("chrome://browser/content/preferences/connection.xhtml"),
               url("chrome://browser/content/preferences/connections.xhtml"),
               url("chrome://browser/content/preferences/fonts.xhtml"),
               url("chrome://browser/content/preferences/containers.xhtml") {
  window,
  dialog,
  #FontsDialog {
    background-color: var(--base) !important;
    color: var(--text) !important;
  }

  /* Inner content area with Name/Color/Icon fields */
  #containers-content,
  .contentPane,
  vbox {
    background-color: var(--overlay) !important;
    color: var(--text) !important;
    padding: 14px !important;
  }

  /* Labels and text elements */
  label,
  description,
  groupbox,
  checkbox {
    color: var(--text) !important;
  }

  separator {
    border-color: var(--subtle) !important;
  }

  /* Text input for container name / sizes / urls */
  *|input:not([type="checkbox"]):not([type="radio"]), *|textarea {
    background-color: var(--surface) !important;
    color: var(--text) !important;
    border-color: var(--subtle) !important;
    border-radius: 6px !important;
    transition: border-color 0.15s ease !important;
  }

  *|input:not([type="checkbox"]):not([type="radio"]):focus,
  *|textarea:focus {
    border-color: var(--primary) !important;
    outline: 1px solid var(--primary) !important;
    outline-offset: -1px !important;
  }

  /* Dropdowns */
  menulist, *|select {
    background-color: var(--surface) !important;
    color: var(--text) !important;
    border: 1px solid var(--subtle) !important;
    border-radius: 6px !important;
  }

  /* Tree Columns (List headers) */
  treecol {
    appearance: none !important;
    background-color: var(--surface) !important;
    color: var(--text) !important;
    border: none !important;
    box-shadow: none !important;
    border-radius: 0 !important;
  }

  /* Language Selection Box / Lists */
  richlistbox {
    background-color: var(--surface) !important;
    color: var(--text) !important;
    border-color: var(--subtle) !important;
  }

  richlistitem {
    background-color: transparent !important;
    color: var(--text) !important;
  }

  /* Override AccentColor for the whole document */
  :root {
    --color-accent-primary: var(--primary);
    --color-accent-primary-hover: var(--primary);
    --color-accent-primary-active: var(--primary);
    --checkbox-checked-bgcolor: var(--primary);
    --checkbox-checked-border-color: var(--primary);
    --radio-checked-bgcolor: var(--primary);
    --radio-checked-border-color: var(--primary);
  }

  /* accent-color on HTML host elements */
  checkbox,
  radio,
  input[type="checkbox"],
  input[type="radio"] {
    accent-color: var(--primary) !important;
  }

  /* XUL Radio & Checkbox Native Appearance Stripping & Manual Styling */
  radio > .radio-check,
  checkbox > .checkbox-check {
    -moz-appearance: none !important;
    appearance: none !important;
    display: inline-block !important;
    box-sizing: border-box !important;
    width: 16px !important;
    height: 16px !important;
    border: 2px solid var(--subtle) !important;
    background-color: transparent !important;
    flex-shrink: 0 !important;
  }

  radio > .radio-check {
    border-radius: 50% !important;
  }

  checkbox > .checkbox-check {
    border-radius: 3px !important;
  }

  /* Radio: selected state — primary dot on base background */
  radio[selected="true"] > .radio-check,
  radio[selected] > .radio-check,
  radio:checked > .radio-check {
    border-color: var(--primary) !important;
    background-color: var(--base) !important;
    background-image: radial-gradient(circle, var(--primary) 0%, var(--primary) 40%, transparent 45%) !important;
    box-shadow: none !important;
  }

  /* Checkbox: checked state — primary fill with native SVG checkmark */
  checkbox[checked="true"] > .checkbox-check,
  checkbox[checked] > .checkbox-check,
  checkbox:checked > .checkbox-check {
    border-color: var(--primary) !important;
    background-color: var(--primary) !important;
    background-image: url("chrome://global/skin/icons/check.svg") !important;
    background-repeat: no-repeat !important;
    background-position: center !important;
    background-size: 12px !important;
    -moz-context-properties: fill !important;
    fill: var(--base) !important;
  }

  /* Done / Cancel buttons */
  button, dialog::part(button) {
    background-color: var(--muted) !important;
    color: var(--text) !important;
    border-radius: 6px !important;
    margin-top: 16px !important;
  }

  button:hover {
    background-color: var(--subtle) !important;
  }
}

/* --- Primary Password Dialog (chrome://mozapps) --- */
@-moz-document url("chrome://mozapps/content/preferences/changemp.xhtml"),
               url-prefix("chrome://mozapps/content/preferences/") {
  /* Outer frame */
  window,
  dialog,
  #changemp {
    background-color: var(--base) !important;
    color: var(--text) !important;
  }

  /* Content boxes — uniform overlay fill with padding */
  vbox, groupbox {
    background-color: var(--overlay) !important;
    color: var(--text) !important;
    padding: 10px !important;
  }

  /* Button row — same overlay fill, no extra padding */
  hbox {
    background-color: var(--overlay) !important;
    padding: 0 !important;
  }

  label, description {
    color: var(--text) !important;
    background-color: transparent !important;
  }

  /* Password inputs */
  *|input:not([type="checkbox"]):not([type="radio"]) {
    background-color: var(--surface) !important;
    color: var(--text) !important;
    border: 1px solid var(--subtle) !important;
    border-radius: 6px !important;
    transition: border-color 0.15s ease !important;
  }

  *|input:not([type="checkbox"]):not([type="radio"]):focus {
    border-color: var(--primary) !important;
    outline: 1px solid var(--primary) !important;
    outline-offset: -1px !important;
  }

  /* Password quality meter */
  meter {
    accent-color: var(--primary) !important;
  }

  /* Buttons — transparent so they match their surrounding background */
  button, dialog::part(button) {
    background-color: transparent !important;
    color: var(--text) !important;
    border: none !important;
    border-radius: 6px !important;
  }

  button:hover {
    box-shadow: 0 0 0 1px var(--subtle) !important;
    background-color: transparent !important;
  }
}

/* Variables and styles specific to about:addons */
@-moz-document url-prefix("about:addons") {
  :root {
    --zen-dark-color-mix-base: var(--surface) !important;
    --background-color-box: var(--overlay) !important;
  }

	div#sidebar {
		background: var(--surface) !important;
		color: var(--text) !important;
	}
}

/* Variables and styles specific to about:logins */
@-moz-document url-prefix("about:logins") {
  :root {
    --background-color-box: var(--overlay) !important;
    --in-content-box-background: var(--overlay) !important;
    --background-color-canvas: var(--subtle) !important;
  }

  header {
    background-color: var(--base) !important;
  }

  input[type="text"],
  input[type="password"],
  input[type="search"],
  input:not([type="checkbox"]):not([type="radio"]) {
    padding: 8px 12px !important;
  }

  /* Restore left padding for the search input to avoid icon overlap */
  input.filter:not([type="checkbox"]):not([type="radio"]) {
    padding-left: 32px !important;
  }
}

/* Variables and styles specific to about:protections */
@-moz-document url-prefix("about:protections") {
  :root {
    --zen-primary-color: var(--base) !important;
    --in-content-primary-button-text-color-hover: var(--text) !important;
    --in-content-primary-button-background: var(--overlay) !important;
    --in-content-primary-button-text-color: var(--text) !important;
  }

	.body {
		background: var(--overlay) !important;
	}

  .card {
    background-color: var(--base) !important;
  }

	div#social.tab-content {
		background-color: var(--overlay) !important;
		color: var(--text) !important;
	}
}

/* Variables and styles specific to about:config */
@-moz-document url-prefix("about:config") {
  div#toolbar {
    background: var(--primary) !important;
  }

  button#warningButton {
    background-color: var(--secondary) !important;
    color: var(--base) !important;
  }

  button#warningButton:hover {
    background-color: color-mix(in srgb, var(--secondary) 80%, transparent) !important;
    color: var(--base) !important;
  }
}

@-moz-document url-prefix("about:cache"), url-prefix("about:profiles") {
	td {
		background-color: var(--surface) !important;
		color: var(--text) !important;
	}
}

@-moz-document url-prefix("about:profiles") {
  div.action-box {
    background-color: var(--overlay) !important;
    color: var(--text) !important;
  }
}

@-moz-document url-prefix("about:home") {
	div.tile {
		background-color: var(--base) !important;
	}
}

/* Custom text selection styling */
::selection {
  background-color: var(--primary) !important;
  color: var(--base) !important;
}

/* Style checkboxes and radio buttons globally in content */
*|input[type="checkbox"],
[type="checkbox"],
*|input[type="radio"],
[type="radio"] {
  accent-color: var(--primary) !important;
}

/* Style external links globally in content (like New Tab settings) */
.external-link {
  color: var(--secondary) !important;
}
.external-link:hover {
  color: color-mix(in srgb, var(--secondary) 70%, transparent) !important;
}

/* Style new tab customize panel and its wrapper with a darker base background */
.customize-menu-animate-wrapper {
  background-color: var(--base) !important;
}
 .customize-menu,
.close-button-wrapper {
  background-color: transparent !important;
}

/* Style personalize (Customize) button on New Tab page */
.personalize-button {
  background-color: color-mix(in srgb, var(--subtle) 40%, transparent) !important;
  color: var(--text) !important;
  border: 1px solid color-mix(in srgb, var(--subtle) 20%, transparent) !important;
  backdrop-filter: blur(10px);
  transition: background-color 0.2s ease, transform 0.2s ease !important;
}
