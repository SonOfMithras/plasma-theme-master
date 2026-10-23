#ifndef TEMPLATEAPPLYSERVICE_H
#define TEMPLATEAPPLYSERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QAtomicInteger>
#include "UniversalThemeExporter.h"
#include "HookRunner.h"
#include "CommunityTemplateCatalog.h"

class TemplateApplyService : public QObject {
    Q_OBJECT
public:
    static TemplateApplyService &instance();

    // Trigger asynchronous application of all enabled templates
    // paletteMode can be "current", "day", "night"
    quint64 applyAsync(const QString &paletteMode = "current");

    // Synchronous application (runs inline and waits for hooks, useful for CLI)
    bool applySync(const QString &paletteMode = "current", int timeoutMs = 15000);

    HookRunner *hookRunner() const { return m_hookRunner; }
    quint64 currentGeneration() const { return m_generation.loadRelaxed(); }
    bool isBusy() const;

signals:
    void syncStarted(quint64 generation);
    void syncProgress(quint64 generation, int current, int total, const QString &appName);
    void syncFinished(quint64 generation, bool success, const QStringList &warnings);

private slots:
    void onAllHooksFinished(quint64 generation);

private:
    explicit TemplateApplyService(QObject *parent = nullptr);
    ~TemplateApplyService() override;

    void executeApply(quint64 generation, const QString &paletteMode);
    QStringList resolveObsidianOutputPaths(const CommunityTemplateApp &app, const CommunityTemplateEntry &entry);

    QAtomicInteger<quint64> m_generation{0};
    HookRunner *m_hookRunner = nullptr;
    bool m_isApplying = false;
    quint64 m_activeGeneration = 0;
    QStringList m_warnings;
};

#endif // TEMPLATEAPPLYSERVICE_H
