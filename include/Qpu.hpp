#ifndef __QPUWRAPPER__QPU_H__
#define __QPUWRAPPER__QPU_H__

#include <functional>
#include <string>
#include <tuple>

#include <stdint.h>

#include "GpuBuffer.hpp"
#include "MappedMemory.hpp"

namespace QPUWrapper
{
    /**
     * This class singleton instance is a main interface
     * that enables using QPU. It allows to allocate memory
     * on GPU's part of RAM, load and execute programs on
     * QPU cores and get basic performance counters.
     */
    class Qpu
    {
        public:
            ~Qpu();

            /**
             * Initializes QPU wrapper (if it's not already initialized) and returns a reference to the singleton
             * @return Wrapper's singleton instance reference
             */
            Qpu& getQpuWrapperInstance();

            /**
             * Allocates a block of memory in CPU-GPU shared memory space
             * 
             * @tparam buffered data type
             * @param size number of elements in allocated buffer
             * @return GpuBuffer object wrapping allocated GPU memory
             */
            template<typename T>
            GpuBuffer<T> allocateGpuBuffer(size_t size)
            {
                auto memoryData = allocateGpuMemory(size * sizeof(T));
                return GpuBuffer<T>(std::get<0>(memoryData), std::get<1>(memoryData), std::get<2>(memoryData));
            }

            /**
             * Frees previously allocated memory buffer from CPU-GPU shared memory
             * @tparam buffered data type
             * @param buffer GpuBuffer object wrapping allocated GPU memory
             */
            template<typename T>
            void freeGpuBuffer(GpuBuffer<T> &buffer)
            {
                freeGpuMemory(buffer.memoryHandle, buffer.localAddress, buffer.localAddress.getMappedBlockSize());
            }

            /**
             * Allows to use memory buffer from CPU-GPU shared memory on CPU side
             * by executing provided function or lambda on the pointer to that memory
             * @tparam buffered data type
             * @param buffer GpuBuffer object wrapping allocated GPU memory
             * @param operation function that accepts pointer of buffered data type
             */
            template<typename T>
            void useGpuBuffer(GpuBuffer<T> &buffer, std::function<void(T*)> operation)
            {
                lockGpuMemory(buffer.memoryHandle);
                try { operation(buffer.localAddress); }
                catch(...)
                {
                    unlockGpuMemory(buffer.memoryHandle);
                    throw;
                }
                unlockGpuMemory(buffer.memoryHandle);
            }

        private:
            Qpu(); //This is a singleton!

            void reserveQpus(int useCount);
            void unlockGpuMemory(uint32_t memoryHandle);
            void freeGpuMemory(uint32_t memoryHandle, void *mappedAddress, size_t size);
            GpuAddress lockGpuMemory(uint32_t memoryHandle);
            std::tuple<uint32_t, GpuAddress, void*> allocateGpuMemory(size_t size);
            
            int qpuCount;
            uint32_t memAllocFlags;
            MappedMemory<volatile uint32_t> peripherals;

            static Qpu *instance;
    };
}

#endif