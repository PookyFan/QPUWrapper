#include <algorithm>

#include "QpuProgram.hpp"
#include "Qpu.hpp"

namespace QPUWrapper
{
    QpuProgram::QpuProgram(int qpuInstancesToUse, int uniformsPerInstance, uint8_t *programCode, size_t programSize)
        : isProgramMemoryAccessed(true),
          isProgramExecuted(false),
          instancesCount(qpuInstancesToUse),
          uniformsPerInstanceCount(uniformsPerInstance),
          codeSectionSize(programSize),
          uniformsMemory(nullptr),
          programMemory(Qpu::getQpuWrapperInstance().allocateGpuBuffer<uint8_t>(programSize + instancesCount * uniformsPerInstance * sizeof(uint32_t)))
    {
        uint8_t *codeMemory = Qpu::getQpuWrapperInstance().lockGpuBuffer(programMemory);
        std::copy(programCode, programCode + programSize, codeMemory);
        uniformsMemory = reinterpret_cast<uint32_t*>(codeMemory + programSize);
    }

    QpuProgram::~QpuProgram()
    {
        if(isProgramExecuted)
            return; //There is no good solution for this situation - we either leak resources, or hang application execution...
        else if(isProgramMemoryAccessed)
            Qpu::getQpuWrapperInstance().unlockGpuBuffer(programMemory);
        
        Qpu::getQpuWrapperInstance().freeGpuBuffer(programMemory);
    }

    void QpuProgram::prepareToModify()
    {
        if(!isProgramMemoryAccessed)
        {
            Qpu::getQpuWrapperInstance().lockGpuBuffer(programMemory);
            isProgramMemoryAccessed = true;
        }
    }

    void QpuProgram::prepareToExecute()
    {
        if(isProgramMemoryAccessed)
        {
            Qpu::getQpuWrapperInstance().unlockGpuBuffer(programMemory);
            isProgramMemoryAccessed = false;
        }
    }
}