#include "CSJWidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QStyle>
#include <QPainter>
#include <QLinearGradient>
#include <QPainterPath>

#include <qDebug>

CSJWidget::CSJWidget(QWidget *parent) 
    : QWidget(parent) {

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    // setAttribute(Qt::WA_TranslucentBackground, true);
    // setAttribute(Qt::WA_OpaquePaintEvent, false);
    setFixedSize(800, 600);

    createTitleBar();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    mainLayout->addWidget(m_titleBar);

    m_centerWidget = new QWidget();
    m_centerWidget->setStyleSheet("background-color: #F5F5F5;");
    QLabel *contentLabel = new QLabel("主内容区域", m_centerWidget);
    contentLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_centerWidget, 1); // 占满剩余空间

    setLayout(mainLayout);

    setObjectName("CSJWidget");
    setStyleSheet(R"(
        QWidget#CSJWidget {
            corner-radius: 5px;
            background-color: #4E94CE;
        }    
    )");
}

void CSJWidget::setContentWidget(QWidget *contentWidget) {
    if (!contentWidget) {
        return ;
    }

    QVBoxLayout *contentLayout = new QVBoxLayout(m_centerWidget);
    contentLayout->addWidget(contentWidget, 1);
}

void CSJWidget::mousePressEvent(QMouseEvent *event) {
    if (event->y() <= m_titleBar->height() && event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void CSJWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() & Qt::LeftButton) {
        if (!m_dragPos.isNull()) {
            m_dragPos = QPoint();
        }
    }
}

void CSJWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !m_dragPos.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

// void CSJWidget::paintEvent(QPaintEvent *event) {
//     Q_UNUSED(event);
//     QPainter painter(this);
//     painter.setRenderHint(QPainter::Antialiasing, true); // 开启抗锯齿，圆角边缘平滑

//     painter.fillRect(rect(), Qt::transparent);

//     // 3. 构建窗口自身的圆角路径（基于整个窗口的矩形区域）
//     QPainterPath windowPath;
//     int cornerRadius = 5; // 窗口圆角半径（可自定义）
//     windowPath.addRoundedRect(rect(), cornerRadius, cornerRadius);

//     // 4. 绘制窗口自身圆角背景（支持纯色/渐变/贴图，仅作用于窗口本身）
//     // 方式1：纯色背景（简单直观）
//     //painter.fillPath(windowPath, QColor(0x4E, 0x94, 0xCE));
//     painter.fillPath(windowPath, QColor(0xFF, 0xFF, 0xFF));

//     // 方式2：渐变背景（可选，替换纯色背景即可，视觉效果更丰富）
// //        QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
// //        gradient.setColorAt(0, QColor(0x2E, 0x64, 0xAA));
// //        gradient.setColorAt(1, QColor(0x4E, 0x94, 0xCE));
// //        painter.fillPath(windowPath, gradient);

//     // 5. 绘制窗口自身圆角边框（可选，增强圆角视觉层次感）
//     //painter.setPen(QPen(QColor(0x1E, 0x4E, 0x8E), 2));
//     painter.drawPath(windowPath);
// }

void CSJWidget::createTitleBar() {
    m_titleBar = new QWidget();
    m_titleBar->setFixedHeight(35); 

    m_titleBar->setStyleSheet(R"(
        QWidget#TitleBar {
            background-color: #4E94CE; /* 标题栏背景色（与主题主色调一致） */
        }
        QLabel {
            color: white; /* 标题文字色 */
            font-size: 16px;
            font-weight: 500;
        }
        QPushButton {
            background-color: transparent; /* 按钮透明背景 */
            color: white; /* 按钮文字/图标色 */
            border: none;
            width: 48px;
            height: 48px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.2); /* hover高亮效果 */
        }
    )");

    m_titleBar->setObjectName("TitleBar");

    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(16, 0, 0, 0);
    titleLayout->setSpacing(0);

    QLabel *titleLabel = new QLabel("自定义标题栏（可设置颜色）");
    titleLayout->addWidget(titleLabel);

    QWidget *btnWidget = new QWidget();
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(0);

    QPushButton *minBtn = new QPushButton();
    minBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);

    QPushButton *maxBtn = new QPushButton();
    maxBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    connect(maxBtn, &QPushButton::clicked, this, [=]() {
        isMaximized() ? showNormal() : showMaximized();
        maxBtn->setIcon(style()->standardIcon(isMaximized() ? QStyle::SP_TitleBarNormalButton : QStyle::SP_TitleBarMaxButton));
    });

    QPushButton *closeBtn = new QPushButton();
    closeBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    btnLayout->addWidget(minBtn);
    btnLayout->addWidget(maxBtn);
    btnLayout->addWidget(closeBtn);
    titleLayout->addStretch(); 
    titleLayout->addWidget(btnWidget);
}