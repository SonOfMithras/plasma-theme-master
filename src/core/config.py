import json
import os
from pathlib import Path

APP_NAME = "plasma-theme-master"
CONFIG_DIR = Path.home() / ".config" / APP_NAME
CONFIG_FILE = CONFIG_DIR / "config.json"

DEFAULT_CONFIG = {
    "latitude": 0.0,
    "longitude": 0.0,
    "day_theme": "Default",
    "night_theme": "DefaultDark",
    "schedule_mode": "solar", # "solar" or "custom"
    "custom_sunrise": "06:00",
    "custom_sunset": "18:00"
}

class ConfigManager:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(ConfigManager, cls).__new__(cls)
            cls._instance.load()
        return cls._instance

    def load(self):
        self._ensure_config_dir()
        if not CONFIG_FILE.exists():
            self._data = DEFAULT_CONFIG.copy()
            self.save()
        else:
            try:
                with open(CONFIG_FILE, 'r') as f:
                    self._data = json.load(f)
            except (json.JSONDecodeError, IOError):
                self._data = DEFAULT_CONFIG.copy()

    def save(self):
        self._ensure_config_dir()
        with open(CONFIG_FILE, 'w') as f:
            json.dump(self._data, f, indent=4)

    def _ensure_config_dir(self):
        CONFIG_DIR.mkdir(parents=True, exist_ok=True)

    def get(self, key, default=None):
        return self._data.get(key, default)

    def set(self, key, value):
        self._data[key] = value
        self.save()

    def reset_defaults(self):
        self._data = DEFAULT_CONFIG.copy()
        self.save()

# Global instance
config = ConfigManager()
