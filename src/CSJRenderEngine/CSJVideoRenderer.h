#ifndef __CSJVIDEORENDERER_H__
#define __CSJVIDEORENDERER_H__

#include <memory>
#include <string>
#include <array>

#include "CSJRenderEngine_Export.h"

#include "CSJUtils/CSJMediaData.h"

#include "CSJMediaEngine/CSJMediaRawData.h"

using csjutils::CSJVideoFramePtr;
using csjutils::CSJPixelFormat;

using csjmediaengine::CSJVideoFormatType;
using csjmediaengine::CSJVideoData;

namespace csjrenderengine {

typedef void* CSJWindowID;

/**
 * The render type of the Renderer.
 */
typedef enum {
    CSJRenderType_ONSCREEN = 0,
    CSJRenderType_OFFSCREEN
} CSJRenderType;

enum CSJRenderContentType : uint8_t {
    CSJRenderContentType_None = 0,
    CSJRenderContentType_RGB,
    CSJRenderContentType_RGBA,
    CSJRenderContentType_ImageFile,
    CSJRenderContentType_I420,
    CSJRenderContentType_NV12
};

/**
 * This is the delegate interface for CSJRenderEngine, and the users can implement
 * the interfaces to provide video data, monitor rendering status, and some customized 
 * notifications in rendering nodes.
 */
class CSJRenderDelegate {
public:
    CSJRenderDelegate() = default;
    virtual ~CSJRenderDelegate() {}

    virtual CSJVideoFramePtr getNextVideoFrame() = 0;

    virtual void beforeARenderingTick() {};
    virtual void afterARenderingTick() {};

    virtual void beforeRenderingStart() {};
    virtual void afterRenderingStart() {};

    virtual void beforeRenderingPause() {};
    virtual void afterRenderingPause() {};

    virtual void beforeRenderingResume() {};
    virtual void afterRenderingResume() {};

    virtual void beforeRenderingStop() {};
    virtual void afterRenderingStop() {};
};

using CSJRenderDelegatePtr = std::shared_ptr<CSJRenderDelegate>;
using CSJRenderDelegateWeakPtr = std::weak_ptr<CSJRenderDelegate>;

/**
 * This is the base class of video renderer, subclasses which implements
 * the interfaces of this will encasuplate the native renderer, subclasses
 * on windows implement with DirectX API, also, the subclasses on macOS
 * implement these interfaces with Metal API.
 * 
 * The pixel formats are supported including rgb, rgba and all kinds of yuv.
 * 
 * For rgb and rgba, users can invoke the setImagePath() function to set the 
 * path of picture, users can aslo invoke updateVideoFrame() function to set 
 * the image buffer from a memory, and users should create a CSJVideoData 
 * object with pixel formats, width and height.
 * 
 * For yuv series formats, users should invoke updateVideoFrame() function to 
 * set the yuv buffers with a  CSJVideoData instance, and must include pixel
 * format, width and height.
 */
class CSJRENDERENGINE_API CSJVideoRenderer {
public:
    CSJVideoRenderer() {};
    virtual ~CSJVideoRenderer() {};

    /**
     * @brief Start rendering, this function will start the render cycle which is according 
     *        to the vsync signal from system.
     */
    virtual void startRender() = 0;

    /**
     * @brief Stop rendering, this function will stop the render cycle.
     */
    virtual void stopRender() = 0;

    /**
     * @brief Set the render type
     * 
     * @param renderType the render type, default is CSJRenderContentType_None
     */
    virtual void setRenderType(CSJRenderContentType renderType);

    /**
     * @brief Fill TextData, output the offscreen texture data to caller.
     *        the @param width and @param height are used to check the size
     *        of the current render area to avoid the error as the render 
     *        area's size changing.
     * 
     * @param width     width of render area.
     * @param height    height of render area.
     * 
     */
    virtual bool fillTextureData(uint8_t *buf, int width, int height);

    /**
     * @brief Update drawable size for CAMetalLayer, this function is for 
     *        macOS.
     * 
     * @param width      new width of the widget.      
     * @param height     new height of the widget.
     * @param pixelRatio the pixel ratio on the screen.
     */
    virtual void resize(int width, int height, float pixelRatio) = 0;

    /**
     * @brief Load the video frame components for specific video pixel format,
     *        Genenally, different format needs its own components, such as, YUV
     *        pixel formats usually need three textures.
     * @param fmtType the pixel format defined in CSJMediaRawData.h
     * @param width   the width of the video frame
     * @param height  the height of the video frame
     */
    virtual void initialRenderComponents(CSJPixelFormat fmtType,
                                         int width, int height) = 0;

    /**
     * @brief Update the video frame, in case the video frame's size change, every
     *        time invoke this function, the renderer should compare the new size
     *        with the old size, and load new components(mainly textures).
     * @param videoData the video data will be presented.
     */
    virtual void updateVideoFrame(CSJVideoFramePtr videoData) = 0;

    /**
     * @brief Set the picture that will be rendered in this widget.
     * 
     */
    virtual void setImage(const std::string& imagePath) = 0;

    virtual void setRenderDelegate(CSJRenderDelegatePtr delegate);

    std::array<float, 2> computeVideoArea(int widgetW, int widgetH,
                                          int videoW, int videoH);

protected:
    CSJRenderDelegateWeakPtr m_pDelegate;
    CSJRenderContentType     m_renderContentType = CSJRenderContentType_None;
};

using CSJSpVideoRenderer = std::shared_ptr<CSJVideoRenderer>;

struct CSJMediaRendererDeleter {
    void operator()(CSJVideoRenderer *p) const {
        if (p) {
            delete p;
        }
    }
};

CSJRENDERENGINE_API CSJVideoRenderer* createCSJRenderer(CSJWindowID widgetID, 
                                                        int width, int height,
                                                        float pixelRatio);

using CSJVideoRendererPtr = std::unique_ptr<CSJVideoRenderer>;

} // namespace csjrenderengine

#endif // __CSJVIDEORENDERER_H__
