#ifndef __CSJPLAYERCONTROLLER_H__
#define __CSJPLAYERCONTROLLER_H__

#include <memory>

#include <QString>

/**
 * @brief This class is a logical player controller, and it separates player kernel
 *        from UI. UI modules use this class as a player instance, and control player's
 *        logics such as init, start, pause, resume stop and so on.
 */
class CSJPlayerController {
public:

    /**
     * @brief Create a player controller instance.
     * @return player controller instance.
     */
    static std::unique_ptr<CSJPlayerController> createPlayerController();

    CSJPlayerController() = default;
    virtual ~CSJPlayerController() {};

    /**
     * @brief Initialize the player kernel with a specific file.
     */
    virtual bool initPlayerKernel(QString& playFile) = 0;

    /**********************************************
     * Player Operations.
     *********************************************/
    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;

    /**********************************************
     * Player Status.
     *********************************************/
    virtual bool isPlaying() = 0;
    virtual bool isPausing() = 0;
    virtual bool isStopping() = 0;
};

using CSJUniqPlayerController = std::unique_ptr<CSJPlayerController>;

#endif // __CSJPLAYERCONTROLLER_H__
