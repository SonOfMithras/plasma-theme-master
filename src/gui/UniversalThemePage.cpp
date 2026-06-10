#include "UniversalThemePage.h"
#include "../core/TemplateConfig.h"
#include "../core/UniversalThemeExporter.h"
#include "../core/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QScrollArea>
#include <QTimer>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QIcon>

UniversalThemePage::UniversalThemePage(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadSettings();
}

// ---------------------------------------------------------------------------
// Restore button factory — looks up output_path from config.toml at click time
// so it reflects any user edits to config without restarting the app.
// ---------------------------------------------------------------------------
QPushButton *UniversalThemePage::makeRestoreBtn(const QString &templateName,
                                                const QString &displayName) {
    auto *btn = new QPushButton(this);
    btn->setToolTip("Restore original config / remove sync files");
    btn->setIcon(QIcon::fromTheme("edit-undo"));
    if (btn->icon().isNull()) btn->setText("Restore");

    connect(btn, &QPushButton::clicked, this, [this, templateName, displayName]() {
        QString question = QString("Restore %1 config from backup?").arg(displayName);
        if (QMessageBox::question(this, "Restore " + displayName, question)
                != QMessageBox::Yes) return;

        // Find all template entries matching this name (covers kitty_light + kitty_dark)
        QList<TemplateEntry> entries = TemplateConfig::loadTemplates();
        bool any = false;
        for (const TemplateEntry &e : entries) {
            // Match exact name or prefixed name (e.g. "kitty" matches "kitty_light")
            if (e.name == templateName || e.name.startsWith(templateName + "_")) {
                if (!e.outputPath.isEmpty()) {
                    if (UniversalThemeExporter::restoreFile(e.outputPath)) {
                        Logger::log("Restored " + e.name + " from backup.", Logger::Info);
                        any = true;
                    }
                }
            }
        }

        if (any)
            QMessageBox::information(this, "Done",
                                     "Restored " + displayName + " config from backup.");
        else
            QMessageBox::warning(this, "No Backup Found",
                                 "No backup file found for " + displayName +
                                 ".\nOutput may not have been written yet.");
    });

    return btn;
}

void UniversalThemePage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QLabel *header = new QLabel("<h2>Universal Theming</h2>", this);
    mainLayout->addWidget(header);

    QLabel *desc = new QLabel(
        "Automatically export Plasma colors to other applications. "
        "Configuration is stored in <code>~/.config/plasma-theme-master/config.toml</code>.",
        this);
    desc->setWordWrap(true);
    mainLayout->addWidget(desc);

    // Apps Group
    QGroupBox *appsGroup = new QGroupBox(tr("Supported Applications"), this);
    QGridLayout *grid = new QGridLayout(appsGroup);
    grid->setColumnStretch(0, 1);

    int row = 0;

    // Helper: add a standard row (checkbox + optional config btn + restore btn)
    auto addRow = [&](QCheckBox *cb, QPushButton *restoreBtn,
                      QPushButton *configBtn = nullptr) {
        grid->addWidget(cb, row, 0);
        int col = 1;
        if (configBtn) grid->addWidget(configBtn, row, col++);
        grid->addWidget(restoreBtn, row, col);
        row++;
    };

    // VS Code
    m_vscodeCheck      = new QCheckBox("VS Code / VSCodium / Antigravity", this);
    m_vscodeRestoreBtn = makeRestoreBtn("vscode", "VS Code");
    addRow(m_vscodeCheck, m_vscodeRestoreBtn);

    // Firefox
    m_firefoxCheck      = new QCheckBox("Firefox / Zen Browser (userChrome.css)", this);
    m_firefoxRestoreBtn = makeRestoreBtn("firefox", "Firefox");
    addRow(m_firefoxCheck, m_firefoxRestoreBtn);

    // Zen Browser (Noctalia Community Template)
    m_zenBrowserCheck      = new QCheckBox("Zen Browser (Noctalia Community Template)", this);
    m_zenBrowserRestoreBtn = makeRestoreBtn("zen_browser", "Zen Browser");
    m_zenBrowserContentCheck = new QCheckBox("Theme settings and content pages (userContent.css)", this);
    m_zenBrowserContentCheck->setStyleSheet("margin-left: 20px;"); // Indent as sub-option
    
    // Wire dynamic enabling/disabling of the sub-option
    m_zenBrowserContentCheck->setEnabled(m_zenBrowserCheck->isChecked());
    connect(m_zenBrowserCheck, &QCheckBox::toggled, m_zenBrowserContentCheck, &QCheckBox::setEnabled);
    
    addRow(m_zenBrowserCheck, m_zenBrowserRestoreBtn);
    grid->addWidget(m_zenBrowserContentCheck, row++, 0);

    // BetterDiscord
    m_discordCheck      = new QCheckBox("BetterDiscord (Midnight Theme CSS)", this);
    m_discordRestoreBtn = makeRestoreBtn("betterdiscord", "BetterDiscord");
    m_discordConfigBtn  = new QPushButton("Configure", this);
    connect(m_discordConfigBtn, &QPushButton::clicked,
            this, &UniversalThemePage::showBDSettings);
    addRow(m_discordCheck, m_discordRestoreBtn, m_discordConfigBtn);

    // Vencord
    m_vencordCheck      = new QCheckBox("Vencord (Midnight Theme CSS)", this);
    m_vencordRestoreBtn = makeRestoreBtn("vencord", "Vencord");
    m_vencordConfigBtn  = new QPushButton("Configure", this);
    connect(m_vencordConfigBtn, &QPushButton::clicked,
            this, &UniversalThemePage::showVencordSettings);
    addRow(m_vencordCheck, m_vencordRestoreBtn, m_vencordConfigBtn);

    // Kitty
    m_kittyCheck      = new QCheckBox("Kitty Terminal", this);
    m_kittyRestoreBtn = makeRestoreBtn("kitty", "Kitty"); // matches kitty_light + kitty_dark
    addRow(m_kittyCheck, m_kittyRestoreBtn);

    // Konsole
    m_konsoleCheck      = new QCheckBox("Konsole (Colors & Profile)", this);
    m_konsoleRestoreBtn = makeRestoreBtn("konsole", "Konsole");
    addRow(m_konsoleCheck, m_konsoleRestoreBtn);

    // Btop
    m_btopCheck      = new QCheckBox("Btop (System Monitor Theme)", this);
    m_btopRestoreBtn = makeRestoreBtn("btop", "Btop");
    addRow(m_btopCheck, m_btopRestoreBtn);

    // Vicinae
    m_vicinaeCheck      = new QCheckBox("Vicinae (Dynamic Theme)", this);
    m_vicinaeRestoreBtn = makeRestoreBtn("vicinae", "Vicinae");
    addRow(m_vicinaeCheck, m_vicinaeRestoreBtn);

    // Zed
    m_zedCheck      = new QCheckBox("Zed Editor (Theme JSON)", this);
    m_zedRestoreBtn = makeRestoreBtn("zed", "Zed");
    addRow(m_zedCheck, m_zedRestoreBtn);

    // Millennium Steam (Material Theme)
    m_millenniumCheck      = new QCheckBox("Millennium Steam (Material Theme)", this);
    m_millenniumRestoreBtn = makeRestoreBtn("millennium", "Millennium");
    addRow(m_millenniumCheck, m_millenniumRestoreBtn);

    // GTK & Libadwaita (Non-Breeze Themes)
    m_gtkCheck      = new QCheckBox("GTK & Libadwaita (Non-Breeze Themes only)", this);
    m_gtkRestoreBtn = makeRestoreBtn("gtk", "GTK");
    addRow(m_gtkCheck, m_gtkRestoreBtn);

    // Obsidian — checkbox + restore in grid, path row below
    m_obsidianCheck      = new QCheckBox("Obsidian (Vault Snippet)", this);
    m_obsidianRestoreBtn = makeRestoreBtn("obsidian", "Obsidian");
    grid->addWidget(m_obsidianCheck,      row, 0);
    grid->addWidget(m_obsidianRestoreBtn, row, 1);
    row++;

    QWidget *obsContainer = new QWidget(this);
    QHBoxLayout *obsLayout = new QHBoxLayout(obsContainer);
    obsLayout->setContentsMargins(0, 0, 0, 0);
    m_obsidianPathEdit = new QLineEdit(this);
    m_obsidianPathEdit->setPlaceholderText("Path to Obsidian Vault...");
    m_browseObsidianBtn = new QPushButton("Browse...", this);
    obsLayout->addWidget(m_obsidianPathEdit);
    obsLayout->addWidget(m_browseObsidianBtn);
    grid->addWidget(obsContainer, row, 0, 1, 3);

    mainLayout->addWidget(appsGroup);

    // Sync button
    m_syncButton = new QPushButton("Sync Now", this);
    m_syncButton->setMinimumHeight(40);
    mainLayout->addStretch();
    mainLayout->addWidget(m_syncButton);

    // Connections
    connect(m_browseObsidianBtn, &QPushButton::clicked,
            this, &UniversalThemePage::pickObsidianVault);
    connect(m_syncButton, &QPushButton::clicked,
            this, &UniversalThemePage::syncNow);

    // Auto-save on toggle — writes directly to config.toml
    connect(m_vscodeCheck,    &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_firefoxCheck,   &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_zenBrowserCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_zenBrowserContentCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_discordCheck,   &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_kittyCheck,     &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_konsoleCheck,   &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_vencordCheck,   &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_btopCheck,      &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_vicinaeCheck,   &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_zedCheck,       &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_millenniumCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_gtkCheck,       &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_obsidianCheck,  &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_obsidianPathEdit, &QLineEdit::editingFinished,
            this, &UniversalThemePage::saveSettings);
}

void UniversalThemePage::loadSettings() {
    m_isLoading = true;
    TemplateConfig::ensureUserConfig();

    m_vscodeCheck->setChecked(TemplateConfig::isEnabled("vscode"));
    m_firefoxCheck->setChecked(TemplateConfig::isEnabled("firefox"));
    m_zenBrowserCheck->setChecked(TemplateConfig::isEnabled("zen_browser_chrome"));
    m_zenBrowserContentCheck->setChecked(TemplateConfig::isEnabled("zen_browser_content"));
    m_zenBrowserContentCheck->setEnabled(m_zenBrowserCheck->isChecked());
    m_discordCheck->setChecked(TemplateConfig::isEnabled("betterdiscord"));
    m_kittyCheck->setChecked(TemplateConfig::isEnabled("kitty_light"));
    m_konsoleCheck->setChecked(TemplateConfig::isEnabled("konsole"));
    m_vencordCheck->setChecked(TemplateConfig::isEnabled("vencord"));
    m_btopCheck->setChecked(TemplateConfig::isEnabled("btop"));
    m_vicinaeCheck->setChecked(TemplateConfig::isEnabled("vicinae"));
    m_zedCheck->setChecked(TemplateConfig::isEnabled("zed"));
    m_millenniumCheck->setChecked(TemplateConfig::isEnabled("millennium"));
    m_gtkCheck->setChecked(TemplateConfig::isEnabled("gtk4_colors"));
    m_obsidianCheck->setChecked(TemplateConfig::isEnabled("obsidian"));
    m_obsidianPathEdit->setText(
        TemplateConfig::getValue("obsidian", "obsidian_vault_path", ""));

    m_isLoading = false;
}

void UniversalThemePage::saveSettings() {
    if (m_isLoading) return;

    TemplateConfig::setEnabled("vscode",        m_vscodeCheck->isChecked());
    TemplateConfig::setEnabled("firefox",       m_firefoxCheck->isChecked());
    TemplateConfig::setEnabled("zen_browser_chrome",  m_zenBrowserCheck->isChecked());
    TemplateConfig::setEnabled("zen_browser_content", m_zenBrowserCheck->isChecked() && m_zenBrowserContentCheck->isChecked());
    TemplateConfig::setEnabled("betterdiscord", m_discordCheck->isChecked());
    // Kitty drives both light and dark entries together
    TemplateConfig::setEnabled("kitty_light",   m_kittyCheck->isChecked());
    TemplateConfig::setEnabled("kitty_dark",    m_kittyCheck->isChecked());
    TemplateConfig::setEnabled("konsole",       m_konsoleCheck->isChecked());
    TemplateConfig::setEnabled("vencord",       m_vencordCheck->isChecked());
    TemplateConfig::setEnabled("btop",          m_btopCheck->isChecked());
    TemplateConfig::setEnabled("vicinae",       m_vicinaeCheck->isChecked());
    TemplateConfig::setEnabled("zed",           m_zedCheck->isChecked());
    TemplateConfig::setEnabled("millennium",    m_millenniumCheck->isChecked());
    TemplateConfig::setEnabled("gtk4_colors",   m_gtkCheck->isChecked());
    TemplateConfig::setEnabled("gtk3_colors",   m_gtkCheck->isChecked());
    TemplateConfig::setEnabled("obsidian",      m_obsidianCheck->isChecked());

    if (!m_obsidianPathEdit->text().isEmpty())
        TemplateConfig::setValue("obsidian", "obsidian_vault_path",
                                 m_obsidianPathEdit->text());
}

void UniversalThemePage::pickObsidianVault() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Obsidian Vault Folder"), QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_obsidianPathEdit->setText(dir);
        saveSettings();
    }
}

void UniversalThemePage::showBDSettings() {
    QDialog dlg(this);
    dlg.setWindowTitle("Configure BetterDiscord Sync");
    dlg.resize(500, 350);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QCheckBox *useMidnight = new QCheckBox(
        "Use Midnight UI (recommended for cohesive colors)", &dlg);
    useMidnight->setChecked(
        TemplateConfig::getValue("betterdiscord", "midnight_enabled", "true") == "true");
    layout->addWidget(useMidnight);

    layout->addWidget(new QLabel("<b>Custom Import URLs (one per line):</b>"));
    QTextEdit *customEdit = new QTextEdit(&dlg);
    customEdit->setText(TemplateConfig::getList("betterdiscord", "custom_imports").join("\n"));
    layout->addWidget(customEdit);

    QDialogButtonBox *bbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        TemplateConfig::setValue("betterdiscord", "midnight_enabled",
                                 useMidnight->isChecked() ? "true" : "false");
        QStringList imports;
        for (const QString &line : customEdit->toPlainText().split("\n"))
            if (!line.trimmed().isEmpty()) imports << line.trimmed();
        imports.removeDuplicates();
        TemplateConfig::setList("betterdiscord", "custom_imports", imports);
    }
}

void UniversalThemePage::showVencordSettings() {
    QDialog dlg(this);
    dlg.setWindowTitle("Configure Vencord Sync");
    dlg.resize(500, 350);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QCheckBox *useMidnight = new QCheckBox(
        "Use Midnight Theme Base (recommended for cohesive colors)", &dlg);
    useMidnight->setChecked(
        TemplateConfig::getValue("vencord", "midnight_enabled", "true") == "true");
    layout->addWidget(useMidnight);

    layout->addWidget(new QLabel("<b>Custom Import URLs (one per line):</b>"));
    QTextEdit *customEdit = new QTextEdit(&dlg);
    customEdit->setText(TemplateConfig::getList("vencord", "custom_imports").join("\n"));
    layout->addWidget(customEdit);

    QDialogButtonBox *bbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        TemplateConfig::setValue("vencord", "midnight_enabled",
                                 useMidnight->isChecked() ? "true" : "false");
        QStringList imports;
        for (const QString &line : customEdit->toPlainText().split("\n"))
            if (!line.trimmed().isEmpty()) imports << line.trimmed();
        imports.removeDuplicates();
        TemplateConfig::setList("vencord", "custom_imports", imports);
    }
}

void UniversalThemePage::syncNow() {
    saveSettings();
    UniversalThemeExporter::syncAll();

    // Double-pass: let kdeglobals settle then sync again
    QTimer::singleShot(2000, this, []() {
        UniversalThemeExporter::syncAll();
    });

    QMessageBox::information(this, "Sync Universal Theme",
                             "Theme sync triggered. Check logs for details.");
}
