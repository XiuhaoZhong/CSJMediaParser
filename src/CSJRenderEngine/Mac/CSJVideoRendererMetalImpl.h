#ifndef __CSJVIDEORENDERERMETAL_H__
#define __CSJVIDEORENDERERMETAL_H__

#include "CSJRenderEngine/CSJVideoRenderer.h"
#include "CSJVSyncHandler.h"

#import "CSJMetalRenderer.h"

namespace csjrenderengine {

class CSJVideoRendererMetalImpl: public CSJVideoRenderer {
public:
    CSJVideoRendererMetalImpl();
    CSJVideoRendererMetalImpl(CSJWindowID widgetID, int width, int height, float pixelRatio);
    ~CSJVideoRendererMetalImpl();

    void startRender() override;
    void stopRender() override;

    void resize(int width, int height, float pixelRatio) override;

    virtual void initialRenderComponents(CSJPixelFormat fmtType,
                                         int width, int height) override;

    virtual void updateVideoFrame(CSJVideoFramePtr videoData) override;

    virtual void setImage(const std::string& imagePath) override;

protected:
    bool init();
    void draw(double timeStamp);
    bool updateScene(double timeStamp);

private:
    std::atomic<bool> m_bInitSuccess = false;
    int               m_iWidth;
    int               m_iHeight;
    float             m_fPixelRatio;
    CSJWindowID       m_pWinID;
    CSJMetalRenderer *m_pRenderer;
    CSJVSyncPtr       m_pVSyncHandler;
};

} // csjrenderengine

#endif // __CSJVIDEORENDERERMETAL_H__
