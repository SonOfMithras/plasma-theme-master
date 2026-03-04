#include "CLIHandler.h"
#include "FlatpakManager.h"
#include "GlobalThemeManager.h"
#include "Logger.h"
#include "Solar.h"
#include "ThemeReader.h"
#include "UniversalThemeExporter.h"
#include "ThemeWriter.h"
#include "Config.h"
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QFileSystemWatcher>
#include <iostream>

// Forward declaration if needed, or include appropriate headers
// Assuming needed headers are already included above.

void CLIHandler::printHelp() {
  QTextStream out(stdout);
  out << "Usage: plasma-theme-master [options] command [args]\n"
      << "KDE Plasma Theme Master\n\n"
      << "Options:\n"
      << "  -h, --help    Display this help.\n"
      << "  -v, --version Display version.\n\n"
      << "Commands:\n"
      << "  status        Check the status of themes and solar info.\n"
      << "                Example: plasma-theme-master status\n\n"
      << "  flatpak-status\n"
      << "                Check the status of Flatpak theme integration.\n"
      << "                Example: plasma-theme-master flatpak-status\n\n"
      << "  flatpak-setup\n"
      << "                Setup Flatpak environment (grant filesystem "
         "permissions).\n"
      << "                Example: plasma-theme-master flatpak-setup\n\n"
      << "  set-auto      Enable/Disable auto look and feel (uses Solar "
         "calculation).\n"
      << "                Example: plasma-theme-master set-auto true\n\n"
      << "  set-offset    Set solar padding in minutes (extends the day).\n"
      << "                Example: plasma-theme-master set-offset 30\n\n"
      << "  set-kvantum   Set the active Kvantum theme immediately.\n"
      << "                Example: plasma-theme-master set-kvantum "
         "GraphiteDark\n\n"
      << "  set-kvantum-day <theme>\n"
      << "                Set preferred Kvantum theme for Day mode.\n\n"
      << "  set-kvantum-night <theme>\n"
      << "                Set preferred Kvantum theme for Night mode.\n\n"
      << "  set-gtk <theme>\n"
      << "                Instantly apply a GTK theme (updates GTK 3/4 "
         "settings).\n\n"
      << "  set-gtk-day <theme>\n"
      << "                Set preferred GTK theme for Day mode.\n\n"
      << "  set-gtk-night <theme>\n"
      << "                Set preferred GTK theme for Night mode.\n\n"
      << "  set-global-light <theme>\n"
      << "                Set preferred Global Theme for Day mode.\n\n"
      << "  set-global-dark <theme>\n"
      << "                Set preferred Global Theme for Night mode.\n\n"
      << "  set-static-dark\n"
      << "                Disable auto l&f and apply defaults (Global + "
         "Kvantum + GTK).\n"
      << "                Example: plasma-theme-master set-static-dark\n\n"
      << "  set-static-light\n"
      << "                Disable auto l&f and apply defaults (Global + "
         "Kvantum + GTK).\n"
      << "                Example: plasma-theme-master set-static-light\n\n"
      << "  clone-global <source> <dest>\n"
      << "                Clone a global theme to the user directory.\n"
      << "                Example: plasma-theme-master clone-global Breeze "
         "MyBreeze\n\n"
      << "  uninstall\n"
      << "                Run the uninstallation script.\n\n"
      << "  sync-universal (or sync-now)\n"
      << "                Sync enabled universal apps immediately.\n\n"
      << "  sync-enable <app>\n"
      << "                Enable universal sync for an app (vscode, firefox, discord, kitty, obsidian).\n"
      << "                WARNING: backups will be created.\n\n"
      << "  sync-disable <app>\n"
      << "                Disable universal sync for an app.\n\n"
      << "  sync-list\n"
      << "                List universal sync apps and their status.\n\n"
      << "  sync-restore <app>\n"
      << "                Restore an app configuration from backup.\n\n"
      << "  log [-n <lines>] [--errors]\n"
      << "                View application logs. Default: last 100 lines.\n"
      << "                Example: plasma-theme-master log -n 50 --errors\n\n";
}

int CLIHandler::handleCommand(const QString &command, const QStringList &args) {
  if (command == "status") {
    QTextStream out(stdout);

    double lat = ThemeReader::nativeLatitude();
    double lon = ThemeReader::nativeLongitude();
    int offset = ThemeReader::solarPadding();

    QDateTime nowUtc = QDateTime::currentDateTime().toUTC();
    QPair<QDateTime, QDateTime> times =
        Solar::calculateSunTimes(lat, lon, nowUtc.date());

    // Convert to local time for display
    QString sunriseStr = times.first.isValid()
                             ? times.first.toLocalTime().toString("HH:mm")
                             : "N/A";
    QString sunsetStr = times.second.isValid()
                            ? times.second.toLocalTime().toString("HH:mm")
                            : "N/A";

    // Calculate effective Day Start/End with Offset
    int shiftSecs = (offset * 60) / 2;
    QDateTime dayStart =
        times.first.isValid() ? times.first.addSecs(-shiftSecs) : QDateTime();
    QDateTime dayEnd =
        times.second.isValid() ? times.second.addSecs(shiftSecs) : QDateTime();

    QString dayStartStr =
        dayStart.isValid() ? dayStart.toLocalTime().toString("HH:mm") : "N/A";
    QString dayEndStr =
        dayEnd.isValid() ? dayEnd.toLocalTime().toString("HH:mm") : "N/A";

    out << "=== Plasma Theme Master Status ===\n";
    out << "Global Theme: " << ThemeReader::currentGlobalTheme() << "\n";
    out << "Kvantum Theme: " << ThemeReader::currentKvantumTheme() << "\n";
    out << "GTK Theme: " << ThemeReader::currentGtkTheme() << "\n";
    out << "Auto LookAndFeel: "
        << (ThemeReader::isAutoLookAndFeel() ? "True" : "False") << "\n";
    out << "\n[Defaults]\n";
    out << "Global Dark: " << ThemeReader::defaultDarkTheme() << "\n";
    out << "Global Light: " << ThemeReader::defaultLightTheme() << "\n";
    out << "Kvantum Dark: " << ThemeReader::nightKvantumTheme() << "\n";
    out << "Kvantum Light: " << ThemeReader::dayKvantumTheme() << "\n";
    out << "GTK Dark: " << ThemeReader::nightGtkTheme() << "\n";
    out << "GTK Light: " << ThemeReader::dayGtkTheme() << "\n";
    out << "Klassy Day Preset: " << ThemeReader::dayKlassyPreset() << "\n";
    out << "Klassy Night Preset: " << ThemeReader::nightKlassyPreset() << "\n";
    out << "Last Applied Klassy: " << ThemeReader::lastAppliedKlassyPreset() << "\n";
    out << "\n[Solar]\n";
    out << "Location: " << lat << ", " << lon << "\n";
    out << "Sunrise: " << sunriseStr << "\n";
    out << "Sunset: " << sunsetStr << "\n";
    out << "Offset (Padding): " << offset << " minutes\n";
    out << "Day Start: " << dayStartStr << "\n";
    out << "Night Start: " << dayEndStr << "\n";

    // Add Flatpak status to general status
    out << "\n[Flatpak]\n";
    out << "Status: " << FlatpakManager::flatpakStatus() << "\n";

    return 0;
  } else if (command == "flatpak-status") {
    if (!FlatpakManager::isFlatpakInstalled()) {
      std::cout << "Flatpak is not installed on this system.\n";
      return 1;
    }

    std::cout << "Flatpak Status: "
              << qPrintable(FlatpakManager::flatpakStatus()) << "\n";
    if (FlatpakManager::hasFilesystemAccess()) {
      std::cout << "\nExposed Directories (Overrides):\n";
      QStringList exposed = FlatpakManager::getExposedDirectories();
      for (const QString &path : exposed) {
        std::cout << " - " << qPrintable(path) << "\n";
      }
    } else {
      std::cout << "\nEnvironment is NOT setup. Run 'plasma-theme-master "
                   "flatpak-setup' to configure.\n";
    }
    return 0;
  } else if (command == "flatpak-setup") {
    std::cout << "Setting up Flatpak environment...\n";
    if (FlatpakManager::setupFlatpakEnvironment()) {
      std::cout << "Success! Flatpak overrides applied.\n";
      return 0;
    } else {
      std::cerr << "Failed to apply Flatpak overrides. Check log.\n";
      return 1;
    }
  } else if (command == "set-flatpak") {
    if (args.size() < 2) {
      std::cerr << "Error: set-flatpak requires a theme name.\n";
      return 1;
    }
    FlatpakManager::setFlatpakGtkTheme(args.at(1));
    return 0;
  } else if (command == "set-flatpak-day") {
    if (args.size() < 2) {
      std::cerr << "Error: set-flatpak-day requires a theme name.\n";
      return 1;
    }
    ThemeWriter::setDayFlatpakTheme(args.at(1));
    return 0;
  } else if (command == "set-flatpak-night") {
    if (args.size() < 2) {
      std::cerr << "Error: set-flatpak-night requires a theme name.\n";
      return 1;
    }
    ThemeWriter::setNightFlatpakTheme(args.at(1));
    return 0;
  } else if (command == "set-flatpak-follow") {
    if (args.size() < 2) {
      std::cerr << "Error: set-flatpak-follow requires a value (true/false).\n";
      return 1;
    }
    QString val = args.at(1).toLower();
    bool enabled = (val == "true" || val == "on" || val == "1");
    ThemeWriter::setFlatpakFollowsGtk(enabled);
    return 0;
  } else if (command == "set-offset") {
    if (args.size() < 2) {
      std::cerr << "Error: set-offset requires a value (minutes).\n";
      return 1;
    }
    bool ok;
    int val = args.at(1).toInt(&ok);
    if (!ok) {
      std::cerr << "Error: Invalid integer value.\n";
      return 1;
    }
    ThemeWriter::setSolarPadding(val);
    return 0;
  } else if (command == "set-auto") {
    if (args.size() < 2) {
      std::cerr << "Error: set-auto requires a value (true/false).\n";
      printHelp(); // Show help on error
      return 1;
    }
    QString val = args.at(1).toLower();
    bool enabled = (val == "true" || val == "on" || val == "1");

    if (enabled) {
      // 1. Calculate correct theme
      double lat = ThemeReader::nativeLatitude();
      double lon = ThemeReader::nativeLongitude();
      int offset = ThemeReader::solarPadding();
      bool isDay = Solar::isDaytime(lat, lon, offset);

      QString targetGlobal;
      QString targetKvantum;

      if (isDay) {
        targetGlobal = ThemeReader::defaultLightTheme();
        targetKvantum = ThemeReader::dayKvantumTheme();
      } else {
        targetGlobal = ThemeReader::defaultDarkTheme();
        targetKvantum = ThemeReader::nightKvantumTheme();
      }

      if (!targetGlobal.isEmpty()) {
        std::cout << "Applying calculated Global (" << (isDay ? "Day" : "Night")
                  << "): " << qPrintable(targetGlobal) << "\n";
        ThemeWriter::applyGlobalTheme(targetGlobal);
        if (Config::isMaterialYouOverrideEnabled()) {
             ThemeWriter::applyColorScheme(isDay ? "MaterialYouLight" : "MaterialYouDark");
        }
        // Kvantum might be reset by Global Theme, so always apply it AFTER
      }

      if (!targetKvantum.isEmpty()) {
        std::cout << "Applying calculated Kvantum ("
                  << (isDay ? "Day" : "Night")
                  << "): " << qPrintable(targetKvantum) << "\n";
        ThemeWriter::setKvantumTheme(targetKvantum);
      }

      QString targetGtk =
          isDay ? ThemeReader::dayGtkTheme() : ThemeReader::nightGtkTheme();
      if (!targetGtk.isEmpty()) {
        std::cout << "Applying calculated GTK (" << (isDay ? "Day" : "Night")
                  << "): " << qPrintable(targetGtk) << "\n";
        ThemeWriter::setGtkTheme(targetGtk);
      }

      // Enable Auto AFTER applying themes, as applyGlobalTheme might reset it.
      ThemeWriter::setAutoLookAndFeel(true);
      UniversalThemeExporter::syncAll();
      std::cout << "AutomaticLookAndFeel set to: True\n";
    } else {
      ThemeWriter::setAutoLookAndFeel(false);
      std::cout << "AutomaticLookAndFeel set to: False\n";
    }

    return 0;
  } else if (command == "set-kvantum") {
    if (args.size() < 2) {
      std::cerr << "Error: set-kvantum requires a theme name.\n";
      return 1;
    }
    QString theme = args.at(1);
    // Verify existence
    QStringList themes = ThemeReader::listKvantumThemes();
    QString targetTheme = theme;
    bool found = false;
    for (const QString &t : themes) {
      if (t.compare(theme, Qt::CaseInsensitive) == 0) {
        targetTheme = t;
        found = true;
        break;
      }
    }
    if (!found && themes.contains(theme))
      found = true; // Exact match fallback

    if (!found) {
      std::cerr << "Error: Theme '" << qPrintable(theme) << "' not found.\n";
      std::cerr << "Available themes:\n";
      for (const QString &t : themes)
        std::cerr << "  " << qPrintable(t) << "\n";
      return 1;
    }

    ThemeWriter::setKvantumTheme(targetTheme);
    return 0;
  } else if (command == "set-kvantum-day") {
    if (args.size() < 2) {
      std::cerr << "Error: " << qPrintable(command)
                << " requires a theme name.\n";
      return 1;
    }
    ThemeWriter::setDayKvantumTheme(args.at(1));
    Logger::log("Set DayKvantumTheme to \"" + args.at(1) + "\"", Logger::Info);
    return 0;
  } else if (command == "set-kvantum-night") {
    if (args.size() < 2) {
      std::cerr << "Error: " << qPrintable(command)
                << " requires a theme name.\n";
      return 1;
    }
    ThemeWriter::setNightKvantumTheme(args.at(1));
    Logger::log("Set NightKvantumTheme to \"" + args.at(1) + "\"",
                Logger::Info);
    return 0;
  } else if (command == "set-gtk") {
    if (args.size() < 2) {
      std::cerr << "Error: set-gtk requires a theme name.\n";
      return 1;
    }
    QString theme = args.at(1);
    // Verify existence
    QStringList themes = ThemeReader::listGtkThemes();
    QString targetTheme = theme;
    bool found = false;
    for (const QString &t : themes) {
      if (t.compare(theme, Qt::CaseInsensitive) == 0) {
        targetTheme = t;
        found = true;
        break;
      }
    }
    if (!found && themes.contains(theme))
      found = true; // Exact match fallback

    if (!found) {
      std::cerr << "Error: Theme '" << qPrintable(theme) << "' not found.\n";
      std::cerr << "Available themes:\n";
      for (const QString &t : themes)
        std::cerr << "  " << qPrintable(t) << "\n";
      return 1;
    }

    ThemeWriter::setGtkTheme(targetTheme);
    return 0;
  } else if (command == "set-gtk-day") {
    if (args.size() < 2) {
      std::cerr << "Error: " << qPrintable(command)
                << " requires a theme name.\n";
      return 1;
    }
    QString value = args.at(1);
    ThemeWriter::setDayGtkTheme(value);
    Logger::log("Set DayGtkTheme to \"" + value + "\"", Logger::Info);
    return 0;
  } else if (command == "set-gtk-night") {
    if (args.size() < 2) {
      std::cerr << "Error: " << qPrintable(command)
                << " requires a theme name.\n";
      return 1;
    }
    QString value = args.at(1);
    ThemeWriter::setNightGtkTheme(value);
    Logger::log("Set NightGtkTheme to \"" + value + "\"", Logger::Info);
    return 0;
  } else if (command == "set-global-dark") {
    if (args.size() < 2)
      return 1;
    ThemeWriter::setDefaultDarkTheme(args.at(1));
    return 0;
  } else if (command == "set-global-light") {
    if (args.size() < 2)
      return 1;
    ThemeWriter::setDefaultLightTheme(args.at(1));
    return 0;
  } else if (command == "set-static-dark") {
    QString global = ThemeReader::defaultDarkTheme();
    QString kvantum = ThemeReader::nightKvantumTheme();

    if (global.isEmpty())
      std::cerr << "Warning: No Default Dark Global Theme.\n";

    ThemeWriter::setAutoLookAndFeel(false);
    
    auto applyTheme = [&]() {
        if (!global.isEmpty()) {
            ThemeWriter::applyGlobalTheme(global, true);
            if (Config::isMaterialYouOverrideEnabled()) {
                ThemeWriter::applyColorScheme("MaterialYouDark", true);
            }
        }
        if (!kvantum.isEmpty()) {
          ThemeWriter::setKvantumTheme(kvantum, true);
        }
        QString gtk = ThemeReader::nightGtkTheme();
        if (!gtk.isEmpty()) {
          ThemeWriter::setGtkTheme(gtk, true);
        }
    };

    std::cout << "Applying Default Dark Themes...\n";
    applyTheme();

    std::cout << "Waiting 2 seconds for kded to write to kdeglobals...\n";
    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    
    UniversalThemeExporter::syncAll();
    std::cout << "Done.\n";
    
    return 0;
  } else if (command == "set-static-light") {
    QString global = ThemeReader::defaultLightTheme();
    QString kvantum = ThemeReader::dayKvantumTheme();

    if (global.isEmpty())
      std::cerr << "Warning: No Default Light Global Theme.\n";

    ThemeWriter::setAutoLookAndFeel(false);
    
    auto applyTheme = [&]() {
        if (!global.isEmpty()) {
            ThemeWriter::applyGlobalTheme(global, true);
            if (Config::isMaterialYouOverrideEnabled()) {
                ThemeWriter::applyColorScheme("MaterialYouLight", true);
            }
        }
        if (!kvantum.isEmpty()) {
          ThemeWriter::setKvantumTheme(kvantum, true);
        }
        QString gtk = ThemeReader::dayGtkTheme();
        if (!gtk.isEmpty()) {
          ThemeWriter::setGtkTheme(gtk, true);
        }
    };

    std::cout << "Applying Default Light Themes...\n";
    applyTheme();

    std::cout << "Waiting 2 seconds for kded to write to kdeglobals...\n";
    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    
    UniversalThemeExporter::syncAll();
    std::cout << "Done.\n";

    return 0;
  } else if (command == "log") {
    int lines = 500;
    bool errorsOnly = false;

    const QStringList rawArgs = QCoreApplication::arguments();

    // Simple manual parsing for log sub-args
    // We scan starting from index 2 to find flags
    for (int i = 2; i < rawArgs.size(); ++i) {
      if (rawArgs[i] == "-n" && i + 1 < rawArgs.size()) {
        lines = rawArgs[i + 1].toInt();
        i++;
      } else if (rawArgs[i] == "--errors") {
        errorsOnly = true;
      }
    }

    QStringList logs = Logger::readLogs(lines, errorsOnly);
    for (const QString &line : logs) {
      std::cout << qPrintable(line) << "\n";
    }
    return 0;
  } else if (command == "daemon") {
    std::cout << "Starting Plasma Theme Master Daemon...\n";
    Logger::log("Daemon started", Logger::Info);

    // Monitor setup for automatic theme switching and universal sync triggers
    QTimer *solarTimer = new QTimer(qApp);
    
    // We create a separate debounce timer for file watcher events
    QTimer *syncDebounceTimer = new QTimer(qApp);
    syncDebounceTimer->setSingleShot(true);
    syncDebounceTimer->setInterval(2000); // 2000ms debounce
    
    QObject::connect(syncDebounceTimer, &QTimer::timeout, qApp, []() {
        Logger::log("Daemon: Triggering syncAll due to kdeglobals change (MaterialYou/Color sync)", Logger::Info);
        UniversalThemeExporter::syncAll();
    });

    QFileSystemWatcher *watcher = new QFileSystemWatcher(qApp);
    QString kdeGlobalsFile = QDir::homePath() + "/.config/kdeglobals";
    // Add the file to watch for changes
    if (QFile::exists(kdeGlobalsFile)) {
        watcher->addPath(kdeGlobalsFile);
    }
    
    QObject::connect(watcher, &QFileSystemWatcher::fileChanged, syncDebounceTimer, [watcher, kdeGlobalsFile, syncDebounceTimer](const QString &path) {
        (void)path;
        // Restart the timer. If it receives multiple signals quickly, it will only fire once after 2 seconds.
        syncDebounceTimer->start();
        
        // Qt sometimes removes a watched file if it's replaced (like some text editors do).
        // Let's ensure it's re-added if it was removed.
        if (!watcher->files().contains(kdeGlobalsFile) && QFile::exists(kdeGlobalsFile)) {
             watcher->addPath(kdeGlobalsFile);
        }
    });

    auto performSolarCheck = []() {
      // 1. Check if Auto is enabled
      if (ThemeReader::isAutoLookAndFeel()) {
        double lat = ThemeReader::nativeLatitude();
        double lon = ThemeReader::nativeLongitude();
        int offset = ThemeReader::solarPadding();

        bool isDay = Solar::isDaytime(lat, lon, offset);

        QString currentGlobal = ThemeReader::currentGlobalTheme();
        QString targetGlobal = isDay ? ThemeReader::defaultLightTheme()
                                     : ThemeReader::defaultDarkTheme();

        bool needUpdate = false;

        if (!targetGlobal.isEmpty() && currentGlobal != targetGlobal) {
          Logger::log("Daemon: Global theme mismatch detected. Applying " +
                          targetGlobal,
                      Logger::Info);
          ThemeWriter::applyGlobalTheme(targetGlobal);
          if (Config::isMaterialYouOverrideEnabled()) {
               ThemeWriter::applyColorScheme(isDay ? "MaterialYouLight" : "MaterialYouDark");
          }
          needUpdate = true;
        }

        // Kvantum
        QString currentKvantum = ThemeReader::currentKvantumTheme();
        QString targetKvantum = isDay ? ThemeReader::dayKvantumTheme()
                                      : ThemeReader::nightKvantumTheme();
        if (!targetKvantum.isEmpty() && currentKvantum != targetKvantum) {
          Logger::log("Daemon: Kvantum theme mismatch detected. Applying " +
                          targetKvantum,
                      Logger::Info);
          ThemeWriter::setKvantumTheme(targetKvantum);
          needUpdate = true;
        }

        // GTK
        QString currentGtk = ThemeReader::currentGtkTheme();
        QString targetGtk =
            isDay ? ThemeReader::dayGtkTheme() : ThemeReader::nightGtkTheme();
        if (!targetGtk.isEmpty() && currentGtk != targetGtk) {
          Logger::log("Daemon: GTK theme mismatch detected. Applying " +
                          targetGtk,
                      Logger::Info);
          ThemeWriter::setGtkTheme(targetGtk);
          needUpdate = true;
        }

        // Klassy Window Decorations
        QString currentKlassy = ThemeReader::lastAppliedKlassyPreset();
        QString targetKlassy = isDay ? ThemeReader::dayKlassyPreset() : ThemeReader::nightKlassyPreset();
        if (!targetKlassy.isEmpty() && currentKlassy != targetKlassy) {
            Logger::log("Daemon: Klassy preset mismatch detected. Applying " + targetKlassy, Logger::Info);
            ThemeWriter::setKlassyPreset(targetKlassy);
            needUpdate = true;
        }
        
        // Flatpak (Independent configuration)
        if (!ThemeReader::flatpakFollowsGtk()) {
             QString currentFlatpak = FlatpakManager::getFlatpakGtkTheme();
             QString targetFlatpak = isDay ? ThemeReader::dayFlatpakTheme() : ThemeReader::nightFlatpakTheme();
             
             if (!targetFlatpak.isEmpty() && currentFlatpak != targetFlatpak) {
                 Logger::log("Daemon: Flatpak theme mismatch detected. Applying " + targetFlatpak, Logger::Info);
                 FlatpakManager::setFlatpakGtkTheme(targetFlatpak);
                 // No need to set needUpdate=true as this doesn't affect system auto l&f state directly
             }
        }

        // If we updated anything, ensure Auto flag remains True (it might
        // technically not change but good to be sure)
        if (needUpdate) {
          ThemeWriter::setAutoLookAndFeel(true);
          UniversalThemeExporter::syncAll();
        }
      }
    };

    QObject::connect(solarTimer, &QTimer::timeout, qApp, performSolarCheck);
    
    // Perform initial check immediately
    performSolarCheck();
    
    // Start interval timer
    solarTimer->start(60000); // 60 seconds

    return qApp->exec();
  } else if (command == "uninstall") {
    std::cout << "Launching Uninstaller...\n";
    int ret = std::system("plasma-theme-master-uninstall");
    return WEXITSTATUS(ret);
  } else if (command == "sync-universal" || command == "sync-now") {
    std::cout << "Syncing universal theme to configured apps...\n";
    
    // 1. Run standard sync for all enabled apps
    UniversalThemeExporter::syncAll();

    // 2. Extra CLI-only Check: Workspace
    if (Config::isVSCodeSyncEnabled()) {
        QDir currentDir = QDir::current();
        QStringList workspaceFiles = currentDir.entryList(QStringList() << "*.code-workspace", QDir::Files);
        if (currentDir.exists(".vscode") || currentDir.exists("CMakeLists.txt") || currentDir.exists("package.json") || !workspaceFiles.isEmpty()) {
             UniversalPalette palette = UniversalThemeExporter::extractColors();
             QString workspaceSettings = currentDir.absolutePath() + "/.vscode/settings.json";
             UniversalThemeExporter::exportToVSCodeJSON(workspaceSettings, palette);
             Logger::log("Detected workspace, exported to: " + workspaceSettings, Logger::Info);
        }
    }
    
    std::cout << "Sync completed. Check logs for details.\n";
    
    return 0;
  } else if (command == "sync-enable") {
      if (args.size() < 2) { std::cerr << "Usage: sync-enable <app>\n"; return 1; }
      QString app = args.at(1).toLower();
      if (app == "vscode") {
          std::cout << "Enabling VS Code Sync. WARNING: modification of settings.json. Backups will be created.\n";
          Config::setVSCodeSyncEnabled(true);
      } else if (app == "firefox") {
          std::cout << "Enabling Firefox Sync. WARNING: modification of userChrome.css. Backups will be created.\n";
          Config::setFirefoxSyncEnabled(true);
      } else if (app == "discord") {
          std::cout << "Enabling BetterDiscord Sync. WARNING: modification of theme css.\n";
          Config::setBetterDiscordSyncEnabled(true);
      } else if (app == "kitty") {
          std::cout << "Enabling Kitty Sync. WARNING: modification of kitty.conf include.\n";
          Config::setKittySyncEnabled(true);
      } else if (app == "obsidian") {
           std::cout << "Enabling Obsidian Sync.\n";
           Config::setObsidianSyncEnabled(true);
      } else {
          std::cerr << "Unknown app: " << qPrintable(app) << "\n";
          return 1;
      }
      std::cout << "Enabled sync for " << qPrintable(app) << "\n";
      return 0;
  } else if (command == "sync-disable") {
      if (args.size() < 2) { std::cerr << "Usage: sync-disable <app>\n"; return 1; }
      QString app = args.at(1).toLower();
      if (app == "vscode") Config::setVSCodeSyncEnabled(false);
      else if (app == "firefox") Config::setFirefoxSyncEnabled(false);
      else if (app == "discord") Config::setBetterDiscordSyncEnabled(false);
      else if (app == "kitty") Config::setKittySyncEnabled(false);
      else if (app == "obsidian") Config::setObsidianSyncEnabled(false);
      else { std::cerr << "Unknown app: " << qPrintable(app) << "\n"; return 1; }
      std::cout << "Disabled sync for " << qPrintable(app) << "\n";
      return 0;
   } else if (command == "sync-list") {
       std::cout << "Universal Sync Status:\n";
       std::cout << "  VS Code: " << (Config::isVSCodeSyncEnabled() ? "Enabled" : "Disabled") << "\n";
       std::cout << "  Firefox: " << (Config::isFirefoxSyncEnabled() ? "Enabled" : "Disabled") << "\n";
       std::cout << "  BetterDiscord: " << (Config::isBetterDiscordSyncEnabled() ? "Enabled" : "Disabled") << "\n";
       std::cout << "  Kitty: " << (Config::isKittySyncEnabled() ? "Enabled" : "Disabled") << "\n";
       std::cout << "  Obsidian: " << (Config::isObsidianSyncEnabled() ? "Enabled" : "Disabled") << "\n";
       return 0;
    } else if (command == "sync-restore") {
        if (args.size() < 2) { std::cerr << "Usage: sync-restore <app>\n"; return 1; }
        QString app = args.at(1).toLower();
        bool success = false;
        
        if (app == "vscode") {
            std::cout << "Restoring VS Code settings...\n";
            success = UniversalThemeExporter::restoreVSCode();
        } else if (app == "firefox") {
             std::cout << "Restoring Firefox (Partial)...\n";
             success = UniversalThemeExporter::restoreFirefox();
        } else if (app == "discord") {
             std::cout << "Restoring BetterDiscord...\n";
             success = UniversalThemeExporter::restoreBetterDiscord();
        } else if (app == "kitty") {
            std::cout << "Restoring Kitty config...\n";
            success = UniversalThemeExporter::restoreKitty();
        } else if (app == "obsidian") {
            std::cout << "Restoring Obsidian snippet...\n";
            success = UniversalThemeExporter::restoreObsidian();
        } else {
            std::cout << "Unknown app or restore not supported: " << qPrintable(app) << "\n";
            return 1;
        }
        
        if (success) std::cout << "Restore successful.\n";
        else std::cout << "Restore finished (some files may not have existed or failed).\n";
        
        return 0;
   } else if (command == "clone-global") {
    if (args.size() < 2) {
      std::cout << "Usage: plasma-theme-master clone-global <source> <dest>\n";
      return 1;
    }
    QString src = args.at(0);
    QString dest = args.at(1);

    std::cout << "Cloning global theme '" << qPrintable(src) << "' to '"
              << qPrintable(dest) << "'...\n";
    if (GlobalThemeManager::cloneTheme(src, dest)) {
      std::cout << "Success! Cloned to " << qPrintable(dest) << "\n";
      return 0;
    } else {
      std::cerr << "Failed to clone theme. Check logs for details.\n";
      return 1;
    }
  }

  std::cerr << "Unknown command: " << qPrintable(command) << "\n";
  printHelp();
  return 1;
}
