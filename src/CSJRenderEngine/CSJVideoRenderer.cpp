#include "CSJVideoRenderer.h"

#ifdef _WIN32
#include "Win/CSJVideoRendererDXImpl.h"
#elif __APPLE__
#include "Mac/CSJVideoRendererMetalImpl.h"
#endif

CSJSpVideoRenderer CSJVideoRenderer::getRendererInstance() {
#ifdef _WIN32
    return std::make_shared<CSJVideoRendererDXImpl>();
#elif __APPLE__
    return std::make_shared<CSJVideoRendererMetalImpl>();
#endif
}

bool CSJVideoRenderer::initForOffScreen(int width, int height) {
    return false;
}

bool CSJVideoRenderer::fillTextureData(uint8_t *buf, int width, int height) {
    return false;
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

void CSJVideoRenderer::resize(int width, int height) {

}

void CSJVideoRenderer::updateDrawableSize(int width, int height, float pixelRatio) {

}
