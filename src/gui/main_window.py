from PySide6.QtWidgets import (
    QMainWindow, QTabWidget, QToolBar, QWidget, QSizePolicy, 
    QMenu, QToolButton, QMessageBox, QDialog, QTextEdit, QVBoxLayout, QHBoxLayout, QPushButton,
    QLabel, QCheckBox
)
from PySide6.QtGui import QAction, QIcon, QFont
from PySide6.QtCore import Qt

from gui.scheduler_tab import SchedulerTab
from gui.global_theme_editor_tab import GlobalThemeEditorTab
from core.config import config

from core.plasma import PlasmaThemeManager
from core.logger import LOG_FILE
from core.version import APP_VERSION


class LogViewerDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Activity Log")
        self.resize(600, 400)
        self.init_ui()
        self.load_log()

    def init_ui(self):
        layout = QVBoxLayout()
        
        self.text_edit = QTextEdit()
        self.text_edit.setReadOnly(True)
        # Use monospaced font
        font = QFont("Monospace")
        font.setStyleHint(QFont.TypeWriter)
        self.text_edit.setFont(font)
        
        layout.addWidget(self.text_edit)
        
        btn_layout = QHBoxLayout()
        refresh_btn = QPushButton("Refresh")
        refresh_btn.clicked.connect(self.load_log)
        close_btn = QPushButton("Close")
        close_btn.clicked.connect(self.accept)
        
        btn_layout.addWidget(refresh_btn)
        btn_layout.addStretch()
        btn_layout.addWidget(close_btn)
        
        layout.addLayout(btn_layout)
        self.setLayout(layout)

    def load_log(self):
        if not LOG_FILE.exists():
            self.text_edit.setPlainText("No log file found.")
            return

        try:
            lines = LOG_FILE.read_text().splitlines()
            # Show newest first
            reversed_content = "\n".join(reversed(lines))
            self.text_edit.setPlainText(reversed_content)
            # Ensure scrolled to top
            self.text_edit.verticalScrollBar().setValue(0)
        except Exception as e:
            self.text_edit.setPlainText(f"Error reading log: {e}")

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Plasma Theme Master")
        self.resize(800, 600)
        
        # Setup Toolbar (Single Header)
        self.setup_toolbar()
        
        # Explicitly remove status bar to prevent bottom margin/gap issues
        self.setStatusBar(None)
        
        self.tabs = QTabWidget()
        self.tabs.setDocumentMode(True) # Removes extra borders/margins
        self.setCentralWidget(self.tabs)
        
        # Ensure main window isn't transparent
        self.setAutoFillBackground(True)
        # Remove internal tab pane borders/margins
        self.setStyleSheet("QTabWidget::pane { border: 0; margin: 0; padding: 0; }")
        
        self.scheduler_tab = SchedulerTab()
        self.theme_editor_tab = GlobalThemeEditorTab()
        
        self.tabs.addTab(self.scheduler_tab, "Scheduler")
        self.tabs.addTab(self.theme_editor_tab, "Global Theme Editor")

    def setup_toolbar(self):
        toolbar = QToolBar()
        toolbar.setMovable(False)
        self.addToolBar(toolbar)
        
        # === Left Side: Files Menu ===
        btn_files = QToolButton()
        btn_files.setText("Files")
        btn_files.setToolTip("File Operations")
        btn_files.setPopupMode(QToolButton.InstantPopup)
        
        files_menu = QMenu(self)
        
        action_themes = QAction("Open Global Themes Folder", self)
        action_themes.triggered.connect(self.open_theme_folder)
        files_menu.addAction(action_themes)
        
        action_config = QAction("Open Config Folder", self)
        action_config.triggered.connect(self.open_config_folder)
        files_menu.addAction(action_config)
        
        btn_files.setMenu(files_menu)
        toolbar.addWidget(btn_files)
        
        # === Left Side: Help Menu ===
        btn_help = QToolButton()
        btn_help.setText("Help")
        btn_help.setToolTip("Help info")
        btn_help.setPopupMode(QToolButton.InstantPopup)
        
        help_menu = QMenu(self)
        action_log = QAction("View Activity Log", self)
        action_log.triggered.connect(self.show_log)
        help_menu.addAction(action_log)
        
        action_about = QAction("About", self)
        action_about.triggered.connect(self.show_about)
        help_menu.addAction(action_about)
        
        help_menu.addSeparator()
        
        action_uninstall = QAction("Uninstall...", self)
        action_uninstall.triggered.connect(self.run_uninstall)
        help_menu.addAction(action_uninstall)
        
        btn_help.setMenu(help_menu)
        toolbar.addWidget(btn_help)

        # === Spacer ===
        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        toolbar.addWidget(spacer)
        
        # === Right Side: Hamburger Menu ===
        btn_menu = QToolButton()
        btn_menu.setText("☰")
        btn_menu.setToolTip("Menu")
        btn_menu.setPopupMode(QToolButton.InstantPopup)
        
        menu = QMenu(self)
        
        # Clear Config
        action_clear = QAction("Clear Config", self)
        action_clear.triggered.connect(self.clear_config)
        menu.addAction(action_clear)
        
        btn_menu.setMenu(menu)
        toolbar.addWidget(btn_menu)
    
    def open_theme_folder(self):
        import subprocess
        import os
        path = os.path.expanduser("~/.local/share/plasma/look-and-feel/")
        if not os.path.exists(path):
            os.makedirs(path, exist_ok=True)
        subprocess.Popen(["xdg-open", path])

    def open_config_folder(self):
        import subprocess
        import os
        path = os.path.expanduser("~/.config/plasma-theme-master/")
        if not os.path.exists(path):
            os.makedirs(path, exist_ok=True)
        subprocess.Popen(["xdg-open", path])

    def run_uninstall(self):
        # Confirm uninstall
        msg = QMessageBox(self)
        msg.setWindowTitle("Uninstall Plasma Theme Master")
        msg.setText("Are you sure you want to uninstall this application?")
        msg.setIcon(QMessageBox.Warning)
        
        # Checkbox for data removal
        clean_cb = QCheckBox("Remove user configuration and logs")
        msg.setCheckBox(clean_cb)
        
        msg.setStandardButtons(QMessageBox.Yes | QMessageBox.Cancel)
        msg.setDefaultButton(QMessageBox.Cancel)
        
        if msg.exec() == QMessageBox.Yes:
            import subprocess
            flag = "--clean" if clean_cb.isChecked() else "--keep"
            
            # Construct command for konsole
            # Using /bin/bash -c to wrap the sudo command potentially, or just direct
            # Typically: konsole -e sudo /path/to/script --flag
            cmd = ["konsole", "-e", "sudo", "/opt/plasma-theme-master/uninstall.sh", flag]
            
            try:
                subprocess.Popen(cmd)
                # Maybe quit app?
                self.close()
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Failed to launch uninstaller: {e}")

    def show_about(self):
        text = f"""
        <h3>Plasma Theme Master v{APP_VERSION}</h3>
        <p>A unified tool for Kvantum day/night scheduling and Plasma Global Theme editing.</p>
        <p><b>License:</b> GPLv3</p>
        <p><b>Author Info:</b> Ammar Al-Riyamy <a href="mailto:ammar.alriyamy@gmail.com">Contact Author</a></p>
        <p><a href="https://github.com/SonOfMithras/plasma-theme-master">GitHub Repository</a></p>
        """
        QMessageBox.about(self, "About", text)

    def clear_config(self):
        res = QMessageBox.warning(
            self, 
            "Reset Configuration", 
            "Are you sure you want to reset all settings to defaults? This cannot be undone.",
            QMessageBox.Yes | QMessageBox.No
        )
        if res == QMessageBox.Yes:
            config.reset_defaults()
            # Reload tabs
            try:
                self.scheduler_tab.load_config()
            except: pass
            try:
                self.theme_editor_tab.refresh_list()
            except: pass
             
            QMessageBox.information(self, "Reset", "Configuration reset to defaults.")

    def show_log(self):
        dialog = LogViewerDialog(self)
        dialog.exec()
