#pragma once

#include <string>
#include <thread>

#include "CSJMediaPlayerBase.h"
#include "CSJUtils/CSJRingQueue.h"

using csjutils::CSJRingQueue;

struct AVFormatContext;
struct AVCodecContext;

namespace csjmediaengine {
class CSJPacketWrapper;
class CSJFrameWrapper;

class CSJMediaPlayer : public CSJMediaPlayerBase {
public:
    CSJMediaPlayer();
    ~CSJMediaPlayer();

    void setPlayFile(std::string &file) override;
    bool initPlayer() override;

    /**
     * @brief   Get the duration of the media file.
     * @return  the duration.
     */
    int getDuration() override;

    /**********************************************
     * Player operations.
     *********************************************/
    void play() override;
    void pause() override;
    void resume() override;
    void stop() override;
    void seek(double timeStamp) override;

    /**********************************************
     * Player status.
     *********************************************/
    bool isPlaying() override;
    bool isPause() override;
    bool isStop() override;

    void setVideoPresentDelegate(std::shared_ptr<CSJVideoPresentDelegate> delegate) override;

protected:
    /**
     * Open the media file, and get the decoders to be ready, if there are any prblems,
     * this function will return false;
     */
    bool readyForPlay();

    void readPacketsFunc();
    void videoDecodeFunc();
    void audioDecodeFunc();

private:
    bool                     m_bPlayerInit = false;

    std::string              m_file;
    CSJPlayMode              m_playMode = CSJPlayMode::CSJPlayMode_NONE;
    CSJPlayerStatus          m_status;
    std::mutex               m_pauseMtx;
    cond_va                  m_pauseCond;

    bool                     m_bReadFinish = false;
    bool                     m_bAudioDecodeFinish = false;
    bool                     m_bVideoDecodeFinish = false;

    /* Stream and decoder */
    int                      m_iAudioStreamIndex = -1;
    int                      m_iVideoStreamIndex = -1;
    AVFormatContext         *m_pFormatCtx;
    AVCodecContext          *m_pAudioCodecCtx;
    AVCodecContext          *m_pVideoCodecCtx;

    /* Working threads. */
    std::unique_ptr<std::thread> m_readThread        = nullptr;
    std::unique_ptr<std::thread> m_videoDecodeThread = nullptr;
    std::unique_ptr<std::thread> m_audioDecodeThread = nullptr;

    /* Media data. */
    CSJRingQueue<CSJPacketWrapper *>       m_pAudioPacketsQueue;
    CSJRingQueue<CSJPacketWrapper *>       m_pVideoPacketsQueue;
    CSJRingQueue<CSJFrameWrapper *>        m_pAudioFramesQueue;
    CSJRingQueue<CSJFrameWrapper *>        m_pVideoFramesQueue;
    std::weak_ptr<CSJVideoPresentDelegate> m_pVideoPresentDelegate;

    // TODO:
    /*
     *  FileReader: 
     *      reading content from file, and demux video packets and audio packets into ring buffers.
     *
     *  Decoder:
     *      video decoder: reading packets from video packet ring buffer and decode video packet, then 
     *                     put the decoded frame into video frame ring buffer.
     *      audio decoder: reading packets from video packet ring buffer and decode video packet, then 
     *                     put the decoded frame into video frame ring buffer.
     * 
     * Audio/Video syncer: sync audio and video frames
     * VideoRenderDelegate: Get video frame from AVSyncer, and rendering video frame.
     * AudioRenderDelegate: Get audio frame from AVSyncer, and rendering audio frame.
     */

};

} // namespace csjmediaengine;