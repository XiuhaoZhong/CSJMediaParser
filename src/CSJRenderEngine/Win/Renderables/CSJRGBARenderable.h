#pragma once 

#include "CSJRenderableBase.h"

#include <array>
#include <atomic>

namespace csjrenderengine {

constexpr int tex_number = 2;

class CSJRGBARenderable : public CSJRenderableBase {
public:
 
    struct RenderDataContext {
        CSJVideoFramePtr                 videoFrame = nullptr;
        ComPtr<ID3D11Texture2D>          tex = nullptr;
        ComPtr<ID3D11ShaderResourceView> texRV = nullptr;
        bool                             texValid = false;

        RenderDataContext() {

        }
    };

    using SpRenderDataCtx = std::shared_ptr<RenderDataContext>;

    CSJRGBARenderable();
    ~CSJRGBARenderable();

    bool initRenderable(DirectXParams &params) override;
    bool fillRenderrableData() override;
    bool bindRenderableComponents() override;
    void setFreshImage(const std::string &imageFile) override;
    void setFreshVideoData(CSJVideoFramePtr &frame) override;
    bool updateRenderable(double timeStamp) override;
    void drawRenderable(double timeStamp) override;

protected:
    struct InputData {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 texCoord;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

    bool createShader();
    bool createBasicData();
    bool createTextureSampler();
    bool createBuffers();

private:
    ComPtr<ID3D11InputLayout>   m_pVertexLayout;   // input vertex layout;
    ComPtr<ID3D11VertexShader>  m_pVertexShader;
    ComPtr<ID3D11PixelShader>   m_pPixelShader;
    ComPtr<ID3D11Buffer>        m_pVertexBuffer;
    ComPtr<ID3D11Buffer>        m_pIndexBuffer;
    ComPtr<ID3D11SamplerState>  m_pSamplerState; // sampler state
    int                         m_iCurTexIndex = 0;
    std::string                 m_imagePath;

    std::array<ComPtr<ID3D11Texture2D>, tex_number>          m_texArray;
    std::array<ComPtr<ID3D11ShaderResourceView>, tex_number> m_texRVArray;

    std::atomic<size_t>                     m_iUpdateSlot = 0;
    std::atomic<size_t>                     m_iRenderSlot = 1;
    std::array<SpRenderDataCtx, tex_number> m_renderDataCtxArray;
};

} // namespace csjrenderengine