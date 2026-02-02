#ifndef __CSJVIDEORENDERERWIDGET_H__
#define __CSJVIDEORENDERERWIDGET_H__

#include <QWidget>

#include <memory>
#include <thread>

#include "renderClient/CSJVideoRenderer.h"
#include "CSJMediaEngine/CSJVideoPresentDelegate.h"

using csjmediaengine::CSJVideoPresentDelegate;

/** 
 * The render mode of CSJVideoRendererWidget. 
 */
typedef enum {
    NONE_RENDERING = -1,

    /**
     * The widget schedules the render loop with a thread, and the defualt FPS is 30.
     * User can set the FPS if needed. This is the default mode.
     */
    ACTIVE_RENDERING = 0,
    /**
     * There isn't a render loop, the users just update the content by calling 
     * updateVideoFrame(...) function.
     */
    PASSIVE_RENDERING
} RenderMode;

class CSJVideoRendererWidget : public QWidget
                             , public CSJVideoPresentDelegate {
    Q_OBJECT
public:
    CSJVideoRendererWidget(QWidget *parent = nullptr);
    ~CSJVideoRendererWidget();

    QPaintEngine* paintEngine() const override {
        return NULL;
    }

    void setRenderType(RenderMode renderType);

    /**********************************************************************
     * Override interfaces from CSJVideoPresentDelegate.
     ***********************************************************************/
    void initializeVideoInfo(CSJVideoFormatType fmtType,
                             int width, int height) override;

    void updateVideoFrame(CSJVideoData *videoData) override;

    /**
     * Set the image path of the widget.
     */
    void setImagePath(QString& image_path);

public slots:
    void showDefaultImage();

protected:
    void showEvent(QShowEvent *event) override;
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

    RenderMode m_renderType;
    bool       m_exitRenderThread = false;
    QString    m_imagePath;

    std::unique_ptr<std::thread> m_pRenderThread;
};

using CSJSpPlayerWidget = std::shared_ptr<CSJVideoRendererWidget>;

#endif // __CSJVIDEORENDERERWIDGET_H__
