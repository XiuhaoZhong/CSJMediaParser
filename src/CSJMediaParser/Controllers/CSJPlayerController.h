#ifndef __CSJPLAYERCONTROLLER_H__
#define __CSJPLAYERCONTROLLER_H__

#include <memory>
#include <string>
#include <functional>

#include "CSJRenderEngine/CSJVideoRenderer.h"

using csjrenderengine::CSJRenderDelegate;

using RenderCallbackFunc = std::function<void()>;

/**
 * @brief This class is a logical player controller, and it separates player kernel
 *        from UI. UI modules use this class as a player instance, and control player's
 *        logics such as init, start, pause, resume stop and so on.
 * 
 *        This class implements video render delegate and audio render delegate, so this 
 *        class is responsible for providing video and audio data that would be rendered,
 *        and notifying video and audio rendering states to UI.
 */
class CSJPlayerController : public CSJRenderDelegate {
public:

    /**
     * @brief Create a player controller instance.
     * @return player controller instance.
     */
    static std::unique_ptr<CSJPlayerController> createPlayerController();

    CSJPlayerController() = default;
    virtual ~CSJPlayerController();

    /**
     * @brief Initialize the player kernel.
     */
    virtual bool init() = 0;

    /**
     * @brief Set the media file which will be played.
     */
    virtual bool setPlayFile(std::string& playFile) = 0;

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

public: // CSJRenderDelegate interfaces.
    virtual CSJVideoFramePtr getNextVideoFrame() = 0;

    void beforeARenderingTick() override;
    void afterARenderingTick() override;

    void beforeRenderingStart() override;
    void afterRenderingStart() override;

    void beforeRenderingPause() override;
    void afterRenderingPause() override;

    void beforeRenderingResume() override;
    void afterRenderingResume() override;

    void beforeRenderingStop() override;
    void afterRenderingStop() override;

    void setBeforeARenderingTickFunc(RenderCallbackFunc func);
    void setAfterARenderingTickFunc(RenderCallbackFunc func);

    void setBeforeRenderingStartFunc(RenderCallbackFunc func);
    void setAfterRenderingStartFunc(RenderCallbackFunc func);

    void setBeforeRenderingPauseFunc(RenderCallbackFunc func);
    void setAfterRenderingPauseFunc(RenderCallbackFunc func);

    void setBeforeRenderingResumeFunc(RenderCallbackFunc func);
    void setAfterRenderingResumeFunc(RenderCallbackFunc func);

    void setBeforeRenderingStopFunc(RenderCallbackFunc func);
    void setAfterRenderingStopFunc(RenderCallbackFunc func);

private:
    RenderCallbackFunc m_funcBeforeARenderingTick  = nullptr;
    RenderCallbackFunc m_funcAfterARenderingTick   = nullptr;
    RenderCallbackFunc m_funcBeforeRenderingStart  = nullptr;
    RenderCallbackFunc m_funcAfterRenderingStart   = nullptr;
    RenderCallbackFunc m_funcBeforeRenderingPause  = nullptr;
    RenderCallbackFunc m_funcAfterRenderingPause   = nullptr;
    RenderCallbackFunc m_funcBeforeRenderingResume = nullptr;
    RenderCallbackFunc m_funcAfterRenderingResume  = nullptr;
    RenderCallbackFunc m_funcBeforeRenderingStop   = nullptr;
    RenderCallbackFunc m_funcAfterRenderingStop    = nullptr;
};

using CSJPlayerControllerPtr = std::unique_ptr<CSJPlayerController>;

using CSJPlayerControllerSharedPtr = std::shared_ptr<CSJPlayerController>;
CSJPlayerControllerSharedPtr createSharedPlayerController();

#endif // __CSJPLAYERCONTROLLER_H__
