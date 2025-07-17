#include "CSJMediaSpaceInfoArea.h"

#include <QWidget>
#include <QHBoxLayout>

CSJMediaSpaceInfoArea::CSJMediaSpaceInfoArea()
    : m_pAreaWidget(nullptr) {

}

CSJMediaSpaceInfoArea::~CSJMediaSpaceInfoArea() {

}

QWidget *CSJMediaSpaceInfoArea::getAreaWidget() {
    return m_pAreaWidget;
}
