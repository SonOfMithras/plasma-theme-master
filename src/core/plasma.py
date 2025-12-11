import os
import shutil
import glob
from pathlib import Path

PLASMA_THEMES_DIR = Path.home() / ".local/share/plasma/look-and-feel"

class PlasmaThemeManager:
    @staticmethod
    def list_sub_themes(category):
        """
        Generic lister for sub-themes.
        category: 'colors', 'cursors', 'plasma_style', 'icons', 'window_decorations', 'application_styles'
        """
        import subprocess
        from pathlib import Path
        
        results = []
        
        if category == 'colors':
            cmd = ["plasma-apply-colorscheme", "--list-schemes"]
            try:
                out = subprocess.check_output(cmd, text=True)
                for line in out.splitlines():
                    if line.strip() and not line.startswith("You have"):
                        # Remove " * " prefix and " (current...)" suffix
                        clean = line.strip().lstrip("* ").split(" (")[0]
                        results.append(clean)
            except: pass
            
        elif category == 'cursors':
            # 1. Get system/known themes
            cmd = ["plasma-apply-cursortheme", "--list-themes"]
            try:
                out = subprocess.check_output(cmd, text=True)
                for line in out.splitlines():
                    if line.strip() and not line.startswith("You have"):
                         clean = line.strip().lstrip("* ")
                         if "[" in clean and "]" in clean:
                             clean = clean.split("[")[1].split("]")[0]
                         results.append(clean)
            except: pass
            
            # 2. Scan user directories for "-Cursors"
            cursor_paths = [
                Path.home() / ".icons",
                Path.home() / ".local/share/icons"
            ]
            for p in cursor_paths:
                if p.exists():
                    for item in p.iterdir():
                        if item.is_dir() and item.name.endswith("-Cursors"):
                            results.append(item.name)

        elif category == 'plasma_style':
            cmd = ["plasma-apply-desktoptheme", "--list-themes"]
            try:
                out = subprocess.check_output(cmd, text=True)
                for line in out.splitlines():
                    if line.strip() and not line.startswith("You have"):
                        clean = line.strip().lstrip("* ").split(" (")[0]
                        results.append(clean)
            except: pass

        elif category == 'icons':
            # Scan common icon dirs and ~/.icons
            # Exclude those ending in -Cursors as per user logic
            icon_paths = [
                Path.home() / ".local/share/icons",
                Path.home() / ".icons",
                Path("/usr/share/icons")
            ]
            for p in icon_paths:
                if p.exists():
                    for item in p.iterdir():
                        if item.is_dir() and (item / "index.theme").exists():
                             if not item.name.endswith("-Cursors"):
                                results.append(item.name)
        
        elif category == 'window_decorations':
            # 1. Standard/Binary themes
            results = ["org.kde.breeze", "org.kde.oxygen", "Plastik"]
            
            # 2. Scan Aurorae themes
            aurorae_paths = [
                Path.home() / ".local/share/aurorae/themes",
                Path("/usr/share/aurorae/themes")
            ]
            for p in aurorae_paths:
                if p.exists():
                    for item in p.iterdir():
                        if item.is_dir():
                             results.append(item.name)

        elif category == 'window_decoration_engines':
            results = ["org.kde.kwin.aurorae", "org.kde.breeze", "org.kde.oxygen"]

        elif category == 'application_styles':
             # Widget Styles: Breeze, Oxygen, Fusion, Kvantum
             # kvantummanager lists themes, but the style itself is 'kvantum'
             results = ["Breeze", "Oxygen", "Fusion", "kvantum"]
             
        return sorted(list(set(results)))

    @staticmethod
    def list_installed_themes():
        """
        Returns a list of installed Global Themes with metadata.
        Returns list of dicts: {'name': str, 'type': 'user'|'system', 'path': Path}
        """
        import subprocess
        
        # 1. Get list from plasma tool
        raw_list = []
        try:
            result = subprocess.run(
                ["plasma-apply-lookandfeel", "--list"], 
                capture_output=True, text=True, check=True
            )
            for line in result.stdout.splitlines():
                line = line.strip()
                if line and not line.startswith("Listing") and not line.startswith("KPackageStructure"):
                    raw_list.append(line)
        except Exception:
            pass

        # 2. Determine type and path
        themes = []
        user_dir = Path.home() / ".local/share/plasma/look-and-feel"
        system_dir = Path("/usr/share/plasma/look-and-feel")

        for name in raw_list:
            # Check user first
            user_path = user_dir / name
            system_path = system_dir / name
            
            if user_path.exists():
                themes.append({'name': name, 'type': 'user', 'path': str(user_path)})
            elif system_path.exists():
                themes.append({'name': name, 'type': 'system', 'path': str(system_path)})
            else:
                # Fallback, might be in other system paths or just listed by ID
                themes.append({'name': name, 'type': 'system', 'path': None})
                
        return sorted(themes, key=lambda x: x['name'])

    @staticmethod
    def find_theme_path(theme_name):
        """
        Locates the directory of a theme by name.
        Checks user directory first, then system directory.
        """
        search_paths = [
            Path.home() / ".local/share/plasma/look-and-feel",
            Path("/usr/share/plasma/look-and-feel")
        ]
        
        for p in search_paths:
            candidate = p / theme_name
            if candidate.exists() and candidate.is_dir():
                return candidate
        return None

    @staticmethod
    def clone_theme(source_theme_name, new_theme_name):
        """
        Clones an existing theme to a new directory.
        """
        src = PlasmaThemeManager.find_theme_path(source_theme_name)
        if not src:
            raise FileNotFoundError(f"Source theme '{source_theme_name}' not found.")
            
        dest = PLASMA_THEMES_DIR / new_theme_name

        if dest.exists():
            raise FileExistsError(f"Destination theme '{new_theme_name}' already exists.")

        shutil.copytree(src, dest)
        
        # Update metadata.desktop or metadata.json in the new specific theme to reflect new name
        PlasmaThemeManager._update_metadata(dest, new_theme_name)
        
        return dest

    @staticmethod
    def _update_metadata(theme_dir, new_name):
        """
        Internal helper to update the name in metadata.
        Handles both metadata.desktop and metadata.json.
        """
        import json
        
        safe_id = new_name.replace(" ", "-") # Ensure ID has no spaces
        
        # 1. Handle metadata.desktop
        metadata_desktop = theme_dir / "metadata.desktop"
        if metadata_desktop.exists():
            try:
                content = metadata_desktop.read_text()
                lines = content.splitlines()
                with open(metadata_desktop, 'w') as f:
                    for line in lines:
                        if line.startswith("Name="):
                            f.write(f"Name={new_name}\n")
                        elif line.startswith("Id="):
                            f.write(f"Id={safe_id}\n")
                        elif line.startswith("X-KDE-PluginInfo-Name="):
                            f.write(f"X-KDE-PluginInfo-Name={safe_id}\n")
                        else:
                            f.write(line + "\n")
            except Exception as e:
                print(f"Failed to update metadata.desktop: {e}")

        # 2. Handle metadata.json (Newer Plasma versions)
        metadata_json = theme_dir / "metadata.json"
        if metadata_json.exists():
            try:
                data = {}
                with open(metadata_json, 'r') as f:
                    data = json.load(f)
                
                # Check structure. Usually wrapped in "KPlugin" object in newer versions, 
                # or flat in some intermediates. We'll update wherever we find the keys.
                
                # Helper to update dict recursively
                def update_recursive(d, name, nid):
                    for k, v in d.items():
                        if k == "Id":
                            d[k] = nid
                        elif k == "Name":
                            d[k] = name
                        elif isinstance(v, dict):
                            update_recursive(v, name, nid)

                # If "KPlugin" exists, update inside it
                if "KPlugin" in data:
                    update_recursive(data["KPlugin"], new_name, safe_id)
                else:
                    # Fallback for flat structure
                    if "Id" in data:
                        data["Id"] = safe_id
                    if "Name" in data:
                        data["Name"] = new_name
                    if "KPlugin" not in data and "Id" not in data:
                        # Some files might be strictly typed, but let's try to inject if missing
                        # or just leave it. Safest is to only update existing keys.
                        pass
                
                with open(metadata_json, 'w') as f:
                    json.dump(data, f, indent=4)
            except Exception as e:
                print(f"Failed to update metadata.json: {e}")

    @staticmethod
    def get_defaults_path(theme_name):
        """
        Returns the path to the defaults file, resolved via finding the theme first.
        """
        theme_path = PlasmaThemeManager.find_theme_path(theme_name)
        if theme_path:
            return theme_path / "contents" / "defaults"
        return None  # Should not happen if we selected from list, but safer

    @staticmethod
    def read_defaults(theme_name):
        """
        Reads the contents/defaults file and returns it as a string.
        """
        defaults_file = PlasmaThemeManager.get_defaults_path(theme_name)
        if defaults_file and defaults_file.exists():
            return defaults_file.read_text()
        return ""

    @staticmethod
    def write_defaults(theme_name, content):
        """
        Writes content to contents/defaults file.
        WARNING: This will attempt to write to the theme's location.
        If the theme is a system theme, this will fail.
        """
        defaults_file = PlasmaThemeManager.get_defaults_path(theme_name)
        if not defaults_file:
            raise FileNotFoundError(f"Theme '{theme_name}' path could not be resolved.")
            
        defaults_file.parent.mkdir(parents=True, exist_ok=True)
        defaults_file.write_text(content)
