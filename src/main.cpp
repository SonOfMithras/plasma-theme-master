#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>
#include <iostream>
#include <QThread>
#include "core/ThemeReader.h"
#include "core/ThemeWriter.h"
#include "core/Solar.h"
#include "core/Config.h"
#include "core/Logger.h"
#include "core/GlobalThemeManager.h"
#include "gui/MainWindow.h"


// ... (Top includes remain same, adding QDateTime/QTimeZone if needed, but they are in Solar.h)

static void printHelp() {
    QTextStream out(stdout);
    out << "Usage: plasma-theme-master [options] command [args]\n"
              << "KDE Plasma Theme Master\n\n"
              << "Options:\n"
              << "  -h, --help    Display this help.\n"
              << "  -v, --version Display version.\n\n"
              << "Commands:\n"
              << "  status        Check the status of themes and solar info.\n"
              << "                Example: plasma-theme-master status\n\n"
              << "  set-auto      Enable/Disable auto look and feel (uses Solar calculation).\n"
              << "                Example: plasma-theme-master set-auto true\n\n"
              << "  set-offset    Set solar padding in minutes (extends the day).\n"
              << "                Example: plasma-theme-master set-offset 30\n\n"
              << "  set-kvantum   Set the active Kvantum theme immediately.\n"
              << "                Example: plasma-theme-master set-kvantum GraphiteDark\n\n"
              << "  set-kvantum-day <theme>\n"
              << "                Set preferred Kvantum theme for Day mode.\n\n"
              << "  set-kvantum-night <theme>\n"
              << "                Set preferred Kvantum theme for Night mode.\n\n"
              << "  set-gtk <theme>\n"
              << "                Instantly apply a GTK theme (updates GTK 3/4 settings).\n\n"
              << "  set-gtk-day <theme>\n"
              << "                Set preferred GTK theme for Day mode.\n\n"
              << "  set-gtk-night <theme>\n"
              << "                Set preferred GTK theme for Night mode.\n\n"
              << "  set-global-light <theme>\n"
              << "                Set preferred Global Theme for Day mode.\n\n"
              << "  set-global-dark <theme>\n"
              << "                Set preferred Global Theme for Night mode.\n\n"
              << "  set-static-dark\n"
              << "                Disable auto l&f and apply defaults (Global + Kvantum + GTK).\n"
              << "                Example: plasma-theme-master set-static-dark\n\n"
              << "  set-static-light\n"
              << "                Disable auto l&f and apply defaults (Global + Kvantum + GTK).\n"
              << "                Example: plasma-theme-master set-static-light\n\n"
              << "  clone-global <source> <dest>\n"
              << "                Clone a global theme to the user directory.\n"
              << "                Example: plasma-theme-master clone-global Breeze MyBreeze\n\n"
              << "  uninstall\n"
              << "                Run the uninstallation script.\n\n"
              << "  log [-n <lines>] [--errors]\n"
              << "                View application logs. Default: last 100 lines.\n"
              << "                Example: plasma-theme-master log -n 50 --errors\n\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("plasma-theme-master");
    QCoreApplication::setApplicationVersion("0.1.0");

    // Check for CLI args *before* creating QApplication if possible, 
    // but Config class uses QSettings which might need QCoreApplication (or not).
    // Safest to create QCoreApplication or QApplication depending on mode.
    
    // Simple check: if we have args other than app name, likely CLI.
    bool guiMode = true;
    if (argc > 1) {
        QString arg = QString::fromLocal8Bit(argv[1]);
        if (arg == "status" || arg == "set-auto" || arg == "set-offset" ||
            arg == "set-kvantum" || arg == "set-kvantum-day" || arg == "set-kvantum-night" ||
            arg == "set-gtk" || arg == "set-gtk-day" || arg == "set-gtk-night" ||
            arg == "set-global-dark" || arg == "set-global-light" ||
            arg == "log" || arg == "daemon" || arg == "uninstall" ||
            arg == "set-static-dark" || arg == "set-static-light" ||
            arg == "clone-global" ||
            arg == "--help" || arg == "-h" || arg == "--version" || arg == "-v") {
            guiMode = false;
            // No break here, as the original code had a loop.
            // The provided snippet for main() implies a different guiMode check.
            // Let's stick to the original loop structure but update the condition.
        }
    }

    QScopedPointer<QCoreApplication> app;
    if (guiMode) {
        app.reset(new QApplication(argc, argv));
    } else {
        app.reset(new QCoreApplication(argc, argv));
    }

    app->setApplicationName("plasma-theme-master");
    app->setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("KDE Plasma Theme Master");
    
    // Manual help option to override default formatting
    auto helpOption = QCommandLineOption(QStringList() << "h" << "help", "Display this help.");
    parser.addOption(helpOption);
    parser.addVersionOption();
    
    parser.addPositionalArgument("command", "Command to execute");
    parser.addPositionalArgument("value", "Value for command");

    // Don't process yet if we're just defaulting to GUI
    if (!guiMode) {
        parser.parse(QCoreApplication::arguments());
        
        if (parser.isSet(helpOption)) {
            printHelp();
            return 0;
        }

        if (parser.isSet("version")) {
            parser.showVersion();
            return 0;
        }

        const QStringList args = parser.positionalArguments();
        
        if (!args.isEmpty()) {
            const QString command = args.first();
            
            if (command == "status") {
                QTextStream out(stdout);
                
                double lat = ThemeReader::nativeLatitude();
                double lon = ThemeReader::nativeLongitude();
                int offset = ThemeReader::solarPadding();
                
                QDateTime nowUtc = QDateTime::currentDateTime().toUTC();
                QPair<QDateTime, QDateTime> times = Solar::calculateSunTimes(lat, lon, nowUtc.date());
                
                // Convert to local time for display
                QString sunriseStr = times.first.isValid() ? times.first.toLocalTime().toString("HH:mm") : "N/A";
                QString sunsetStr = times.second.isValid() ? times.second.toLocalTime().toString("HH:mm") : "N/A";
                
                // Calculate effective Day Start/End with Offset
                int shiftSecs = (offset * 60) / 2;
                QDateTime dayStart = times.first.isValid() ? times.first.addSecs(-shiftSecs) : QDateTime();
                QDateTime dayEnd = times.second.isValid() ? times.second.addSecs(shiftSecs) : QDateTime();
                
                QString dayStartStr = dayStart.isValid() ? dayStart.toLocalTime().toString("HH:mm") : "N/A";
                QString dayEndStr = dayEnd.isValid() ? dayEnd.toLocalTime().toString("HH:mm") : "N/A";

                out << "=== Plasma Theme Master Status ===\n";
                out << "Global Theme: " << ThemeReader::currentGlobalTheme() << "\n";
                out << "Kvantum Theme: " << ThemeReader::currentKvantumTheme() << "\n";
                out << "GTK Theme: " << ThemeReader::currentGtkTheme() << "\n";
                out << "Auto LookAndFeel: " << (ThemeReader::isAutoLookAndFeel() ? "True" : "False") << "\n";
                out << "\n[Defaults]\n";
                out << "Global Dark: " << ThemeReader::defaultDarkTheme() << "\n";
                out << "Global Light: " << ThemeReader::defaultLightTheme() << "\n";
                out << "Kvantum Dark: " << ThemeReader::nightKvantumTheme() << "\n";
                out << "Kvantum Light: " << ThemeReader::dayKvantumTheme() << "\n";
                out << "GTK Dark: " << ThemeReader::nightGtkTheme() << "\n";
                out << "GTK Light: " << ThemeReader::dayGtkTheme() << "\n";
                out << "\n[Solar]\n";
                out << "Location: " << lat << ", " << lon << "\n";
                out << "Sunrise: " << sunriseStr << "\n";
                out << "Sunset: " << sunsetStr << "\n";
                out << "Offset (Padding): " << offset << " minutes\n";
                out << "Day Start: " << dayStartStr << "\n";
                out << "Night Start: " << dayEndStr << "\n";
                
                return 0;
            } 
            else if (command == "set-offset") {
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
            }
            else if (command == "set-auto") {
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
                        std::cout << "Applying calculated Global (" << (isDay ? "Day" : "Night") << "): " << qPrintable(targetGlobal) << "\n";
                        ThemeWriter::applyGlobalTheme(targetGlobal);
                        // Kvantum might be reset by Global Theme, so always apply it AFTER
                    }
                    
                    if (!targetKvantum.isEmpty()) {
                         std::cout << "Applying calculated Kvantum (" << (isDay ? "Day" : "Night") << "): " << qPrintable(targetKvantum) << "\n";
                         ThemeWriter::setKvantumTheme(targetKvantum);
                    }

                    QString targetGtk = isDay ? ThemeReader::dayGtkTheme() : ThemeReader::nightGtkTheme();
                    if (!targetGtk.isEmpty()) {
                        std::cout << "Applying calculated GTK (" << (isDay ? "Day" : "Night") << "): " << qPrintable(targetGtk) << "\n";
                        ThemeWriter::setGtkTheme(targetGtk);
                    }
                    
                    // Enable Auto AFTER applying themes, as applyGlobalTheme might reset it.
                    ThemeWriter::setAutoLookAndFeel(true);
                    std::cout << "AutomaticLookAndFeel set to: True\n";
                } else {
                    ThemeWriter::setAutoLookAndFeel(false);
                    std::cout << "AutomaticLookAndFeel set to: False\n";
                }

                return 0;
            }
            else if (command == "set-kvantum") {
                if (args.size() < 2) {
                    std::cerr << "Error: set-kvantum requires a theme name.\n";
                    return 1;
                }
                // ... (Existing logic for set-kvantum, but can simplify by just using ThemeWriter::setKvantumTheme directly if we trust user, or keep validation)
                // Let's keep validation logic but condensed.
                QString theme = args.at(1);
                 // Verify existence
                QStringList themes = ThemeReader::listKvantumThemes();
                QString targetTheme = theme;
                bool found = false;
                for(const QString &t : themes) {
                     if (t.compare(theme, Qt::CaseInsensitive) == 0) {
                         targetTheme = t;
                         found = true; 
                         break;
                     }
                }
                if (!found && themes.contains(theme)) found = true; // Exact match fallback

                if (!found) {
                    std::cerr << "Error: Theme '" << qPrintable(theme) << "' not found.\n";
                    std::cerr << "Available themes:\n";
                    for(const QString &t : themes) std::cerr << "  " << qPrintable(t) << "\n";
                    return 1;
                }
                
                ThemeWriter::setKvantumTheme(targetTheme);
                return 0;
            }
            else if (command == "set-kvantum-day") {
                 if (args.size() < 2) {
                     std::cerr << "Error: " << qPrintable(command) << " requires a theme name.\n";
                     return 1; 
                 }
                 ThemeWriter::setDayKvantumTheme(args.at(1));
                 Logger::log("Set DayKvantumTheme to \"" + args.at(1) + "\"", Logger::Info);
                 return 0;
            }
            else if (command == "set-kvantum-night") {
                 if (args.size() < 2) {
                     std::cerr << "Error: " << qPrintable(command) << " requires a theme name.\n";
                     return 1; 
                 }
                 ThemeWriter::setNightKvantumTheme(args.at(1));
                 Logger::log("Set NightKvantumTheme to \"" + args.at(1) + "\"", Logger::Info);
                 return 0;
            }
            else if (command == "set-gtk") {
                if (args.size() < 2) {
                    std::cerr << "Error: set-gtk requires a theme name.\n";
                    return 1;
                }
                QString theme = args.at(1);
                 // Verify existence
                QStringList themes = ThemeReader::listGtkThemes();
                QString targetTheme = theme;
                bool found = false;
                for(const QString &t : themes) {
                     if (t.compare(theme, Qt::CaseInsensitive) == 0) {
                         targetTheme = t;
                         found = true; 
                         break;
                     }
                }
                if (!found && themes.contains(theme)) found = true; // Exact match fallback

                if (!found) {
                    std::cerr << "Error: Theme '" << qPrintable(theme) << "' not found.\n";
                    std::cerr << "Available themes:\n";
                    for(const QString &t : themes) std::cerr << "  " << qPrintable(t) << "\n";
                    return 1;
                }
                
                ThemeWriter::setGtkTheme(targetTheme);
                return 0;
            }
            else if (command == "set-gtk-day") {
                if (args.size() < 2) {
                    std::cerr << "Error: " << qPrintable(command) << " requires a theme name.\n";
                    return 1;
                }
                QString value = args.at(1);
                ThemeWriter::setDayGtkTheme(value);
                Logger::log("Set DayGtkTheme to \"" + value + "\"", Logger::Info);
                return 0;
            }
            else if (command == "set-gtk-night") {
                if (args.size() < 2) {
                    std::cerr << "Error: " << qPrintable(command) << " requires a theme name.\n";
                    return 1;
                }
                QString value = args.at(1);
                ThemeWriter::setNightGtkTheme(value);
                Logger::log("Set NightGtkTheme to \"" + value + "\"", Logger::Info);
                return 0;
            }
            else if (command == "set-global-dark") {
                if (args.size() < 2) return 1;
                ThemeWriter::setDefaultDarkTheme(args.at(1));
                return 0;
            }
            else if (command == "set-global-light") {
                if (args.size() < 2) return 1;
                ThemeWriter::setDefaultLightTheme(args.at(1));
                return 0;
            }
            else if (command == "set-static-dark") {
                QString global = ThemeReader::defaultDarkTheme();
                QString kvantum = ThemeReader::nightKvantumTheme();
                
                if (global.isEmpty()) std::cerr << "Warning: No Default Dark Global Theme.\n";
                
                ThemeWriter::setAutoLookAndFeel(false);
                if (!global.isEmpty()) ThemeWriter::applyGlobalTheme(global);
                if (!kvantum.isEmpty()) {
                    std::cout << "Applying Default Dark Kvantum: " << qPrintable(kvantum) << "\n";
                    ThemeWriter::setKvantumTheme(kvantum);
                }
                QString gtk = ThemeReader::nightGtkTheme();
                if (!gtk.isEmpty()) {
                    std::cout << "Applying Default Dark GTK: " << qPrintable(gtk) << "\n";
                    ThemeWriter::setGtkTheme(gtk);
                }
                return 0;
            }
            else if (command == "set-static-light") {
                QString global = ThemeReader::defaultLightTheme();
                QString kvantum = ThemeReader::dayKvantumTheme();
                
                if (global.isEmpty()) std::cerr << "Warning: No Default Light Global Theme.\n";
                
                ThemeWriter::setAutoLookAndFeel(false);
                if (!global.isEmpty()) ThemeWriter::applyGlobalTheme(global);
                if (!kvantum.isEmpty()) {
                    std::cout << "Applying Default Light Kvantum: " << qPrintable(kvantum) << "\n";
                    ThemeWriter::setKvantumTheme(kvantum);
                }
                QString gtk = ThemeReader::dayGtkTheme();
                if (!gtk.isEmpty()) {
                    std::cout << "Applying Default Light GTK: " << qPrintable(gtk) << "\n";
                    ThemeWriter::setGtkTheme(gtk);
                }
                return 0;
            }
            else if (command == "log") {
                int lines = 500;
                bool errorsOnly = false;
                
                const QStringList rawArgs = QCoreApplication::arguments();
                
                // Simple manual parsing for log sub-args
                // rawArgs[0] is executable, rawArgs[1] is "log" (usually)
                // We scan starting from index 2 to find flags
                for (int i = 2; i < rawArgs.size(); ++i) {
                    if (rawArgs[i] == "-n" && i + 1 < rawArgs.size()) {
                        lines = rawArgs[i+1].toInt();
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
            }
            else if (command == "daemon") {
                std::cout << "Starting Plasma Theme Master Daemon...\n";
                Logger::log("Daemon started", Logger::Info);
                
                while (true) {
                    // 1. Check if Auto is enabled
                    if (ThemeReader::isAutoLookAndFeel()) {
                        double lat = ThemeReader::nativeLatitude();
                        double lon = ThemeReader::nativeLongitude();
                        int offset = ThemeReader::solarPadding();
                        
                        bool isDay = Solar::isDaytime(lat, lon, offset);
                        
                        // Check CURRENT theme vs TARGET theme
                        // Optimization: Only apply if different?
                        // ThemeWriter functions generally just write config, but applyGlobal calls `lookandfeeltool`.
                        // We should avoid calling `lookandfeeltool` repeatedly if already set.
                        
                        // But verifying "Current" global theme is hard because `currentGlobalTheme()` reads from config,
                        // which might already be set even if visual didn't change (rare).
                        // However, if the user changed the theme via System Settings, the config usually updates.
                        // So checking `ThemeReader::currentGlobalTheme()` is robust enough.
                        
                        QString currentGlobal = ThemeReader::currentGlobalTheme();
                        QString targetGlobal = isDay ? ThemeReader::defaultLightTheme() : ThemeReader::defaultDarkTheme();
                        
                        bool needUpdate = false;
                        
                        // Strict check: If target is defined and different, apply.
                        if (!targetGlobal.isEmpty() && currentGlobal != targetGlobal) {
                             Logger::log("Daemon: Global theme mismatch detected. Applying " + targetGlobal, Logger::Info);
                             ThemeWriter::applyGlobalTheme(targetGlobal);
                             needUpdate = true;
                        }

                        // Kvantum
                        QString currentKvantum = ThemeReader::currentKvantumTheme();
                        QString targetKvantum = isDay ? ThemeReader::dayKvantumTheme() : ThemeReader::nightKvantumTheme();
                         if (!targetKvantum.isEmpty() && currentKvantum != targetKvantum) {
                             Logger::log("Daemon: Kvantum theme mismatch detected. Applying " + targetKvantum, Logger::Info);
                             ThemeWriter::setKvantumTheme(targetKvantum);
                             needUpdate = true;
                        }

                        // GTK
                        QString currentGtk = ThemeReader::currentGtkTheme();
                        QString targetGtk = isDay ? ThemeReader::dayGtkTheme() : ThemeReader::nightGtkTheme();
                         if (!targetGtk.isEmpty() && currentGtk != targetGtk) {
                             Logger::log("Daemon: GTK theme mismatch detected. Applying " + targetGtk, Logger::Info);
                             ThemeWriter::setGtkTheme(targetGtk);
                             needUpdate = true;
                        }
                        
                        // If we updated anything, ensure Auto flag remains True (it might technically not change but good to be sure)
                        if (needUpdate) {
                             ThemeWriter::setAutoLookAndFeel(true);
                        }
                    }
                    
                    // Sleep for 60 seconds
                    QThread::sleep(60); 
                }
                return 0;
            }
            else if (command == "uninstall") {
                std::cout << "Launching Uninstaller...\n";
                // Execute the installed uninstall script
                // using system() to ensure it inherits terminal for interactivity
                int ret = std::system("plasma-theme-master-uninstall");
                return WEXITSTATUS(ret);
            }
            else if (command == "clone-global") {
                if (args.size() < 2) {
                    std::cout << "Usage: plasma-theme-master clone-global <source> <dest>\n";
                    return 1;
                }
                QString src = args.at(0);
                QString dest = args.at(1);
                
                std::cout << "Cloning global theme '" << qPrintable(src) << "' to '" << qPrintable(dest) << "'...\n";
                if (GlobalThemeManager::cloneTheme(src, dest)) {
                    std::cout << "Success! Cloned to " << qPrintable(dest) << "\n";
                    return 0;
                } else {
                    std::cerr << "Failed to clone theme. Check logs for details.\n";
                    return 1;
                }
            }
            else {
                std::cerr << "Unknown command: " << qPrintable(command) << "\n";
                printHelp();
                return 1;
            }
        }
        // If no args but started in CLI mode (e.g. via check failure?), show help
        printHelp();
        return 0; 
    }

    // GUI Mode
    if (auto guiApp = qobject_cast<QApplication*>(app.data())) {
        MainWindow w;
        w.show();
        return guiApp->exec();
    }

    return 1;
}
