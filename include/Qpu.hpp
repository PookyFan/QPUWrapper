#ifndef __QPUWRAPPER__QPU_H__
#define __QPUWRAPPER__QPU_H__

#include <chrono>
#include <mutex>
#include <future>
#include <tuple>

#include <stdint.h>

#include "GpuBuffer.hpp"
#include "MappedMemory.hpp"
#include "QpuProgram.hpp"

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
            static Qpu& getQpuWrapperInstance();

            /**
             * Execute program on QPU in a new thread
             * @param program QpuProgram object wrapping program to be executed
             * @param timeout time (in us) to wait for program to finish on all QPU instances before considering its execution failed
             * @return std::future object that allows to retrieve execution result of the program, and if it hasn't
             * finished yet - to wait on program's executing thread to finish and then retrieve the result of execution
             */
            std::future<ExecutionResult> executeProgram(QpuProgram &program, std::chrono::microseconds timeout);

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
                size_t bytesCount = size * sizeof(T);
                auto memoryData = allocateGpuMemory(bytesCount);
                return GpuBuffer<T>(std::get<0>(memoryData), std::get<1>(memoryData),
                                    MappedMemory<T>(reinterpret_cast<T*>(std::get<2>(memoryData)), bytesCount));
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
             * Locks block of shared CPU-GPU memory so it could be used on CPU side
             * Be aware that it must be unlocked before it can be used by QPU (or GPU in general) again
             * @tparam buffered data type
             * @param buffer GpuBuffer object wrapping allocated GPU memory
             * @return pointer to buffer's memory available on CPU side
             */
            template<typename T>
            T* lockGpuBuffer(GpuBuffer<T> &buffer)
            {
                lockGpuMemory(buffer.memoryHandle);
                return buffer.localAddress;
            }

            /**
             * Unlocks block of shared CPU-GPU memory so it could be used on GPU side
             * After this operation, the pointer returned by lockGpuBuffer() (called on the
             * same buffer object) is not usable anymore
             * @tparam T 
             * @param buffer 
             */
            template<typename T>
            void unlockGpuBuffer(GpuBuffer<T> &buffer)
            {
                unlockGpuMemory(buffer.memoryHandle);
            }

        private:
            Qpu(); //This is a singleton!

            void reserveQpus(int useCount);
            void unlockGpuMemory(uint32_t memoryHandle);
            void freeGpuMemory(uint32_t memoryHandle, void *mappedAddress, size_t size);
            GpuAddress lockGpuMemory(uint32_t memoryHandle);
            std::tuple<uint32_t, GpuAddress, void*> allocateGpuMemory(size_t size);
            ExecutionResult runProgram(QpuProgram &program, std::chrono::microseconds timeout);
            
            int                             qpuCount;
            uint32_t                        memAllocFlags;
            std::mutex                      programExecutionMutex;
            MappedMemory<volatile uint32_t> peripherals;

            static Qpu *instance;
    };
}

#endif