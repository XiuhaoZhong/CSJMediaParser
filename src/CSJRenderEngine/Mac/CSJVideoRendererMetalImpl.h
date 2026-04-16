#ifndef __CSJVIDEORENDERERMETAL_H__
#define __CSJVIDEORENDERERMETAL_H__

#include "CSJRenderEngine/CSJVideoRenderer.h"

#import "CSJMetalRenderer.h"

class CSJVideoRendererMetalImpl: public CSJVideoRenderer {
public:
    CSJVideoRendererMetalImpl();
    ~CSJVideoRendererMetalImpl();

    bool init(CSJWindowID widgetID, int width, int height) override;
    bool init(CSJWindowID widgetID, int width, int height, float pixelRatio) override;
    bool updateScene(double timeStamp) override;
    void drawScene() override;
    void updateDrawableSize(int width, int height, float pixelRatio) override;

    virtual void initialRenderComponents(CSJVideoFormatType fmtType,
                                         int width, int height) override;

    virtual void updateVideoFrame(CSJVideoData *videoData) override;

    virtual void setImage(const std::string& imagePath) override;

private:
    CSJMetalRenderer *m_pRenderer;
};

#endif // __CSJVIDEORENDERERMETAL_H__
