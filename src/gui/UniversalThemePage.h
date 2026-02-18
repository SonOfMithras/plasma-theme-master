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
    void showBDSettings(); // New slot

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    QCheckBox *m_vscodeCheck;
    QPushButton *m_vscodeRestoreBtn;

    QCheckBox *m_firefoxCheck;
    QPushButton *m_firefoxRestoreBtn;

    QCheckBox *m_discordCheck;
    QPushButton *m_discordRestoreBtn;
    QPushButton *m_discordConfigBtn;

    QCheckBox *m_kittyCheck;
    QPushButton *m_kittyRestoreBtn;

    QCheckBox *m_konsoleCheck;
    QPushButton *m_konsoleRestoreBtn;

    QCheckBox *m_genericCheck;
    QPushButton *m_genericRestoreBtn;

    QCheckBox *m_obsidianCheck;
    QPushButton *m_obsidianRestoreBtn;
    
    QLineEdit *m_obsidianPathEdit;
    QPushButton *m_browseObsidianBtn;

    QPushButton *m_syncButton;

    bool m_isLoading = false;
};

#endif // UNIVERSALTHEMEPAGE_H
