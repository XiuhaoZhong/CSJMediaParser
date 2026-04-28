#include "CSJMediaPlayerBase.h"

#include "CSJMediaPlayer.hpp"

namespace csjmediaengine {

CSJMediaPlayerBase::CSJMediaPlayerBase() {

}

CSJMEDIAENGINE_API CSJMediaPlayerBase* createPlayerCore() {
    return new CSJMediaPlayer();
}

} // namespace csjmediaengine