#ifndef CLIHANDLER_H
#define CLIHANDLER_H

#include <QString>
#include <QStringList>

class CLIHandler {
public:
    static int handleCommand(const QString &command, const QStringList &args);
    static void printHelp();
};

#endif // CLIHANDLER_H
