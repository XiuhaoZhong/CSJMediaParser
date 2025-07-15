#include "CSJMediaPlayerBase.h"

#include "CSJFFPlayerKernel.h"

namespace csjmediaengine {
CSJMediaPlayerBase::CSJMediaPlayerBase() {

}

CSJMediaPlayerBase::~CSJMediaPlayerBase() {

}

CSJMediaPlayerBase *CSJMediaPlayerBase::getPlayerInstance() {
    return nullptr;
}

std::unique_ptr<CSJMediaPlayerBase> CSJMediaPlayerBase::getPlayerKernel(CSJPlayerType playerType) {
    return std::make_unique<CSJFFPlayerKernel>();
}

} // namespace csjmediaengine