#include "CSJVideoRendererWidget.h"

#include <QDebug>

#include <chrono>
#include <thread>

CSJVideoRendererWidget::CSJVideoRendererWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderType(NONE_RENDERING) {

    setAttribute(Qt::WA_PaintOnScreen, true);
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

void CSJVideoRendererWidget::resizeEvent(QResizeEvent *event) {
    if (!m_spVideoRenderer) {
        m_spVideoRenderer = CSJVideoRenderer::getRendererInstance();
        m_spVideoRenderer->init(winId(), width(), height());
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

void CSJVideoRendererWidget::internalRender() {
    if (!m_spVideoRenderer) {
        m_spVideoRenderer = CSJVideoRenderer::getRendererInstance();
        m_spVideoRenderer->init(winId(), width(), height());
    }

    while (true) {
        if (m_exitRenderThread) {
            break;
        }

        m_spVideoRenderer->drawSence();

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
