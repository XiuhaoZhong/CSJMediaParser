#ifndef __CSJRESOURCEMANAGER_H__
#define __CSJRESOURCEMANAGER_H__

#include <QIcon>

#include "CSJResourceIdentifiers.h"

class CSJResourceManager {
public:
    CSJResourceManager();
    ~CSJResourceManager();

    /**
     * @brief Get the icon with button id.
     * 
     * @param   buttonId, id of button.
     * 
     * @return  the icon of the button.
     */
    QIcon getIconById(CSJButtonID buttonId);

protected:
    void releaseResources();

};

#endif // __CSJRESOURCEMANAGER_H__