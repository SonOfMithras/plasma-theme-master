#ifndef PALETTECARDWIDGET_H
#define PALETTECARDWIDGET_H

#include <QFrame>
#include <QString>
#include <QList>
#include <QPair>
#include <QColor>
#include "../core/PaletteCatalog.h"

class QLabel;
class QPushButton;
class PalettePreviewWidget;

class PaletteCardWidget : public QFrame {
    Q_OBJECT
public:
    explicit PaletteCardWidget(const ThemePalette &palette, bool previewDark = true, QWidget *parent = nullptr);
    ~PaletteCardWidget() override;

    QString paletteName() const { return m_palette.name; }
    QString paletteId() const { return m_palette.id; }
    bool isOfficial() const { return m_palette.isOfficial; }
    bool isUser() const { return m_palette.isUser; }
    bool isCurrent() const { return m_isCurrent; }

    void setPreviewDark(bool isDark);
    void setCurrent(bool isCurrent);

signals:
    void applyRequested(const QString &paletteName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void updateSwatches();
    void updatePaletteColors();

    ThemePalette m_palette;
    bool m_previewDark = true;
    bool m_isCurrent = false;
    bool m_isHovered = false;

    QLabel *m_nameLabel = nullptr;
    QLabel *m_badgeLabel = nullptr;
    QLabel *m_currentBadge = nullptr;
    PalettePreviewWidget *m_previewWidget = nullptr;
    QPushButton *m_applyBtn = nullptr;
};

#endif // PALETTECARDWIDGET_H
