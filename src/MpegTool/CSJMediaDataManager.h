#ifndef __CSJMEDIADATAMANAGER_H__
#define __CSJMEDIADATAMANAGER_H__

#include "CSJMpegHeader.h"

#include <QMutex>
#include <QSharedPointer>

#include "CSJMediaData.h"
#include "Utils/CSJLockUtils.h"


#define PACKET_READ_NUM 20
#define FRAME_READ_NUM 20

/**
 * @brief The CSJMediaDataManager is responsible for managing media data which is
 *        read from the media file.
 */
class CSJMediaDataManager {
public:
    CSJMediaDataManager();
    ~CSJMediaDataManager();

    CSJMediaDataManager(const CSJMediaDataManager& mgr) = delete;
    CSJMediaDataManager(CSJMediaDataManager&& mgr) = delete;

    static CSJMediaDataManager* getInstance();

    QString getStreamTypeWithIndex(int streamIndex);

    void addStream(CSJSpStreamInfo stream);
    void addPacket(CSJSpPacketInfo packet);
    void addFrame(CSJSpFrameInfo frame);

    void addStream(AVStream *stream);
    void addPacket(AVPacket *pkt);
    void addFrame(AVFrame *frame);

    QVector<CSJSpStreamInfo> getStreams();
    QVector<CSJSpPacketInfo> getPackets(int from = 0, int size = PACKET_READ_NUM);
    QVector<CSJSpFrameInfo>  getFrames(int from = 0, int size = FRAME_READ_NUM);

    void clearData();

private:
    QReadWriteLock           m_streamRWLock;
    QMutex                   m_streamLock;
    QVector<CSJSpStreamInfo> m_vStreams;

    QReadWriteLock           m_packetRWLock;
    QMutex                   m_packetLock;
    QVector<CSJSpPacketInfo> m_vPackets;
    int                      m_iPktReadPos = 0;

    QReadWriteLock           m_frameRWLock;
    QMutex                   m_frameLock;
    QVector<CSJSpFrameInfo>  m_vFrames;
    int                      m_iFrmReadPos = 0;
};

#endif // __CSJMEDIADATAMANAGER_H__
