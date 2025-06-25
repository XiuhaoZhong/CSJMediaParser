#ifndef __CSJMEDIARAWDATA_H__
#define __CSJMEDIARAWDATA_H__

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdint.h>
#include <string>

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

class CSJVideoData {
public:
    CSJVideoData();

    CSJVideoData(CSJVideoFormatType fmtType, uint8_t* data, int width, int height);

    ~CSJVideoData();

    CSJVideoData(CSJVideoData && videoData);

    CSJVideoData& operator=(CSJVideoData && videoData);

    CSJVideoFormatType getFmtType() const;

    uint8_t* getData() const;

    uint8_t* getyuvY() const;
    uint8_t* getyuvU() const;
    uint8_t* getyuvV() const;

    uint8_t* getRGB24() const;

    int getWidth() const;

    int getHeight() const;

protected:
    void initWithYUV420();
    void initWithRGB24();

private:
    CSJVideoFormatType m_fmtType;
    uint8_t           *m_data;
    int                m_width;
    int                m_height;

    uint8_t           *m_dataY = nullptr;
    uint8_t           *m_dataU = nullptr;
    uint8_t           *m_dataV = nullptr;

    uint8_t           *m_dataRGB24 = nullptr;
};

class CSJAudioData {
public:
    CSJAudioData();

    CSJAudioData(CSJAudioFormatType fmtType, uint8_t* pdata,
                 int sampleRate, int channels, int bitsPerSample);

    ~CSJAudioData();

    CSJAudioData(CSJAudioData && audioData);

    CSJAudioData& operator=(CSJAudioData && audioData);

private:
    CSJAudioFormatType m_fmtType;
    uint8_t           *m_data;
    int                m_sampleRate;
    int                m_channels;
    int                m_bitsPerSample;
};

#endif // __CSJMEDIARAWDATA_H__
