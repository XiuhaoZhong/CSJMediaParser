#include "CSJDirectXHelper.h"

#include <d3dcompiler.h>

#include <iostream>

#include "WICTextureLoader11.h"
#include "DDSTextureLoader11.h"
#include "DXTrace.h"

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"
#include "CSJUtils/CSJStringUtils.h"

using namespace csjutils;

namespace csjrenderengine {

bool CSJDirectXHelper::createDepthStencilViewResources(ComPtr<ID3D11Device> &device, 
                                                       ComPtr<ID3D11Texture2D> &depthStencilBuffer,
                                                       ComPtr<ID3D11DepthStencilView> &depthStencilView,
                                                       int width, int height, 
                                                       bool enable4xMsaa, 
                                                       UINT msaaQuality,
                                                       DXGI_FORMAT format,
                                                       D3D11_USAGE usage,
                                                       UINT bindFlags,
                                                       UINT mipLevel,
                                                       UINT arraySize) {
    if (!device || width <= 0 || height <= 0) {
        return false;
    }

    depthStencilBuffer.Reset();
    depthStencilView.Reset();

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width     = width;
    depthStencilDesc.Height    = height;
    depthStencilDesc.MipLevels = mipLevel;
    depthStencilDesc.ArraySize = arraySize;

    /** DXGI_FORMAT_D24_UNORM_S8_UINT: 32bit z-buffer, 24 bits to depth and 8 bits
     *  for stencil.
     */
    depthStencilDesc.Format = format;

    /* Using 4x MSAA, need to set MASS params to swap chain. */
    if (enable4xMsaa) {
        depthStencilDesc.SampleDesc.Count   = 4;
        depthStencilDesc.SampleDesc.Quality = msaaQuality - 1;
    } else {
        depthStencilDesc.SampleDesc.Count   = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }

    depthStencilDesc.Usage          = usage;
    depthStencilDesc.BindFlags      = bindFlags;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags      = 0;

    /* create depth buffer and depth tamplate view. */
    HRESULT hr = device->CreateTexture2D(&depthStencilDesc, nullptr,
                                         depthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr,
                                        depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }
    
    return true;
}

bool CSJDirectXHelper::createRenderTargetView(ComPtr<ID3D11Device> &device, 
                                              ComPtr<IDXGISwapChain> &swapChain, 
                                              int width, int height, 
                                              ComPtr<ID3D11RenderTargetView>& targetView) {
    if (!device || !swapChain || width <= 0 || height <= 0) {
        return false;
    }

    /* Reset swap chain and recreate renderering target view. */
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain->ResizeBuffers(1, width, height,
                                          DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr)) {
        return false;
    }

    hr = swapChain->GetBuffer(0, 
                              __uuidof(ID3D11Texture2D),
                              reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                        targetView.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    backBuffer.Reset();

    return true;
}

HRESULT CSJDirectXHelper::CreateShaderFromFile(const WCHAR *csoFileNameOut, 
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
                const char *errMsg = (const char *)errorBlob->GetBufferPointer();
                if (errMsg) {
                    LOG_Info("Compile shader failed: %s", errMsg);
                }
            }

            return hr;
        }

        if (csoFileNameOut) {
            return D3DWriteBlobToFile(*ppBlocbOut, csoFileNameOut, FALSE);
        }
    }

    return hr;
}

bool CSJDirectXHelper::createVertexShader(ComPtr<ID3D11Device> &device,
                                          std::string &vertShaderFile, 
                                          std::string &vertCso, 
                                          LPCSTR entryPoint, 
                                          LPCSTR shaderModel, 
                                          const D3D11_INPUT_ELEMENT_DESC *layoutDesc,
                                          int layoutDescNum,
                                          ComPtr<ID3D11VertexShader> &shader,
                                          ComPtr<ID3D11InputLayout> &layout) {
    if (!device) {
        return false;
    }

    ComPtr<ID3DBlob> blob;

    std::wstring vertShaderaPath = csjutils::CSJStringUtil::string2wstring(vertShaderFile);
    std::wstring vertCsoPath = csjutils::CSJStringUtil::string2wstring(vertCso);

    /* Create vertex shader */
    HRESULT hr = CSJDirectXHelper::CreateShaderFromFile(vertCsoPath.c_str(),
                                                        vertShaderaPath.c_str(),
                                                        entryPoint,
                                                        shaderModel,
                                                        blob.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        // TODO: create shaders failed.
        return false;
    }

    hr = device->CreateVertexShader(blob->GetBufferPointer(),
                                    blob->GetBufferSize(),
                                    nullptr,
                                    shader.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    hr = device->CreateInputLayout(layoutDesc,
                                   layoutDescNum,
                                   blob->GetBufferPointer(),
                                   blob->GetBufferSize(),
                                   layout.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }
    
    return true;
}

bool CSJDirectXHelper::createPixelShader(ComPtr<ID3D11Device> &device, 
                                         std::string &pixelShaderFile, 
                                         std::string &pixelCso, 
                                         LPCSTR entryPoint, 
                                         LPCSTR shaderModel, 
                                         ComPtr<ID3D11PixelShader> &shader) {
    if (!device) {
        return false;
    }

    ComPtr<ID3DBlob> blob;

    std::wstring pixelShaderPath = csjutils::CSJStringUtil::string2wstring(pixelShaderFile);
    std::wstring pixelCsoPath = csjutils::CSJStringUtil::string2wstring(pixelCso);

    /* Creating Pixel shader */
    HRESULT hr = CSJDirectXHelper::CreateShaderFromFile(pixelCsoPath.c_str(),
                                                        pixelShaderPath.c_str(),
                                                        entryPoint,
                                                        shaderModel,
                                                        blob.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        // TODO: create shaders failed. 
        return false;
    }

    hr = device->CreatePixelShader(blob->GetBufferPointer(),
                                    blob->GetBufferSize(),
                                    nullptr,
                                    shader.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

bool CSJDirectXHelper::createD3DTexture(ComPtr<ID3D11Device> &device,
                                        bool enable4xMsaa, 
                                        UINT msaaQuality,      
                                        int width, int height, 
                                        DXGI_FORMAT format, 
                                        UINT miplevels, 
                                        UINT arraySize, 
                                        D3D11_USAGE usage, 
                                        UINT bindFlags, 
                                        UINT CPUAccessFlags, 
                                        UINT MiscFlags, ComPtr<ID3D11Texture2D> &tex) {
    if (!device || width <= 0 || height <= 0) {
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
    if (enable4xMsaa) {
        texDesc.SampleDesc.Count   = 4;
        texDesc.SampleDesc.Quality = msaaQuality - 1;
    } else {
        texDesc.SampleDesc.Count   = 1;
        texDesc.SampleDesc.Quality = 0;
    }
    
    HRESULT hr = device->CreateTexture2D(&texDesc, 
                                         nullptr, 
                                         tex.ReleaseAndGetAddressOf());

    if (FAILED(hr)) {
        // texY create failed.
        return false;
    }
    
    return true;
}

bool CSJDirectXHelper::createD3DTextureWithResourceView(ComPtr<ID3D11Device> &device, 
                                                        ComPtr<ID3D11Texture2D> &tex, 
                                                        ComPtr<ID3D11ShaderResourceView> &resourceView, 
                                                        bool enable4xMsaa, 
                                                        UINT msaaQuality,
                                                        int width, int height, 
                                                        DXGI_FORMAT format, 
                                                        UINT miplevels, 
                                                        UINT arraySize, 
                                                        D3D11_USAGE usage, 
                                                        UINT bindFlags, 
                                                        UINT CPUAccessFlags, 
                                                        UINT MiscFlags, 
                                                        D3D11_SRV_DIMENSION srvDemension) {
    if (!device || width <= 0 || height <= 0) {
        return false;
    }

    bool res = createD3DTexture(device, 
                                enable4xMsaa, 
                                msaaQuality, 
                                width, height, 
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
    srvDesc.ViewDimension             = enable4xMsaa ? D3D_SRV_DIMENSION_TEXTURE2DMS : srvDemension;
    srvDesc.Texture2D.MipLevels       = miplevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = device->CreateShaderResourceView(tex.Get(),
                                                  &srvDesc, 
                                                  resourceView.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

bool CSJDirectXHelper::createRGBATextureFromImageFile(ComPtr<ID3D11Device> &device, 
                                                      std::wstring &imageFile, 
                                                      ComPtr<ID3D11Texture2D> &tex, 
                                                      ComPtr<ID3D11ShaderResourceView> &resourceView, 
                                                      int &width, int &height) {
    if (!device || imageFile.empty()) {
        return false;
    }

    if (!CSJPathTool::fileExists(imageFile)) {
        LOG_Error("image file not exists: %s", imageFile.c_str());
        return false;
    }

    HRESULT hr = DirectX::CreateWICTextureFromFile(device.Get(), 
                                                   imageFile.c_str(), 
                                                   (ID3D11Resource **)tex.ReleaseAndGetAddressOf(), 
                                                   resourceView.ReleaseAndGetAddressOf(), 
                                                   0);

    if (FAILED(hr)) {
        return false;
    }

    D3D11_TEXTURE2D_DESC texDesc;
    tex->GetDesc(&texDesc);
    width = texDesc.Width;
    height = texDesc.Height;
    std::wcout << L"[Log] load image: " << imageFile 
                << L", with: " << texDesc.Width 
                << L", height: " << texDesc.Height << std::endl;
    return true;
}

bool CSJDirectXHelper::createRGBATextureFromBuffer(ComPtr<ID3D11Device> &device, 
                                                   uint8_t *buffer, 
                                                   int width, int height, int pitch, 
                                                   ComPtr<ID3D11Texture2D> &tex, 
                                                   ComPtr<ID3D11ShaderResourceView> &resourceView) {
    if (!device || !buffer || width <= 0 || height <= 0 || pitch <= 0) {
        return false;
    }

    HRESULT hr = DirectX::CreateDDSTextureFromMemory(device.Get(), 
                                                     buffer, 
                                                     width * height * pitch,
                                                     (ID3D11Resource **)tex.ReleaseAndGetAddressOf(), 
                                                     resourceView.ReleaseAndGetAddressOf(), 
                                                     0);
    return !FAILED(hr);
}

bool CSJDirectXHelper::createConstBuffer(ComPtr<ID3D11Device> &device, 
                                         ComPtr<ID3D11Buffer> &constBuffer,
                                         UINT bufferSize, 
                                         D3D11_USAGE usage, 
                                         UINT bindFlags, 
                                         UINT CPUAccessFlags, 
                                         UINT MiscFlags, 
                                         UINT structByteStride) {
    if (!device) {
        return false;
    }

    D3D11_BUFFER_DESC cbDesc{};
    cbDesc.ByteWidth = bufferSize;
    cbDesc.Usage = usage;
    cbDesc.BindFlags = bindFlags;
    cbDesc.CPUAccessFlags = CPUAccessFlags;
    cbDesc.MiscFlags = MiscFlags;
    cbDesc.StructureByteStride = structByteStride;

    HRESULT hr = device->CreateBuffer(&cbDesc, NULL, constBuffer.ReleaseAndGetAddressOf());
    return SUCCEEDED(hr);
}
}