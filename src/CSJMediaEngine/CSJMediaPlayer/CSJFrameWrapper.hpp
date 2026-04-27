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

    AVFrame* release();
private:
    AVFrame *m_pFrame;
};
} // namespace csjmediaengine