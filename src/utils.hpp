#ifndef __QPUWRAPPER__UTILS_H__
#define __QPUWRAPPER__UTILS_H__

#include <stdint.h>

#include "MappedMemory.hpp"

namespace QPUWrapper
{
    void* mapPhysicalAddress(unsigned int address, size_t size);
    void  unmapPhysicalAddress(void* address, size_t size);

    template<typename T>
    MappedMemory<T> mapPhysicalAddress(unsigned int address, size_t size)
    {
        return MappedMemory(mapPhysicalAddress(address, size), size);
    }

    template<typename T>
    void unmapPhysicalAddress(MappedMemory<T> &&mappedMem)
    {
        unmapPhysicalAddress(mappedMem, mappedMem.getMappedBlockSize());
        mappedMem = {nullptr, 0};
    }
}

#endif