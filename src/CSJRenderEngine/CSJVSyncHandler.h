#pragma once 

#include <functional>
#include <memory>

namespace csjrenderengine {

using CSJVSyncCallback = std::function<void(double timestamp)>;

class CSJVsyncHandler {
public:
    CSJVsyncHandler() = default;
    virtual ~CSJVsyncHandler() {}

    static std::unique_ptr<CSJVsyncHandler> createVsync();

#ifdef __APPLE__
    virtual void start(void* currentView, CSJVSyncCallback callback) = 0;
#else
    virtual void start(CSJVSyncCallback callback) = 0;
#endif

    virtual void stop() = 0;
};

using CSJVSyncPtr = std::unique_ptr<CSJVsyncHandler>;

} // namespace csjrenderengine