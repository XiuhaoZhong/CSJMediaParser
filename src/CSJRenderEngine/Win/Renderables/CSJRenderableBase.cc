#include "CSJRenderableBase.h"

#include "CSJRGBARenderable.h"
#include "CSJYUVRenderable.h"

#include "CSJUtils/CSJLogger.h"

using namespace csjutils;

namespace csjrenderengine {

CSJRenderablePtr createRenderableWithType(CSJRenderableType type) {
    switch (type)
    {
    case CSJRenderableType_RGBA:
        return std::make_shared<CSJRGBARenderable>();
    break;
    case CSJRenderableType_YUV:
        return std::make_shared<CSJYUVRenderable>();
    break;
    default:
    break;
    }

    return nullptr;
}

bool CSJRenderableBase::copyDataFromBufferToTex(ComPtr<ID3D11Resource> &resource, 
                                                rsize_t len, uint8_t *data) {
    
    auto context = m_directxParams.context;
    if (!context) {
        LOG_Error("Current context is null!");
        return false;
    }

    if (!resource || !data) {
        LOG_Error("Resource or data is null!");
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE mappedData;
    HRESULT hr = context->Map(resource.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    if (FAILED(hr)) {
        // TODO: map texture failed.
        return false;
    }

    memcpy_s(mappedData.pData, len, data, len);
    context->Unmap(resource.Get(), 0);

    return true; 
}

}