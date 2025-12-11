from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QListWidget, QPushButton, 
    QLabel, QInputDialog, QMessageBox, QTextEdit, QSplitter,
    QComboBox, QFormLayout, QTabWidget, QGroupBox, QStackedWidget
)
from PySide6.QtCore import Qt
import configparser
import io
from core.plasma import PlasmaThemeManager

class GlobalThemeEditorTab(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()
        self.refresh_list()

    def init_ui(self):
        layout = QHBoxLayout()
        
        # Left Side: List
        left_layout = QVBoxLayout()
        self.theme_list = QListWidget()
        self.theme_list.currentItemChanged.connect(self.on_theme_selected)
        
        refresh_btn = QPushButton("Refresh List")
        refresh_btn.clicked.connect(self.refresh_list)
        
        clone_btn = QPushButton("Clone Selected Theme")
        clone_btn.clicked.connect(self.clone_theme)
        
        left_layout.addWidget(QLabel("Installed Global Themes:"))
        left_layout.addWidget(self.theme_list)
        left_layout.addWidget(refresh_btn)
        left_layout.addWidget(clone_btn)
        
        left_widget = QWidget()
        left_widget.setLayout(left_layout)

        # Right Side: Editor (Tabs: Easy, Raw)
        self.editor_tabs = QTabWidget()
        
        # 1. Easy Edit Tab
        self.easy_edit_widget = QWidget()
        easy_layout = QFormLayout()
        
        # Define fields mapping
        # Label -> (Category, Section, Key)
        # Note: configparser reads '[kdeglobals][KDE]' as section 'kdeglobals][KDE'
        self.fields_map = [
            ("Application Style", "application_styles", "kdeglobals][KDE", "widgetStyle"),
            ("Color Scheme", "colors", "kdeglobals][General", "ColorScheme"),
            ("Icon Theme", "icons", "kdeglobals][Icons", "Theme"),
            ("Cursor Theme", "cursors", "kcminputrc][Mouse", "cursorTheme"),
            ("Plasma Style", "plasma_style", "plasmarc][Theme", "name"),
            ("Window Decoration Theme", "window_decorations", "kwinrc][org.kde.kdecoration2", "theme"),
            ("Window Decoration Engine", "window_decoration_engines", "kwinrc][org.kde.kdecoration2", "library")
        ]
        
        self.combos = {}
        
        for label, category, section, key in self.fields_map:
            combo = QComboBox()
            # Checkable combo or just standard? Standard.
            # Populate
            items = PlasmaThemeManager.list_sub_themes(category)
            combo.addItems([""] + items) # Add empty option
            combo.setEditable(True) # Allow custom entry
            self.combos[(section, key)] = combo
            easy_layout.addRow(label + ":", combo)

        # Save Button for Easy Mode
        self.save_easy_btn = QPushButton("Save Changes")
        self.save_easy_btn.clicked.connect(self.save_easy_defaults)
        self.save_easy_btn.clicked.connect(self.save_easy_defaults)
        
        self.restore_btn = QPushButton("Restore Original Defaults")
        self.restore_btn.clicked.connect(self.restore_defaults)
        
        btn_layout = QHBoxLayout()
        btn_layout.addWidget(self.save_easy_btn)
        btn_layout.addWidget(self.restore_btn)
        easy_layout.addRow(btn_layout)
        
        self.easy_edit_widget.setLayout(easy_layout)
        
        # 2. Raw Edit Tab
        self.raw_widget = QWidget()
        raw_layout = QVBoxLayout()
        self.editor = QTextEdit()
        self.save_raw_btn = QPushButton("Save Raw File")
        self.save_raw_btn.clicked.connect(self.save_raw_defaults)
        
        raw_layout.addWidget(self.editor)
        raw_layout.addWidget(self.save_raw_btn)
        self.raw_widget.setLayout(raw_layout)
        
        self.editor_tabs.addTab(self.easy_edit_widget, "Easy Editor")
        self.editor_tabs.addTab(self.raw_widget, "Raw Editor")
        self.editor_tabs.setEnabled(False)

        # Splitter
        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(left_widget)
        splitter.addWidget(self.editor_tabs)
        splitter.setStretchFactor(1, 1)

        layout.addWidget(splitter)
        self.setLayout(layout)

    def refresh_list(self):
        self.theme_list.clear()
        themes = PlasmaThemeManager.list_installed_themes()
        # themes is list of dicts: {'name': str, 'type': 'user'|'system', 'path': str}
        for t in themes:
            display_text = t['name']
            if t['type'] == 'system':
                display_text += " (System)"
            self.theme_list.addItem(display_text)

    def on_theme_selected(self, current, previous):
        if not current:
            self.editor.clear()
            self.editor_tabs.setEnabled(False)
            return
        
        # Clean name from display text "Name (System)" -> "Name"
        theme_name = current.text().replace(" (System)", "")
        content = PlasmaThemeManager.read_defaults(theme_name)
        self.editor.setPlainText(content)
        self.parse_defaults_to_ui(content)
        self.editor_tabs.setEnabled(True)

    def parse_defaults_to_ui(self, content):
        config = configparser.ConfigParser(strict=False)
        # Separate parsing to handle loose INI
        try:
            config.read_string(content)
            for (section, key), combo in self.combos.items():
                val = ""
                if config.has_section(section) and config.has_option(section, key):
                    val = config.get(section, key)
                    
                    # Strip __aurorae__svg__ for display matching
                    if key == "theme" and section == "kwinrc][org.kde.kdecoration2":
                        if val.startswith("__aurorae__svg__"):
                             val = val.replace("__aurorae__svg__", "")

                combo.setCurrentText(val)
        except Exception as e:
            print(f"INI Parsing Error: {e}")
            # Reset combos
            for combo in self.combos.values():
                combo.setCurrentIndex(0)

    def clone_theme(self):
        item = self.theme_list.currentItem()
        if not item:
            QMessageBox.warning(self, "No Selection", "Please select a theme to clone.")
            return

        src_name = item.text().replace(" (System)", "")
        default_new = f"{src_name}-U"
        new_name, ok = QInputDialog.getText(self, "Clone Theme", f"New name for clone of '{src_name}':", text=default_new)
        
        if ok and new_name:
            if not new_name.endswith("-U"):
                 res = QMessageBox.question(self, "Suffix Missing", "It is recommended to append '-U' to user themes. Append now?", QMessageBox.Yes | QMessageBox.No)
                 if res == QMessageBox.Yes:
                     new_name += "-U"

            try:
                PlasmaThemeManager.clone_theme(src_name, new_name)
                QMessageBox.information(self, "Success", f"Theme cloned to '{new_name}'")
                self.refresh_list()
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Failed to clone theme: {e}")

    def save_easy_defaults(self):
        # 1. Read current raw content (source of truth) to preserve comments/structure if possible?
        # configparser writes clean INI, might lose comments.
        # User accepted simple editing, so we'll use configparser.
        config = configparser.ConfigParser(strict=False)
        config.optionxform = str # Preserve case sensitivity
        
        # Load existing first
        item = self.theme_list.currentItem()
        if not item: return
        try:
            theme_name = item.text().replace(" (System)", "")
            current_raw = self.editor.toPlainText() # Get from raw editor to ensure we are in sync?
            # Or read from file again? Better read from UI raw editor in case user made changes there.
            config.read_string(current_raw)
            
            for (section, key), combo in self.combos.items():
                val = combo.currentText()
                if val:
                    # Special handling for Aurorae Window Decorations
                    if section == "kwinrc][org.kde.kdecoration2" and key == "theme":
                         # Check if engine is aurorae
                         engine_combo = self.combos.get(("kwinrc][org.kde.kdecoration2", "library"))
                         if engine_combo and engine_combo.currentText() == "org.kde.kwin.aurorae":
                             if not val.startswith("__aurorae__svg__"):
                                 val = "__aurorae__svg__" + val
                    
                    if not config.has_section(section):
                        config.add_section(section)
                    config.set(section, key, val)
            
            # Write to string
            out = io.StringIO()
            config.write(out)
            new_content = out.getvalue()
            
            # Update file and raw editor
            PlasmaThemeManager.write_defaults(theme_name, new_content)
            self.editor.setPlainText(new_content)
            QMessageBox.information(self, "Saved", "Defaults file updated.")
            
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to save: {e}")

    def restore_defaults(self):
        item = self.theme_list.currentItem()
        if not item: return
        
        theme_name = item.text().replace(" (System)", "")
        res = QMessageBox.warning(
            self, 
            "Confirm Restore",
            "Are you sure you want to restore the defaults file to its original state? Current changes will be lost.",
            QMessageBox.Yes | QMessageBox.No
        )
        
        if res == QMessageBox.Yes:
            content = PlasmaThemeManager.restore_defaults(theme_name)
            if content:
                self.editor.setPlainText(content)
                self.parse_defaults_to_ui(content)
                QMessageBox.information(self, "Restored", "Defaults restored from backup.")
            else:
                 QMessageBox.warning(self, "Failed", "Could not restore defaults (Backup might not exist).")

    def save_raw_defaults(self):
        item = self.theme_list.currentItem()
        if not item: return
        
        theme_name = item.text().replace(" (System)", "")
        content = self.editor.toPlainText()
        try:
            PlasmaThemeManager.write_defaults(theme_name, content)
            # Update UI combos
            self.parse_defaults_to_ui(content)
            QMessageBox.information(self, "Saved", "Defaults file updated.")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to save defaults: {e}")
