#ifndef __CSJVIDEORENDERERDXIMPL_H__
#define __CSJVIDEORENDERERDXIMPL_H__

#include <windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <directxmath.h>

#include <mutex>
#include <array>

#include "renderClient/CSJVideoRenderer.h"

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace csjutils {
class CSJLogger;
}
using csjutils::CSJLogger;

class CSJVideoRendererDXImpl : public CSJVideoRenderer {
public:
    struct VertexPosColor {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 texCoord;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

    struct ConstantBuffer {
        DirectX::XMFLOAT4X4A world;
        DirectX::XMFLOAT4X4A view;
        DirectX::XMFLOAT4X4A proj;
    };

    CSJVideoRendererDXImpl();
    ~CSJVideoRendererDXImpl();

    bool init(WId widgetID, int width, int height) override;
    bool initForOffScreen(int width, int height) override;
    bool updateSence(double timeStamp) override;
    bool fillTextureData(uint8_t *buf, int width, int height);
    void drawSence() override;
    void resize(int width, int height) override;
    void initialRenderComponents(CSJVideoFormatType fmtType, 
                                 int width, int height) override;
    void updateVideoFrame(CSJVideoData *videoData) override;

    void setImage(const QString& imagePath) override;

protected:
    bool initD3D(int width, int height);

    bool createShaders();

    bool initShaders(std::string & vertShaderFile,
                     std::string & vertCso,
                     std::string & pixelShaderFile,
                     std::string & pixelCso);

    bool initRenderData();

    bool createDepthStencilView(int width, int height, 
                                ComPtr<ID3D11DepthStencilView> &depthStencilView);

    bool createRenderTargetView(int width, int height, 
                                ComPtr<ID3D11RenderTargetView> &targetView, 
                                bool ONScreen = true);

    void setViewPort(int width, int height);

    HRESULT CreateShaderFromFile(const WCHAR * csoFileNameOut,
                                 const WCHAR * hisFileName,
                                 LPCSTR entryPoint,
                                 LPCSTR shaderModel,
                                 ID3DBlob ** ppBlocbOut);

    bool createTextureByFmtType(CSJVideoFormatType fmtType, int width, int height);

    bool createD3DTexture(int width, int height, 
                          DXGI_FORMAT format, 
                          UINT miplevels, 
                          UINT arraySize,
                          D3D11_USAGE usage,
                          UINT bindFlags,
                          UINT CPUAccessFlags,
                          UINT MiscFlags,
                          ComPtr<ID3D11Texture2D>& tex);

    bool createD3DTextureWithResourceView(int width, int height, 
                                          DXGI_FORMAT format, 
                                          UINT miplevels, 
                                          UINT arraySize,
                                          D3D11_USAGE usage,
                                          UINT bindFlags,
                                          UINT CPUAccessFlags,
                                          UINT MiscFlags,
                                          ComPtr<ID3D11Texture2D>& tex,
                                          ComPtr<ID3D11ShaderResourceView>& resourceView,
                                          D3D11_SRV_DIMENSION srvDemension = D3D11_SRV_DIMENSION_TEXTURE2D);
    bool createTextureForRGBA(int width, int height);
    bool createTexturesForYUV420(int width, int height);
    void createTextureSampler();

    void updateFrameData();
    void updateRGBAFrame(CSJVideoData* videoData);
    void updateYUV420Frame(CSJVideoData* videoData);

    bool updateDynamicResource(ComPtr<ID3D11Resource> resource, rsize_t len, uint8_t* data);

    void bindRenderComponents();
    void bindYUV420TextureResources();
    void bindRGBATextureResources();
    void bindTextureResources();

    ComPtr<ID3D11DeviceContext> getCurrentContext();
    ComPtr<ID3D11Device>        getCurrentDevice();
    ComPtr<IDXGISwapChain>      getCurrentSwapChain();

private:
    ComPtr<ID3D11VertexShader>  m_pVertexShader;
    ComPtr<ID3D11PixelShader>   m_pPixelShader;
    ComPtr<ID3D11PixelShader>   m_pYuvPixelShader;

    ComPtr<ID3D11InputLayout>   m_pVertexLayout;   // input vertex layout;
    ComPtr<ID3D11Buffer>        m_pVertexBuffer;
    ComPtr<ID3D11Buffer>        m_pIndexBuffer;
    ComPtr<ID3D11Buffer>        m_ConstantBuffer;

    bool       m_bOnScreenRender = true;// OnScreen rendering or offscreen renderding, default is onscreen rendering.
    HWND       m_hMainWnd;              // 主窗口句柄
    int        m_renderFPS;             // 渲染帧率
    int        m_ClientWidth;           // 视口宽度
    int        m_ClientHeight;          // 视口高度
    bool       m_Enable4xMsaa;          // 是否开启4倍多重采样
    UINT       m_4xMsaaQuality;         // MSAA支持的质量等级             
    bool       m_initSuccess = false;   // whether Direct3D is initialized or not.
    HANDLE     m_pInitEvent = NULL;     // 初始化成功之后的通知
    CSJLogger *m_pLogger;  

    /* Direct3D 11 */
    ComPtr<ID3D11Device>          m_pd3dDevice;            // D3D11设备
    ComPtr<ID3D11DeviceContext>   m_pd3dImmediateContext;  // D3D11设备上下文
    ComPtr<IDXGISwapChain>        m_pSwapChain;            // D3D11交换链

    /* Direct3D 11.1 */
    ComPtr<ID3D11Device1>          m_pd3dDevice1;           // D3D11.1设备
    ComPtr<ID3D11DeviceContext1>   m_pd3dImmediateContext1; // D3D11.1设备上下文
    ComPtr<IDXGISwapChain1>        m_pSwapChain1;           // D3D11.1交换链

    ComPtr<ID3D11Texture2D>        m_pOffScreenTex;         // D3D11Texture of offscreen render. 
    ComPtr<ID3D11RenderTargetView> m_pOffScreenTargetView;  // 

    /* 常用资源 */
    ComPtr<ID3D11Texture2D>        m_pDepthStencilBuffer;  // 深度模板缓冲区
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;    // 渲染目标视图
    ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;    // 深度模板视图
    D3D11_VIEWPORT                 m_ScreenViewport;       // 视口

    /******************* video frame parameters *******************/
    int                            m_videoWidth;
    int                            m_videoHeight;
    CSJVideoFormatType             m_pixelFmt;
    bool                           m_bShowImage = false;

    std::mutex                     m_videoMtx;
    CSJVideoData*                  m_curVideoData;
    QString                        m_curImagePath;

    /*********************************************************
     * Textures for YUV
     ********************************************************/
    ComPtr<ID3D11Texture2D>        m_texY;
    ComPtr<ID3D11Texture2D>        m_texU;
    ComPtr<ID3D11Texture2D>        m_texV;

    std::array<ComPtr<ID3D11Texture2D>, 3>          m_texYUV;
    std::array<ComPtr<ID3D11ShaderResourceView>, 3> m_resourceViewYUV;

    ComPtr<ID3D11ShaderResourceView> m_pShaderResViewY;
    ComPtr<ID3D11ShaderResourceView> m_pShaderResViewU;
    ComPtr<ID3D11ShaderResourceView> m_pShaderResViewV;

    /*********************************************************
     * Texture for single fmt, such as rgba and so on.
     *********************************************************/
    ComPtr<ID3D11Texture2D>          m_singleTex;
    ComPtr<ID3D11Resource>           m_imageTex;
    ComPtr<ID3D11ShaderResourceView> m_pShaderResViewRGBA;

    bool                           m_bContentNeedUpdate = false;

    // sampler state
    ComPtr<ID3D11SamplerState>     m_pSamplerState;
};

#endif // __CSJVIDEORENDERERDXIMPL_H__
