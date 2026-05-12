#include "CSJMediaPlayer.hpp"

#include <chrono>

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

    m_status = CSJPLAYERSTATUS_PLAYING;

    // Start a thread to read packets from media file.
    m_readThread.reset(new std::thread(&CSJMediaPlayer::readPacketsFunc, this));
    LOG_Info("Read thread starts ...");

    // Starting decoder thread according to the play mode.
    if (containVideoMode(m_playMode)) {
        m_videoDecodeThread.reset(new std::thread(&CSJMediaPlayer::videoDecodeFunc, this));
        LOG_Info("Video decoding thread starts ...");
    }

    if (containAudioMode(m_playMode)) {
        m_audioDecodeThread.reset(new std::thread(&CSJMediaPlayer::audioDecodeFunc, this));
        LOG_Info("Audio decoding thread starts ...");
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

    clearComponents();
}

void CSJMediaPlayer::seek(double timeStamp) {

}

void CSJMediaPlayer::releaseCodecCtx(AVCodecContext **codecCtx) {
    if (!(*codecCtx)) {
        return ;
    }

    avcodec_send_packet(*codecCtx, nullptr);
    avcodec_free_context(codecCtx);
    *codecCtx = nullptr;
}

void CSJMediaPlayer::clearComponents() {
    // TODO: the stop logic: must join all threads, then clear queues.
    /**
     * 1. Stop render thread, including video and audio
     * 2. Stop the reading thread
     * 3. Stop decoder thread
     * 4. Clear video and audio packets queue.
     * 5. Clear video and audio frames queue.
     */

    m_pVideoPacketsQueue.wakeUpToExit();
    m_pAudioPacketsQueue.wakeUpToExit();

    m_pVideoFramesQueue.wakeUpToExit();
    m_pAudioFramesQueue.wakeUpToExit();

    if (m_readThread && m_readThread->joinable()) {
        m_readThread->join();
        m_readThread.reset();
    }

    if (m_videoDecodeThread && m_videoDecodeThread->joinable()) {
        m_videoDecodeThread->join();
    }

    if (m_audioDecodeThread && m_audioDecodeThread->joinable()) {
        m_audioDecodeThread->join();
    }

    releaseCodecCtx(&m_pVideoCodecCtx);
    releaseCodecCtx(&m_pAudioCodecCtx);

    if (m_pFormatCtx) {
        avformat_close_input(&m_pFormatCtx);
        m_pFormatCtx = nullptr;
    }

    clearMediaPackets();
    clearMediaFrames();

    m_iVideoPktSeqNum = 0;
    m_iAudioPktSeqNum = 0;

    LOG_Info("play components clear over!");
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
        LOG_Error("Media file %s open failed!", m_file.c_str());
        return false;
    }

    res = avformat_find_stream_info(m_pFormatCtx, NULL);
    if (res < 0) {
        LOG_Error("Can't find valid streams in media file!");
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
            CSJPacketWrapperPtr pktWrapper = std::make_shared<CSJPacketWrapper>(pkt);
            pktWrapper->setSeqNumber(m_iVideoPktSeqNum++);
            m_pVideoPacketsQueue.enBuffer(pktWrapper);
            LOG_Info("Reading the %dth video packet, put it into ring buffer.", pktWrapper->getSeqNumber());
        } else if (pkt->stream_index == m_iAudioStreamIndex) {
            CSJPacketWrapperPtr pktWrapper = std::make_shared<CSJPacketWrapper>(pkt);
            pktWrapper->setSeqNumber(m_iAudioPktSeqNum++);
            m_pAudioPacketsQueue.enBuffer(pktWrapper);
            LOG_Info("Reading the %dth audio packet, put it into ring buffer.", pktWrapper->getSeqNumber());
        }
    }

    if (pkt) {
        av_packet_free(&pkt);
    }

    LOG_Info("Quit reading from media file %s", m_file.c_str());
}

void CSJMediaPlayer::videoDecodeFunc() {
    LOG_Info("Start decoding video packets...");

    if (!avcodec_is_open(m_pVideoCodecCtx)) {
        int ret = avcodec_open2(m_pVideoCodecCtx, m_pVideoCodecCtx->codec, NULL);
        if (ret < 0) {
            LOG_Info("Video codec open failed! Exit video decode function!");
            return ;
        }
        
        LOG_Info("Video codec open successfully!");
    }

    while (true) {
        if (isStop()) {
            LOG_Info("Player stop, exit video decoding.");
            break;
        }

        if (isPause()) {
            LOG_Info("Pause video decoding.");
            std::unique_lock<std::mutex> lock(m_pauseMtx);
            m_pauseCond.wait(lock);
        }

        auto opt = m_pVideoPacketsQueue.deBuffer();
        if (!opt) {
            LOG_Info("The video packet queue is empty, exit decoding!");
            break;
        }

        CSJPacketWrapperPtr packetWrapper = opt.value();
        int seqNum = packetWrapper->getSeqNumber();
        CSJFrameWrapperPtr frameWrapper = commonDecode(m_pVideoCodecCtx, packetWrapper);

        if (!frameWrapper) {
            LOG_Warn("The %dth video packet decoded an empty frame", seqNum);
        } else {
            frameWrapper->setSeqNumber(seqNum);
            LOG_Info("The %dth video frame has been decoded.", frameWrapper->getSeqNumber());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //m_pVideoFramesQueue.enBuffer(frameWrapper);
    }
}

void CSJMediaPlayer::audioDecodeFunc() {
    LOG_Info("Start decoding audio packets...");

    if (!avcodec_is_open(m_pAudioCodecCtx)) {
        int ret = avcodec_open2(m_pAudioCodecCtx, m_pAudioCodecCtx->codec, NULL);
        if (ret < 0) {
            LOG_Info("Audio codec open failed! Exit video decode function!");
            return ;
        }
        
        LOG_Info("Audio codec open successfully!");
    }

    while (true) {
        if (isStop()) {
            LOG_Info("Player stop, exit audio decoding.");
            break;
        }

        if (isPause()) {
            LOG_Info("Pause video decoding.");
            std::unique_lock<std::mutex> lock(m_pauseMtx);
            m_pauseCond.wait(lock);
        }

        auto opt = m_pAudioPacketsQueue.deBuffer();
        if (!opt) {
            LOG_Info("The video packet queue is empty, exit decoding!");
            break;
        }

        CSJPacketWrapperPtr packetWrapper = opt.value();
        int seqNum = packetWrapper->getSeqNumber();
        CSJFrameWrapperPtr frameWrapper = commonDecode(m_pAudioCodecCtx, packetWrapper);

        if (!frameWrapper) {
            LOG_Warn("The %dth audio packet decoded an empty frame", seqNum);
        } else {
            frameWrapper->setSeqNumber(seqNum);
            LOG_Info("The %dth audio frame has been decoded.", frameWrapper->getSeqNumber());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //m_pAudioFramesQueue.enBuffer(frameWrapper);
    }
}

CSJFrameWrapperPtr CSJMediaPlayer::commonDecode(AVCodecContext *codec_ctx, CSJPacketWrapperPtr wrapperPkt) {
    if (!codec_ctx || !wrapperPkt) {
        return nullptr;
    }

    AVPacket *pkt = wrapperPkt->release();
    int ret = avcodec_send_packet(codec_ctx, pkt);
    if (ret < 0) {
        av_packet_free(&pkt);
        return nullptr;
    }

    CSJFrameWrapperPtr FrameWrapper = nullptr;

    while (true) {
        AVFrame* frame = av_frame_alloc();

        ret = avcodec_receive_frame(codec_ctx, frame);
        if (ret == AVERROR(EAGAIN)) {
            av_frame_free(&frame);
             LOG_Info("The %dth packet needs more packet to decode!", wrapperPkt->getSeqNumber());
            break;
        } else if (ret == AVERROR_EOF) {
             LOG_Info("The decode failed because eof!");
            av_frame_free(&frame);
            break;
        } else if (ret < 0) {
             LOG_Info("The %dth packet decode failed because an error!", wrapperPkt->getSeqNumber());
            av_frame_free(&frame);
            break;
        }

        LOG_Info("The %dth packet decoded successfully!!", wrapperPkt->getSeqNumber());
        FrameWrapper = std::make_shared<CSJFrameWrapper>(frame);
        av_frame_free(&frame);
    }

    av_packet_free(&pkt);
    return FrameWrapper;
}

void CSJMediaPlayer::clearMediaPackets() {
    m_pVideoPacketsQueue.reset();
    m_pAudioPacketsQueue.reset();
}

void CSJMediaPlayer::clearMediaFrames() {
   m_pVideoFramesQueue.reset();
   m_pAudioFramesQueue.reset();
}

void CSJMediaPlayer::clearPacketsQueue(CSJRingQueue<CSJPacketWrapperPtr> &queue) {
    
}

void CSJMediaPlayer::clearFrameQueue(CSJRingQueue<CSJFrameWrapperPtr> &queue) {
    while (!queue.is_empty()) {
        auto item = queue.deBuffer();
    }
}

} // namespace csjmediaengine;