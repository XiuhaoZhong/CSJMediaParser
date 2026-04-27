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

    AVPacket* release();
private:
    AVPacket *m_pPkt;
};
} // namespace csjmediaengine