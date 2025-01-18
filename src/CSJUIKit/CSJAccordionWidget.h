#ifndef __CSJACCORDIONWIDGET_H__
#define __CSJACCORDIONWIDGET_H__

#include <QWidget>
#include <QVector>

class QScrollArea;
class QVBoxLayout;
class CSJAccordionPage;

class CSJAccordionWidget : public QWidget {
    Q_OBJECT
public:
    CSJAccordionWidget(QWidget *parent = nullptr);
    ~CSJAccordionWidget();

    void addPageWithTitle(QString &title, QVector<QString> &itemStringArray);
    void addPageWithTitle(QString &title, QVector<QWidget *> &itemWidgetArray);

    void showTestPage();
protected:


private:
    QWidget     *m_pCenterWidget = nullptr;
    QVBoxLayout *m_pLayout = nullptr;
    QScrollArea *m_pScrollArea = nullptr;
    QVector<CSJAccordionPage *> m_vPages;
};

#endif // __CSJACCORDIONWIDGET_H__
