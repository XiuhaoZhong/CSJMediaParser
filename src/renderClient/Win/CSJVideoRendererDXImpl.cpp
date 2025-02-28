#include "CSJVideoRendererDXImpl.h"

#include <d3dcompiler.h>

#include <vector>

#include "WICTextureLoader11.h"
#include "DXTrace.h"

const D3D11_INPUT_ELEMENT_DESC CSJVideoRendererDXImpl::VertexPosColor::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

    std::wstring vertshaderFile(L"resources\\DXShaders\\DXVertexShader.hlsl");
    std::wstring vertCso(L"resources\\DXShaders\\DXVertexShader.cso");
    std::wstring pixelShaderFile(L"resources\\DXShaders\\DXRGBAShader.hlsl");
    std::wstring pixelCso(L"resources\\DXShaders\\DXRGBAShader.cso");
    if (!initShaders(vertshaderFile, vertCso, pixelShaderFile, pixelCso)) {
        return false;
    }

    if (!initRenderData()) {
        return false;
    }

    m_initSuccess = true;
    return true;
}

void CSJVideoRendererDXImpl::updateSence(double timeStamp) {

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

    if (!m_bIsBindingResource) {
        showDefaultIamge();
        UINT stride = sizeof(VertexPosColor);
        UINT offset = 0;

        curContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
        curContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

        curContext->IASetInputLayout(m_pVertexLayout.Get());
        curContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        curContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        curContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        curContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

        bindRGBATextureResources();

        curContext->DrawIndexed(6, 0, 0);
        m_bIsBindingResource = true;
    } else {
        curContext->DrawIndexed(6, 0, 0);
    }

    HR(curSwapChain->Present(0, 0));
}

void CSJVideoRendererDXImpl::resize(int width, int height) {
    m_ClientWidth = width;
    m_ClientHeight = height;

    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    ComPtr<ID3D11Device> curDevice         = getCurrentDevice();
    ComPtr<IDXGISwapChain> curSwapChian    = getCurrentSwapChain();

    assert(curContext);
    assert(curDevice);
    assert(curSwapChian);

    /* Release relative resource the renderer pipeline using. */
    m_pRenderTargetView.Reset();
    m_pDepthStencilView.Reset();
    m_pDepthStencilBuffer.Reset();

    /* Reset swap chain and recreate renderering target view. */
    ComPtr<ID3D11Texture2D> backBuffer;
    HR(curSwapChian->ResizeBuffers(1, m_ClientWidth, m_ClientHeight,
                                   DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    HR(curSwapChian->GetBuffer(0, __uuidof(ID3D11Texture2D),
                               reinterpret_cast<void**>(backBuffer.GetAddressOf())));

    HR(curDevice->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                         m_pRenderTargetView.GetAddressOf()));

    /* Setting debug object name. */
    //D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");
    backBuffer.Reset();

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
                                  m_pDepthStencilBuffer.GetAddressOf()));

    HR(curDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr,
                                         m_pDepthStencilView.GetAddressOf()));

    /* Combine renderer target view and depth/tamplate buffer into pipeline. */
    curContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(),
                                   m_pDepthStencilView.Get());

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

void CSJVideoRendererDXImpl::loadVideoComponents(CSJVideoFormatType fmtType, 
                                                 int width, int height) {
}

void CSJVideoRendererDXImpl::updateVideoFrame(CSJVideoData *videoData) {
    if (videoData->getData() == nullptr || 
        videoData->getWidth() == 0 || 
        videoData->getHeight() == 0) {
        return ;
    }

    if (videoData->getWidth() != m_videoWidth || 
        videoData->getHeight() != m_videoHeight ||
        videoData->getFmtType() != m_pixelFmt) {
        
        m_videoWidth = videoData->getWidth();
        m_videoHeight = videoData->getHeight();
        m_pixelFmt = videoData->getFmtType();
        createTextureByFmtType(videoData->getFmtType());
    }

    switch (m_pixelFmt) {
    case CSJVIDEO_FMT_YUV420P:
        updateYUV420Frame(videoData);
        break;
    case CSJVIDEO_FMT_RGB24:
        updateRGBAFrame(videoData);
        break;
    default:
        // TODO: Not supported video fmt.
        break;
    }
}

void CSJVideoRendererDXImpl::showDefaultIamge() {

    m_pixelFmt = CSJVIDEO_FMT_RGB24;
    if (!m_pShaderResViewRGBA) {
        std::wstring imagePath = L"resources/Images/cross_street.jpeg";
        ComPtr<ID3D11Device> curDevice = getCurrentDevice();
        assert(curDevice);
        HRESULT hr = DirectX::CreateWICTextureFromFile(curDevice.Get(), 
                                                    imagePath.c_str(), 
                                                    m_imageTex.GetAddressOf(), 
                                                    m_pShaderResViewRGBA.GetAddressOf(), 
                                                    0);

        if (FAILED(hr)) {
            return ;
        }
    }
}

void CSJVideoRendererDXImpl::updateRGBAFrame(CSJVideoData * videoData) {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    D3D11_SUBRESOURCE_DATA sourceData{};
    sourceData.pSysMem = videoData->getRGB24();
    sourceData.SysMemPitch = m_videoWidth;
    sourceData.SysMemSlicePitch = 0;
    curContext->UpdateSubresource(m_singleTex.Get(),
                                  0, 
                                  nullptr, 
                                  sourceData.pSysMem, 
                                  sourceData.SysMemPitch,
                                  sourceData.SysMemSlicePitch);
}

bool CSJVideoRendererDXImpl::initShaders(std::wstring &vertShaderFile,
                                         std::wstring &vertCso,
                                         std::wstring &pixelShaderFile,
                                         std::wstring &pixelCso) {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return false;
    }

    ComPtr<ID3DBlob> blob;

    /* Create vertex shader */
    HR(CreateShaderFromFile(vertCso.c_str(),
                            vertShaderFile.c_str(),
                            "main",
                            "vs_5_0",
                            blob.ReleaseAndGetAddressOf()));

    HR(curDevice->CreateVertexShader(blob->GetBufferPointer(),
                                     blob->GetBufferSize(),
                                     nullptr,
                                     m_pVertexShader.GetAddressOf()));

    /* Creating and Binding layout */
    HR(curDevice->CreateInputLayout(VertexPosColor::inputLayout,
                                    ARRAYSIZE(VertexPosColor::inputLayout),
                                    blob->GetBufferPointer(),
                                    blob->GetBufferSize(),
                                    m_pVertexLayout.GetAddressOf()));

    /* Creating Pixel shader */
    HR(CreateShaderFromFile(pixelCso.c_str(),
                            pixelShaderFile.c_str(),
                            "main",
                            "ps_5_0",
                            blob.ReleaseAndGetAddressOf()));

    HR(curDevice->CreatePixelShader(blob->GetBufferPointer(),
                                    blob->GetBufferSize(),
                                    nullptr,
                                    m_pPixelShader.GetAddressOf()));

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
    indexBufferDes.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDes.ByteWidth = sizeof(indices);
    indexBufferDes.BindFlags = D3D11_BIND_INDEX_BUFFER;
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

void CSJVideoRendererDXImpl::createTextureByFmtType(CSJVideoFormatType fmtType) {
    switch (fmtType) {
    case CSJVIDEO_FMT_YUV420P:
        createTexturesForYUV420();    
        break;
    case CSJVIDEO_FMT_RGB24:
        createTextureForRGBA();
        break;
    default:
        // TODO: Not supported video fmt;
        break;
    }
}

void CSJVideoRendererDXImpl::createTextureForRGBA() {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return ;
    }

    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = m_videoWidth;
    texDesc.Height = m_videoHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    HRESULT hr = curDevice->CreateTexture2D(&texDesc, nullptr, &m_singleTex);
    if (FAILED(hr)) {
        // TODO: create single texture failed.
    }
}

void CSJVideoRendererDXImpl::createTexturesForYUV420() {
    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    if (!curDevice) {
        return ;
    }

    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = m_videoWidth;
    texDesc.Height = m_videoHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8_UINT;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if (m_Enable4xMsaa) {
        texDesc.SampleDesc.Count = 4;
        texDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    } else {
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
    }

    HRESULT hr = curDevice->CreateTexture2D(&texDesc, nullptr, &m_texY);
    if (FAILED(hr)) {
        // texY create failed.
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = curDevice->CreateShaderResourceView(m_texY.Get(), 
                                             &srvDesc, 
                                             m_pShaderResViewY.GetAddressOf());
    D3D11_TEXTURE2D_DESC uvTexDesc{};
    uvTexDesc.Width = m_videoWidth / 2;
    uvTexDesc.Height = m_videoHeight / 2;
    uvTexDesc.MipLevels = 1;
    uvTexDesc.ArraySize = 1;
    uvTexDesc.Format = DXGI_FORMAT_R8_UINT;
    uvTexDesc.Usage = D3D11_USAGE_DEFAULT;
    uvTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    uvTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    uvTexDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if (m_Enable4xMsaa) {
        uvTexDesc.SampleDesc.Count = 4;
        uvTexDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    } else {
        uvTexDesc.SampleDesc.Count = 1;
        uvTexDesc.SampleDesc.Quality = 0;
    }
    
    hr = curDevice->CreateTexture2D(&uvTexDesc, nullptr, &m_texU);
    if (FAILED(hr)) {

    }

    hr = curDevice->CreateTexture2D(&uvTexDesc, nullptr, &m_texV);
    if (FAILED(hr)) {

    }

    D3D11_SHADER_RESOURCE_VIEW_DESC uvSrvDesc{};
    uvSrvDesc.Format = texDesc.Format;
    uvSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    uvSrvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    uvSrvDesc.Texture2D.MostDetailedMip = 0;

    hr = curDevice->CreateShaderResourceView(m_texU.Get(), 
                                             &uvSrvDesc, 
                                             m_pShaderResViewU.GetAddressOf());

    hr = curDevice->CreateShaderResourceView(m_texV.Get(), 
                                             &uvSrvDesc,
                                             m_pShaderResViewV.GetAddressOf());
                                
}

void CSJVideoRendererDXImpl::createTextureSampler() {
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11Device> curDevice = getCurrentDevice();
    assert(curDevice);
    HRESULT hr = curDevice->CreateSamplerState(&samplerDesc, m_pSamplerState.GetAddressOf());
    if (FAILED(hr)) {
        // TODO: sampler create failed.
    }
}

void CSJVideoRendererDXImpl::updateYUV420Frame(CSJVideoData * videoData) {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    D3D11_SUBRESOURCE_DATA sourceData{};
    sourceData.pSysMem = videoData->getyuvY();
    sourceData.SysMemPitch = m_videoWidth;
    sourceData.SysMemSlicePitch = 0;
    curContext->UpdateSubresource(m_texY.Get(), 
                                  0,
                                  nullptr, 
                                  sourceData.pSysMem, 
                                  sourceData.SysMemPitch,
                                  sourceData.SysMemSlicePitch);

    sourceData.pSysMem = videoData->getyuvU();
    sourceData.SysMemPitch = m_videoWidth / 2;
    curContext->UpdateSubresource(m_texU.Get(), 
                                  0, 
                                  nullptr, 
                                  sourceData.pSysMem, 
                                  sourceData.SysMemPitch,
                                  sourceData.SysMemSlicePitch);

    sourceData.pSysMem = videoData->getyuvV();  
    curContext->UpdateSubresource(m_texV.Get(), 
                                  0, 
                                  nullptr, 
                                  sourceData.pSysMem, 
                                  sourceData.SysMemPitch,
                                  sourceData.SysMemSlicePitch);
}

void CSJVideoRendererDXImpl::bindYUV420TextureResources() {
    ComPtr<ID3D11DeviceContext> curContext = getCurrentContext();
    if (!curContext) {
        return ;
    }

    if (m_pShaderResViewY) {
        curContext->PSSetShaderResources(0, 1, &m_pShaderResViewY);
    }

    if (m_pShaderResViewU) {
        curContext->PSSetShaderResources(1, 1, &m_pShaderResViewU);
    }

    if (m_pShaderResViewV) {
        curContext->PSSetShaderResources(2, 1, &m_pShaderResViewV);
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
    return m_pSwapChain1 ? m_pSwapChain1 : m_pSwapChain1;
}
