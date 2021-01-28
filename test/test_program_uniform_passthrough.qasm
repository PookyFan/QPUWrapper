#This program just copies value from first uniform into address pointed by second uniform

#https://github.com/maazl/vc4asm/blob/master/share/vc4inc/vc4.qinc
.include "vc4.qinc"

.set data, ra1
.set addr, ra2

mov data, unif;
mov addr, unif;

mov vw_setup, vpm_setup(0, 0, h32(0));
mov vpm, data;
read vw_wait;

mov vw_setup, vdw_setup_0(1, 16, dma_h32(0, 0));
mov vw_setup, vdw_setup_1(0);
mov vw_addr, addr;
read vw_wait;

mov.setf irq, nop;
nop ; thrend
nop;
nop;