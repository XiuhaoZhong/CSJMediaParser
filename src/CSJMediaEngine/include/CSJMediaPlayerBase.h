#ifndef __CSJMEDIAPLAYERBASE_H__
#define __CSJMEDIAPLAYERBASE_H__

#include "CSJMediaEngine_Export.h"

namespace CSJMediaEngine {

class CSJMEDIAENGINE_API CSJMediaPlayerBase {
public:
    CSJMediaPlayerBase();
    virtual ~CSJMediaPlayerBase();

    static CSJMediaPlayerBase* getPlayerInstance();
};

} // namespace CSJMediaEngine

#endif // __CSJMEDIAPLAYERBASE_H