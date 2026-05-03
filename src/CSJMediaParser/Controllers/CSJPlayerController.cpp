#include "CSJPlayerController.h"

#include <QString>

#include "CSJUtils/CSJLogger.h"

#include "CSJMediaEngine/CSJMediaPlayerBase.h"

using namespace csjmediaengine;

class CSJPlayerControllerImpl : public CSJPlayerController {
public:
    CSJPlayerControllerImpl();
    ~CSJPlayerControllerImpl();

    bool initPlayerKernel() override;

    bool setPlayFile(QString& playFile) override;

    void start() override;
    void pause() override;
    void resume() override;
    void stop() override;

    bool isPlaying() override;
    bool isPausing() override;
    bool isStopping() override;

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

bool CSJPlayerControllerImpl::initPlayerKernel() {
    if (!m_pPlayerKernel) {
        return false;
    }

    if (!m_pPlayerKernel->initPlayer()) {
        return false;
    }

    LOG_Info("Player kernel has been created!");

    return true;
}

bool CSJPlayerControllerImpl::setPlayFile(QString & playFile) {
    if (!m_pPlayerKernel) {
        LOG_Warn("Player kernel hasn't been initialized!");
        return false;
    }

    if (playFile.size() == 0) {
        LOG_Warn("Play file is null");
        return false;
    }

    std::string play_file_path = playFile.toStdString();
    m_pPlayerKernel->setPlayFile(play_file_path);

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

CSJPlayerControllerPtr CSJPlayerController::createPlayerController() {
    return std::make_unique<CSJPlayerControllerImpl>();
}

