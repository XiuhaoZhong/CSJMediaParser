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

void CSJVideoRendererWidget::initializeVideoInfo(CSJVideoFormatType fmtType, int width, int height) {
    if (!m_pVideoRenderer) {
        return ;
    }

    m_pVideoRenderer->initialRenderComponents(fmtType, width, height);
}

void CSJVideoRendererWidget::updateVideoFrame(CSJVideoData *videoData) {
    if (!videoData) {
        return ;
    }

    if (!m_pVideoRenderer) {
        return ;
    }

    m_pVideoRenderer->updateVideoFrame(videoData);
    m_pVideoRenderer->drawScene();
}

void CSJVideoRendererWidget::setImagePath(QString &image_path) {
    m_imagePath = image_path;
}

void CSJVideoRendererWidget::setMediaFile(QString & media_file_path) {
    
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
    m_pVideoRenderer->setImage("resources/Images/cross_street.jpeg");
#endif
}

void CSJVideoRendererWidget::onUpdateFrame() {
    
}

void CSJVideoRendererWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    if (!initRenderer()) {
        return ;
    }

    LOG_Info("Starts rendering ... ");
    m_pVideoRenderer->startRender();
    LOG_Info("Rendering started ... ");
}

void CSJVideoRendererWidget::closeEvent(QCloseEvent * event) {
    if (!m_pVideoRenderer) {
        m_pVideoRenderer->stopRender();
    }
}

void CSJVideoRendererWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    if (!initRenderer()) {
        return ;
    }

#ifdef __APPLE__
    m_pVideoRenderer->updateDrawableSize(width(), height(), devicePixelRatio());
#elif _WIN32
    m_pVideoRenderer->resize(width(), height());
#else

#endif
}

void CSJVideoRendererWidget::paintEvent(QPaintEvent *event) {

}

void CSJVideoRendererWidget::keyPressEvent(QKeyEvent *event) {

}

void CSJVideoRendererWidget::keyReleaseEvent(QKeyEvent *event) {

}

void CSJVideoRendererWidget::mousePressEvent(QMouseEvent *event) {

}

void CSJVideoRendererWidget::mouseReleaseEvent(QMouseEvent *event) {

}

void CSJVideoRendererWidget::mouseMoveEvent(QMouseEvent *event) {

}

void CSJVideoRendererWidget::wheelEvent(QWheelEvent *event) {

}

bool CSJVideoRendererWidget::initRenderer() {
    // TODO: initialze the renderer by offscreen rendering.

    if (!m_pVideoRenderer) {
        if (!m_pVideoRenderer) {
            m_pVideoRenderer = CSJVideoRendererPtr(createCSJRenderer());//CSJVideoRenderer::getRendererInstance();
            /**
             * Currently, there will be a problem if use offscreen rendering when use 
             * customized CSJWidget, the problem is that if I extend a QWidget to customize
             * the UI style, there must set WA_TranslucentBackground attribte, and then 
             * the DirectX/Metal rendering will conflict with qt, so now I won't use the 
             * customized CSJWidget, and use the default UI style to compelte the functionalities
             * first.
             */
            //m_pVideoRenderer->initForOffScreen(width(), height());
#if __APPLE__
            m_pVideoRenderer->init(reinterpret_cast<void*>(winId()), width(), height(), devicePixelRatio());
#elif _WIN32
            m_pVideoRenderer->init(reinterpret_cast<void*>(winId()), width(), height());
#endif
        }
    }

    return true;
}

void CSJVideoRendererWidget::internalRender() {
    if (!initRenderer()) {
        return ;
    }

    while (true) {
        if (m_exitRenderThread) {
            break;
        }

        m_pVideoRenderer->drawScene();

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
