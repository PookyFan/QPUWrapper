#include <fcntl.h>
#include <stdexcept>

#include <unistd.h>
#include <bcm_host.h>

#include "Qpu.hpp"
#include "utils.hpp"

constexpr uint32_t  MEM_FLAG_DIRECT   = 1 << 2;
constexpr uint32_t  MEM_FLAG_COHERENT = 2 << 2;
constexpr uint32_t  PI1_SDRAM_ADDRESS = 0x40000000;

namespace QPUWrapper
{
    Qpu::Qpu()
    {
        //Open mailbox
        mailboxHandle = open("/dev/vcio", 0);
        if(mailboxHandle < 0)
            throw std::runtime_error("Qpu::Qpu(): can't open mailbox device (probably not run as root)");
        
        //Set proper memory allocation flag (apparently only RPI1 and Zero (W) can use cached memory)
        memAllocFlag = MEM_FLAG_DIRECT | (bcm_host_get_sdram_address() == PI1_SDRAM_ADDRESS) ? MEM_FLAG_COHERENT : 0;

        //Map peripherals address to process' address space
        unsigned int peripheralAddress = bcm_host_get_peripheral_address();
        size_t peripheralMemSize = bcm_host_get_peripheral_size();
        peripheralsAddress = MappedMemory(reinterpret_cast<volatile uint32_t*>(mapPhysicalAddress(peripheralAddress, peripheralMemSize)), peripheralMemSize);
    }

    Qpu::~Qpu()
    {
        close(mailboxHandle);
    }

    Qpu& Qpu::getQpuWrapperInstance()
    {
        if(instance == nullptr)
            instance = new Qpu();
        
        return *instance;
    }

    /* Static field initialization */
    Qpu *Qpu::instance = nullptr;
}