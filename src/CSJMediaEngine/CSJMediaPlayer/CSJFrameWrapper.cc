#include "CSJFrameWrapper.hpp"

#include <utility>

extern "C" {
#include <libavutil/frame.h>
}

#include "CSJUtils/CSJLogger.h"

namespace csjmediaengine {

CSJFrameWrapper::CSJFrameWrapper()
    : m_pFrame(nullptr) {
}

CSJFrameWrapper::CSJFrameWrapper(AVFrame * Frame) 
    : m_pFrame(Frame) {
    m_pFrame = av_frame_alloc();
    av_frame_move_ref(m_pFrame, Frame);
}

CSJFrameWrapper::~CSJFrameWrapper() {
    LOG_Info("The %dth frame released.", m_seqNumber);
    reset();
}

CSJFrameWrapper::CSJFrameWrapper(const CSJFrameWrapper & other) {
    m_pFrame = nullptr;

    if (other.m_pFrame) {
        //m_pFrame = av_packet_clone(other.m_pFrame);
    } 
}

CSJFrameWrapper::CSJFrameWrapper(CSJFrameWrapper && other) {
    m_pFrame = other.m_pFrame;
    m_seqNumber = other.m_seqNumber;

    other.m_pFrame = nullptr;
    other.m_seqNumber = 0;
}

CSJFrameWrapper & CSJFrameWrapper::operator=(const CSJFrameWrapper & other) {
    if (this == &other) {
        return *this;
    }

    reset();
    if (other.m_pFrame) {
        //m_pFrame = av_packet_clone(other.m_pFrame);
    }

    return *this;
}

CSJFrameWrapper & CSJFrameWrapper::operator=(CSJFrameWrapper && other) {
    if (this == &other) {
        *this;
    }

    reset();

    m_pFrame = other.m_pFrame;
    m_seqNumber = other.m_seqNumber;

    other.m_pFrame = nullptr;
    other.m_seqNumber = 0;

    return *this;
}

CSJFrameWrapper::operator bool() const {
    return m_pFrame != nullptr;
}

AVFrame* CSJFrameWrapper::operator->() const {
    return m_pFrame;
}

bool CSJFrameWrapper::isNull() const {
    return m_pFrame == nullptr;
}

void CSJFrameWrapper::reset() {
    if (m_pFrame) {
        //av_packet_free(m_pFrame);
        m_pFrame = nullptr;
    }

    m_seqNumber = 0;
}

void CSJFrameWrapper::swap(CSJFrameWrapper & other) {
    std::swap(m_pFrame, other.m_pFrame);
    std::swap(m_seqNumber, other.m_seqNumber);
}

AVFrame * CSJFrameWrapper::release() {
    AVFrame *Frame = m_pFrame;
    m_pFrame = nullptr;

    return Frame;
}

} // namespace csjmediaengine
