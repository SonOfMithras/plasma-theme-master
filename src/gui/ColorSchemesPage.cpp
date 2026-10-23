#include "ColorSchemesPage.h"
#include "PaletteCardWidget.h"
#include "FlowLayout.h"
#include "../core/PaletteCatalog.h"
#include "../core/TemplateConfig.h"
#include "../core/ThemeReader.h"
#include "../core/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QMessageBox>
#include <QFont>
#include <QIcon>

ColorSchemesPage::ColorSchemesPage(QWidget *parent) : QWidget(parent) {
    m_syncLocked = (TemplateConfig::getValue("palette", "sync_preview", "true") == "true");
    setupUi();
    updateSyncLockState();
    if (m_syncLocked) {
        syncWithEnforcedState();
    } else {
        UniversalPalette cur = UniversalThemeExporter::extractColors();
        m_previewDark = (cur.windowBg.lightness() < 128);
        m_darkPreviewBtn->setChecked(m_previewDark);
        m_lightPreviewBtn->setChecked(!m_previewDark);
    }
    populatePalettes();
}

ColorSchemesPage::~ColorSchemesPage() = default;

void ColorSchemesPage::refreshCurrentPalette() {
    QString current = PaletteCatalog::instance().activePaletteName();
    for (PaletteCardWidget *card : m_cardWidgets) {
        card->setCurrent(!current.isEmpty() && card->paletteName().compare(current, Qt::CaseInsensitive) == 0);
    }
    if (m_syncLocked) {
        syncWithEnforcedState();
    }
}

void ColorSchemesPage::onSyncLockToggled() {
    m_syncLocked = m_syncLockBtn->isChecked();
    TemplateConfig::setValue("palette", "sync_preview", m_syncLocked ? "true" : "false");
    updateSyncLockState();
    if (m_syncLocked) {
        syncWithEnforcedState();
    }
}

void ColorSchemesPage::updateSyncLockState() {
    if (m_syncLocked) {
        QIcon ic = QIcon::fromTheme(QStringLiteral("object-locked"), QIcon::fromTheme(QStringLiteral("lock")));
        if (!ic.isNull()) {
            m_syncLockBtn->setIcon(ic);
            m_syncLockBtn->setText(QString());
        } else {
            m_syncLockBtn->setIcon(QIcon());
            m_syncLockBtn->setText(QStringLiteral("🔒"));
        }
        m_syncLockBtn->setChecked(true);
        m_syncLockBtn->setToolTip(tr("Synced with active Day/Night schedule (Locked). Click to unlock manual preview."));
        m_darkPreviewBtn->setEnabled(false);
        m_lightPreviewBtn->setEnabled(false);
        m_darkPreviewBtn->setToolTip(tr("Locked to active Day/Night schedule. Click the sync lock button to preview manually."));
        m_lightPreviewBtn->setToolTip(tr("Locked to active Day/Night schedule. Click the sync lock button to preview manually."));
    } else {
        QIcon ic = QIcon::fromTheme(QStringLiteral("object-unlocked"), QIcon::fromTheme(QStringLiteral("unlock")));
        if (!ic.isNull()) {
            m_syncLockBtn->setIcon(ic);
            m_syncLockBtn->setText(QString());
        } else {
            m_syncLockBtn->setIcon(QIcon());
            m_syncLockBtn->setText(QStringLiteral("🔓"));
        }
        m_syncLockBtn->setChecked(false);
        m_syncLockBtn->setToolTip(tr("Manual preview mode (Unlocked). Click to lock and sync with Day/Night schedule."));
        m_darkPreviewBtn->setEnabled(true);
        m_lightPreviewBtn->setEnabled(true);
        m_darkPreviewBtn->setToolTip(tr("Preview dark palette variants"));
        m_lightPreviewBtn->setToolTip(tr("Preview light palette variants"));
    }
}

void ColorSchemesPage::syncWithEnforcedState() {
    bool isDay;
    if (ThemeReader::isAutoLookAndFeel()) {
        isDay = ThemeReader::isKWinDaytime();
    } else {
        QString overrideState = ThemeReader::temporaryOverride();
        if (overrideState == "light") {
            isDay = true;
        } else if (overrideState == "dark") {
            isDay = false;
        } else {
            isDay = (UniversalThemeExporter::extractColors().windowBg.lightness() >= 128);
        }
    }
    bool shouldBeDark = !isDay;

    bool bD = m_darkPreviewBtn->blockSignals(true);
    bool bL = m_lightPreviewBtn->blockSignals(true);
    m_darkPreviewBtn->setChecked(shouldBeDark);
    m_lightPreviewBtn->setChecked(!shouldBeDark);
    m_darkPreviewBtn->blockSignals(bD);
    m_lightPreviewBtn->blockSignals(bL);

    if (m_previewDark != shouldBeDark) {
        onPreviewModeToggled(shouldBeDark);
    }
}

void ColorSchemesPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(12);

    // Header
    QVBoxLayout *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(3);

    QLabel *title = new QLabel(tr("<h2>Color Schemes & Palettes</h2>"), this);
    QLabel *sub = new QLabel(
        tr("Explore curated color palettes from bundled and user themes. "
           "Applying a palette generates a KDE Plasma color scheme (<code>ptm.colors</code>) and automatically "
           "synchronizes your selected universal templates."),
        this);
    sub->setWordWrap(true);
    QFont subFont = sub->font();
    subFont.setPointSize(9);
    sub->setFont(subFont);
    QPalette sp = sub->palette();
    sp.setColor(QPalette::WindowText, sp.color(QPalette::PlaceholderText));
    sub->setPalette(sp);

    headerLayout->addWidget(title);
    headerLayout->addWidget(sub);
    mainLayout->addLayout(headerLayout);

    // Filter and Search Toolbar
    QVBoxLayout *controlBox = new QVBoxLayout();
    controlBox->setSpacing(8);

    // Row A: Search box + Preview Mode Selector + Count
    QHBoxLayout *searchRow = new QHBoxLayout();
    searchRow->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search 120+ color schemes by name..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumHeight(32);
    searchRow->addWidget(m_searchEdit, 1);

    // Dark/Light Preview toggle using native buttons in exclusive group
    QHBoxLayout *modeRow = new QHBoxLayout();
    modeRow->setSpacing(2);

    QButtonGroup *modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);

    m_darkPreviewBtn = new QPushButton(tr("🌙 Dark"), this);
    m_darkPreviewBtn->setCheckable(true);
    m_darkPreviewBtn->setChecked(true);
    m_darkPreviewBtn->setCursor(Qt::PointingHandCursor);

    m_lightPreviewBtn = new QPushButton(tr("☀️ Light"), this);
    m_lightPreviewBtn->setCheckable(true);
    m_lightPreviewBtn->setChecked(false);
    m_lightPreviewBtn->setCursor(Qt::PointingHandCursor);

    modeGroup->addButton(m_darkPreviewBtn, 0);
    modeGroup->addButton(m_lightPreviewBtn, 1);

    m_syncLockBtn = new QPushButton(this);
    m_syncLockBtn->setCheckable(true);
    m_syncLockBtn->setCursor(Qt::PointingHandCursor);
    m_syncLockBtn->setFixedWidth(36);
    m_syncLockBtn->setIconSize(QSize(16, 16));

    modeRow->addWidget(m_darkPreviewBtn);
    modeRow->addWidget(m_lightPreviewBtn);
    modeRow->addWidget(m_syncLockBtn);
    searchRow->addLayout(modeRow);

    m_countLabel = new QLabel(this);
    QFont cntFont = m_countLabel->font();
    cntFont.setPointSize(9);
    m_countLabel->setFont(cntFont);
    QPalette cp = m_countLabel->palette();
    cp.setColor(QPalette::WindowText, cp.color(QPalette::PlaceholderText));
    m_countLabel->setPalette(cp);
    searchRow->addWidget(m_countLabel);

    controlBox->addLayout(searchRow);

    // Row B: Category Filter Pills (native checkable pushbuttons)
    QHBoxLayout *pillsLayout = new QHBoxLayout();
    pillsLayout->setSpacing(6);

    m_filterGroup = new QButtonGroup(this);
    m_filterGroup->setExclusive(true);

    m_filterAllBtn = new QPushButton(tr("All"), this);
    m_filterAllBtn->setCheckable(true);
    m_filterAllBtn->setChecked(true);
    m_filterAllBtn->setCursor(Qt::PointingHandCursor);
    m_filterGroup->addButton(m_filterAllBtn, 0);
    pillsLayout->addWidget(m_filterAllBtn);

    m_filterBundledBtn = new QPushButton(tr("Bundled"), this);
    m_filterBundledBtn->setCheckable(true);
    m_filterBundledBtn->setCursor(Qt::PointingHandCursor);
    m_filterGroup->addButton(m_filterBundledBtn, 1);
    pillsLayout->addWidget(m_filterBundledBtn);

    m_filterUserBtn = new QPushButton(tr("User"), this);
    m_filterUserBtn->setCheckable(true);
    m_filterUserBtn->setCursor(Qt::PointingHandCursor);
    m_filterGroup->addButton(m_filterUserBtn, 2);
    pillsLayout->addWidget(m_filterUserBtn);

    pillsLayout->addStretch();
    controlBox->addLayout(pillsLayout);

    mainLayout->addLayout(controlBox);

    // Cards Grid in Scroll Area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_cardsContainer = new QWidget(scrollArea);
    m_flowLayout = new FlowLayout(m_cardsContainer, 0, 10, 10);
    m_flowLayout->setFillColumns(true);
    m_flowLayout->setMinItemWidth(240);
    m_cardsContainer->setLayout(m_flowLayout);

    scrollArea->setWidget(m_cardsContainer);
    mainLayout->addWidget(scrollArea, 1);

    // Status bar at bottom
    m_statusLabel = new QLabel(this);
    QFont stFont = m_statusLabel->font();
    stFont.setPointSize(8);
    m_statusLabel->setFont(stFont);
    QPalette stp = m_statusLabel->palette();
    stp.setColor(QPalette::WindowText, stp.color(QPalette::PlaceholderText));
    m_statusLabel->setPalette(stp);
    mainLayout->addWidget(m_statusLabel);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ColorSchemesPage::onSearchChanged);
    connect(m_filterGroup, &QButtonGroup::idClicked, this, &ColorSchemesPage::onFilterGroupChanged);

    connect(m_darkPreviewBtn, &QPushButton::clicked, this, [this]() {
        onPreviewModeToggled(true);
    });

    connect(m_lightPreviewBtn, &QPushButton::clicked, this, [this]() {
        onPreviewModeToggled(false);
    });

    connect(m_syncLockBtn, &QPushButton::clicked, this, &ColorSchemesPage::onSyncLockToggled);
}

void ColorSchemesPage::populatePalettes() {
    PaletteCatalog &catalog = PaletteCatalog::instance();
    catalog.reload();
    const QList<ThemePalette> &allPalettes = catalog.palettes();

    if (m_filterBundledBtn) {
        m_filterBundledBtn->setText(tr("Bundled (%1)").arg(catalog.bundledCount()));
    }
    if (m_filterUserBtn) {
        m_filterUserBtn->setText(tr("User (%1)").arg(catalog.userCount()));
    }

    QString currentPalette = catalog.activePaletteName();

    for (const ThemePalette &pal : allPalettes) {
        auto *card = new PaletteCardWidget(pal, m_previewDark, m_cardsContainer);
        if (!currentPalette.isEmpty() && pal.name.compare(currentPalette, Qt::CaseInsensitive) == 0) {
            card->setCurrent(true);
        }

        connect(card, &PaletteCardWidget::applyRequested,
                this, &ColorSchemesPage::onApplyPalette);

        m_flowLayout->addWidget(card);
        m_cardWidgets.append(card);
    }

    filterPalettes();
}

void ColorSchemesPage::onSearchChanged(const QString &) {
    filterPalettes();
}

void ColorSchemesPage::onFilterGroupChanged(int id) {
    m_activeFilter = id;
    filterPalettes();
}

void ColorSchemesPage::onPreviewModeToggled(bool isDark) {
    m_previewDark = isDark;
    for (PaletteCardWidget *card : m_cardWidgets) {
        card->setPreviewDark(isDark);
    }
}

void ColorSchemesPage::filterPalettes() {
    QString query = m_searchEdit->text().trimmed().toLower();
    int visibleCount = 0;

    for (PaletteCardWidget *card : m_cardWidgets) {
        bool matchQuery = query.isEmpty() || card->paletteName().toLower().contains(query);
        bool matchFilter = true;
        if (m_activeFilter == 1) {
            matchFilter = !card->isUser();
        } else if (m_activeFilter == 2) {
            matchFilter = card->isUser();
        }

        bool visible = matchQuery && matchFilter;
        card->setVisible(visible);
        if (visible) visibleCount++;
    }

    m_countLabel->setText(tr("Showing %1 of %2 color schemes").arg(visibleCount).arg(m_cardWidgets.size()));
}

void ColorSchemesPage::onApplyPalette(const QString &paletteName) {
    PaletteCatalog &catalog = PaletteCatalog::instance();
    bool success = catalog.applyPalette(paletteName, m_previewDark ? "night" : "day");

    if (success) {
        for (PaletteCardWidget *card : m_cardWidgets) {
            card->setCurrent(card->paletteName() == paletteName);
        }
        m_statusLabel->setText(tr("Applied palette '%1'. Plasma color scheme generated and universal sync dispatched.").arg(paletteName));
        QMessageBox::information(this, tr("Color Scheme Applied"),
                                 tr("Palette '%1' has been applied to Plasma and universal sync has been triggered.").arg(paletteName));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to apply palette '%1'.").arg(paletteName));
    }
}
