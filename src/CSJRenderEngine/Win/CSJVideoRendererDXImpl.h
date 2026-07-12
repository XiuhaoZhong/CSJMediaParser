#ifndef __CSJVIDEORENDERERDXIMPL_H__
#define __CSJVIDEORENDERERDXIMPL_H__

#include <windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <directxmath.h>

#include <mutex>
#include <array>
#include <memory>

#include "CSJRenderEngine/CSJVideoRenderer.h"
#include "Renderables/CSJRenderableBase.h"

constexpr int g_YUVTexNum = 2;

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using csjutils::CSJPixelFormat;
using csjutils::CSJVideoFramePtr;

namespace csjrenderengine {

class CSJVideoRendererDXImpl : public CSJVideoRenderer {
public:
    CSJVideoRendererDXImpl();
    CSJVideoRendererDXImpl(CSJWindowID widgetID, int width, int height, float pixelRatio);
    ~CSJVideoRendererDXImpl();
    
    void startRender() override;
    void stopRender() override;
    bool fillTextureData(uint8_t *buf, int width, int height) override {return false;};
    void resize(int width, int height, float pixelRatio) override;
    void initialRenderComponents(CSJPixelFormat fmtType, 
                                 int width, int height) override;
    void updateVideoFrame(CSJVideoFramePtr videoData) override;

    void setImage(const std::string& imagePath) override;

protected:
    bool initRenderer();

    bool initD3D(int width, int height);

    void renderFunc();

    void drawScene(double timeStamp);
    bool updateScene(double timeStamp);

    /* Render I420 video frames. */
    bool updateRenderDataForI420(double timeStamp);

    void setViewPort(int width, int height);

    bool updateDynamicResource(ComPtr<ID3D11Resource> resource, rsize_t len, uint8_t* data);

    ComPtr<ID3D11DeviceContext> getCurrentContext();
    ComPtr<ID3D11Device>        getCurrentDevice();
    ComPtr<IDXGISwapChain>      getCurrentSwapChain();

private:
    std::atomic<bool>            m_bIsQuitRender = false;
    std::atomic<bool>            m_bInitSuccess = false;   // whether Direct3D is initialized or not.
    LARGE_INTEGER                m_timeFreq;
    std::unique_ptr<std::thread> m_pRenderThread = nullptr;
    bool                         m_bOnScreenRender = true;// OnScreen rendering or offscreen renderding, default is onscreen rendering.
    HWND                         m_hMainWnd;              // 主窗口句柄
    int                          m_renderFPS;             // 渲染帧率
    int                          m_ClientWidth;           // 视口宽度
    int                          m_ClientHeight;          // 视口高度
    float                        m_PixelRatio;            // 像素比率
    bool                         m_Enable4xMsaa;          // 是否开启4倍多重采样
    UINT                         m_4xMsaaQuality;         // MSAA支持的质量等级             
    HANDLE                       m_pInitEvent = NULL;     // 初始化成功之后的通知
    std::mutex                   m_renderMtx;

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

    double                         m_dVideoUpdateTimeStamp = 0.0;
    std::mutex                     m_videoMtx;
    CSJVideoFramePtr               m_pCurVideoData = nullptr;
    std::string                    m_curImagePath;

    // Rendering RGB/RGBA buffer and image file.
    CSJRenderablePtr m_pRGBARenderable = nullptr;
    // Rendering YUV frame.
    CSJRenderablePtr m_pYUVRenderable  = nullptr;
};

} //namespace csjrenderengine

#endif // __CSJVIDEORENDERERDXIMPL_H__
