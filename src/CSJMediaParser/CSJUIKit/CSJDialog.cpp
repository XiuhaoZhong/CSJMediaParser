#include "CSJDialog.h"

#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

CSJDialog::CSJDialog(QString &content, QWidget *parent, CSJDIALOGTYPE type)
    : QDialog(parent)
    , m_content(content) {

    setFixedSize(QSize(300, 200));

    initUI();
}

CSJDialog::~CSJDialog() {

}

void CSJDialog::open() {

}

void CSJDialog::done(int) {
    std::cout << "function done!" << std::endl;
    QDialog::done(1);
}

void CSJDialog::accept() {
    std::cout << "function accept!" << std::endl;
    done(1);
}

void CSJDialog::reject() {
    std::cout << "function reject!" << std::endl;
    done(0);
}

void CSJDialog::initUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    QLabel *label = new QLabel(this);
    label->setFixedWidth(275);
    label->setText(m_content);
    label->setWordWrap(true);
    label->setStyleSheet("background-color:#C7ABAB");

    QPushButton *closeBtn = new QPushButton(this);
    closeBtn->setFixedWidth(80);
    closeBtn->setText("确定");
    connect(closeBtn, SIGNAL(pressed()), this, SLOT(accept()));

    mainLayout->addWidget(label);
    mainLayout->addWidget(closeBtn);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setAlignment(closeBtn, Qt::AlignHCenter);
}

// This function is already completed, just add this layout into mainlayout.
QHBoxLayout *CSJDialog::createBtnLayout() {
    QPushButton *okBtn = new QPushButton(this);
    okBtn->setFixedWidth(80);
    okBtn->setText("确定");
    connect(okBtn, SIGNAL(pressed()), this, SLOT(accept()));

    QPushButton *cancelBtn = new QPushButton(this);
    cancelBtn->setFixedWidth(80);
    cancelBtn->setText("取消");
    connect(cancelBtn, SIGNAL(pressed()), this, SLOT(reject()));

    QHBoxLayout *hboxLayout = new QHBoxLayout();
    QVBoxLayout *leftBoxLayout = new QVBoxLayout();
    QVBoxLayout *rightBoxLayout = new QVBoxLayout();

    hboxLayout->addLayout(leftBoxLayout, 1);
    hboxLayout->addLayout(rightBoxLayout, 1);

    leftBoxLayout->addWidget(cancelBtn);
    leftBoxLayout->setAlignment(cancelBtn, Qt::AlignHCenter);

    rightBoxLayout->addWidget(okBtn);
    rightBoxLayout->setAlignment(okBtn, Qt::AlignHCenter);

    return hboxLayout;
}
