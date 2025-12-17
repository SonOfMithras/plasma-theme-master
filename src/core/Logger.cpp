#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

QString Logger::logFilePath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path + "/plasma-theme-master.log";
}

void Logger::log(const QString &message, Level level) {
    QString path = logFilePath();
    QFile file(path);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        out << "[" << timestamp << "] [" << levelToString(level) << "] " << message << "\n";
        file.close();
    } else {
        qWarning() << "Failed to write to log file:" << path;
    }
}

QStringList Logger::readLogs(int maxLines, bool onlyErrors) {
    QString path = logFilePath();
    QFile file(path);
    QStringList result;
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QStringList allLines;
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (onlyErrors && !line.contains("[ERROR]")) continue;
            allLines.append(line);
        }
        
        // Return only the last maxLines
        int start = qMax(0, allLines.size() - maxLines);
        for (int i = start; i < allLines.size(); ++i) {
            result.append(allLines.at(i));
        }
        file.close();
    }
    return result;
}

void Logger::clear() {
    QString path = logFilePath();
    QFile::remove(path);
}

QString Logger::levelToString(Level level) {
    switch (level) {
        case Info: return "INFO";
        case Warning: return "WARNING";
        case Error: return "ERROR";
        default: return "UNKNOWN";
    }
}
