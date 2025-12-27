#include "GlobalThemeEditor.h"
#include "core/GlobalThemeManager.h"
#include "core/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

GlobalThemeEditor::GlobalThemeEditor(QWidget *parent) : QWidget(parent) {
    initUi();
    refreshList();
}

void GlobalThemeEditor::initUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    // --- Left Pane: List ---
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    
    leftLayout->addWidget(new QLabel("Installed Global Themes:"));
    m_themeList = new QListWidget();
    connect(m_themeList, &QListWidget::currentItemChanged, this, &GlobalThemeEditor::onThemeSelected);
    leftLayout->addWidget(m_themeList);
    
    QPushButton *refreshBtn = new QPushButton("Refresh List");
    connect(refreshBtn, &QPushButton::clicked, this, &GlobalThemeEditor::refreshList);
    leftLayout->addWidget(refreshBtn);
    
    QPushButton *cloneBtn = new QPushButton("Clone Selected Theme");
    connect(cloneBtn, &QPushButton::clicked, this, &GlobalThemeEditor::cloneTheme);
    leftLayout->addWidget(cloneBtn);
    
    splitter->addWidget(leftWidget);
    
    // --- Right Pane: Editor Tabs ---
    m_editorTabs = new QTabWidget();
    m_editorTabs->setEnabled(false);
    
    // 1. Easy Editor
    m_easyWidget = new QWidget();
    QFormLayout *easyLayout = new QFormLayout(m_easyWidget);
    
    // Define fields
    struct FieldDef { QString label; QString category; QString group; QString subgroup; QString key; };
    QList<FieldDef> fields = {
        {"Application Style", "application_styles", "kdeglobals", "KDE", "widgetStyle"},
        {"Color Scheme", "colors", "kdeglobals", "General", "ColorScheme"},
        {"Icon Theme", "icons", "kdeglobals", "Icons", "Theme"},
        {"Cursor Theme", "cursors", "kcminputrc", "Mouse", "cursorTheme"},
        {"Plasma Style", "plasma_style", "plasmarc", "Theme", "name"},
        {"Window Decoration", "window_decorations", "kwinrc", "org.kde.kdecoration2", "theme"},
        {"Window Decoration Engine", "window_decoration_engines", "kwinrc", "org.kde.kdecoration2", "library"}
    };
    
    for (const auto &f : fields) {
        QComboBox *combo = new QComboBox();
        combo->setEditable(true); // Allow custom
        // Populate (deferred or now? now is fine, categories are static-ish)
        QStringList items = GlobalThemeManager::listSubThemes(f.category);
        combo->addItem(""); // Empty default
        combo->addItems(items);
        
        ConfigKey key = {f.subgroup.isEmpty() ? f.group : f.group + "][" + f.subgroup, f.key};
        // Actually KConfigGroup identifiers are simpler. construct a string key for map?
        // Using struct ConfigKey matches QMap.
        // Wait, KConfig handles nested groups by accessing group("A").group("B").
        // Let's store "group|subgroup" in section string for easy parsing later.
        QString section = f.group;
        if (!f.subgroup.isEmpty()) section += "|" + f.subgroup;
        
        ConfigKey mapKey = {section, f.key};
        m_combos[mapKey] = combo;
        
        easyLayout->addRow(f.label + ":", combo);
    }
    
    QHBoxLayout *easyBtnLayout = new QHBoxLayout();
    QPushButton *saveEasyBtn = new QPushButton("Save Changes");
    connect(saveEasyBtn, &QPushButton::clicked, this, &GlobalThemeEditor::saveEasyDefaults);
    QPushButton *restoreBtn = new QPushButton("Restore Original Defaults");
    connect(restoreBtn, &QPushButton::clicked, this, &GlobalThemeEditor::restoreDefaults);
    easyBtnLayout->addWidget(saveEasyBtn);
    easyBtnLayout->addWidget(restoreBtn);
    easyLayout->addRow(easyBtnLayout);
    
    m_editorTabs->addTab(m_easyWidget, "Easy Editor");
    
    // 2. Raw Editor
    m_rawWidget = new QWidget();
    QVBoxLayout *rawLayout = new QVBoxLayout(m_rawWidget);
    m_rawEditor = new QTextEdit();
    // Use monospace font
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    m_rawEditor->setFont(font);
    
    QPushButton *saveRawBtn = new QPushButton("Save Raw File");
    connect(saveRawBtn, &QPushButton::clicked, this, &GlobalThemeEditor::saveRawDefaults);
    
    rawLayout->addWidget(m_rawEditor);
    rawLayout->addWidget(saveRawBtn);
    
    m_editorTabs->addTab(m_rawWidget, "Raw Editor");
    
    m_editorTabs->addTab(m_rawWidget, "Raw Editor");
    
    splitter->addWidget(m_editorTabs);
    splitter->setStretchFactor(1, 1); // Expand right side
    
    mainLayout->addWidget(splitter);
}

void GlobalThemeEditor::refreshList() {
    m_themeList->clear();
    QList<GlobalThemeInfo> themes = GlobalThemeManager::listInstalledThemes();
    for (const auto &t : themes) {
        QString label = t.name;
        if (t.isSystem) label += " (System)";
        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, t.name); // Store real name
        m_themeList->addItem(item);
    }
}

void GlobalThemeEditor::onThemeSelected(QListWidgetItem *current, QListWidgetItem * /*previous*/) {
    if (!current) {
        m_editorTabs->setEnabled(false);
        m_rawEditor->clear();
        return;
    }
    
    QString themeName = current->data(Qt::UserRole).toString();
    QString content = GlobalThemeManager::readDefaults(themeName);
    m_rawEditor->setPlainText(content);
    
    parseDefaultsToUi(themeName); // Pass themeName to open file with KConfig
    
    m_editorTabs->setEnabled(true);
}

void GlobalThemeEditor::parseDefaultsToUi(const QString &themeName) {
    if (themeName.isEmpty()) return;
    QString path = GlobalThemeManager::getDefaultsPath(themeName);
    KConfig config(path, KConfig::SimpleConfig);
    
    for (auto it = m_combos.begin(); it != m_combos.end(); ++it) {
        ConfigKey key = it.key();
        QComboBox *combo = it.value();
        
        // Parse "group|subgroup"
        QStringList parts = key.section.split('|');
        KConfigGroup group = config.group(parts[0]);
        if (parts.size() > 1) {
            group = group.group(parts[1]);
        }
        
        QString val = group.readEntry(key.key, QString());
        
        // Strip __aurorae__svg__ for display matching
        if (key.key == "theme" && key.section == "kwinrc|org.kde.kdecoration2") {
            if (val.startsWith("__aurorae__svg__")) {
                val.remove(0, 16); // Remove prefix
            }
        }
        
        combo->setCurrentText(val); // Sets text even if not in items (editable)
    }
}

void GlobalThemeEditor::saveEasyDefaults() {
    QListWidgetItem *item = m_themeList->currentItem();
    if (!item) return;
    QString themeName = item->data(Qt::UserRole).toString();
    
    QString path = GlobalThemeManager::getDefaultsPath(themeName);
    KConfig config(path, KConfig::SimpleConfig);
    
    bool changed = false;
    
    // Check Engine first if we need to modify 'theme'
    QString engine = "org.kde.breeze"; // Default presumption
    ConfigKey engineKey = {"kwinrc|org.kde.kdecoration2", "library"};
    if (m_combos.contains(engineKey)) {
        engine = m_combos[engineKey]->currentText();
    }
    
     for (auto it = m_combos.begin(); it != m_combos.end(); ++it) {
        ConfigKey key = it.key();
        QComboBox *combo = it.value();
        
        QStringList parts = key.section.split('|');
        KConfigGroup group = config.group(parts[0]);
        if (parts.size() > 1) {
            group = group.group(parts[1]);
        }
        
        QString val = combo->currentText();
        
        // Special Handling for Aurorae
        if (key.key == "theme" && key.section == "kwinrc|org.kde.kdecoration2") {
            if (engine == "org.kde.kwin.aurorae" && !val.isEmpty()) {
                if (!val.startsWith("__aurorae__svg__")) {
                    val = "__aurorae__svg__" + val;
                }
            }
        }
        
        if (!val.isEmpty()) {
            group.writeEntry(key.key, val);
            changed = true;
        }
    }
    
    if (changed) {
        config.sync();
        Logger::log("Easy Edit: Updates synced to " + path, Logger::Info);
        QMessageBox::information(this, "Saved", "Theme defaults updated.");
        
        // Refresh raw view
        m_rawEditor->setPlainText(GlobalThemeManager::readDefaults(themeName));
    }
}

void GlobalThemeEditor::saveRawDefaults() {
    QListWidgetItem *item = m_themeList->currentItem();
    if (!item) return;
    QString themeName = item->data(Qt::UserRole).toString();
    
    if (GlobalThemeManager::writeDefaults(themeName, m_rawEditor->toPlainText())) {
        QMessageBox::information(this, "Saved", "Raw File Saved.");
        parseDefaultsToUi(themeName); // Update easy view
    } else {
        QMessageBox::critical(this, "Error", "Failed to save file. Check permissions.");
    }
}

void GlobalThemeEditor::cloneTheme() {
    QListWidgetItem *item = m_themeList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No Selection", "Please select a theme to clone.");
        return;
    }
    QString srcName = item->data(Qt::UserRole).toString();
    
    bool ok;
    QString newName = QInputDialog::getText(this, "Clone Theme", 
                                            "New name for clone:", QLineEdit::Normal, 
                                            srcName + "-Copy", &ok);
    if (ok && !newName.isEmpty()) {
        if (GlobalThemeManager::cloneTheme(srcName, newName)) {
            QMessageBox::information(this, "Success", "Theme Cloned.");
            refreshList();
        } else {
            QMessageBox::critical(this, "Error", "Clone Failed. Destination might exist.");
        }
    }
}

void GlobalThemeEditor::restoreDefaults() {
    QListWidgetItem *item = m_themeList->currentItem();
    if (!item) return;
    QString themeName = item->data(Qt::UserRole).toString();

    QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Confirm Restore"),
                                     tr("Are you sure you want to restore defaults? Current changes will be lost."),
                                     QMessageBox::Yes | QMessageBox::No);
                                     
    if (res == QMessageBox::Yes) {
        QString content = GlobalThemeManager::restoreDefaults(themeName);
        if (!content.isEmpty()) {
            m_rawEditor->setPlainText(content);
            parseDefaultsToUi(themeName);
            QMessageBox::information(this, tr("Success"), tr("Defaults restored from backup."));
        } else {
            QMessageBox::warning(this, tr("Failed"), tr("Could not restore defaults (Backup might not exist)."));
        }
    }
}
