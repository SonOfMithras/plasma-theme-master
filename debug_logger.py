import sys
from pathlib import Path
sys.path.append(str(Path.cwd() / "src"))

from core.logger import setup_logger, log_activity, LOG_FILE

print(f"Log File Path: {LOG_FILE}")
logger = setup_logger()
log_activity("Test Log Entry via reproduction script.")
print("Logged test entry.")

if LOG_FILE.exists():
    print(f"Log file exists. Size: {LOG_FILE.stat().st_size} bytes")
    print("Content:")
    print(LOG_FILE.read_text())
else:
    print("Log file does NOT exist.")
