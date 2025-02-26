#ifndef __CSJVIDEORENDERERWIDGET_H__
#define __CSJVIDEORENDERERWIDGET_H__

#include <QWidget>

#include <memory>
#include <thread>

#include "renderClient/CSJVideoRenderer.h"
#include "player/CSJVideoPresentDelegate.h"

typedef enum {
    /**
     * The render function invoked int the paintEvent function
     */
    RENDER_WITH_EVENT = 0,
    /**
     * The render function invoked by other controller, such as
     * a player core, which will invoke the render function when
     * a video frame should be rendered.
     */
    RENDER_WITH_OTHER_CORE,
    /**
     * CSJVideoRendererWidget will start a thread to execute the
     * render function, and users should set the FPS. The default
     * FPS is 25.
     */
    RENDER_WITH_TIMER
} CSJVideoRenderType;

class CSJVideoRendererWidget : public QWidget
                             , public CSJVideoPresentDelegate {
    Q_OBJECT
public:
    CSJVideoRendererWidget(QWidget *parent = nullptr);
    ~CSJVideoRendererWidget();

    QPaintEngine* paintEngine() const override {
        return NULL;
    }

    void setRenderType(CSJVideoRenderType renderType);

    /**********************************************************************
     * Override interfaces from CSJVideoPresentDelegate.
     ***********************************************************************/
    void initializeVideoInfo(CSJVideoFormatType fmtType,
                                     int width, int height) override;

    void updateVideoFrame(CSJVideoData *videoData) override;

public slots:
    void showDefaultImage();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void internalRender();

private:
    CSJSpVideoRenderer m_spVideoRenderer = nullptr;

    CSJVideoRenderType m_renderType;
    bool               m_exitRenderThread = false;

    std::unique_ptr<std::thread> m_pRenderThread;
};

using CSJSpPlayerWidget = std::shared_ptr<CSJVideoRendererWidget>;

#endif // __CSJVIDEORENDERERWIDGET_H__
