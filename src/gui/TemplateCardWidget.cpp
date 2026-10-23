#include "TemplateCardWidget.h"
#include "../core/TemplateConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QIcon>
#include <QFont>
#include <QPainter>
#include <QMouseEvent>

TemplateCardWidget::TemplateCardWidget(const CommunityTemplateApp &app, QWidget *parent)
    : QFrame(parent),
      m_appId(app.id),
      m_name(app.name),
      m_category(app.resolvedCategoryName()),
      m_description(app.description),
      m_iconName(app.categoryIconName()),
      m_isInstalled(app.isInstalled()),
      m_isUser(app.isUser) {
    setupUi();
    buildRichTooltip();
}

TemplateCardWidget::TemplateCardWidget(const QString &id,
                                       const QString &name,
                                       const QString &category,
                                       const QString &desc,
                                       bool installed,
                                       QWidget *parent)
    : QFrame(parent),
      m_appId(id),
      m_name(name),
      m_category(category),
      m_description(desc),
      m_iconName("applications-other"),
      m_isInstalled(installed) {
    setupUi();
    buildRichTooltip();
}

TemplateCardWidget::~TemplateCardWidget() = default;

void TemplateCardWidget::setupUi() {
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);
    setMinimumSize(250, 110);
    setMaximumHeight(130);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *cardLayout = new QVBoxLayout(this);
    cardLayout->setContentsMargins(10, 8, 10, 8);
    cardLayout->setSpacing(6);

    // Top Header: Icon + Name + Category Pill + Toggle
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    QLabel *iconLabel = new QLabel(this);
    QIcon icon = QIcon::fromTheme(m_iconName);
    if (icon.isNull()) icon = QIcon::fromTheme("applications-other");
    iconLabel->setPixmap(icon.pixmap(20, 20));
    topLayout->addWidget(iconLabel);

    m_nameLabel = new QLabel(m_name, this);
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(10);
    m_nameLabel->setFont(nameFont);
    topLayout->addWidget(m_nameLabel);

    m_catBadge = new QLabel(m_category, this);
    QFont catFont = m_catBadge->font();
    catFont.setPointSize(8);
    m_catBadge->setFont(catFont);
    topLayout->addWidget(m_catBadge);

    topLayout->addStretch();

    m_toggle = new QCheckBox(this);
    m_toggle->setCursor(Qt::PointingHandCursor);
    m_toggle->setToolTip(tr("Enable or disable theme sync for %1").arg(m_name));
    topLayout->addWidget(m_toggle);

    cardLayout->addLayout(topLayout);

    // Middle: Description
    m_descLabel = new QLabel(this);
    m_descLabel->setWordWrap(true);
    QFont descFont = m_descLabel->font();
    descFont.setPointSize(8);
    m_descLabel->setFont(descFont);
    QString shortDesc = m_description;
    if (shortDesc.isEmpty()) shortDesc = tr("Synchronizes application colors with system theme.");
    if (shortDesc.length() > 95) shortDesc = shortDesc.left(92) + "...";
    m_descLabel->setText(shortDesc);
    cardLayout->addWidget(m_descLabel);

    // Bottom Action Row
    QHBoxLayout *botLayout = new QHBoxLayout();
    botLayout->setSpacing(6);

    m_statusBadge = new QLabel(this);
    QFont sFont = m_statusBadge->font();
    sFont.setPointSize(8);
    m_statusBadge->setFont(sFont);
    if (m_isInstalled) {
        m_statusBadge->setText(tr("<span style='color: #4CAF50;'>●</span> Installed"));
    } else {
        m_statusBadge->setText(tr("<span style='color: #888888;'>○</span> Available"));
    }
    botLayout->addWidget(m_statusBadge);

    botLayout->addStretch();

    m_restoreBtn = new QPushButton(this);
    m_restoreBtn->setIcon(QIcon::fromTheme("edit-undo"));
    m_restoreBtn->setToolTip(tr("Restore original config from backup"));
    m_restoreBtn->setFlat(true);
    m_restoreBtn->setFixedSize(24, 24);
    botLayout->addWidget(m_restoreBtn);

    m_configBtn = new QPushButton(this);
    m_configBtn->setIcon(QIcon::fromTheme("configure"));
    m_configBtn->setToolTip(tr("Template settings and documentation"));
    m_configBtn->setFlat(true);
    m_configBtn->setFixedSize(24, 24);
    botLayout->addWidget(m_configBtn);

    cardLayout->addLayout(botLayout);

    updatePaletteColors();

    // Signals
    connect(m_toggle, &QCheckBox::toggled, this, [this](bool checked) {
        emit appToggled(m_appId, checked);
    });
    connect(m_configBtn, &QPushButton::clicked, this, [this]() {
        emit configureClicked(m_appId);
    });
    connect(m_restoreBtn, &QPushButton::clicked, this, [this]() {
        emit restoreClicked(m_appId);
    });
}

void TemplateCardWidget::updatePaletteColors() {
    if (m_catBadge) {
        QPalette cp = m_catBadge->palette();
        cp.setColor(QPalette::WindowText, palette().color(QPalette::PlaceholderText));
        m_catBadge->setPalette(cp);
    }

    if (m_descLabel) {
        QPalette dp = m_descLabel->palette();
        dp.setColor(QPalette::WindowText, palette().color(QPalette::PlaceholderText));
        m_descLabel->setPalette(dp);
    }
}

void TemplateCardWidget::buildRichTooltip() {
    QString tooltip = QString(
        "<div style='min-width: 240px;'>"
        "<b>%1</b> &nbsp; <span style='color: #888;'>[%2]</span><br>"
        "<hr style='border: none; border-top: 1px solid #555; margin: 4px 0;'>"
        "<p>%3</p>"
        "<small>Status: %4</small><br>"
        "<small>Origin: %5</small>"
        "</div>"
    ).arg(m_name,
          m_category,
          m_description.isEmpty() ? tr("Applies Plasma colors automatically.") : m_description,
          m_isInstalled ? tr("<b>Detected on this system</b>") : tr("Not detected (available)"),
          m_isUser ? tr("User template") : tr("Bundled template"));

    setToolTip(tooltip);
}

bool TemplateCardWidget::isEnabled() const {
    return m_toggle->isChecked();
}

void TemplateCardWidget::setChecked(bool checked) {
    if (m_toggle->isChecked() != checked) {
        QSignalBlocker blocker(m_toggle);
        m_toggle->setChecked(checked);
    }
}

void TemplateCardWidget::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    if (m_isHovered) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QColor hoverColor = palette().color(QPalette::Highlight);
        hoverColor.setAlpha(120);
        QPen pen(hoverColor, 1.5);
        p.setPen(pen);
        p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);
    }
}

void TemplateCardWidget::enterEvent(QEnterEvent *event) {
    m_isHovered = true;
    update();
    QFrame::enterEvent(event);
}

void TemplateCardWidget::leaveEvent(QEvent *event) {
    m_isHovered = false;
    update();
    QFrame::leaveEvent(event);
}

void TemplateCardWidget::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange) {
        updatePaletteColors();
        update();
    }
}
