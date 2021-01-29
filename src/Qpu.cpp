#include <bcm_host.h>
#include <sched.h>

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
constexpr uint32_t V3D_IDENT1        = 0xC00004 / sizeof(uint32_t);
constexpr uint32_t V3D_L2CACTL       = 0xC00020 / sizeof(uint32_t);
constexpr uint32_t V3D_SLCACTL       = 0xC00024 / sizeof(uint32_t);
constexpr uint32_t V3D_SQRSV0        = 0xC00410 / sizeof(uint32_t);
constexpr uint32_t V3D_SQRSV1        = 0xC00414 / sizeof(uint32_t);
constexpr uint32_t V3D_SRQPC	     = 0xC00430 / sizeof(uint32_t);
constexpr uint32_t V3D_SRQUA	     = 0xC00434 / sizeof(uint32_t);
constexpr uint32_t V3D_SRQUL	     = 0xC00438 / sizeof(uint32_t);
constexpr uint32_t V3D_SRQCS	     = 0xC0043C / sizeof(uint32_t);
constexpr uint32_t V3D_VPMBASE       = 0xC00504 / sizeof(uint32_t);
constexpr uint32_t V3D_DBCFG         = 0xC00E00 / sizeof(uint32_t); //This isn't documented, but seems to disallow IRQ on QPUs when set to 0
constexpr uint32_t V3D_DBQITE	     = 0xC00E2C / sizeof(uint32_t);
constexpr uint32_t V3D_DBQITC	     = 0xC00E30 / sizeof(uint32_t);

//Other addresses
constexpr uint32_t PI1_SDRAM_ADDRESS = 0x40000000;

//Other constants
constexpr uint32_t MEMORY_ALIGN      = QPUWrapper::PAGE_SIZE;
constexpr uint32_t ENABLE_QPU        = 1;
constexpr uint32_t DISABLE_QPU       = 0;
constexpr uint32_t RESERVE_ENABLED   = 0xE;
constexpr uint32_t RESERVE_DISABLED  = 0xF;

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
        memAllocFlags = MEM_FLAG_DIRECT | MEM_FLAG_NO_INIT | (bcm_host_get_sdram_address() == PI1_SDRAM_ADDRESS ? MEM_FLAG_COHERENT : 0);

        //Map peripherals address to process' address space
        unsigned int peripheralAddress = bcm_host_get_peripheral_address();
        size_t peripheralMemSize = bcm_host_get_peripheral_size();
        peripherals = MappedMemory(reinterpret_cast<volatile uint32_t*>(mapPhysicalAddress(peripheralAddress, peripheralMemSize)), peripheralMemSize);

        //Get real number of QPU instances
        uint32_t ident1 = peripherals[V3D_IDENT1];
        int slicesCount = (ident1 >> 4) & 0xF;
        int qpuInstancesPerSlice = (ident1 >> 8) & 0xF;
        qpuCount = slicesCount * qpuInstancesPerSlice;

        //Reserve all available QPU instances (for now disable executing programs on them)
        reserveQpus(0);

        //Give QPU programs maximum ammount of VPM memory (in multiples of quadruple 32-bit 16-way vectors)
        peripherals[V3D_VPMBASE] = 16;

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
        unmapPhysicalAddress(bcm_host_get_peripheral_address(), peripherals.getGenericDataPointer(), peripherals.getMappedBlockSize());
    }

    Qpu& Qpu::getQpuWrapperInstance()
    {
        if(instance == nullptr)
            instance = new Qpu();
        
        return *instance;
    }

    std::future<ExecutionResult> Qpu::executeProgram(QpuProgram &program, std::chrono::microseconds timeout)
    {
        program.prepareToExecute();
        return std::async(std::launch::async, &Qpu::runProgram, this, std::ref(program), timeout);
    }

    void Qpu::reserveQpus(int useCount)
    {
        uint32_t reservationConfig = 0;
        for(int i = 0; i <= 7 && i < qpuCount; ++i)
            reservationConfig |= ((useCount > i ? RESERVE_ENABLED : RESERVE_DISABLED) << (i * 4));
        peripherals[V3D_SQRSV0] = reservationConfig;

        reservationConfig = 0;
        for(int i = 8; i < qpuCount; ++i)
            reservationConfig |= ((useCount > i ? RESERVE_ENABLED : RESERVE_DISABLED) << ((i - 8) * 4));
        peripherals[V3D_SQRSV1] = reservationConfig;
    }

    void Qpu::unlockGpuMemory(uint32_t memoryHandle)
    {
        MailboxRequest request(RequestType::MemoryUnlock);
        request << memoryHandle;
        sendMailboxRequest(request);
    }

    void Qpu::freeGpuMemory(uint32_t memoryHandle, uint32_t physicalAddress, void *mappedAddress, size_t size)
    {
        MailboxRequest request(RequestType::MemoryDeallocation);
        request << memoryHandle;
        unmapPhysicalAddress(physicalAddress, mappedAddress, size);
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

    ExecutionResult Qpu::runProgram(QpuProgram &program, std::chrono::microseconds timeout)
    {
        std::lock_guard lck(programExecutionMutex);

        //Check how many programs are queued at the moment and how many have already finished
        int queued = peripherals[V3D_SRQCS] & 0x3F;
        int finished = (peripherals[V3D_SRQCS] >> 16) & 0xFF;
        if(int freeQpus = qpuCount - queued; freeQpus < program.instancesCount)
            return ExecutionResult::QpuBusy;
        else
            reserveQpus(program.instancesCount);

        //Do not use interrupts
        peripherals[V3D_DBCFG] = 0;
        peripherals[V3D_DBQITE] = 0;
        peripherals[V3D_DBQITC] = 0xFFFF;

        //Clear all caches
        peripherals[V3D_L2CACTL] = (1<<2);
	    peripherals[V3D_SLCACTL] = 0b1111<<24 | 0b1111<<16 | 0b1111<<8 | 0b1111<<0;

        //Enqueue program on QPU instances
        program.isProgramExecuted = true;
        for(int i = 0; i < program.instancesCount; ++i)
        {
            peripherals[V3D_SRQUA] = program.programMemory.gpuAddress
                                     + program.codeSectionSize
                                     + i * program.uniformsPerInstanceCount * sizeof(uint32_t);
            peripherals[V3D_SRQUL] = program.uniformsPerInstanceCount;
            peripherals[V3D_SRQPC] = program.programMemory.gpuAddress;
        }

        auto timeStart = std::chrono::steady_clock::now();
        int finishedTarget = (finished + program.instancesCount) % 256; //The counter rolls back to 0 when it counts to 256
        do
        {
            sched_yield(); //Give it some time while not blocking other threads
            if(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count() > timeout.count())
                return ExecutionResult::Timeout;
            
            finished = (peripherals[V3D_SRQCS] >> 16) & 0xFF;
        } while(finished != finishedTarget);

        reserveQpus(0);
        program.isProgramExecuted = false;
        return ExecutionResult::Success;
    }

    /* Static field initialization */
    Qpu *Qpu::instance = nullptr;
}