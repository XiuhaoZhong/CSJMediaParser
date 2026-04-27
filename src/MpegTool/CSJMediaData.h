#ifndef __CSJMEDIADATA_H__
#define __CSJMEDIADATA_H__

#include "CSJMpegHeader.h"

#include <QSize>
#include <QRect>
#include <QString>
#include <QSharedPointer>

/**
 * @brief Stream information
 */
class CSJMpegStreamInfo {
public:
    CSJMpegStreamInfo(AVStream *stream);
    ~CSJMpegStreamInfo();

    int getIndex();
    int getId();
    QSize getSize();
    int getVideoDelay();

    int getNBChannels();
    int getSampleRate();
    int getFrameSize();
    int getBlockAlign();
    int getBitsPerCodedSecond();
    int getBitsPerRawSample();
    int getProfile();
    int getLevel();
    int getInitialPadding();
    int getTrailingPadding();
    int getSeekPreroll();

    int64_t getStartTime();
    int64_t getDuration();
    int64_t getBitRate();
    int64_t getNBFrames();

    QString getCodecId();
    QString getCodecFormat();
    QString getType();

    AVStream* getStream();

private:
    QString  m_type;
    AVStream *m_pStream;
};

/**
 * @brief Packet information
 */
class CSJMpegPacketInfo {
public:
    CSJMpegPacketInfo(AVPacket *);

    CSJMpegPacketInfo(const CSJMpegPacketInfo& pktInfo) {
        m_pPacket          = av_packet_alloc();
        av_packet_move_ref(m_pPacket, pktInfo.m_pPacket);
    }

    ~CSJMpegPacketInfo() {
        if (m_pPacket) {
            av_packet_unref(m_pPacket);
            av_packet_free(&m_pPacket);
        }
    }

    int getSize();
    int getStreamIndex();
    int getFlags();
    int64_t getDuration();
    int64_t getPts();
    int64_t getDts();
    int64_t getPos();

private:
    AVPacket *m_pPacket;
};

/**
 * @brief Frame information
 */
class CSJMpegFrameInfo {
public:
    CSJMpegFrameInfo(AVFrame *);

    void setType(QString type);

    CSJMpegFrameInfo(const CSJMpegFrameInfo &frameInfo) {
        m_pFrame = av_frame_alloc();
        av_frame_move_ref(m_pFrame, frameInfo.m_pFrame);
    }

    ~CSJMpegFrameInfo() {
        if (m_pFrame) {
            av_frame_unref(m_pFrame);
            av_frame_free(&m_pFrame);
        }
    }

    int64_t getPts();
    int64_t getDuration();
    int64_t getPktDts();

    QSize   getSize();
    /* Get video frame's crop info */
    QRect   getVideoCrop();

    bool    isKeyFrame();
    int     getSampleRate();
    int     getNBChannels();
    int     getNBSamples();

    QString getFormat();
    QString getPictureType();

private:
    AVFrame *m_pFrame;
    /* Devided by the input packet when decoding. */
    QString  m_type;
};

using CSJSpStreamInfo = QSharedPointer<CSJMpegStreamInfo>;
using CSJSpPacketInfo = QSharedPointer<CSJMpegPacketInfo>;
using CSJSpFrameInfo  = QSharedPointer<CSJMpegFrameInfo>;

#endif
