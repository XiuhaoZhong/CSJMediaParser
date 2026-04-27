#ifndef __CSJPLAYERKERNELBASE_H__
#define __CSJPLAYERKERNELBASE_H__

#include "CSJMpegHeader.h"

#include <QString>

#include "CommonTools/CSJMediaData.h"
#include "CSJVideoPresentDelegate.h"

typedef enum {
    CSJPlAYERTYPE_DEFAULT = 0,
} CSJPlayerType;

/**
 * @brief The CSJPlayerKernelBase class defines the common functions
 *        for player, its subclasses is the player kernel in the
 *        CSJPlayer class.
 */
class CSJPlayerKernelBase {
public:
    CSJPlayerKernelBase() {}
    virtual ~CSJPlayerKernelBase() {}

    static std::unique_ptr<CSJPlayerKernelBase> getPlayerKernel(CSJPlayerType playerType = CSJPlAYERTYPE_DEFAULT);

    virtual void setPlayFile(QString &file) = 0;
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

using CSJUniquePlayer = std::unique_ptr<CSJPlayerKernelBase>;

#endif // __CSJPLAYERKERNELBASE_H__
