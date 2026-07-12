#include "CSJVideoRendererWidget.h"

#include <QPainter>
#include <QDebug>

#include <chrono>
#include <thread>

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"

using namespace csjutils;
using namespace csjrenderengine;

CSJVideoRendererWidget::CSJVideoRendererWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderType(NONE_RENDERING) {

    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground);

    setAutoFillBackground(false);
}

CSJVideoRendererWidget::~CSJVideoRendererWidget() {
    qDebug() << "CSJVideoRendererWidget destoryed!";

    if (m_pVideoRenderer) {
        m_pVideoRenderer->stopRender();
    }
}

void CSJVideoRendererWidget::setImagePath(QString &image_path) {
    m_imagePath = image_path;
}

void CSJVideoRendererWidget::setMediaFile(QString & media_file_path) {
    
}

void CSJVideoRendererWidget::setRenderDelegate(CSJRenderDelegatePtr delegate) {
    m_pRenderDelegate = delegate;
}

void CSJVideoRendererWidget::showDefaultImage() {
    if (!m_pVideoRenderer)  {
        return ;
    }

    std::string imageName("cross_street.jpeg");
    std::string imagePath = CSJPathTool::getImageWithName(imageName);
    qDebug() << "Image Path: " << imagePath;

#if defined(__APPLE__)
    m_pVideoRenderer->setImage(imagePath);
#else
    m_pVideoRenderer->setImage(imagePath);// "resources/Images/cross_street.jpeg"
#endif
}

void CSJVideoRendererWidget::onUpdateFrame() {
    
}

void CSJVideoRendererWidget::showEvent(QShowEvent *event) {
    if (!initRenderer()) {
        return ;
    }

    if (m_pVideoRenderer) {

        m_pVideoRenderer->setRenderDelegate(m_pRenderDelegate);

        LOG_Info("Starts rendering ... ");
        m_pVideoRenderer->startRender();
        LOG_Info("Rendering started ... ");
    }

    QWidget::showEvent(event);
}

void CSJVideoRendererWidget::closeEvent(QCloseEvent * event) {
    if (!m_pVideoRenderer) {
        m_pVideoRenderer->stopRender();
    }

    QWidget::closeEvent(event);
}

void CSJVideoRendererWidget::resizeEvent(QResizeEvent *event) {
    if (m_pVideoRenderer) {
        m_pVideoRenderer->resize(width(), height(), devicePixelRatio());
    }

    QWidget::resizeEvent(event);
}

void CSJVideoRendererWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
}

void CSJVideoRendererWidget::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
}

void CSJVideoRendererWidget::keyReleaseEvent(QKeyEvent *event) {
    QWidget::keyReleaseEvent(event);
}

void CSJVideoRendererWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
}

void CSJVideoRendererWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void CSJVideoRendererWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
}

void CSJVideoRendererWidget::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);
}

bool CSJVideoRendererWidget::initRenderer() {
    if (!m_pVideoRenderer) {
        m_pVideoRenderer = CSJVideoRendererPtr(createCSJRenderer(reinterpret_cast<void*>(winId()), 
                                                                    width(), 
                                                                    height(),
                                                                    devicePixelRatio()));
    }
    
    return true;
}
