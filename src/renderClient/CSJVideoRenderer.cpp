#include "CSJVideoRenderer.h"

#ifdef _WIN32
#include "renderClient/Win/CSJVideoRendererDXImpl.h"
#elif __APPLE__
#include "renderClient/Mac/CSJVideoRendererMetalImpl.h"
#endif

CSJSpVideoRenderer CSJVideoRenderer::getRendererInstance() {
#ifdef _WIN32
    return std::make_shared<CSJVideoRendererDXImpl>();
#elif __APPLE__
    return std::make_shared<CSJVideoRendererMetalImpl>();
#endif
}




