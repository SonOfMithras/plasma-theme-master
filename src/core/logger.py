import logging
import logging.handlers
import pathlib
import os

# Define log path
LOG_DIR = pathlib.Path.home() / ".local" / "share" / "plasma-theme-master"
LOG_FILE = LOG_DIR / "activity.log"

def setup_logger():
    """
    Sets up the application logger with file rotation.
    """
    # Ensure directory exists
    try:
        LOG_DIR.mkdir(parents=True, exist_ok=True)
    except Exception:
        pass # If we can't create dir, logging will fail gracefully below or to stderr

    logger = logging.getLogger("PlasmaThemeMaster")
    logger.setLevel(logging.INFO)

    # Avoid adding multiple handlers if setup is called multiple times
    if logger.handlers:
        return logger

    # Check if this is a new log file
    is_new_file = not LOG_FILE.exists()

    # Rotating File Handler (1MB limit, 1 backup)
    try:
        handler = logging.handlers.RotatingFileHandler(
            LOG_FILE, maxBytes=1_000_000, backupCount=1, encoding='utf-8'
        )
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        
        if is_new_file:
            logger.info("-- no log found, log file created --")
            
    except Exception as e:
        print(f"Failed to setup logging to file: {e}")
        # Fallback to console if file logging fails
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(logging.Formatter('%(levelname)s: %(message)s'))
        logger.addHandler(console_handler)

    return logger

def get_logger():
    """Returns the configured logger instance."""
    return logging.getLogger("PlasmaThemeMaster")

def log_activity(message):
    """Helper to log an info message (Activity)."""
    get_logger().info(message)

def log_error(message):
    """Helper to log an error message."""
    get_logger().error(message)
