#include "CSJGlassWidget.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QGraphicsBlurEffect>

CSJGlassWidget::CSJGlassWidget(QWidget *parent)
    : QWidget(parent) {
    
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //m_bgWidget = new QWidget(this);

    m_blurEffect = new QGraphicsBlurEffect(this);
    m_blurEffect->setBlurRadius(20);
    m_blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
    //m_bgWidget->setGraphicsEffect(m_blurEffect);
    //this->setGraphicsEffect(m_blurEffect);

    // m_bgWidget->setStyleSheet(R"(
    //     QWidget {
    //         background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
    //                                     stop:0 rgba(255,255,255,0.2),
    //                                     stop:1 rgba(255,255,255,0.3));
    //         border-radius:  20px;
    //     }   
    // )");
}

void CSJGlassWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    //m_bgWidget->setGeometry(rect());
}

void CSJGlassWidget::paintEvent(QPaintEvent *event) {

    // Q_UNUSED(event);

    // //QWidget::paintEvent(event);

    // QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);

    // QPen pen(QColor(255, 255, 255, 50));
    // pen.setWidth(1);
    
    // painter.setPen(pen);
    // painter.setBrush(Qt::NoBrush);
    // painter.drawRoundedRect(rect(), 20, 20);


    // Q_UNUSED(event);
    // QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing, true);
    // painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // // 1. 创建圆角路径（与背景层圆角一致）
    // QPainterPath path;
    // path.addRoundedRect(rect(), 20, 20); // 圆角半径12px，与样式中一致

    // // 2. 设置裁剪路径：Widget仅绘制圆角区域，矩形区域被裁剪
    // painter.setClipPath(path);

    // // 3. 绘制玻璃背景（可选：补充底层颜色，避免漏色）
    // //painter.fillPath(path, QColor(30, 41, 59, 10)); // 极浅的背景色，覆盖底层漏出

    // // 4. 绘制圆角边框（增强轮廓，避免模糊边缘）
    // QPen pen(QColor(255, 255, 255, 30));
    // pen.setWidth(1);
    // pen.setCapStyle(Qt::RoundCap);
    // pen.setJoinStyle(Qt::RoundJoin);
    // painter.setPen(pen);
    // painter.drawPath(path);

    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 步骤1：创建圆角路径（裁剪核心）
    QPainterPath path;
    path.addRoundedRect(rect(), 20, 20); // 圆角半径12px
    painter.setClipPath(path);           // 强制裁剪为圆角，消除直角

    // 步骤2：绘制玻璃背景（渐变+半透，替代原bgWidget的样式）
    QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
    gradient.setColorAt(0, QColor(255, 255, 255, 80));  // 深灰蓝半透（alpha=50）
    gradient.setColorAt(0, QColor(255, 255, 255, 80)); 
    gradient.setColorAt(1, QColor(255, 255, 255, 80));  // 更深的半透
    painter.fillPath(path, gradient);                // 填充圆角路径

    // 步骤3：绘制圆角边框（避免边缘漏色）
    QPen pen(QColor(255, 255, 255, 255));
    pen.setWidth(1.5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(pen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 20, 20);
}
