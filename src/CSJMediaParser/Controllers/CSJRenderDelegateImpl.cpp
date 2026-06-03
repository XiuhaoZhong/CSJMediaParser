#include "CSJRenderDelegateImpl.h"

#include "CSJUtils/CSJMediaData.h"
#include "CSJUtils/CSJLogger.h"

CSJRenderDelegateImpl::CSJRenderDelegateImpl(QObject *parent)
    : QObject(parent) {

}

CSJRenderDelegateImpl::~CSJRenderDelegateImpl() {

}

CSJVideoFramePtr CSJRenderDelegateImpl::getNextVideoFrame() {
    return CSJVideoFramePtr();
}

void CSJRenderDelegateImpl::beforeARenderingTick() {

}

void CSJRenderDelegateImpl::afterARenderingTick() {
    // Don't ouput the frame count by default;
#if 0 
    static int render_count = 0;
    LOG_Info("The %dth frames rendered!", render_count++);
#endif 
}

void CSJRenderDelegateImpl::beforeRenderingStart() {

}

void CSJRenderDelegateImpl::afterRenderingStart() {

}

void CSJRenderDelegateImpl::beforeRenderingPause() {

}

void CSJRenderDelegateImpl::afterRenderingPause() {

}

void CSJRenderDelegateImpl::beforeRenderingResume() {

}

void CSJRenderDelegateImpl::afterRenderingResume() {

}

void CSJRenderDelegateImpl::beforeRenderingStop() {

}

void CSJRenderDelegateImpl::afterRenderingStop() {

}

CSJRenderDelegateImplPtr createRenderDelegateImpl() {
    return std::shared_ptr<CSJRenderDelegateImpl>(new CSJRenderDelegateImpl(nullptr), 
                                                  [](CSJRenderDelegateImpl* p) { 
                                                  if (p) p->deleteLater();
                                                });
}
