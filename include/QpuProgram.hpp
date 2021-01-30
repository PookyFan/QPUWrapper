#ifndef __QPUWRAPPER__QPU_PROGRAM_H__
#define __QPUWRAPPER__QPU_PROGRAM_H__

#include <stdexcept>
#include <string>

#include <stdint.h>

#include "GpuBuffer.hpp"

namespace QPUWrapper
{
    enum class ExecutionResult : uint8_t { Success, QpuBusy, Timeout };

    /**
     * Wrapper for user's program that is to be run on QPU instance(s).
     * Its interface allows only to create new program and manage its uniforms.
     * To run it, use the appropriate method of Qpu class instance.
     */
    class QpuProgram
    {
        friend class Qpu;

        public:
            QpuProgram(int qpuInstancesToUse, int uniformsPerInstance, uint8_t *programCode, size_t programSize);
            ~QpuProgram();

            /**
             * Allows to manipulate uniforms of specified program instance.
             * @param index Instance's index
             * @return Starting address of program instance's uniforms
             * (you can use second [] operator to access specific uniform)
             */
            uint32_t* operator[](int index)
            {
                if(isProgramExecuted)
                    return nullptr;
                
                prepareToModify();
                if(index < 0 || index >= instancesCount)
                    throw std::runtime_error("QpuProgram::operator[](): invalid instance index " + std::to_string(index));

                return uniformsMemory + index * uniformsPerInstanceCount;
            }

        private:
            void prepareToModify();
            void prepareToExecute();

            bool               isProgramMemoryAccessed;
            bool               isProgramExecuted;
            int                instancesCount;
            int                uniformsPerInstanceCount;
            size_t             codeSectionSize;
            uint32_t          *uniformsMemory;
            GpuBuffer<uint8_t> programMemory;
    };
}

#endif