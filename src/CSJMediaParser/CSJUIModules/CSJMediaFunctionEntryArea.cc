#include "CSJMediaFunctionEntryArea.h"

CSJMediaFunctionEntryArea::CSJMediaFunctionEntryArea()
    : m_pAreaWidget(nullptr) {

}

CSJMediaFunctionEntryArea::~CSJMediaFunctionEntryArea() {

}

QWidget *CSJMediaFunctionEntryArea::getAreaWidget() {
    return m_pAreaWidget;
}