#include "CSJAccordionWidget.h"

#include <QVBoxLayout>
#include <QScrollArea>

#include "CSJAccordionPage.h"

CSJAccordionWidget::CSJAccordionWidget(QWidget *parent)
    : QWidget(parent) {

    m_pCenterWidget = new QWidget();

    m_pScrollArea = new QScrollArea(this);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setWidget(m_pCenterWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_pScrollArea);

    m_pLayout = new QVBoxLayout(m_pCenterWidget);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setSpacing(2);
    m_pLayout->setAlignment(Qt::AlignTop);
}

CSJAccordionWidget::~CSJAccordionWidget() {

}

void CSJAccordionWidget::addPageWithTitle(QString &title,
                                         QVector<QString> &itemStringArray) {
    CSJAccordionPage *page = new CSJAccordionPage(m_pCenterWidget);
    m_pLayout->addWidget(page);
    page->setTitle(title);
    page->setStringItems(itemStringArray);
}

void CSJAccordionWidget::addPageWithTitle(QString &title,
                                         QVector<QWidget *> &itemWidgetArray) {

}

void CSJAccordionWidget::showTestPage() {
    QVector<QString> strArray = {"type: video", "fps: 30", "width: 1280", "height: 960"};
    QString title = "Track 01";
    addPageWithTitle(title, strArray);

    QVector<QString> audioAttrArray = {"type: video", "fps: 30", "width: 1280", "height: 960"};
    QString audioTitle = "Track 02";
    addPageWithTitle(audioTitle, audioAttrArray);

    //QVector<QString> audioAttrArray = {"type: video", "fps: 30", "width: 1280", "height: 960"};
    QString title1 = "Track 03";
    addPageWithTitle(title1, audioAttrArray);

    //QVector<QString> audioAttrArray = {"type: video", "fps: 30", "width: 1280", "height: 960"};
    QString title2 = "Track 04";
    addPageWithTitle(title2, audioAttrArray);

    m_pCenterWidget->show();
}


