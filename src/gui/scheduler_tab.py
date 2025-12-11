from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, 
    QPushButton, QFormLayout, QGroupBox, QComboBox, QRadioButton,
    QTimeEdit, QButtonGroup
)
from PySide6.QtCore import Qt, QTimer, QTime
import datetime

from core.config import config
from core.solar import get_solar_times
from core.kvantum import KvantumManager

class SchedulerTab(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()
        self.load_config()
        
        # Timer to update status periodically
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_status)
        self.timer.start(60000) # Every minute
        
        self.update_status()

    def init_ui(self):
        layout = QVBoxLayout()
                
        # Schedule Mode Group
        mode_group = QGroupBox("Schedule Mode")
        mode_layout = QVBoxLayout()
        
        self.mode_btn_group = QButtonGroup()
        self.solar_radio = QRadioButton("Solar (Sun Position)")
        self.custom_radio = QRadioButton("Custom Times")
        self.mode_btn_group.addButton(self.solar_radio, 0)
        self.mode_btn_group.addButton(self.custom_radio, 1)
        
        mode_layout.addWidget(self.solar_radio)
        mode_layout.addWidget(self.custom_radio)
        
        # Custom Time Inputs
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
        
        # Toggle visibility
        self.solar_radio.toggled.connect(self.toggle_time_inputs)
        
        mode_layout.addWidget(self.custom_time_widget)
        mode_group.setLayout(mode_layout)
        layout.addWidget(mode_group)

        # Location Group
        loc_group = QGroupBox("Location Settings")
        loc_layout = QFormLayout()
        
        self.lat_input = QLineEdit()
        self.lon_input = QLineEdit()
        self.lat_input.setPlaceholderText("e.g. 40.7127")
        self.lon_input.setPlaceholderText("e.g. -74.0060")
        
        loc_layout.addRow("Latitude:", self.lat_input)
        loc_layout.addRow("Longitude:", self.lon_input)
        
        save_loc_btn = QPushButton("Save Location")
        save_loc_btn.clicked.connect(self.save_location)
        loc_layout.addRow(save_loc_btn)
        
        loc_group.setLayout(loc_layout)
        layout.addWidget(loc_group)

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

        # Status Group
        status_group = QGroupBox("Current Status")
        status_layout = QVBoxLayout()
        
        self.status_label = QLabel("Calculating...")
        self.status_label.setAlignment(Qt.AlignCenter)
        self.status_label.setContentsMargins(10, 10, 10, 10)  # Add margins
        font = self.status_label.font()
        font.setPointSize(12)  # Smaller font
        self.status_label.setFont(font)
        
        btn_layout = QHBoxLayout() # Horizontal layout for buttons
        
        self.refresh_btn = QPushButton("Refresh")
        self.refresh_btn.clicked.connect(self.manual_refresh)
        
        self.apply_btn = QPushButton("Apply Now")
        self.apply_btn.clicked.connect(self.manual_apply)
        
        btn_layout.addWidget(self.refresh_btn)
        btn_layout.addWidget(self.apply_btn)
        
        status_layout.addWidget(self.status_label)
        status_layout.addLayout(btn_layout)
        status_group.setLayout(status_layout)
        layout.addWidget(status_group)
        
        layout.addStretch()
        self.setLayout(layout)

    def save_location(self):
        try:
            lat = float(self.lat_input.text())
            lon = float(self.lon_input.text())
            config.set('latitude', lat)
            config.set('longitude', lon)
            self.update_status()
        except ValueError:
            self.status_label.setText("Invalid Latitude/Longitude!")

    def load_config(self):
        self.lat_input.setText(str(config.get('latitude', 0.0)))
        self.lon_input.setText(str(config.get('longitude', 0.0)))
        
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
        day_theme = config.get('day_theme', '')
        night_theme = config.get('night_theme', '')
        self.day_theme_combo.setCurrentText(day_theme)
        self.night_theme_combo.setCurrentText(night_theme)
        
        self.toggle_time_inputs()
        
    def toggle_time_inputs(self):
        self.custom_time_widget.setVisible(self.custom_radio.isChecked())
        
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

    def manual_refresh(self):
        self.update_status(apply=False)
        
    def manual_apply(self):
        self.update_status(apply=True)

    def update_status(self, apply=False):
        now = datetime.datetime.now().astimezone()
        
        mode_setting = config.get('schedule_mode', 'solar')
        is_day = False
        sunrise_dt = None
        sunset_dt = None
        
        if mode_setting == 'solar':
            lat = float(config.get('latitude', 0.0))
            lon = float(config.get('longitude', 0.0))
            
            if lat == 0.0 and lon == 0.0:
                self.status_label.setText("Location not set.")
                return

            today_utc = datetime.datetime.now(datetime.timezone.utc).date()
            times = get_solar_times(today_utc, lat, lon)
            
            if not times:
                self.status_label.setText("Solar calc unavailable.")
                return
                
            sunrise_dt = times['sunrise'].astimezone()
            sunset_dt = times['sunset'].astimezone()
            is_day = sunrise_dt <= now < sunset_dt
            
        else: # Custom
            sunrise_str = config.get('custom_sunrise', '06:00')
            sunset_str = config.get('custom_sunset', '18:00')
            
            t_now = now.time()
            t_rise = datetime.datetime.strptime(sunrise_str, "%H:%M").time()
            t_set = datetime.datetime.strptime(sunset_str, "%H:%M").time()
            
            # Simple comparison for same day interval
            if t_rise < t_set:
                is_day = t_rise <= t_now < t_set
            else:
                # Night crossover (e.g. 20:00 to 06:00)
                is_day = not (t_set <= t_now < t_rise)
                
            sunrise_dt = now.replace(hour=t_rise.hour, minute=t_rise.minute) # approx for display
            sunset_dt = now.replace(hour=t_set.hour, minute=t_set.minute)

        mode_text = "Day" if is_day else "Night"
        target_theme = config.get('day_theme') if is_day else config.get('night_theme')
        
        rise_s = sunrise_dt.strftime('%H:%M') if sunrise_dt else "N/A"
        set_s = sunset_dt.strftime('%H:%M') if sunset_dt else "N/A"
        
        status_text = (
            f"Current: {now.strftime('%H:%M')}\n"
            f"Start: {rise_s} | End: {set_s}\n"
            f"Mode: {mode_text}\n"
            f"Active theme: {target_theme}"
        )
        self.status_label.setText(status_text)
        
        if apply:
             KvantumManager.set_theme(target_theme)
