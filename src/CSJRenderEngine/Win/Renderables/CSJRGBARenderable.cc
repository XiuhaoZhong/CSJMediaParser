#include "CSJRGBARenderable.h"

#include <string>

#include "Win/CSJDirectXHelper.h"

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"
#include "CSJUtils/CSJStringUtils.h"
#include "CSJUtils/CSJMediaData.h"

using namespace csjutils;

namespace csjrenderengine {

static std::string vertexShaderFile   = "DXVertexShader.hlsl";
static std::string vertexShaderCso    = "DXVertexShader.cso";
static std::string fragmentShaderFile = "DXRGBAShader.hlsl";
static std::string fragmentShaderCso  = "DXRGBAShader.cso";

const D3D11_INPUT_ELEMENT_DESC CSJRGBARenderable::InputData::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

CSJRGBARenderable::CSJRGBARenderable() {

}

CSJRGBARenderable::~CSJRGBARenderable() {

}

bool CSJRGBARenderable::initRenderable(DirectXParams &params) {
    m_directxParams = params;

    bool res = createShader();
    if (!res) {
        LOG_Warn("RGBARenderable Create shader failed!");
    }

    res = createBasicData();
    if (!res) {
        LOG_Warn("RGBARenderable create basic data failed!");
    }

    res = createTextureSampler();
    if (!res) {
        LOG_Warn("RGBARenderable create sampler failed!");
    }

    for (size_t i = 0; i < tex_number; i++) {
        m_renderDataCtxArray[i] = std::make_shared<RenderDataContext>();
    }

    return res;
}

bool CSJRGBARenderable::fillRenderrableData() {
    return false;
}

bool CSJRGBARenderable::bindRenderableComponents() {
    auto context = m_directxParams.context;
    if (!context) {
        return false;
    }

    UINT stride = sizeof(InputData);
    UINT offset = 0;

    context->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    context->IASetInputLayout(m_pVertexLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

    return true;
}

void CSJRGBARenderable::setFreshImage(const std::string &imageFile) {
    if (imageFile == m_imagePath) {
        return ;
    }
    m_imagePath = imageFile;

    size_t updateIndex = m_iUpdateSlot.load(std::memory_order_acquire);

    std::wstring imagePath = CSJStringUtil::string2wstring(m_imagePath);
    int width = 0, height = 0;
    bool res = CSJDirectXHelper::createRGBATextureFromImageFile(m_directxParams.device, 
                                                                imagePath, 
                                                                m_renderDataCtxArray[updateIndex]->tex, 
                                                                m_renderDataCtxArray[updateIndex]->texRV, 
                                                                width, height);
    if (!res) {
        return ;
    }
    m_renderDataCtxArray[updateIndex]->texValid = true;

    size_t oldIndex = m_iUpdateSlot.exchange(m_iRenderSlot, std::memory_order_release);
    m_iRenderSlot.store(oldIndex, std::memory_order_release);
}

void CSJRGBARenderable::setFreshVideoData( CSJVideoFramePtr &frame) {
    if (!m_directxParams.device || !frame) {
        return ;
    }

    size_t updateIndex = m_iUpdateSlot.load(std::memory_order_acquire);

   
    // TODO: create texture from buffer.
    

    size_t oldIndex = m_iUpdateSlot.exchange(m_iRenderSlot, std::memory_order_release);
    m_iUpdateSlot.store(oldIndex, std::memory_order_release);
}

bool CSJRGBARenderable::updateRenderable(double timeStamp) {
    auto context = m_directxParams.context;
    if (!context) {
        return false;
    }

    size_t renderIndex = m_iRenderSlot.load(std::memory_order_acquire);
    if (!m_renderDataCtxArray[renderIndex]->texValid) {
        return false;
    }

    context->PSSetShaderResources(3, 1, m_renderDataCtxArray[renderIndex]->texRV.GetAddressOf());

    return true;
}

void CSJRGBARenderable::drawRenderable( double timeStamp) {
    auto device = m_directxParams.device;
    auto context = m_directxParams.context;

    if (!device || !context) {
        // TODO: log the context is error.
        return ;
    }

    // TODO: There is a problem, if the image hasn't been updated, shouldn't udpate the image content, and render immediately.
    bindRenderableComponents();

    if (!updateRenderable(timeStamp)) {
        return ;
    }

    context->DrawIndexed(6, 0, 0);

}

bool CSJRGBARenderable::createShader() {
    auto device = m_directxParams.device;
    if (!device) {
        return false;
    }

    bool res = false;

    /* Create Vertex Shader. */
    std::string vertshaderFile = CSJPathTool::getShaderDir().append(vertexShaderFile).string();
    std::string vertCso = CSJPathTool::getShaderDir().append(vertexShaderCso).string();
    res = CSJDirectXHelper::createVertexShader(device, 
                                               vertshaderFile, 
                                               vertCso, 
                                               "main", 
                                               "vs_5_0", 
                                               InputData::inputLayout,
                                               ARRAYSIZE(InputData::inputLayout),
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

bool CSJRGBARenderable::createBasicData() {
    auto device = m_directxParams.device;
    if (!device) {
        return false;
    }

    static InputData vertices[] = {
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

    return false;
}

bool CSJRGBARenderable::createTextureSampler() {
    auto device = m_directxParams.device;
    if (!device) {
        return false;
    }

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD         = 0;
    samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

    HRESULT hr = device->CreateSamplerState(&samplerDesc, m_pSamplerState.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        LOG_Warn("Create sampler failed!");
        return false;
    }

    return true;
}

bool CSJRGBARenderable::createBuffers() {
    return false;
}

} // namespace csjrenderengine