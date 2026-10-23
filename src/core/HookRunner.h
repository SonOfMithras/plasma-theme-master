#ifndef HOOKRUNNER_H
#define HOOKRUNNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QQueue>

struct HookTask {
    QString command;
    QString workingDir;
    quint64 generation = 0;
    QString label;
};

class HookRunner : public QObject {
    Q_OBJECT
public:
    explicit HookRunner(int maxConcurrent = 4, QObject *parent = nullptr);
    ~HookRunner() override;

    void enqueue(const QString &command,
                 quint64 generation,
                 const QString &workingDir = QString(),
                 const QString &label = QString());

    void invalidateBefore(quint64 generation);
    void cancelAll();
    bool isIdle() const;
    int pendingCount() const;

signals:
    void hookStarted(const QString &label, quint64 generation);
    void hookCompleted(const QString &label, quint64 generation, bool success, int exitCode);
    void allHooksFinished(quint64 generation);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void pump();

    int m_maxConcurrent;
    quint64 m_currentGeneration = 0;
    QQueue<HookTask> m_queue;

    struct RunningProcess {
        QProcess *process = nullptr;
        HookTask task;
    };
    QList<RunningProcess> m_running;
};

#endif // HOOKRUNNER_H
