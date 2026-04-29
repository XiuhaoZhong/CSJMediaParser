#include "CSJVideoRendererMetalImpl.h"

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

CSJVideoRendererMetalImpl::~CSJVideoRendererMetalImpl() {
    NSLog(@"CSJVideoRendererMetalImpl destoryed!");

#if __has_feature(objc_arc)

#else
    if (m_pRenderer) {
        [m_pRenderer release];
    }
#endif

}

bool CSJVideoRendererMetalImpl::init(CSJWindowID widgetID, int width, int height) {
    /* This interface is for Windows, not macOS, so return false in MacOS. */
    return false;
}

bool CSJVideoRendererMetalImpl::init(CSJWindowID widgetID, int width, int height, float pixelRatio) {
    NSView *view = (__bridge NSView *)(widgetID);
    if (!view) {
        return false;
    }

    CGRect rect = {{0, 0}, {(CGFloat)width, (CGFloat)height}};
    CSJMetalRenderer *renderer = [[CSJMetalRenderer alloc] initWithFrameWithView:view 
                                                                           frame:rect 
                                                                       pixelRatio:pixelRatio];
    if (!renderer) {
        return false;
    }

    m_pRenderer = renderer;
}

bool CSJVideoRendererMetalImpl::updateScene(double timeStamp) {
    if (!m_pRenderer) {
        return false;
    }

    return [m_pRenderer updateSceneWithTimeStamp:0.0];
}

void CSJVideoRendererMetalImpl::drawScene() {
    if (!m_pRenderer) {
        return ;
    }

    bool need_update = updateScene(0.0);
    if (need_update) {
        // TODO: draw
    }

    [m_pRenderer drawContent:need_update];
}

void CSJVideoRendererMetalImpl::updateDrawableSize(int width, int height, 
                                                   float pixelRatio) {
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

} // namespace csjrenderengine
