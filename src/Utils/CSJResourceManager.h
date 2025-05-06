#ifndef __CSJRESOURCEMANAGER_H__
#define __CSJRESOURCEMANAGER_H__

#include <QtGui/QIcon>

class CSJResourceManager {
public:
    CSJResourceManager();
    ~CSJResourceManager();

    QIcon getIconById(int buttonId);


};

#endif // __CSJRESOURCEMANAGER_H__