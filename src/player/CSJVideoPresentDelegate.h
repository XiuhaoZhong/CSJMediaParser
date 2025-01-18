#ifndef __CSJVIDEOPRESENTDELEGATE_H__
#define __CSJVIDEOPRESENTDELEGATE_H__

#include "CSJRawData/CSJMediaRawData.h"

/**
 * @brief The CSJVideoPresentDelegate defines interfaces that the player kernel
 *        inintializes the video frame which will be presented in the video widget,
 *        and updates the frames at the appropriate time.
 *        These interfaces should be implemented by the class which is responsible
 *        for video frames rendering, and set into player kernel object as a delegate.
 */
class CSJVideoPresentDelegate {
public:
    CSJVideoPresentDelegate() = default;
    ~CSJVideoPresentDelegate() = default;

    /**
     * @brief Initialize the video frame information, including pixel format,
     *        and size of the video frame.
     * @param fmtType   pixel format of the video frame.
     * @param width     width of the video frame.
     * @param height    height of the video frame.
     */
    virtual void initializeVideoInfo(CSJVideoFormatType fmtType,
                                     int width, int height) = 0;

    /**
     * @brief Update the video frame.
     * @param videoData     The frame information will be present.
     */
    virtual void updateVideoFrame(CSJVideoData *videoData) = 0;
};

#endif // __CSJVIDEOPRESENTDELEGATE_H__
