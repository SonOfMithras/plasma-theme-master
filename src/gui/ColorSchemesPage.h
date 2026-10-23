#ifndef COLORSCHEMESPAGE_H
#define COLORSCHEMESPAGE_H

#include <QWidget>
#include <QList>
#include <QString>

class QLineEdit;
class QButtonGroup;
class QLabel;
class QPushButton;
class FlowLayout;
class PaletteCardWidget;

class ColorSchemesPage : public QWidget {
    Q_OBJECT
public:
    explicit ColorSchemesPage(QWidget *parent = nullptr);
    ~ColorSchemesPage() override;

    void refreshCurrentPalette();
    void syncWithEnforcedState();
    void updateSyncLockState();

private slots:
    void onSearchChanged(const QString &text);
    void onFilterGroupChanged(int id);
    void onPreviewModeToggled(bool isDark);
    void onApplyPalette(const QString &paletteName);
    void onSyncLockToggled();

private:
    void setupUi();
    void populatePalettes();
    void filterPalettes();

    QLineEdit       *m_searchEdit = nullptr;
    QButtonGroup    *m_filterGroup = nullptr;
    QPushButton     *m_darkPreviewBtn = nullptr;
    QPushButton     *m_lightPreviewBtn = nullptr;
    QPushButton     *m_syncLockBtn = nullptr;
    QLabel          *m_countLabel = nullptr;
    QLabel          *m_statusLabel = nullptr;

    QWidget         *m_cardsContainer = nullptr;
    FlowLayout      *m_flowLayout = nullptr;
    QList<PaletteCardWidget *> m_cardWidgets;

    int              m_activeFilter = 0; // 0 = All, 1 = Bundled, 2 = User
    QPushButton     *m_filterAllBtn = nullptr;
    QPushButton     *m_filterBundledBtn = nullptr;
    QPushButton     *m_filterUserBtn = nullptr;
    bool             m_previewDark = true;
    bool             m_syncLocked = true;
};

#endif // COLORSCHEMESPAGE_H
