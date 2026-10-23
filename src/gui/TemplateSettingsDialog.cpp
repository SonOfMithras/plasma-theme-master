#include "TemplateSettingsDialog.h"
#include "../core/TemplateConfig.h"
#include "../core/UniversalThemeExporter.h"
#include "../core/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QIcon>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QProcess>

TemplateSettingsDialog::TemplateSettingsDialog(const QString &appId, QWidget *parent)
    : QDialog(parent), m_appId(appId) {
    m_app = CommunityTemplateCatalog::instance().findApp(m_appId);

    QString title = m_app ? m_app->name : m_appId;
    setWindowTitle(tr("%1 Settings").arg(title));
    resize(580, 480);

    setupUi();
}

TemplateSettingsDialog::~TemplateSettingsDialog() = default;

void TemplateSettingsDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    // Header with Icon & Name
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(this);
    QString iconName = m_app ? m_app->categoryIconName() : "applications-other";
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull()) {
        iconLabel->setPixmap(icon.pixmap(32, 32));
    }
    headerLayout->addWidget(iconLabel);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    QLabel *titleLabel = new QLabel(QString("<h3>%1</h3>").arg(m_app ? m_app->name : m_appId), this);
    QLabel *subLabel = new QLabel(QString("Category: <i>%1</i>").arg(m_app ? m_app->resolvedCategoryName() : "General"), this);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subLabel);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // App-specific customization widget
    if (m_appId == "obsidian") {
        setupObsidianUi(this);
    } else if (m_appId == "discord" || m_appId == "betterdiscord" || m_appId == "vencord") {
        setupDiscordUi(this);
    } else if (m_appId == "zed") {
        setupZedUi(this);
    } else {
        setupGenericUi(this);
    }

    // Bottom action buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_restoreBtn = new QPushButton(tr("Restore Original Config"), this);
    m_restoreBtn->setIcon(QIcon::fromTheme("edit-undo"));
    m_restoreBtn->setToolTip(tr("Restore backed up configuration files if available"));
    connect(m_restoreBtn, &QPushButton::clicked, this, &TemplateSettingsDialog::onRestoreBackup);
    actionLayout->addWidget(m_restoreBtn);
    actionLayout->addStretch();

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btnBox, &QDialogButtonBox::accepted, this, &TemplateSettingsDialog::saveAndAccept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    actionLayout->addWidget(btnBox);

    mainLayout->addLayout(actionLayout);
}

void TemplateSettingsDialog::setupObsidianUi(QWidget *container) {
    auto *layout = qobject_cast<QVBoxLayout *>(container->layout());
    if (!layout) return;

    QTabWidget *tabs = new QTabWidget(this);

    // Tab 1: Vault Management
    QWidget *vaultTab = new QWidget(this);
    QVBoxLayout *vaultLayout = new QVBoxLayout(vaultTab);

    m_obsidianAutoDetectCheck = new QCheckBox(tr("Automatically detect local Obsidian vaults"), vaultTab);
    m_obsidianAutoDetectCheck->setChecked(TemplateConfig::isObsidianAutoDetectEnabled());
    vaultLayout->addWidget(m_obsidianAutoDetectCheck);

    vaultLayout->addWidget(new QLabel(tr("<b>Configured Vault Directories:</b>"), vaultTab));
    m_vaultList = new QListWidget(vaultTab);
    QStringList vaults = TemplateConfig::getObsidianVaults();
    for (const QString &v : vaults) {
        m_vaultList->addItem(v);
    }
    vaultLayout->addWidget(m_vaultList);

    QHBoxLayout *vaultBtns = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton(tr("Add Vault..."), vaultTab);
    addBtn->setIcon(QIcon::fromTheme("list-add"));
    QPushButton *removeBtn = new QPushButton(tr("Remove Selected"), vaultTab);
    removeBtn->setIcon(QIcon::fromTheme("list-remove"));
    QPushButton *detectBtn = new QPushButton(tr("Detect Vaults Now"), vaultTab);
    detectBtn->setIcon(QIcon::fromTheme("system-search"));

    connect(addBtn, &QPushButton::clicked, this, &TemplateSettingsDialog::onAddVault);
    connect(removeBtn, &QPushButton::clicked, this, &TemplateSettingsDialog::onRemoveVault);
    connect(detectBtn, &QPushButton::clicked, this, &TemplateSettingsDialog::onAutoDetectVaults);

    vaultBtns->addWidget(addBtn);
    vaultBtns->addWidget(removeBtn);
    vaultBtns->addWidget(detectBtn);
    vaultBtns->addStretch();
    vaultLayout->addLayout(vaultBtns);

    QLabel *info = new QLabel(
        tr("<small>Sync will generate <code>ptm.css</code> inside <code>.obsidian/snippets/</code> for each configured vault.</small>"),
        vaultTab);
    info->setWordWrap(true);
    vaultLayout->addWidget(info);

    tabs->addTab(vaultTab, tr("Vaults"));

    // Tab 2: Instructions / Readme
    if (m_app && !m_app->readmePath.isEmpty() && QFile::exists(m_app->readmePath)) {
        QWidget *readmeTab = new QWidget(this);
        QVBoxLayout *readmeLayout = new QVBoxLayout(readmeTab);
        m_readmeBrowser = new QTextBrowser(readmeTab);
        QFile rf(m_app->readmePath);
        if (rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_readmeBrowser->setMarkdown(QString::fromUtf8(rf.readAll()));
            rf.close();
        }
        readmeLayout->addWidget(m_readmeBrowser);
        tabs->addTab(readmeTab, tr("Instructions"));
    }

    layout->addWidget(tabs);
}

void TemplateSettingsDialog::setupDiscordUi(QWidget *container) {
    auto *layout = qobject_cast<QVBoxLayout *>(container->layout());
    if (!layout) return;

    QTabWidget *tabs = new QTabWidget(this);

    QWidget *configTab = new QWidget(this);
    QVBoxLayout *tabLayout = new QVBoxLayout(configTab);

    m_discordMidnightCheck = new QCheckBox(
        tr("Use Midnight Theme CSS (Recommended for cohesive dark/light colors)"), configTab);
    m_discordMidnightCheck->setChecked(
        TemplateConfig::getValue("betterdiscord", "midnight_enabled", "true") == "true");
    tabLayout->addWidget(m_discordMidnightCheck);

    tabLayout->addWidget(new QLabel(tr("<b>Custom Import URLs (one per line):</b>"), configTab));
    m_discordCustomImportsEdit = new QTextEdit(configTab);
    m_discordCustomImportsEdit->setText(
        TemplateConfig::getList("betterdiscord", "custom_imports").join("\n"));
    tabLayout->addWidget(m_discordCustomImportsEdit);

    tabs->addTab(configTab, tr("Configuration"));

    // Tab 2: Readme / Targets
    if (m_app && !m_app->readmePath.isEmpty() && QFile::exists(m_app->readmePath)) {
        QWidget *readmeTab = new QWidget(this);
        QVBoxLayout *readmeLayout = new QVBoxLayout(readmeTab);
        m_readmeBrowser = new QTextBrowser(readmeTab);
        QFile rf(m_app->readmePath);
        if (rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_readmeBrowser->setMarkdown(QString::fromUtf8(rf.readAll()));
            rf.close();
        }
        readmeLayout->addWidget(m_readmeBrowser);
        tabs->addTab(readmeTab, tr("Supported Clients"));
    }

    layout->addWidget(tabs);
}


void TemplateSettingsDialog::setupZedUi(QWidget *container) {
    auto *layout = qobject_cast<QVBoxLayout *>(container->layout());
    if (!layout) return;

    QTabWidget *tabs = new QTabWidget(this);

    QWidget *configTab = new QWidget(this);
    QVBoxLayout *tabLayout = new QVBoxLayout(configTab);
    tabLayout->setSpacing(12);

    QLabel *infoLabel = new QLabel(
        tr("<b>Theme Variant:</b><br/>"
           "Choose whether Zed Editor should use the standard opaque theme or the translucent/transparent theme variant."),
        configTab);
    infoLabel->setWordWrap(true);
    tabLayout->addWidget(infoLabel);

    m_zedOpaqueRadio = new QRadioButton(tr("Opaque (PTM Dark / PTM Light)"), configTab);
    m_zedOpaqueRadio->setToolTip(tr("Solid, opaque background matching your system palette."));
    tabLayout->addWidget(m_zedOpaqueRadio);

    m_zedTransparentRadio = new QRadioButton(tr("Transparent (PTM Dark Transparent / PTM Light Transparent)"), configTab);
    m_zedTransparentRadio->setToolTip(tr("Translucent background designed for window blur or transparency."));
    tabLayout->addWidget(m_zedTransparentRadio);

    QString currentOpacity = TemplateConfig::getValue("zed", "opacity", "opaque");
    if (currentOpacity == "transparent") {
        m_zedTransparentRadio->setChecked(true);
    } else {
        m_zedOpaqueRadio->setChecked(true);
    }

    tabLayout->addStretch();
    tabs->addTab(configTab, tr("Options"));

    if (m_app && !m_app->readmePath.isEmpty() && QFile::exists(m_app->readmePath)) {
        QWidget *readmeTab = new QWidget(this);
        QVBoxLayout *readmeLayout = new QVBoxLayout(readmeTab);
        m_readmeBrowser = new QTextBrowser(readmeTab);
        QFile rf(m_app->readmePath);
        if (rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_readmeBrowser->setMarkdown(QString::fromUtf8(rf.readAll()));
            rf.close();
        }
        readmeLayout->addWidget(m_readmeBrowser);
        tabs->addTab(readmeTab, tr("Instructions"));
    }

    layout->addWidget(tabs);
}

void TemplateSettingsDialog::setupGenericUi(QWidget *container) {
    auto *layout = qobject_cast<QVBoxLayout *>(container->layout());
    if (!layout) return;

    if (m_app && !m_app->readmePath.isEmpty() && QFile::exists(m_app->readmePath)) {
        m_readmeBrowser = new QTextBrowser(this);
        QFile rf(m_app->readmePath);
        if (rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_readmeBrowser->setMarkdown(QString::fromUtf8(rf.readAll()));
            rf.close();
        }
        layout->addWidget(m_readmeBrowser);
    } else {
        // Fallback info
        QLabel *noReadme = new QLabel(
            tr("<p>No extended documentation provided for this template.</p>"
               "<p>When enabled, files will be rendered automatically to their standard application locations.</p>"),
            this);
        noReadme->setWordWrap(true);
        layout->addWidget(noReadme);
    }
}

void TemplateSettingsDialog::onAddVault() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Obsidian Vault Folder"), QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        // Prevent duplicates
        for (int i = 0; i < m_vaultList->count(); ++i) {
            if (m_vaultList->item(i)->text() == dir) return;
        }
        m_vaultList->addItem(dir);
    }
}

void TemplateSettingsDialog::onRemoveVault() {
    int row = m_vaultList->currentRow();
    if (row >= 0) {
        delete m_vaultList->takeItem(row);
    }
}

void TemplateSettingsDialog::onAutoDetectVaults() {
    QStringList found;

    // 1. Inspect Obsidian's obsidian.json config
    QStringList cfgPaths = {
        QDir::homePath() + "/.config/obsidian/obsidian.json",
        QDir::homePath() + "/.var/app/md.obsidian.Obsidian/config/obsidian/obsidian.json"
    };

    for (const QString &cfgPath : cfgPaths) {
        QFile f(cfgPath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            f.close();
            if (doc.isObject()) {
                QJsonObject vaultsObj = doc.object().value("vaults").toObject();
                for (auto it = vaultsObj.begin(); it != vaultsObj.end(); ++it) {
                    if (it.value().isObject()) {
                        QString p = it.value().toObject().value("path").toString();
                        if (!p.isEmpty() && QDir(p).exists()) {
                            found << p;
                        }
                    }
                }
            }
        }
    }

    // 2. If dynamic output resolution is available from the catalog app
    if (m_app) {
        for (const auto &entry : m_app->entries) {
            if (!entry.outputPathDynamic.isEmpty()) {
                QStringList dynamicOutputs = CommunityTemplateCatalog::resolveDynamicOutputPaths(
                    entry.outputPathDynamic, m_app->templateDir);
                for (const QString &out : dynamicOutputs) {
                    // Extract vault path from snippet path: <vault>/.obsidian/snippets/noctalia.css
                    int idx = out.indexOf("/.obsidian");
                    if (idx > 0) {
                        QString vaultPath = out.left(idx);
                        if (!found.contains(vaultPath)) found << vaultPath;
                    }
                }
            }
        }
    }

    int added = 0;
    for (const QString &v : found) {
        bool exists = false;
        for (int i = 0; i < m_vaultList->count(); ++i) {
            if (m_vaultList->item(i)->text() == v) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_vaultList->addItem(v);
            added++;
        }
    }

    QMessageBox::information(this, tr("Auto-detection Complete"),
                             tr("Found and added %1 new Obsidian vault(s).").arg(added));
}

void TemplateSettingsDialog::onRestoreBackup() {
    QString appName = m_app ? m_app->name : m_appId;
    if (QMessageBox::question(this, tr("Restore Config"),
                             tr("Restore original configuration for %1 from backup?").arg(appName))
        != QMessageBox::Yes) {
        return;
    }

    bool restoredAny = false;

    // Check community app entries
    if (m_app) {
        for (const auto &e : m_app->entries) {
            for (const QString &out : e.outputPaths) {
                QString p = CommunityTemplateCatalog::expandPath(out);
                if (UniversalThemeExporter::restoreFile(p)) restoredAny = true;
            }
        }
    }

    // Check built-in entries
    QList<TemplateEntry> builtins = TemplateConfig::loadTemplates();
    for (const TemplateEntry &e : builtins) {
        if (e.name == m_appId || e.name.startsWith(m_appId + "_")) {
            if (!e.outputPath.isEmpty() && UniversalThemeExporter::restoreFile(e.outputPath)) {
                restoredAny = true;
            }
        }
    }

    if (restoredAny) {
        QMessageBox::information(this, tr("Restored"),
                                 tr("Configuration for %1 was restored successfully.").arg(appName));
    } else {
        QMessageBox::warning(this, tr("No Backup Found"),
                             tr("No backup (.bak) files were found for %1.").arg(appName));
    }
}

void TemplateSettingsDialog::saveAndAccept() {
    if (m_appId == "obsidian") {
        QStringList vaults;
        for (int i = 0; i < m_vaultList->count(); ++i) {
            vaults << m_vaultList->item(i)->text().trimmed();
        }
        vaults.removeDuplicates();
        TemplateConfig::setObsidianVaults(vaults);
        TemplateConfig::setObsidianAutoDetect(m_obsidianAutoDetectCheck->isChecked());
    } else if (m_appId == "discord" || m_appId == "betterdiscord" || m_appId == "vencord") {
        QString midnight = m_discordMidnightCheck->isChecked() ? "true" : "false";
        TemplateConfig::setValue("betterdiscord", "midnight_enabled", midnight);
        TemplateConfig::setValue("vencord", "midnight_enabled", midnight);

        QStringList imports;
        for (const QString &line : m_discordCustomImportsEdit->toPlainText().split('\n')) {
            if (!line.trimmed().isEmpty()) imports << line.trimmed();
        }
        imports.removeDuplicates();
        TemplateConfig::setList("betterdiscord", "custom_imports", imports);
        TemplateConfig::setList("vencord", "custom_imports", imports);
    } else if (m_appId == "zed") {
        QString opacity = (m_zedTransparentRadio && m_zedTransparentRadio->isChecked()) ? "transparent" : "opaque";
        TemplateConfig::setValue("zed", "opacity", opacity);
        if (TemplateConfig::isAppEnabled("zed")) {
            QProcess::startDetached("plasma-theme-master-helper-zed", QStringList() << (opacity == "transparent" ? "--transparent" : "--opaque"));
        }
    }

    accept();
}
