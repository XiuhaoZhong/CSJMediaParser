#pragma once

#include <QObject>

#include "CSJRenderEngine/CSJVideoRenderer.h"

using csjutils::CSJVideoFramePtr;
using csjrenderengine::CSJRenderDelegate;

class CSJRenderDelegateImpl : public QObject
                            , public CSJRenderDelegate {
    Q_OBJECT
public:
    CSJRenderDelegateImpl(QObject *parent = nullptr);
    ~CSJRenderDelegateImpl();

     CSJVideoFramePtr getNextVideoFrame() override;

     void beforeARenderingTick() override;
     void afterARenderingTick() override;

     void beforeRenderingStart() override;
     void afterRenderingStart() override;

     void beforeRenderingPause() override;
     void afterRenderingPause() override;

     void beforeRenderingResume() override;
     void afterRenderingResume() override;

     void beforeRenderingStop() override;
     void afterRenderingStop() override;
};

using CSJRenderDelegateImplPtr = std::shared_ptr<CSJRenderDelegateImpl>;
CSJRenderDelegateImplPtr createRenderDelegateImpl();
