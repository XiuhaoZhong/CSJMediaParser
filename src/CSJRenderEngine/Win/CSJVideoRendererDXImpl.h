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

    ComPtr<ID3D11DeviceContext> getCurrentContext();
    ComPtr<ID3D11Device>        getCurrentDevice();
    ComPtr<IDXGISwapChain>      getCurrentSwapChain();

private:
    std::atomic<bool>            m_bIsQuitRender = false;
    std::atomic<bool>            m_bInitSuccess = false;    // whether Direct3D is initialized or not.
    LARGE_INTEGER                m_timeFreq;
    std::unique_ptr<std::thread> m_pRenderThread = nullptr;
    bool                         m_bOnScreenRender = true;  // OnScreen rendering or offscreen renderding, default is onscreen rendering.
    HWND                         m_hMainWnd;                // Main window handle.
    int                          m_renderFPS;               // FPS of rendering.
    int                          m_ClientWidth;             // width of viewport.
    int                          m_ClientHeight;            // height of viewport.
    float                        m_PixelRatio;              // pixel ratio.
    bool                         m_Enable4xMsaa;            // enable msaa
    UINT                         m_4xMsaaQuality;           // level of msaa
    HANDLE                       m_pInitEvent = NULL;       // notify of initialize success.
    std::mutex                   m_renderMtx;

    /* Direct3D 11 */
    ComPtr<ID3D11Device>          m_pd3dDevice;            // D3D11 device.
    ComPtr<ID3D11DeviceContext>   m_pd3dImmediateContext;  // D3D11 context.
    ComPtr<IDXGISwapChain>        m_pSwapChain;            // D3D11 swapchain.

    /* Direct3D 11.1 */
    ComPtr<ID3D11Device1>          m_pd3dDevice1;           // D3D11.1 device.
    ComPtr<ID3D11DeviceContext1>   m_pd3dImmediateContext1; // D3D11.1 context.
    ComPtr<IDXGISwapChain1>        m_pSwapChain1;           // D3D11.1 swapchian.

    ComPtr<ID3D11Texture2D>        m_pOffScreenTex;         // D3D11Texture of offscreen render. 
    ComPtr<ID3D11RenderTargetView> m_pOffScreenTargetView;  // 

    /* 常用资源 */
    ComPtr<ID3D11Texture2D>        m_pDepthStencilBuffer;  // depth stencil buffer.
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;    // rendering target view. 
    ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;    // depth stencil view.
    D3D11_VIEWPORT                 m_ScreenViewport;       // screen viewport.

    double                         m_dVideoUpdateTimeStamp = 0.0;
    std::mutex                     m_videoMtx;
    CSJVideoFramePtr               m_pCurVideoData = nullptr;
    std::string                    m_curImagePath;

    CSJRenderablePtr m_pRGBARenderable = nullptr; // Rendering RGB/RGBA buffer and image file.
    CSJRenderablePtr m_pYUVRenderable  = nullptr; // Rendering YUV frame.
};

} //namespace csjrenderengine

#endif // __CSJVIDEORENDERERDXIMPL_H__
