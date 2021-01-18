#include <chrono>
#include <fstream>
#include <thread>

#include <bcm_host.h>

#include "Qpu.hpp"

using namespace std::chrono_literals;

constexpr size_t BUFSIZE = 32;

int main()
{
    bcm_host_init();

    QPUWrapper::Qpu &qpu = QPUWrapper::Qpu::getQpuWrapperInstance();
    QPUWrapper::GpuBuffer buffer = qpu.allocateGpuBuffer<uint8_t>(BUFSIZE);
    uint8_t *addr = qpu.lockGpuBuffer(buffer);//.useGpuBuffer<uint8_t>(buffer, [](uint8_t *addr){
    for(int i = 0; i < BUFSIZE; ++i)
        addr[i] = 0xBA;
    qpu.unlockGpuBuffer(buffer);

    std::fstream file("vcmem_addr", std::ios::out);
    file << std::hex << buffer.getGpuAddress();
    file.close();
    std::this_thread::sleep_for(5s);
    qpu.freeGpuBuffer(buffer);

    return 0;
}