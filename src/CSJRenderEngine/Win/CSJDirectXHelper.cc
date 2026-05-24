#include "CSJDirectXHelper.h"

#include "DXTrace.h"

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
    depthStencilDesc.Format = format;//DXGI_FORMAT_D24_UNORM_S8_UINT;

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

bool CSJDirectXHelper::createRenderTargetView(int width, int height, ComPtr<ID3D11RenderTargetView> &targetView, bool ONScreen)
{
    return false;
}

HRESULT CSJDirectXHelper::CreateShaderFromFile(const WCHAR *csoFileNameOut, const WCHAR *hisFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob **ppBlocbOut)
{
    return E_NOTIMPL;
}

// bool CSJDirectXHelper::createTextureByFmtType(CSJVideoFormatType fmtType, int width, int height, ComPtr<ID3D11Texture2D> &tex)
// {
//     return false;
// }

bool CSJDirectXHelper::createD3DTexture(int width, int height, DXGI_FORMAT format, UINT miplevels, UINT arraySize, D3D11_USAGE usage, UINT bindFlags, UINT CPUAccessFlags, UINT MiscFlags, ComPtr<ID3D11Texture2D> &tex)
{
    return false;
}

bool CSJDirectXHelper::createD3DTextureWithResourceView(int width, int height, DXGI_FORMAT format, UINT miplevels, UINT arraySize, D3D11_USAGE usage, UINT bindFlags, UINT CPUAccessFlags, UINT MiscFlags, ComPtr<ID3D11Texture2D> &tex, ComPtr<ID3D11ShaderResourceView> &resourceView, D3D11_SRV_DIMENSION srvDemension)
{
    return false;
}

}
