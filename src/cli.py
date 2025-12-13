import argparse
import sys
import datetime
from pathlib import Path

# Add src to path to import core modules
sys.path.append(str(Path(__file__).parent))

from core.solar import get_solar_times
from core.config import config
from core import kvantum, plasma
from core.logger import setup_logger, log_activity, log_error
from core.plasma import PlasmaThemeManager
from core.gtk import GtkManager

def check_and_apply(apply=True):
    """
    Core logic to check time and apply theme.
    Returns True if theme was set, False otherwise.
    """
    lat = config.get('latitude')
    lon = config.get('longitude')
    day_theme = config.get('day_theme')
    night_theme = config.get('night_theme')
    day_gtk = config.get('day_gtk_theme')
    night_gtk = config.get('night_gtk_theme')
    
    # Check for valid config
    if lat == 0.0 and lon == 0.0:
        return

    # Check native Plasma Auto Mode
    # If False, we adhere to Static Mode and do NOT interfere.
    try:
        from core.plasma import PlasmaThemeManager
        is_auto = PlasmaThemeManager.is_auto_enabled()
        if not is_auto and apply:
            # log_activity("Daemon skipping check: Plasma Auto Mode is disabled (Static Mode active).")
            return
    except Exception as e:
        log_error(f"Daemon failed to check Auto Mode: {e}")
        
    now = datetime.datetime.now().astimezone()
    
    # Determine proper mode
    is_day = False
    mode_setting = config.get('schedule_mode', 'solar')
    
    if mode_setting == 'solar':
        today_utc = datetime.datetime.now(datetime.timezone.utc).date()
        times = get_solar_times(today_utc, lat, lon)
        if times:
            sunrise = times['sunrise'].astimezone()
            sunset = times['sunset'].astimezone()
            
            # Apply padding logic
            # Padding -30 (Shorten day): Start +15, End -15
            # Shift = -15 mins
            # Sunrise - (-15) = +15. Sunset + (-15) = -15.
            padding = float(config.get('solar_padding', 0))
            shift = datetime.timedelta(minutes=padding / 2)
            
            sunrise = sunrise - shift
            sunset = sunset + shift
            
            is_day = sunrise <= now < sunset
    else:
        # Custom
        sunrise_str = config.get('custom_sunrise', '06:00')
        sunset_str = config.get('custom_sunset', '18:00')
        t_now = now.time()
        t_rise = datetime.datetime.strptime(sunrise_str, "%H:%M").time()
        t_set = datetime.datetime.strptime(sunset_str, "%H:%M").time()
        
        if t_rise < t_set:
            is_day = t_rise <= t_now < t_set
        else:
            is_day = not (t_set <= t_now < t_rise)

    target_theme = day_theme if is_day else night_theme
    target_gtk = day_gtk if is_day else night_gtk
    
    # If apply is requested or daemon mode (which sets apply=True)
    if apply:
        # Determine target theme
        target_kv = config.get("day_theme") if is_day else config.get("night_theme")
        
        if target_kv:
            # Check if we are already on this theme
            current_kv = kvantum.KvantumManager.get_current_theme()
            log_activity(f"Scheduler Check: Day={is_day}. Target Theme={target_kv}. Current Theme={current_kv}")

            if current_kv == target_kv:
                log_activity("Nothing to do correct theme set")
            else:
                try:
                    kvantum.KvantumManager.set_theme(target_kv)
                    log_activity(f"Applied Kvantum Theme: {target_kv}")
                except Exception as e:
                    log_error(f"Failed to apply Kvantum theme '{target_kv}': {e}")
                    print(f"Error applying Kvantum theme: {e}")
                    
        if target_gtk:
            try:
                GtkManager.set_theme(target_gtk)
                log_activity(f"Applied GTK Theme: {target_gtk}")
            except Exception as e:
                log_error(f"Failed to apply GTK theme '{target_gtk}': {e}")

def cmd_daemon(args):
    import time
    print("Starting Plasma Theme Master Daemon...")
    log_activity("Daemon started.")
    print("Press Ctrl+C to stop.")
    
    while True:
        try:
            # Reload config each iteration to pick up GUI changes
            config.load()
            check_and_apply(apply=True)
            time.sleep(60) # Check every minute
        except KeyboardInterrupt:
            print("Stopping daemon.")
            log_activity("Daemon stopped by user.")
            break
        except Exception as e:
            msg = f"Daemon error: {e}"
            print(msg)
            log_error(msg)
            time.sleep(60)

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
    if args.day_gtk:
        config.set('day_gtk_theme', args.day_gtk)
        print(f"Day GTK theme updated to {args.day_gtk}")
    if args.night_gtk:
        config.set('night_gtk_theme', args.night_gtk)
        print(f"Night GTK theme updated to {args.night_gtk}")

    # 2. Run logic using the helper
    # We can't reuse check_and_apply easily for *printing* status without applying
    # unless we refactor more. But cmd_scheduler is verbose for human CLI usage.
    # The Daemon logic is silent.
    # Let's keep cmd_scheduler mostly as is but fix the logic to match the new custom mode support.
    
    # ... (Actually, cmd_scheduler logic in file was missing custom mode support!)
    # Let's fix cmd_scheduler to duplicate the robust logic or reuse it.
    
    # Reuse check_and_apply logic? It doesn't print status.
    # Let's just fix cmd_scheduler to be robust first.
    
    lat = config.get('latitude')
    lon = config.get('longitude')
    day_theme = config.get('day_theme')
    night_theme = config.get('night_theme')
    mode = config.get('schedule_mode', 'solar')
    
    now = datetime.datetime.now().astimezone()
    is_day = False
    
    # Check Native Mode status for reporting
    try:
        from core.plasma import PlasmaThemeManager
        is_native_auto = PlasmaThemeManager.is_auto_enabled()
        print(f"Plasma Auto Mode: {'Enabled' if is_native_auto else 'Disabled (Static)'}")
    except:
        print("Plasma Auto Mode: Unknown")
    
    if mode == 'solar':
        today_utc = datetime.datetime.now(datetime.timezone.utc).date()
        times = get_solar_times(today_utc, lat, lon)
        if times:
            sunrise = times['sunrise'].astimezone()
            sunset = times['sunset'].astimezone()
            print(f"Solar Mode: Rise {sunrise.strftime('%H:%M')} | Set {sunset.strftime('%H:%M')}")
            is_day = sunrise <= now < sunset
        else:
            print("Solar calc failed.")
    else:
        sunrise_str = config.get('custom_sunrise', '06:00')
        sunset_str = config.get('custom_sunset', '18:00')
        print(f"Custom Mode: Day {sunrise_str} | Night {sunset_str}")
        t_now = now.time()
        t_rise = datetime.datetime.strptime(sunrise_str, "%H:%M").time()
        t_set = datetime.datetime.strptime(sunset_str, "%H:%M").time()
        if t_rise < t_set:
            is_day = t_rise <= t_now < t_set
        else:
            is_day = not (t_set <= t_now < t_rise)

    target_theme = day_theme if is_day else night_theme
    target_gtk = day_gtk if is_day else night_gtk
    
    print(f"Current Status: {'Day' if is_day else 'Night'}")
    print(f"Target Kvantum: {target_theme}")
    print(f"Target GTK:     {target_gtk}")

    if args.apply:
        kvantum.KvantumManager.set_theme(target_theme)
        if target_gtk:
            GtkManager.set_theme(target_gtk)
            print(f"Applied GTK Theme: {target_gtk}")
    else:
        print("Dry run. Use --apply to actually switch.")

def cmd_theme(args):
    if args.action == 'list':
        print("Installed Global Themes:")
        themes = PlasmaThemeManager.list_installed_themes()
        for t in themes:
            path_info = f" ({t['type']})"
            print(f"- {t['name']}{path_info}")
            
    elif args.action == 'clone':
        if not args.source or not args.dest:
            print("Error: --source and --dest are required for clone action.")
            return
            
        try:
            PlasmaThemeManager.clone_theme(args.source, args.dest)
            print(f"Successfully cloned '{args.source}' to '{args.dest}'")
        except Exception as e:
            print(f"Error cloning theme: {e}")

def cmd_log(args):
    from core.logger import LOG_FILE
    if not LOG_FILE.exists():
        print("No activity log found.")
        return

    lines_to_show = 50 if args.verbose else 20
    print(f"--- Last {lines_to_show} lines of Activity Log ---")
    
    try:
        # Read all lines and take last N
        # For huge logs we might want seek, but rotation keeps it small (<1MB)
        content = LOG_FILE.read_text().splitlines()
        for line in content[-lines_to_show:]:
            print(line)
    except Exception as e:
        print(f"Error reading log: {e}")

def main():
    setup_logger()
    examples = """
Examples:

  1. Configure location and themes for the Scheduler:
     plasma-theme-master scheduler --lat 40.7 --lon -74.0 --day-theme Kennedy --night-theme KvDark

  2. Manually trigger a check and apply the correct theme now:
     plasma-theme-master scheduler --apply

  3. Run as a background daemon (checking every minute):
     plasma-theme-master daemon

  4. List all installed Global Themes:
     plasma-theme-master theme list

  5. Clone a Global Theme to create a custom version:
     plasma-theme-master theme clone --source "Breeze" --dest "MyBreeze-U"

  6. Enable the systemd service for background automation:
     systemctl --user enable --now plasma-theme-master.service
"""

    parser = argparse.ArgumentParser(
        description="Plasma Theme Master CLI - Unified Kvantum & Plasma Theme Manager",
        epilog=examples,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    subparsers = parser.add_subparsers(dest='command', help='Sub-commands')

    # Scheduler Command
    parser_sched = subparsers.add_parser('scheduler', help='Manage Day/Night cycle configuration and manual checks')
    parser_sched.add_argument('--lat', type=float, help='Set Latitude (e.g., 40.7)')
    parser_sched.add_argument('--lon', type=float, help='Set Longitude (e.g., -74.0)')
    parser_sched.add_argument('--day-theme', type=str, help='Set Day Kvantum Theme name')
    parser_sched.add_argument('--night-theme', type=str, help='Set Night Kvantum Theme name')
    parser_sched.add_argument('--day-gtk', type=str, help='Set Day GTK Theme name')
    parser_sched.add_argument('--night-gtk', type=str, help='Set Night GTK Theme name')
    parser_sched.add_argument('--apply', action='store_true', help='Apply the calculated theme immediately based on current time')
    parser_sched.set_defaults(func=cmd_scheduler)
    
    # Daemon Command
    parser_daemon = subparsers.add_parser('daemon', help='Run the background service loop (blocks indefinitely)')
    parser_daemon.set_defaults(func=cmd_daemon)

    # Theme Command
    parser_theme = subparsers.add_parser('theme', help='List or Clone installed Plasma Global Themes')
    parser_theme.add_argument('action', choices=['list', 'clone'], help='Action: list installed themes or clone one')
    parser_theme.add_argument('--source', type=str, help='Name of the theme to clone (for clone action)')
    parser_theme.add_argument('--dest', type=str, help='New name for the cloned theme (for clone action)')
    parser_theme.set_defaults(func=cmd_theme)

    # Log Command
    parser_log = subparsers.add_parser('log', help='View recent activity logs')
    parser_log.add_argument('-v', '--verbose', action='store_true', help='Show more lines (50 instead of 20)')
    parser_log.set_defaults(func=cmd_log)

    args = parser.parse_args()
    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
