#include <windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <directxmath.h>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace csjrenderengine {
class CSJDirectXHelper {
public:
    CSJDirectXHelper() = default;
    ~CSJDirectXHelper() = default;

    static bool createDepthStencilViewResources(ComPtr<ID3D11Device> &device, 
                                                ComPtr<ID3D11Texture2D> &depthStencilBuffer,
                                                ComPtr<ID3D11DepthStencilView> &depthStencilView,
                                                int width, int height, 
                                                bool enable4xMsaa, 
                                                UINT msaaQuality,
                                                DXGI_FORMAT format,
                                                D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
                                                UINT bindFlags = D3D11_BIND_DEPTH_STENCIL,
                                                UINT mipLevel = 1,
                                                UINT arraySize = 1);

    static bool createRenderTargetView(int width, int height, 
                                       ComPtr<ID3D11RenderTargetView> &targetView, 
                                       bool ONScreen = true);

    static HRESULT CreateShaderFromFile(const WCHAR * csoFileNameOut,
                                        const WCHAR * hisFileName,
                                        LPCSTR entryPoint,
                                        LPCSTR shaderModel,
                                        ID3DBlob ** ppBlocbOut);

    // static bool createTextureByFmtType(CSJVideoFormatType fmtType, int width, int height, 
    //                                    ComPtr<ID3D11Texture2D>& tex);

    static bool createD3DTexture(int width, int height, 
                          DXGI_FORMAT format, 
                          UINT miplevels, 
                          UINT arraySize,
                          D3D11_USAGE usage,
                          UINT bindFlags,
                          UINT CPUAccessFlags,
                          UINT MiscFlags,
                          ComPtr<ID3D11Texture2D>& tex);

    static bool createD3DTextureWithResourceView(int width, int height, 
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
};
} // namespace csjrenderengine
