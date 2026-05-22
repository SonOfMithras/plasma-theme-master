#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

class QTabWidget;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSlider;
class QTimer;

class GlobalThemeEditor; // Forward Declaration
class ColorSwatchPicker;  // Forward Declaration

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private Q_SLOTS:
  void refreshStatus();
  void saveSettings();
  void toggleAuto(bool checked);
  void applyStaticDay();
  void applyStaticNight();
  void applyCurrentTarget();
  void onDayOffsetChanged(int value);
  void onNightOffsetChanged(int value);
  void updateLogs();
  void clearLogs();
  void showAbout();
  // File Menu Actions
  void openGlobalThemesFolder();
  void openKvantumThemesFolder();
  void openGtkThemesFolder();
  void openAppConfigFolder();
  // Help Menu Actions
  void clearConfig();
  void triggerUninstall();
  void refreshPlasma();

  // Flatpak
  void showFlatpakSettings();
  
  void checkDaemonStatus();
  void toggleDaemon(bool checked);

  // Material You
  void installMaterialYou();
  void upgradeMaterialYou();
  void toggleMaterialYouAutostart(bool checked);
  void promptMaterialYouInstall();
  void applyMaterialYouSettings();
  void onMaterialYouSettingsChanged();

private:
  void setupUi();
  void setupMenuBar();
  void populateThemes();
  void loadSettings();
  void setupDashboardTab();
  void setupLogsTab(); // Tabs
  void setupMaterialYouTab();
  QTabWidget *m_mainTabs;
  QWidget *m_dashboardTab;
  QWidget *m_logsTab;
  GlobalThemeEditor *m_globalEditorTab;
  QWidget *m_universalTab; 
  QWidget *m_materialYouTab;


  QAction *m_daemonAction;

  QAction *m_myAutostartAction;

  // --- Dashboard Widgets ---
  // Status
  QLabel *m_statusLabel;
  QLabel *m_sunInfoLabel;

  // Config
  QCheckBox *m_autoCheck;
  QCheckBox *m_materialYouCheck;
  QCheckBox *m_reenableAutoCheck;
  QSlider *m_dayOffsetSlider;
  QLabel *m_dayOffsetValueLabel;
  QSlider *m_nightOffsetSlider;
  QLabel *m_nightOffsetValueLabel;

  QComboBox *m_globalDayCombo;
  QComboBox *m_globalNightCombo;
  QComboBox *m_kvantumDayCombo;
  QComboBox *m_kvantumNightCombo;
  QComboBox *m_gtkDayCombo;
  QComboBox *m_gtkNightCombo;
  QComboBox *m_klassyDayCombo;
  QComboBox *m_klassyNightCombo;

  QPushButton *m_applyDayBtn;
  QPushButton *m_applyNightBtn;
  QPushButton *m_applyTargetBtn;
  QPushButton *m_refreshButton;
  QPushButton *m_kcmNightColorBtn;

  // --- Night Color Settings ---
  QSlider *m_dayTempSlider;
  QSpinBox *m_dayTempSpinBox;
  QSlider *m_nightTempSlider;
  QSpinBox *m_nightTempSpinBox;

  // --- Material You Tab Widgets ---
  QComboBox *m_schemeVariantCombo;
  QSlider *m_chromaSlider;
  QDoubleSpinBox *m_chromaSpinBox;
  QSlider *m_toneSlider;
  QDoubleSpinBox *m_toneSpinBox;
  ColorSwatchPicker *m_nColorPicker;
  QPushButton *m_myApplyBtn;
  QPushButton *m_refreshSwatchesBtn;

  // --- Log Widgets ---
  QTextEdit *m_logViewer;
  QPushButton *m_clearLogBtn;
  QTimer *m_logTimer;
};

#endif // MAINWINDOW_H
