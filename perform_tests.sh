#!/bin/bash

#This script will run all available tests (needs to be run as superuser)

if [[ ! -f ./bin/memoryAllocationTest || ! -f ./bin/runningProgramTest ]]; then
    echo "Can't find test binary file(s). Make sure test are built (run 'make test') and try again."
    exit 1
fi

cd bin
rm -f vcmem_addr
./memoryAllocationTest &
while [ ! -f "vcmem_addr" ]; do
    echo "Waiting until memoryAllocationTest allocates GPU memory..."
    sleep 1
done

ADDR=`cat vcmem_addr`
echo "Checking 32 bytes of GPU memory at 0x$ADDR (it should be filled with 0xBA)"
vcdbg dump 0x$ADDR 32

while true; do
    read -p "Does it look ok? " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) echo "Test failed!"; exit;;
        * ) echo "Please answer [y]es or [n]o";;
    esac
done
echo "Memory test passed!"
./runningProgramTest