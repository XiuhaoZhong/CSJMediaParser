#ifndef __CSJMPEGTOOLWORKER_H__
#define __CSJMPEGTOOLWORKER_H__

#include <QThread>

#include "MpegHeaders/CSJMpegHeader.h"

class CSJMpegToolWorker : public QObject {
    Q_OBJECT
public:
    static CSJMpegToolWorker* getInstance();

    CSJMpegToolWorker();
    ~CSJMpegToolWorker();

    void setMediaUrl(QString &mediaUrl);

    void setPacketReadNumberPerTime(int packetRDNumber);
    void setFrameReadNumberPerTime(int frameRDNumber);

    void stop();

signals:
    void updateStreams();
    void updatePackets();
    void updateFrames();

public slots:
    void onParseStreams();
    void onParsePackets();
    void onParseFrames();

protected:
    int setup_find_stream_info_opts(AVFormatContext *s,
                                    AVDictionary *codec_opts,
                                    AVDictionary ***dst);

    int filter_codec_opts(const AVDictionary *opts, enum AVCodecID codec_id,
                          AVFormatContext *s, AVStream *st, const AVCodec *codec,
                          AVDictionary **dst);

    int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);

private:
    QString          m_mediaUrl;

    AVFormatContext *m_pFmtCtx;
    AVInputFormat   *m_pInputFmt;

    AVDictionary    *m_pFmtOpts;
    AVDictionary    *m_pCodecOpts;

    bool             m_bCanRead;

};

#endif //__CSJMPEGTOOLWORKER_H__
