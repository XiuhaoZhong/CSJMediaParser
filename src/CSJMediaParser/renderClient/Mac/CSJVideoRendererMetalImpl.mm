#include "CSJVideoRendererMetalImpl.h"

#import <Cocoa/Cocoa.h>
#import <dispatch/dispatch.h>

#import "CSJMetalRenderer.h"

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

bool CSJVideoRendererMetalImpl::init(WId widgetID, int width, int height) {
    /* This interface is for Windows, not macOS, so return false in MacOS. */
    return false;
}

bool CSJVideoRendererMetalImpl::init(WId widgetID, int width, int height, float pixelRatio) {
    NSView *view = (__bridge NSView *)((void *)widgetID);
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

bool CSJVideoRendererMetalImpl::updateSence(double timeStamp) {
    //[m_pRenderer drawContent];
    return false;
}

void CSJVideoRendererMetalImpl::drawSence() {
    dispatch_queue_t mainQueue = dispatch_get_main_queue();//dispatch_get_global_queue( QOS_CLASS_USER_INITIATED, 0);

    __weak CSJMetalRenderer *weakRender = m_pRenderer;
    dispatch_async(mainQueue, ^(){
        __strong CSJMetalRenderer *renderer = weakRender;
        if (renderer) {
            [renderer drawContent];
        }
    });

    //[m_pRenderer drawContent];
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
