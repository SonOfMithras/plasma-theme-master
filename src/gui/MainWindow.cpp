
#include "MainWindow.h"
#include "../core/Config.h"
#include "../core/Solar.h"
#include "../core/ThemeReader.h"
#include "../core/ThemeWriter.h"
#include "../core/UniversalThemeExporter.h"
#include "GlobalThemeEditor.h"
#include "UniversalThemePage.h"
#include "ColorSwatchPicker.h"
#include "core/Logger.h"
#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include "../core/FlatpakManager.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupUi();
  populateThemes();
  loadSettings();

  checkDaemonStatus();
  refreshStatus();

  if (FlatpakManager::isFlatpakInstalled()) {
    Logger::log("Flatpak status: " + FlatpakManager::flatpakStatus(),
                Logger::Info);
  }

  updateLogs();

  m_logTimer = new QTimer(this);
  connect(m_logTimer, &QTimer::timeout, this, &MainWindow::updateLogs);
  m_logTimer->start(5000);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  m_mainTabs = new QTabWidget(this);
  setCentralWidget(m_mainTabs);

  setupDashboardTab();
  setupLogsTab();
  setupMaterialYouTab();

  m_mainTabs->addTab(m_dashboardTab, tr("Dashboard"));

  // Global Theme Editor Tab
  m_globalEditorTab = new GlobalThemeEditor(this);
  m_mainTabs->addTab(m_globalEditorTab, tr("Global Theme Editor"));

  // Universal Theme Tab
  m_universalTab = new UniversalThemePage(this);
  m_mainTabs->addTab(m_universalTab, tr("Universal Sync"));

  // Material You Tab
  m_mainTabs->addTab(m_materialYouTab, tr("Material You"));

  // Logs Tab
  m_mainTabs->addTab(m_logsTab, tr("Logs"));

  // Menu Bar
  setupMenuBar();

  setWindowTitle(tr("Plasma Theme Master"));
  resize(650, 650);
}

void MainWindow::setupMenuBar() {
  QMenuBar *menuBar = new QMenuBar(this);
  setMenuBar(menuBar); // QMainWindow takes ownership

  // --- File Menu ---
  QMenu *fileMenu = menuBar->addMenu(tr("&File"));

  QAction *openGlobalAction =
      new QAction(tr("Open &Global Themes Folder"), this);
  connect(openGlobalAction, &QAction::triggered, this,
          &MainWindow::openGlobalThemesFolder);
  fileMenu->addAction(openGlobalAction);

  QAction *openKvantumAction =
      new QAction(tr("Open &Kvantum Themes Folder"), this);
  connect(openKvantumAction, &QAction::triggered, this,
          &MainWindow::openKvantumThemesFolder);
  fileMenu->addAction(openKvantumAction);

  QAction *openGtkAction = new QAction(tr("Open G&TK Themes Folder"), this);
  connect(openGtkAction, &QAction::triggered, this,
          &MainWindow::openGtkThemesFolder);
  fileMenu->addAction(openGtkAction);

  fileMenu->addSeparator();

  QAction *openConfigAction = new QAction(tr("Open &App Config Folder"), this);
  connect(openConfigAction, &QAction::triggered, this,
          &MainWindow::openAppConfigFolder);
  fileMenu->addAction(openConfigAction);

  fileMenu->addSeparator();

  QAction *quitAction = new QAction(tr("&Quit"), this);
  quitAction->setShortcut(QKeySequence::Quit);
  connect(quitAction, &QAction::triggered, this, &MainWindow::close);
  fileMenu->addAction(quitAction);

  // --- Help Menu ---
  QMenu *helpMenu = menuBar->addMenu(tr("&Help"));

  QMenu *materialMenu = helpMenu->addMenu(tr("Material You Colors"));
  QAction *installAction = new QAction(tr("Install dependencies (pipx)"), this);
  connect(installAction, &QAction::triggered, this,
          &MainWindow::installMaterialYou);
  materialMenu->addAction(installAction);

  QAction *upgradeAction = new QAction(tr("Upgrade"), this);
  connect(upgradeAction, &QAction::triggered, this,
          &MainWindow::upgradeMaterialYou);
  materialMenu->addAction(upgradeAction);

  m_myAutostartAction = new QAction(tr("Autostart on Login"), this);
  m_myAutostartAction->setCheckable(true);

  // Check if autostart file exists
  QString autostartPath =
      QDir::homePath() + "/.config/autostart/kde-material-you-colors.desktop";
  m_myAutostartAction->setChecked(QFile::exists(autostartPath));

  connect(m_myAutostartAction, &QAction::toggled, this,
          &MainWindow::toggleMaterialYouAutostart);
  materialMenu->addAction(m_myAutostartAction);

  helpMenu->addSeparator();

  QAction *flatpakAction = new QAction(tr("&Flatpak Settings..."), this);
  connect(flatpakAction, &QAction::triggered, this,
          &MainWindow::showFlatpakSettings);
  helpMenu->addAction(flatpakAction);

  m_daemonAction = new QAction(tr("Enable Background Service"), this);
  m_daemonAction->setCheckable(true);
  connect(m_daemonAction, &QAction::toggled, this, &MainWindow::toggleDaemon);
  helpMenu->addAction(m_daemonAction);

  helpMenu->addSeparator();

  QAction *clearLogAction = new QAction(tr("Clear &Log"), this);
  connect(clearLogAction, &QAction::triggered, this, &MainWindow::clearLogs);
  helpMenu->addAction(clearLogAction);

  QAction *clearConfigAction = new QAction(tr("Clear &Config"), this);
  connect(clearConfigAction, &QAction::triggered, this,
          &MainWindow::clearConfig);
  helpMenu->addAction(clearConfigAction);

  helpMenu->addSeparator();

  QAction *refreshPlasmaAction = new QAction(tr("Restart &Plasma Shell"), this);
  connect(refreshPlasmaAction, &QAction::triggered, this,
          &MainWindow::refreshPlasma);
  helpMenu->addAction(refreshPlasmaAction);

  helpMenu->addSeparator();

  QAction *uninstallAction = new QAction(tr("&Uninstall App"), this);
  connect(uninstallAction, &QAction::triggered, this,
          &MainWindow::triggerUninstall);
  helpMenu->addAction(uninstallAction);

  helpMenu->addSeparator();

  QAction *aboutAction = new QAction(tr("&About"), this);
  connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
  helpMenu->addAction(aboutAction);
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

void MainWindow::showAbout() {
  QString version = QString::fromLatin1(TOSTRING(PROJECT_VERSION));
  QString repoUrl =
      "https://github.com/SonOfMithras/plasma-theme-master/releases";
  QString supportUrl = "https://ko-fi.com/sonofmithras";

  QDialog aboutDialog(this);
  aboutDialog.setWindowTitle(tr("About Plasma Theme Master"));
  aboutDialog.resize(400, 300);
  QVBoxLayout *mainLayout = new QVBoxLayout(&aboutDialog);
  QLabel *infoLabel = new QLabel(&aboutDialog);
  infoLabel->setTextFormat(Qt::RichText);
  infoLabel->setWordWrap(true);
  infoLabel->setAlignment(Qt::AlignCenter);
  infoLabel->setText(
      tr("<h2>Plasma Theme Master</h2>"
         "<h3>Version %1</h3>"
         "<p><b>Author:</b> Ammar Al-Riyamy</p>"
         "<p><a "
         "href='https://github.com/SonOfMithras/plasma-theme-master'>GitHub "
         "Repo</a></p>"
         "<br>"
         "<p>Automatic Day/Night Theme Switcher for KDE Plasma.</p>"
         "<br>"
         "<p>Features:</p>"
         "<ul>"
         "<li>Solar calculation to sync themes</li>"
         "<li>Global Theme Switching</li>"
         "<li>Kvantum, GTK & Flatpak Settings & Sync</li>"
         "<li>Global Theme Editor, Backup & Restore</li>"
         "</ul>"
         "<p>Redesigned in C++ for efficiency.</p>")
          .arg(version));
  infoLabel->setOpenExternalLinks(true);

  mainLayout->addWidget(infoLabel);
  mainLayout->addSpacing(10);

  // Custom Action Buttons
  QHBoxLayout *actionLayout = new QHBoxLayout();
  QPushButton *updateBtn =
      new QPushButton(tr("Check for Updates"), &aboutDialog);
  QPushButton *supportBtn =
      new QPushButton(tr("Support the Dev!"), &aboutDialog);

  actionLayout->addStretch();
  actionLayout->addWidget(updateBtn);
  actionLayout->addWidget(supportBtn);
  actionLayout->addStretch();

  mainLayout->addLayout(actionLayout);

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok, &aboutDialog);
  buttonBox->setCenterButtons(true);
  connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog,
          &QDialog::accept);

  connect(updateBtn, &QPushButton::clicked,
          [repoUrl]() { QDesktopServices::openUrl(QUrl(repoUrl)); });
  connect(supportBtn, &QPushButton::clicked,
          [supportUrl]() { QDesktopServices::openUrl(QUrl(supportUrl)); });

  mainLayout->addWidget(buttonBox);

  aboutDialog.exec();
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
  QVBoxLayout *statusMainLayout = new QVBoxLayout(statusGroup);

  QLabel *statusHeader =
      new QLabel("<h2><b>System Status</b></h2>", dashboardContent);
  statusMainLayout->addWidget(statusHeader);

  QHBoxLayout *statusColumnsLayout = new QHBoxLayout();
  m_statusLabel = new QLabel(dashboardContent);
  m_statusLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_sunInfoLabel = new QLabel(dashboardContent);
  m_sunInfoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  statusColumnsLayout->addWidget(m_statusLabel);
  statusColumnsLayout->addWidget(m_sunInfoLabel);

  statusMainLayout->addLayout(statusColumnsLayout);

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

  m_kcmNightColorBtn = new QPushButton(tr("Configure Day-Night Cycle"), this);
  solarLayout->addWidget(m_kcmNightColorBtn);

  connect(m_offsetSlider, &QSlider::valueChanged, this,
          &MainWindow::onOffsetChanged);
  connect(m_offsetSlider, &QSlider::sliderReleased, this,
          &MainWindow::saveSettings);

  connect(m_kcmNightColorBtn, &QPushButton::clicked, this,
          []() { QProcess::startDetached("kcmshell6", {"kcm_nighttime"}); });

  layout->addWidget(solarGroup);

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

  // Klassy
  QLabel *klassyLabel = new QLabel(tr("<b>Klassy Preset</b>"), this);
  configLayout->addWidget(klassyLabel);
  QFormLayout *klassyLayout = new QFormLayout();
  m_klassyDayCombo = new QComboBox(this);
  m_klassyNightCombo = new QComboBox(this);
  klassyLayout->addRow(tr("Day:"), m_klassyDayCombo);
  klassyLayout->addRow(tr("Night:"), m_klassyNightCombo);
  configLayout->addLayout(klassyLayout);

  connect(m_globalDayCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_globalNightCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_kvantumDayCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_kvantumNightCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_gtkDayCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_gtkNightCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_klassyDayCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);
  connect(m_klassyNightCombo, &QComboBox::activated, this,
          &MainWindow::saveSettings);

  layout->addWidget(configGroup);

  // --- Automation Control ---
  QGroupBox *autoGroup = new QGroupBox(tr("Automation Control"), this);
  QVBoxLayout *autoMainLayout = new QVBoxLayout(autoGroup);

  QHBoxLayout *autoLayout = new QHBoxLayout();
  m_autoCheck = new QCheckBox(tr("Enable Auto-Switch"), this);
  connect(m_autoCheck, &QCheckBox::toggled, this, &MainWindow::toggleAuto);

  m_refreshButton = new QPushButton(tr("Refresh"), this);
  connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
    populateThemes();
    loadSettings();
    refreshStatus();
  });

  autoLayout->addWidget(m_autoCheck);
  autoLayout->addStretch();
  autoLayout->addWidget(m_refreshButton);
  autoMainLayout->addLayout(autoLayout);

  m_materialYouCheck =
      new QCheckBox(tr("Override color scheme with Material You"), this);
  connect(m_materialYouCheck, &QCheckBox::toggled, this, [this](bool checked) {
    if (checked)
      promptMaterialYouInstall();
    saveSettings();
  });
  autoMainLayout->addWidget(m_materialYouCheck);

  layout->addWidget(autoGroup);

  // --- Night Color Settings ---
  QGroupBox *ncGroup = new QGroupBox(tr("Night Color Settings"), this);
  QVBoxLayout *ncLayout = new QVBoxLayout(ncGroup);

  QHBoxLayout *tempLayout = new QHBoxLayout();

  // Day
  QVBoxLayout *dayTempLayout = new QVBoxLayout();
  dayTempLayout->addWidget(new QLabel(tr("Day Temp (K):")));
  m_dayTempSlider = new QSlider(Qt::Horizontal, this);
  m_dayTempSlider->setRange(1000, 6500);
  m_dayTempSpinBox = new QSpinBox(this);
  m_dayTempSpinBox->setRange(1000, 6500);
  dayTempLayout->addWidget(m_dayTempSlider);
  dayTempLayout->addWidget(m_dayTempSpinBox);
  tempLayout->addLayout(dayTempLayout);

  // Night
  QVBoxLayout *nightTempLayout = new QVBoxLayout();
  nightTempLayout->addWidget(new QLabel(tr("Night Temp (K):")));
  m_nightTempSlider = new QSlider(Qt::Horizontal, this);
  m_nightTempSlider->setRange(1000, 6500);
  m_nightTempSpinBox = new QSpinBox(this);
  m_nightTempSpinBox->setRange(1000, 6500);
  nightTempLayout->addWidget(m_nightTempSlider);
  nightTempLayout->addWidget(m_nightTempSpinBox);
  tempLayout->addLayout(nightTempLayout);

  ncLayout->addLayout(tempLayout);

  connect(m_dayTempSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
          m_dayTempSlider, &QSlider::setValue);
  connect(m_dayTempSlider, &QSlider::valueChanged, m_dayTempSpinBox,
          &QSpinBox::setValue);
  connect(m_nightTempSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
          m_nightTempSlider, &QSlider::setValue);
  connect(m_nightTempSlider, &QSlider::valueChanged, m_nightTempSpinBox,
          &QSpinBox::setValue);

  connect(m_dayTempSlider, &QSlider::sliderReleased, this,
          &MainWindow::saveSettings);
  connect(m_nightTempSlider, &QSlider::sliderReleased, this,
          &MainWindow::saveSettings);
  connect(m_dayTempSpinBox, &QSpinBox::editingFinished, this,
          &MainWindow::saveSettings);
  connect(m_nightTempSpinBox, &QSpinBox::editingFinished, this,
          &MainWindow::saveSettings);

  layout->addWidget(ncGroup);

  // --- Manual Overrides ---
  QGroupBox *manualGroup = new QGroupBox(tr("Manual Override"), this);
  QHBoxLayout *manualLayout = new QHBoxLayout(manualGroup);
  m_applyDayBtn = new QPushButton(tr("Force Day Mode"), this);
  m_applyNightBtn = new QPushButton(tr("Force Night Mode"), this);

  connect(m_applyDayBtn, &QPushButton::clicked, this,
          &MainWindow::applyStaticDay);
  connect(m_applyNightBtn, &QPushButton::clicked, this,
          &MainWindow::applyStaticNight);

  m_applyTargetBtn = new QPushButton(tr("Apply Target"), this);
  connect(m_applyTargetBtn, &QPushButton::clicked, this,
          &MainWindow::applyCurrentTarget);

  manualLayout->addWidget(m_applyDayBtn);
  manualLayout->addWidget(m_applyTargetBtn);
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

void MainWindow::setupMaterialYouTab() {
  m_materialYouTab = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(m_materialYouTab);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(15);

  QLabel *header =
      new QLabel("<h2><b>Material You Settings</b></h2>", m_materialYouTab);
  layout->addWidget(header);

  // Scheme Variant
  QGroupBox *variantGroup = new QGroupBox(tr("Color Scheme"), m_materialYouTab);
  QHBoxLayout *variantLayout = new QHBoxLayout(variantGroup);
  m_schemeVariantCombo = new QComboBox(m_materialYouTab);
  m_schemeVariantCombo->addItems({"Content", "Expressive", "Fidelity",
                                  "Monochrome", "Neutral", "TonalSpot",
                                  "Vibrant", "Rainbow", "FruitSalad"});
  variantLayout->addWidget(m_schemeVariantCombo);
  layout->addWidget(variantGroup);

  // Chroma (Colorfulness)
  QGroupBox *chromaGroup =
      new QGroupBox(tr("Colorfulness (Chroma Multiplier)"), m_materialYouTab);
  QHBoxLayout *chromaLayout = new QHBoxLayout(chromaGroup);

  m_chromaSlider = new QSlider(Qt::Horizontal, m_materialYouTab);
  m_chromaSlider->setRange(5, 100); // 0.5 to 10.0 (multiplier x 10)
  m_chromaSpinBox = new QDoubleSpinBox(m_materialYouTab);
  m_chromaSpinBox->setRange(0.5, 10.0);
  m_chromaSpinBox->setSingleStep(0.1);
  m_chromaSpinBox->setDecimals(1);

  chromaLayout->addWidget(m_chromaSlider);
  chromaLayout->addWidget(m_chromaSpinBox);
  layout->addWidget(chromaGroup);

  // Tone (Brightness)
  QGroupBox *toneGroup =
      new QGroupBox(tr("Brightness (Tone Multiplier)"), m_materialYouTab);
  QHBoxLayout *toneLayout = new QHBoxLayout(toneGroup);

  m_toneSlider = new QSlider(Qt::Horizontal, m_materialYouTab);
  m_toneSlider->setRange(0, 15); // 0.0 to 1.5 (multiplier x 10)
  m_toneSpinBox = new QDoubleSpinBox(m_materialYouTab);
  m_toneSpinBox->setRange(0.0, 1.5);
  m_toneSpinBox->setSingleStep(0.1);
  m_toneSpinBox->setDecimals(1);

  toneLayout->addWidget(m_toneSlider);
  toneLayout->addWidget(m_toneSpinBox);
  layout->addWidget(toneGroup);

  // Dominant Seed Color (ncolor)
  QGroupBox *seedGroup =
      new QGroupBox(tr("Dominant Seed Color"), m_materialYouTab);
  QVBoxLayout *seedLayout = new QVBoxLayout(seedGroup);

  QLabel *seedHint = new QLabel(
      tr("Colors extracted from your current wallpaper. "
         "Click a swatch to use it as the scheme seed."),
      m_materialYouTab);
  seedHint->setWordWrap(true);
  seedHint->setEnabled(false); // muted appearance
  seedLayout->addWidget(seedHint);

  QHBoxLayout *swatchRowLayout = new QHBoxLayout();
  m_nColorPicker = new ColorSwatchPicker(m_materialYouTab);
  m_nColorPicker->refreshFromDaemon();
  swatchRowLayout->addWidget(m_nColorPicker);
  swatchRowLayout->addStretch();

  m_refreshSwatchesBtn = new QPushButton(tr("Refresh Colors"), m_materialYouTab);
  m_refreshSwatchesBtn->setToolTip(tr("Re-read wallpaper colors from the running kde-material-you-colors daemon"));
  swatchRowLayout->addWidget(m_refreshSwatchesBtn);
  seedLayout->addLayout(swatchRowLayout);
  layout->addWidget(seedGroup);


  // Apply Button
  QHBoxLayout *btnLayout = new QHBoxLayout();
  m_myApplyBtn =
      new QPushButton(tr("Apply Variables to Daemon"), m_materialYouTab);
  m_myApplyBtn->setEnabled(false);
  btnLayout->addStretch();
  btnLayout->addWidget(m_myApplyBtn);
  layout->addLayout(btnLayout);

  layout->addStretch();

  // Connections
  connect(m_chromaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, [this](double val) {
            if (!m_chromaSlider->signalsBlocked()) {
              m_chromaSlider->blockSignals(true);
              m_chromaSlider->setValue(val * 10);
              m_chromaSlider->blockSignals(false);
            }
            onMaterialYouSettingsChanged();
          });

  connect(m_chromaSlider, &QSlider::valueChanged, this, [this](int val) {
    if (!m_chromaSpinBox->signalsBlocked()) {
      m_chromaSpinBox->blockSignals(true);
      m_chromaSpinBox->setValue(val / 10.0);
      m_chromaSpinBox->blockSignals(false);
    }
    onMaterialYouSettingsChanged();
  });

  connect(m_toneSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, [this](double val) {
            if (!m_toneSlider->signalsBlocked()) {
              m_toneSlider->blockSignals(true);
              m_toneSlider->setValue(val * 10);
              m_toneSlider->blockSignals(false);
            }
            onMaterialYouSettingsChanged();
          });

  connect(m_toneSlider, &QSlider::valueChanged, this, [this](int val) {
    if (!m_toneSpinBox->signalsBlocked()) {
      m_toneSpinBox->blockSignals(true);
      m_toneSpinBox->setValue(val / 10.0);
      m_toneSpinBox->blockSignals(false);
    }
    onMaterialYouSettingsChanged();
  });

  connect(m_schemeVariantCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::onMaterialYouSettingsChanged);

  connect(m_nColorPicker, &ColorSwatchPicker::colorSelected, this,
          &MainWindow::onMaterialYouSettingsChanged);

  connect(m_refreshSwatchesBtn, &QPushButton::clicked, this, [this]() {
    int savedIndex = m_nColorPicker->selectedIndex();
    m_nColorPicker->refreshFromDaemon();
    // Restore selection if still in range, otherwise resets to 0
    m_nColorPicker->setSelectedIndex(savedIndex);
  });

  connect(m_myApplyBtn, &QPushButton::clicked, this,
          &MainWindow::applyMaterialYouSettings);
}

void MainWindow::onMaterialYouSettingsChanged() {
  double savedChroma = Config::materialYouChroma();
  double savedTone = Config::materialYouTone();
  int savedVariant = Config::materialYouSchemeVariant();

  bool changed = (qAbs(m_chromaSpinBox->value() - savedChroma) > 0.01) ||
                 (qAbs(m_toneSpinBox->value() - savedTone) > 0.01) ||
                 (m_schemeVariantCombo->currentIndex() != savedVariant) ||
                 (m_nColorPicker->selectedIndex() != Config::materialYouNColor());

  m_myApplyBtn->setEnabled(changed);
}

void MainWindow::applyMaterialYouSettings() {
  Config::setMaterialYouChroma(m_chromaSpinBox->value());
  Config::setMaterialYouTone(m_toneSpinBox->value());
  Config::setMaterialYouSchemeVariant(m_schemeVariantCombo->currentIndex());
  Config::setMaterialYouNColor(m_nColorPicker->selectedIndex());

  m_myApplyBtn->setEnabled(false);

  if (Config::isMaterialYouOverrideEnabled()) {
    QProcess::execute("killall", QStringList() << "kde-material-you-colors");
    ThemeWriter::syncMaterialYouIcons(true);
  }
}

void MainWindow::updateLogs() {
  if (m_mainTabs->currentWidget() != m_logsTab)
    return;

  QStringList logs = Logger::readLogs(200, false);
  QString reversedContent;
  for (int i = logs.size() - 1; i >= 0; --i) {
    reversedContent += logs[i] + "\n";
  }

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

  m_globalDayCombo->clear();
  m_globalNightCombo->clear();
  m_kvantumDayCombo->clear();
  m_kvantumNightCombo->clear();
  m_gtkDayCombo->clear();
  m_gtkNightCombo->clear();

  m_globalDayCombo->addItems(globalThemes);
  m_globalNightCombo->addItems(globalThemes);
  m_kvantumDayCombo->addItems(kvantumThemes);
  m_kvantumNightCombo->addItems(kvantumThemes);

  QStringList gtkThemes = ThemeReader::listGtkThemes();
  m_gtkDayCombo->addItems(gtkThemes);
  m_gtkNightCombo->addItems(gtkThemes);

  QStringList klassyPresets = ThemeReader::listKlassyPresets();
  m_klassyDayCombo->addItems(klassyPresets);
  m_klassyNightCombo->addItems(klassyPresets);
}

void MainWindow::loadSettings() {
  bool bA = m_autoCheck->blockSignals(true);
  bool bM = m_materialYouCheck->blockSignals(true);

  m_globalDayCombo->setCurrentText(ThemeReader::defaultLightTheme());
  m_globalNightCombo->setCurrentText(ThemeReader::defaultDarkTheme());

  // Kvantum Defaults
  QString kDay = ThemeReader::dayKvantumTheme();
  QString kNight = ThemeReader::nightKvantumTheme();
  if (!kDay.isEmpty())
    m_kvantumDayCombo->setCurrentText(kDay);
  if (!kNight.isEmpty())
    m_kvantumNightCombo->setCurrentText(kNight);

  // GTK Defaults
  QString gDay = ThemeReader::dayGtkTheme();
  QString gNight = ThemeReader::nightGtkTheme();
  if (!gDay.isEmpty())
    m_gtkDayCombo->setCurrentText(gDay);
  if (!gNight.isEmpty())
    m_gtkNightCombo->setCurrentText(gNight);

  // Klassy Defaults
  QString kDayP = ThemeReader::dayKlassyPreset();
  QString kNightP = ThemeReader::nightKlassyPreset();
  if (!kDayP.isEmpty())
    m_klassyDayCombo->setCurrentText(kDayP);
  if (!kNightP.isEmpty())
    m_klassyNightCombo->setCurrentText(kNightP);

  // Solar Padding
  int padding = ThemeReader::solarPadding();
  m_offsetSlider->setValue(padding);
  m_offsetValueLabel->setText(QString("%1 mins").arg(padding));

  // Auto Switch
  m_autoCheck->setChecked(ThemeReader::isAutoLookAndFeel());
  m_materialYouCheck->setChecked(Config::isMaterialYouOverrideEnabled());

  // Night Color Temps
  bool bDayS = m_dayTempSlider->blockSignals(true);
  bool bDayB = m_dayTempSpinBox->blockSignals(true);
  bool bNitS = m_nightTempSlider->blockSignals(true);
  bool bNitB = m_nightTempSpinBox->blockSignals(true);

  int dTemp = ThemeReader::kwinDayTemperature();
  int nTemp = ThemeReader::kwinNightTemperature();

  m_dayTempSlider->setValue(dTemp);
  m_dayTempSpinBox->setValue(dTemp);
  m_nightTempSlider->setValue(nTemp);
  m_nightTempSpinBox->setValue(nTemp);

  m_dayTempSlider->blockSignals(bDayS);
  m_dayTempSpinBox->blockSignals(bDayB);
  m_nightTempSlider->blockSignals(bNitS);
  m_nightTempSpinBox->blockSignals(bNitB);

  // Material You Sliders/SpinBoxes/ComboBox
  bool bC = m_chromaSpinBox->blockSignals(true);
  bool bT = m_toneSpinBox->blockSignals(true);
  bool bV = m_schemeVariantCombo->blockSignals(true);
  double savedChroma = Config::materialYouChroma();
  double savedTone = Config::materialYouTone();
  int savedVariant = Config::materialYouSchemeVariant();

  m_chromaSpinBox->setValue(savedChroma);
  m_chromaSlider->setValue(qRound(savedChroma * 10));

  m_toneSpinBox->setValue(savedTone);
  m_toneSlider->setValue(qRound(savedTone * 10));

  if (savedVariant >= 0 && savedVariant < m_schemeVariantCombo->count()) {
    m_schemeVariantCombo->setCurrentIndex(savedVariant);
  }

  // Refresh swatches from daemon and restore saved ncolor index
  m_nColorPicker->refreshFromDaemon();
  m_nColorPicker->setSelectedIndex(Config::materialYouNColor());

  m_chromaSpinBox->blockSignals(bC);
  m_toneSpinBox->blockSignals(bT);
  m_schemeVariantCombo->blockSignals(bV);

  m_myApplyBtn->setEnabled(false);

  m_autoCheck->blockSignals(bA);
  m_materialYouCheck->blockSignals(bM);
}

void MainWindow::saveSettings() {
  ThemeWriter::setDefaultLightTheme(m_globalDayCombo->currentText());
  ThemeWriter::setDefaultDarkTheme(m_globalNightCombo->currentText());
  ThemeWriter::setDayKvantumTheme(m_kvantumDayCombo->currentText());
  ThemeWriter::setNightKvantumTheme(m_kvantumNightCombo->currentText());
  ThemeWriter::setDayGtkTheme(m_gtkDayCombo->currentText());
  ThemeWriter::setNightGtkTheme(m_gtkNightCombo->currentText());
  ThemeWriter::setDayKlassyPreset(m_klassyDayCombo->currentText());
  ThemeWriter::setNightKlassyPreset(m_klassyNightCombo->currentText());
  ThemeWriter::setSolarPadding(m_offsetSlider->value());
  Config::setMaterialYouOverrideEnabled(m_materialYouCheck->isChecked());
  if (m_materialYouCheck->isChecked()) {
    QProcess::execute("killall", QStringList() << "kde-material-you-colors");
    ThemeWriter::syncMaterialYouIcons(true);
  } else {
    QProcess::execute("killall", QStringList() << "kde-material-you-colors");
  }

  ThemeWriter::setKWinTemperatures(m_dayTempSlider->value(),
                                   m_nightTempSlider->value());

  // If Auto is enabled, re-apply logic immediately to reflect changes
  if (m_autoCheck->isChecked()) {
    toggleAuto(true);
  }

  refreshStatus();
}

void MainWindow::toggleAuto(bool checked) {
  if (checked) {
    auto applyAction = [this]() {
      bool isDay = ThemeReader::isKWinDaytime();

      QString global = isDay ? m_globalDayCombo->currentText()
                             : m_globalNightCombo->currentText();
      QString kvantum = isDay ? m_kvantumDayCombo->currentText()
                              : m_kvantumNightCombo->currentText();

      if (!global.isEmpty()) {
        ThemeWriter::applyGlobalTheme(global, true);
        if (Config::isMaterialYouOverrideEnabled()) {
          ThemeWriter::applyColorScheme(
              isDay ? "MaterialYouLight" : "MaterialYouDark", true);
        }
      }
      if (!kvantum.isEmpty())
        ThemeWriter::setKvantumTheme(kvantum, true);

      QString gtk =
          isDay ? m_gtkDayCombo->currentText() : m_gtkNightCombo->currentText();
      if (!gtk.isEmpty())
        ThemeWriter::setGtkTheme(gtk, true);

      QString klassy = isDay ? m_klassyDayCombo->currentText()
                             : m_klassyNightCombo->currentText();
      if (!klassy.isEmpty())
        ThemeWriter::setKlassyPreset(klassy, true);

      ThemeWriter::enforceKWinNightColorActive();
      ThemeWriter::setAutoLookAndFeel(true);
    };

    applyAction();

    // Double-pass: Solid delay to let first pass settle, then apply again to
    // fix contrast issues
    QTimer::singleShot(2000, this, [this, applyAction]() {
      applyAction();
      if (m_autoCheck->isChecked()) {
        UniversalThemeExporter::syncAll();
      }
    });

  } else {
    ThemeWriter::setAutoLookAndFeel(false);
  }
  refreshStatus();
}

#include <QTimer>

void MainWindow::applyStaticDay() {
  m_autoCheck->setChecked(false); // Disables auto

  auto applyAction = [this]() {
    ThemeWriter::applyGlobalTheme(m_globalDayCombo->currentText(), true);
    if (Config::isMaterialYouOverrideEnabled()) {
      ThemeWriter::applyColorScheme("MaterialYouLight", true);
    }
    ThemeWriter::setKvantumTheme(m_kvantumDayCombo->currentText(), true);
    ThemeWriter::setGtkTheme(m_gtkDayCombo->currentText(), true);
    ThemeWriter::setKlassyPreset(m_klassyDayCombo->currentText(), true);
  };

  applyAction();

  // Double-pass: Solid delay to let first pass settle, then apply again to fix
  // contrast issues
  QTimer::singleShot(2000, this, [this, applyAction]() {
    applyAction();
    if (!m_autoCheck->isChecked()) {
      UniversalThemeExporter::syncAll();
    }
  });

  refreshStatus();
}

void MainWindow::applyStaticNight() {
  m_autoCheck->setChecked(false); // Disables auto

  auto applyAction = [this]() {
    ThemeWriter::applyGlobalTheme(m_globalNightCombo->currentText(), true);
    if (Config::isMaterialYouOverrideEnabled()) {
      ThemeWriter::applyColorScheme("MaterialYouDark", true);
    }
    ThemeWriter::setKvantumTheme(m_kvantumNightCombo->currentText(), true);
    ThemeWriter::setGtkTheme(m_gtkNightCombo->currentText(), true);
    ThemeWriter::setKlassyPreset(m_klassyNightCombo->currentText(), true);
  };

  applyAction();

  // Double-pass: Solid delay to let first pass settle, then apply again to fix
  // contrast issues
  QTimer::singleShot(2000, this, [this, applyAction]() {
    applyAction();
    if (!m_autoCheck->isChecked()) {
      UniversalThemeExporter::syncAll();
    }
  });

  refreshStatus();
}

void MainWindow::applyCurrentTarget() {
  bool wasAuto = m_autoCheck->isChecked(); // Capture current auto state

  auto applyAction = [this]() {
    bool isDay = ThemeReader::isKWinDaytime();

    // Apply target themes
    if (isDay) {
      ThemeWriter::applyGlobalTheme(m_globalDayCombo->currentText(), true);
      if (Config::isMaterialYouOverrideEnabled()) {
        ThemeWriter::applyColorScheme("MaterialYouLight", true);
      }
      ThemeWriter::setKvantumTheme(m_kvantumDayCombo->currentText(), true);
      ThemeWriter::setGtkTheme(m_gtkDayCombo->currentText(), true);
      ThemeWriter::setKlassyPreset(m_klassyDayCombo->currentText(), true);
    } else {
      ThemeWriter::applyGlobalTheme(m_globalNightCombo->currentText(), true);
      if (Config::isMaterialYouOverrideEnabled()) {
        ThemeWriter::applyColorScheme("MaterialYouDark", true);
      }
      ThemeWriter::setKvantumTheme(m_kvantumNightCombo->currentText(), true);
      ThemeWriter::setGtkTheme(m_gtkNightCombo->currentText(), true);
      ThemeWriter::setKlassyPreset(m_klassyNightCombo->currentText(), true);
    }
  };

  applyAction();

  // Restore auto state if it was enabled (as applyGlobalTheme might have
  // disabled it)
  if (wasAuto) {
    ThemeWriter::setAutoLookAndFeel(true);
  }

  // Double-pass: Solid delay to let first pass settle, then apply again to fix
  // contrast issues
  QTimer::singleShot(2000, this, [this, applyAction, wasAuto]() {
    applyAction();
    if (wasAuto) {
      ThemeWriter::setAutoLookAndFeel(true);
    }
    UniversalThemeExporter::syncAll();
  });

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

  m_statusLabel->setText(tr("<b>Global:</b> %1<br><b>Kvantum:</b> "
                            "%2<br><b>GTK:</b> %3<br><b>Auto:</b> %4")
                             .arg(global, kvantum, gtk, autoLabel));

  // Solar Info
  double lat = ThemeReader::nativeLatitude();
  double lon = ThemeReader::nativeLongitude();
  int offset = m_offsetSlider->value();

  bool isDay = ThemeReader::isKWinDaytime();
  QString target = isDay ? "Day" : "Night";

  m_statusLabel->setText(
      tr("<b>Global:</b> %1<br><b>Kvantum:</b> %2<br><b>GTK:</b> "
         "%3<br><b>Auto:</b> %4<br><br><b>KWin State:</b> %5")
          .arg(global, kvantum, gtk, autoLabel, target));

  QPair<QDateTime, QDateTime> times =
      Solar::calculateSunTimes(lat, lon, QDate::currentDate());
  QString sr = times.first.isValid()
                   ? times.first.toLocalTime().toString("HH:mm")
                   : "N/A";
  QString ss = times.second.isValid()
                   ? times.second.toLocalTime().toString("HH:mm")
                   : "N/A";

  // Calculated
  int shift = (offset * 60) / 2;
  QDateTime start =
      times.first.isValid() ? times.first.addSecs(-shift) : QDateTime();
  QDateTime end =
      times.second.isValid() ? times.second.addSecs(shift) : QDateTime();

  QString startStr =
      start.isValid() ? start.toLocalTime().toString("HH:mm") : "N/A";
  QString endStr = end.isValid() ? end.toLocalTime().toString("HH:mm") : "N/A";

  int dayTemp = ThemeReader::kwinDayTemperature();
  int nightTemp = ThemeReader::kwinNightTemperature();

  m_sunInfoLabel->setText(
      tr("<b><u>KWin Night Color</u></b><br>"
         "<b>Day Temp:</b> %1 K<br><b>Night Temp:</b> %2 K<br><br>"
         "<b><u>Solar Settings</u></b><br>"
         "<b>Location:</b> %3, %4<br>"
         "<b>Sunrise:</b> %5 &nbsp;&nbsp;<b>Sunset:</b> %6<br>"
         "<b>Day Start:</b> %7 &nbsp;&nbsp;<b>Night Start:</b> %8")
          .arg(dayTemp)
          .arg(nightTemp)
          .arg(lat)
          .arg(lon)
          .arg(sr, ss, startStr, endStr));

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
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
      "/plasma/look-and-feel";
  QDir().mkpath(path);
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openKvantumThemesFolder() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
      "/Kvantum";
  if (!QDir(path).exists()) {
    path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
           "/Kvantum";
  }
  QDir().mkpath(path);
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openGtkThemesFolder() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
      "/themes";
  QDir().mkpath(path);
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openAppConfigFolder() {
  // Open the folder containing the config file.
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/plasma-theme-master";
  QDir().mkpath(path); // Ensure it exists
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::clearConfig() {
  QMessageBox::StandardButton res = QMessageBox::warning(
      this, tr("Clear Config"),
      tr("Are you sure you want to reset all settings? This cannot be undone."),
      QMessageBox::Yes | QMessageBox::No);

  if (res == QMessageBox::Yes) {
    QString path =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
        "/plasma-theme-masterrc";
    if (QFile::exists(path)) {
      if (QFile::remove(path)) {
        QMessageBox::information(
            this, tr("Success"),
            tr("Configuration cleared. Please restart the app."));
        close();
      } else {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to delete config file."));
      }
    } else {
      QMessageBox::information(this, tr("Info"),
                               tr("No configuration file found."));
    }
  }
}

void MainWindow::triggerUninstall() {
  QMessageBox::StandardButton res =
      QMessageBox::warning(this, tr("Uninstall"),
                           tr("This will launch the uninstaller script. All "
                              "application data might be removed.\nContinue?"),
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
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not launch terminal. Please run "
                                "'plasma-theme-master-uninstall' manually."));
      } else {
        close();
      }
    } else {
      close();
    }
  }
}

void MainWindow::refreshPlasma() {
  QMessageBox::StandardButton res = QMessageBox::warning(
      this, tr("Restart Plasma Shell"),
      tr("This will restart the Plasma Shell (taskbar, wallpaper, etc.).\n"
         "Your open applications will NOT be closed.\n"
         "Refer to this if you see visual glitches.\n\n"
         "Continue?"),
      QMessageBox::Yes | QMessageBox::No);

  if (res == QMessageBox::Yes) {
    QProcess::startDetached("plasmashell", QStringList() << "--replace");
  }
}

void MainWindow::checkDaemonStatus() {
  int ret = QProcess::execute("systemctl",
                              QStringList() << "--user" << "is-active"
                                            << "plasma-theme-master.service");
  // 0 = active, otherwise inactive
  bool isActive = (ret == 0);

  // Block signals to avoid triggering toggleDaemon
  bool blocked = m_daemonAction->blockSignals(true);
  m_daemonAction->setChecked(isActive);
  m_daemonAction->blockSignals(blocked);
}

void MainWindow::toggleDaemon(bool checked) {
  QStringList args;
  args << "--user";
  if (checked) {
    args << "enable" << "--now" << "plasma-theme-master.service";
  } else {
    // Disable and stop
    args << "disable" << "--now" << "plasma-theme-master.service";
  }

  QProcess::startDetached("systemctl", args);
}

void MainWindow::showFlatpakSettings() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Flatpak Settings"));
  dialog.resize(400, 300);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);

  // Status Section
  QGroupBox *statusGroup = new QGroupBox(tr("Status"), &dialog);
  QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
  QLabel *statusLabel =
      new QLabel(FlatpakManager::flatpakStatus(), statusGroup);
  statusLayout->addWidget(statusLabel);

  QPushButton *permsBtn =
      new QPushButton(tr("Re-apply Permissions"), statusGroup);
  statusLayout->addWidget(permsBtn);
  layout->addWidget(statusGroup);

  connect(permsBtn, &QPushButton::clicked, [&]() {
    if (FlatpakManager::setupFlatpakEnvironment()) {
      QMessageBox::information(&dialog, tr("Success"),
                               tr("Permissions granted successfully."));
      statusLabel->setText(FlatpakManager::flatpakStatus());
    } else {
      QMessageBox::critical(
          &dialog, tr("Error"),
          tr("Failed to setup Flatpak environment. Check logs."));
    }
  });

  // Theme Configuration
  QGroupBox *configGroup = new QGroupBox(tr("Theme Configuration"), &dialog);
  QFormLayout *formLayout = new QFormLayout(configGroup);

  QCheckBox *followCheck =
      new QCheckBox(tr("Follow System GTK Theme"), configGroup);
  followCheck->setChecked(ThemeReader::flatpakFollowsGtk());

  QComboBox *dayCombo = new QComboBox(configGroup);
  QComboBox *nightCombo = new QComboBox(configGroup);

  QStringList themes = ThemeReader::listGtkThemes();
  dayCombo->addItems(themes);
  nightCombo->addItems(themes);

  dayCombo->setCurrentText(ThemeReader::dayFlatpakTheme());
  nightCombo->setCurrentText(ThemeReader::nightFlatpakTheme());

  formLayout->addRow(followCheck);
  formLayout->addRow(tr("Day Theme:"), dayCombo);
  formLayout->addRow(tr("Night Theme:"), nightCombo);

  // Logic to disable combos if Follow is checked
  auto updateState = [&](int state) {
    bool custom = (state == Qt::Unchecked);
    dayCombo->setEnabled(custom);
    nightCombo->setEnabled(custom);
  };
  connect(followCheck, &QCheckBox::checkStateChanged, updateState);
  updateState(followCheck->isChecked() ? Qt::Checked : Qt::Unchecked);

  layout->addWidget(configGroup);

  // Buttons
  QDialogButtonBox *btnBox = new QDialogButtonBox(
      QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
  layout->addWidget(btnBox);
  connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  if (dialog.exec() == QDialog::Accepted) {
    // Save Settings
    ThemeWriter::setFlatpakFollowsGtk(followCheck->isChecked());
    if (!followCheck->isChecked()) {
      ThemeWriter::setDayFlatpakTheme(dayCombo->currentText());
      ThemeWriter::setNightFlatpakTheme(nightCombo->currentText());
    }

    // Apply immediately
    if (followCheck->isChecked()) {
      FlatpakManager::setFlatpakGtkTheme(ThemeReader::currentGtkTheme());
    } else {
      // Calculate target
      bool isDay = ThemeReader::isKWinDaytime();
      QString target =
          isDay ? dayCombo->currentText() : nightCombo->currentText();
      if (!target.isEmpty()) {
        FlatpakManager::setFlatpakGtkTheme(target);
      }
    }

    QMessageBox::information(this, tr("Saved"),
                             tr("Flatpak settings saved and applied."));
  }
}

void MainWindow::installMaterialYou() {
  QMessageBox::information(
      this, tr("Install Material You"),
      tr("A terminal window will open to install the required python packages. "
         "Press OK to proceed."));
  QProcess::startDetached(
      "konsole", QStringList()
                     << "-e" << "bash" << "-c"
                     << "pipx install kde-material-you-colors && pipx inject "
                        "kde-material-you-colors pywal16 && pipx install "
                        "pywal16; echo 'Press Enter to close'; read");
}

void MainWindow::upgradeMaterialYou() {
  QProcess::startDetached(
      "konsole", QStringList() << "-e" << "bash" << "-c"
                               << "pipx upgrade kde-material-you-colors; echo "
                                  "'Press Enter to close'; read");
}

void MainWindow::toggleMaterialYouAutostart(bool checked) {
  if (checked) {
    QProcess::startDetached("kde-material-you-colors", QStringList() << "-a");
    Logger::log("Added kde-material-you-colors to autostart.", Logger::Info);
  } else {
    QString autostartPath =
        QDir::homePath() + "/.config/autostart/kde-material-you-colors.desktop";
    if (QFile::exists(autostartPath)) {
      QFile::remove(autostartPath);
      Logger::log("Removed kde-material-you-colors from autostart.",
                  Logger::Info);
    }
  }
}

void MainWindow::promptMaterialYouInstall() {
  QString exe = QStandardPaths::findExecutable("kde-material-you-colors");
  if (exe.isEmpty()) {
    QMessageBox::StandardButton reply;
    reply =
        QMessageBox::question(this, tr("Missing Dependency"),
                              tr("kde-material-you-colors is not installed. "
                                 "Would you like to install it now via pipx?"),
                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      installMaterialYou();
    }
  }
}
