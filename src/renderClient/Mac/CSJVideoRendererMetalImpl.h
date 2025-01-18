#ifndef __CSJVIDEORENDERERMETAL_H__
#define __CSJVIDEORENDERERMETAL_H__

#include "renderClient/CSJVideoRenderer.h"

#import "CSJMTKRenderer.h"

class CSJVideoRendererMetalImpl: public CSJVideoRenderer {
public:
    CSJVideoRendererMetalImpl();
    ~CSJVideoRendererMetalImpl();

    bool init(WId widgetID, int width, int height) override;
    void updateSence(double timeStamp) override;
    void drawSence() override;
    void resize(int width, int height) override;

    virtual void loadVideoComponents(CSJVideoFormatType fmtType,
                                     int width, int height) override;

    virtual void updateVideoFrame(CSJVideoData *videoData) override;

private:
    CSJMTKRenderer *m_pRenderer;
};

#endif // __CSJVIDEORENDERERMETAL_H__
