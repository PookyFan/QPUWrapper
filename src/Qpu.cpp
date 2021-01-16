#include <bcm_host.h>

#include "MailboxRequest.hpp"
#include "Qpu.hpp"
#include "utils.hpp"

//Memory allocation flags
constexpr uint32_t MEM_FLAG_DIRECT   = 1 << 2;
constexpr uint32_t MEM_FLAG_COHERENT = 2 << 2;
constexpr uint32_t MEM_FLAG_NO_INIT  = 1 << 5;

//VideoCore registers addresses (descriptions available in official VideoCore4 documentation at https://docs.broadcom.com/docs/12358545 )
//Actually these are indices for accessing 32-bit registers as table of uint32_t elements, thus address is always divided by 4
//The addresses have offset of 0xC00000 probably to access registers in no-cache mode (not sure about that, though)
constexpr uint32_t V3D_IDENT1        = 0xC00004 >> 2;
constexpr uint32_t V3D_SQRSV0        = 0xC00410 >> 2;
constexpr uint32_t V3D_SQRSV1        = 0xC00414 >> 2;
constexpr uint32_t V3D_VPMBASE       = 0xC00504 >> 2;

//Other addresses
constexpr uint32_t PI1_SDRAM_ADDRESS = 0x40000000;

//Other constants
constexpr uint32_t MEMORY_ALIGN      = QPUWrapper::PAGE_SIZE;
constexpr uint32_t ENABLE_QPU        = 1;
constexpr uint32_t DISABLE_QPU       = 0;

namespace QPUWrapper
{
    uint32_t busAddressToPhysicalAddress(uint32_t busAddr)
    {
        return busAddr & ~0xC0000000;
    }

    Qpu::Qpu()
    {
        //Initialize mailbox
        openMailbox();

        //Enable QPU
        MailboxRequest request(RequestType::SetQpuState);
        request << ENABLE_QPU;
        if(*sendMailboxRequest(request) != 0)
            throw std::runtime_error("Qpu::Qpu(): could not enable QPU");
        
        //Set proper memory allocation flag (apparently only RPI1 and Zero (W) can use cached memory)
        memAllocFlags = MEM_FLAG_DIRECT | MEM_FLAG_NO_INIT | (bcm_host_get_sdram_address() == PI1_SDRAM_ADDRESS) ? MEM_FLAG_COHERENT : 0;

        //Map peripherals address to process' address space
        unsigned int peripheralAddress = bcm_host_get_peripheral_address();
        size_t peripheralMemSize = bcm_host_get_peripheral_size();
        peripherals = MappedMemory(reinterpret_cast<volatile uint32_t*>(mapPhysicalAddress(peripheralAddress, peripheralMemSize)), peripheralMemSize);

        //Get real number of QPU instances
        uint32_t ident1 = peripherals[V3D_IDENT1];
        int slicesCount = (ident1 >> 4) & 0xF;
        int qpuInstancesPerSlice = (ident1 >> 8) & 0xF;
        qpuCount = slicesCount * qpuInstancesPerSlice;

        //Give QPU programs maximum ammount of VPM memory (in multiples of quadruple 32-bit 16-way vectors)
        peripherals[V3D_VPMBASE] = 16;

        //Reserve all available QPU instances (for now disable executing programs on them)
        reserveQpus(0);
    }

    Qpu::~Qpu()
    {
        //Free all QPU instances
        peripherals[V3D_SQRSV0] = 0;
        peripherals[V3D_SQRSV1] = 0;

        //Disable QPU
        MailboxRequest request(RequestType::SetQpuState);
        request << DISABLE_QPU;
        sendMailboxRequest(request);

        //Unmap perihperals address
        unmapPhysicalAddress(peripherals);
    }

    Qpu& Qpu::getQpuWrapperInstance()
    {
        if(instance == nullptr)
            instance = new Qpu();
        
        return *instance;
    }

    void Qpu::reserveQpus(int useCount)
    {
        for(int i = 0; i <= 7 && i < qpuCount; ++i)
            peripherals[V3D_SQRSV0] |= ((useCount > i ? 0xE : 0xF) << (i * 4));
        for(int i = 8; i < qpuCount; ++i)
            peripherals[V3D_SQRSV1] |= ((useCount > i ? 0xE : 0xF) << (i * 4));
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

        void *localAddress = mapPhysicalAddress(busAddressToPhysicalAddress(gpuAddress), size);
        return { memoryHandle, gpuAddress, localAddress };
    }

    /* Static field initialization */
    Qpu *Qpu::instance = nullptr;
}