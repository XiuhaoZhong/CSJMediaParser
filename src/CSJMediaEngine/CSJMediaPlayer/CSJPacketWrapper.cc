#include "CSJPacketWrapper.hpp"

#include <utility>

extern "C" {
#include <libavcodec/packet.h>
}

namespace csjmediaengine {

CSJPacketWrapper::CSJPacketWrapper()
    : m_pPkt(nullptr) {
}

CSJPacketWrapper::CSJPacketWrapper(AVPacket * pkt) 
    : m_pPkt(pkt) {
}

CSJPacketWrapper::~CSJPacketWrapper() {
    reset();
}

CSJPacketWrapper::CSJPacketWrapper(const CSJPacketWrapper & other) {
    m_pPkt = nullptr;

    if (other.m_pPkt) {
        m_pPkt = av_packet_clone(other.m_pPkt);
    } 
}

CSJPacketWrapper::CSJPacketWrapper(CSJPacketWrapper && other) {
    m_pPkt = other.m_pPkt;
    other.m_pPkt = nullptr;
}

CSJPacketWrapper & CSJPacketWrapper::operator=(const CSJPacketWrapper & other) {
    if (this == &other) {
        return *this;
    }

    reset();
    if (other.m_pPkt) {
        m_pPkt = av_packet_clone(other.m_pPkt);
    }

    return *this;
}

CSJPacketWrapper & CSJPacketWrapper::operator=(CSJPacketWrapper && other) {
    if (this == &other) {
        *this;
    }

    reset();

    m_pPkt = other.m_pPkt;
    other.m_pPkt = nullptr;

    return *this;
}

CSJPacketWrapper::operator bool() const {
    return m_pPkt != nullptr;
}

AVPacket* CSJPacketWrapper::operator->() const {
    return m_pPkt;
}

bool CSJPacketWrapper::isNull() const {
    return m_pPkt == nullptr;
}

void CSJPacketWrapper::reset() {
    if (m_pPkt) {
        av_packet_free(&m_pPkt);
        m_pPkt = nullptr;
    }
}

void CSJPacketWrapper::swap(CSJPacketWrapper & other) {
    std::swap(m_pPkt, other.m_pPkt);
}

AVPacket * CSJPacketWrapper::release() {
    AVPacket *pkt = m_pPkt;
    m_pPkt = nullptr;

    return pkt;
}

} // namespace csjmediaengine
