#include "CSJVideoRendererWidget.h"

#include <QPainter>
#include <QDebug>

#include <chrono>
#include <thread>

CSJVideoRendererWidget::CSJVideoRendererWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderType(NONE_RENDERING) {

    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    //connect(this, &CSJVideoRendererWidget::updateFrame, this, &CSJVideoRendererWidget::onUpdateFrame);
}

CSJVideoRendererWidget::~CSJVideoRendererWidget() {
    qDebug() << "CSJVideoRendererWidget destoryed!";

    m_exitRenderThread = true;
    if (m_pRenderThread->joinable()) {
        m_pRenderThread->join();
    }
}

void CSJVideoRendererWidget::setRenderType(RenderMode renderType) {
    if (m_renderType == renderType) {
        return ;
    }

    if (m_renderType == ACTIVE_RENDERING) {
        m_exitRenderThread = true;
        if (m_pRenderThread->joinable()) {
            m_pRenderThread->join();
        }
    }

    switch (renderType) {
    case ACTIVE_RENDERING:
        m_pRenderThread.reset(new std::thread(&CSJVideoRendererWidget::internalRender, this));
        break;
    }

    m_renderType = renderType;
}

void CSJVideoRendererWidget::initializeVideoInfo(CSJVideoFormatType fmtType, int width, int height) {
    if (!m_spVideoRenderer) {
        return ;
    }

    m_spVideoRenderer->initialRenderComponents(fmtType, width, height);
}

void CSJVideoRendererWidget::updateVideoFrame(CSJVideoData *videoData) {
    if (!videoData) {
        return ;
    }

    if (!m_spVideoRenderer) {
        return ;
    }

    m_spVideoRenderer->updateVideoFrame(videoData);
    m_spVideoRenderer->drawSence();
}

void CSJVideoRendererWidget::setImagePath(QString &image_path) {
    m_imagePath = image_path;
}

void CSJVideoRendererWidget::showDefaultImage() {
    if (!m_spVideoRenderer)  {
        return ;
    }

    m_spVideoRenderer->setImage("resources/Images/cross_street.jpeg");
}

void CSJVideoRendererWidget::onUpdateFrame() {
    update();
}

void CSJVideoRendererWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
}

void CSJVideoRendererWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    if (!initRenderer()) {
        return ;
    }

    m_spVideoRenderer->resize(width(), height());
}

void CSJVideoRendererWidget::paintEvent(QPaintEvent *event) {
    // if (!m_spVideoRenderer || m_renderType != RENDER_WITH_EVENT) {
    //     return ;
    // }

    // m_spVideoRenderer->drawSence();
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

    if (!m_spVideoRenderer) {
        if (!m_spVideoRenderer) {
            m_spVideoRenderer = CSJVideoRenderer::getRendererInstance();
            /**
             * Currently, there will be a problem if use offscreen rendering when use 
             * customized CSJWidget, the problem is that if I extend a QWidget to customize
             * the UI style, there must set WA_TranslucentBackground attribte, and then 
             * the DirectX/Metal rendering will conflict with qt, so now I won't use the 
             * customized CSJWidget, and use the default UI style to compelte the functionalities
             * first.
             */
            //m_spVideoRenderer->initForOffScreen(width(), height());
            m_spVideoRenderer->init(winId(), width(), height());
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

        m_spVideoRenderer->drawSence();

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
