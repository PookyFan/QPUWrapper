# QPUWrapper
## Description
This library serves as an interface to VideoCore4 QPU so that custom user programs could be run on it. For that purpose it allows to:
- create program wrapper objects that manage easy setup of uniforms
- allocate data buffers on GPU memory available from CPU (used mainly as one or more uniforms for program output data)
- execute program in the background, so that it doesn't have to be awaited on

While creating programs for QPU is a complicated task itself, QPUWrapper aims to make at least running them as easy as possible.

## Usage
1. Include `Qpu.hpp` header in your application code. It will allow you to use library classes from `QPUWrapper` namespace.
2. Call `Qpu::getQpuWrapperInstance()` static method to get QPU wrapper's reference.
3. Allocate some buffers (if needed) using `allocateGpuBuffer()` method on wrapper object.
4. Create `QpuProgram` object that will represent your program.
5. Use double `[]` operator on program's object to setup the uniforms (first `[]` to specify program instance index, second `[]` to access certain uniform). For example to set first uniform on the second instance, use `programObject[1][0] = value;` (tip: you can use your buffer's `getGpuAddress()` method output value to set it as uniform - it will be needed for output data of your programs).
6. Call QPU wrapper's `executeProgram()` method by providing program and timeout to start program instances execution right away. To make sure all of them have finished, call `get()` on the `std::future` object you just received from wrapper (don't forget to check the result!).
7. Using QPU wrapper's `lockGpuBuffer()` method acquire data pointer accessible on CPU side to process data from any output buffer you passed to your program earlier. When you're done, call `unlockGpuBuffer()` on that buffer. And if you're not going to use it anymore, call `freeGpuBuffer()` afterwards.
8. You can re-use program wrapper objects as many times as you want, changing uniforms between executions if needed - just make sure any GPU memory buffers used as uniforms stay valid and are not locked while program is being run. There is no need for cleaning up program objects' resources, as it will be handled automatically upon their destruction.

**Important**: you can only run one program at once, even if it doesn't use all of the available QPU instances.

When your code is ready, all that's left is compiling it and linking QPUWrapper library (you need to build it first with a simple `make` command), as well as `bcm_host` library that can be found in `/opt/vc/lib` directory. Of course you need to build both your application and QPUWrapper library on the Raspberry Pi, unless you have properly set-up cross-compiler toolchain with required headers and libraries.

## Examples and tests
In *tests* directory there are some basic tests that you may run to make sure QPUWrapper is working correctly on your Raspberry Pi. After you build them with `make test`, they can be run manually or by using `perform_tests.sh` script (both require superuser privileges). If you want to see a complete example of running QPU program, `runningProgramTest.cpp` is what you're looking for.

## Licensing information of used code
This library was heavily inspired by two other projects - [VC4CV](https://github.com/Seneral/VC4CV) and *hello_fft* / *gpu_fft* of the [Raspberry Pi's firmware](https://github.com/raspberrypi/firmware/tree/master/hardfp/opt/vc/src/hello_pi) repository. So heavily that, in fact, I believe it may be acknowledged as re-distribution of parts of the original code by some. Because of that I hereby attach licensing information as required by the licenses of aforementioned projects. As there is also some code licensed by Broadcom used from those repositories, I'm attaching their licensing notice as well.

### VC4CV license information
> MIT License
>
>Copyright (c) 2020 Levin G.
>
>Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
>
>The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
>
>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
>© 2021 GitHub, Inc.

### hello_fft / GPU_FFT license information
>BCM2835 "GPU_FFT" release 3.0
Copyright (c) 2015, Andrew Holme.
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### Broadcom license information
>Copyright (c) 2012, Broadcom Europe Ltd.
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
