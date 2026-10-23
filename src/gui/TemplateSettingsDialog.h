#ifndef TEMPLATESETTINGSDIALOG_H
#define TEMPLATESETTINGSDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include "../core/CommunityTemplateCatalog.h"

class QListWidget;
class QCheckBox;
class QTextEdit;
class QTextBrowser;
class QPushButton;

class TemplateSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit TemplateSettingsDialog(const QString &appId, QWidget *parent = nullptr);
    ~TemplateSettingsDialog() override;

private slots:
    void onAddVault();
    void onRemoveVault();
    void onAutoDetectVaults();
    void onRestoreBackup();
    void saveAndAccept();

private:
    void setupUi();
    void setupObsidianUi(QWidget *container);
    void setupDiscordUi(QWidget *container);
    void setupZedUi(QWidget *container);
    void setupGenericUi(QWidget *container);

    QString m_appId;
    const CommunityTemplateApp *m_app = nullptr;

    // Obsidian controls
    QCheckBox *m_obsidianAutoDetectCheck = nullptr;
    QListWidget *m_vaultList = nullptr;

    // Discord controls
    QCheckBox *m_discordMidnightCheck = nullptr;
    QTextEdit *m_discordCustomImportsEdit = nullptr;

    // Zed controls
    class QRadioButton *m_zedOpaqueRadio = nullptr;
    class QRadioButton *m_zedTransparentRadio = nullptr;

    // Generic info / readme
    QTextBrowser *m_readmeBrowser = nullptr;
    QPushButton *m_restoreBtn = nullptr;
};

#endif // TEMPLATESETTINGSDIALOG_H
