from PySide6.QtWidgets import QMainWindow, QTabWidget
from gui.scheduler_tab import SchedulerTab
from gui.global_theme_editor_tab import GlobalThemeEditorTab

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Plasma Theme Master")
        self.resize(800, 600)
        
        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)
        
        self.scheduler_tab = SchedulerTab()
        self.theme_editor_tab = GlobalThemeEditorTab()
        
        self.tabs.addTab(self.scheduler_tab, "Scheduler")
        self.tabs.addTab(self.theme_editor_tab, "Global Theme Editor")
