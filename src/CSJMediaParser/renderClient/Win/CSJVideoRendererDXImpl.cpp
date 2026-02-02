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

using namespace csjutils;
using namespace csjmediaengine;

const D3D11_INPUT_ELEMENT_DESC CSJVideoRendererDXImpl::VertexPosColor::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

CSJVideoRendererDXImpl::CSJVideoRendererDXImpl()
    : m_pLogger(CSJLogger::getLoggerInst()) {
    m_pixelFmt = CSJVIDEO_FMT_NONE;
    m_bContentNeedUpdate = false;
}

CSJVideoRendererDXImpl::~CSJVideoRendererDXImpl() {

}

bool CSJVideoRendererDXImpl::init(WId widgetID, int width, int height) {
    m_hMainWnd = (HWND)widgetID;
    if (!m_hMainWnd) {
        return false;
    }

    if (!initD3D(width, height)) {
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

    if (!createDepthStencilView(m_ClientWidth, m_ClientHeight, m_pDepthStencilView)) {
        return false;
    }

    if (!createRenderTargetView(m_ClientWidth, m_ClientHeight, m_pRenderTargetView)) {
        return false;
    }

    if (!createShaders()) {
        return false;
    }

    if (!initRenderData()) {
        return false;
    }

    setViewPort(m_ClientWidth, m_ClientHeight);

    m_initSuccess = true;
    return true;
}

bool CSJVideoRendererDXImpl::initForOffScreen(int width, int height) {
    if (!initD3D(width, height)) {
        return false;
    }

    m_bOnScreenRender = false;
    if (!createRenderTargetView(width, height, m_pRenderTargetView, m_bOnScreenRender)) {
        return false;
    }

    getCurrentContext()->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

    if (!createShaders()) {
        return false;
    }

    if (!initRenderData()) {
        return false;
    }

    setViewPort(m_ClientWidth, m_ClientHeight);

    m_initSuccess = true;

    return true;
}

bool CSJVideoRendererDXImpl::updateSence(double timeStamp) {
    // If load the render content successfully or not.
    bool loadRenderContent = false;

    auto curContext = getCurrentContext();
    if (!curContext) {
        return loadRenderContent;
    }

    std::lock_guard<std::mutex> lock_guard(m_videoMtx);

    // Check if the rendering content needed to update.
    if (!m_bContentNeedUpdate) {
        return loadRenderContent;
    }

    // when pixel format changed or show a image, need to recreate all the resources.
    if (m_curVideoData->getFmtType() != m_pixelFmt || m_bShowImage) {
        // Compare pixel format
        //     a) The pixel format changes
        //     1. Set the accordance shader
        //     2. Release the previous texture resources, and create new texture resources
        //     3. Compute the coordinates, and bind to shader
        //     4. Record the new pixel format and picture size into class members. 
        // the pixel format changed, should reallocate all textures, and record the new size and pixel format 
        loadRenderContent = createTextureByFmtType(m_curVideoData->getFmtType(), 
                                                   m_curVideoData->getWidth(), 
                                                   m_curVideoData->getHeight());
        m_pixelFmt = m_curVideoData->getFmtType();
        if (loadRenderContent) {
            bindRenderComponents();
            bindTextureResources();
        }
    } else if (m_curVideoData->getWidth() != m_videoWidth || m_curVideoData->getHeight() != m_videoHeight) {
        // rendering pixel format is not changed, but the size of rendering content changed, so recreate texture(s).
        // Don't need to re bind shaders.
        loadRenderContent = createTextureByFmtType(m_curVideoData->getFmtType(), 
                                                   m_curVideoData->getWidth(), 
                                                   m_curVideoData->getHeight());

        if (loadRenderContent) {
            bindTextureResources();
        } 
    } else {
        // the new videoData's pixel format, width, height are all same with current value, only needs to update the 
        // texture content.
        // update texture content with pixel format.
        updateFrameData();
    }

    // If load render content failed, set the render pixel format to CSJVIDEO_FMT_NONE, and thus won't render anything.
    m_pixelFmt = loadRenderContent ? m_pixelFmt : CSJVIDEO_FMT_NONE;
    m_bContentNeedUpdate = false;

    return loadRenderContent;
}

bool CSJVideoRendererDXImpl::fillTextureData(uint8_t *buf, int width, int height) {
    return false;
}

void CSJVideoRendererDXImpl::drawSence() {
    if (!m_initSuccess) {
        return ;
    }

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
    bool need_render = updateSence(0.0);
    if (m_pixelFmt != CSJVIDEO_FMT_NONE) {
        curContext->DrawIndexed(6, 0, 0);
    }

    HR(curSwapChain->Present(0, 0));
}

void CSJVideoRendererDXImpl::resize(int width, int height) {

    // Must cancel the binding of render resources, in case crashes when resize
    // the window during the rendering.
    getCurrentContext()->OMSetRenderTargets(NULL, NULL, NULL);

    m_ClientWidth = width;
    m_ClientHeight = height;

    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    ComPtr<ID3D11Device> curDevice         = getCurrentDevice();
    
    assert(curContext);
    assert(curDevice);

    /* Release relative resource the renderer pipeline using. */
    m_pRenderTargetView.Reset();
    m_pDepthStencilView.Reset();
    m_pDepthStencilBuffer.Reset();

    if (!createRenderTargetView(width, height, m_pRenderTargetView, m_bOnScreenRender)) {
        // TODO: create render target view failed.
        return ;
    }

    if (!createDepthStencilView(m_ClientWidth, m_ClientHeight, m_pDepthStencilView)) {
        // TODO: create depth stencil view failed;
        return ;
    }

    /* Combine renderer target view and depth/tamplate buffer into pipeline. */
    curContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(),
                                   m_pDepthStencilView.Get());

    setViewPort(m_ClientWidth, m_ClientHeight);
}

void CSJVideoRendererDXImpl::initialRenderComponents(CSJVideoFormatType fmtType, 
                                                     int width, int height) {
}

void CSJVideoRendererDXImpl::updateVideoFrame(CSJVideoData *videoData) {
    std::lock_guard lock(m_videoMtx);
    m_curVideoData = std::move(videoData);
    m_bContentNeedUpdate = true;
}

void CSJVideoRendererDXImpl::setImage(const QString & imagePath) {
    std::lock_guard lock(m_videoMtx);
    m_curImagePath = imagePath;

    CSJVideoData *fakeVideoData = new CSJVideoData(CSJVIDEO_FMT_RGB24, nullptr, 0, 0);
    m_curVideoData = std::move(fakeVideoData);

    m_bShowImage = true;
    m_bContentNeedUpdate = true;
}

bool CSJVideoRendererDXImpl::initD3D(int width, int height) {
    if (width == 0 || height == 0) {
        return false;
    }

    m_ClientWidth  = width;
    m_ClientHeight = height;
    m_pixelFmt     = CSJVIDEO_FMT_NONE;

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
        //MessageBox(0, L"D3DllCreateDevice Failed", 0, 0);
        return false;
    }

    /* check if support feature level 11.0 or 11.1 */
    if (featureLevel != D3D_FEATURE_LEVEL_11_0 &&
        featureLevel != D3D_FEATURE_LEVEL_11_1) {
        //MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
        return false;
    }

    /* check supported quality level of MSAA */
    m_pd3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
                                                4,
                                                &m_4xMsaaQuality);
    assert(m_4xMsaaQuality > 0);

    return true;
}

bool CSJVideoRendererDXImpl::createShaders() {
    CSJPathTool *pathTool = CSJPathTool::getInstance();
    std::string vertshaderFile = pathTool->getShaderDir().append("DXVertexShader.hlsl").string();
    std::string vertCso = pathTool->getShaderDir().append("DXVertexShader.cso").string();

    // Create pixel shader with video pixel type, and default is rgba.
    std::string pixelShaderFile = pathTool->getShaderDir().append("DXRGBAShader.hlsl").string();
    std::string pixelCso = pathTool->getShaderDir().append("DXRGBAShader.cso").string();

    if (m_pixelFmt == CSJVIDEO_FMT_YUV420P) {
        pixelShaderFile = pathTool->getShaderDir().append("DXYUVShader.hlsl").string();
        pixelCso = pathTool->getShaderDir().append("DXYUVShader.cso").string();
    }
    
    if (!initShaders(vertshaderFile, vertCso, pixelShaderFile, pixelCso)) {
        m_pLogger->log_error("Shader initialize failed!");
        return false;
    }

    return true;
}

bool CSJVideoRendererDXImpl::initShaders(std::string &vertShaderFile,
                                         std::string &vertCso,
                                         std::string &pixelShaderFile,
                                         std::string &pixelCso) {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return false;
    }

    ComPtr<ID3DBlob> blob;

    std::wstring vertShaderaPath = csjutils::CSJStringUtil::string2wstring(vertShaderFile);
    std::wstring vertCsoPath = csjutils::CSJStringUtil::string2wstring(vertCso);

    /* Create vertex shader */
    HR(CreateShaderFromFile(vertCsoPath.c_str(),
                            vertShaderaPath.c_str(),
                            "main",
                            "vs_5_0",
                            blob.ReleaseAndGetAddressOf()));

    HR(curDevice->CreateVertexShader(blob->GetBufferPointer(),
                                     blob->GetBufferSize(),
                                     nullptr,
                                     m_pVertexShader.ReleaseAndGetAddressOf()));

    /* Creating and Binding layout */
    HR(curDevice->CreateInputLayout(VertexPosColor::inputLayout,
                                    ARRAYSIZE(VertexPosColor::inputLayout),
                                    blob->GetBufferPointer(),
                                    blob->GetBufferSize(),
                                    m_pVertexLayout.ReleaseAndGetAddressOf()));

    std::wstring pixelCsoPath = csjutils::CSJStringUtil::string2wstring(pixelCso);
    std::wstring pixelShaderPath = csjutils::CSJStringUtil::string2wstring(pixelShaderFile);

    /* Creating Pixel shader */
    HR(CreateShaderFromFile(pixelCsoPath.c_str(),
                            pixelShaderPath.c_str(),
                            "main",
                            "ps_5_0",
                            blob.ReleaseAndGetAddressOf()));

    HR(curDevice->CreatePixelShader(blob->GetBufferPointer(),
                                    blob->GetBufferSize(),
                                    nullptr,
                                    m_pPixelShader.ReleaseAndGetAddressOf()));

    return true;
}

bool CSJVideoRendererDXImpl::initRenderData() {
    static VertexPosColor vertices[] = {
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
    HRESULT hr = m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.ReleaseAndGetAddressOf());
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
    hr = m_pd3dDevice1->CreateBuffer(&indexBufferDes, &indexData, m_pIndexBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    createTextureSampler();

    return true;
}

bool CSJVideoRendererDXImpl::createDepthStencilView(int width, int height, 
                                                    ComPtr<ID3D11DepthStencilView> &depthStencilView) {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return false;
    }
    
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width     = m_ClientWidth;
    depthStencilDesc.Height    = m_ClientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;

    /** DXGI_FORMAT_D24_UNORM_S8_UINT: 32bit z-buffer, 24 bits to depth and 8 bits
     *  for stencil.
     */
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    /* Using 4x MSAA, need to set MASS params to swap chain. */
    if (m_Enable4xMsaa) {
        depthStencilDesc.SampleDesc.Count   = 4;
        depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    } else {
        depthStencilDesc.SampleDesc.Count   = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }

    depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags      = 0;

    /* create depth buffer and depth tamplate view. */
    HR(curDevice->CreateTexture2D(&depthStencilDesc, nullptr,
                                  m_pDepthStencilBuffer.ReleaseAndGetAddressOf()));

    HR(curDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr,
                                         m_pDepthStencilView.ReleaseAndGetAddressOf()));
    
    return true;
}

bool CSJVideoRendererDXImpl::createRenderTargetView(int width, int height, 
                                                    ComPtr<ID3D11RenderTargetView> &targetView, 
                                                    bool ONScreen) {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return false;
    }

    if (ONScreen) {
        ComPtr<IDXGISwapChain> curSwapChian = getCurrentSwapChain();
        assert(curSwapChian);

        /* Reset swap chain and recreate renderering target view. */
        ComPtr<ID3D11Texture2D> backBuffer;
        HR(curSwapChian->ResizeBuffers(1, m_ClientWidth, m_ClientHeight,
                                    DXGI_FORMAT_R8G8B8A8_UNORM, 0));

        HR(curSwapChian->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                reinterpret_cast<void**>(backBuffer.GetAddressOf())));

        HR(curDevice->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                            m_pRenderTargetView.ReleaseAndGetAddressOf()));

        /* Setting debug object name. */
        //D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");
        backBuffer.Reset();
    } else {
        bool res = createD3DTexture(width, 
                                    height, 
                                    DXGI_FORMAT_R8G8B8A8_UNORM, 
                                    1, 
                                    1, 
                                    D3D11_USAGE_DEFAULT,
                                    D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,  
                                    0,
                                    D3D11_RESOURCE_MISC_GENERATE_MIPS, m_pOffScreenTex);

        if (!res) {
            return false;
        }

        HRESULT hr = curDevice->CreateRenderTargetView(m_pOffScreenTex.Get(), 
                                                                nullptr, 
                                                                m_pRenderTargetView.ReleaseAndGetAddressOf());
        
        if (FAILED(hr)) {
            // TODO: Create render target view failed.
            return false;
        }

    }

    return true;
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

HRESULT CSJVideoRendererDXImpl::CreateShaderFromFile(const WCHAR *csoFileNameOut,
                                                     const WCHAR *hisFileName,
                                                     LPCSTR entryPoint,
                                                     LPCSTR shaderModel,
                                                     ID3DBlob **ppBlocbOut) {
    HRESULT hr = S_OK;

    /* Find the vertex shader which already be compiled */
    if (csoFileNameOut && D3DReadFileToBlob(csoFileNameOut, ppBlocbOut) == S_OK) {
        return hr;
    } else {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        /* Setting D3DCOMPILE_DEBUG flag is to get the shader's debug infomation.
         * Setting this flag could improve the debug experience.
         *
         **/
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        /* Disable the optimizing under the DEBUG enviroment to avoid some
           unreasonable case.
         */
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ComPtr<ID3DBlob> errorBlob = nullptr;
        hr = D3DCompileFromFile(hisFileName,
                                nullptr,
                                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                entryPoint,
                                shaderModel,
                                dwShaderFlags,
                                0,
                                ppBlocbOut,
                                &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob != nullptr) {
                OutputDebugString(reinterpret_cast<const WCHAR*>(errorBlob->GetBufferPointer()));
            }

            return hr;
        }

        if (csoFileNameOut) {
            return D3DWriteBlobToFile(*ppBlocbOut, csoFileNameOut, FALSE);
        }
    }

    return hr;
}

bool CSJVideoRendererDXImpl::createTextureByFmtType(CSJVideoFormatType fmtType, int width, int height) {
    bool res = false;
    switch (fmtType) {
    case CSJVIDEO_FMT_YUV420P:
        res = createTexturesForYUV420(width, height);    
        break;
    case CSJVIDEO_FMT_RGB24:
        res = createTextureForRGBA(width, height);
        break;
    default:
        // TODO: Not supported video fmt;
        break;
    }

    return res;
}

bool CSJVideoRendererDXImpl::createD3DTexture(int width, 
                                              int height, 
                                              DXGI_FORMAT format,
                                              UINT miplevels, 
                                              UINT arraySize, 
                                              D3D11_USAGE usage, 
                                              UINT bindFlags, 
                                              UINT CPUAccessFlags, 
                                              UINT MiscFlags, 
                                              ComPtr<ID3D11Texture2D> &tex) {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return false;
    }

    D3D11_TEXTURE2D_DESC    texDesc{};
    texDesc.Width           = width;
    texDesc.Height          = height;
    texDesc.MipLevels       = miplevels;
    texDesc.ArraySize       = arraySize;
    texDesc.Format          = format;
    texDesc.Usage           = usage;
    texDesc.BindFlags       = bindFlags;
    texDesc.CPUAccessFlags  = CPUAccessFlags;
    texDesc.MiscFlags       = MiscFlags;
    if (m_Enable4xMsaa) {
        texDesc.SampleDesc.Count   = 4;
        texDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    } else {
        texDesc.SampleDesc.Count   = 1;
        texDesc.SampleDesc.Quality = 0;
    }

    HRESULT hr = curDevice->CreateTexture2D(&texDesc, 
                                            nullptr, 
                                            tex.ReleaseAndGetAddressOf());

    if (FAILED(hr)) {
        // texY create failed.
        return false;
    }
    
    return true;
}

bool CSJVideoRendererDXImpl::createD3DTextureWithResourceView(int width, int height, 
                                                              DXGI_FORMAT format, 
                                                              UINT miplevels, 
                                                              UINT arraySize,
                                                              D3D11_USAGE usage,
                                                              UINT bindFlags,
                                                              UINT CPUAccessFlags,
                                                              UINT MiscFlags,
                                                              ComPtr<ID3D11Texture2D>& tex,
                                                              ComPtr<ID3D11ShaderResourceView>& resourceView,
                                                              D3D11_SRV_DIMENSION srvDemension /*= D3D11_SRV_DIMENSION_TEXTURE2D*/) {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return false;
    }

    bool res = createD3DTexture(width, 
                                height, 
                                format, 
                                miplevels, 
                                arraySize, 
                                usage, 
                                bindFlags, 
                                CPUAccessFlags, 
                                MiscFlags, 
                                tex);

    if (!res) {
        return false;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC   srvDesc{};
    srvDesc.Format                    = format;
    srvDesc.ViewDimension             = srvDemension;
    srvDesc.Texture2D.MipLevels       = miplevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = curDevice->CreateShaderResourceView(tex.Get(),
                                             &srvDesc, 
                                             resourceView.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

bool CSJVideoRendererDXImpl::createTextureForRGBA(int width, int height) {
    bool res = false;

    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return res;
    }

    if (m_bShowImage) {
        std::wstring image = m_curImagePath.toStdWString();
        ComPtr<ID3D11Device> curDevice = getCurrentDevice();
        assert(curDevice);
        HRESULT hr = DirectX::CreateWICTextureFromFile(curDevice.Get(), 
                                                       image.c_str(), 
                                                       (ID3D11Resource **)m_singleTex.ReleaseAndGetAddressOf(), 
                                                       m_pShaderResViewRGBA.ReleaseAndGetAddressOf(), 
                                                       0);

        if (FAILED(hr)) {
            return res;
        }

        D3D11_TEXTURE2D_DESC texDesc;
        m_singleTex->GetDesc(&texDesc);
        std::wcout << L"[Log] load image: " << image 
                   << L"with: " << texDesc.Width 
                   << L" , height: " << texDesc.Height << std::endl;
        m_bShowImage = false;
        res = true;
    } else {
        HRESULT hr = DirectX::CreateDDSTextureFromMemory(curDevice.Get(), 
                                                         m_curVideoData->getData(), 
                                                         m_curVideoData->getWidth() * m_curVideoData->getHeight() * 4,
                                                         (ID3D11Resource **)m_singleTex.ReleaseAndGetAddressOf(), 
                                                         m_pShaderResViewRGBA.ReleaseAndGetAddressOf(), 
                                                         0);
        res = FAILED(hr) ? false : true;
    }

    return res;
}

bool CSJVideoRendererDXImpl::createTexturesForYUV420(int width, int height) {
    bool res = false;

    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return res;
    }

    for (size_t i = 0; i < m_texYUV.size(); i++) {
        int w = i > 0 ? width / 2 : width;
        int h = i > 0 ? height / 2 : height;

        res = createD3DTextureWithResourceView(width, 
                                               height, 
                                               DXGI_FORMAT_R8_UINT,
                                               1,
                                               1,
                                               D3D11_USAGE_DEFAULT, 
                                               D3D11_BIND_SHADER_RESOURCE, 
                                               D3D11_CPU_ACCESS_WRITE,
                                               D3D11_RESOURCE_MISC_GENERATE_MIPS,
                                               m_texYUV[i],
                                               m_resourceViewYUV[i]);
                                
        if (!res) {
            // TODO: create tex and resouce
            break;
        }
    }

    return res;
}

void CSJVideoRendererDXImpl::createTextureSampler() {
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD         = 0;
    samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    assert(curDevice);
    HRESULT hr = curDevice->CreateSamplerState(&samplerDesc, m_pSamplerState.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        // TODO: sampler create failed.
    }
}

void CSJVideoRendererDXImpl::updateFrameData() {
    switch (m_pixelFmt) {
    case CSJVIDEO_FMT_RGB24:
        updateRGBAFrame(m_curVideoData);
        break;
    case CSJVIDEO_FMT_YUV420P:
        updateYUV420Frame(m_curVideoData);
        break;
    default:
        m_pLogger->log_error("Update a unsupported pixel format!");
        break;
    }
}

void CSJVideoRendererDXImpl::updateRGBAFrame(CSJVideoData *videoData) {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    bool updateState = updateDynamicResource(m_singleTex, 
                                             videoData->getWidth() * videoData->getHeight(), 
                                             videoData->getData());
    if (!updateState) {
        m_pLogger->log_error("Update rgba data failed!");
    }
}

void CSJVideoRendererDXImpl::updateYUV420Frame(CSJVideoData * videoData) {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    m_pLogger->log_info("Update yuv data");

    rsize_t len = m_videoWidth * m_videoHeight;
    bool updateState = updateDynamicResource(m_texYUV[0], len, videoData->getyuvY());
    if (!updateState) {
        m_pLogger->log_error("Update Y plane data failed!");
        return ;
    }

    updateState = updateDynamicResource(m_texYUV[1], len / 4, videoData->getyuvU());
    if (!updateState) {
        m_pLogger->log_error("Update U plane data failed!");
        return ;
    }

    updateState = updateDynamicResource(m_texYUV[2], len / 4, videoData->getyuvV());
    if (!updateState) {
        m_pLogger->log_error("Update V plane data failed!");
        return ;
    }

    m_pLogger->log_info("Update yuv data successfully!");
}

bool CSJVideoRendererDXImpl::updateDynamicResource(ComPtr<ID3D11Resource> resource, 
                                                   rsize_t len, 
                                                   uint8_t * data) {
    
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        m_pLogger->log_error("Current context is null!");
        return false;
    }

    if (!resource || !data) {
        m_pLogger->log_error("Resource or data is null!");
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

void CSJVideoRendererDXImpl::bindRenderComponents() {
    auto curContext = getCurrentContext();

    switch(m_pixelFmt) {
    case CSJVIDEO_FMT_RGB24: {
        UINT stride = sizeof(VertexPosColor);
        UINT offset = 0;

        curContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
        curContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

        curContext->IASetInputLayout(m_pVertexLayout.Get());
        curContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        curContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        curContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        curContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
    }
    break;
    case CSJVIDEO_FMT_NV12:
    case CSJVIDEO_FMT_YUV420P:
        // TODO: Bind yuv rendering shaders.
        break;
        
    }
}

void CSJVideoRendererDXImpl::bindYUV420TextureResources() {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    for (size_t i = 0; i < m_resourceViewYUV.size(); i++) {
        curContext->PSSetShaderResources(i, 1, m_resourceViewYUV[i].GetAddressOf());    
    }
}

void CSJVideoRendererDXImpl::bindRGBATextureResources() {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    if (m_pShaderResViewRGBA) {
        curContext->PSSetShaderResources(3, 1, m_pShaderResViewRGBA.GetAddressOf());
    }                                      
}

void CSJVideoRendererDXImpl::bindTextureResources() {
    switch (m_pixelFmt) {
    case CSJVIDEO_FMT_YUV420P:
        bindYUV420TextureResources();
        break;
    case CSJVIDEO_FMT_RGB24:
        bindRGBATextureResources();
        break;
    default:
        break;
    }
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
