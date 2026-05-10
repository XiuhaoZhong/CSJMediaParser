#pragma once

struct AVPacket;

namespace csjmediaengine {
class CSJPacketWrapper {
public:
    CSJPacketWrapper();
    CSJPacketWrapper(AVPacket *pkt);
    ~CSJPacketWrapper();

    CSJPacketWrapper(const CSJPacketWrapper &other);
    CSJPacketWrapper(CSJPacketWrapper &&other);
    CSJPacketWrapper& operator=(const CSJPacketWrapper &other);
    CSJPacketWrapper& operator=(CSJPacketWrapper&& other);
    explicit operator bool() const;
    AVPacket* operator ->() const;
    bool isNull() const;

    void reset();
    void swap(CSJPacketWrapper& other);

    void setSeqNumber(int seqNumber) {
        m_seqNumber = seqNumber;
    }

    int getSeqNumber() const {
        return m_seqNumber;
    }

    AVPacket* release();
private:
    AVPacket *m_pPkt;
    // seqNumber used to track.
    int       m_seqNumber = 0;
};
} // namespace csjmediaengine