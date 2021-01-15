#ifndef __QPUWRAPPER__QPU_H__
#define __QPUWRAPPER__QPU_H__

#include <string>
#include <stdint.h>

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

        private:
            Qpu(); //This is a singleton!
            
            uint32_t mailboxHandle;
            uint32_t memAllocFlag;
            MappedMemory<volatile uint32_t> peripheralsAddress;

            static Qpu *instance;
    };
}

#endif