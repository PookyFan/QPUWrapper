#include <stdexcept>
#include <fcntl.h>

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "utils.hpp"

constexpr int    MAJOR_NUM = 100;

uint32_t mailboxHandle;

namespace QPUWrapper
{
    void* mapPhysicalAddress(unsigned int address, size_t size)
    {
        int offset = address % PAGE_SIZE;
        int fd = open("/dev/mem", O_RDWR|O_SYNC);
        if(fd < 0)
            throw std::runtime_error("mapPhysicalAddress(): can't open memory device (probably not run as root) - error code was: " + std::to_string(errno));
        
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