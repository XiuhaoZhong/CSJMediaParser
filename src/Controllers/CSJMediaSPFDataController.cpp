#include "CSJMediaSPFDataController.h"


#include <QtCore/QDebug>
#include <QThread>
#include <QMap>

#include "CSJUIKit/CSJAccordionWidget.h"

#include "MpegTool/CSJMpegTool.h"
#include "MpegTool/CSJMediaDataManager.h"
#include "MpegTool/CSJMpegToolWorker.h"

#define PACKET_READ_NUM 10
#define FRAME_READ_NUM 10

CSJMediaSPFDataController *CSJMediaSPFDataController::getInstance() {
    static CSJMediaSPFDataController instance;
    return &instance;
}

CSJMediaSPFDataController::CSJMediaSPFDataController()
    : m_bIsParsing(false) {
    CSJMpegToolWorker *worker = CSJMpegToolWorker::getInstance();
    connect(this, SIGNAL(parseStreams()), worker, SLOT(onParseStreams()));
    connect(this, SIGNAL(parsePackets()), worker, SLOT(onParsePackets()));
    connect(this, SIGNAL(parseFrames()), worker, SLOT(onParseFrames()));

    connect(worker, SIGNAL(updateStreams()), this, SLOT(onUpdateStreams()));
    connect(worker, SIGNAL(updatePackets()), this, SLOT(onUpdatePackets()));
    connect(worker, SIGNAL(updateFrames()), this, SLOT(onUpdateFrames()));

    worker->moveToThread(&m_parseThread);
    m_parseThread.start();

    connect(&m_parseThread, &QThread::finished, this, &CSJMediaSPFDataController::onParseThreadFinished);
}

void CSJMediaSPFDataController::setMediaUrl(QString &mediaUrl) {
    qDebug() << "[INFO] CSJMediaSPFDataController::setMediaUrl, current tid: " << QThread::currentThreadId();
    CSJMpegToolWorker::getInstance()->setMediaUrl(mediaUrl);
}

CSJMediaSPFDataController::~CSJMediaSPFDataController() {
    stopParse();
}

void CSJMediaSPFDataController::setMediaDataWidget(CSJAccordionWidget *streamWidget,
                                                   CSJAccordionWidget *packetWidget,
                                                   CSJAccordionWidget *frameWidget) {
    m_pStreamWidget = streamWidget;
    m_pPacketWidget = packetWidget;
    m_pFrameWidget = frameWidget;
}

void CSJMediaSPFDataController::startParse() {
    emit parseStreams();
    m_bIsParsing = true;
}

void CSJMediaSPFDataController::stopParse() {
    CSJMpegToolWorker::getInstance()->stop();
    m_parseThread.quit();
    m_parseThread.wait();
}

void CSJMediaSPFDataController::parseStreamInfo(CSJSpStreamInfo stream,
                                                QString &streamTitle,
                                                QVector<QString> &attributeArr) {

    QString codecID = "";
    codecID += "CodecID: " + stream->getCodecId();
    attributeArr.push_back(codecID);

    QString codecType = "";
    codecType += "Format: " + stream->getCodecFormat();
    attributeArr.push_back(codecType);

    QString durationString = "Duration: ";
    int hours = 0;
    int mins = 0;
    int secs = 0;
    int us = 0;
    CSJMpegTool::getStreamDuration(stream->getStream(), hours, mins, secs, us);
    durationString += hours >= 10 ? QString::number(hours) : QString::number(0) + QString::number(hours);
    durationString += ":";
    durationString += mins >= 10 ? QString::number(mins) : QString::number(0) + QString::number(mins);
    durationString += ":";
    durationString += secs >= 10 ? QString::number(secs) : QString::number(0) + QString::number(secs);
    attributeArr.push_back(durationString);

    QString bitRateString = "Bit rate: ";
    bitRateString += QString::number(stream->getBitRate());
    attributeArr.push_back(bitRateString);

    if (stream->getType().compare("video") == 0) {
        QString videoSizeString = "Size: ";
        QSize size = stream->getSize();
        videoSizeString += QString::number(size.width()) + "x" +
                           QString::number(size.height());
        attributeArr.push_back(videoSizeString);
    }

    if (stream->getType().compare("audio") == 0) {
        QString channelString = "Channels: ";
        channelString += QString::number(stream->getNBChannels());
        attributeArr.push_back(channelString);

        QString sampleRateString = "SampleRate: ";
        sampleRateString += QString::number(stream->getSampleRate());
        attributeArr.push_back(sampleRateString);
    }

}

void CSJMediaSPFDataController::parsePacketInfo(CSJSpPacketInfo packet,
                                                QString &packetTitle,
                                                QVector<QString> &attributeArr) {
    if (!packet) {
        return ;
    }

    QString indexString = "Stream Index: ";
    indexString += QString::number(packet->getStreamIndex());

    QString sizeString = "Size: ";
    sizeString += QString::number(packet->getSize());

    QString ptsString = "Pts: ";
    ptsString += QString::number(packet->getPts());

    QString dtsString = "Dts: ";
    dtsString += QString::number(packet->getDts());

    QString posString = "Pos: ";
    posString += QString::number(packet->getPos());

    attributeArr.push_back(indexString);
    attributeArr.push_back(sizeString);
    attributeArr.push_back(ptsString);
    attributeArr.push_back(dtsString);
    attributeArr.push_back(posString);
}

void CSJMediaSPFDataController::parseFrameInfo(CSJSpFrameInfo frame,
                                               QString &frameTitle,
                                               QVector<QString> &attributeArr) {

}

void CSJMediaSPFDataController::onUpdateStreams() {
    qDebug() << "[INFO] CSJMediaSPFDataController::onUpdateStreams, current tid: " << QThread::currentThreadId();

    if (!m_pStreamWidget) {
        return ;
    }

    QVector<CSJSpStreamInfo> streams;

    streams = CSJMediaDataManager::getInstance()->getStreams();
    if (streams.size() == 0) {
        return ;
    }

    for (int i = 0; i < streams.size(); i++) {
        CSJSpStreamInfo st = streams[i];

        QString streamTitle = "";
        QVector<QString> attriArr;

        streamTitle += "Track " +
                       QString::number(st->getIndex()) +
                       ": " + st->getType();

        parseStreamInfo(st, streamTitle, attriArr);

        m_pStreamWidget->addPageWithTitle(streamTitle, attriArr);
    }
}

void CSJMediaSPFDataController::onUpdatePackets() {
    if (!m_pPacketWidget) {
        return ;
    }

    QVector<CSJSpPacketInfo> packets;
    packets = CSJMediaDataManager::getInstance()->getPackets(m_iNextPacketIndex);
    if (packets.size() == 0) {
        return ;
    }

    m_iNextPacketIndex += packets.size();
    for (int i = 0; i < packets.size(); i++) {
        CSJSpPacketInfo pkt = packets[i];

        QString pktTitle = "Packet " + QString::number(i) + ": ";
        pktTitle += CSJMediaDataManager::getInstance()->getStreamTypeWithIndex(pkt->getStreamIndex());

        QVector<QString> pktInfos;
        parsePacketInfo(pkt, pktTitle, pktInfos);

        m_pPacketWidget->addPageWithTitle(pktTitle, pktInfos);
    }
}

void CSJMediaSPFDataController::onUpdateFrames() {
    if (!m_pFrameWidget) {
        return ;
    }
}

void CSJMediaSPFDataController::onParseThreadFinished() {
    CSJMediaDataManager::getInstance()->clearData();
    m_bIsParsing = false;
}
