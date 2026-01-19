from pathlib import Path
import configparser

class KdeConfigParser:
    @staticmethod
    def read_kdeglobals_value(group: str, key: str, default=None):
        """
        Reads a value from ~/.config/kdeglobals directly.
        Returns the value as a string, or default if not found.
        """
        config_path = Path.home() / ".config/kdeglobals"
        if not config_path.exists():
            return default

        try:
            # KDE config files are essentially INI files, but sometimes have quirks.
            # configparser with allow_no_value=True handles most cases.
            parser = configparser.ConfigParser(interpolation=None, allow_no_value=True)
            # Preserve case sensitivity which is important for some KDE keys
            parser.optionxform = str 
            
            # Use read_string or read to parse
            parser.read(config_path)
            
            if parser.has_section(group):
                if parser.has_option(group, key):
                     return parser.get(group, key)
            
            return default
        except Exception:
            return default
