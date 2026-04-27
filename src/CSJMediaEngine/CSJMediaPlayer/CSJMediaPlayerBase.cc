#include "CSJMediaPlayerBase.h"

#include "CSJMediaPlayer.hpp"

namespace csjmediaengine {
CSJMediaPlayerBase::CSJMediaPlayerBase() {

}

CSJMediaPlayerBase::~CSJMediaPlayerBase() {

}

CSJMediaPlayerBase *CSJMediaPlayerBase::getPlayerInstance() {
    return nullptr;
}

std::unique_ptr<CSJMediaPlayerBase> CSJMediaPlayerBase::getPlayerKernel(CSJPlayerType playerType) {
    return std::make_unique<CSJMediaPlayer>();
}

} // namespace csjmediaengine