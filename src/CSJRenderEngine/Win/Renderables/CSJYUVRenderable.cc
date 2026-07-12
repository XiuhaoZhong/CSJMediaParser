#include "CSJYUVRenderable.h"

#include "Win/CSJDirectXHelper.h"

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"
#include "CSJUtils/CSJStringUtils.h"
#include "CSJUtils/CSJMediaData.h"

using namespace csjutils;

namespace csjrenderengine {

static std::string vertexShaderFile   = "DXVertexShader.hlsl";
static std::string vertexShaderCso    = "DXVertexShader.cso";
static std::string fragmentShaderFile = "DXYUVShader.hlsl";
static std::string fragmentShaderCso  = "DXYUVShader.cso";

const D3D11_INPUT_ELEMENT_DESC CSJYUVRenderable::YUVRenderInputData::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

CSJYUVRenderable::CSJYUVRenderable() {

}

CSJYUVRenderable::~CSJYUVRenderable() {

}

bool CSJYUVRenderable::initRenderable(DirectXParams &params) {
    m_directxParams = params;
    auto device = m_directxParams.device;
    if (!device) {
        return false;
    }

    if (!device) {
        return false;
    }

    bool res = createShader(device);
    if (!res) {
        LOG_Warn("RGBARenderable Create shader failed!");
    }

    res = createBasicData(device);
    if (!res) {
        LOG_Warn("RGBARenderable create basic data failed!");
    }

    return res;
}

bool CSJYUVRenderable::fillRenderrableData() {

    return false;
}

bool CSJYUVRenderable::bindRenderableComponents() {

    return false;
}

void CSJYUVRenderable::setFreshImage( const std::string &imageFile) {
    if (imageFile.size() == 0) {
        return ;
    }

}

void CSJYUVRenderable::setFreshVideoData( CSJVideoFramePtr &frame) {
    auto device = m_directxParams.device;
    auto context = m_directxParams.context;
    if (!device || !context) {
        return ;
    }

    // TODO:
    /**
     * 1. Choose the render context array with frame->format.
     * 2. Get the current update slot render context from the context array.
     * 3. If the current render context is null, create a new one.
     * 4. Copy video data into texture and texture resource view.
     * 5. Exchange the update slot and render slot.
     */
    int updateSlot = m_iUpdateSlot.load(std::memory_order_acquire);

    bool res = false;
    int planes = 0;
    SpYuvSizeArrayPtr sizeArrPtr;
    res = getFramePlanesAndSizeArr(frame, planes, sizeArrPtr);
    if (!res) {
        return ;
    }
    SpYUVRenderCtxPtr renderCtx = m_renderCtxArr[updateSlot];
    if (!renderCtx) {
        // If the renderContext is null at the updateSlot, create a new one.
        renderCtx = std::make_shared<YUVRenderContext>();
        renderCtx->udpateContextWithFrame(frame);
        m_renderCtxArr[updateSlot] = renderCtx;

        res = createTexResourceForRenderContext(renderCtx);
        if (res) {
            LOG_Warn("Create tex resource for update render context failed!");
        }
    } else {
        // If the renderContext isn't null at updateSlot, then compare the 
        // width and height with new video frame, if one is different, recreate
        // tex and texRV.
        if (needRecreateTexture(renderCtx, frame)) {
            // video frame size changes or the format chagnge.
            renderCtx->udpateContextWithFrame(frame);
            res = createTexResourceForRenderContext(renderCtx);
            if (res) {
                LOG_Warn("Recreate tex resource for update render context failed!");
            } 
        } else {
            renderCtx->videoFrame = frame;
        }
    }

    YUVSizeArray *sizeArr = renderCtx->sizeArr.get();
    for (size_t i = 0; i < planes; i++) {
        ComPtr<ID3D11Resource> texResource = renderCtx->texArr[i];
        copyDataFromBufferToTex(texResource, (*sizeArr)[i].first * (*sizeArr)[i].second, frame->data[i]);
    }

    int oldSlot = m_iUpdateSlot.exchange(m_iRenderSlot, std::memory_order_relaxed);
    m_iRenderSlot.store(oldSlot, std::memory_order_release);
}

bool CSJYUVRenderable::updateRenderable(double timeStamp) {
    auto context = m_directxParams.context;
    if (!context) {
        return false;
    }

    if (!m_pVideoFrame) {
        return false;
    }

    int planes = getFramePlanes(m_pVideoFrame);
    if (planes != 3 || planes != 2) {
        return false;
    }
    int renderSlot = m_iRenderSlot.load(std::memory_order_acquire);
    auto curRenderCtx = m_renderCtxArr[renderSlot];
    for (size_t i = 0; i < planes; i++) {
        context->PSSetShaderResources(i, 1, curRenderCtx->texRVArr[i].GetAddressOf());
    }

    if (m_pVideoParamBuffer) {
        VideoParam videoparam{};
        videoparam.isI420 = curRenderCtx->planes == 3 ? 1 : 0;
        context->UpdateSubresource(m_pVideoParamBuffer.Get(), 
                                   0, nullptr, 
                                   &videoparam, 
                                   sizeof(VideoParam), 
                                   sizeof(VideoParam));
    } else {
        LOG_Warn("There isn't a video param buffer, and the video frame maybe rendered error!");
    }

    return true;
}

void CSJYUVRenderable::drawRenderable(double timeStamp) {
    auto context = m_directxParams.context;
    if (!context) {
        LOG_Warn("Current DirectX context is null, skip renderer!");
        return ;
    }

    context->PSSetConstantBuffers(0, 1, m_pVideoParamBuffer.GetAddressOf());
    // TODO: When rendering yuv data, there will set more render components.

}

int CSJYUVRenderable::getFramePlanes(CSJVideoFramePtr videoFrame) {
    if (!!videoFrame) {
        return 0;
    }

    CSJPixelFormat format = videoFrame->format;
    switch (format) {
    case CSJPixelFormat::CSJPixelFormat_YUV420P:
        return 3;
    case CSJPixelFormat::CSJPixelFormat_NV12:
        return 2;
    default:
        LOG_Warn("Current pixel format isn't yuv series!");
    }

    return 0;
}

bool CSJYUVRenderable::getFramePlanesAndSizeArr(CSJVideoFramePtr &frame, int &planes, SpYuvSizeArrayPtr sizeArrayPtr)
{
    if (!frame) {
        return false;
    }

    // TODO: I need to think about how to fill the sizeArray!!! 
    CSJPixelFormat format = frame->format;
    switch (format) {
    case CSJPixelFormat::CSJPixelFormat_YUV420P:
        planes = 3;
        if (sizeArrayPtr) {
            sizeArrayPtr = std::make_shared<YUVSizeArray>(3);
            auto sizeArr = sizeArrayPtr.get();
            int width = frame->width;
            int height = frame->height;
            (*sizeArr)[0] = std::pair<int, int>(width, height);
            (*sizeArr)[1] = std::pair<int, int>(width / 2, height / 2);
            (*sizeArr)[2] = std::pair<int, int>(width / 2, height / 2);
        }
    break;
    case CSJPixelFormat::CSJPixelFormat_NV12:
        planes = 2;
        if (sizeArrayPtr) {
            sizeArrayPtr = std::make_shared<YUVSizeArray>(2);
            auto sizeArr = sizeArrayPtr.get();
            int width = frame->width;
            int height = frame->height;
            (*sizeArr)[0] = std::pair<int, int>(width, height);
            (*sizeArr)[1] = std::pair<int, int>(width / 2, height / 2);
        }
    break;
    default:
        planes = 0;
        LOG_Warn("Current pixel format isn't yuv series!");
        break;
    }

    if (planes != 2 || planes != 3) {
        return false;
    }

    return true;
}

bool CSJYUVRenderable::needRecreateTexture(SpYUVRenderCtxPtr &renderCtx, 
                                           CSJVideoFramePtr &freshFrame) {
    if (!renderCtx || !freshFrame) {
        return true;
    }

    if (!renderCtx->videoFrame) {
        return true;
    }

    if (renderCtx->videoFrame->width == freshFrame->width && 
        renderCtx->videoFrame->height == freshFrame->height &&
        renderCtx->videoFrame->format == freshFrame->format) {
        return false;
    }

    return true;
}

bool CSJYUVRenderable::createShader(ComPtr<ID3D11Device> &device) {
    bool res = false;

    /* Create Vertex Shader. */
    std::string vertshaderFile = CSJPathTool::getShaderDir().append(vertexShaderFile).string();
    std::string vertCso = CSJPathTool::getShaderDir().append(vertexShaderCso).string();
    res = CSJDirectXHelper::createVertexShader(device, 
                                               vertshaderFile, 
                                               vertCso, 
                                               "main", 
                                               "vs_5_0", 
                                               YUVRenderInputData::inputLayout,
                                               ARRAYSIZE(YUVRenderInputData::inputLayout),
                                               m_pVertexShader,
                                               m_pVertexLayout);
    if (!res) {
        LOG_Warn("Create Vertex Shader failed!");
    }

    /* Creating RGBA Pixel shader */
    std::string RGBAPixelShaderFile = CSJPathTool::getShaderDir().append(fragmentShaderFile).string();
    std::string RGBAPixelCso = CSJPathTool::getShaderDir().append(fragmentShaderCso).string();
    res = CSJDirectXHelper::createPixelShader(device, 
                                              RGBAPixelShaderFile, 
                                              RGBAPixelCso, 
                                              "main", 
                                              "ps_5_0", 
                                              m_pPixelShader);
    if (!res) {
        LOG_Warn("Create RGBA Pixel Shader failed!");
    }

    return res;
}

bool CSJYUVRenderable::createBasicData(ComPtr<ID3D11Device> &device) {
     static YUVRenderInputData vertices[] = {
        /**
         * vertex data and texture coordinates.
         */
        {DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)},
        {DirectX::XMFLOAT3(-0.5f,  0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
        {DirectX::XMFLOAT3( 0.5f,  0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)},
        {DirectX::XMFLOAT3( 0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)}
    };

    /* set vertex buffer desc */
    D3D11_BUFFER_DESC vbd{};
    vbd.Usage          = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth      = sizeof(vertices);
    vbd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    /* create vertex buffer */
    D3D11_SUBRESOURCE_DATA InitData{};
    InitData.pSysMem = vertices;
    HRESULT hr = device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    /**
     * Create index buffer.
     */
    static const DWORD indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    D3D11_BUFFER_DESC indexBufferDes{};
    indexBufferDes.Usage          = D3D11_USAGE_IMMUTABLE;
    indexBufferDes.ByteWidth      = sizeof(indices);
    indexBufferDes.BindFlags      = D3D11_BIND_INDEX_BUFFER;
    indexBufferDes.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = indices;
    hr = device->CreateBuffer(&indexBufferDes, &indexData, m_pIndexBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    bool res = CSJDirectXHelper::createConstBuffer(device, m_pVideoParamBuffer, sizeof(VideoParam));
    if (!res) {
        LOG_Warn("Create const buffer for video param failed.");
    }

    return true;
}

bool CSJYUVRenderable::createBuffers(ComPtr<ID3D11Device> &device) {
    return false;
}

bool CSJYUVRenderable::createTexResourceForRenderContext(SpYUVRenderCtxPtr &ctx) {
    auto device = m_directxParams.device;
    if (!device) {
        return false;
    }

    if (!ctx) {
        return false;
    }

    if (!ctx->videoFrame) { 
        return false;
    }

    auto sizeArrPtr = ctx->sizeArr;
    if (!sizeArrPtr) {
        return false;
    }

    int video_planes = ctx->planes;
    if (video_planes == 0) {
        LOG_Warn("Current video frame has the wrong planes!");
        return false;
    }

    bool res = false;
    for (size_t i = 0; i < video_planes; i++) {
        auto sizePair = sizeArrPtr->at(i);
        res = CSJDirectXHelper::createD3DTextureWithResourceView(device,
                                                                 ctx->texArr[i],
                                                                 ctx->texRVArr[i],
                                                                 m_directxParams.enable4xMsaa,
                                                                 m_directxParams.msaaQuality,
                                                                 sizePair.first, sizePair.second,
                                                                 DXGI_FORMAT_R8_UNORM,
                                                                 1,
                                                                 1,
                                                                 D3D11_USAGE_DEFAULT, 
                                                                 D3D11_BIND_SHADER_RESOURCE, 
                                                                 D3D11_CPU_ACCESS_WRITE,
                                                                 D3D11_RESOURCE_MISC_GENERATE_MIPS);

        if (!res) {
            LOG_Warn("In YUV Renderable, create %dth tex failed!");
            continue;
        }
    }

    return true;
}

void CSJYUVRenderable::YUVRenderContext::udpateContextWithFrame(CSJVideoFramePtr &frame) {
    if (!frame) {
        return ;
    }

    this->videoFrame = frame;
    // TODO: I need to think about how to fill the sizeArray!!! 
    CSJPixelFormat format = frame->format;
    switch (format) {
    case CSJPixelFormat::CSJPixelFormat_YUV420P:
        this->planes = 3;
    break;
    case CSJPixelFormat::CSJPixelFormat_NV12:
        this->planes = 2;
    break;
    default:
        this->planes = 0;
        LOG_Warn("Current pixel format isn't yuv series!");
        break;
    }

    if (!this->sizeArr) {
        this->sizeArr = std::make_shared<YUVSizeArray>(3);
    }

    int width = frame->width;
    int height = frame->height;
    for (size_t i = 0; i < this->planes; i++) {
        YUVSizeArray *sizeArr = this->sizeArr.get();
        if (i == 0) {
            (*sizeArr)[i] = std::pair<int, int>(width, height);
        } else {
            (*sizeArr)[i] = std::pair<int, int>(width / 2, height / 2);
        }
    }

    if (this->planes < 3) {
        (*sizeArr)[2] = std::pair<int, int>(0, 0);
    }
}

} // namespace csjrenderengine {
