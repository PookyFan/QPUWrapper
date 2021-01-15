#ifndef __QPUWRAPPER__MAPPED_MEMORY_H__
#define __QPUWRAPPER__MAPPED_MEMORY_H__

#include <stddef.h>

namespace QPUWrapper
{
    template<typename T>
    class MappedMemory
    {
        public:
            MappedMemory() : localAddress(nullptr), mappedBlockSize(0)
            {
            }

            MappedMemory(T *address, size_t size) : localAddress(address), mappedBlockSize(size)
            {
            }

            MappedMemory(MappedMemory &&other) : localAddress(other.localAddress), mappedBlockSize(other.mappedBlockSize)
            {
            }

            MappedMemory& operator=(MappedMemory &&other)
            {
                localAddress = other.localAddress;
                mappedBlockSize = other.mappedBlockSize;
                return *this;
            }

            MappedMemory(const MappedMemory&) = delete;

            MappedMemory& operator=(const MappedMemory&) = delete;

            operator T*()
            {
                return localAddress;
            }

            size_t getMappedBlockSize()
            {
                return mappedBlockSize;
            }

        private:
            T* localAddress;
            size_t mappedBlockSize; //In bytes
    };
}

#endif