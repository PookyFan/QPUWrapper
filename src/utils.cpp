#include <stdexcept>
#include <fcntl.h>

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "utils.hpp"

constexpr int MAJOR_NUM = 100;

uint32_t mailboxHandle;

namespace QPUWrapper
{
    void* mapPhysicalAddress(unsigned int physicalAddress, size_t size)
    {
        int offset = physicalAddress % PAGE_SIZE;
        int fd = open("/dev/mem", O_RDWR|O_SYNC);
        if(fd < 0)
            throw std::runtime_error("mapPhysicalAddress(): can't open memory device (probably not run as root) - error code was: " + std::to_string(errno));
        
        void *mappedAddress = mmap(0, size + offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, physicalAddress - offset);
        close(fd);
        if(mappedAddress == MAP_FAILED)
            throw std::runtime_error("mapPhysicalAddress(): can't map physical address to local process' address space");
        
        return reinterpret_cast<uint8_t*>(mappedAddress) + offset;
    }

    void unmapPhysicalAddress(unsigned int physicalAddress, void* mappedAddress, size_t size)
    {
        int offset = physicalAddress % PAGE_SIZE;
        munmap(reinterpret_cast<uint8_t*>(mappedAddress) - offset, size + offset);
    }

    void openMailbox()
    {
        mailboxHandle = open("/dev/vcio", 0);
        if(mailboxHandle < 0)
            throw std::runtime_error("openMailbox(): can't open mailbox device (probably not run as root) - error code was: " + std::to_string(errno));
    }

    uint32_t* sendMailboxRequest(MailboxRequest &request)
    {
        auto data = request.getRequestData();
        if(ioctl(mailboxHandle, _IOWR(MAJOR_NUM, 0, char *), data) < 0)
            throw std::runtime_error("sendMailboxRequest(): couldn't execute ioctl - error code was: " + std::to_string(errno));
        
        return &data[FIRST_ARG_INDEX]; //Response overwrites the request, and both have similar structure
    }
}