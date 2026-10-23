#include "HookRunner.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QThread>

HookRunner::HookRunner(int maxConcurrent, QObject *parent)
    : QObject(parent), m_maxConcurrent(qMax(1, maxConcurrent)) {}

HookRunner::~HookRunner() {
    cancelAll();
}

void HookRunner::enqueue(const QString &command,
                         quint64 generation,
                         const QString &workingDir,
                         const QString &label) {
    if (command.trimmed().isEmpty()) return;
    if (generation < m_currentGeneration) return;

    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, [this, command, generation, workingDir, label]() {
            enqueue(command, generation, workingDir, label);
        }, Qt::QueuedConnection);
        return;
    }

    // Deduplicate: Don't enqueue identical commands in the same generation
    for (const HookTask &existing : m_queue) {
        if (existing.generation == generation && existing.command == command) {
            return;
        }
    }
    for (const RunningProcess &rp : m_running) {
        if (rp.task.generation == generation && rp.task.command == command) {
            return;
        }
    }

    HookTask task;
    task.command = command;
    task.workingDir = workingDir;
    task.generation = generation;
    task.label = label.isEmpty() ? command : label;

    m_queue.enqueue(task);
    pump();
}

void HookRunner::invalidateBefore(quint64 generation) {
    m_currentGeneration = qMax(m_currentGeneration, generation);

    // Prune older queued tasks
    QQueue<HookTask> validQueue;
    while (!m_queue.isEmpty()) {
        HookTask t = m_queue.dequeue();
        if (t.generation >= m_currentGeneration) {
            validQueue.enqueue(t);
        }
    }
    m_queue = validQueue;
}

void HookRunner::cancelAll() {
    m_queue.clear();
    for (auto &rp : m_running) {
        if (rp.process) {
            rp.process->disconnect();
            rp.process->terminate();
            if (!rp.process->waitForFinished(500)) {
                rp.process->kill();
            }
            rp.process->deleteLater();
        }
    }
    m_running.clear();
}

bool HookRunner::isIdle() const {
    return m_running.isEmpty() && m_queue.isEmpty();
}

int HookRunner::pendingCount() const {
    return m_running.size() + m_queue.size();
}

void HookRunner::pump() {
    while (m_running.size() < m_maxConcurrent && !m_queue.isEmpty()) {
        HookTask task = m_queue.dequeue();
        if (task.generation < m_currentGeneration) {
            continue; // superseded
        }

        auto *proc = new QProcess(this);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString appDir = QCoreApplication::applicationDirPath();
        QString path = env.value("PATH");
        env.insert("PATH", appDir + "/helpers:" + appDir + "/../helpers:" + appDir + ":" + path);
        proc->setProcessEnvironment(env);

        if (!task.workingDir.isEmpty()) {
            proc->setWorkingDirectory(task.workingDir);
        }

        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &HookRunner::onProcessFinished);
        connect(proc, &QProcess::errorOccurred,
                this, &HookRunner::onProcessError);

        RunningProcess rp;
        rp.process = proc;
        rp.task = task;
        m_running.append(rp);

        Logger::log(QString("HookRunner [%1 concurrent]: Running hook for [%2]")
                        .arg(m_running.size())
                        .arg(task.label),
                    Logger::Info);

        emit hookStarted(task.label, task.generation);
        proc->start("/bin/sh", {"-c", task.command});
    }

    if (m_running.isEmpty() && m_queue.isEmpty()) {
        emit allHooksFinished(m_currentGeneration);
    }
}

void HookRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    auto *proc = qobject_cast<QProcess *>(sender());
    if (!proc) return;

    int idx = -1;
    for (int i = 0; i < m_running.size(); ++i) {
        if (m_running[i].process == proc) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        HookTask task = m_running[idx].task;
        m_running.removeAt(idx);

        bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
        if (!success) {
            QString err = QString::fromUtf8(proc->readAllStandardError()).trimmed();
            Logger::log(QString("HookRunner: Hook [%1] exited with code %2%3")
                            .arg(task.label)
                            .arg(exitCode)
                            .arg(err.isEmpty() ? QString() : " - stderr: " + err),
                        Logger::Warning);
        } else {
            Logger::log(QString("HookRunner: Completed hook for [%1]").arg(task.label),
                        Logger::Info);
        }

        emit hookCompleted(task.label, task.generation, success, exitCode);
    }

    proc->deleteLater();
    pump();
}

void HookRunner::onProcessError(QProcess::ProcessError error) {
    auto *proc = qobject_cast<QProcess *>(sender());
    if (!proc) return;

    Logger::log(QString("HookRunner: Process error occurred: %1").arg(error),
                Logger::Warning);
}
