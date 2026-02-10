#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
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
  void onOffsetChanged(int value);
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

private:
  void setupUi();
  void setupMenuBar();
  void populateThemes();
  void loadSettings();
  void setupDashboardTab();
  void setupLogsTab(); // Tabs
  QTabWidget *m_mainTabs;
  QWidget *m_dashboardTab;
  QWidget *m_logsTab;
  GlobalThemeEditor *m_globalEditorTab;
  QWidget *m_universalTab; // We'll instantiate the class as a widget


  QAction *m_daemonAction;

  // --- Dashboard Widgets ---
  // Status
  QLabel *m_statusLabel;
  QLabel *m_sunInfoLabel;

  // Config
  QCheckBox *m_autoCheck;
  QSlider *m_offsetSlider;
  QLabel *m_offsetValueLabel;

  QComboBox *m_globalDayCombo;
  QComboBox *m_globalNightCombo;
  QComboBox *m_kvantumDayCombo;
  QComboBox *m_kvantumNightCombo;
  QComboBox *m_gtkDayCombo;
  QComboBox *m_gtkNightCombo;

  QPushButton *m_applyDayBtn;
  QPushButton *m_applyNightBtn;
  QPushButton *m_applyTargetBtn;
  QPushButton *m_refreshButton;

  // --- Log Widgets ---
  QTextEdit *m_logViewer;
  QPushButton *m_clearLogBtn;
  QTimer *m_logTimer;
};

#endif // MAINWINDOW_H
