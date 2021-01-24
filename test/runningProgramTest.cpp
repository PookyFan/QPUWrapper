#include <iostream>

#include <bcm_host.h>

#include "Qpu.hpp"

using namespace std::chrono_literals;

constexpr int testNumber = 17;
constexpr int testAddedNumber = 1;

uint32_t uniformPassthroughBin[] = {
    0x15827d80, 0x10020067,
    0x15827d80, 0x100200a7,
    0x00000a00, 0xe0021c67,
    0x15067d80, 0x10020c27,
    0x009f2000, 0x100009e7,
    0x80904000, 0xe0021c67,
    0xc0000000, 0xe0021c67,
    0x150a7d80, 0x10021ca7,
    0x009f2000, 0x100009e7,
    0x159e7d80, 0x100229a7,
    0x009e7000, 0x300009e7,
    0x009e7000, 0x100009e7,
    0x009e7000, 0x100009e7
};

uint32_t uniformsAddition[] = {
    0x15827d80, 0x10020067,
    0x15827d80, 0x10021067,
    0x15827d80, 0x100200a7,
    0x15827d80, 0x100210a7,
    0x00000a00, 0xe0020827,
    0x0c9e61c0, 0x10021c67,
    0x0c041dc0, 0x10020c27,
    0x009f2000, 0x100009e7,
    0x00000000, 0xe0020827,
    0x0c9e61c0, 0x10022867,
    0x00000058, 0xf03809e7,
    0x00000000, 0xe80009e7,
    0x00000017, 0xe00208a7,
    0x119c2e80, 0x10020867,
    0x159c2fc0, 0x10020827,
    0x0d9c11c0, 0xd0022827,
    0xffffffd8, 0xf03809e7,
    0x00000010, 0xe80009e7,
    0x009e7000, 0x100009e7,
    0x009e7000, 0x100009e7,
    0x80104000, 0xe0020827,
    0x0c9e7040, 0x10021c67,
    0xc0000000, 0xe0021c67,
    0x150a7d80, 0x10021ca7,
    0x009f2000, 0x100009e7,
    0x159e7d80, 0x100229a7,
    0x009e7000, 0x300009e7,
    0x009e7000, 0x100009e7,
    0x009e7000, 0x100009e7
};

int main()
{
    bcm_host_init();

    QPUWrapper::Qpu &qpu = QPUWrapper::Qpu::getQpuWrapperInstance();
    QPUWrapper::QpuProgram passthrough(1, 2, reinterpret_cast<uint8_t*>(uniformPassthroughBin), sizeof(uniformPassthroughBin));
    QPUWrapper::GpuBuffer resultBuffer = qpu.allocateGpuBuffer<uint32_t>(16);
    passthrough[0][0] = testNumber;
    passthrough[0][1] = resultBuffer.getGpuAddress();
    std::cout << "Testing first program (passthrough) on one QPU instance...\n";

    auto result = qpu.executeProgram(passthrough, 1000us).get();
    if(result != QPUWrapper::ExecutionResult::Success)
    {
        std::cout << "QPU passthrough program failed with result " << static_cast<int>(result) << std::endl;
        return -1;
    }

    uint32_t *resultsAddr = qpu.lockGpuBuffer(resultBuffer);
    for(int i = 0; i < 16; ++i)
    {
        if(resultsAddr[i] != testNumber)
        {
            std::cout << "QPU passthrough result at index " << i << " invalid: expected " << testNumber << " got " << resultsAddr[i] << std::endl;
            qpu.unlockGpuBuffer(resultBuffer);
            qpu.freeGpuBuffer(resultBuffer);
            return -2;
        }
    }
    qpu.unlockGpuBuffer(resultBuffer);
    qpu.freeGpuBuffer(resultBuffer);
    std::cout << "QPU passthrough program finished successfully!\n";

    const int qpuCount = qpu.getQpuCount();
    resultBuffer = qpu.allocateGpuBuffer<uint32_t>(qpuCount * 16);
    QPUWrapper::QpuProgram addition(qpuCount, 4, reinterpret_cast<uint8_t*>(uniformsAddition), sizeof(uniformsAddition));
    for(int qpuNr = 0; qpuNr < qpuCount; ++qpuNr)
    {
        addition[qpuNr][0] = qpuNr;
        addition[qpuNr][1] = testAddedNumber;
        addition[qpuNr][2] = resultBuffer.getGpuAddress();
        addition[qpuNr][3] = qpuCount;
    }

    std::cout << "Testing second program (addition) on " << qpuCount << " QPU instances...\n";
    result = qpu.executeProgram(addition, 1000us).get();
    if(result != QPUWrapper::ExecutionResult::Success)
    {
        std::cout << "QPU addition program failed with result " << static_cast<int>(result) << std::endl;
        return -3;
    }

    resultsAddr = qpu.lockGpuBuffer(resultBuffer);
    for(int qpuNr = 0; qpuNr < qpuCount; ++qpuNr)
    {
        for(int i = 0; i < 16; ++i)
        {
            int expected = qpuCount - 1 - qpuNr + testAddedNumber; //Looks like programs are scheduled starting from the last available QPU instance
            if(auto result = resultsAddr[qpuNr * 16 + i]; result != expected)
            {
                std::cout << "QPU addition result at QPU nr " << qpuNr << " and index " << i << " invalid: expected " << expected << " got " << result << std::endl;
                qpu.unlockGpuBuffer(resultBuffer);
                qpu.freeGpuBuffer(resultBuffer);
                return -4;
            }
        }
    }
    qpu.unlockGpuBuffer(resultBuffer);
    qpu.freeGpuBuffer(resultBuffer);
    std::cout << "QPU addition program finished successfully!\n";

    std::cout << "All tests passed!\n";
    return 0;
}