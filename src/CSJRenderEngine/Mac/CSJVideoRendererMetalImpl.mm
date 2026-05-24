#include "CSJVideoRendererMetalImpl.h"

#include "CSJUtils/CSJLogger.h"

#import <Cocoa/Cocoa.h>
#import <dispatch/dispatch.h>

#import "CSJMetalRenderer.h"

namespace csjrenderengine {

CSJVideoRendererMetalImpl::CSJVideoRendererMetalImpl() {

#if __has_feature(objc_arc)
    NSLog(@"ARC is ON");
#else
    NSLog(@"ARC is OFF");
#endif

}

CSJVideoRendererMetalImpl::CSJVideoRendererMetalImpl(CSJWindowID widgetID, 
                                                     int width, 
                                                     int height, 
                                                     float pixelRatio)
    : m_pWinID(widgetID)
    , m_iWidth(width)
    , m_iHeight(height)
    , m_fPixelRatio(pixelRatio) {

}

CSJVideoRendererMetalImpl::~CSJVideoRendererMetalImpl() {
    NSLog(@"CSJVideoRendererMetalImpl destoryed!");

#if __has_feature(objc_arc)

#else
    if (m_pRenderer) {
        [m_pRenderer release];
    }
#endif

}

bool CSJVideoRendererMetalImpl::init() {
    if (m_bInitSuccess.load()) {
        return true;
    }

    if (!m_pWinID) {
        LOG_Error("Render view is null, initialize failed!");
        return false;
    }

    NSView *view = (__bridge NSView *)(m_pWinID);
    if (!view) {
        LOG_Error("Render view is invalid, initialize failed!");
        return false;
    }

    CGRect rect = {{0, 0}, {(CGFloat)m_iWidth, (CGFloat)m_iHeight}};
    CSJMetalRenderer *renderer = [[CSJMetalRenderer alloc] initWithFrameWithView:view 
                                                                           frame:rect 
                                                                      pixelRatio:m_fPixelRatio];
    if (!renderer) {
        return false;
    }

    m_pRenderer = renderer;

    m_pVSyncHandler = CSJVsyncHandler::createVsync();
    m_bInitSuccess.store(true);

    return true;
}

void CSJVideoRendererMetalImpl::startRender() {
    if (!m_bInitSuccess.load()) {
        if (!init()) {
            return ;
        }
    }

    LOG_Info("Start VSync ...");
    m_pVSyncHandler->start(m_pWinID, [this](double ts) {
        LOG_Info("Render callback");
        draw(ts);
    });
}

void CSJVideoRendererMetalImpl::stopRender() {
    m_pVSyncHandler->stop();
}

bool CSJVideoRendererMetalImpl::updateScene(double timeStamp) {
    if (!m_pRenderer) {
        return false;
    }

    return [m_pRenderer updateSceneWithTimeStamp:0.0];
}

void CSJVideoRendererMetalImpl::resize(int width, int height, float pixelRatio) {
    if (!m_pRenderer) {
        return ;
    }
                  
    [m_pRenderer updateDrawable:width height:height pixelRatio:pixelRatio];
}

void CSJVideoRendererMetalImpl::initialRenderComponents(CSJVideoFormatType fmtType,
                                                        int width, int height) {
    if (!m_pRenderer) {
        return ;
    }

    [m_pRenderer loadVideoComponentWithPixelFmt:fmtType width:width height:height];
}

void CSJVideoRendererMetalImpl::updateVideoFrame(CSJVideoData *pData) {
    if (!m_pRenderer) {
        return ;
    }

    [m_pRenderer updateVideoFrameWithData:(void *)pData];
}

void CSJVideoRendererMetalImpl::setImage(const std::string &imagePath) {
    if (!m_pRenderer) {
        return ;
    }

    NSString * image_path = [NSString stringWithUTF8String:imagePath.c_str()];
    [m_pRenderer setImageWithPath:image_path];
}

void CSJVideoRendererMetalImpl::draw(double timeStamp) {
    if (!m_pRenderer) {
        return ;
    }

    bool need_update = updateScene(timeStamp);
    if (need_update) {
        // TODO: draw
    }

    [m_pRenderer drawContent:need_update];
}

} // namespace csjrenderengine
