import subprocess
import shutil

class KvantumManager:
    @staticmethod
    def set_theme(theme_name):
        """
        Sets the active Kvantum theme. 
        Attempts to use 'kvantummanager' with offscreen platform first.
        Falls back to editing 'kvantum.kvconfig' directly if the tool crashes or fails.
        """
        import os
        import configparser
        from pathlib import Path
        
        # Method 1: kvantummanager (Preferred if it works, handles signals)
        if shutil.which("kvantummanager"):
            try:
                # Force offscreen to avoid display requirement crashing the service
                env = os.environ.copy()
                env["QT_QPA_PLATFORM"] = "offscreen"
                
                subprocess.run(["kvantummanager", "--set", theme_name], check=True, env=env)
                return True
            except subprocess.CalledProcessError as e:
                print(f"kvantummanager failed: {e}. Falling back to config edit.")
            except Exception as e:
                print(f"Error running kvantummanager: {e}. Falling back to config edit.")
        
        # Method 2: Direct Config Edit (Robust fallback)
        try:
            config_path = Path.home() / ".config/Kvantum/kvantum.kvconfig"
            if not config_path.exists():
                # Ensure dir exists
                config_path.parent.mkdir(parents=True, exist_ok=True)
                
            parser = configparser.ConfigParser()
            # Preserve case sensitivity if needed, but Kvantum usually okay.
            # parser.optionxform = str 
            
            if config_path.exists():
                parser.read(config_path)
            
            if "General" not in parser:
                parser["General"] = {}
            
            parser["General"]["theme"] = theme_name
            
            with open(config_path, 'w') as f:
                parser.write(f)
                
            print(f"Directly updated kvantum.kvconfig to {theme_name}")
            return True
            
        except Exception as e:
            print(f"Failed to set Kvantum theme manually: {e}")
            return False

    @staticmethod
    def get_current_theme():
        """
        Retrieves the currently active Kvantum theme by reading ~/.config/Kvantum/kvantum.kvconfig.
        """
        import configparser
        from pathlib import Path
        
        config_path = Path.home() / ".config/Kvantum/kvantum.kvconfig"
        if not config_path.exists():
            return None
            
        try:
            # kvantum.kvconfig is an INI file
            # ConfigParser is strict, Kvantum config should be compliant
            parser = configparser.ConfigParser()
            parser.read(config_path)
            if "General" in parser and "theme" in parser["General"]:
                return parser["General"]["theme"]
        except Exception as e:
            print(f"Error reading Kvantum config: {e}")
            
        return None

    @staticmethod
    def list_themes():
        """
        Lists all available Kvantum themes by scanning system and user directories for .kvconfig files.
        One directory might contain multiple themes (e.g. Theme.kvconfig, ThemeDark.kvconfig).
        """
        import os
        from pathlib import Path
        
        paths = [
            Path("/usr/share/Kvantum"),
            Path.home() / ".config/Kvantum"
        ]
        
        themes = set()
        
        for p in paths:
            if not p.exists():
                continue
            # Recursively find all .kvconfig files
            for config_file in p.rglob("*.kvconfig"):
                theme_name = config_file.stem
                # 'kvantum.kvconfig' is the main config file, not a theme
                if theme_name != "kvantum":
                    themes.add(theme_name)
        
        return sorted(list(themes))
