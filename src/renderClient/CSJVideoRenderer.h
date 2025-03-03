#ifndef __CSJVIDEORENDERER_H__
#define __CSJVIDEORENDERER_H__

#include <QWidget>
#include <memory>

#include <array>

#include "MpegHeaders/CSJMediaRawData.h"

/**
 * This is the base class of video renderer, subclasses which implements
 * the interfaces of this will encasuplate the native renderer, subclasses
 * on windows implement with DirectX API, also, the subclasses on macOS
 * implement these interfaces with Metal API.
 */
class CSJVideoRenderer {
public:
    CSJVideoRenderer() {};
    virtual ~CSJVideoRenderer() {};

    static std::shared_ptr<CSJVideoRenderer> getRendererInstance();

    /**
     * @brief Initializes renderer.
     *
     * @param widgetID  the widget ID, for windows, it's the handle of a window.
     * @param width     width of widget.
     * @param height    height of widget.
     *
     * @return return true when success, of return false.
     */
    virtual bool init(WId widgetID, int width, int height) = 0;

    /**
     * @brief Update the contents before draw it.
     *
     * @param timeStamp the time stamp of drawing.
     */
    virtual void updateSence(double timeStamp) = 0;

    /**
     * @brief Draw the widget's content.
     */
    virtual void drawSence() = 0;

    /**
     * @brief Resize renderer context when the size of widget changes.
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
    virtual void loadVideoComponents(CSJVideoFormatType fmtType,
                                     int width, int height) = 0;

    /**
     * @brief Update the video frame, in case the video frame's size change, every
     *        time invoke this function, the renderer should compare the new size
     *        with the old size, and load new components(mainly textures).
     * @param videoData the video data will be presented.
     */
    virtual void updateVideoFrame(CSJVideoData *videoData) = 0;

    /**
     * @brief This function show a default image to test the renderer functionalities.
     */
    virtual void showDefaultIamge() {};

    std::array<float, 2> computeVideoArea(int widgetW, int widgetH,
                                          int videoW, int videoH);
};

using CSJSpVideoRenderer = std::shared_ptr<CSJVideoRenderer>;

#endif // __CSJVIDEORENDERER_H__
