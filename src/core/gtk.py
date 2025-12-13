import os
import configparser
from pathlib import Path
from core.logger import log_activity, log_error

GTK3_CONFIG = Path.home() / ".config/gtk-3.0/settings.ini"
GTK4_CONFIG = Path.home() / ".config/gtk-4.0/settings.ini"

class GtkManager:
    @staticmethod
    def list_themes():
        """
        Scans common theme directories for GTK themes.
        Returns a sorted list of unique theme names.
        """
        search_paths = [
            Path.home() / ".themes",
            Path.home() / ".local/share/themes",
            Path("/usr/share/themes")
        ]
        
        themes = set()
        
        for path in search_paths:
            if not path.exists():
                continue
                
            for item in path.iterdir():
                if item.is_dir():
                    # Check if it looks like a theme (has index.theme or gtk-3.0/gtk-4.0 subdir)
                    # Although many valid themes just exist as folders.
                    # Let's filter slightly to avoid trash.
                    if (item / "index.theme").exists() or (item / "gtk-3.0").exists():
                        themes.add(item.name)
                        
        return sorted(list(themes))

    @staticmethod
    def get_current_theme():
        """
        Reads the current GTK theme from ~/.config/gtk-3.0/settings.ini
        """
        if not GTK3_CONFIG.exists():
            return "Breeze" # Default fallback
            
        try:
            config = configparser.ConfigParser()
            config.read(GTK3_CONFIG)
            if 'Settings' in config and 'gtk-theme-name' in config['Settings']:
                return config['Settings']['gtk-theme-name']
        except Exception as e:
            log_error(f"Failed to read GTK config: {e}")
            
        return "Breeze"

    @staticmethod
    def set_theme(theme_name):
        """
        Sets the GTK theme in gtk-3.0 and gtk-4.0 settings.ini
        """
        paths = [GTK3_CONFIG, GTK4_CONFIG]
        success = False
        
        for path in paths:
            try:
                # Ensure parent dir exists
                path.parent.mkdir(parents=True, exist_ok=True)
                
                config = configparser.ConfigParser()
                # Preserve case sensitivity? GTK keys are case sensitive usually?
                # ConfigParser converts to lowercase by default.
                config.optionxform = str 
                
                if path.exists():
                    config.read(path)
                    
                if 'Settings' not in config:
                    config['Settings'] = {}
                    
                config['Settings']['gtk-theme-name'] = theme_name
                
                with open(path, 'w') as f:
                    config.write(f)
                    
                success = True
            except Exception as e:
                log_error(f"Failed to write GTK config at {path}: {e}")
                
        if success:
            log_activity(f"Applied GTK Theme: {theme_name}")
            return True
        return False
