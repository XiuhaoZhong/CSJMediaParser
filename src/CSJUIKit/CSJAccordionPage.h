#ifndef __CSJACCORDIONPAGE_H__
#define __CSJACCORDIONPAGE_H__

#include <QWidget>
#include <QString>

class QVBoxLayout;

class QPushButton;
class CSJAccordionPage : public QWidget {
    Q_OBJECT
public:
    CSJAccordionPage(QWidget *parent = nullptr);
    ~CSJAccordionPage();

    void setTitle(QString &title);
    void setStringItems(QVector<QString> &itemStringArray);
    void setWidgetItems(QVector<QWidget *> &itemWidgetArray);
    void expand();
    void collapse();

protected:
    void initUI();

protected slots:
    void onTitleBtnClicked();

private:
    QVBoxLayout *m_pLayout = nullptr;
    QString      m_title;
    QPushButton *m_pTitleBtn = nullptr;
    QWidget     *m_pContentWidget = nullptr;

    bool         m_isExpand = true;
};

#endif // __CSJACCORDIONPAGE_H__
