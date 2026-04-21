#include "CSJMediaRawData.h"

namespace csjmediaengine{

#ifdef _WIN32
std::wstring SubTypeToString(GUID& subtype) {
    WCHAR buffer[128];
    StringFromGUID2(subtype, buffer, sizeof(buffer));

    std::wstring res = L"";
    std::wstring st(buffer);
    if (st.compare(L"{3231564E-0000-0010-8000-00AA00389B71}") == 0) {
        res = L"NV12";
    } else if (st.compare(L"{47504A4D-0000-0010-8000-00AA00389B71}") == 0) {
        res = L"MJPG";
    } else if (st.compare(L"{32595559-0000-0010-8000-00AA00389B71}") == 0) {
        res = L"YV12";
    } else if (st.compare(L"{00000003-0000-0010-8000-00AA00389B71}") == 0) {
        res = L"MFAudioFormat_Float";
    } else if (st.compare(L"{00001610-0000-0010-8000-00AA00389B71}") == 0) {
        res = L"MFAudioFormat_AAC";
    } else if (st.compare(L"00000055-0000-0010-8000-00AA00389B71") == 0) {
        res = L"MFAudioFormat_MP3";
    }

    return res;
}
#endif

CSJVideoFormatType string2VideoType(std::wstring &fmtString) {
    if (fmtString.compare(L"NV12") == 0) {
        return CSJVIDEO_FMT_NV12;
    } else if (fmtString.compare(L"I420") == 0) {
        return CSJVIDEO_FMT_YUV420P;
    } else if (fmtString.compare(L"YV12") == 0) {
        return CSJVIDEO_FMT_YV12;
    }

    return CSJVIDEO_FMT_NONE;
}

CSJVideoData::CSJVideoData() {

}

CSJVideoData::CSJVideoData(CSJVideoFormatType fmtType, uint8_t* data, int width, int height)
    : m_fmtType(fmtType)
    , m_pData(nullptr)
    , m_iWidth(width)
    , m_iHeight(height) {

    if (fmtType == CSJVIDEO_FMT_NV12) {
        initWithYUV420(data);
    } else if (fmtType == CSJVIDEO_FMT_RGB24) {
        initWithRGB24(data);
    }
}

CSJVideoData::CSJVideoData(CSJVideoFormatType fmtType, int width, int height, double pts, 
                    double timeStamp, double duration, uint8_t *data)
    : m_fmtType(fmtType)
    , m_pData(nullptr)
    , m_iWidth(width)
    , m_iHeight(height)
    , m_dPts(pts)
    , m_dTimeStamp(timeStamp)
    , m_dDuration(duration) {
            
    if (fmtType == CSJVIDEO_FMT_NV12) {
        initWithYUV420(data);
    } else if (fmtType == CSJVIDEO_FMT_RGB24) {
        initWithRGB24(data);
    }
}

CSJVideoData::~CSJVideoData() {
    if (m_pData) {
        delete m_pData;
        m_pData = nullptr;
    }
}

CSJVideoData::CSJVideoData(CSJVideoData && videoData)
    : m_fmtType(videoData.m_fmtType)
    , m_pData(videoData.m_pData)
    , m_iWidth(videoData.m_iWidth)
    , m_iHeight(videoData.m_iHeight)
    , m_dPts(videoData.m_dPts)
    , m_dTimeStamp(videoData.m_dTimeStamp)
    , m_dDuration(videoData.m_dDuration) {

    videoData.m_fmtType    = CSJVIDEO_FMT_NONE;
    videoData.m_pData      = nullptr;
    videoData.m_iWidth     = 0;
    videoData.m_iHeight    = 0;
    videoData.m_dPts       = 0.0;
    videoData.m_dTimeStamp = 0.0;
    videoData.m_dDuration  = 0.0;
}

CSJVideoData& CSJVideoData::operator=(CSJVideoData && videoData) {
    CSJVideoData tmp(std::move(*this));

    m_fmtType    = videoData.m_fmtType;
    m_pData      = videoData.m_pData;
    m_iWidth     = videoData.m_iWidth;
    m_iHeight    = videoData.m_iHeight;
    m_dPts       = videoData.m_dPts;
    m_dTimeStamp = videoData.m_dTimeStamp;
    m_dDuration  = videoData.m_dDuration;

    videoData.m_fmtType    = CSJVIDEO_FMT_NONE;
    videoData.m_pData      = nullptr;
    videoData.m_iWidth     = 0;
    videoData.m_iHeight    = 0;
    videoData.m_dPts       = 0.0;
    videoData.m_dTimeStamp = 0.0;
    videoData.m_dDuration  = 0.0;

    return *this;
}

CSJVideoFormatType CSJVideoData::getFmtType() const {
    return m_fmtType;
}

uint8_t* CSJVideoData::getData() const {
    return m_pData;
}

uint8_t * CSJVideoData::getyuvY() const {
    // TODO: Calculate Y address with the video format, width and height.
    return nullptr;
}

uint8_t * CSJVideoData::getyuvU() const {
    // TODO: Calculate U address with the video format, width and height.
    return nullptr;
}

uint8_t * CSJVideoData::getyuvV() const {
    // TODO: Calculate V address with the video format, width and height.
    return nullptr;
}

int CSJVideoData::getWidth() const {
    return m_iWidth;
}

int CSJVideoData::getHeight() const {
    return m_iHeight;
}

double CSJVideoData::getDuration() const {
    return m_dDuration;
}

double CSJVideoData::getPts() const {
    return m_dPts;
}

double CSJVideoData::getTimeStamp() const {
    return m_dTimeStamp;
}

void CSJVideoData::initWithYUV420(uint8_t* data) {
    int yuvBytesLen = m_iWidth * m_iHeight * 3 / 2;

    m_pData = new uint8_t[yuvBytesLen];
    memcpy(m_pData, data, yuvBytesLen);
}

void CSJVideoData::initWithRGB24(uint8_t* data) {
    int length = m_iWidth * m_iHeight * 3;
    m_pData = new uint8_t[length];
    memcpy(m_pData, data, length);
}

CSJAudioData::CSJAudioData() {

}

CSJAudioData::CSJAudioData(CSJAudioFormatType fmtType, uint8_t* pdata,
                           int sampleRate, int channels, int bitsPerSample)
    : m_fmtType(fmtType)
    , m_pData(pdata)
    , m_iSampleRate(sampleRate)
    , m_iChannels(channels)
    , m_iBitsPerSample(bitsPerSample) {

}

CSJAudioData::~CSJAudioData() {
    if (m_pData) {
        delete m_pData;
        m_pData = nullptr;
    }
}

CSJAudioData::CSJAudioData(CSJAudioData && audioData)
    : m_fmtType(audioData.m_fmtType)
    , m_pData(audioData.m_pData)
    , m_iSampleRate(audioData.m_iSampleRate)
    , m_iChannels(audioData.m_iChannels)
    , m_iBitsPerSample(audioData.m_iBitsPerSample) {

    audioData.m_fmtType = CSJAUDIO_FMT_NONE;
    audioData.m_pData = nullptr;
    audioData.m_iSampleRate = 0;
    audioData.m_iChannels = 0;
    audioData.m_iBitsPerSample = 0;
}

CSJAudioData& CSJAudioData::operator=(CSJAudioData && audioData) {
    m_fmtType        = audioData.m_fmtType;
    m_pData          = audioData.m_pData;
    m_iSampleRate    = audioData.m_iSampleRate;
    m_iChannels      = audioData.m_iChannels;
    m_iBitsPerSample = audioData.m_iBitsPerSample;

    audioData.m_fmtType        = CSJAUDIO_FMT_NONE;
    audioData.m_pData          = nullptr;
    audioData.m_iSampleRate    = 0;
    audioData.m_iChannels      = 0;
    audioData.m_iBitsPerSample = 0;

    return *this;
}

} // namespace csjmediaengine