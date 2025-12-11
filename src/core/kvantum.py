import subprocess
import shutil

class KvantumManager:
    @staticmethod
    def set_theme(theme_name):
        """
        Sets the active Kvantum theme using 'kvantummanager --set <theme>'.
        """
        if not shutil.which("kvantummanager"):
            raise FileNotFoundError("kvantummanager executable not found in PATH")

        try:
            subprocess.run(["kvantummanager", "--set", theme_name], check=True)
            return True
        except subprocess.CalledProcessError as e:
            print(f"Error setting theme '{theme_name}': {e}")
            return False

    @staticmethod
    def get_current_theme():
        # TODO: Parse current theme from config file if no CLI output available
        # checking ~/.config/Kvantum/kvantum.kvconfig
        pass

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
