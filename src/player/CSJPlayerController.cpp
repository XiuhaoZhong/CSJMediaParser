#include "CSJPlayerController.h"

#include "CSJPlayerKernelBase.h"

class CSJPlayerControllerImpl : public CSJPlayerController {
public:
    CSJPlayerControllerImpl();
    ~CSJPlayerControllerImpl();

    bool initPlayerKernel(QString& playFile) override;

    void start() override;
    void pause() override;
    void resume() override;
    void stop() override;

private:
    std::unique_ptr<CSJPlayerKernelBase> m_pPlayerKernel;
};

CSJPlayerControllerImpl::CSJPlayerControllerImpl() {
    m_pPlayerKernel = CSJPlayerKernelBase::getPlayerKernel();
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

    m_pPlayerKernel->setPlayFile(filePath);
    return true;
}

void CSJPlayerControllerImpl::start() {

}

void CSJPlayerControllerImpl::pause() {

}

void CSJPlayerControllerImpl::resume() {

}

void CSJPlayerControllerImpl::stop() {

}

CSJUniqPlayerController CSJPlayerController::createPlayerController() {
    return std::make_unique<CSJPlayerControllerImpl>();
}

