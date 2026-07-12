#pragma once

#include "CSJRenderableBase.h"

#include <atomic>
#include <vector>
#include <array>
#include <optional>

namespace csjrenderengine {

using YUVTexArray = std::array<ComPtr<ID3D11Texture2D>, 3>;
using YUVTexRVArray = std::array<ComPtr<ID3D11ShaderResourceView>, 3>; 

using YUVSizeArray = std::vector<std::pair<int, int>>;
using SpYuvSizeArrayPtr = std::shared_ptr<YUVSizeArray>;

class CSJYUVRenderable : public CSJRenderableBase {
public:

    /**
     * If the format has three planes use texArr[0] ~ texArr[2], such as I420
     * If the format has two planes, use texArr[0] ~ texArr[1], such as NV12, NV21
     */
    struct YUVRenderContext {
        CSJVideoFramePtr  videoFrame = nullptr;
        YUVTexArray       texArr;
        YUVTexRVArray     texRVArr;
        SpYuvSizeArrayPtr sizeArr    = nullptr;
        int               planes;
        bool              texValid   = false;

        void udpateContextWithFrame(CSJVideoFramePtr &frame);
    };
    using SpYUVRenderCtxPtr = std::shared_ptr<YUVRenderContext>;

    CSJYUVRenderable();
    ~CSJYUVRenderable();

    bool initRenderable(DirectXParams &params) override;
    bool fillRenderrableData() override;
    bool bindRenderableComponents() override;
    void setFreshImage(const std::string &imageFile) override;
    void setFreshVideoData(CSJVideoFramePtr &frame) override;
    bool updateRenderable(double timeStamp) override;
    void drawRenderable(double timeStamp) override;

protected:
    /**
     * Get the video data planes in a video frame.
     * 
     * @return 3 for I420 or relatives, 2 for NV12, NV21, etc.
     */
    int getFramePlanes(CSJVideoFramePtr videoFrame);

    /**
     * Get the video planes and size array for every plane.
     * 
     * @param[in]  frame      input video frame.
     * @param[out] sizeArray  save size for every plane, and if don't need to get size array,
     *                        use nullptr.
     * @return if the planes are match one of a yuv format planes, return true, otherwise return false;
     */
    bool getFramePlanesAndSizeArr(CSJVideoFramePtr &frame, int &planes, SpYuvSizeArrayPtr sizeArrayPtr);

    bool needRecreateTexture(SpYUVRenderCtxPtr &renderCtx, CSJVideoFramePtr &freshFrame);

    struct YUVRenderInputData {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 texCoord;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

    /**
     * Currently, this struct deliveries the isI420 flag into GPU, the second member is used to
     * align memory with 16 bytes.
     */
    struct VideoParam {
        int isI420;
        float params[3];
    };

    bool createShader(ComPtr<ID3D11Device> &device);
    bool createBasicData(ComPtr<ID3D11Device> &device);
    bool createBuffers(ComPtr<ID3D11Device> &device);
    bool createTexResourceForRenderContext(SpYUVRenderCtxPtr &ctx);

private:

    std::array<SpYUVRenderCtxPtr, 2> m_renderCtxArr;

    ComPtr<ID3D11InputLayout>   m_pVertexLayout;   // input vertex layout;
    ComPtr<ID3D11VertexShader>  m_pVertexShader;
    ComPtr<ID3D11PixelShader>   m_pPixelShader;
    ComPtr<ID3D11Buffer>        m_pVertexBuffer;
    ComPtr<ID3D11Buffer>        m_pIndexBuffer;
    ComPtr<ID3D11SamplerState>  m_pSamplerState; // sampler state
    ComPtr<ID3D11Buffer>        m_pVideoParamBuffer;
    int                         m_iCurTexIndex = 0;

    CSJPixelFormat              m_curPixelFormat;
    CSJVideoFramePtr            m_pVideoFrame;
    std::atomic<int>            m_iUpdateSlot;
    std::atomic<int>            m_iRenderSlot;    

};

} // namespace csjrenderengine