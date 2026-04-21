#ifndef __CSJMEDIARAWDATA_H__
#define __CSJMEDIARAWDATA_H__

#include "CSJMediaEngine_Export.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdint.h>
#include <string>

namespace csjmediaengine {

typedef enum {
    CSJVIDEO_FMT_NONE = -1,
    CSJVIDEO_FMT_RGB  = 0,
    CSJVIDEO_FMT_RGB24,
    CSJVIDEO_FMT_NV12,
    CSJVIDEO_FMT_YV12,
    CSJVIDEO_FMT_YUV420P
} CSJVideoFormatType;

typedef enum {
    CSJAUDIO_FMT_NONE = -1,
    CSJAUDIO_FMT_PCM  = 0,
    CSJAUDIO_FMT_AAC
} CSJAudioFormatType;

#ifdef _WIN32
std::wstring SubTypeToString(GUID& subtype);
#endif

CSJVideoFormatType string2VideoType(std::wstring &fmtString);

class CSJMEDIAENGINE_API CSJVideoData {
public:
    CSJVideoData();

    CSJVideoData(CSJVideoFormatType fmtType, uint8_t* data, int width, int height);
    CSJVideoData(CSJVideoFormatType fmtType, int width, int height, double pts, 
                    double timeStamp, double duration, uint8_t *data);

    ~CSJVideoData();

    CSJVideoData(CSJVideoData && videoData);

    CSJVideoData& operator=(CSJVideoData && videoData);

    CSJVideoFormatType getFmtType() const;

    uint8_t* getData() const;

    uint8_t* getyuvY() const;
    uint8_t* getyuvU() const;
    uint8_t* getyuvV() const;

    int getWidth() const;
    int getHeight() const;
    double getDuration() const;
    double getPts() const;
    double getTimeStamp() const;

protected:
    void initWithYUV420(uint8_t* data);
    void initWithRGB24(uint8_t* data);

private:
    CSJVideoFormatType m_fmtType    = CSJVIDEO_FMT_NONE;
    uint8_t           *m_pData      = nullptr;
    int                m_iWidth     = 0;
    int                m_iHeight    = 0;

    double             m_dPts       = 0.0;
    double             m_dTimeStamp = 0.0;
    double             m_dDuration  = 0.0;
};

class CSJMEDIAENGINE_API CSJAudioData {
public:
    CSJAudioData();

    CSJAudioData(CSJAudioFormatType fmtType, uint8_t* pdata,
                 int sampleRate, int channels, int bitsPerSample);

    ~CSJAudioData();

    CSJAudioData(CSJAudioData && audioData);

    CSJAudioData& operator=(CSJAudioData && audioData);

private:
    CSJAudioFormatType m_fmtType        = CSJAUDIO_FMT_NONE;
    uint8_t           *m_pData          = nullptr;
    int                m_iSampleRate    = 0;
    int                m_iChannels      = 0;
    int                m_iBitsPerSample = 0;
};

} // namespace csjmediaengine

#endif // __CSJMEDIARAWDATA_H__
