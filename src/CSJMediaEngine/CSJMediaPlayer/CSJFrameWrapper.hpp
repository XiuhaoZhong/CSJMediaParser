#pragma once

struct AVFrame;

namespace csjmediaengine {
class CSJFrameWrapper {
public:
    CSJFrameWrapper();
    CSJFrameWrapper(AVFrame *frame);
    ~CSJFrameWrapper();

    CSJFrameWrapper(const CSJFrameWrapper &other);
    CSJFrameWrapper(CSJFrameWrapper &&other);
    CSJFrameWrapper& operator=(const CSJFrameWrapper &other);
    CSJFrameWrapper& operator=(CSJFrameWrapper&& other);
    explicit operator bool() const;
    AVFrame* operator ->() const;
    bool isNull() const;

    void reset();
    void swap(CSJFrameWrapper& other);

    void setSeqNumber(int seqNumber) {
        m_seqNumber = seqNumber;
    }

    int getSeqNumber() const {
        return m_seqNumber;
    }

    AVFrame* release();
private:
    AVFrame *m_pFrame;
    // seqNumber used to track.
    int      m_seqNumber = 0;
};
} // namespace csjmediaengine