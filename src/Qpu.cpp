#include <bcm_host.h>

#include "MailboxRequest.hpp"
#include "Qpu.hpp"
#include "utils.hpp"

constexpr uint32_t  MEM_FLAG_DIRECT   = 1 << 2;
constexpr uint32_t  MEM_FLAG_COHERENT = 2 << 2;
constexpr uint32_t  MEM_FLAG_NO_INIT  = 1 << 5;
constexpr uint32_t  PI1_SDRAM_ADDRESS = 0x40000000;
constexpr uint32_t  MEMORY_ALIGN      = QPUWrapper::PAGE_SIZE;

namespace QPUWrapper
{
    Qpu::Qpu()
    {
        //Initialize mailbox
        openMailbox();
        
        //Set proper memory allocation flag (apparently only RPI1 and Zero (W) can use cached memory)
        memAllocFlags = MEM_FLAG_DIRECT | MEM_FLAG_NO_INIT | (bcm_host_get_sdram_address() == PI1_SDRAM_ADDRESS) ? MEM_FLAG_COHERENT : 0;

        //Map peripherals address to process' address space
        unsigned int peripheralAddress = bcm_host_get_peripheral_address();
        size_t peripheralMemSize = bcm_host_get_peripheral_size();
        peripheralsAddress = MappedMemory(reinterpret_cast<volatile uint32_t*>(mapPhysicalAddress(peripheralAddress, peripheralMemSize)), peripheralMemSize);
    }

    Qpu::~Qpu()
    {
    }

    Qpu& Qpu::getQpuWrapperInstance()
    {
        if(instance == nullptr)
            instance = new Qpu();
        
        return *instance;
    }

    void Qpu::unlockGpuMemory(uint32_t memoryHandle)
    {
        MailboxRequest request(RequestType::MemoryUnlock);
        request << memoryHandle;
        sendMailboxRequest(request);
    }

    void Qpu::freeGpuMemory(uint32_t memoryHandle, void *mappedAddress, size_t size)
    {
        MailboxRequest request(RequestType::MemoryDeallocation);
        request << memoryHandle;
        unmapPhysicalAddress(mappedAddress, size);
        sendMailboxRequest(request);
    }

    GpuAddress Qpu::lockGpuMemory(uint32_t memoryHandle)
    {
        MailboxRequest request(RequestType::MemoryLock);
        request << memoryHandle;
        return *sendMailboxRequest(request);
    }

    std::tuple<uint32_t, GpuAddress, void*> Qpu::allocateGpuMemory(size_t size)
    {
        MailboxRequest request(RequestType::MemoryAllocation);
        request << size << MEMORY_ALIGN << memAllocFlags;

        uint32_t memoryHandle = *sendMailboxRequest(request);
        GpuAddress gpuAddress = lockGpuMemory(memoryHandle);
        unlockGpuMemory(memoryHandle);

        void *localAddress = mapPhysicalAddress(gpuAddress, size);
        return { memoryHandle, gpuAddress, localAddress };
    }

    /* Static field initialization */
    Qpu *Qpu::instance = nullptr;
}