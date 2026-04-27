#include "CSJPlayerKernelBase.h"

#include "CSJFFPlayerKernel.h"

CSJUniquePlayer CSJPlayerKernelBase::getPlayerKernel(CSJPlayerType playerType) {
    switch (playerType) {
    case CSJPlAYERTYPE_DEFAULT:
        return std::unique_ptr<CSJFFPlayerKernel>();
    default:
        return nullptr;
    }
}
