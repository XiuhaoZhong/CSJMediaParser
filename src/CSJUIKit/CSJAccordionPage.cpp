#include "CSJAccordionPage.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

CSJAccordionPage::CSJAccordionPage(QWidget *parent) {

    m_pLayout = new QVBoxLayout(this);
    m_pLayout->setContentsMargins(0, 2, 0, 2);
    m_pLayout->setAlignment(Qt::AlignTop);
    m_pLayout->setSpacing(1);

    m_pTitleBtn = new QPushButton(this);
    m_pTitleBtn->setStyleSheet("QPushButton{text-align : left;}");
    m_pLayout->addWidget(m_pTitleBtn);
    connect(m_pTitleBtn, SIGNAL(pressed()), this, SLOT(onTitleBtnClicked()));

    m_pContentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(m_pContentWidget);
    m_pLayout->addWidget(m_pContentWidget);

    // QPalette pale(this->palette());
    // pale.setColor(QPalette::Active, QPalette::Window, QColor(0,0,0));
    // this->setAutoFillBackground(true);
    // this->setPalette(pale);
}

CSJAccordionPage::~CSJAccordionPage() {

}

void CSJAccordionPage::setTitle(QString &title) {
    if (!m_pTitleBtn) {
        return ;
    }

    m_pTitleBtn->setText(title);
}

void CSJAccordionPage::setStringItems(QVector<QString> &itemStringArray) {
    if (!m_pContentWidget || itemStringArray.size() == 0) {
        return ;
    }

    QLayout *detailLayout = m_pContentWidget->layout();
    for (auto &item : itemStringArray) {
        QLabel *label = new QLabel(m_pContentWidget);
        label->setText(item);
        detailLayout->addWidget(label);
    }
}

void CSJAccordionPage::setWidgetItems(QVector<QWidget *> &itemWidgetArray) {
    if (!m_pContentWidget || itemWidgetArray.size() == 0) {
        return ;
    }

    QLayout *detailLayout = m_pContentWidget->layout();
    for (auto item : itemWidgetArray) {
        detailLayout->addWidget(item);
    }
}

void CSJAccordionPage::expand() {
    if (m_isExpand) {
        return ;
    }

    m_isExpand = true;
    m_pContentWidget->show();
}

void CSJAccordionPage::collapse() {
    if (!m_isExpand) {
        return ;
    }

    m_isExpand = false;
    m_pContentWidget->hide();
}

void CSJAccordionPage::initUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_pTitleBtn = new QPushButton();
    mainLayout->addWidget(m_pTitleBtn);

    m_pContentWidget = new QWidget();
    mainLayout->addWidget(m_pContentWidget);

    QVBoxLayout *detailLayout = new QVBoxLayout(m_pContentWidget);
}

void CSJAccordionPage::onTitleBtnClicked() {
    if (m_isExpand) {
        collapse();
        m_isExpand = false;
    } else {
        expand();
        m_isExpand = true;
    }

}
