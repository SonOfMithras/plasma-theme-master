import argparse
import sys
import datetime
from pathlib import Path

# Add src to path to import core modules
sys.path.append(str(Path(__file__).parent))

from core.solar import get_solar_times
from core.config import config
from core.kvantum import KvantumManager
from core.plasma import PlasmaThemeManager

def cmd_scheduler(args):
    # 1. Update config if args provided
    if args.lat is not None:
        config.set('latitude', args.lat)
        print(f"Latitude updated to {args.lat}")
    if args.lon is not None:
        config.set('longitude', args.lon)
        print(f"Longitude updated to {args.lon}")
    if args.day_theme:
        config.set('day_theme', args.day_theme)
        print(f"Day theme updated to {args.day_theme}")
    if args.night_theme:
        config.set('night_theme', args.night_theme)
        print(f"Night theme updated to {args.night_theme}")

    # 2. Run logic
    lat = config.get('latitude')
    lon = config.get('longitude')
    day_theme = config.get('day_theme')
    night_theme = config.get('night_theme')

    if lat == 0.0 and lon == 0.0:
        print("Warning: Latitude and Longitude are 0.0. Please set them using --lat and --lon.")
    
    now = datetime.datetime.now().astimezone()
    today_utc = datetime.datetime.now(datetime.timezone.utc).date()
    
    times = get_solar_times(today_utc, lat, lon)
    
    if not times:
        print("Could not calculate solar times. Defaulting to Day theme.")
        target_theme = day_theme
    else:
        sunrise = times['sunrise'].astimezone()
        sunset = times['sunset'].astimezone()
        
        print(f"Current Time: {now.strftime('%H:%M:%S')}")
        print(f"Sunrise:      {sunrise.strftime('%H:%M:%S')}")
        print(f"Sunset:       {sunset.strftime('%H:%M:%S')}")

        if sunrise <= now < sunset:
            print("Status: Day")
            target_theme = day_theme
        else:
            print("Status: Night")
            target_theme = night_theme
    
    print(f"Target Theme: {target_theme}")
    
    if args.apply:
        KvantumManager.set_theme(target_theme)
    else:
        print("Dry run. Use --apply to actually switch the theme.")

def cmd_theme(args):
    if args.action == 'list':
        themes = PlasmaThemeManager.list_installed_themes()
        print("Installed Global Themes:")
        for t in themes:
            print(f" - {t['name']} ({t['type']})")
    elif args.action == 'clone':
        if not args.source or not args.dest:
            print("Error: --source and --dest are required for clone.")
            return
        try:
            new_path = PlasmaThemeManager.clone_theme(args.source, args.dest)
            print(f"Theme cloned successfully to: {new_path}")
        except Exception as e:
            print(f"Error cloning theme: {e}")

def main():
    parser = argparse.ArgumentParser(description="Plasma Theme Master CLI")
    subparsers = parser.add_subparsers(dest='command', help='Sub-commands')

    # Scheduler Command
    parser_sched = subparsers.add_parser('scheduler', help='Manage Day/Night cycle')
    parser_sched.add_argument('--lat', type=float, help='Set Latitude')
    parser_sched.add_argument('--lon', type=float, help='Set Longitude')
    parser_sched.add_argument('--day-theme', type=str, help='Set Day Kvantum Theme')
    parser_sched.add_argument('--night-theme', type=str, help='Set Night Kvantum Theme')
    parser_sched.add_argument('--apply', action='store_true', help='Apply the calculated theme now')
    parser_sched.set_defaults(func=cmd_scheduler)

    # Theme Command
    parser_theme = subparsers.add_parser('theme', help='Manage Global Themes')
    parser_theme.add_argument('action', choices=['list', 'clone'], help='Action to perform')
    parser_theme.add_argument('--source', type=str, help='Source theme name (for clone)')
    parser_theme.add_argument('--dest', type=str, help='Destination theme name (for clone)')
    parser_theme.set_defaults(func=cmd_theme)

    args = parser.parse_args()
    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
