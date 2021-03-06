#ifndef __QPUWRAPPER__MAPPED_MEMORY_H__
#define __QPUWRAPPER__MAPPED_MEMORY_H__

#include <type_traits>

#include <stddef.h>

namespace QPUWrapper
{
    /**
     * An internal class for memory management.
     * @tparam Data type of elements contained in the mapped memory chunk
     */
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
                other.localAddress = nullptr;
                other.mappedBlockSize = 0;
            }

            MappedMemory(const MappedMemory&) = delete;

            MappedMemory& operator=(MappedMemory &&other)
            {
                localAddress = other.localAddress;
                mappedBlockSize = other.mappedBlockSize;
                other.localAddress = nullptr;
                other.mappedBlockSize = 0;
                return *this;
            }   

            MappedMemory& operator=(const MappedMemory&) = delete;

            operator T*()
            {
                return localAddress;
            }

            operator bool()
            {
                return (localAddress != nullptr && mappedBlockSize != 0);
            }

            T& operator[](int index)
            {
                return localAddress[index];
            }

            void* getGenericDataPointer()
            {
                auto normalAddress = const_cast<typename std::remove_cv<T>::type*>(localAddress); //To cast away volatile and const
                return reinterpret_cast<void*>(normalAddress);
            }

            size_t getMappedBlockSize()
            {
                return mappedBlockSize;
            }

        private:
            T*     localAddress;
            size_t mappedBlockSize; //In bytes
    };
}

#endif