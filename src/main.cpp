#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>
#include <iostream>
#include <QThread>
#include "core/CLIHandler.h"
#include "gui/MainWindow.h"


// ... (Top includes remain same, adding QDateTime/QTimeZone if needed, but they are in Solar.h)


int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("plasma-theme-master");
    QCoreApplication::setApplicationVersion("1.0.5");

    // Manual check for GUI vs CLI mode
    bool guiMode = true;
    // If any arguments are provided, assume CLI mode
    if (argc > 1) {
        guiMode = false;
    }

    QScopedPointer<QCoreApplication> app;
    if (guiMode) {
        app.reset(new QApplication(argc, argv));
    } else {
        app.reset(new QCoreApplication(argc, argv));
    }

    app->setApplicationName("plasma-theme-master");
    app->setApplicationVersion("1.0.5");

    QCommandLineParser parser;
    parser.setApplicationDescription("KDE Plasma Theme Master");
    
    // Manual help option to override default formatting
    auto helpOption = QCommandLineOption(QStringList() << "h" << "help", "Display this help.");
    parser.addOption(helpOption);
    parser.addVersionOption();
    
    parser.addPositionalArgument("command", "Command to execute");
    parser.addPositionalArgument("value", "Value for command");

    if (!guiMode) {
        parser.parse(QCoreApplication::arguments());
        
        if (parser.isSet(helpOption)) {
            CLIHandler::printHelp();
            return 0;
        }

        if (parser.isSet("version")) {
            parser.showVersion();
            return 0;
        }

        const QStringList args = parser.positionalArguments();
        
        if (!args.isEmpty()) {
            const QString command = args.first();
            // Pass the full positional argument list to the handler
            return CLIHandler::handleCommand(command, args);
        }
        
        // No args but not GUI mode? Show help.
        CLIHandler::printHelp();
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
