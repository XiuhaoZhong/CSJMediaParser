#include "CSJVideoRendererDXImpl.h"

#include <d3dcompiler.h>

#include <iostream>
#include <vector>

#include "WICTextureLoader11.h"
#include "DDSTextureLoader11.h"
#include "DXTrace.h"

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"
#include "CSJUtils/CSJStringUtils.h"
#include "CSJUtils/CSJMediaData.h"

#include "CSJDirectXHelper.h"

using namespace csjutils;
using namespace csjmediaengine;

namespace csjrenderengine {

CSJVideoRendererDXImpl::CSJVideoRendererDXImpl() {

}

CSJVideoRendererDXImpl::CSJVideoRendererDXImpl(CSJWindowID widgetID, 
                                               int width, 
                                               int height, 
                                               float pixelRatio)
    : m_hMainWnd((HWND)widgetID)
    , m_ClientWidth(width)
    , m_ClientHeight(height)
    , m_PixelRatio(pixelRatio) {
}

CSJVideoRendererDXImpl::~CSJVideoRendererDXImpl() {
    stopRender();
}

void CSJVideoRendererDXImpl::startRender() {
    m_pRenderThread.reset(new std::thread(&CSJVideoRendererDXImpl::renderFunc, this));
    LOG_Info("Renderer thread started.");
}

void CSJVideoRendererDXImpl::stopRender() {
    m_bIsQuitRender.store(true);

    if (m_pRenderThread && m_pRenderThread->joinable()) {
        m_pRenderThread->join();
        m_pRenderThread.reset();
    }

    m_bIsQuitRender.store(false);
    LOG_Info("Renderer thread exits.");
}

bool CSJVideoRendererDXImpl::updateScene(double timeStamp) {
    // If load the render content successfully or not.
    bool loadRenderContent = false;

    auto curContext = getCurrentContext();
    if (!curContext) {
        return loadRenderContent;
    }

    std::lock_guard<std::mutex> lock_guard(m_videoMtx);

    switch (m_renderContentType) {
    case CSJRenderContentType_RGB:
        // TODO: bind new rgb data.
    break;
    case CSJRenderContentType_RGBA:
        // TODO: bind new rgba data.
    break;
    case CSJRenderContentType_ImageFile:
        // TODO: bind new data from file.
    break;
    case CSJRenderContentType_I420:
        // TODO: bind new I420 data.
    break;
    case CSJRenderContentType_NV12:
        // TODO: bind new NV12 data.
    default:
    break;
    }

    return loadRenderContent;
}

bool CSJVideoRendererDXImpl::updateRenderDataForI420(double timeStamp) {
    auto delegate = m_pDelegate.lock();
    if (!delegate) {
        return false;
    }

    bool needFillYUVData = false;

    if (m_pCurVideoData) {
        if (timeStamp < m_dVideoUpdateTimeStamp) {
            LOG_Info("[Render Debug] Render the same video frame");
            // TODO: continue rendering current video frame.
        } else {
            CSJVideoFramePtr freshVideoFrame = delegate ? delegate->getNextVideoFrame() : nullptr;
            if (freshVideoFrame) {
                LOG_Info("[Render Debug] Render a new video frame");
                //LOG_Info("Current video frame, pts: %f, duration: %f", freshVideoFrame->pts, freshVideoFrame->duration);
                m_dVideoUpdateTimeStamp = timeStamp + freshVideoFrame->duration;
                m_pCurVideoData = freshVideoFrame;
                needFillYUVData = true;
            }
        }
    } else {
        CSJVideoFramePtr freshVideoFrame = delegate ? delegate->getNextVideoFrame() : nullptr;
        if (freshVideoFrame) {
            LOG_Info("[Render Debug] Render the first video frame");
            m_pCurVideoData = freshVideoFrame;
            m_dVideoUpdateTimeStamp = timeStamp + freshVideoFrame->duration;
            needFillYUVData = true;
        }
    }

    if (!needFillYUVData) {
        return true;
    }

    return true;
}

void CSJVideoRendererDXImpl::resize(int width, int height, float pixelRatio) {
    if (width < 1 || height < 1) {
        return ;
    }

    std::lock_guard<std::mutex> guard(m_renderMtx);

    // Must cancel the binding of render resources, in case crashes when resize
    // the window during the rendering.
    getCurrentContext()->OMSetRenderTargets(NULL, NULL, NULL);

    m_ClientWidth = width;
    m_ClientHeight = height;

    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    ComPtr<ID3D11Device> curDevice         = getCurrentDevice();
    ComPtr<IDXGISwapChain> curSwapChain    = getCurrentSwapChain();
    
    assert(curContext);
    assert(curDevice);
    assert(curSwapChain);

    /* Release relative resource the renderer pipeline using. */
    m_pRenderTargetView.Reset();
    m_pDepthStencilView.Reset();
    m_pDepthStencilBuffer.Reset();

    if (!CSJDirectXHelper::createRenderTargetView(curDevice, 
                                                  curSwapChain, 
                                                  m_ClientWidth, 
                                                  m_ClientHeight, 
                                                  m_pRenderTargetView)) {
        // TODO: create render target view failed.
        return ;
    }

    if (!CSJDirectXHelper::createDepthStencilViewResources(curDevice, 
                                                           m_pDepthStencilBuffer, 
                                                           m_pDepthStencilView, 
                                                           m_ClientWidth, 
                                                           m_ClientHeight, 
                                                           m_Enable4xMsaa, 
                                                           m_4xMsaaQuality, 
                                                           DXGI_FORMAT_D24_UNORM_S8_UINT)) {
        // TODO: create depth stencil view failed.
        return ;
    }

    /* Combine renderer target view and depth/tamplate buffer into pipeline. */
    curContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(),
                                   m_pDepthStencilView.Get());

    setViewPort(m_ClientWidth, m_ClientHeight);
}

void CSJVideoRendererDXImpl::initialRenderComponents(CSJPixelFormat fmtType, int width, int height) {

}

void CSJVideoRendererDXImpl::updateVideoFrame(CSJVideoFramePtr videoData) {
    std::lock_guard<std::mutex> guard(m_videoMtx);
    m_pCurVideoData = std::move(videoData);
}

void CSJVideoRendererDXImpl::setImage(const std::string & imagePath) {
    std::lock_guard lock(m_videoMtx);

    if (!m_pRGBARenderable) {
        return ;
    }

    m_pRGBARenderable->setFreshImage(imagePath);

    m_renderContentType = CSJRenderContentType_ImageFile;
}

bool CSJVideoRendererDXImpl::initRenderer() {
    if (m_bInitSuccess.load()) {
        return true;
    }

    if (!m_hMainWnd) {
        return false;
    }

    if (!initD3D(m_ClientWidth, m_ClientHeight)) {
        return false;
    }

    ComPtr<IDXGIDevice>   dxgiDevice   = nullptr;
    ComPtr<IDXGIAdapter>  dxgiAdapter  = nullptr;
    ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr; // D3D11.0(包含DXGI1.1)的接口类
    ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr; // D3D11.1(包含DXGI1.2)特有的接口类

    /** 为了正确创建 DXGI交换链，首先我们需要获取创建 D3D设备 的 DXGI工厂，否则会引发报错：
     * "IDXGIFactory::CreateSwapChain: This function is being called with a device
     * from a different IDXGIFactory."
     */

    /** ID3D11Device implements IDXGIDevice interface, so we could get a IDXGIDevice
     *  interface from a ID3D11Deivce.
     */
    HR(m_pd3dDevice.As(&dxgiDevice));
    HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
    HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1),
                              reinterpret_cast<void **>(dxgiFactory1.GetAddressOf())));

    /* check the object includes IDXGIFactory2 interface or not */
    dxgiFactory1.As(&dxgiFactory2);
    if (dxgiFactory2) {
        /* including IDXGIFactory2 interface */
        HR(m_pd3dDevice.As(&m_pd3dDevice1));
        HR(m_pd3dImmediateContext.As(&m_pd3dImmediateContext1));

        /* fill structs to descrip the swap chain */
        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = m_ClientWidth;
        sd.Height = m_ClientHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        if (m_Enable4xMsaa) {
            sd.SampleDesc.Count = 4;
            sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
        } else {
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
        }

        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
        fd.RefreshRate.Numerator = 60;
        fd.RefreshRate.Denominator = 1;
        fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fd.Windowed = TRUE;

        /* create swap chain for current widnow */
        HR(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(),
                                                m_hMainWnd,
                                                &sd,
                                                &fd,
                                                nullptr,
                                                m_pSwapChain1.GetAddressOf()));
        HR(m_pSwapChain1.As(&m_pSwapChain));
    } else {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = m_ClientWidth;
        sd.BufferDesc.Height = m_ClientHeight;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        /* 是否开启4倍多重采样？ */
        if (m_Enable4xMsaa) {
            sd.SampleDesc.Count = 4;
            sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
        } else {
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
        }
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.OutputWindow = m_hMainWnd;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;
        HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), &sd,
                                         m_pSwapChain.GetAddressOf()));
    }

    auto curDevice = getCurrentDevice();
    auto curSwapChain = getCurrentSwapChain();
    if (!CSJDirectXHelper::createDepthStencilViewResources(curDevice, 
                                                           m_pDepthStencilBuffer, 
                                                           m_pDepthStencilView, 
                                                           m_ClientWidth, m_ClientHeight, 
                                                           m_Enable4xMsaa, 
                                                           m_4xMsaaQuality, 
                                                           DXGI_FORMAT_D24_UNORM_S8_UINT)) {
        return false;
    }

    if (!CSJDirectXHelper::createRenderTargetView(curDevice,
                                                  curSwapChain,
                                                  m_ClientWidth,
                                                  m_ClientHeight,
                                                  m_pRenderTargetView)) {
        return false;
    }

    auto curContext = getCurrentContext();
    curContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(),
                                   m_pDepthStencilView.Get());

    setViewPort(m_ClientWidth, m_ClientHeight);

    QueryPerformanceFrequency(&m_timeFreq);

    struct DirectXParams params;
    params.enable4xMsaa = m_Enable4xMsaa;
    params.msaaQuality  = m_4xMsaaQuality;
    params.device       = curDevice;
    params.context      = curContext;

    m_pRGBARenderable = createRenderableWithType(CSJRenderableType_RGBA);
    if (!m_pRGBARenderable->initRenderable(params)) {
        LOG_Warn("RGBA renderable init failed!");
    }

    m_pYUVRenderable = createRenderableWithType(CSJRenderableType_YUV);
    if (!m_pYUVRenderable->initRenderable(params)) {
        LOG_Warn("YUV renderable init failed!");
    }

    m_bInitSuccess = true;
    return true;
}

bool CSJVideoRendererDXImpl::initD3D(int width, int height) {
    if (width == 0 || height == 0) {
        return false;
    }

    m_ClientWidth  = width;
    m_ClientHeight = height;

    HRESULT hr = S_OK;

    /* create D3D device and D3D device context. */
    UINT createDeviceFlags = 0;
#if defined(DEGUB) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    /* Deiver type array. */
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    /* Feature level array */
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;
    D3D_DRIVER_TYPE d3dDriverType;
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        d3dDriverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr,
                               d3dDriverType,
                               nullptr,
                               createDeviceFlags,
                               featureLevels,
                               numFeatureLevels,
                               D3D11_SDK_VERSION,
                               m_pd3dDevice.GetAddressOf(),
                               &featureLevel,
                               m_pd3dImmediateContext.GetAddressOf());

        if (SUCCEEDED(hr)) {
            break;
        }
    }

    if (FAILED(hr)) {
        return false;
    }

    /* check if support feature level 11.0 or 11.1 */
    if (featureLevel != D3D_FEATURE_LEVEL_11_0 &&
        featureLevel != D3D_FEATURE_LEVEL_11_1) {
        return false;
    }

    /* check supported quality level of MSAA */
    m_pd3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
                                                4,
                                                &m_4xMsaaQuality);
    assert(m_4xMsaaQuality > 0);

    return true;
}

void CSJVideoRendererDXImpl::renderFunc() {
    if (!initRenderer()) {
        LOG_Error("Failed to initialize render.");
        return ;
    }
    
    auto swapChain = getCurrentSwapChain();
    if (!swapChain) {
        LOG_Error("There is no valid swap chain.");
        return ;
    }

    ComPtr<IDXGIOutput> output = nullptr;
    HRESULT hr = swapChain->GetContainingOutput(&output);
    if (FAILED(hr) || !output) {
        LOG_Error("Failed to get containing output.");
        return ;
    }

    // static int frame_count = 0;

    while (!m_bIsQuitRender.load()) {
        //output->WaitForVBlank();

        if (m_bIsQuitRender.load()) {
            break;
        }

        /** Solution for render video frame, this is just for regular video frame rendering,
         *  when seeking there will need other solutions. 
         *  if (curVideoFrame) {
         *      if (curTimeStamp < videoFramdUpdateTimeStamp) {
         *          render curVideoFrame
         *      } else {
         *          curVideoFrame = getNextFrame();
         *          render curVideoFrame
         *          update videoFrameUpdateTimeStamp
         *      }
         *  } else {
         *      curVideoFrame = getNextFrame();
         *      update videoFrameUpdateTimeStamp
         *      render curVideoFrame.
         *  }
        */

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        double timeStamp = counter.QuadPart / (double)m_timeFreq.QuadPart;
        LOG_Info("Current time stamp: %f", timeStamp);

        // TODO: get a new video frame.
        auto delegate = m_pDelegate.lock();
        if (delegate) {
            delegate->beforeARenderingTick();
        }

        drawScene(timeStamp);
        //LOG_Info("%d th frame rendered! timeStamp: %f", frame_count++, timeStamp);
        if (delegate) {
            delegate->afterARenderingTick();
        }
    }
}

void CSJVideoRendererDXImpl::drawScene(double timeStamp) {
    std::lock_guard<std::mutex> guard(m_renderMtx);

    auto curContext = getCurrentContext();
    auto curSwapChain = getCurrentSwapChain();

    assert(curContext);
    assert(curSwapChain);

    static float color[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // RGBA
    curContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
    curContext->ClearDepthStencilView(m_pDepthStencilView.Get(),
                                      D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                      1.0f,
                                      0.0f);

    // check shader.  
    bool need_render = updateScene(timeStamp);

    if (m_renderContentType == CSJRenderContentType_ImageFile && m_pRGBARenderable) {
        auto device = getCurrentDevice();
        m_pRGBARenderable->drawRenderable(timeStamp);
    }

    // TODO: Using Present(1,0), and then use double textures to render
    HR(curSwapChain->Present(1, 0));
}

void CSJVideoRendererDXImpl::setViewPort(int width, int height) {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();

    assert(curContext);

    /* Set viewport transition. */
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width    = static_cast<float>(m_ClientWidth);
    m_ScreenViewport.Height   = static_cast<float>(m_ClientHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    /* 设置视口尺寸 */
    curContext->RSSetViewports(1, &m_ScreenViewport);
}

bool CSJVideoRendererDXImpl::updateDynamicResource(ComPtr<ID3D11Resource> resource, 
                                                   rsize_t len, 
                                                   uint8_t * data) {
    
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        LOG_Error("Current context is null!");
        return false;
    }

    if (!resource || !data) {
        LOG_Error("Resource or data is null!");
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE mappedData;
    HRESULT hr = curContext->Map(resource.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    if (FAILED(hr)) {
        // TODO: map texture failed.
        return false;
    }

    memcpy_s(mappedData.pData, len, data, len);
    curContext->Unmap(resource.Get(), 0);

    return true;
}

ComPtr<ID3D11DeviceContext> CSJVideoRendererDXImpl::getCurrentContext() {
    return m_pd3dImmediateContext1 ? m_pd3dImmediateContext1 : m_pd3dImmediateContext;
}

ComPtr<ID3D11Device> CSJVideoRendererDXImpl::getCurrentDevice() {
    return m_pd3dDevice1 ? m_pd3dDevice1 : m_pd3dDevice;
}

ComPtr<IDXGISwapChain> CSJVideoRendererDXImpl::getCurrentSwapChain() {
    return m_pSwapChain1 ? m_pSwapChain1 : m_pSwapChain;
}

} // namespace csjrenderengine
