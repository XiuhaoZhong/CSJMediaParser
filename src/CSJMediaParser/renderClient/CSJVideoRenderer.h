#ifndef __CSJVIDEORENDERER_H__
#define __CSJVIDEORENDERER_H__

#include <QWidget>
#include <memory>

#include <array>

#include "CSJMediaEngine/CSJMediaRawData.h"

using csjmediaengine::CSJVideoFormatType;
using csjmediaengine::CSJVideoData;

/**
 * The render type of the Renderer.
 */
typedef enum {
    CSJRenderType_ONSCREEN = 0,
    CSJRenderType_OFFSCREEN
} CSJRenderType;

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
class CSJVideoRenderer {
public:
    CSJVideoRenderer() {};
    virtual ~CSJVideoRenderer() {};

    static std::shared_ptr<CSJVideoRenderer> getRendererInstance();

    static std::shared_ptr<CSJVideoRenderer> getOffScreenRendererInstance();

    /**
     * @brief Initializes renderer. This function is for onscreen rendering, need 
     *        the @param widgetID, which indecates the hwnd of the native window, 
     *        including windows and MacOS.
     *
     * @param widgetID  the widget ID, for windows, it's the handle of a window.
     * @param width     width of widget.
     * @param height    height of widget.
     *
     * @return return true when success, of return false.
     */
    virtual bool init(WId widgetID, int width, int height) = 0;

    /**
     * @brief Initiliaze off screen render. This function is for offscreen rendering.
     * 
     * @param width     width of render area.
     * @param height    height of render area.
     * 
     */
    virtual bool initForOffScreen(int width, int height);

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
     * @brief Update the contents before draw it.
     *
     * @param timeStamp the time stamp of drawing.
     */
    virtual bool updateSence(double timeStamp) = 0;

    /**
     * @brief Draw the widget's content.
     */
    virtual void drawSence() = 0;

    /**
     * @brief Resize renderer context when the size of widget changes.
     * 
     * @param width  new width of the widget.
     * @param height new height of the widget.
     */
    virtual void resize(int width, int height) = 0;

    /**
     * @brief Load the video frame components for specific video pixel format,
     *        Genenally, different format needs its own components, such as, YUV
     *        pixel formats usually need three textures.
     * @param fmtType the pixel format defined in CSJMediaRawData.h
     * @param width   the width of the video frame
     * @param height  the height of the video frame
     */
    virtual void initialRenderComponents(CSJVideoFormatType fmtType,
                                         int width, int height) = 0;

    /**
     * @brief Update the video frame, in case the video frame's size change, every
     *        time invoke this function, the renderer should compare the new size
     *        with the old size, and load new components(mainly textures).
     * @param videoData the video data will be presented.
     */
    virtual void updateVideoFrame(CSJVideoData *videoData) = 0;

    /**
     * @brief Set the picture that will be rendered in this widget.
     * 
     */
    virtual void setImage(const QString& imagePath) = 0;

    std::array<float, 2> computeVideoArea(int widgetW, int widgetH,
                                          int videoW, int videoH);
};

using CSJSpVideoRenderer = std::shared_ptr<CSJVideoRenderer>;

#endif // __CSJVIDEORENDERER_H__
