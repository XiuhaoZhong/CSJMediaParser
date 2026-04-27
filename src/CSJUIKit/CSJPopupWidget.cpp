#include "CSJPopupWidget.h"

#include <QWidget>
#include <QLabel>

CSJPopupWidget::CSJPopupWidget() {
    m_pWidget = new QWidget();
    m_pWidget->setFixedSize(QSize(300, 200));
    m_pWidget->setVisible(false);

    QLabel *textLabel = new QLabel(m_pWidget);
    textLabel->setText("this is a popup widget!");
}

CSJPopupWidget::~CSJPopupWidget() {

}

void CSJPopupWidget::show() {
    m_pWidget->setVisible(true);
}
