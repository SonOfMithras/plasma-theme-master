#ifndef GLOBALTHEMEEDITOR_H
#define GLOBALTHEMEEDITOR_H

#include <QWidget>
#include <QListWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QMap>

class GlobalThemeEditor : public QWidget {
    Q_OBJECT
public:
    explicit GlobalThemeEditor(QWidget *parent = nullptr);

private slots:
    void refreshList();
    void onThemeSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void cloneTheme();
    void saveEasyDefaults();
    void restoreDefaults();
    void saveRawDefaults();

private:
    void initUi();
    void populateCombos();
    void parseDefaultsToUi(const QString &content);
    
    QListWidget *m_themeList;
    QTabWidget *m_editorTabs;
    
    // Easy Editor Widgets
    QWidget *m_easyWidget;
    // Map: Key -> QComboBox
    // We need a way to map UI fields to Config keys.
    // Let's use a struct or map.
    struct ConfigKey {
        QString section;
        QString key;
        bool operator<(const ConfigKey &other) const {
             if (section != other.section) return section < other.section;
             return key < other.key;
        }
    };
    QMap<ConfigKey, QComboBox*> m_combos;
    QMap<ConfigKey, QString> m_comboCategories; // Map key to category for listing
    
    // Raw Editor Widgets
    QWidget *m_rawWidget;
    QTextEdit *m_rawEditor;
};

#endif // GLOBALTHEMEEDITOR_H
