#ifndef __CSJMEDIAPLAYERBASE_H__
#define __CSJMEDIAPLAYERBASE_H__

#include "CSJMediaEngine_Export.h"

#include <memory>
#include <string>

#include "CSJVideoPresentDelegate.h"

namespace csjmediaengine {

typedef enum {
    CSJPlAYERTYPE_DEFAULT = 0,
} CSJPlayerType;

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

    void setVideoPresentDelegate(std::shared_ptr<CSJVideoPresentDelegate> delegate) {
        m_pVideoPresentDelegate = delegate;
    }

protected:
    std::weak_ptr<CSJVideoPresentDelegate> m_pVideoPresentDelegate;
};

} // namespace csjmediaengine

#endif // __CSJMEDIAPLAYERBASE_H