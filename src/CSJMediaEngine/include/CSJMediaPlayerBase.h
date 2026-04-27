#ifndef __CSJMEDIAPLAYERBASE_H__
#define __CSJMEDIAPLAYERBASE_H__

#include "CSJMediaEngine_Export.h"

#include <memory>
#include <string>

#include "CSJVideoPresentDelegate.h"

namespace csjmediaengine {

/**
 * Play mode
 */
enum class CSJPlayMode : uint8_t {
    CSJPlayMode_NONE  = 0,
    CSJPlayMode_Audio = 1 << 0,   /* Play audio only. */
    CSJPlayMode_Video = 1 << 1,   /* Play video onlu. */
    CSJPlayMode_AV    = 1 << 2,      /* Play audio and video. */
};

inline CSJPlayMode operator|(CSJPlayMode a, CSJPlayMode b) {
    return static_cast<CSJPlayMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline CSJPlayMode operator&(CSJPlayMode a, CSJPlayMode b) {
    return static_cast<CSJPlayMode>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline CSJPlayMode operator|=(CSJPlayMode a, CSJPlayMode b) {
    return a = a | b;
}

inline CSJPlayMode operator&=(CSJPlayMode a, CSJPlayMode b) {
    return a = a & b;
}

inline bool containVideoMode(CSJPlayMode a) {
    return (a & CSJPlayMode::CSJPlayMode_Video) == CSJPlayMode::CSJPlayMode_Video;
}

inline bool containAudioMode(CSJPlayMode a) {
    return (a & CSJPlayMode::CSJPlayMode_Audio) == CSJPlayMode::CSJPlayMode_Audio;
}

typedef enum {
    CSJPlAYERTYPE_DEFAULT = 0,
} CSJPlayerType;

typedef enum {
    CSJPLAYERSTATUS_STOP = 0,
    CSJPLAYERSTATUS_PLAYING,
    CSJPLAYERSTATUS_PAUSE
} CSJPlayerStatus;

class CSJMEDIAENGINE_API CSJMediaPlayerBase {
public:
    CSJMediaPlayerBase();
    virtual ~CSJMediaPlayerBase();

    static CSJMediaPlayerBase* getPlayerInstance();

    static std::unique_ptr<CSJMediaPlayerBase> getPlayerKernel(CSJPlayerType playerType = CSJPlAYERTYPE_DEFAULT);

    virtual void setPlayFile(std::string &file) = 0;
    virtual bool initPlayer() = 0;

    /**
     * @brief   Get the duration of the media file.
     * @return  the duration.
     */
    virtual int getDuration() = 0;

    /**********************************************
     * Player operations.
     *********************************************/
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual void seek(double timeStamp) = 0;

    /**********************************************
     * Player status.
     *********************************************/
    virtual bool isPlaying() = 0;
    virtual bool isPause() = 0;
    virtual bool isStop() = 0;

    virtual void setVideoPresentDelegate(std::shared_ptr<CSJVideoPresentDelegate> delegate) = 0;
};

} // namespace csjmediaengine

#endif // __CSJMEDIAPLAYERBASE_H