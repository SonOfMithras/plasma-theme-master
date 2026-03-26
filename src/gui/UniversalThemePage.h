#ifndef UNIVERSALTHEMEPAGE_H
#define UNIVERSALTHEMEPAGE_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class UniversalThemePage : public QWidget {
    Q_OBJECT
public:
    explicit UniversalThemePage(QWidget *parent = nullptr);

private slots:
    void syncNow();
    void pickObsidianVault();
    void showBDSettings();
    void showVencordSettings();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    // Helper: create a restore button wired to a specific template name
    QPushButton *makeRestoreBtn(const QString &templateName,
                                const QString &displayName);

    QCheckBox   *m_vscodeCheck;
    QPushButton *m_vscodeRestoreBtn;

    QCheckBox   *m_firefoxCheck;
    QPushButton *m_firefoxRestoreBtn;

    QCheckBox   *m_discordCheck;
    QPushButton *m_discordRestoreBtn;
    QPushButton *m_discordConfigBtn;

    QCheckBox   *m_vencordCheck;
    QPushButton *m_vencordRestoreBtn;
    QPushButton *m_vencordConfigBtn;

    QCheckBox   *m_kittyCheck;
    QPushButton *m_kittyRestoreBtn;

    QCheckBox   *m_konsoleCheck;
    QPushButton *m_konsoleRestoreBtn;

    QCheckBox   *m_btopCheck;
    QPushButton *m_btopRestoreBtn;

    QCheckBox   *m_vicinaeCheck;
    QPushButton *m_vicinaeRestoreBtn;

    QCheckBox   *m_zedCheck;
    QPushButton *m_zedRestoreBtn;

    QCheckBox   *m_obsidianCheck;
    QPushButton *m_obsidianRestoreBtn;

    QLineEdit   *m_obsidianPathEdit;
    QPushButton *m_browseObsidianBtn;
    QPushButton *m_syncButton;

    bool m_isLoading = false;
};

#endif // UNIVERSALTHEMEPAGE_H
