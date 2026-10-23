#ifndef TEMPLATECARDWIDGET_H
#define TEMPLATECARDWIDGET_H

#include <QFrame>
#include <QString>
#include "../core/CommunityTemplateCatalog.h"

class QCheckBox;
class QLabel;
class QPushButton;

class TemplateCardWidget : public QFrame {
    Q_OBJECT
public:
    explicit TemplateCardWidget(const CommunityTemplateApp &app, QWidget *parent = nullptr);
    explicit TemplateCardWidget(const QString &id,
                                const QString &name,
                                const QString &category,
                                const QString &desc,
                                bool installed,
                                QWidget *parent = nullptr);
    ~TemplateCardWidget() override;

    QString appId() const { return m_appId; }
    QString category() const { return m_category; }
    QString displayName() const { return m_name; }
    QString description() const { return m_description; }
    bool isInstalled() const { return m_isInstalled; }
    bool isUser() const { return m_isUser; }
    bool isEnabled() const;

    void setChecked(bool checked);

signals:
    void appToggled(const QString &appId, bool enabled);
    void configureClicked(const QString &appId);
    void restoreClicked(const QString &appId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void buildRichTooltip();
    void updatePaletteColors();

    QString m_appId;
    QString m_name;
    QString m_category;
    QString m_description;
    QString m_iconName;
    bool m_isInstalled = false;
    bool m_isUser = false;
    bool m_isHovered = false;

    QLabel *m_nameLabel = nullptr;
    QLabel *m_catBadge = nullptr;
    QLabel *m_descLabel = nullptr;
    QLabel *m_statusBadge = nullptr;
    QCheckBox *m_toggle = nullptr;
    QPushButton *m_configBtn = nullptr;
    QPushButton *m_restoreBtn = nullptr;
};

#endif // TEMPLATECARDWIDGET_H
