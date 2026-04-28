#include "CSJMediaPlayer.hpp"

#include "CSJMpegHeader.h"
#include "CSJPacketWrapper.hpp"
#include "CSJFrameWrapper.hpp"

static size_t g_video_queue_len = 5;
static size_t g_audio_queue_len = 5; 

namespace csjmediaengine {

CSJMediaPlayer::CSJMediaPlayer()
    : m_pAudioPacketsQueue(g_audio_queue_len)
    , m_pVideoPacketsQueue(g_video_queue_len)
    , m_pAudioFramesQueue(g_audio_queue_len)
    , m_pVideoFramesQueue(g_video_queue_len) {
    
}

CSJMediaPlayer::~CSJMediaPlayer() {

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
        return ;
    }

    // Start a thread to read packets from media file.
    m_readThread.reset(new std::thread(&CSJMediaPlayer::readPacketsFunc, this));

    // Starting decoder thread according to the play mode.
    if (containVideoMode(m_playMode)) {
        m_videoDecodeThread.reset(new std::thread(&CSJMediaPlayer::videoDecodeFunc, this));
    }

    if (containAudioMode(m_playMode)) {
        m_audioDecodeThread.reset(new std::thread(&CSJMediaPlayer::audioDecodeFunc, this));
    }
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
}

void CSJMediaPlayer::seek(double timeStamp) {

}

bool CSJMediaPlayer::isPlaying() {
    return m_status == CSJPLAYERSTATUS_PLAYING;
}

bool CSJMediaPlayer::isPause() {
    return m_status == CSJPLAYERSTATUS_PAUSE;
}

bool CSJMediaPlayer::isStop() {
    return m_status = CSJPLAYERSTATUS_STOP;
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
    while (1) {
        if (isStop()) {
            break;
        }

        if (isPause()) {
            std::unique_lock<std::mutex> lock(m_pauseMtx);
            m_pauseCond.wait(lock);
        }

        AVPacket *pkt = av_packet_alloc();
        int ret = av_read_frame(m_pFormatCtx, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                // TODO: read to end of the file.
                break;
            }
        }

        if (pkt->stream_index == m_iVideoStreamIndex) {

            CSJPacketWrapper *pktWrapper = new CSJPacketWrapper(pkt);
            m_pVideoPacketsQueue.enBuffer(pktWrapper);
        } else if (pkt->stream_index == m_iAudioStreamIndex) {

            CSJPacketWrapper *pktWrapper = new CSJPacketWrapper(pkt);
            m_pAudioPacketsQueue.enBuffer(pktWrapper);
        }

    }
}

void CSJMediaPlayer::videoDecodeFunc() {

}

void CSJMediaPlayer::audioDecodeFunc() {

}

} // namespace csjmediaengine;