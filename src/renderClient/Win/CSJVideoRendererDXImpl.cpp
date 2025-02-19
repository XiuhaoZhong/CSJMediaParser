#include "CSJVideoRendererDXImpl.h"

#include <d3dcompiler.h>

#include "DXTrace.h"

const D3D11_INPUT_ELEMENT_DESC CSJVideoRendererDXImpl::VertexPosColor::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

CSJVideoRendererDXImpl::CSJVideoRendererDXImpl() {

}

CSJVideoRendererDXImpl::~CSJVideoRendererDXImpl() {

}

bool CSJVideoRendererDXImpl::init(WId widgetID, int width, int height) {
    m_hMainWnd = (HWND)widgetID;
    if (!m_hMainWnd) {
        return false;
    }

    m_ClientWidth = width;
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

    ComPtr<IDXGIDevice>   dxgiDevice = nullptr;
    ComPtr<IDXGIAdapter>  dxgiAdapter = nullptr;
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
    hr = dxgiFactory1.As(&dxgiFactory2);
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

//    std::wstring vertshaderFile(L"resources\\DXShader\\Triangle_VS.hlsl");
//    std::wstring vertCso(L"resources\\DXShader\\Triangle_VS.cso");
//    std::wstring pixelShaderFile(L"resources\\DXShader\\Triangle_PS.hlsl");
//    std::wstring pixelCso(L"resources\\DXShader\\Triangle_PS.cso");
//    if (!initShaders(vertshaderFile, vertCso, pixelShaderFile, pixelCso)) {
//        return false;
//    }

//    if (!initRenderData()) {
//        return false;
//    }

    m_initSuccess = true;
    return true;
}

void CSJVideoRendererDXImpl::updateSence(double timeStamp) {

}

void CSJVideoRendererDXImpl::drawSence() {
    if (!m_initSuccess) {
        return ;
    }

    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float color[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // RGBA
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(),
                                                  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                  1.0f,
                                                  0.0f);

    HR(m_pSwapChain->Present(0, 0));
}

void CSJVideoRendererDXImpl::resize(int width, int height) {
    m_ClientWidth = width;
    m_ClientHeight = height;

    assert(m_pd3dImmediateContext);
    assert(m_pd3dDevice);
    assert(m_pSwapChain);

    if (m_pd3dDevice1 != nullptr) {
        assert(m_pd3dImmediateContext1);
        assert(m_pd3dDevice1);
        assert(m_pSwapChain1);
    }

    /* Release relative resource the renderer pipeline using. */
    m_pRenderTargetView.Reset();
    m_pDepthStencilView.Reset();
    m_pDepthStencilBuffer.Reset();

    /* Reset swap chain and recreate renderering target view. */
    ComPtr<ID3D11Texture2D> backBuffer;
    HR(m_pSwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight,
                                   DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                               reinterpret_cast<void**>(backBuffer.GetAddressOf())));

    HR(m_pd3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                            m_pRenderTargetView.GetAddressOf()));

    /* Setting debug object name. */
    //D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");
    backBuffer.Reset();

    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = m_ClientWidth;
    depthStencilDesc.Height = m_ClientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;

    /** DXGI_FORMAT_D24_UNORM_S8_UINT: 32bit z-buffer, 24 bits to depth and 8 bits
     *  for stencil.
     */
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    /* Using 4x MSAA, need to set MASS params to swap chain. */
    if (m_Enable4xMsaa) {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    } else {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }

    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    /* create depth buffer and depth tamplate view. */
    HR(m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr,
                                     m_pDepthStencilBuffer.GetAddressOf()));

    HR(m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr,
                                            m_pDepthStencilView.GetAddressOf()));

    /* Combine renderer target view and depth/tamplate buffer into pipeline. */
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(),
                                               m_pDepthStencilView.Get());

    /* Set viewport transition. */
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
    m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    /* 设置视口尺寸 */
    m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}

void CSJVideoRendererDXImpl::loadVideoComponents(CSJVideoFormatType fmtType, 
                                                 int width, int height) {
}

void CSJVideoRendererDXImpl::updateVideoFrame(CSJVideoData *videoData) {

}

bool CSJVideoRendererDXImpl::initShaders(std::wstring &vertShaderFile,
                                         std::wstring &vertCso,
                                         std::wstring &pixelShaderFile,
                                         std::wstring &pixelCso) {
    ComPtr<ID3DBlob> blob;

    /* Create vertex shader */
    HR(CreateShaderFromFile(vertCso.c_str(),
                            vertShaderFile.c_str(),
                            "VS",
                            "vs_5_0",
                            blob.ReleaseAndGetAddressOf()));

    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(),
                                        blob->GetBufferSize(),
                                        nullptr,
                                        m_pVertexShader.GetAddressOf()));

    /* Creating and Binding layout */
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout,
                                       ARRAYSIZE(VertexPosColor::inputLayout),
                                       blob->GetBufferPointer(),
                                       blob->GetBufferSize(),
                                       m_pVertexLayout.GetAddressOf()));

    /* Creating Pixel shader */
    HR(CreateShaderFromFile(pixelCso.c_str(),
                            pixelShaderFile.c_str(),
                            "PS",
                            "ps_5_0",
                            blob.ReleaseAndGetAddressOf()));

    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(),
                                       blob->GetBufferSize(),
                                       nullptr,
                                       m_pPixelShader.GetAddressOf()));

    return true;
}

bool CSJVideoRendererDXImpl::initRenderData() {
    static VertexPosColor vertices[] = {
        /**
         * 绘制一个三角形的顶点
         * 绘制三角形时，对于D3D绘制上下文来说，顶点顺时针设置，表示正面，能够绘制出来，
         * 如果顶点顺序为逆时针，则认为是背面，不会进行绘制
         * 所以一下三个顶点，如果交换第一个和第三个顶点的顺序，则不会绘制
         * 后面绘制失败的情况，起始的三个点都是逆时针顺序，所以绘制不成功。
         */
        {DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
        {DirectX::XMFLOAT3(0.5, -0.5f, 0.5f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
        {DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}
    };

    /* set vertex buffer desc */
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof vertices;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    /* create vertex buffer */
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    HRESULT hr = m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
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

void CSJVideoRendererDXImpl::initTexturesForYUV420() {

    D3D11_TEXTURE2D_DESC texDescY;
    texDescY.Width = m_videoWidth;
    texDescY.Height = m_videoHeight;
    texDescY.MipLevels = 1;
    texDescY.ArraySize = 1;
    texDescY.Format = DXGI_FORMAT_R8_UINT;
    texDescY.Usage = D3D11_USAGE_DEFAULT;
    texDescY.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDescY.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    texDescY.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if (m_Enable4xMsaa) {
        texDescY.SampleDesc.Count = 4;
        texDescY.SampleDesc.Quality = m_4xMsaaQuality - 1;
    } else {
        texDescY.SampleDesc.Count = 1;
        texDescY.SampleDesc.Quality = 0;
    }

    HRESULT hr = m_pd3dDevice->CreateTexture2D(&texDescY, nullptr, &m_texY);
    if (FAILED(hr)) {
        // texY create failed.
    }
}
