#ifndef __QPUWRAPPER__MAILBOX_REQUEST_H__
#define __QPUWRAPPER__MAILBOX_REQUEST_H__

#include <stddef.h>
#include <stdint.h>

namespace QPUWrapper
{
    constexpr size_t MAX_REQUEST_SIZE = 16;
    constexpr size_t FIRST_ARG_INDEX  = 5;

    enum class RequestType : uint32_t
    {
        MemoryAllocation   = 0x3000C,
        MemoryLock         = 0x3000D,
        MemoryUnlock       = 0x3000E,
        MemoryDeallocation = 0x3000F
    };

    class MailboxRequest
    {
        public:
            MailboxRequest(RequestType type) : currentArgsCount(0)
            {
                data[1] = 0; //"Process request" token
                data[2] = static_cast<uint32_t>(type);
            }

            MailboxRequest(const MailboxRequest&) = delete;
            MailboxRequest(MailboxRequest&&) = delete;

            MailboxRequest& operator=(const MailboxRequest&) = delete;
            MailboxRequest& operator=(MailboxRequest&&) = delete;

            MailboxRequest& operator<<(uint32_t arg)
            {
                data[FIRST_ARG_INDEX + currentArgsCount] = arg;
                ++currentArgsCount;
                return *this;
            }

            uint32_t* getRequestData()
            {
                auto tokensCount = FIRST_ARG_INDEX + currentArgsCount;
                data[tokensCount++] = 0; //End token
                data[0] = tokensCount * sizeof(*data);
                data[3] = data[4] = currentArgsCount * sizeof(*data); //Payload size tokens
                return data;
            }

        private:
            unsigned int currentArgsCount;
            uint32_t data[MAX_REQUEST_SIZE];
    };
}

#endif