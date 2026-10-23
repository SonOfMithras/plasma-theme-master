#ifndef UNIVERSALTHEMEPAGE_H
#define UNIVERSALTHEMEPAGE_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QMap>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QProgressBar;
class QLabel;
class QButtonGroup;
class FlowLayout;
class TemplateCardWidget;

class UniversalThemePage : public QWidget {
    Q_OBJECT
public:
    explicit UniversalThemePage(QWidget *parent = nullptr);
    ~UniversalThemePage() override;

private slots:
    void onSearchTextChanged(const QString &text);
    void onCategoryFilterChanged(int id);
    void onInstalledOnlyToggled(bool checked);
    void onAppToggled(const QString &appId, bool enabled);
    void onConfigureClicked(const QString &appId);
    void onRestoreClicked(const QString &appId);

    void onSyncClicked();
    void onSyncStarted(quint64 generation);
    void onSyncProgress(quint64 generation, int current, int total, const QString &appName);
    void onSyncFinished(quint64 generation, bool success, const QStringList &warnings);

private:
    void setupUi();
    void populateCards();
    void filterCards();
    void updateCounts();

    // App Templates Controls & Cards
    QLineEdit       *m_searchEdit = nullptr;
    QButtonGroup    *m_categoryGroup = nullptr;
    QCheckBox       *m_installedOnlyCheck = nullptr;
    QLabel          *m_countLabel = nullptr;

    QWidget         *m_cardsContainer = nullptr;
    FlowLayout      *m_flowLayout = nullptr;
    QList<TemplateCardWidget *> m_cardWidgets;

    QLabel          *m_statusLabel = nullptr;
    QProgressBar    *m_progressBar = nullptr;
    QPushButton     *m_syncButton = nullptr;

    QString          m_activeCategory;
    int              m_activeSourceFilter = 0; // 0 = All, 1 = Bundled, 2 = User
    QButtonGroup    *m_sourceGroup = nullptr;
    QPushButton     *m_sourceAllBtn = nullptr;
    QPushButton     *m_sourceBundledBtn = nullptr;
    QPushButton     *m_sourceUserBtn = nullptr;
    bool             m_isLoading = false;
};

#endif // UNIVERSALTHEMEPAGE_H
