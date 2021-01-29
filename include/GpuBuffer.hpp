#ifndef __QPUWRAPPER__GPU_BUFFER_H__
#define __QPUWRAPPER__GPU_BUFFER_H__

#include <utility>

#include "MappedMemory.hpp"

namespace QPUWrapper
{
    typedef uint32_t GpuAddress;

    template<typename T>
    class GpuBuffer
    {
        friend class Qpu;

        public:
            GpuBuffer(GpuBuffer &&other) : memoryHandle(other.memoryHandle), gpuAddress(other.gpuAddress), localAddress(std::move(localAddress))
            {
                other.memoryHandle = 0;
                other.gpuAddress = 0;
                other.localAddress = { nullptr, 0 };
            }

            GpuBuffer(const GpuBuffer&) = delete;

            GpuBuffer& operator=(GpuBuffer &&other)
            {
                memoryHandle = other.memoryHandle;
                gpuAddress = other.gpuAddress;
                localAddress = std::move(other.localAddress);

                other.memoryHandle = 0;
                other.gpuAddress = 0;
                other.localAddress = { nullptr, 0 };
                return *this;
            }

            GpuBuffer& operator=(const GpuBuffer&) = delete;

            operator bool()
            {
                return (memoryHandle != 0 && gpuAddress != 0 && localAddress);
            }

            /**
             * Get size of the buffer in bytes
             * @return size of the buffer (in bytes)
             */
            size_t getSize()
            {
                return localAddress.getMappedBlockSize();
            }

            /**
             * Get number of elements that fit in the buffer
             * @return number of elements of type specified as buffer object's template argument
             */
            size_t getCount()
            {
                return getSize() / sizeof(T);
            }

            GpuAddress getGpuAddress()
            {
                return gpuAddress;
            }

        private:
            GpuBuffer(uint32_t handle, uint32_t gpuAddr, MappedMemory<T> &&mappedAddr)
                : memoryHandle(handle), gpuAddress(gpuAddr), localAddress(std::move(mappedAddr))
            {
            }

            uint32_t        memoryHandle;
            GpuAddress      gpuAddress;
            MappedMemory<T> localAddress;
    };
}

#endif