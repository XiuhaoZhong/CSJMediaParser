#include "CSJPlayerController.h"

#include <QString>

#include "CSJUtils/CSJLogger.h"

#include "CSJMediaEngine/CSJMediaPlayerBase.h"

using namespace csjmediaengine;

CSJPlayerController::~CSJPlayerController() {
    m_funcBeforeARenderingTick  = nullptr;
    m_funcAfterARenderingTick   = nullptr;
    m_funcBeforeRenderingStart  = nullptr;
    m_funcAfterRenderingStart   = nullptr;
    m_funcBeforeRenderingPause  = nullptr;
    m_funcAfterRenderingPause   = nullptr;
    m_funcBeforeRenderingResume = nullptr;
    m_funcAfterRenderingResume  = nullptr;
    m_funcBeforeRenderingStop   = nullptr;
    m_funcAfterRenderingStop    = nullptr;
}

void CSJPlayerController::setBeforeARenderingTickFunc(RenderCallbackFunc func) {
    m_funcBeforeARenderingTick = std::move(func);
}

void CSJPlayerController::setAfterARenderingTickFunc(RenderCallbackFunc func) {
    m_funcAfterARenderingTick = std::move(func);
}

void CSJPlayerController::setBeforeRenderingStartFunc(RenderCallbackFunc func) {
    m_funcBeforeRenderingStart = std::move(func);
}

void CSJPlayerController::setAfterRenderingStartFunc(RenderCallbackFunc func) {
    m_funcAfterRenderingStart = std::move(func);
}

void CSJPlayerController::setBeforeRenderingPauseFunc(RenderCallbackFunc func) {
    m_funcBeforeRenderingPause = std::move(func);
}

void CSJPlayerController::setAfterRenderingPauseFunc(RenderCallbackFunc func) {
    m_funcAfterRenderingPause = std::move(func);
}

void CSJPlayerController::setBeforeRenderingResumeFunc(RenderCallbackFunc func) {
    m_funcBeforeRenderingResume = std::move(func);
}

void CSJPlayerController::setAfterRenderingResumeFunc(RenderCallbackFunc func) {
    m_funcAfterRenderingResume = std::move(func);
}

void CSJPlayerController::setBeforeRenderingStopFunc(RenderCallbackFunc func) {
    m_funcBeforeRenderingStop = std::move(func);
}

void CSJPlayerController::setAfterRenderingStopFunc(RenderCallbackFunc func) {
    m_funcAfterRenderingStop = std::move(func);
}

void CSJPlayerController::beforeARenderingTick() {
    if (m_funcBeforeARenderingTick) {
        m_funcBeforeARenderingTick();
    }
}

void CSJPlayerController::afterARenderingTick() {
    if (m_funcAfterARenderingTick) {
        m_funcAfterARenderingTick();
    }
}

void CSJPlayerController::beforeRenderingStart() {
    if (m_funcBeforeRenderingStart) {
        m_funcBeforeRenderingStart();
    }
}

void CSJPlayerController::afterRenderingStart() {
    if (m_funcAfterRenderingStart) {
        m_funcAfterRenderingStart();
    }
}

void CSJPlayerController::beforeRenderingPause() {
    if (m_funcBeforeRenderingPause) {
        m_funcBeforeRenderingPause();
    }
}

void CSJPlayerController::afterRenderingPause() {
    if (m_funcAfterRenderingPause) {
        m_funcAfterRenderingPause();
    }
}

void CSJPlayerController::beforeRenderingResume() {
    if (m_funcBeforeRenderingResume) {
        m_funcBeforeRenderingResume();
    }
}

void CSJPlayerController::afterRenderingResume() {
    if (m_funcAfterRenderingResume) {
        m_funcAfterRenderingResume();
    }
}

void CSJPlayerController::beforeRenderingStop() {
    if (m_funcBeforeRenderingStop) {
        m_funcBeforeRenderingStop();
    }
}

void CSJPlayerController::afterRenderingStop() {
    if (m_funcAfterRenderingStop) {
        m_funcAfterRenderingStop();
    }
}

class CSJPlayerControllerImpl : public CSJPlayerController {
public:
    CSJPlayerControllerImpl();
    ~CSJPlayerControllerImpl();

    bool init() override;

    bool setPlayFile(std::string& playFile) override;

    void start() override;
    void pause() override;
    void resume() override;
    void stop() override;

    bool isPlaying() override;
    bool isPausing() override;
    bool isStopping() override;

public: // CSJRenderDeleagte interfaces.
    CSJVideoFramePtr getNextVideoFrame() override;

private:
    CSJMediaPlayerPtr m_pPlayerKernel;
};

CSJPlayerControllerImpl::CSJPlayerControllerImpl() {
    m_pPlayerKernel = CSJMediaPlayerPtr(createPlayerCore());
    LOG_Info("Player controller instance construct");
}

CSJPlayerControllerImpl::~CSJPlayerControllerImpl() {
    LOG_Info("Player controller instance deconstruct");
}

bool CSJPlayerControllerImpl::init() {
    if (!m_pPlayerKernel) {
        return false;
    }

    if (!m_pPlayerKernel->initPlayer()) {
        return false;
    }

    LOG_Info("Player kernel initialize successfully!");

    return true;
}

bool CSJPlayerControllerImpl::setPlayFile(std::string & playFile) {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been initialized!");
        return false;
    }

    if (playFile.size() == 0) {
        LOG_Warn("Play file is null");
        return false;
    }

    m_pPlayerKernel->setPlayFile(playFile);

    return true;
}

void CSJPlayerControllerImpl::start() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return ;
    }

    m_pPlayerKernel->play();
}

void CSJPlayerControllerImpl::pause() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return ;
    }

    m_pPlayerKernel->pause();
}

void CSJPlayerControllerImpl::resume() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return ;
    }

    m_pPlayerKernel->resume();
}

void CSJPlayerControllerImpl::stop() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return ;
    }

    if (m_pPlayerKernel->isStop()) {
        return ;
    }

    m_pPlayerKernel->stop();
}

bool CSJPlayerControllerImpl::isPlaying() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return false;
    }

    return m_pPlayerKernel->isPlaying();
}

bool CSJPlayerControllerImpl::isPausing() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return false;
    }

    return m_pPlayerKernel->isPause();
}

bool CSJPlayerControllerImpl::isStopping() {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been created!");
        return false;
    }

    return m_pPlayerKernel->isStop();
}

 CSJVideoFramePtr CSJPlayerControllerImpl::getNextVideoFrame() {
    if (!m_pPlayerKernel) {
        return nullptr;
    }

    return m_pPlayerKernel->getNextVideoFrame();
 }

CSJPlayerControllerPtr CSJPlayerController::createPlayerController() {
    return std::make_unique<CSJPlayerControllerImpl>();
}

CSJPlayerControllerSharedPtr createSharedPlayerController() {
    return std::make_shared<CSJPlayerControllerImpl>();
}

