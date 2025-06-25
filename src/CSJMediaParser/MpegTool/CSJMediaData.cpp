#include "CSJMediaData.h"

QString codecID2String(AVCodecID codec_id) {
    QString codecIDString = "";

    switch (codec_id) {
    case AV_CODEC_ID_H264:
        codecIDString = "H264";
        break;
    case AV_CODEC_ID_HEVC:
        codecIDString = "HEVC";
        break;
    case AV_CODEC_ID_VP8:
        codecIDString = "VBP8";
        break;
    case AV_CODEC_ID_VP9:
        codecIDString = "VP9";
        break;
    case AV_CODEC_ID_MP3:
        codecIDString = "MP3";
        break;
    case AV_CODEC_ID_AAC:
        codecIDString = "AAC";
        break;
    case AV_CODEC_ID_FLAC:
        codecIDString = "FLAC";
        break;
    case AV_CODEC_ID_APE:
        codecIDString = "APE";
        break;
    case AV_CODEC_ID_OPUS:
        codecIDString = "OPUS";
        break;
    default:
        codecIDString = "Unknown";
        break;
    }

    return codecIDString;
}

QString videoFMT2String(AVPixelFormat format) {
    QString videoFmtString = "";

    switch (format) {
    case AV_PIX_FMT_YUV420P:
        videoFmtString = "YUV_420P";
        break;
    case AV_PIX_FMT_YUYV422:
        videoFmtString = "YUV_YV422";
        break;
    case AV_PIX_FMT_RGB24:
        videoFmtString = "RGB24";
        break;
    case AV_PIX_FMT_BGR24:
        videoFmtString = "BGR24";
        break;
    case AV_PIX_FMT_YUV422P:
        videoFmtString = "YUV_422P";
        break;
    case AV_PIX_FMT_YUV444P:
        videoFmtString = "YUV_444P";
        break;
    case AV_PIX_FMT_NV12:
        videoFmtString = "NV12";
        break;
    case AV_PIX_FMT_NV21:
        videoFmtString = "NV21";
        break;
    default:
        videoFmtString = "Unknown";
        break;
    }

    return videoFmtString;
}

QString audioFMT2String(AVSampleFormat format) {
    QString audioFmtString = "";

    switch (format) {
    case AV_SAMPLE_FMT_U8:
        audioFmtString = "U8";
        break;
    case AV_SAMPLE_FMT_S16:
        audioFmtString = "S16";
        break;
    case AV_SAMPLE_FMT_S32:
        audioFmtString = "S32";
        break;
    case AV_SAMPLE_FMT_FLT:
        audioFmtString = "Float";
        break;
    case AV_SAMPLE_FMT_DBL:
        audioFmtString = "Double";
        break;
    case AV_SAMPLE_FMT_U8P:
        audioFmtString = "U8P";
        break;
    case AV_SAMPLE_FMT_S16P:
        audioFmtString = "S16P";
        break;
    case AV_SAMPLE_FMT_S32P:
        audioFmtString = "S32P";
        break;
    case AV_SAMPLE_FMT_FLTP:
        audioFmtString = "FloatP";
        break;
    case AV_SAMPLE_FMT_DBLP:
        audioFmtString = "DoubleP";
        break;
    default:
        audioFmtString = "Unknown";
        break;
    }

    return audioFmtString;
}

QString pictype2String(AVPictureType pict_type) {
    QString pictureType = "";

    switch (pict_type) {
    case AV_PICTURE_TYPE_I:
        pictureType = "I帧";
        break;
    case AV_PICTURE_TYPE_P:
        pictureType = "P帧";
        break;
    case AV_PICTURE_TYPE_B:
        pictureType = "B帧";
        break;
    case AV_PICTURE_TYPE_S:
        pictureType = "";
        break;
    case AV_PICTURE_TYPE_SI:
        pictureType = "SI帧";
        break;
    case AV_PICTURE_TYPE_SP:
        pictureType = "SP帧";
        break;
    case AV_PICTURE_TYPE_BI:
        pictureType = "";
        break;
    case AV_PICTURE_TYPE_NONE:
    default:
        break;
    }

    return pictureType;
};

CSJMpegStreamInfo::CSJMpegStreamInfo(AVStream *stream) {
    if (!stream) {
        this->m_pStream = nullptr;
        return ;
    }

    m_type = av_get_media_type_string(stream->codecpar->codec_type);
    this->m_pStream = stream;
}

CSJMpegStreamInfo::~CSJMpegStreamInfo() {
    m_pStream = nullptr;
}

int CSJMpegStreamInfo::getIndex() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->index;
}

int CSJMpegStreamInfo::getId() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->id;
}

int64_t CSJMpegStreamInfo::getStartTime() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->start_time;
}

int64_t CSJMpegStreamInfo::getDuration() {
    if (!m_pStream) {
        return 0;
    }

    return m_pStream->duration;
}

int64_t CSJMpegStreamInfo::getBitRate() {
    if (!m_pStream) {
        return 0;
    }

    return m_pStream->codecpar->bit_rate;
}

int64_t CSJMpegStreamInfo::getNBFrames() {
    if (!m_pStream) {
        return 0;
    }

    return m_pStream->nb_frames;
}

QString CSJMpegStreamInfo::getCodecId() {
    if (!m_pStream) {
        return QString("");
    }

    return codecID2String(m_pStream->codecpar->codec_id);
}

QSize CSJMpegStreamInfo::getSize() {
    if (!m_pStream) {
        return {0, 0};
    }

    return {m_pStream->codecpar->width, m_pStream->codecpar->height};
}

int CSJMpegStreamInfo::getVideoDelay() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->video_delay;
}

QString CSJMpegStreamInfo::getCodecFormat() {
    if (!m_pStream) {
        return "";
    }

    if (m_type.compare("video") == 0) {
        return videoFMT2String((AVPixelFormat)m_pStream->codecpar->format);
    }

    if (m_type.compare("audio") == 0) {
        return audioFMT2String((AVSampleFormat)m_pStream->codecpar->format);
    }

    return "";
}

QString CSJMpegStreamInfo::getType() {
    return m_type;
}

AVStream *CSJMpegStreamInfo::getStream() {
    return m_pStream;
}

int CSJMpegStreamInfo::getNBChannels() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->ch_layout.nb_channels;
}

int CSJMpegStreamInfo::getSampleRate() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->sample_rate;
}

int CSJMpegStreamInfo::getFrameSize() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->frame_size;
}

int CSJMpegStreamInfo::getBlockAlign() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->block_align;
}

int CSJMpegStreamInfo::getBitsPerCodedSecond() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->bits_per_coded_sample;
}

int CSJMpegStreamInfo::getBitsPerRawSample() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->bits_per_raw_sample;
}

int CSJMpegStreamInfo::getProfile() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->profile;
}

int CSJMpegStreamInfo::getLevel() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->level;
}

int CSJMpegStreamInfo::getInitialPadding() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->initial_padding;
}

int CSJMpegStreamInfo::getTrailingPadding() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->trailing_padding;
}

int CSJMpegStreamInfo::getSeekPreroll() {
    if (!m_pStream) {
        return -1;
    }

    return m_pStream->codecpar->seek_preroll;
}

CSJMpegPacketInfo::CSJMpegPacketInfo(AVPacket *packet) {
    if (!packet) {
        m_pPacket = nullptr;
        return ;
    }

    this->m_pPacket = av_packet_alloc();
    av_packet_move_ref(this->m_pPacket, packet);
}

int CSJMpegPacketInfo::getSize() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->size;
}

int CSJMpegPacketInfo::getStreamIndex() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->stream_index;
}

int CSJMpegPacketInfo::getFlags() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->flags;
}

int64_t CSJMpegPacketInfo::getDuration() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->duration;
}

int64_t CSJMpegPacketInfo::getPts() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->pts;
}

int64_t CSJMpegPacketInfo::getDts() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->dts;
}

int64_t CSJMpegPacketInfo::getPos() {
    if (!m_pPacket) {
        return -1;
    }

    return m_pPacket->pos;
}

CSJMpegFrameInfo::CSJMpegFrameInfo(AVFrame* frame) {

    this->m_pFrame = av_frame_alloc();
    av_frame_move_ref(this->m_pFrame, frame);
}

int64_t CSJMpegFrameInfo::getPts() {
    if (!m_pFrame) {
        return -1;
    }

    return m_pFrame->pts;
}

int64_t CSJMpegFrameInfo::getDuration() {
    if (!m_pFrame) {
        return -1;
    }

    return m_pFrame->duration;
}

int64_t CSJMpegFrameInfo::getPktDts() {
    if (!m_pFrame) {
        return -1;
    }

    return m_pFrame->pkt_dts;
}

QSize CSJMpegFrameInfo::getSize() {
    if (!m_pFrame) {
        return {0, 0};
    }

    return {m_pFrame->width, m_pFrame->height};
}

QRect CSJMpegFrameInfo::getVideoCrop() {
    if (!m_pFrame) {
        return {0, 0, 0, 0};
    }

    return {(int)m_pFrame->crop_left,
            (int)m_pFrame->crop_top,
            (int)m_pFrame->crop_right,
            (int)m_pFrame->crop_bottom};
}

bool CSJMpegFrameInfo::isKeyFrame() {
    if (!m_pFrame) {
        return false;
    }

    return m_pFrame->flags & !AV_FRAME_FLAG_KEY;
}

int CSJMpegFrameInfo::getSampleRate() {
    if (!m_pFrame) {
        return -1;
    }

    return m_pFrame->sample_rate;
}

int CSJMpegFrameInfo::getNBChannels() {
    if (!m_pFrame) {
        return -1;
    }

    return m_pFrame->ch_layout.nb_channels;
}

int CSJMpegFrameInfo::getNBSamples() {
    if (!m_pFrame) {
        return -1;
    }

    return m_pFrame->nb_samples;
}

QString CSJMpegFrameInfo::getFormat() {
    if (!m_pFrame) {
        return "";
    }

    if (m_type.compare("video") == 0) {
        return videoFMT2String((AVPixelFormat)m_pFrame->format);
    }

    if (m_type.compare("audio") == 0) {
        return audioFMT2String((AVSampleFormat)m_pFrame->format);
    }

    return "";
}

QString CSJMpegFrameInfo::getPictureType() {
    if (!m_pFrame) {
        return "";
    }

    return pictype2String(m_pFrame->pict_type);
}
