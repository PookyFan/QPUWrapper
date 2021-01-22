#ifndef __QPUWRAPPER__QPU_PROGRAM_H__
#define __QPUWRAPPER__QPU_PROGRAM_H__

#include <stdexcept>
#include <string>

#include <stdint.h>

#include "GpuBuffer.hpp"

namespace QPUWrapper
{
    enum class ExecutionResult : uint8_t { Success, QpuBusy, Timeout };

    class QpuProgram
    {
        friend class Qpu;

        public:
            QpuProgram(int qpuInstancesToUse, int uniformsPerInstance, uint8_t *programCode, size_t programSize);
            ~QpuProgram();

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