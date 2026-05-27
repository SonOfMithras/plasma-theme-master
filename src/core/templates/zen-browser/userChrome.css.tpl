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

:root {
  /* --- Zen Color Variables --- */
  --zen-primary-color: var(--primary) !important;
  --zen-colors-primary: var(--base) !important;
  --zen-colors-secondary: var(--surface) !important;
  --zen-colors-tertiary: var(--base) !important;
  --zen-colors-border: var(--primary) !important;

  /* --- Backgrounds --- */
  --toolbar-bgcolor: var(--base) !important;
  --newtab-background-color: var(--overlay) !important;
  --zen-themed-toolbar-bg: var(--base) !important;
  --zen-main-browser-background: var(--base) !important;
  --toolbox-bgcolor-inactive: var(--base) !important;
  --zen-urlbar-background: var(--surface) !important;
  --zen-colors-input-bg: var(--surface) !important;
  --zen-themed-toolbar-bg-transparent: transparent !important;

  /* --- Text & Icons --- */
  --lwt-text-color: var(--text) !important;
  --toolbarbutton-icon-fill: var(--primary) !important;
  --toolbar-field-color: var(--text) !important;
  --toolbar-field-focus-color: var(--text) !important;
  --toolbar-color: var(--text) !important;
  --toolbox-textcolor: var(--text) !important;
  --toolbox-textcolor-inactive: var(--text) !important;
  --tab-selected-textcolor: var(--primary) !important;
  --newtab-text-primary-color: var(--text) !important;

  /* --- Popups, Menus & Sidebars --- */
  --arrowpanel-color: var(--text) !important;
  --arrowpanel-background: var(--overlay) !important;
  --sidebar-text-color: var(--text) !important;
  --lwt-sidebar-text-color: var(--text) !important;
  --lwt-sidebar-background-color: var(--base) !important;
}

/* --- Sidebar & Layout --- */

#navigator-toolbox,
#zen-tabbox-wrapper,
.sidebar-panel {
  background-color: var(--base) !important;
}

#zen-sidebar-splitter {
  background-color: var(--base) !important;
  opacity: 1 !important;
}

.sidebar-placesTree {
  background-color: var(--surface) !important;
}

#zen-workspaces-button {
  background-color: var(--surface) !important;
}

#zen-appcontent-navbar-container {
  background-color: var(--base) !important;
}

/* --- Navigation Controls --- */

#back-button .toolbarbutton-icon,
#forward-button .toolbarbutton-icon,
#reload-button .toolbarbutton-icon,
#stop-button .toolbarbutton-icon,
#PanelUI-menu-button .toolbarbutton-icon,
#unified-extensions-button .toolbarbutton-icon,
#nav-bar-overflow-button .toolbarbutton-icon,
#zen-toggle-compact-mode .toolbarbutton-icon {
  fill: var(--primary) !important;
  color: var(--primary) !important;
}

#back-button[disabled] .toolbarbutton-icon,
#forward-button[disabled] .toolbarbutton-icon {
  opacity: 0.3 !important;
}

toolbar .toolbarbutton-1 {
  &:not([disabled]) {
    &:is([open], [checked]) > :is(
    .toolbarbutton-icon,
    .toolbarbutton-text,
    .toolbarbutton-badge-stack
    ) {
      fill: color-mix(in srgb, var(--text) 40%, transparent) !important;
    }
  }
}

.tab-text {
  color: var(--text);
}

toolbarbutton#tabs-newtab-button {
  color: var(--text) !important;
}
/* --- URL Bar --- */

#urlbar-input::selection {
  background-color: var(--primary) !important;
  color: var(--on_primary) !important;
}

.urlbar-background {
  background-color: var(--overlay) !important;
}

#urlbar-input-container {
  background-color: var(--base) !important;
}

.urlbarView-url {
  color: var(--primary) !important;
}

.urlbarView-favicon {
  background-color: transparent !important;
}

#urlbar-results {
  background-color: var(--overlay) !important;
  color: var(--text) !important;
}

.urlbarView-row:not([selected]) :is(.urlbarView-title, .urlbarView-action) {
  color: var(--text) !important;
}

/* --- UI Components --- */

#zen-toast-container {
  color: var(--primary) !important;
}

#zen-toast-container .zen-toast,
#zen-toast-container > .zen-toast,
.zen-toast {
  background-color: color-mix(in srgb, var(--overlay) 85%, transparent) !important;
  background-image: none !important;
  backdrop-filter: blur(8px) !important;
  --zen-primary-color: var(--overlay) !important;
  --zen-dialog-background: var(--overlay) !important;
}


#permissions-granted-icon {
  color: var(--base) !important;
}

.tab-background[selected=""] {
  background: var(--overlay) !important;
}

.content-shortcuts {
  background-color: var(--surface) !important;
  border-color: var(--secondary) !important;
}

#zenEditBookmarkPanelFaviconContainer {
  background: var(--base) !important;
}

/* --- Bookmark Panel Spacing --- */
.zenEditBMPanel_fieldContainer > label {
  margin-right: 12px !important;
}

#TabsToolbar, hbox#titlebar {
  background-color: var(--base) !important;
}

.zen-current-workspace-indicator-name {
  color: var(--text);
}

.zen-current-workspace-indicator-icon {	
	fill: var(--text) !important;
}

/* --- Fixing Select Dropdowns colors on Webpages --- */

#ContentSelectDropdown,
#ContentSelectDropdown > menupopup,
#ContentSelectDropdownPopup,
#ContentSelectDropdownPopup .menupopup-arrowscrollbox,
#ContentSelectDropdownPopup .panel-arrowcontent {
  --arrowpanel-background: var(--overlay) !important;
  --arrowpanel-color: var(--text) !important;
  --arrowpanel-border-color: transparent !important;
  --panel-border-color: transparent !important;
  background-color: var(--overlay) !important;
  background-image: none !important;
  --content-select-background-image: none !important;
  color: var(--text) !important;
  border: none !important;
  border-radius: 14px !important;
  overflow: hidden !important;
}

#ContentSelectDropdown menupopup menuitem,
#ContentSelectDropdownPopup richlistitem,
#ContentSelectDropdownPopup .ContentSelectDropdown-item {
  color: var(--text) !important;
  background-color: transparent !important;
}

#ContentSelectDropdown menupopup menuitem[_moz-menuactive="true"],
#ContentSelectDropdownPopup richlistitem[_moz-menuactive="true"],
#ContentSelectDropdownPopup .ContentSelectDropdown-item[_moz-menuactive="true"],
#ContentSelectDropdownPopup richlistitem:hover,
#ContentSelectDropdownPopup .ContentSelectDropdown-item:hover {
  background-color: var(--subtle) !important;
}

#ContentSelectDropdown menupopup menuitem[selected="true"],
#ContentSelectDropdownPopup richlistitem[selected="true"],
#ContentSelectDropdownPopup .ContentSelectDropdown-item[selected="true"] {
  background-color: var(--muted) !important;
}

/* Force inner text elements to inherit the primary color */
#ContentSelectDropdown menupopup menuitem[_moz-menuactive="true"] *,
#ContentSelectDropdownPopup richlistitem[_moz-menuactive="true"] *,
#ContentSelectDropdownPopup .ContentSelectDropdown-item[_moz-menuactive="true"] *,
#ContentSelectDropdownPopup richlistitem:hover *,
#ContentSelectDropdownPopup .ContentSelectDropdown-item:hover *,
#ContentSelectDropdown menupopup menuitem[selected="true"] *,
#ContentSelectDropdownPopup richlistitem[selected="true"] *,
#ContentSelectDropdownPopup .ContentSelectDropdown-item[selected="true"] * {
  color: var(--primary) !important;
}


/* Color for dialogs */
dialog {
	background: var(--base) !important;
	color: var(--text) !important;
}

/* Style standard dialog buttons (Cancel, Disable, etc.) */
dialog button,
panel button,
window[role="dialog"] button,
window[windowtype="alert:alert"] button,
.dialog-button-box button {
  appearance: none !important;
  background-color: var(--overlay) !important;
  color: var(--text) !important;
  border-radius: 4px !important;
  border: 1px solid color-mix(in srgb, var(--text) 10%, transparent) !important;
}
dialog button:hover,
panel button:hover,
window[role="dialog"] button:hover,
window[windowtype="alert:alert"] button:hover,
.dialog-button-box button:hover {
  background-color: color-mix(in srgb, var(--overlay) 90%, black) !important;
}
/* Style the Primary/Accept/OK button */
dialog button[dlgtype="accept"],
dialog button.primary,
dialog button[default="true"],
panel button.primary,
panel button[default="true"],
window[role="dialog"] button[default="true"],
window[windowtype="alert:alert"] button[default="true"],
.dialog-button-box button[dlgtype="accept"],
.dialog-button-box button.primary {
  background-color: var(--secondary) !important;
  color: var(--base) !important;
}
dialog button[dlgtype="accept"]:hover,
dialog button.primary:hover,
dialog button[default="true"]:hover,
panel button.primary:hover,
panel button[default="true"]:hover,
window[role="dialog"] button[default="true"]:hover,
window[windowtype="alert:alert"] button[default="true"]:hover,
.dialog-button-box button[dlgtype="accept"]:hover,
.dialog-button-box button.primary:hover {
  background-color: color-mix(in srgb, var(--secondary) 90%, black) !important;
}

/* Style dialog text links */
.text-link {
  color: var(--secondary) !important;
}
.text-link:hover {
  color: color-mix(in srgb, var(--secondary) 70%, transparent) !important;
}

/* Style checkboxes and radio buttons globally in UI */
*|input[type="checkbox"],
[type="checkbox"],
*|input[type="radio"],
[type="radio"] {
  accent-color: var(--primary) !important;
}

/* Custom text selection styling */
::selection {
  background-color: var(--primary) !important;
  color: var(--base) !important;
}

/* Style context menus hover */
menu:where([_moz-menuactive="true"]:not([disabled="true"])), 
menuitem:where([_moz-menuactive="true"]:not([disabled="true"])) {
  background-color: var(--primary) !important;
  color: var(--base) !important;
}
menu:where([_moz-menuactive="true"]:not([disabled="true"])) :is(.menu-text, .menu-iconic-text, .menu-accel):not(menupopup *), 
menuitem:where([_moz-menuactive="true"]:not([disabled="true"])) :is(.menu-text, .menu-iconic-text, .menu-accel) {
  color: var(--base) !important;
}

toolbarbutton.zen-glance-sidebar-close.toolbarbutton-1,
toolbarbutton.zen-glance-sidebar-open.toolbarbutton-1,
toolbarbutton.zen-glance-sidebar-split.toolbarbutton-1 {
	background-color: var(--overlay) !important;
}

toolbarbutton.zen-glance-sidebar-close.toolbarbutton-1:hover,
toolbarbutton.zen-glance-sidebar-open.toolbarbutton-1:hover,
toolbarbutton.zen-glance-sidebar-split.toolbarbutton-1:hover {
	background-color: color-mix(in srgb, var(--muted) 90%, black) !important;
}

/* --- Extensions Hover Style --- */
.subviewbutton:hover,
.unified-extensions-item:hover,
.unified-extensions-item-action-button:hover,
#zen-site-data-addons .unified-extensions-list > *:hover,
#zen-site-data-addons #unified-extensions-area > *:hover,
#zen-site-data-addons #overflowed-extensions-list > *:hover {
  background-color: var(--subtle) !important;
}

/* --- Autocomplete Popup (Login/Password Dropdown) --- */
#PopupAutoComplete {
  --panel-background: var(--overlay) !important;
  --panel-color: var(--text) !important;
  --panel-border-color: transparent !important;
  border: none !important;
  outline: none !important;
  box-shadow: none !important;
}

panel[type="autocomplete-richlistbox"],
#PopupAutoComplete > richlistbox,
.autocomplete-richlistbox {
  background-color: var(--overlay) !important;
  color: var(--text) !important;
  border: none !important;
  border-radius: 8px !important;
  overflow-y: auto !important;
}

/* Individual autocomplete items */
.autocomplete-richlistbox richlistitem,
.autocomplete-richlistitem {
  background-color: transparent !important;
  color: var(--text) !important;
  border: none !important;
}

/* Hover state */
.autocomplete-richlistbox richlistitem:hover,
.autocomplete-richlistitem:hover {
  background-color: var(--muted) !important;
  color: var(--text) !important;
}

/* Selected / focused state */
.autocomplete-richlistbox richlistitem[selected],
.autocomplete-richlistitem[selected] {
  background-color: var(--primary) !important;
  color: var(--base) !important;
}

/* Inner text and icon elements in selected items */
.autocomplete-richlistbox richlistitem[selected] *,
.autocomplete-richlistitem[selected] * {
  color: var(--base) !important;
  fill: var(--base) !important;
}

/* Settings gear icon in selected autocomplete items */
.autocomplete-richlistbox richlistitem[selected] .ac-secondary-action,
.autocomplete-richlistitem[selected] .ac-secondary-action {
  background-color: transparent !important;
  fill: var(--base) !important;
  color: var(--base) !important;
  -moz-context-properties: fill, fill-opacity !important;
}

/* Footer and warning areas in the popup */
#PopupAutoComplete .autocomplete-richlistbox-popup,
#PopupAutoComplete .popup-notification-body {
  background-color: var(--overlay) !important;
  color: var(--text) !important;
}

/* --- Identity/Site Data Panel Icons --- */
/* Bookmark, Send Notifications, Tracking Protection backgrounds */
#zen-site-data-header toolbarbutton.active::before,
.permission-popup-permission-icon::before,
.permission-popup-boost-icon::before,
.identity-popup-protections-icon::before,
#tracking-protection-icon-container::before {
  background-color: var(--primary) !important;
  opacity: 0.2 !important; /* using a translucent background to match Zen's design */
}

/* Update the icon fill color to match the primary color for a consistent theme */
#zen-site-data-header toolbarbutton.active,
.permission-popup-permission-icon,
.permission-popup-boost-icon,
.identity-popup-protections-icon,
#tracking-protection-icon-container {
  color: var(--primary) !important;
  fill: var(--primary) !important;
}

/* Hover states for Identity/Site Data panel icons */
#zen-site-data-header toolbarbutton:hover::before,
.permission-popup-permission-item:hover .permission-popup-permission-icon::before,
.permission-popup-boost-item:hover .permission-popup-boost-icon::before,
.identity-popup-protections-icon:hover::before,
#tracking-protection-icon-container:hover::before,
#identity-popup-security-button:hover::before {
  background-color: var(--subtle) !important;
  opacity: 1 !important;
}

/* --- Bookmark Panel Folder Tree Styling --- */
#editBMPanel_folderTree {
  background-color: var(--subtle) !important;
  background-image: none !important;
  color: var(--text) !important;
  border-radius: 8px !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-row {
  background-color: transparent !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-row(selected) {
  background-color: var(--primary) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-cell-text {
  color: var(--text) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-cell-text(selected) {
  color: var(--on_primary) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-image {
  fill: var(--text) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-image(selected) {
  fill: var(--on_primary) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-twisty {
  fill: var(--text) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-twisty(selected) {
  fill: var(--on_primary) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-twisty {
  fill: var(--text) !important;
}

#editBMPanel_folderTree treechildren::-moz-tree-twisty(selected) {
  fill: var(--on_primary) !important;
}

/* Style the About Zen Browser Dialog background */
@-moz-document url("chrome://browser/content/aboutDialog.xhtml") {
  #aboutDialogContainer,
  #clientBox {
    background-color: var(--base) !important;
    background-image: none !important;
  }

  #bottomBox {
    background-color: var(--overlay) !important;
    background-image: none !important;
  }
}
