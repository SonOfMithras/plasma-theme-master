#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QList>

class Logger {
public:
    enum Level {
        Info,
        Warning,
        Error
    };

    static void log(const QString &message, Level level = Info);
    static QStringList readLogs(int maxLines = 100, bool onlyErrors = false);
    static void clear();
    static QString logFilePath();

private:
   static QString levelToString(Level level);
};

#endif // LOGGER_H
