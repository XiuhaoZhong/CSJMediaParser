// This class is responsible for data interactions between data backend
// and UI. Current data backend is CSJMediaDataManager, and UI is the place
// where shows the stream/packet/frame data on UI.

#ifndef __CSJMEDIASPFDATACONTROLLER_H__
#define __CSJMEDIASPFDATACONTROLLER_H__

#include <QThread>

// #include "MpegTool/CSJMpegInteractions.h"

// #include "MpegTool/CSJMediaData.h"

class CSJAccordionWidget;
class CSJMediaSPFDataController : public QObject/*CSJMpegDataUpdateDelegate*/ {
    Q_OBJECT
public:
    static CSJMediaSPFDataController *getInstance();

    CSJMediaSPFDataController();
    ~CSJMediaSPFDataController();

    CSJMediaSPFDataController(const CSJMediaSPFDataController &controller) = delete;
    CSJMediaSPFDataController operator=(const CSJMediaSPFDataController &controller) = delete;

    CSJMediaSPFDataController(CSJMediaSPFDataController&& controller) = delete;
    CSJMediaSPFDataController operator=(CSJMediaSPFDataController &&controller) = delete;

    void setMediaUrl(QString &mediaUrl);

    void setMediaDataWidget(CSJAccordionWidget *streamWidget,
                            CSJAccordionWidget *packetWidget,
                            CSJAccordionWidget *frameWidget);

    void startParse();
    void stopParse();

signals:
    void parseStreams();
    void parsePackets();
    void parseFrames();

protected:
    // void parseStreamInfo(CSJSpStreamInfo stream, QString &streamTitle, QVector<QString> &attributeArr);
    // void parsePacketInfo(CSJSpPacketInfo packet, QString &packetTitle, QVector<QString> &attributeArr);
    // void parseFrameInfo(CSJSpFrameInfo frame, QString &frameTitle, QVector<QString> &attributeArr);

public slots:
    void onUpdateStreams();
    void onUpdatePackets();
    void onUpdateFrames();
    void onParseThreadFinished();

private:
    QThread            m_parseThread;

    CSJAccordionWidget *m_pStreamWidget = nullptr;
    CSJAccordionWidget *m_pPacketWidget = nullptr;
    CSJAccordionWidget *m_pFrameWidget = nullptr;

    // the next indexes are based zero.
    int m_iNextPacketIndex = 0;
    int m_iNextFrameIndex = 0;

    bool m_bIsParsing;
};

#endif // __CSJMEDIASPFDATACONTROLLER_H__
