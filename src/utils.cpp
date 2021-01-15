#include <stdexcept>
#include <fcntl.h>

#include <unistd.h>
#include <sys/mman.h>

#include "utils.hpp"

constexpr size_t PAGE_SIZE = 4096;

namespace QPUWrapper
{
    void* mapPhysicalAddress(unsigned int address, size_t size)
    {
        int offset = address % PAGE_SIZE;
        int fd = open("/dev/mem", O_RDWR|O_SYNC);
        if(fd < 0)
            throw std::runtime_error("mapPhysicalAddress(): can't open memory device (probably not run as root)");
        
        void *localAddress = mmap(0, size + offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, address - offset);
        close(fd);
        if(localAddress == MAP_FAILED)
            throw std::runtime_error("mapPhysicalAddress(): can't map physical address to local process' address space");
        
        return localAddress;
    }

    void unmapPhysicalAddress(void* address, size_t size)
    {
        munmap(address, size);
    }
}