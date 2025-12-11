import sys
from pathlib import Path
from PySide6.QtWidgets import QApplication

# Add src to path
sys.path.append(str(Path(__file__).parent))

from gui.main_window import MainWindow

def main():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
