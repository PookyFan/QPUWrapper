#ifndef __QPUWRAPPER__UTILS_H__
#define __QPUWRAPPER__UTILS_H__

#include <stdint.h>

#include "MailboxRequest.hpp"
#include "MappedMemory.hpp"

namespace QPUWrapper
{
    constexpr size_t PAGE_SIZE = 4096;
    
    void* mapPhysicalAddress(unsigned int address, size_t size);
    void  unmapPhysicalAddress(void* address, size_t size);
    void  openMailbox();

    uint32_t* sendMailboxRequest(MailboxRequest &request);

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