#include "CSJMediaPlayer.hpp"

#include "CSJMpegHeader.h"
#include "CSJPacketWrapper.hpp"
#include "CSJFrameWrapper.hpp"

#include "CSJUtils/CSJLogger.h"

static size_t g_video_queue_len = 20;
static size_t g_audio_queue_len = 20; 

namespace csjmediaengine {

CSJMediaPlayer::CSJMediaPlayer()
    : m_pAudioPacketsQueue(g_audio_queue_len)
    , m_pVideoPacketsQueue(g_video_queue_len)
    , m_pAudioFramesQueue(g_audio_queue_len)
    , m_pVideoFramesQueue(g_video_queue_len) {
    
}

CSJMediaPlayer::~CSJMediaPlayer() {
    clearMediaPackets();
    clearMediaFrames();
}

void CSJMediaPlayer::setPlayFile(std::string &file) {
    m_file = file;
}

bool CSJMediaPlayer::initPlayer() {
    return true;
}

int CSJMediaPlayer::getDuration() {
    return 0;
}

void CSJMediaPlayer::play() {
    if (m_file.size() == 0) {
        return ;
    }

    bool isReady = readyForPlay();
    if (!isReady) {
        // TODO: There are some issues and current player can't play.
        LOG_Error("Current media file can't be ready to play!");
        return ;
    }

    LOG_Info("Current media file is ready to play!");

    // Start a thread to read packets from media file.
    m_readThread.reset(new std::thread(&CSJMediaPlayer::readPacketsFunc, this));

    // Starting decoder thread according to the play mode.
    // if (containVideoMode(m_playMode)) {
    //     m_videoDecodeThread.reset(new std::thread(&CSJMediaPlayer::videoDecodeFunc, this));
    // }

    // if (containAudioMode(m_playMode)) {
    //     m_audioDecodeThread.reset(new std::thread(&CSJMediaPlayer::audioDecodeFunc, this));
    // }
}

void CSJMediaPlayer::pause() {
    m_status = CSJPLAYERSTATUS_PAUSE;
}

void CSJMediaPlayer::resume() {
    m_status = CSJPLAYERSTATUS_PLAYING;
    m_pauseCond.notify_all();
}

void CSJMediaPlayer::stop() {
    m_status = CSJPLAYERSTATUS_STOP;
    /* In case current status is pause. */
    m_pauseCond.notify_all();

    clear();
}

void CSJMediaPlayer::seek(double timeStamp) {

}

void CSJMediaPlayer::clear() {
    // TODO: the stop logic: must join all threads, then clear queues.
    /**
     * 1. Stop render thread, including video and audio
     * 2. Stop the reading thread
     * 3. Stop decoder thread
     * 4. Clear video and audio packets queue.
     * 5. Clear video and audio frames queue.
     */

    m_pVideoPacketsQueue.wakeUpToExit();
    // m_pAudioPacketsQueue.wakeUpToExit();

    if (m_readThread->joinable()) {
        m_readThread->join();
        m_readThread.reset();
    }

    clearMediaPackets();

    // TODO: join decoder threads if needed.
}

bool CSJMediaPlayer::isPlaying() {
    return m_status == CSJPLAYERSTATUS_PLAYING;
}

bool CSJMediaPlayer::isPause() {
    return m_status == CSJPLAYERSTATUS_PAUSE;
}

bool CSJMediaPlayer::isStop() {
    return m_status == CSJPLAYERSTATUS_STOP;
}

void CSJMediaPlayer::setVideoPresentDelegate(std::shared_ptr<CSJVideoPresentDelegate> delegate) {
    m_pVideoPresentDelegate = delegate;
}

bool CSJMediaPlayer::readyForPlay() {
    m_pFormatCtx = avformat_alloc_context();
    m_pFormatCtx->probesize = 50 * 1024;
    m_pFormatCtx->max_analyze_duration = 75000;

    int res = avformat_open_input(&m_pFormatCtx, m_file.c_str(), NULL, NULL);
    if (res != 0) {
        return false;
    }

    res = avformat_find_stream_info(m_pFormatCtx, NULL);
    if (res < 0) {
        return false;
    }

    for (int i = 0; i < m_pFormatCtx->nb_streams; i++) {
        if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_iVideoStreamIndex = i;
            m_playMode |= CSJPlayMode::CSJPlayMode_Video;
        }

        if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_iAudioStreamIndex= i;
            m_playMode |= CSJPlayMode::CSJPlayMode_Audio;
        }
    }

    if (containVideoMode(m_playMode)) {
        m_pVideoCodecCtx = avcodec_alloc_context3(NULL);
        if (avcodec_parameters_to_context(m_pVideoCodecCtx, m_pFormatCtx->streams[m_iVideoStreamIndex]->codecpar) < 0 ) {
            // TODO: failed to get video decoder.
        }

        if (!m_pVideoCodecCtx->codec) {
            m_pVideoCodecCtx->codec = avcodec_find_decoder(m_pVideoCodecCtx->codec_id);
        }
    }

    if (containAudioMode(m_playMode)) {
        m_pAudioCodecCtx = avcodec_alloc_context3(NULL);
        if (avcodec_parameters_to_context(m_pAudioCodecCtx, m_pFormatCtx->streams[m_iAudioStreamIndex]->codecpar) < 0) {
            // TODO: failed to get audio decoder.
        }

        if (!m_pAudioCodecCtx->codec) {
            m_pAudioCodecCtx->codec = avcodec_find_decoder(m_pAudioCodecCtx->codec_id);
        }
    }

    if (!m_pVideoCodecCtx->codec && !m_pAudioCodecCtx->codec) {
        // TODO: video decoder and audio decoder are all nullptr, there must be errors.
        return false;
    }

    return true;
}

void CSJMediaPlayer::readPacketsFunc() {
    AVPacket *pkt = av_packet_alloc();
    LOG_Info("Start reading packets from media file %s", m_file.c_str());
    while (1) {
        if (isStop()) {
            LOG_Info("Stop reading packets from media file %s", m_file.c_str());
            break;
        }

        if (isPause()) {
            LOG_Info("Pause reading packets from media file %s", m_file.c_str());
            std::unique_lock<std::mutex> lock(m_pauseMtx);
            m_pauseCond.wait(lock);
        }

        int ret = av_read_frame(m_pFormatCtx, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                // TODO: read to end of the file.
                break;
            }
        }

        if (pkt->stream_index == m_iVideoStreamIndex) {
            LOG_Info("Before put a video packet into ring buffer!");
            CSJPacketWrapperPtr pktWrapper = std::make_shared<CSJPacketWrapper>(pkt);
            m_pVideoPacketsQueue.enBuffer(pktWrapper);
            LOG_Info("Reading a video packet, put it into ring buffer.");
        } else if (pkt->stream_index == m_iAudioStreamIndex) {
            CSJPacketWrapperPtr pktWrapper = std::make_shared<CSJPacketWrapper>(pkt);
            m_pAudioPacketsQueue.enBuffer(pktWrapper);
            LOG_Info("Reading an audio packet, put it into ring buffer.");
        }

    }

    if (pkt) {
        av_packet_free(&pkt);
    }

    LOG_Info("Quit reading from media file %s", m_file.c_str());
}

void CSJMediaPlayer::videoDecodeFunc() {

}

void CSJMediaPlayer::audioDecodeFunc() {

}

void CSJMediaPlayer::clearMediaPackets() {
    m_pVideoPacketsQueue.reset();
    m_pAudioPacketsQueue.reset();
}

void CSJMediaPlayer::clearMediaFrames() {
   
}

void CSJMediaPlayer::clearPacketsQueue(CSJRingQueue<CSJPacketWrapperPtr> &queue) {
    
}

void CSJMediaPlayer::clearFrameQueue(CSJRingQueue<CSJFrameWrapperPtr> &queue) {
    while (!queue.is_empty()) {
        auto item = queue.deBuffer();
    }
}

} // namespace csjmediaengine;