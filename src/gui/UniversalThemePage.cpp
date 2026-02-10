#include "UniversalThemePage.h"
#include "../core/Config.h"
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

UniversalThemePage::UniversalThemePage(QWidget *parent) : QWidget(parent) {
    setupUi();
    loadSettings();
}

void UniversalThemePage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QLabel *header = new QLabel("<h2>Universal Theming</h2>", this);
    mainLayout->addWidget(header);

    QLabel *desc = new QLabel("Automatically export Plasma colors to other applications.", this);
    mainLayout->addWidget(desc);

    // Apps Group
    QGroupBox *appsGroup = new QGroupBox(tr("Supported Applications"), this);
    QGridLayout *grid = new QGridLayout(appsGroup);
    grid->setColumnStretch(0, 1); // Checkbox column stretches

    int row = 0;

    // Helper to add row
    auto addRow = [&](QCheckBox* cb, QPushButton* restoreBtn, QPushButton* configBtn = nullptr) {
        grid->addWidget(cb, row, 0);
        if (configBtn) grid->addWidget(configBtn, row, 1);
        
        restoreBtn->setToolTip("Restore original config / Remove sync files");
        // Using a compact style for restore
        restoreBtn->setIcon(QIcon::fromTheme("edit-undo")); 
        if (restoreBtn->icon().isNull()) restoreBtn->setText("Restore");
        
        grid->addWidget(restoreBtn, row, configBtn ? 2 : 1);
        row++;
    };

    // VS Code
    m_vscodeCheck = new QCheckBox("VS Code (settings.json)", this);
    m_vscodeRestoreBtn = new QPushButton(this);
    connect(m_vscodeRestoreBtn, &QPushButton::clicked, [this]() {
        if (QMessageBox::question(this, "Restore VS Code", "Revert changes to settings.json?") == QMessageBox::Yes) {
            UniversalThemeExporter::restoreVSCode();
            QMessageBox::information(this, "Done", "Restored VS Code configs.");
        }
    });
    addRow(m_vscodeCheck, m_vscodeRestoreBtn);

    // Firefox
    m_firefoxCheck = new QCheckBox("Firefox / Zen Browser (userChrome.css)", this);
    m_firefoxRestoreBtn = new QPushButton(this);
    connect(m_firefoxRestoreBtn, &QPushButton::clicked, [this]() {
         QMessageBox::information(this, "Restore Firefox", "Automatic restore for Firefox is partial. Please check your chrome folder for backups (.bak) if needed.");
    });
    addRow(m_firefoxCheck, m_firefoxRestoreBtn);

    // BetterDiscord
    m_discordCheck = new QCheckBox("BetterDiscord (theme.css)", this);
    m_discordRestoreBtn = new QPushButton(this);
    m_discordConfigBtn = new QPushButton("Configure", this);
    connect(m_discordRestoreBtn, &QPushButton::clicked, [this]() {
        if (UniversalThemeExporter::restoreBetterDiscord()) 
            QMessageBox::information(this, "Done", "Removed PlasmaMaster theme from BetterDiscord.");
        else QMessageBox::warning(this, "Error", "Could not remove theme or generic error.");
    });
    connect(m_discordConfigBtn, &QPushButton::clicked, this, &UniversalThemePage::showBDSettings);
    addRow(m_discordCheck, m_discordRestoreBtn, m_discordConfigBtn);

    // Kitty
    m_kittyCheck = new QCheckBox("Kitty Terminal (kitty.conf)", this);
    m_kittyRestoreBtn = new QPushButton(this);
    connect(m_kittyRestoreBtn, &QPushButton::clicked, [this]() {
        if (QMessageBox::question(this, "Restore Kitty", "Revert kitty.conf and remove plasma-colors.conf?") == QMessageBox::Yes) {
             UniversalThemeExporter::restoreKitty();
             QMessageBox::information(this, "Done", "Restored Kitty config.");
        }
    });
    addRow(m_kittyCheck, m_kittyRestoreBtn);

    // Generic
    m_genericCheck = new QCheckBox("Generic CSS (~/.cache/.../universal.css)", this);
    m_genericRestoreBtn = new QPushButton(this);
    connect(m_genericRestoreBtn, &QPushButton::clicked, [this]() {
        UniversalThemeExporter::restoreGeneric();
        QMessageBox::information(this, "Done", "Removed generic universal.css.");
    });
    addRow(m_genericCheck, m_genericRestoreBtn);

    // Obsidian
    m_obsidianCheck = new QCheckBox("Obsidian (Vault Snippet)", this);
    m_obsidianRestoreBtn = new QPushButton(this);
    connect(m_obsidianRestoreBtn, &QPushButton::clicked, [this]() {
        if (UniversalThemeExporter::restoreObsidian())
             QMessageBox::information(this, "Done", "Removed Obsidian snippet.");
        else QMessageBox::warning(this, "Error", "Could not remove snippet (Check Vault path).");
    });
    m_obsidianRestoreBtn->setToolTip("Remove Obsidian snippet");
    m_obsidianRestoreBtn->setIcon(QIcon::fromTheme("edit-undo"));
    if (m_obsidianRestoreBtn->icon().isNull()) m_obsidianRestoreBtn->setText("Restore");
    
    // Obsidian Path UI (Row + 1)
    QWidget *obsContainer = new QWidget(this);
    QHBoxLayout *obsLayout = new QHBoxLayout(obsContainer);
    obsLayout->setContentsMargins(0,0,0,0);
    m_obsidianPathEdit = new QLineEdit(this);
    m_obsidianPathEdit->setPlaceholderText("Path to Obsidian Vault...");
    m_browseObsidianBtn = new QPushButton("Browse...", this);
    obsLayout->addWidget(m_obsidianPathEdit);
    obsLayout->addWidget(m_browseObsidianBtn);
    
    // Add Obsidian Row manually since it has extra UI
    grid->addWidget(m_obsidianCheck, row, 0);
    grid->addWidget(m_obsidianRestoreBtn, row, 1);
    row++;
    grid->addWidget(obsContainer, row, 0, 1, 3);

    mainLayout->addWidget(appsGroup);

    // Actions
    m_syncButton = new QPushButton("Sync Now", this);
    m_syncButton->setMinimumHeight(40);
    mainLayout->addStretch();
    mainLayout->addWidget(m_syncButton);

    // Connects
    connect(m_browseObsidianBtn, &QPushButton::clicked, this, &UniversalThemePage::pickObsidianVault);
    connect(m_syncButton, &QPushButton::clicked, this, &UniversalThemePage::syncNow);

    // Auto-save on toggle
    connect(m_vscodeCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_firefoxCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_discordCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_kittyCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_genericCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_obsidianCheck, &QCheckBox::toggled, this, &UniversalThemePage::saveSettings);
    connect(m_obsidianPathEdit, &QLineEdit::editingFinished, this, &UniversalThemePage::saveSettings);
}

void UniversalThemePage::loadSettings() {
    m_isLoading = true;
    m_vscodeCheck->setChecked(Config::isVSCodeSyncEnabled());
    m_firefoxCheck->setChecked(Config::isFirefoxSyncEnabled());
    m_discordCheck->setChecked(Config::isBetterDiscordSyncEnabled());
    m_kittyCheck->setChecked(Config::isKittySyncEnabled());
    m_genericCheck->setChecked(Config::isGenericSyncEnabled());
    
    m_obsidianCheck->setChecked(Config::isObsidianSyncEnabled());
    m_obsidianPathEdit->setText(Config::obsidianVaultPath());
    m_isLoading = false;
}

void UniversalThemePage::saveSettings() {
    if (m_isLoading) return;

    Config::setVSCodeSyncEnabled(m_vscodeCheck->isChecked());
    Config::setFirefoxSyncEnabled(m_firefoxCheck->isChecked());
    Config::setBetterDiscordSyncEnabled(m_discordCheck->isChecked());
    Config::setKittySyncEnabled(m_kittyCheck->isChecked());
    Config::setGenericSyncEnabled(m_genericCheck->isChecked());
    
    Config::setObsidianSyncEnabled(m_obsidianCheck->isChecked());
    Config::setObsidianVaultPath(m_obsidianPathEdit->text());
}

void UniversalThemePage::pickObsidianVault() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Obsidian Vault Folder"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_obsidianPathEdit->setText(dir);
        saveSettings();
    }
}

void UniversalThemePage::showBDSettings() {
    QDialog dlg(this);
    dlg.setWindowTitle("Configure BetterDiscord Sync");
    dlg.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    
    QCheckBox *useMaterial = new QCheckBox("Use Material UI (recommended for cohesive use of colors)", &dlg);
    useMaterial->setChecked(Config::isBetterDiscordMaterialEnabled());
    layout->addWidget(useMaterial);
    
    layout->addWidget(new QLabel("<b>Detected Imports:</b>"));
    QScrollArea *scroll = new QScrollArea(&dlg);
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scroll->setWidget(scrollContent);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll);
    
    QStringList currentImports = Config::betterDiscordImports();
    QStringList detectedImports = UniversalThemeExporter::scanBetterDiscordImports();
    QList<QCheckBox*> importChecks;
    
    for (const QString &url : detectedImports) {
        QCheckBox *cb = new QCheckBox(url, scrollContent);
        // If it's already in our config, check it.
        // Also note: the config stores exact strings.
        if (currentImports.contains(url)) cb->setChecked(true);
        importChecks << cb;
        scrollLayout->addWidget(cb);
    }
    
    if (detectedImports.isEmpty()) {
        scrollLayout->addWidget(new QLabel("No other themes with imports found in themes folder."));
    }
    
    layout->addWidget(new QLabel("<b>Custom Import URLs (One per line):</b>"));
    QTextEdit *customEdit = new QTextEdit(&dlg);
    // Populate with imports that ARE listed in config but NOT in detected (i.e. purely custom ones)
    // Or just list all currently saved custom ones that aren't checked above? 
    // Simpler: Just allow the user to add extras here. 
    // We need to parse 'currentImports' and if it matches a detected one, check the box. If not, put in text area.
    
    QStringList textImports;
    for (const QString &saved : currentImports) {
        bool found = false;
        for (const QString &detected : detectedImports) {
            if (saved == detected) { found = true; break; }
        }
        if (!found) textImports << saved;
    }
    customEdit->setText(textImports.join("\n"));
    
    layout->addWidget(customEdit);
    
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    
    if (dlg.exec() == QDialog::Accepted) {
        Config::setBetterDiscordMaterialEnabled(useMaterial->isChecked());
        
        QStringList finalImports;
        // Add checked detected imports
        for (int i=0; i<importChecks.size(); ++i) {
            if (importChecks[i]->isChecked()) {
                finalImports << detectedImports[i];
            }
        }
        // Add text imports
        QStringList lines = customEdit->toPlainText().split("\n");
        for (const QString &line : lines) {
            if (!line.trimmed().isEmpty()) finalImports << line.trimmed();
        }
        // Remove dupes
        finalImports.removeDuplicates();
        Config::setBetterDiscordImports(finalImports);
    }
}

void UniversalThemePage::syncNow() {
    saveSettings(); // Ensure saved
    
    UniversalPalette palette = UniversalThemeExporter::extractColors();
    QStringList results;
    
    if (m_vscodeCheck->isChecked()) {
        bool ok = UniversalThemeExporter::exportToVSCode(palette);
        results << QString("VS Code: %1").arg(ok ? "OK" : "Failed");
    }
    
    if (m_firefoxCheck->isChecked()) {
        bool ok = UniversalThemeExporter::exportToFirefox(palette);
        results << QString("Firefox: %1").arg(ok ? "OK" : "Failed");
    }

    if (m_discordCheck->isChecked()) {
        bool ok = UniversalThemeExporter::exportToBetterDiscord(palette);
        results << QString("BetterDiscord: %1").arg(ok ? "OK" : "Failed");
    }

    if (m_kittyCheck->isChecked()) {
        bool ok = UniversalThemeExporter::exportToKitty(palette);
        results << QString("Kitty: %1").arg(ok ? "OK" : "Failed");
    }

    if (m_genericCheck->isChecked()) {
        bool ok = UniversalThemeExporter::exportGeneric(palette);
        results << QString("Generic CSS: %1").arg(ok ? "OK" : "Failed");
    }
    
    if (m_obsidianCheck->isChecked()) {
        if (m_obsidianPathEdit->text().isEmpty()) {
            results << "Obsidian: Skipped (No Path)";
        } else {
            bool ok = UniversalThemeExporter::exportToObsidian(palette, m_obsidianPathEdit->text());
            results << QString("Obsidian: %1").arg(ok ? "OK" : "Failed");
        }
    }
    
    if (results.isEmpty()) {
        QMessageBox::information(this, "Sync Universal Theme", "No apps selected.");
    } else {
        QMessageBox::information(this, "Sync Universal Theme", results.join("\n"));
    }
}
