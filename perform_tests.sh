#!/bin/bash
make test
cd bin
rm -f vcmem_addr
sudo ./memoryAllocationTest &
while [ ! -f "vcmem_addr" ]; do
    echo Waiting until memoryAllocationTest allocates GPU memory...
    sleep 1
done

ADDR=`cat vcmem_addr`
echo "Checking 32 bytes of GPU memory at 0x$ADDR (it should be filled with 0xBA)"
sudo vcdbg dump 0x$ADDR 32