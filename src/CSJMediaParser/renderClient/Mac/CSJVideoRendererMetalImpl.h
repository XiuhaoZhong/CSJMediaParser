#ifndef __CSJVIDEORENDERERMETAL_H__
#define __CSJVIDEORENDERERMETAL_H__

#include "renderClient/CSJVideoRenderer.h"

#import "CSJMetalRenderer.h"

class CSJVideoRendererMetalImpl: public CSJVideoRenderer {
public:
    CSJVideoRendererMetalImpl();
    ~CSJVideoRendererMetalImpl();

    bool init(WId widgetID, int width, int height) override;
    bool init(WId widgetID, int width, int height, float pixelRatio) override;
    bool updateSence(double timeStamp) override;
    void drawSence() override;
    void updateDrawableSize(int width, int height, float pixelRatio) override;

    virtual void initialRenderComponents(CSJVideoFormatType fmtType,
                                         int width, int height) override;

    virtual void updateVideoFrame(CSJVideoData *videoData) override;

    virtual void setImage(const QString& imagePath) {};

private:
    CSJMetalRenderer *m_pRenderer;
};

#endif // __CSJVIDEORENDERERMETAL_H__
