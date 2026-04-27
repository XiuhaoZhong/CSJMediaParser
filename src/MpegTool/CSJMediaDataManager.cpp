#include "CSJMediaDataManager.h"

#include <QMutexLocker>



CSJMediaDataManager* CSJMediaDataManager::getInstance() {
    static CSJMediaDataManager mediaDataMgr;
    return &mediaDataMgr;
}

QString CSJMediaDataManager::getStreamTypeWithIndex(int streamIndex) {
    if (streamIndex >= m_vStreams.size()) {
        return "Unknown";
    }

    return m_vStreams[streamIndex]->getType();

}

CSJMediaDataManager::CSJMediaDataManager() {

}

CSJMediaDataManager::~CSJMediaDataManager() {
    if (m_vStreams.size() > 0) {
        m_vStreams.clear();
    }

    if (m_vPackets.size() > 0) {
        m_vPackets.clear();
    }

    if (m_vFrames.size() > 0) {
        m_vFrames.clear();
    }
}

void CSJMediaDataManager::addStream(CSJSpStreamInfo stream) {
    //QMutexLocker<QMutex> lock(&m_streamLock);
    CSJRWLock(m_streamRWLock, CSJRWLock_WR);
    m_vStreams.push_back(stream);
}

void CSJMediaDataManager::addPacket(CSJSpPacketInfo packet) {
    //QMutexLocker<QMutex> lock(&m_frameLock);
    CSJRWLock(m_packetRWLock, CSJRWLock_WR);
    m_vPackets.push_back(packet);
}

void CSJMediaDataManager::addFrame(CSJSpFrameInfo frame) {
    CSJRWLock(m_frameRWLock, CSJRWLock_WR);
    //QMutexLocker<QMutex> lock(&m_frameLock);
    m_vFrames.push_back(frame);
}

void CSJMediaDataManager::addStream(AVStream *stream) {
    //QMutexLocker<QMutex> lock(&m_streamLock);
    CSJRWLock(m_streamRWLock, CSJRWLock_WR);
    m_vStreams.push_back(CSJSpStreamInfo(new CSJMpegStreamInfo(stream)));
}

void CSJMediaDataManager::addPacket(AVPacket *pkt) {
   // QMutexLocker<QMutex> lock(&m_packetLock);
    CSJRWLock(m_packetRWLock, CSJRWLock_WR);
    m_vPackets.push_back(CSJSpPacketInfo(new CSJMpegPacketInfo(pkt)));
}

void CSJMediaDataManager::addFrame(AVFrame *frame) {
    //QMutexLocker<QMutex> lock(&m_frameLock);
    CSJRWLock(m_frameRWLock, CSJRWLock_WR);
    m_vFrames.push_back(CSJSpFrameInfo(new CSJMpegFrameInfo(frame)));
}

QVector<CSJSpStreamInfo> CSJMediaDataManager::getStreams() {
    //QMutexLocker<QMutex> lock(&m_streamLock);
    CSJRWLock(m_streamRWLock, CSJRWLock_WR);
    return m_vStreams;
}

QVector<CSJSpPacketInfo> CSJMediaDataManager::getPackets(int from,
                                                         int size /* = PACKET_READ_NUM */) {
    //QMutexLocker<QMutex> lock(&m_packetLock);
    CSJRWLock(m_packetRWLock, CSJRWLock_WR);
    QVector<CSJSpPacketInfo> resVec;

    if (m_vPackets.size() == 0 || from >= m_vPackets.size()) {
        return resVec;
    }

    if (from + size >= m_vPackets.size()) {
        resVec = m_vPackets.sliced(from, m_vPackets.size() - from);
    } else {
        resVec = m_vPackets.sliced(from, size);
    }

    return resVec;
}

QVector<CSJSpFrameInfo> CSJMediaDataManager::getFrames(int from,
                                                       int size /* = FRAME_READ_NUM */) {
    //QMutexLocker<QMutex> lock(&m_frameLock);
    CSJRWLock(m_frameRWLock, CSJRWLock_WR);
    QVector<CSJSpFrameInfo> resVec;

    if (m_vFrames.size() == 0 || from >= m_vFrames.size()) {
        return resVec;
    }

    if (from + size >= m_vFrames.size()) {
        resVec = m_vFrames.sliced(from, m_vFrames.size() - from);
    } else {
        resVec = m_vFrames.sliced(from, size);
    }

    return resVec;
}

void CSJMediaDataManager::clearData() {

}
