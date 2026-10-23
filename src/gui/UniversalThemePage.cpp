#include "UniversalThemePage.h"
#include "FlowLayout.h"
#include "TemplateCardWidget.h"
#include "TemplateSettingsDialog.h"
#include "../core/CommunityTemplateCatalog.h"
#include "../core/TemplateConfig.h"
#include "../core/TemplateApplyService.h"
#include "../core/UniversalThemeExporter.h"
#include "../core/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QScrollArea>
#include <QButtonGroup>
#include <QMessageBox>
#include <QTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QFont>

UniversalThemePage::UniversalThemePage(QWidget *parent) : QWidget(parent) {
    m_activeCategory = tr("All");
    setupUi();
    populateCards();

    // Wire async sync signals from TemplateApplyService
    TemplateApplyService &service = TemplateApplyService::instance();
    connect(&service, &TemplateApplyService::syncStarted,
            this, &UniversalThemePage::onSyncStarted);
    connect(&service, &TemplateApplyService::syncProgress,
            this, &UniversalThemePage::onSyncProgress);
    connect(&service, &TemplateApplyService::syncFinished,
            this, &UniversalThemePage::onSyncFinished);
}

UniversalThemePage::~UniversalThemePage() = default;

void UniversalThemePage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(10);

    // Scroll Area for all content
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget(scrollArea);
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(14);

    // Header: App Templates Title & Subtitle
    QVBoxLayout *headerBox = new QVBoxLayout();
    headerBox->setSpacing(4);

    QLabel *titleLabel = new QLabel(tr("<h3>App Templates</h3>"), scrollContent);
    QLabel *descLabel = new QLabel(
        tr("Configure application templates to automatically synchronize colors on theme and palette changes."),
        scrollContent);
    descLabel->setWordWrap(true);
    QFont descFont = descLabel->font();
    descFont.setPointSize(9);
    descLabel->setFont(descFont);
    QPalette dp = descLabel->palette();
    dp.setColor(QPalette::WindowText, dp.color(QPalette::PlaceholderText));
    descLabel->setPalette(dp);

    headerBox->addWidget(titleLabel);
    headerBox->addWidget(descLabel);
    contentLayout->addLayout(headerBox);

    // Filter & Search Controls
    QVBoxLayout *controlBox = new QVBoxLayout();
    controlBox->setSpacing(8);

    QHBoxLayout *searchRow = new QHBoxLayout();
    searchRow->setSpacing(10);

    m_searchEdit = new QLineEdit(scrollContent);
    m_searchEdit->setPlaceholderText(tr("Search application templates by name, category, or keyword..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(32);
    searchRow->addWidget(m_searchEdit, 1);

    m_installedOnlyCheck = new QCheckBox(tr("Detected on System"), scrollContent);
    m_installedOnlyCheck->setToolTip(tr("Show only applications found on this machine"));
    m_installedOnlyCheck->setCursor(Qt::PointingHandCursor);
    searchRow->addWidget(m_installedOnlyCheck);

    m_countLabel = new QLabel(scrollContent);
    QFont cntFont = m_countLabel->font();
    cntFont.setPointSize(9);
    m_countLabel->setFont(cntFont);
    QPalette cp = m_countLabel->palette();
    cp.setColor(QPalette::WindowText, cp.color(QPalette::PlaceholderText));
    m_countLabel->setPalette(cp);
    searchRow->addWidget(m_countLabel);

    controlBox->addLayout(searchRow);

    // Source Filter Pills: All | Bundled (N) | User (M)
    QHBoxLayout *sourceRow = new QHBoxLayout();
    sourceRow->setSpacing(6);

    m_sourceGroup = new QButtonGroup(this);
    m_sourceGroup->setExclusive(true);

    m_sourceAllBtn = new QPushButton(tr("All"), scrollContent);
    m_sourceAllBtn->setCheckable(true);
    m_sourceAllBtn->setChecked(true);
    m_sourceAllBtn->setCursor(Qt::PointingHandCursor);
    m_sourceGroup->addButton(m_sourceAllBtn, 0);
    sourceRow->addWidget(m_sourceAllBtn);

    m_sourceBundledBtn = new QPushButton(tr("Bundled"), scrollContent);
    m_sourceBundledBtn->setCheckable(true);
    m_sourceBundledBtn->setCursor(Qt::PointingHandCursor);
    m_sourceGroup->addButton(m_sourceBundledBtn, 1);
    sourceRow->addWidget(m_sourceBundledBtn);

    m_sourceUserBtn = new QPushButton(tr("User"), scrollContent);
    m_sourceUserBtn->setCheckable(true);
    m_sourceUserBtn->setCursor(Qt::PointingHandCursor);
    m_sourceGroup->addButton(m_sourceUserBtn, 2);
    sourceRow->addWidget(m_sourceUserBtn);

    sourceRow->addStretch();
    controlBox->addLayout(sourceRow);

    // Category Filter Pills (native checkable pushbuttons wrapping via FlowLayout)
    QWidget *pillsContainer = new QWidget(scrollContent);
    FlowLayout *pillsFlow = new FlowLayout(pillsContainer, 0, 4, 4);
    pillsFlow->setFillColumns(false);

    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);

    QStringList categories = {
        tr("All"), tr("Terminal"), tr("Editor"), tr("Browser"), tr("Chat"),
        tr("Launcher"), tr("System"), tr("Audio"), tr("Gaming"), tr("AI & Tools"), tr("Misc")
    };

    int catIdx = 0;
    for (const QString &cat : categories) {
        QPushButton *btn = new QPushButton(cat, pillsContainer);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        if (catIdx == 0) btn->setChecked(true);
        m_categoryGroup->addButton(btn, catIdx++);
        pillsFlow->addWidget(btn);
    }
    controlBox->addWidget(pillsContainer);

    contentLayout->addLayout(controlBox);

    // Cards Grid
    m_cardsContainer = new QWidget(scrollContent);
    m_flowLayout = new FlowLayout(m_cardsContainer, 0, 10, 10);
    m_flowLayout->setFillColumns(true);
    m_flowLayout->setMinItemWidth(260);
    m_cardsContainer->setLayout(m_flowLayout);
    contentLayout->addWidget(m_cardsContainer);

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // Bottom Action & Status Bar
    QHBoxLayout *bottomBar = new QHBoxLayout();
    bottomBar->setSpacing(12);

    QVBoxLayout *statusBox = new QVBoxLayout();
    statusBox->setSpacing(3);

    m_statusLabel = new QLabel(tr("Ready to sync templates"), this);
    QFont stFont = m_statusLabel->font();
    stFont.setPointSize(8.5);
    m_statusLabel->setFont(stFont);
    statusBox->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->setVisible(false);
    statusBox->addWidget(m_progressBar);

    bottomBar->addLayout(statusBox, 1);

    m_syncButton = new QPushButton(tr("Sync Now"), this);
    m_syncButton->setMinimumHeight(38);
    m_syncButton->setMinimumWidth(130);
    m_syncButton->setCursor(Qt::PointingHandCursor);
    QFont syncFont = m_syncButton->font();
    syncFont.setBold(true);
    m_syncButton->setFont(syncFont);
    bottomBar->addWidget(m_syncButton);

    mainLayout->addLayout(bottomBar);

    // --- Connections ---
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &UniversalThemePage::onSearchTextChanged);
    connect(m_sourceGroup, &QButtonGroup::idClicked,
            this, [this](int id) {
                m_activeSourceFilter = id;
                filterCards();
            });
    connect(m_categoryGroup, &QButtonGroup::idClicked,
            this, &UniversalThemePage::onCategoryFilterChanged);
    connect(m_installedOnlyCheck, &QCheckBox::toggled,
            this, &UniversalThemePage::onInstalledOnlyToggled);
    connect(m_syncButton, &QPushButton::clicked,
            this, &UniversalThemePage::onSyncClicked);
}

void UniversalThemePage::populateCards() {
    m_isLoading = true;
    TemplateConfig::ensureUserConfig();

    CommunityTemplateCatalog &catalog = CommunityTemplateCatalog::instance();
    catalog.reload();

    if (m_sourceBundledBtn) {
        m_sourceBundledBtn->setText(tr("Bundled (%1)").arg(catalog.bundledCount()));
    }
    if (m_sourceUserBtn) {
        m_sourceUserBtn->setText(tr("User (%1)").arg(catalog.userCount()));
    }

    while (QLayoutItem *item = m_flowLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) delete w;
        delete item;
    }
    m_cardWidgets.clear();

    const QList<CommunityTemplateApp> &allApps = catalog.apps();

    for (const CommunityTemplateApp &app : allApps) {
        if (app.category.toLower().trimmed() == "compositor") continue;

        auto *card = new TemplateCardWidget(app, m_cardsContainer);
        card->setChecked(TemplateConfig::isAppEnabled(app.id));

        connect(card, &TemplateCardWidget::appToggled,
                this, &UniversalThemePage::onAppToggled);
        connect(card, &TemplateCardWidget::configureClicked,
                this, &UniversalThemePage::onConfigureClicked);
        connect(card, &TemplateCardWidget::restoreClicked,
                this, &UniversalThemePage::onRestoreClicked);

        m_flowLayout->addWidget(card);
        m_cardWidgets.append(card);
    }

    m_isLoading = false;
    filterCards();
}

void UniversalThemePage::onSearchTextChanged(const QString &) {
    filterCards();
}

void UniversalThemePage::onCategoryFilterChanged(int id) {
    QAbstractButton *btn = m_categoryGroup->button(id);
    if (btn) {
        m_activeCategory = btn->text();
        filterCards();
    }
}

void UniversalThemePage::onInstalledOnlyToggled(bool) {
    filterCards();
}

void UniversalThemePage::filterCards() {
    QString search = m_searchEdit->text().trimmed().toLower();
    bool installedOnly = m_installedOnlyCheck->isChecked();

    int visibleCount = 0;
    for (TemplateCardWidget *card : m_cardWidgets) {
        bool matchSearch = search.isEmpty() ||
                           card->displayName().toLower().contains(search) ||
                           card->category().toLower().contains(search) ||
                           card->description().toLower().contains(search) ||
                           card->appId().toLower().contains(search);

        bool matchCat = (m_activeCategory == tr("All")) ||
                        (card->category().compare(m_activeCategory, Qt::CaseInsensitive) == 0);

        bool matchInstalled = !installedOnly || card->isInstalled();

        bool matchSource = true;
        if (m_activeSourceFilter == 1) {
            matchSource = !card->isUser();
        } else if (m_activeSourceFilter == 2) {
            matchSource = card->isUser();
        }

        bool visible = matchSearch && matchCat && matchInstalled && matchSource;
        card->setVisible(visible);
        if (visible) visibleCount++;
    }

    m_countLabel->setText(tr("Showing %1 of %2 templates").arg(visibleCount).arg(m_cardWidgets.size()));
}

void UniversalThemePage::onAppToggled(const QString &appId, bool enabled) {
    if (m_isLoading) return;

    TemplateConfig::setAppEnabled(appId, enabled);
    Logger::log(QString("UniversalThemePage: App %1 toggled %2").arg(appId).arg(enabled ? "ON" : "OFF"),
                Logger::Info);

    if (appId == "zed" && enabled) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Zed Theme Style"));
        msgBox.setText(tr("<b>Choose Zed Theme Variant</b>"));
        msgBox.setInformativeText(tr("Select your preferred background style for Zed Editor:\n\n"
                                     "• Opaque: Solid background following system palette\n"
                                     "• Transparent: Translucent background for blur effects"));
        QPushButton *opaqueBtn = msgBox.addButton(tr("Opaque"), QMessageBox::AcceptRole);
        QPushButton *transBtn = msgBox.addButton(tr("Transparent"), QMessageBox::AcceptRole);

        QString current = TemplateConfig::getValue("zed", "opacity", "opaque");
        if (current == "transparent") {
            msgBox.setDefaultButton(transBtn);
        } else {
            msgBox.setDefaultButton(opaqueBtn);
        }

        msgBox.exec();
        if (msgBox.clickedButton() == transBtn) {
            TemplateConfig::setValue("zed", "opacity", "transparent");
        } else {
            TemplateConfig::setValue("zed", "opacity", "opaque");
        }
    }
}

void UniversalThemePage::onConfigureClicked(const QString &appId) {
    TemplateSettingsDialog dlg(appId, this);
    dlg.exec();
}

void UniversalThemePage::onRestoreClicked(const QString &appId) {
    QString appName = appId;
    for (TemplateCardWidget *c : m_cardWidgets) {
        if (c->appId() == appId) {
            appName = c->displayName();
            break;
        }
    }

    if (QMessageBox::question(this, tr("Restore %1").arg(appName),
                             tr("Restore %1 configuration from backup?").arg(appName))
        != QMessageBox::Yes) {
        return;
    }

    bool restoredAny = false;

    const CommunityTemplateApp *app = CommunityTemplateCatalog::instance().findApp(appId);
    if (app) {
        for (const auto &e : app->entries) {
            for (const QString &p : e.outputPaths) {
                QString exp = CommunityTemplateCatalog::expandPath(p);
                if (UniversalThemeExporter::restoreFile(exp)) restoredAny = true;
            }
        }
    }

    if (restoredAny) {
        QMessageBox::information(this, tr("Done"),
                                 tr("Restored original configuration for %1.").arg(appName));
    } else {
        QMessageBox::warning(this, tr("No Backup Found"),
                             tr("No backup files found for %1.").arg(appName));
    }
}

void UniversalThemePage::onSyncClicked() {
    TemplateApplyService::instance().applyAsync("current");
}

void UniversalThemePage::onSyncStarted(quint64) {
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setText(tr("Starting asynchronous template sync..."));
    m_syncButton->setEnabled(false);
    m_syncButton->setText(tr("Syncing..."));
}

void UniversalThemePage::onSyncProgress(quint64, int current, int total, const QString &appName) {
    m_progressBar->setMaximum(total > 0 ? total : 1);
    m_progressBar->setValue(current);
    m_statusLabel->setText(tr("Syncing [%1] (%2 of %3)...").arg(appName).arg(current).arg(total));
}

void UniversalThemePage::onSyncFinished(quint64, bool success, const QStringList &warnings) {
    m_progressBar->setVisible(false);
    m_syncButton->setEnabled(true);
    m_syncButton->setText(tr("Sync Now"));

    QString timeStr = QTime::currentTime().toString("hh:mm:ss");
    if (success) {
        m_statusLabel->setText(tr("Last sync completed successfully at %1").arg(timeStr));
    } else {
        m_statusLabel->setText(tr("Sync completed with warnings at %1").arg(timeStr));
    }

    if (!warnings.isEmpty()) {
        Logger::log("UniversalThemePage: Sync warnings:\n" + warnings.join("\n"), Logger::Warning);
    }
}
