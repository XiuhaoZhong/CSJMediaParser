#include "CSJVideoRendererMetalImpl.h"

#include <QDebug>

#import <Cocoa/Cocoa.h>
#import <dispatch/dispatch.h>

#import "CSJMTKRenderer.h"

CSJVideoRendererMetalImpl::CSJVideoRendererMetalImpl() {

#if __has_feature(objc_arc)
    qDebug() << "ARC is ON";
#else
    qDebug() << "ARC is OFF";
#endif

}

CSJVideoRendererMetalImpl::~CSJVideoRendererMetalImpl() {
    qDebug() << "CSJVideoRendererMetalImpl destoryed!";

#if __has_feature(objc_arc)

#else
  if (m_pRenderer) {
    [m_pRenderer release];
  }
#endif


}

bool CSJVideoRendererMetalImpl::init(WId widgetID, int width, int height) {
    //NSView *view = (NSView *)widgetID;

  NSView *view = (__bridge NSView *)((void *)widgetID);
  if (!view) {
    return false;
  }

  CGRect rect = {{0, 0}, {(CGFloat)width, (CGFloat)height}};
  CSJMTKRenderer *renderer = [[CSJMTKRenderer alloc] initWithFrame:rect parent:view];
  if (!renderer) {
    return false;
  }

  m_pRenderer = renderer;

  return true;
}

void CSJVideoRendererMetalImpl::updateSence(double timeStamp) {
  //[m_pRenderer drawContent];
}

void CSJVideoRendererMetalImpl::drawSence() {
  dispatch_queue_t mainQueue = dispatch_get_main_queue();//dispatch_get_global_queue( QOS_CLASS_USER_INITIATED, 0);

  __weak CSJMTKRenderer *weakRender = m_pRenderer;
  dispatch_async(mainQueue, ^(){
  __strong CSJMTKRenderer *renderer = weakRender;
    if (renderer) {
      [renderer drawContent];
    }
  });

  //[m_pRenderer drawContent];
}

void CSJVideoRendererMetalImpl::resize(int width, int height) {

}

void CSJVideoRendererMetalImpl::loadVideoComponents(CSJVideoFormatType fmtType,
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
