import argparse
import sys
import datetime
import time
from pathlib import Path

# Add src to path
sys.path.append(str(Path(__file__).parent))

from core.solar import get_solar_times
from core.config import config
from core import kvantum
from core.plasma import PlasmaThemeManager
from core.gtk import GtkManager
from core.logger import setup_logger, log_activity, log_error

def is_daytime():
    """Calculates if it is currently daytime based on config."""
    lat = config.get('latitude')
    lon = config.get('longitude')
    now = datetime.datetime.now().astimezone()
    
    if config.get('schedule_mode', 'solar') == 'solar':
        if lat == 0.0 and lon == 0.0: return False
        
        today_utc = datetime.datetime.now(datetime.timezone.utc).date()
        times = get_solar_times(today_utc, lat, lon)
        if not times: return False
        
        s_rise = times['sunrise'].astimezone()
        s_set = times['sunset'].astimezone()
        
        # Apply padding
        shift = datetime.timedelta(minutes=float(config.get('solar_padding', 0)) / 2)
        return (s_rise - shift) <= now < (s_set + shift)
    else:
        # Custom Times
        try:
            t_now = now.time()
            t_rise = datetime.datetime.strptime(config.get('custom_sunrise', '06:00'), "%H:%M").time()
            t_set = datetime.datetime.strptime(config.get('custom_sunset', '18:00'), "%H:%M").time()
            return t_rise <= t_now < t_set if t_rise < t_set else not (t_set <= t_now < t_rise)
        except ValueError:
            return False

def check_and_apply(apply=True):
    """Checks requirements and applies themes if needed."""
    # 1. Verify Auto Mode is Enabled
    try:
        if not PlasmaThemeManager.is_auto_enabled() and apply:
            return # Static mode active, do not interfere
    except Exception as e:
        log_error(f"Auto-Check Failed: {e}")
        return

    # 2. Determine State
    day_mode = is_daytime()
    
    # 3. Get Targets
    target_kv = config.get("day_theme") if day_mode else config.get("night_theme")
    target_gtk = config.get("day_gtk_theme") if day_mode else config.get('night_gtk_theme')
    
    # Global Theme Targets (Best effort from Native Prefs as fallback, or UI config if we had it?)
    # The UI saves specific day/night Global Themes to Plasma's native prefs.
    # We should read those back to know what to apply.
    native_day, native_night = PlasmaThemeManager.get_native_prefs()
    target_gl = native_day if day_mode else native_night

    if not apply:
        return # Just checking, logic ends here for dry-run if we were using it that way

    # 4. Apply Global Theme
    if target_gl:
        curr_gl = PlasmaThemeManager.get_current_theme()
        if curr_gl != target_gl:
            log_activity(f"Scheduler: Switching Global Theme to {target_gl} (Day={day_mode})")
            if PlasmaThemeManager.apply_theme(target_gl):
                # Critical: Applying manually disables Auto. Re-enable it to keep daemon alive.
                PlasmaThemeManager.set_auto_enabled(True)
    
    # 5. Apply Kvantum
    if target_kv:
        if kvantum.KvantumManager.get_current_theme() != target_kv:
            log_activity(f"Scheduler: Switching Kvantum to {target_kv}")
            kvantum.KvantumManager.set_theme(target_kv)

    # 6. Apply GTK
    if target_gtk:
        if GtkManager.get_current_theme() != target_gtk:
            # GtkManager logs internally, but we can log context if needed
            GtkManager.set_theme(target_gtk)

def cmd_daemon(args):
    print("Starting Plasma Theme Master Daemon... (Ctrl+C to stop)")
    log_activity("Daemon started.")
    
    while True:
        try:
            config.load()
            check_and_apply(apply=True)
            time.sleep(60)
        except KeyboardInterrupt:
            print("\nStopping daemon.")
            log_activity("Daemon stopped by user.")
            break
        except Exception as e:
            log_error(f"Daemon error: {e}")
            time.sleep(60)

def cmd_scheduler(args):
    # CLI Config Update
    if args.lat: config.set('latitude', args.lat)
    if args.lon: config.set('longitude', args.lon)
    if args.day_theme: config.set('day_theme', args.day_theme)
    if args.night_theme: config.set('night_theme', args.night_theme)
    if args.day_global: config.set('day_global_theme', args.day_global)
    if args.night_global: config.set('night_global_theme', args.night_global)
    if args.day_gtk: config.set('day_gtk_theme', args.day_gtk)
    if args.night_gtk: config.set('night_gtk_theme', args.night_gtk)
    
    # Status Report
    is_auto = PlasmaThemeManager.is_auto_enabled()
    day_mode = is_daytime()
    print(f"System Mode: {'Auto' if is_auto else 'Static'}")
    print(f"Cycle State: {'Day' if day_mode else 'Night'}")
    
    if args.apply:
        # Force apply regardless of Auto mode? No, respect logic usually.
        # But CLI user might want to force. 
        # Let's reuse check_and_apply but maybe override the auto-check if desired?
        # For now, stick to standard logic.
        print("Applying themes...")
        check_and_apply(apply=True)
    else:
        print("Use --apply to enforce settings now.")

def cmd_theme(args):
    if args.action == 'list':
        print("Installed Global Themes:")
        for t in PlasmaThemeManager.list_installed_themes():
            print(f"- {t['name']} ({t['type']})")
            
    elif args.action == 'clone':
        if not args.source or not args.dest:
            print("Error: --source and --dest required.")
            return
        try:
            PlasmaThemeManager.clone_theme(args.source, args.dest)
            print(f"Cloned '{args.source}' -> '{args.dest}'")
        except Exception as e:
            print(f"Error: {e}")

def cmd_log(args):
    from core.logger import LOG_FILE
    if not LOG_FILE.exists():
        print("No log found.")
        return
    
    limit = 50 if args.verbose else 20
    print(f"--- Last {limit} lines ---")
    try:
        lines = LOG_FILE.read_text(encoding='utf-8').splitlines()
        for line in lines[-limit:]: print(line)
    except Exception as e:
        print(f"Read error: {e}")

def main():
    setup_logger()
    
    parser = argparse.ArgumentParser(description="Plasma Theme Master CLI")
    subparsers = parser.add_subparsers(dest='command')

    # Scheduler Command
    parser_sched = subparsers.add_parser('scheduler', help='Manage Day/Night cycle configuration and manual checks')
    parser_sched.add_argument('--lat', type=float, help='Set Latitude (e.g., 40.7)')
    parser_sched.add_argument('--lon', type=float, help='Set Longitude (e.g., -74.0)')
    parser_sched.add_argument('--day-theme', type=str, help='Set Day Kvantum Theme name')
    parser_sched.add_argument('--night-theme', type=str, help='Set Night Kvantum Theme name')
    parser_sched.add_argument('--day-global', type=str, help='Set Day Global Theme (Look and Feel) name')
    parser_sched.add_argument('--night-global', type=str, help='Set Night Global Theme (Look and Feel) name')
    parser_sched.add_argument('--day-gtk', type=str, help='Set Day GTK Theme name')
    parser_sched.add_argument('--night-gtk', type=str, help='Set Night GTK Theme name')
    parser_sched.add_argument('--apply', action='store_true', help='Apply the calculated theme immediately based on current time')
    parser_sched.set_defaults(func=cmd_scheduler)
    
    # Daemon
    p_daemon = subparsers.add_parser('daemon')
    p_daemon.set_defaults(func=cmd_daemon)

    # Theme
    p_theme = subparsers.add_parser('theme')
    p_theme.add_argument('action', choices=['list', 'clone'])
    p_theme.add_argument('--source', type=str)
    p_theme.add_argument('--dest', type=str)
    p_theme.set_defaults(func=cmd_theme)

    # Log
    p_log = subparsers.add_parser('log')
    p_log.add_argument('-v', '--verbose', action='store_true')
    p_log.set_defaults(func=cmd_log)

    args = parser.parse_args()
    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
