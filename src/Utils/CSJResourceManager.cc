#include "CSJResourceManager.h"

/* size of icon in the resource files. */
const static int icon_width = 50;
const static int icon_height = 50;

CSJResourceManager::CSJResourceManager() {

}

CSJResourceManager::~CSJResourceManager() {

}

QIcon CSJResourceManager::getIconById(CSJButtonID buttonId) {
    return QIcon();
}

void CSJResourceManager::releaseResources() {

}
