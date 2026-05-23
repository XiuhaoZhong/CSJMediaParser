#pragma once

#include "CSJVSyncHandler.h"

namespace csjrenderengine {
class CSJVSyncHandlerAppleImpl : public CSJVsyncHandler {
public:
    CSJVSyncHandlerAppleImpl() = default;
    ~CSJVSyncHandlerAppleImpl();

    void start(void* currentView, CSJVSyncCallback callback) override;
    void stop() override;

    void display(double timeStamp);

private:
    void            *m_pVsyncHelper = nullptr;
    CSJVSyncCallback m_callback;
};
}