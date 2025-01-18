#include "CSJVideoRendererWidget.h"

#include <QDebug>

#include <chrono>
#include <thread>

CSJVideoRendererWidget::CSJVideoRendererWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderType(RENDER_WITH_EVENT) {

    setAttribute(Qt::WA_PaintOnScreen, true);
}

CSJVideoRendererWidget::~CSJVideoRendererWidget() {
    qDebug() << "CSJVideoRendererWidget destoryed!";
}

void CSJVideoRendererWidget::setRenderType(CSJVideoRenderType renderType) {
    if (m_renderType == renderType) {
        return ;
    }

    if (m_renderType == RENDER_WITH_TIMER) {
        m_exitRenderThread = true;
    }

    switch (renderType) {
    case RENDER_WITH_EVENT:
    case RENDER_WITH_OTHER_CORE:
        break;
    case RENDER_WITH_TIMER:
        m_pRenderThread.reset(new std::thread(&CSJVideoRendererWidget::internalRender, this));
        break;
    }

    m_renderType = renderType;
}

void CSJVideoRendererWidget::initializeVideoInfo(CSJVideoFormatType fmtType, int width, int height) {
    if (!m_spVideoRenderer) {
        return ;
    }

    m_spVideoRenderer->loadVideoComponents(fmtType, width, height);
}

void CSJVideoRendererWidget::updateVideoFrame(CSJVideoData *videoData) {
    if (!videoData) {
        return ;
    }

    if (!m_spVideoRenderer) {
        return ;
    }

    m_spVideoRenderer->updateVideoFrame(videoData);
}

void CSJVideoRendererWidget::resizeEvent(QResizeEvent *event) {
    if (!m_spVideoRenderer) {
        m_spVideoRenderer = CSJVideoRenderer::getRendererInstance();
        m_spVideoRenderer->init(winId(), width(), height());
    }

    m_spVideoRenderer->resize(width(), height());
}

void CSJVideoRendererWidget::paintEvent(QPaintEvent *event) {
    if (!m_spVideoRenderer || m_renderType != RENDER_WITH_EVENT) {
        return ;
    }

    m_spVideoRenderer->drawSence();
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

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}
