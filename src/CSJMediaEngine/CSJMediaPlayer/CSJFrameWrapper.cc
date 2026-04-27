#include "CSJFrameWrapper.hpp"

#include <utility>

extern "C" {
#include <libavutil/frame.h>
}

namespace csjmediaengine {

CSJFrameWrapper::CSJFrameWrapper()
    : m_pFrame(nullptr) {
}

CSJFrameWrapper::CSJFrameWrapper(AVFrame * Frame) 
    : m_pFrame(Frame) {
}

CSJFrameWrapper::~CSJFrameWrapper() {
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
    other.m_pFrame = nullptr;
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
    other.m_pFrame = nullptr;

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
}

void CSJFrameWrapper::swap(CSJFrameWrapper & other) {
    std::swap(m_pFrame, other.m_pFrame);
}

AVFrame * CSJFrameWrapper::release() {
    AVFrame *Frame = m_pFrame;
    m_pFrame = nullptr;

    return Frame;
}

} // namespace csjmediaengine
