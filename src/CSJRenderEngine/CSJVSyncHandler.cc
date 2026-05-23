#include "CSJVSyncHandler.h"

#ifdef __APPLE__
#include "Mac/CSJVSyncHandlerAppleImpl.h"
#else

#endif

namespace csjrenderengine {

std::unique_ptr<CSJVsyncHandler> CSJVsyncHandler::createVsync() {
#ifdef __APPLE__
    return std::make_unique<CSJVSyncHandlerAppleImpl>();
#else 
    return nullptr;
#endif
}

}