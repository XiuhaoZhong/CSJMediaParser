#include "CSJVideoRenderer.h"

#include "CSJUtils/CSJMediaData.h"

#ifdef _WIN32
#include "Win/CSJVideoRendererDXImpl.h"
#elif __APPLE__
#include "Mac/CSJVideoRendererMetalImpl.h"
#endif

using namespace csjutils;

namespace csjrenderengine {
void CSJVideoRenderer::setRenderType(CSJRenderContentType renderType) {
    m_renderContentType = renderType;
}

bool CSJVideoRenderer::fillTextureData(uint8_t *buf, int width, int height) {
    if (!buf || width == 0 || height == 0) {
        return false;
    }

    return false;
}

void CSJVideoRenderer::setRenderDelegate(CSJRenderDelegatePtr delegate) {
    m_pDelegate = delegate;
}

std::array<float, 2> CSJVideoRenderer::computeVideoArea(int widgetW, int widgetH, int videoW, int videoH) {
    float resX = 0.0f;
    float resY = 0.0f;

    if (videoW < widgetW && videoH < widgetH) {
        if (videoW < videoH) { 

        } else {

        }
    } else if (videoW >= widgetW && videoH < widgetH) {
        if (videoW < videoH) { 

        } else {
            
        }
    } else if (videoW < widgetW && videoH >= widgetH) {
        if (videoW < videoH) { 

        } else {
            
        }
    } else if (videoW > widgetW && videoH > widgetH) {
        if (videoW < videoH) { 

        } else {
            
        }
    } else {
        return {0.0f, 0.0f};
    }

    if (videoW < videoH) {
        // vertical
        resY = 1.0f;

        resX = (static_cast<float>(videoW)) * widgetH / videoH;
    } else {
        // horizontal
        resX = 1.0f;

        resY = (static_cast<float>(widgetW)) * videoH / videoW;
    }

    return {resX, resY};
}

CSJVideoRenderer* createCSJRenderer(CSJWindowID widgetID, int width, int height, float pixelRatio) {
#ifdef _WIN32
    return new CSJVideoRendererDXImpl(widgetID, width, height, pixelRatio);
#elif __APPLE__
    return new CSJVideoRendererMetalImpl(widgetID, width, height, pixelRatio);
#else
    return nullptr;
#endif
}

} // csjrenderengine
