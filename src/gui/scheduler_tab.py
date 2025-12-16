from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, 
    QPushButton, QFormLayout, QGroupBox, QComboBox, QRadioButton,
    QTimeEdit, QButtonGroup, QSlider, QMessageBox, QScrollArea, QGridLayout
)
from PySide6.QtCore import Qt, QTimer, QTime
import datetime
from core.config import config
from core.solar import get_solar_times
from core.kvantum import KvantumManager
from core.plasma import PlasmaThemeManager
from core.gtk import GtkManager
from core.logger import log_activity, log_error


class SchedulerTab(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()
        self.load_config()
        
        # Timer to update status periodically
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_status)
        self.timer.start(5000) # Every 5 seconds
        
        self.update_status()

    def init_ui(self):
        # Main wrapper layout with ScrollArea
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        # Remove border from scroll area to blend in
        scroll.setStyleSheet("QScrollArea { border: none; }")
        content_widget = QWidget()
        layout = QVBoxLayout()
        # Add side padding for safe scrolling (Left, Top, Right, Bottom)
        layout.setContentsMargins(50, 10, 50, 10)
        
        # Status Group (Grid Layout) - MOVED TO TOP
        status_group = QGroupBox("Current Status")
        # Enhance Styling: Larger Header, More Spacing
        status_group.setStyleSheet("""
            QGroupBox::title { 
                font-size: 15pt; 
                font-weight: bold; 
                padding: 5px;
            }
            QGroupBox { 
                margin-top: 15px; 
                font-size: 10pt;
            }
        """)
        
        status_layout = QVBoxLayout()
        # Add more padding inside the box (Left, Top, Right, Bottom)
        status_layout.setContentsMargins(20, 30, 20, 20)
        status_layout.setSpacing(15)
        
        # Top Row: Info + Refresh
        top_row_layout = QHBoxLayout()
        
        self.lbl_info = QLabel("Calculating...")
        self.lbl_info.setAlignment(Qt.AlignCenter)
        self.lbl_info.setStyleSheet("font-weight: bold; font-size: 11pt;")
        
        # Small Refresh Button
        self.refresh_btn = QPushButton("⟳") 
        self.refresh_btn.setFixedSize(30, 30)
        self.refresh_btn.setToolTip("Refresh Status")
        self.refresh_btn.clicked.connect(self.manual_refresh)
        
        # Layout: Stretch - Info - Stretch - Refresh
        top_row_layout.addStretch()
        top_row_layout.addWidget(self.lbl_info)
        top_row_layout.addStretch()
        top_row_layout.addWidget(self.refresh_btn)
        
        status_layout.addLayout(top_row_layout)
        
        # Grid for Themes
        grid = QGridLayout()
        grid.addWidget(QLabel("<b>Kvantum Theme</b>"), 0, 0)
        grid.addWidget(QLabel("<b>Global Theme</b>"), 0, 1)
        
        self.lbl_kv_target = QLabel("Target: ...")
        self.lbl_kv_active = QLabel("Active: ...")
        self.lbl_gl_target = QLabel("Target: ...")
        self.lbl_gl_active = QLabel("Active: ...")
        
        # GTK Labels
        grid.addWidget(QLabel("<b>GTK Theme</b>"), 0, 2)
        self.lbl_gtk_target = QLabel("Target: ...")
        self.lbl_gtk_active = QLabel("Active: ...")
        
        grid.addWidget(self.lbl_kv_target, 1, 0)
        grid.addWidget(self.lbl_kv_active, 2, 0)
        
        grid.addWidget(self.lbl_gl_target, 1, 1)
        grid.addWidget(self.lbl_gl_active, 2, 1)
        
        grid.addWidget(self.lbl_gtk_target, 1, 2)
        grid.addWidget(self.lbl_gtk_active, 2, 2)
        
        status_layout.addLayout(grid)
        
        # Buttons removed as requested (Refresh moved to top right, Apply is redundant)
        status_group.setLayout(status_layout)
        layout.addWidget(status_group)
        
        # Theme Mode Group
        mode_select_group = QGroupBox("Theme Mode")
        mode_select_layout = QHBoxLayout() # Changed to Horizontal
        
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["Auto (Follow Schedule)", "Static Light", "Static Dark"])
        # Removed automatic trigger on change to allow user to select first, then apply.
        # self.mode_combo.currentIndexChanged.connect(self.on_mode_change)
        
        self.btn_set_mode = QPushButton("Set Mode")
        self.btn_set_mode.clicked.connect(lambda: self.on_mode_change(self.mode_combo.currentIndex()))
        
        mode_select_layout.addWidget(self.mode_combo)
        mode_select_layout.addWidget(self.btn_set_mode)
        
        mode_select_group.setLayout(mode_select_layout)
        layout.addWidget(mode_select_group)

        # Schedule Calculation Group
        mode_group = QGroupBox("Schedule Calculation")
        mode_layout = QVBoxLayout()
        
        self.mode_btn_group = QButtonGroup()
        self.solar_radio = QRadioButton("Solar (Sun Position)")
        self.custom_radio = QRadioButton("Custom Times")
        self.mode_btn_group.addButton(self.solar_radio, 0)
        self.mode_btn_group.addButton(self.custom_radio, 1)
        
        mode_layout.addWidget(self.solar_radio)
        mode_layout.addWidget(self.custom_radio)
        
        # Location Group (Nested under Solar)
        self.loc_widget = QWidget()
        loc_layout = QFormLayout()
        
        self.location_label = QLabel("Location: Not Set")
        loc_layout.addRow(self.location_label)

        self.lat_input = QLineEdit()
        self.lon_input = QLineEdit()
        self.lat_input.setPlaceholderText("e.g. 40.7127")
        self.lon_input.setPlaceholderText("e.g. -74.0060")
        
        loc_layout.addRow("Latitude:", self.lat_input)
        loc_layout.addRow("Longitude:", self.lon_input)
        
        update_loc_btn = QPushButton("Update Location")
        update_loc_btn.clicked.connect(self.update_location)
        loc_layout.addRow(update_loc_btn)
        
        # Solar Offset Slider
        self.offset_label = QLabel("Daytime Offset: 0 mins")
        self.offset_slider = QSlider(Qt.Horizontal)
        self.offset_slider.setRange(-60, 60)
        self.offset_slider.setValue(0)
        self.offset_slider.setTickPosition(QSlider.TicksBelow)
        self.offset_slider.setTickInterval(15)
        self.offset_slider.setSingleStep(5)
        
        # Connect slider
        self.offset_slider.valueChanged.connect(self.update_offset_label)
        self.offset_slider.sliderReleased.connect(self.update_location) # Save on release
        
        loc_layout.addRow(self.offset_label, self.offset_slider)
        
        self.loc_widget.setLayout(loc_layout)
        mode_layout.addWidget(self.loc_widget)

        # Custom Time Inputs (Nested under Custom)
        self.time_layout = QHBoxLayout()
        self.sunrise_edit = QTimeEdit()
        self.sunset_edit = QTimeEdit()
        self.sunrise_edit.setDisplayFormat("HH:mm")
        self.sunset_edit.setDisplayFormat("HH:mm")
        
        self.time_layout.addWidget(QLabel("Day Start:"))
        self.time_layout.addWidget(self.sunrise_edit)
        self.time_layout.addWidget(QLabel("Night Start:"))
        self.time_layout.addWidget(self.sunset_edit)
        
        self.custom_time_widget = QWidget()
        self.custom_time_widget.setLayout(self.time_layout)
        mode_layout.addWidget(self.custom_time_widget)
        
        # Toggle visibility
        self.solar_radio.toggled.connect(self.toggle_mode_inputs)
        
        mode_group.setLayout(mode_layout)
        layout.addWidget(mode_group)

        # Theme Group
        theme_group = QGroupBox("Kvantum Theme Settings")
        theme_layout = QFormLayout()
        
        self.day_theme_combo = QComboBox()
        self.night_theme_combo = QComboBox()
        
        # Populate Themes
        themes = KvantumManager.list_themes()
        self.day_theme_combo.addItems(themes)
        self.night_theme_combo.addItems(themes)
        
        theme_layout.addRow("Day Theme:", self.day_theme_combo)
        theme_layout.addRow("Night Theme:", self.night_theme_combo)
        
        save_msg = QLabel("Settings are saved automatically on change.")
        save_msg.setStyleSheet("color: gray;")
        theme_layout.addRow(save_msg)
        
        # Connect signals
        self.day_theme_combo.currentTextChanged.connect(self.save_themes)
        self.night_theme_combo.currentTextChanged.connect(self.save_themes)
        self.solar_radio.toggled.connect(self.save_mode)
        self.custom_radio.toggled.connect(self.save_mode)
        self.sunrise_edit.timeChanged.connect(self.save_mode)
        self.sunset_edit.timeChanged.connect(self.save_mode)
        
        theme_group.setLayout(theme_layout)
        layout.addWidget(theme_group)
        
        # Global Theme Group
        global_group = QGroupBox("Global Theme Settings (Native)")
        global_layout = QFormLayout()
        
        self.day_global_combo = QComboBox()
        self.night_global_combo = QComboBox()
        self.day_global_combo.setEditable(False) # Native themes shouldn't be arbitrary text
        self.night_global_combo.setEditable(False)
        
        # Populate Global Themes
        global_themes = PlasmaThemeManager.list_installed_themes()
        global_theme_names = [t['name'] for t in global_themes]
        
        self.day_global_combo.addItems(global_theme_names)
        self.night_global_combo.addItems(global_theme_names)
        
        global_layout.addRow("Day Global Theme:", self.day_global_combo)
        global_layout.addRow("Night Global Theme:", self.night_global_combo)
        
        global_explainer = QLabel("These settings control Plasma's native auto-switch behavior.")
        global_explainer.setStyleSheet("color: gray; font-size: 10px;")
        global_layout.addRow(global_explainer)
        
        # Connect signals
        self.day_global_combo.currentTextChanged.connect(self.save_global_themes)
        self.night_global_combo.currentTextChanged.connect(self.save_global_themes)

        global_group.setLayout(global_layout)
        layout.addWidget(global_group)
        
        # GTK Theme Group
        gtk_group = QGroupBox("GTK Application Theme Settings")
        gtk_layout = QFormLayout()
        
        self.day_gtk_combo = QComboBox()
        self.night_gtk_combo = QComboBox()
        
        # Populate GTK Themes
        gtk_themes = GtkManager.list_themes()
        self.day_gtk_combo.addItems(gtk_themes)
        self.night_gtk_combo.addItems(gtk_themes)
        
        gtk_layout.addRow("Day GTK Theme:", self.day_gtk_combo)
        gtk_layout.addRow("Night GTK Theme:", self.night_gtk_combo)
        
        self.day_gtk_combo.currentTextChanged.connect(self.save_gtk_themes)
        self.night_gtk_combo.currentTextChanged.connect(self.save_gtk_themes)
        
        gtk_group.setLayout(gtk_layout)
        layout.addWidget(gtk_group)
        
        layout.addStretch()
        content_widget.setLayout(layout)
        scroll.setWidget(content_widget)
        
        main_layout.addWidget(scroll)
        self.setLayout(main_layout)

    def update_location(self):
        try:
            lat = float(self.lat_input.text())
            lon = float(self.lon_input.text())
            config.set('latitude', lat)
            config.set('longitude', lon)
            config.set('solar_padding', self.offset_slider.value())
            
            # Update formatted label
            lat_dir = "N" if lat >= 0 else "S"
            lon_dir = "E" if lon >= 0 else "W"
            self.location_label.setText(f"Location: {abs(lat):.1f}° {lat_dir}, {abs(lon):.1f}° {lon_dir}")
            
            self.update_status()
        except ValueError:
            self.lbl_info.setText("Invalid Latitude/Longitude!")

    def load_config(self):
        # Load Application Mode
        is_auto = PlasmaThemeManager.is_auto_enabled()
        self.mode_combo.blockSignals(True)
        if is_auto:
            self.mode_combo.setCurrentIndex(0) # Auto
        else:
            # In static mode, we default to showing "Auto" as selected if we can't determine otherwise,
            # or could implement logic to detect if we match "Static Light" or "Static Dark".
            # For now, leaving as index 0 (Auto) is safe.
            pass
            
        self.mode_combo.blockSignals(False)

        self.lat_input.setText(str(config.get('latitude', 0.0)))
        self.lon_input.setText(str(config.get('longitude', 0.0)))
        
        padding = config.get('solar_padding', 0)
        self.offset_slider.setValue(int(padding))
        self.update_offset_label(int(padding))
        
        # Load switch mode
        mode = config.get('schedule_mode', 'solar')
        if mode == 'custom':
            self.custom_radio.setChecked(True)
        else:
            self.solar_radio.setChecked(True)
            
        # Load times
        sunrise_str = config.get('custom_sunrise', '06:00')
        sunset_str = config.get('custom_sunset', '18:00')
        self.sunrise_edit.setTime(QTime.fromString(sunrise_str, "HH:mm"))
        self.sunset_edit.setTime(QTime.fromString(sunset_str, "HH:mm"))

        # Load Themes
        self.day_theme_combo.blockSignals(True)
        self.night_theme_combo.blockSignals(True)
        day_theme = config.get('day_theme', '')
        night_theme = config.get('night_theme', '')
        self.day_theme_combo.setCurrentText(day_theme)
        self.night_theme_combo.setCurrentText(night_theme)
        self.day_theme_combo.blockSignals(False)
        self.night_theme_combo.blockSignals(False)
        
        # Load Global Themes (Native)
        self.day_global_combo.blockSignals(True)
        self.night_global_combo.blockSignals(True)
        native_day, native_night = PlasmaThemeManager.get_native_prefs()
        if native_day: self.day_global_combo.setCurrentText(native_day)
        if native_night: self.night_global_combo.setCurrentText(native_night)
        self.day_global_combo.blockSignals(False)
        self.night_global_combo.blockSignals(False)
        
        # Load GTK Themes
        self.day_gtk_combo.blockSignals(True)
        self.night_gtk_combo.blockSignals(True)
        day_gtk = config.get('day_gtk_theme', 'Breeze')
        night_gtk = config.get('night_gtk_theme', 'Breeze')
        self.day_gtk_combo.setCurrentText(day_gtk)
        self.night_gtk_combo.setCurrentText(night_gtk)
        self.day_gtk_combo.blockSignals(False)
        self.night_gtk_combo.blockSignals(False)

        # Init location label
        self.update_location()
        
        self.toggle_mode_inputs()
    
    def update_offset_label(self, value):
        self.offset_label.setText(f"Daytime Offset: {value} mins")
        
    def toggle_mode_inputs(self):
        is_custom = self.custom_radio.isChecked()
        self.custom_time_widget.setVisible(is_custom)
        self.loc_widget.setVisible(not is_custom)
        
    def save_mode(self):
        mode = "custom" if self.custom_radio.isChecked() else "solar"
        config.set('schedule_mode', mode)
        config.set('custom_sunrise', self.sunrise_edit.time().toString("HH:mm"))
        config.set('custom_sunset', self.sunset_edit.time().toString("HH:mm"))
        self.update_status()

    def save_themes(self):
        config.set('day_theme', self.day_theme_combo.currentText())
        config.set('night_theme', self.night_theme_combo.currentText())
        self.update_status()

    def save_global_themes(self):
        day = self.day_global_combo.currentText()
        night = self.night_global_combo.currentText()
        if day and night:
            # Avoid writing blank if loading
            PlasmaThemeManager.set_native_prefs(day, night)

    def save_gtk_themes(self):
        config.set('day_gtk_theme', self.day_gtk_combo.currentText())
        config.set('night_gtk_theme', self.night_gtk_combo.currentText())
        self.update_status()

    def on_mode_change(self, index):
        try:
            native_day, native_night = PlasmaThemeManager.get_native_prefs()
            day_kv = self.day_theme_combo.currentText()
            night_kv = self.night_theme_combo.currentText()
            day_global = self.day_global_combo.currentText() or native_day
            night_global = self.night_global_combo.currentText() or native_night
            
            day_gtk = self.day_gtk_combo.currentText()
            night_gtk = self.night_gtk_combo.currentText()
            
            if index == 0: # Auto
                log_activity("Switching to Auto Mode...")
                
                # 1. Determine which Global Theme SHOULD be active right now
                # We need to reuse the time calculation logic from update_status, 
                # or just call update_status logic locally.
                # To be safe/clean, let's fetch the native prefs and check against current time.
                
                # We can trigger a manual update *before* enabling auto, 
                # but we must ensure we re-enable auto after.
                
                # Let's rely on update_status(apply=True) BUT we must temporarily DISABLE the guard!
                # Actually, simpler: Manually apply here, then Enable Auto.
                
                # Re-calculate is_day to know which theme to apply
                # Copying simplified time logic for safety:
                now = datetime.datetime.now().astimezone()
                is_day_temp = False
                mode = config.get('schedule_mode', 'solar')
                
                # Quick re-calc (we trust config is loaded)
                if mode == 'solar':
                     lat = float(config.get('latitude', 0.0))
                     lon = float(config.get('longitude', 0.0))
                     today_utc = datetime.datetime.now(datetime.timezone.utc).date()
                     # If invalid loc, we can't guess, so skip manual apply
                     if lat != 0.0 or lon != 0.0:
                         times = get_solar_times(today_utc, lat, lon)
                         if times:
                             s_rise = times['sunrise'].astimezone()
                             s_set = times['sunset'].astimezone()
                             pad = float(config.get('solar_padding', 0))
                             shift = datetime.timedelta(minutes=pad/2)
                             if s_rise <= now < s_set: is_day_temp = True
                else:
                    # Custom
                    s_rise = config.get('custom_sunrise', '06:00')
                    s_set = config.get('custom_sunset', '18:00')
                    # ... simple string compare if needed, or just let update_status handle it?
                    # Let's reuse the update_status logic by calling it with specific flag?
                    # No, update_status is too monolithic.
                    pass 
                    # If simplified logic is too complex to duplicate, let's just 
                    # FORCE APPLY the one we think is right.
                
                # Actually, easiest way: 
                # 1. Force the correct theme (by forcing auto=False temporarily implicitly)
                # 2. Then set Auto=True.
                
                # Let's call update_status with a special "force_manual" override? No.
                # Let's just run the full update_status logic but FORCE auto=false temporarily just for the apply
                
                # Correct approach:
                # 1. Set Auto = True (Config is right).
                # 2. Check if current Global Theme matches Target.
                # 3. If NO, manually apply Target (which sets Auto=False).
                # 4. If we manually applied, Set Auto = True AGAIN.
                
                PlasmaThemeManager.set_auto_enabled(True)
                
                # We need to know what the target is to check alignment.
                self.update_status(apply=False) # Refreshes labels and internal state calculation? 
                # update_status doesn't return target.
                
                # Let's modify logic to just blindly apply status once with Native Auto check disabled?
                # No, that modifies code elsewhere.
                
                # Let's just do it manually here.
                nav_day, nav_night = PlasmaThemeManager.get_native_prefs()
                # We need to know is_day. 
                # Let's force a "refresh" of the UI state which calculates is_day
                self.update_status(apply=False) 
                
                # Now read the *labels* we just updated? (A bit hacky but works for GUI)
                target_gl = self.lbl_gl_target.text().replace("Target: ", "")
                active_gl = self.lbl_gl_active.text().replace("Active: ", "")
                
                if target_gl and target_gl != "..." and target_gl != active_gl:
                    log_activity(f"Auto Mode Transition: Manually sycing theme to {target_gl}...")
                    PlasmaThemeManager.apply_theme(target_gl) # This sets Auto=False usually
                    # Re-enable Auto
                    PlasmaThemeManager.set_auto_enabled(True)
                
                # Also apply Kvantum/GTK
                self.update_status(apply=True) # This will skip Global (since Auto=True) but apply Kvantum/GTK
                
                log_activity("Auto Mode Enabled & Synced.")
                
            elif index == 1: # Static Light
                log_activity("Switching to Static Light Mode...")
                PlasmaThemeManager.set_auto_enabled(False)
                if day_kv: KvantumManager.set_theme(day_kv)
                if day_global: PlasmaThemeManager.apply_theme(day_global)
                if day_gtk: GtkManager.set_theme(day_gtk)
                
            elif index == 2: # Static Dark
                log_activity("Switching to Static Dark Mode...")
                PlasmaThemeManager.set_auto_enabled(False)
                if night_kv: KvantumManager.set_theme(night_kv)
                if night_global: PlasmaThemeManager.apply_theme(night_global)
                if night_gtk: GtkManager.set_theme(night_gtk)
                
        except Exception as e:
            from PySide6.QtWidgets import QMessageBox
            log_error(f"Mode Change Failed: {e}")
            QMessageBox.critical(self, "Mode Change Failed", f"An error occurred while switching modes:\n{e}")
        
        # specific update_status(apply=False) is already done inside Auto block or redundant
        if index != 0:
             self.update_status(apply=False)

    def manual_refresh(self):
        self.update_status(apply=False)

    def update_status(self, apply=False):
        now = datetime.datetime.now().astimezone()
        
        mode_setting = config.get('schedule_mode', 'solar')
        is_day = False
        sunrise_dt = None
        sunset_dt = None
        
        # Calculate Sun/Time state
        if mode_setting == 'solar':
            lat = float(config.get('latitude', 0.0))
            lon = float(config.get('longitude', 0.0))
            
            if lat == 0.0 and lon == 0.0:
                self.lbl_info.setText("Location not set.")
                return

            today_utc = datetime.datetime.now(datetime.timezone.utc).date()
            times = get_solar_times(today_utc, lat, lon)
            
            if not times:
                self.lbl_info.setText("Solar calc unavailable.")
                return
                
            sunrise_dt = times['sunrise'].astimezone()
            sunset_dt = times['sunset'].astimezone()
            
            padding = float(config.get('solar_padding', 0))
            shift = datetime.timedelta(minutes=padding / 2)
            sunrise_dt = sunrise_dt - shift
            sunset_dt = sunset_dt + shift
            
            is_day = sunrise_dt <= now < sunset_dt
            
        else: # Custom
            sunrise_str = config.get('custom_sunrise', '06:00')
            sunset_str = config.get('custom_sunset', '18:00')
            
            t_now = now.time()
            t_rise = datetime.datetime.strptime(sunrise_str, "%H:%M").time()
            t_set = datetime.datetime.strptime(sunset_str, "%H:%M").time()
            
            if t_rise < t_set:
                is_day = t_rise <= t_now < t_set
            else:
                is_day = not (t_set <= t_now < t_rise)
                
            sunrise_dt = now.replace(hour=t_rise.hour, minute=t_rise.minute)
            sunset_dt = now.replace(hour=t_set.hour, minute=t_set.minute)

        mode_text = "Day" if is_day else "Night"
        
        # Determine Targets
        target_kv = config.get('day_theme') if is_day else config.get('night_theme')
        
        # For Global Target, we trust what's in the UI specific combos for Day/Night calculation
        # But we must be careful: UI might update config later.
        # Let's read from combos if possible, or config/native prefs as fallback?
        # Ideally, Kvantum target uses config. Global target uses Native Prefs.
        native_day, native_night = PlasmaThemeManager.get_native_prefs()
        target_gl = native_day if is_day else native_night
        
        target_gtk = config.get('day_gtk_theme') if is_day else config.get('night_gtk_theme')
        
        # Get Active States
        active_kv = KvantumManager.get_current_theme() or "Unknown"
        active_gl = PlasmaThemeManager.get_current_theme() or "Unknown"
        active_gtk = GtkManager.get_current_theme() or "Unknown"

        rise_s = sunrise_dt.strftime('%H:%M') if sunrise_dt else "N/A"
        set_s = sunset_dt.strftime('%H:%M') if sunset_dt else "N/A"
        
        # Update UI Labels
        # Native Auto Status
        is_native_auto = PlasmaThemeManager.is_auto_enabled()
        native_status_str = "Auto (Scheduled)" if is_native_auto else "Static (Manual)"

        # Update UI Labels
        info_text = (
            f"Current: {now.strftime('%H:%M')} | Cycle: {mode_text}\n"
            f"Start: {rise_s} | End: {set_s}\n"
            f"System Mode: {native_status_str}"
        )
        self.lbl_info.setText(info_text)
        
        self.lbl_kv_target.setText(f"Target: {target_kv}")
        self.lbl_kv_active.setText(f"Active: {active_kv}")
        
        self.lbl_gl_target.setText(f"Target: {target_gl}")
        self.lbl_gl_active.setText(f"Active: {active_gl}")
        
        self.lbl_gtk_target.setText(f"Target: {target_gtk}")
        self.lbl_gtk_active.setText(f"Active: {active_gtk}")
        
        if apply:
             log_activity(f"Manual Apply: Switching to '{target_kv}' (Mode: {mode_text})")
             try:
                 # Apply Global first, as it might reset Kvantum
                 if target_gl:
                     # CRITICAL FIX: Do not manually apply global theme if Native Auto is ON.
                     # Manually applying a theme usually disables the native auto-scheduler.
                     if not PlasmaThemeManager.is_auto_enabled():
                         PlasmaThemeManager.apply_theme(target_gl)
                         log_activity(f"Applied Global: {target_gl}")
                     else:
                         log_activity(f"Skipping manual Global Theme apply for '{target_gl}' (Native Auto Mode is Active)")
                 else:
                     log_error(f"Failed Global: {target_gl} (target not set)")
                     
                 # Then apply Kvantum
                 if KvantumManager.set_theme(target_kv):
                     log_activity(f"Applied Kvantum: {target_kv}")
                 else:
                     log_error(f"Failed Kvantum: {target_kv}")
                      
                 # Apply GTK
                 if target_gtk:
                     if GtkManager.set_theme(target_gtk):
                         log_activity(f"Applied GTK: {target_gtk}")
                     else:
                         log_error(f"Failed GTK: {target_gtk}")
                     
             except Exception as e:
                 log_error(f"Error acting on manual apply: {e}")
