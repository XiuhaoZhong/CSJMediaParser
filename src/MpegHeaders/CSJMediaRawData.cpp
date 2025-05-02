#include "CSJMediaRawData.h"

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

CSJVideoData::CSJVideoData()
    : m_fmtType(CSJVIDEO_FMT_NONE)
    , m_data(nullptr)
    , m_width(0)
    , m_height(0) {
}

CSJVideoData::CSJVideoData(CSJVideoFormatType fmtType, uint8_t* data, int width, int height)
    : m_fmtType(fmtType), m_data(data), m_width(width), m_height(height) {

    if (fmtType == CSJVIDEO_FMT_NV12) {
        initWithYUV420();
    } else if (fmtType == CSJVIDEO_FMT_RGB24) {
        initWithRGB24();
    }
}

CSJVideoData::~CSJVideoData() {
    /*if (m_data) {
        delete m_data;
        m_data = nullptr;
    }*/

    if (m_dataY) {
        delete[] m_dataY;
        m_dataY = nullptr;
    }

    if (m_dataU) {
        delete[] m_dataU;
        m_dataU = nullptr;
    }

    if (m_dataV) {
        delete[] m_dataV;
        m_dataV = nullptr;
    }

    if (m_dataRGB24) {
        delete[] m_dataRGB24;
        m_dataRGB24 = nullptr;
    }
}

CSJVideoData::CSJVideoData(CSJVideoData && videoData)
    : m_fmtType(videoData.m_fmtType)
    , m_data(videoData.m_data)
    , m_width(videoData.m_width)
    , m_height(videoData.m_height) {

    videoData.m_fmtType = CSJVIDEO_FMT_NONE;
    videoData.m_data = nullptr;
    videoData.m_width = 0;
    videoData.m_height = 0;

    m_dataY = videoData.m_dataY;
    m_dataU = videoData.m_dataU;
    m_dataV = videoData.m_dataV;

    m_dataRGB24 = videoData.m_dataRGB24;

    videoData.m_dataY = nullptr;
    videoData.m_dataU = nullptr;
    videoData.m_dataV = nullptr;

    videoData.m_dataRGB24 = nullptr;
}

CSJVideoData& CSJVideoData::operator=(CSJVideoData && videoData) {
    CSJVideoData tmp(std::move(*this));

    m_fmtType = videoData.m_fmtType;
    m_data = videoData.m_data;
    m_width = videoData.m_width;
    m_height = videoData.m_height;

    videoData.m_fmtType = CSJVIDEO_FMT_NONE;
    videoData.m_data = nullptr;
    videoData.m_width = 0;
    videoData.m_height = 0;

    m_dataY = videoData.m_dataY;
    m_dataU = videoData.m_dataU;
    m_dataV = videoData.m_dataV;
    m_dataRGB24 = videoData.m_dataRGB24;

    videoData.m_dataY = nullptr;
    videoData.m_dataU = nullptr;
    videoData.m_dataV = nullptr;

    videoData.m_dataRGB24 = nullptr;

    return *this;
}

CSJVideoFormatType CSJVideoData::getFmtType() const {
    return m_fmtType;
}

uint8_t* CSJVideoData::getData() const {
    return m_data;
}

uint8_t * CSJVideoData::getyuvY() const {
    return m_dataY;
}

uint8_t * CSJVideoData::getyuvU() const {
    return m_dataU;
}

uint8_t * CSJVideoData::getyuvV() const {
    return m_dataV;
}

uint8_t * CSJVideoData::getRGB24() const {
    return m_dataRGB24;
}

int CSJVideoData::getWidth() const {
    return m_width;
}

int CSJVideoData::getHeight() const {
    return m_height;
}

void CSJVideoData::initWithYUV420() {
    int yLength = m_width * m_height;
    m_dataY = new uint8_t[yLength];
    m_dataU = new uint8_t[yLength / 4];
    m_dataV = new uint8_t[yLength / 4];

    memcpy(m_dataY, m_data, yLength);

    uint8_t* uvStart = m_data + yLength;
    for (size_t i = 0; i < yLength / 4; i++) {
        memcpy(m_dataU + i, uvStart + 2 * i, 1);
        memcpy(m_dataV + i, uvStart + 2 * i + 1, 1);
    }
}

void CSJVideoData::initWithRGB24() {
    if (!m_data) {
        return;
    }

    int length = m_width * m_height * 3;
    m_dataRGB24 = new uint8_t[length];
    memcpy(m_dataRGB24, m_dataRGB24, length);
}

CSJAudioData::CSJAudioData()
    : m_fmtType(CSJAUDIO_FMT_NONE)
    , m_data(nullptr)
    , m_sampleRate(0)
    , m_channels(0)
    , m_bitsPerSample(0) {
}

CSJAudioData::CSJAudioData(CSJAudioFormatType fmtType, uint8_t* pdata,
                           int sampleRate, int channels, int bitsPerSample)
    : m_fmtType(fmtType)
    , m_data(pdata)
    , m_sampleRate(sampleRate)
    , m_channels(channels)
    , m_bitsPerSample(bitsPerSample) {

}

CSJAudioData::~CSJAudioData() {
    if (m_data) {
        delete m_data;
        m_data = nullptr;
    }
}

CSJAudioData::CSJAudioData(CSJAudioData && audioData)
    : m_fmtType(audioData.m_fmtType)
    , m_data(audioData.m_data)
    , m_sampleRate(audioData.m_sampleRate)
    , m_channels(audioData.m_channels)
    , m_bitsPerSample(audioData.m_bitsPerSample) {

    audioData.m_fmtType = CSJAUDIO_FMT_NONE;
    audioData.m_data = nullptr;
    audioData.m_sampleRate = 0;
    audioData.m_channels = 0;
    audioData.m_bitsPerSample = 0;
}

CSJAudioData& CSJAudioData::operator=(CSJAudioData && audioData) {
    m_fmtType = audioData.m_fmtType;
    m_data = audioData.m_data;
    m_sampleRate = audioData.m_sampleRate;
    m_channels = audioData.m_channels;
    m_bitsPerSample = audioData.m_bitsPerSample;

    audioData.m_fmtType = CSJAUDIO_FMT_NONE;
    audioData.m_data = nullptr;
    audioData.m_sampleRate = 0;
    audioData.m_channels = 0;
    audioData.m_bitsPerSample = 0;

    return *this;
}
