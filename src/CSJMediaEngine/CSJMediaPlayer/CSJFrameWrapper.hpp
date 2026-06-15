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
    AVFrame* getRawFrame() const;
    AVFrame* operator ->() const;
    bool isNull() const;

    void reset();
    void swap(CSJFrameWrapper& other);

    void setSeqNumber(int seqNumber);
    int getSeqNumber() const;
    int getWidth() const;
    int getHeight() const;

    double getDuration() const;
    double getTimeStamp() const;

    AVFrame* release();
private:
    AVFrame *m_pFrame;
    // seqNumber used to track.
    int      m_seqNumber = 0;
    double   m_dDuration;
    double   m_dTimeStamp;
};
} // namespace csjmediaengine