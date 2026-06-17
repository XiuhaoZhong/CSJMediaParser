#include <windows.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <directxmath.h>
#include <string>

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

    static bool createRenderTargetView(ComPtr<ID3D11Device> &device, 
                                       ComPtr<IDXGISwapChain> &swapChain, 
                                       int width, int height, 
                                       ComPtr<ID3D11RenderTargetView>& targetView);

    static HRESULT CreateShaderFromFile(const WCHAR * csoFileNameOut,
                                        const WCHAR * hisFileName,
                                        LPCSTR entryPoint,
                                        LPCSTR shaderModel,
                                        ID3DBlob ** ppBlocbOut);

    static bool createVertexShader(ComPtr<ID3D11Device> &device,
                                   std::string &csoFileNameOut,
                                   std::string &hisFileName,
                                   LPCSTR entryPoint,
                                   LPCSTR shaderModel,
                                   const D3D11_INPUT_ELEMENT_DESC *layoutDesc,
                                   int layoutDescNum,
                                   ComPtr<ID3D11VertexShader> &shader,
                                   ComPtr<ID3D11InputLayout> &layout);

    static bool createPixelShader(ComPtr<ID3D11Device> &device,
                                  std::string &csoFileNameOut,
                                  std::string &hisFileName,
                                  LPCSTR entryPoint,
                                  LPCSTR shaderModel,
                                  ComPtr<ID3D11PixelShader> &shader);

    static bool createD3DTexture(ComPtr<ID3D11Device> &device,
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
                                 ComPtr<ID3D11Texture2D>& tex);

    static bool createD3DTextureWithResourceView(ComPtr<ID3D11Device> &device,
                                                 ComPtr<ID3D11Texture2D>& tex,
                                                 ComPtr<ID3D11ShaderResourceView>& resourceView,
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
                                                 D3D11_SRV_DIMENSION srvDemension = D3D11_SRV_DIMENSION_TEXTURE2D);

    static bool createRGBATextureFromImageFile(ComPtr<ID3D11Device> &device, 
                                               std::wstring &imageFile,
                                               ComPtr<ID3D11Texture2D> &tex, 
                                               ComPtr<ID3D11ShaderResourceView> &resourceView,
                                               int &width, int &height);

    static bool createRGBATextureFromBuffer(ComPtr<ID3D11Device> &device, 
                                            uint8_t *buffer, 
                                            int width, int height, int pitch,
                                            ComPtr<ID3D11Texture2D> &tex, 
                                            ComPtr<ID3D11ShaderResourceView> &resourceView);
};
} // namespace csjrenderengine
