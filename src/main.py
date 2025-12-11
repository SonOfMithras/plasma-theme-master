import sys
from pathlib import Path
from PySide6.QtWidgets import QApplication
from PySide6.QtGui import QIcon

# Add src to path
sys.path.append(str(Path(__file__).parent))

from gui.main_window import MainWindow
import cli

def main():
    if len(sys.argv) > 1:
        # CLI Mode
        cli.main()
    else:
        # GUI Mode
        from core.logger import setup_logger
        setup_logger()
        
        app = QApplication(sys.argv)
        app.setApplicationName("Plasma Theme Master")
        app.setDesktopFileName("plasma-theme-master.desktop")
        app.setWindowIcon(QIcon.fromTheme("preferences-desktop-theme"))
        
        window = MainWindow()
        window.show()
        sys.exit(app.exec())

if __name__ == "__main__":
    main()
