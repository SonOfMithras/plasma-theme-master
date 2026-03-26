:root {
  --plasma-window-bg: {{WINDOW_BG}};
  --plasma-window-fg: {{WINDOW_FG}};
  --plasma-view-bg: {{VIEW_BG}};
  --plasma-view-fg: {{VIEW_FG}};
  --plasma-accent: {{ACCENT}};
}
/* Automated Theme Override */
#navigator-toolbox { background-color: var(--plasma-window-bg) !important; }
.tab-background[selected="true"] { background-color: var(--plasma-accent) !important; color: #fff !important; }
/* Zen Browser Specifics */
