#ifndef __CSJMEDIAPLAYERBASE_H
#define __CSJMEDIAPLAYERBASE_H

#include "CSJMediaEngine_Export.h"

namespace CSJMEDIAENGINE {

class CSJMEDIAENGINE_API CSJMediaPlayerBase {
public:
    CSJMediaPlayerBase();
    virtual ~CSJMediaPlayerBase();

    static CSJMediaPlayerBase* getPlayerInstance();
};

} // namespace CSJMEDIAENGINE

#endif // __CSJMEDIAPLAYERBASE_H