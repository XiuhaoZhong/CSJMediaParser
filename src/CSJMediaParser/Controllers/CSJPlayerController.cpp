#include "CSJPlayerController.h"

#include <qdebug.h>

#include "CSJMediaEngine/CSJMediaPlayerBase.h"

using namespace csjmediaengine;

class CSJPlayerControllerImpl : public CSJPlayerController {
public:
    CSJPlayerControllerImpl();
    ~CSJPlayerControllerImpl();

    bool initPlayerKernel(QString& playFile) override;

    void start() override;
    void pause() override;
    void resume() override;
    void stop() override;

    bool isPlaying() override;
    bool isPausing() override;
    bool isStopping() override;

private:
    std::unique_ptr<CSJMediaPlayerBase> m_pPlayerKernel;
};

CSJPlayerControllerImpl::CSJPlayerControllerImpl() {
    m_pPlayerKernel = std::move(CSJMediaPlayerBase::getPlayerKernel());
}

CSJPlayerControllerImpl::~CSJPlayerControllerImpl() {

}

bool CSJPlayerControllerImpl::initPlayerKernel(QString& filePath) {
    if (!m_pPlayerKernel) {
        return false;
    }

    if (!m_pPlayerKernel->initPlayer()) {
        return false;
    }

    qDebug() << "[" <<__FILE__  << ": " << __FUNCTIONW__ << "]" << " Player kernel has been created!";

    std::string play_file_path = filePath.toStdString();
    m_pPlayerKernel->setPlayFile(play_file_path);
    return true;
}

void CSJPlayerControllerImpl::start() {
    if (!m_pPlayerKernel) {
        qDebug() << "[" <<__FILE__ << ": " << __FUNCTIONW__ << "]" << " Player kernel hasn't been created!";
        return ;
    }

    m_pPlayerKernel->play();
}

void CSJPlayerControllerImpl::pause() {
    if (!m_pPlayerKernel) {
        qDebug() << "[" <<__FILE__ << ": " << __FUNCTIONW__ << "]" << " Player kernel hasn't been created!";
        return ;
    }

    m_pPlayerKernel->pause();
}

void CSJPlayerControllerImpl::resume() {
    if (!m_pPlayerKernel) {
        qDebug() << "[" <<__FILE__ << ": " << __FUNCTIONW__ << "]" << " Player kernel hasn't been created!";
        return ;
    }

    m_pPlayerKernel->resume();
}

void CSJPlayerControllerImpl::stop() {
    m_pPlayerKernel->stop();
}

bool CSJPlayerControllerImpl::isPlaying() {
    if (!m_pPlayerKernel) {
        qDebug() << "[" <<__FILE__ << ": " << __FUNCTIONW__ << "]" << " Player kernel hasn't been created!";
        return false;
    }

    return m_pPlayerKernel->isPlaying();
}

bool CSJPlayerControllerImpl::isPausing() {
    if (!m_pPlayerKernel) {
        qDebug() << "[" <<__FILE__ << ": " << __FUNCTIONW__ << "]" << " Player kernel hasn't been created!";
        return false;
    }

    return m_pPlayerKernel->isPause();
}

bool CSJPlayerControllerImpl::isStopping() {
    if (!m_pPlayerKernel) {
        qDebug() << "[" <<__FILE__ << ": " << __FUNCTIONW__ << "]" << " Player kernel hasn't been created!";
        return false;
    }

    return m_pPlayerKernel->isStop();
}

CSJUniqPlayerController CSJPlayerController::createPlayerController() {
    return std::make_unique<CSJPlayerControllerImpl>();
}

