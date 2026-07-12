#pragma once 

#include <windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <directxmath.h>

#include <string>

#include "CSJUtils/CSJMediaData.h"

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using csjutils::CSJVideoFramePtr;
using csjutils::CSJPixelFormat;

namespace csjrenderengine {

enum CSJRenderableType : uint8_t {
    CSJRenderableType_RGBA = 1,
    CSJRenderableType_YUV
};

struct DirectXParams {
    bool                        enable4xMsaa;  // Support msaa or not.
    UINT                        msaaQuality;   // Quality level of msas.
    ComPtr<ID3D11Device>        device;
    ComPtr<ID3D11DeviceContext> context; 

    DirectXParams()
        : enable4xMsaa(false)
        , msaaQuality(0)
        , device(nullptr)
        , context(nullptr) {
    }

    DirectXParams(const DirectXParams &params) {
        enable4xMsaa = params.enable4xMsaa;
        msaaQuality  = params.msaaQuality;
        device       = params.device;
        context      = params.context;
    }

    DirectXParams& operator=(const DirectXParams &params) {
        enable4xMsaa = params.enable4xMsaa;
        msaaQuality  = params.msaaQuality;
        device       = params.device;
        context      = params.context;

        return *this;
    }
};

class CSJRenderableBase {
public:
    CSJRenderableBase() = default;
    virtual ~CSJRenderableBase() = default;

    virtual bool initRenderable(DirectXParams &params) = 0;
    virtual bool fillRenderrableData() = 0;
    virtual bool bindRenderableComponents() = 0;
    virtual void setFreshImage(const std::string &imageFile) = 0;
    virtual void setFreshVideoData(CSJVideoFramePtr &frame) = 0;
    virtual bool updateRenderable( double timeStamp) = 0;
    virtual void drawRenderable( double timeStamp) = 0;

    void setDirectXParams(DirectXParams &params);

protected:
    bool copyDataFromBufferToTex(ComPtr<ID3D11Resource> &resource, 
                                 rsize_t len, uint8_t *data);

    DirectXParams m_directxParams;

}; 

using CSJRenderablePtr = std::shared_ptr<CSJRenderableBase>;
CSJRenderablePtr createRenderableWithType(CSJRenderableType type);
} // namespace csjrenderengine