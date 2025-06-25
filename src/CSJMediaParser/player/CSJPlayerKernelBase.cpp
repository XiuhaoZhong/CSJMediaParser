#include "CSJPlayerKernelBase.h"

#include "CSJFFPlayerKernel.h"

CSJUniquePlayer CSJPlayerKernelBase::getPlayerKernel(CSJPlayerType playerType) {
    switch (playerType) {
    case CSJPlAYERTYPE_DEFAULT:
        return std::move(std::make_unique<CSJFFPlayerKernel>());
    default:
        return nullptr;
    }
}
