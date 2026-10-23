#include "PaletteCardWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>

// =============================================================================
// Native PalettePreviewWidget: renders swatch ribbon + ANSI dots via QPainter
// =============================================================================
class PalettePreviewWidget : public QWidget {
public:
    explicit PalettePreviewWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(38);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMouseTracking(true);
    }

    void setColors(const QList<QPair<QString, QColor>> &swatches, const QList<QColor> &termColors) {
        m_swatches = swatches;
        m_termColors = termColors;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // 1. Draw core swatches ribbon
        int totalWidth = width();
        int swatchHeight = 18;
        int count = m_swatches.size();
        if (count > 0 && totalWidth > 10) {
            double chipWidth = static_cast<double>(totalWidth) / count;

            QPainterPath path;
            path.addRoundedRect(0, 0, totalWidth, swatchHeight, 4, 4);
            p.save();
            p.setClipPath(path);
            p.setPen(Qt::NoPen);

            for (int i = 0; i < count; ++i) {
                int x1 = static_cast<int>(i * chipWidth);
                int x2 = static_cast<int>((i + 1) * chipWidth);
                p.setBrush(m_swatches[i].second);
                p.drawRect(x1, 0, x2 - x1, swatchHeight);
            }
            p.restore();

            // Subtle border around ribbon using current palette
            p.setPen(QPen(palette().color(QPalette::Mid), 1));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(0, 0, totalWidth - 1, swatchHeight - 1, 4, 4);
        }

        // 2. Draw 8 ANSI terminal color dots
        int dotSize = 10;
        int dotSpacing = 5;
        int startY = swatchHeight + 5;
        for (int i = 0; i < m_termColors.size() && i < 8; ++i) {
            int dotX = i * (dotSize + dotSpacing);
            p.setPen(QPen(QColor(0, 0, 0, 70), 1));
            p.setBrush(m_termColors[i].isValid() ? m_termColors[i] : QColor("#888888"));
            p.drawEllipse(dotX, startY, dotSize, dotSize);
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        int swatchHeight = 18;
        if (event->pos().y() <= swatchHeight && !m_swatches.isEmpty()) {
            double chipWidth = static_cast<double>(width()) / m_swatches.size();
            int idx = static_cast<int>(event->pos().x() / chipWidth);
            if (idx >= 0 && idx < m_swatches.size()) {
                setToolTip(QString("%1: %2").arg(m_swatches[idx].first, m_swatches[idx].second.name().toUpper()));
                return;
            }
        }
        int dotSize = 10;
        int dotSpacing = 5;
        int startY = swatchHeight + 5;
        if (event->pos().y() >= startY && event->pos().y() <= startY + dotSize) {
            int idx = event->pos().x() / (dotSize + dotSpacing);
            static const QStringList termNames = {
                "Black", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White"
            };
            if (idx >= 0 && idx < m_termColors.size() && idx < termNames.size()) {
                setToolTip(QString("ANSI %1: %2").arg(termNames[idx], m_termColors[idx].name().toUpper()));
                return;
            }
        }
        setToolTip(QString());
    }

private:
    QList<QPair<QString, QColor>> m_swatches;
    QList<QColor> m_termColors;
};

// =============================================================================
// PaletteCardWidget Implementation
// =============================================================================
PaletteCardWidget::PaletteCardWidget(const ThemePalette &palette, bool previewDark, QWidget *parent)
    : QFrame(parent), m_palette(palette), m_previewDark(previewDark) {
    setupUi();
    updateSwatches();
}

PaletteCardWidget::~PaletteCardWidget() = default;

void PaletteCardWidget::setupUi() {
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);
    setMinimumSize(240, 135);
    setMaximumHeight(145);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    // Header: Name + Badge + Active Indicator
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(6);

    m_nameLabel = new QLabel(m_palette.name, this);
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(10);
    m_nameLabel->setFont(nameFont);
    headerLayout->addWidget(m_nameLabel);

    m_badgeLabel = new QLabel(this);
    QFont badgeFont = m_badgeLabel->font();
    badgeFont.setPointSize(8);
    m_badgeLabel->setFont(badgeFont);
    m_badgeLabel->setText(m_palette.isUser ? tr("User") : tr("Bundled"));
    headerLayout->addWidget(m_badgeLabel);

    headerLayout->addStretch();

    m_currentBadge = new QLabel(tr("● Active"), this);
    QFont curFont = m_currentBadge->font();
    curFont.setBold(true);
    curFont.setPointSize(8);
    m_currentBadge->setFont(curFont);
    m_currentBadge->setVisible(m_isCurrent);
    headerLayout->addWidget(m_currentBadge);

    mainLayout->addLayout(headerLayout);

    // Core Swatches & Terminal Dots Preview
    m_previewWidget = new PalettePreviewWidget(this);
    mainLayout->addWidget(m_previewWidget);

    // Bottom Action Row
    QHBoxLayout *botLayout = new QHBoxLayout();
    botLayout->addStretch();

    m_applyBtn = new QPushButton(tr("Apply Palette"), this);
    m_applyBtn->setCursor(Qt::PointingHandCursor);

    connect(m_applyBtn, &QPushButton::clicked, this, [this]() {
        emit applyRequested(m_palette.name);
    });

    botLayout->addWidget(m_applyBtn);
    mainLayout->addLayout(botLayout);

    updatePaletteColors();
}

void PaletteCardWidget::setPreviewDark(bool isDark) {
    if (m_previewDark != isDark) {
        m_previewDark = isDark;
        updateSwatches();
    }
}

void PaletteCardWidget::setCurrent(bool isCurrent) {
    m_isCurrent = isCurrent;
    m_currentBadge->setVisible(isCurrent);
    update();
}

void PaletteCardWidget::updateSwatches() {
    const PaletteModeColors &m = m_previewDark ? m_palette.dark : m_palette.light;

    QList<QPair<QString, QColor>> swatches = {
        {"Primary", m.primary},
        {"Secondary", m.secondary},
        {"Tertiary", m.tertiary},
        {"Surface", m.surface},
        {"SurfaceVariant", m.surfaceVariant},
        {"Error", m.error}
    };

    QList<QColor> termColors;
    for (int i = 0; i < 8; ++i) {
        termColors.append(m.terminalNormal[i]);
    }

    if (m_previewWidget) {
        m_previewWidget->setColors(swatches, termColors);
    }
}

void PaletteCardWidget::updatePaletteColors() {
    if (m_badgeLabel) {
        QPalette bp = m_badgeLabel->palette();
        bp.setColor(QPalette::WindowText, palette().color(QPalette::PlaceholderText));
        m_badgeLabel->setPalette(bp);
    }

    if (m_currentBadge) {
        QPalette cp = m_currentBadge->palette();
        cp.setColor(QPalette::WindowText, palette().color(QPalette::Highlight));
        m_currentBadge->setPalette(cp);
    }
}

void PaletteCardWidget::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_isCurrent) {
        QPen pen(palette().color(QPalette::Highlight), 2);
        p.setPen(pen);
        p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);
    } else if (m_isHovered) {
        QColor hoverColor = palette().color(QPalette::Highlight);
        hoverColor.setAlpha(120);
        QPen pen(hoverColor, 1.5);
        p.setPen(pen);
        p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);
    }
}

void PaletteCardWidget::enterEvent(QEnterEvent *event) {
    m_isHovered = true;
    update();
    QFrame::enterEvent(event);
}

void PaletteCardWidget::leaveEvent(QEvent *event) {
    m_isHovered = false;
    update();
    QFrame::leaveEvent(event);
}

void PaletteCardWidget::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange) {
        updatePaletteColors();
        update();
    }
}
