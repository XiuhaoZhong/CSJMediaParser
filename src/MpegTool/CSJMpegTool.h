#ifndef __CSJMPEGTOOL_H__
#define __CSJMPEGTOOL_H__

#include <QObject>
#include <QString>

#include "MpegHeaders/CSJMpegHeader.h"
#include "CSJMpegInteractions.h"

class CSJMpegDataUpdateDelegate;
class CSJMpegTool : public QObject {
    Q_OBJECT
public:
    static QString getFFMpegVersion();
    static QString getFFMpegBuildConf();
    static CSJMpegTool* getInstance();

    static void getStreamDuration(AVStream *st, int &hours, int &mins, int &secs, int &us);

    CSJMpegTool();
    ~CSJMpegTool();

    /**
     * @brief parseMediaContent start parse media file or media stream.
     *
     * @param[in] fileUrl the file path or stream url.
     *
     * @return true if start success, or return false.
     */
    bool parseMediaContent(QString fileUrl);

    void setDataUpdateDelegate(CSJMpegDataUpdateDelegate *dataDelegate);

    void updateMediaData(CSJMpegUpdateDataType dataType);

protected:
    int setup_find_stream_info_opts(AVFormatContext *s,
                                    AVDictionary *codec_opts,
                                    AVDictionary ***dst);

    int filter_codec_opts(const AVDictionary *opts, enum AVCodecID codec_id,
                          AVFormatContext *s, AVStream *st, const AVCodec *codec,
                          AVDictionary **dst);

    int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);

Q_SIGNALS:
    void updateStreams();
    void updatePackets();
    void updateFrames();

private:
    AVFormatContext *m_pFmtCtx;
    AVInputFormat   *m_pInputFmt;

    AVDictionary    *m_pFmtOpts;
    AVDictionary    *m_pCodecOpts;

    CSJMpegDataUpdateDelegate *m_pDelegate;

};

#endif // __CSJMPEGTOOL_H__
