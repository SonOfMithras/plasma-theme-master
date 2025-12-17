
#include "MainWindow.h"
#include "GlobalThemeEditor.h"
#include "../core/ThemeReader.h"
#include "../core/ThemeWriter.h"
#include "../core/Solar.h"
#include "core/Logger.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include <QTabWidget>
#include <QTextEdit>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    populateThemes();
    loadSettings();
    refreshStatus();
    
    // Initial log update
    updateLogs();
    
    // Timer for logs (every 2 seconds)
    m_logTimer = new QTimer(this);
    connect(m_logTimer, &QTimer::timeout, this, &MainWindow::updateLogs);
    m_logTimer->start(2000);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    // QWidget *centralWidget = new QWidget(this); // Unused
    
    // The main tab widget will be the central widget
    m_mainTabs = new QTabWidget(this);
    setCentralWidget(m_mainTabs);

    setupDashboardTab();
    setupLogsTab();
    
    m_mainTabs->addTab(m_dashboardTab, tr("Dashboard"));
    
    // Global Theme Editor Tab
    m_globalEditorTab = new GlobalThemeEditor(this);
    m_mainTabs->addTab(m_globalEditorTab, tr("Global Theme Editor"));
    
    // Logs Tab
    m_mainTabs->addTab(m_logsTab, tr("Logs"));

    // Menu Bar
    setupMenuBar();

    setWindowTitle(tr("Plasma Theme Master"));
    resize(550, 650);
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar); // QMainWindow takes ownership
    
    // --- File Menu ---
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    
    QAction *openGlobalAction = new QAction(tr("Open &Global Themes Folder"), this);
    connect(openGlobalAction, &QAction::triggered, this, &MainWindow::openGlobalThemesFolder);
    fileMenu->addAction(openGlobalAction);

    QAction *openKvantumAction = new QAction(tr("Open &Kvantum Themes Folder"), this);
    connect(openKvantumAction, &QAction::triggered, this, &MainWindow::openKvantumThemesFolder);
    fileMenu->addAction(openKvantumAction);

    QAction *openGtkAction = new QAction(tr("Open G&TK Themes Folder"), this);
    connect(openGtkAction, &QAction::triggered, this, &MainWindow::openGtkThemesFolder);
    fileMenu->addAction(openGtkAction);

    fileMenu->addSeparator();

    QAction *openConfigAction = new QAction(tr("Open &App Config Folder"), this);
    connect(openConfigAction, &QAction::triggered, this, &MainWindow::openAppConfigFolder);
    fileMenu->addAction(openConfigAction);
    
    fileMenu->addSeparator();

    QAction *quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(quitAction);
    
    // --- Help Menu ---
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    
    QAction *clearLogAction = new QAction(tr("Clear &Log"), this);
    connect(clearLogAction, &QAction::triggered, this, &MainWindow::clearLogs);
    helpMenu->addAction(clearLogAction);

    QAction *clearConfigAction = new QAction(tr("Clear &Config"), this);
    connect(clearConfigAction, &QAction::triggered, this, &MainWindow::clearConfig);
    helpMenu->addAction(clearConfigAction);

    helpMenu->addSeparator();

    QAction *uninstallAction = new QAction(tr("&Uninstall App"), this);
    connect(uninstallAction, &QAction::triggered, this, &MainWindow::triggerUninstall);
    helpMenu->addAction(uninstallAction);
    
    helpMenu->addSeparator();

    QAction *aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::showAbout() {
    QMessageBox::about(this, tr("About Plasma Theme Master"),
                       tr("<h3>Plasma Theme Master</h3>"
                          "<p>Version 2.0 (C++ Edition)</p>"
                          "<p>Automatic Day/Night Theme Switcher for KDE Plasma.</p>"
                          "<p><b>Author:</b> Ammar Al-Riyamy<br>"
                          "<b>GitHub:</b> <a href='https://github.com/SonOfMithras'>https://github.com/SonOfMithras</a></p>"
                          "<p>Features:</p>"
                          "<ul>"
                          "<li>Solar calculation to sync themes</li>"
                          "<li>Global Theme Switching</li>"
                          "<li>Kvantum & GTK Sync</li>"
                          "<li>Global Theme Editor & Backup</li>"
                          "</ul>"
                          "<p>Redesigned in C++ and Qt6.</p>"));
}

void MainWindow::setupDashboardTab() {
    m_dashboardTab = new QWidget(this);
    QVBoxLayout *tabLayout = new QVBoxLayout(m_dashboardTab);
    tabLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(m_dashboardTab);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *dashboardContent = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(dashboardContent);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    scrollArea->setWidget(dashboardContent);
    tabLayout->addWidget(scrollArea);

    // --- Status Section ---
    QGroupBox *statusGroup = new QGroupBox(dashboardContent);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    QLabel *statusHeader = new QLabel("<h2><b>System Status</b></h2>", dashboardContent);
    statusLayout->addWidget(statusHeader);
    
    m_statusLabel = new QLabel(dashboardContent);
    m_sunInfoLabel = new QLabel(dashboardContent);
    
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addSpacing(15); 
    statusLayout->addWidget(m_sunInfoLabel);
    
    layout->addWidget(statusGroup);

    // --- Solar Settings ---
    QGroupBox *solarGroup = new QGroupBox(tr("Solar Configuration"), this);
    QHBoxLayout *solarLayout = new QHBoxLayout(solarGroup);
    m_offsetSlider = new QSlider(Qt::Horizontal, this);
    m_offsetSlider->setRange(-60, 60);
    m_offsetSlider->setTickInterval(15);
    m_offsetSlider->setTickPosition(QSlider::TicksBelow);
    m_offsetValueLabel = new QLabel("0 mins", this);
    m_offsetValueLabel->setFixedWidth(60);
    
    solarLayout->addWidget(new QLabel(tr("Daytime Offset:")));
    solarLayout->addWidget(m_offsetSlider);
    solarLayout->addWidget(m_offsetValueLabel);
    
    connect(m_offsetSlider, &QSlider::valueChanged, this, &MainWindow::onOffsetChanged);
    connect(m_offsetSlider, &QSlider::sliderReleased, this, &MainWindow::saveSettings);
    
    layout->addWidget(solarGroup);

    // --- Theme Configuration ---
    // --- Theme Configuration ---
    QGroupBox *configGroup = new QGroupBox(tr("Theme Defaults"), this);
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
    configLayout->setSpacing(10);
    
    // Global
    QLabel *globalLabel = new QLabel(tr("<b>Global Theme</b>"), this);
    configLayout->addWidget(globalLabel);
    QFormLayout *globalLayout = new QFormLayout();
    m_globalDayCombo = new QComboBox(this);
    m_globalNightCombo = new QComboBox(this);
    globalLayout->addRow(tr("Day:"), m_globalDayCombo);
    globalLayout->addRow(tr("Night:"), m_globalNightCombo);
    configLayout->addLayout(globalLayout);
    
    // Kvantum
    QLabel *kvantumLabel = new QLabel(tr("<b>Kvantum Theme</b>"), this);
    configLayout->addWidget(kvantumLabel);
    QFormLayout *kvantumLayout = new QFormLayout();
    m_kvantumDayCombo = new QComboBox(this);
    m_kvantumNightCombo = new QComboBox(this);
    kvantumLayout->addRow(tr("Day:"), m_kvantumDayCombo);
    kvantumLayout->addRow(tr("Night:"), m_kvantumNightCombo);
    configLayout->addLayout(kvantumLayout);
    
    // GTK
    QLabel *gtkLabel = new QLabel(tr("<b>GTK Theme</b>"), this);
    configLayout->addWidget(gtkLabel);
    QFormLayout *gtkLayout = new QFormLayout();
    m_gtkDayCombo = new QComboBox(this);
    m_gtkNightCombo = new QComboBox(this);
    gtkLayout->addRow(tr("Day:"), m_gtkDayCombo);
    gtkLayout->addRow(tr("Night:"), m_gtkNightCombo);
    configLayout->addLayout(gtkLayout);

    connect(m_globalDayCombo, &QComboBox::activated, this, &MainWindow::saveSettings);
    connect(m_globalNightCombo, &QComboBox::activated, this, &MainWindow::saveSettings);
    connect(m_kvantumDayCombo, &QComboBox::activated, this, &MainWindow::saveSettings);
    connect(m_kvantumNightCombo, &QComboBox::activated, this, &MainWindow::saveSettings);
    connect(m_gtkDayCombo, &QComboBox::activated, this, &MainWindow::saveSettings);
    connect(m_gtkNightCombo, &QComboBox::activated, this, &MainWindow::saveSettings);
    
    layout->addWidget(configGroup);
    
    // --- Automation Control ---
    QGroupBox *autoGroup = new QGroupBox(tr("Automation Control"), this);
    QHBoxLayout *autoLayout = new QHBoxLayout(autoGroup);
    m_autoCheck = new QCheckBox(tr("Enable Auto-Switch"), this);
    connect(m_autoCheck, &QCheckBox::toggled, this, &MainWindow::toggleAuto);
    
    m_refreshButton = new QPushButton(tr("Refresh Status"), this);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshStatus);

    autoLayout->addWidget(m_autoCheck);
    autoLayout->addStretch();
    autoLayout->addWidget(m_refreshButton);
    layout->addWidget(autoGroup);
    
    // --- Manual Overrides ---
    QGroupBox *manualGroup = new QGroupBox(tr("Manual Override"), this);
    QHBoxLayout *manualLayout = new QHBoxLayout(manualGroup);
    m_applyDayBtn = new QPushButton(tr("Force Day Mode"), this);
    m_applyNightBtn = new QPushButton(tr("Force Night Mode"), this);
    
    connect(m_applyDayBtn, &QPushButton::clicked, this, &MainWindow::applyStaticDay);
    connect(m_applyNightBtn, &QPushButton::clicked, this, &MainWindow::applyStaticNight);
    
    manualLayout->addWidget(m_applyDayBtn);
    manualLayout->addWidget(m_applyNightBtn);
    layout->addWidget(manualGroup);
    
    layout->addStretch(); // Push everything up
}

void MainWindow::setupLogsTab() {
    m_logsTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_logsTab);
    layout->setContentsMargins(10, 10, 10, 10);
    
    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    m_logViewer->setFont(QFont("Monospace"));
    layout->addWidget(m_logViewer);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_clearLogBtn = new QPushButton(tr("Clear Log"), this);
    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::clearLogs);
    
    btnLayout->addStretch();
    btnLayout->addWidget(m_clearLogBtn);
    layout->addLayout(btnLayout);
}

void MainWindow::updateLogs() {
    if (m_mainTabs->currentWidget() != m_logsTab) return; // Only update if visible
    
    QStringList logs = Logger::readLogs(200, false);
    // Reverse order: Newest at top
    QString reversedContent;
    for (int i = logs.size() - 1; i >= 0; --i) {
        reversedContent += logs[i] + "\n";
    }
    
    // Only update if changed to avoid flicker/scroll jump
    if (m_logViewer->toPlainText() != reversedContent) {
        m_logViewer->setPlainText(reversedContent);
    }
}

void MainWindow::clearLogs() {
    Logger::clear();
    m_logViewer->clear();
}

void MainWindow::populateThemes() {
    QStringList globalThemes = ThemeReader::listGlobalThemes();
    QStringList kvantumThemes = ThemeReader::listKvantumThemes();
    
    m_globalDayCombo->addItems(globalThemes);
    m_globalNightCombo->addItems(globalThemes);
    m_kvantumDayCombo->addItems(kvantumThemes);
    m_kvantumNightCombo->addItems(kvantumThemes);

    QStringList gtkThemes = ThemeReader::listGtkThemes();
    m_gtkDayCombo->addItems(gtkThemes);
    m_gtkNightCombo->addItems(gtkThemes);
}

void MainWindow::loadSettings() {
    // Block signals to prevent auto-save during load
    bool blocked = blockSignals(true);
    
    // Global Defaults
    m_globalDayCombo->setCurrentText(ThemeReader::defaultLightTheme());
    m_globalNightCombo->setCurrentText(ThemeReader::defaultDarkTheme());
    
    // Kvantum Defaults
    QString kDay = ThemeReader::dayKvantumTheme();
    QString kNight = ThemeReader::nightKvantumTheme();
    if (!kDay.isEmpty()) m_kvantumDayCombo->setCurrentText(kDay);
    if (!kNight.isEmpty()) m_kvantumNightCombo->setCurrentText(kNight);
    
    // GTK Defaults
    QString gDay = ThemeReader::dayGtkTheme();
    QString gNight = ThemeReader::nightGtkTheme();
    if (!gDay.isEmpty()) m_gtkDayCombo->setCurrentText(gDay);
    if (!gNight.isEmpty()) m_gtkNightCombo->setCurrentText(gNight);
    
    // Solar Padding
    int padding = ThemeReader::solarPadding();
    m_offsetSlider->setValue(padding);
    m_offsetValueLabel->setText(QString("%1 mins").arg(padding));
    
    // Auto Switch
    m_autoCheck->setChecked(ThemeReader::isAutoLookAndFeel());
    
    blockSignals(blocked);
}

void MainWindow::saveSettings() {
    ThemeWriter::setDefaultLightTheme(m_globalDayCombo->currentText());
    ThemeWriter::setDefaultDarkTheme(m_globalNightCombo->currentText());
    ThemeWriter::setDayKvantumTheme(m_kvantumDayCombo->currentText());
    ThemeWriter::setNightKvantumTheme(m_kvantumNightCombo->currentText());
    ThemeWriter::setDayGtkTheme(m_gtkDayCombo->currentText());
    ThemeWriter::setNightGtkTheme(m_gtkNightCombo->currentText());
    ThemeWriter::setSolarPadding(m_offsetSlider->value());
    
    // If Auto is enabled, re-apply logic immediately to reflect changes
    if (m_autoCheck->isChecked()) {
        toggleAuto(true);
    }
    
    refreshStatus();
}

void MainWindow::toggleAuto(bool checked) {
    if (checked) {
        // Apply logic FIRST, then enable auto.
        double lat = ThemeReader::nativeLatitude();
        double lon = ThemeReader::nativeLongitude();
        int offset = ThemeReader::solarPadding();
        bool isDay = Solar::isDaytime(lat, lon, offset);
        
        QString global = isDay ? m_globalDayCombo->currentText() : m_globalNightCombo->currentText();
        QString kvantum = isDay ? m_kvantumDayCombo->currentText() : m_kvantumNightCombo->currentText();
        
        if (!global.isEmpty()) ThemeWriter::applyGlobalTheme(global);
        if (!kvantum.isEmpty()) ThemeWriter::setKvantumTheme(kvantum);
        
        QString gtk = isDay ? m_gtkDayCombo->currentText() : m_gtkNightCombo->currentText();
        if (!gtk.isEmpty()) ThemeWriter::setGtkTheme(gtk);
        
        // Ensure this is set LAST, as applyGlobalTheme might have reset it
        ThemeWriter::setAutoLookAndFeel(true);
    } else {
        ThemeWriter::setAutoLookAndFeel(false);
    }
    refreshStatus();
}

void MainWindow::applyStaticDay() {
    m_autoCheck->setChecked(false); // Disables auto
    ThemeWriter::applyGlobalTheme(m_globalDayCombo->currentText());
    ThemeWriter::setKvantumTheme(m_kvantumDayCombo->currentText());
    ThemeWriter::setGtkTheme(m_gtkDayCombo->currentText());
    refreshStatus();
}

void MainWindow::applyStaticNight() {
    m_autoCheck->setChecked(false); // Disables auto
    ThemeWriter::applyGlobalTheme(m_globalNightCombo->currentText());
    ThemeWriter::setKvantumTheme(m_kvantumNightCombo->currentText());
    ThemeWriter::setGtkTheme(m_gtkNightCombo->currentText());
    refreshStatus();
}

void MainWindow::onOffsetChanged(int value) {
    m_offsetValueLabel->setText(QString("%1 mins").arg(value));
    // Don't save on every tick, handled by sliderReleased
}

void MainWindow::refreshStatus() {
    // Current Global
    QString global = ThemeReader::currentGlobalTheme();
    QString kvantum = ThemeReader::currentKvantumTheme();
    QString gtk = ThemeReader::currentGtkTheme();
    QString autoLabel = ThemeReader::isAutoLookAndFeel() ? "Enabled" : "Disabled";
    
    m_statusLabel->setText(tr("<b>Global:</b> %1<br><b>Kvantum:</b> %2<br><b>GTK:</b> %3<br><b>Auto:</b> %4")
                           .arg(global, kvantum, gtk, autoLabel));
                           
    // Solar Info
    double lat = ThemeReader::nativeLatitude();
    double lon = ThemeReader::nativeLongitude();
    int offset = m_offsetSlider->value();
    
    bool isDay = Solar::isDaytime(lat, lon, offset);
    QString target = isDay ? "Day" : "Night";

    m_statusLabel->setText(tr("<b>Global:</b> %1<br><b>Kvantum:</b> %2<br><b>GTK:</b> %3<br><b>Auto:</b> %4<br><br><b>Current target:</b> %5")
                           .arg(global, kvantum, gtk, autoLabel, target));
                           
    QPair<QDateTime, QDateTime> times = Solar::calculateSunTimes(lat, lon, QDate::currentDate());
    QString sr = times.first.isValid() ? times.first.toLocalTime().toString("HH:mm") : "N/A";
    QString ss = times.second.isValid() ? times.second.toLocalTime().toString("HH:mm") : "N/A";
    
    // Calculated
    int shift = (offset * 60) / 2;
    QDateTime start = times.first.isValid() ? times.first.addSecs(-shift) : QDateTime();
    QDateTime end = times.second.isValid() ? times.second.addSecs(shift) : QDateTime();
    
    QString startStr = start.isValid() ? start.toLocalTime().toString("HH:mm") : "N/A";
    QString endStr = end.isValid() ? end.toLocalTime().toString("HH:mm") : "N/A";
    
    m_sunInfoLabel->setText(tr("<b>Location:</b> %1, %2<br><b>Sunrise:</b> %3 <b>Sunset:</b> %4<br>"
                               "<b>Day Start:</b> %5 <b>Night Start:</b> %6")
                            .arg(lat).arg(lon).arg(sr, ss, startStr, endStr));
    
    // Sync checkboxes if changed externally
    bool isAuto = ThemeReader::isAutoLookAndFeel();
    if (m_autoCheck->isChecked() != isAuto) {
         bool blocked = m_autoCheck->blockSignals(true);
         m_autoCheck->setChecked(isAuto);
         m_autoCheck->blockSignals(blocked);
    }
}

// --- Menu Slots ---

void MainWindow::openGlobalThemesFolder() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/look-and-feel";
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openKvantumThemesFolder() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Kvantum";
    if (!QDir(path).exists()) {
       path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Kvantum";
    }
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openGtkThemesFolder() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/themes";
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openAppConfigFolder() {
    // Open the folder containing the config file.
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::clearConfig() {
    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Clear Config"),
        tr("Are you sure you want to reset all settings? This cannot be undone."),
        QMessageBox::Yes | QMessageBox::No);
        
    if (res == QMessageBox::Yes) {
        QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/plasma-theme-masterrc";
        if (QFile::exists(path)) {
             if (QFile::remove(path)) {
                  QMessageBox::information(this, tr("Success"), tr("Configuration cleared. Please restart the app."));
                  close();
             } else {
                  QMessageBox::warning(this, tr("Error"), tr("Failed to delete config file."));
             }
        } else {
             QMessageBox::information(this, tr("Info"), tr("No configuration file found."));
        }
    }
}

void MainWindow::triggerUninstall() {
    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Uninstall"),
        tr("This will launch the uninstaller script. All application data might be removed.\nContinue?"),
        QMessageBox::Yes | QMessageBox::No);
        
    if (res == QMessageBox::Yes) {
        QString program = "konsole";
        QStringList arguments;
        arguments << "-e" << "plasma-theme-master-uninstall";
        
        if (!QProcess::startDetached(program, arguments)) {
             // Fallback
             program = "x-terminal-emulator";
             arguments.clear();
             arguments << "-e" << "plasma-theme-master-uninstall";
             if (!QProcess::startDetached(program, arguments)) {
                 QMessageBox::warning(this, tr("Error"), tr("Could not launch terminal. Please run 'plasma-theme-master-uninstall' manually."));
             } else {
                 close();
             }
        } else {
             close();
        }
    }
}
