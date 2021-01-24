#This program adds first two uniforms and puts the result into address pointed by third uniform
#It is run on as many QPUs as indicated by fourth uniform

.include "vc4.qinc" #Source: https://github.com/maazl/vc4asm/blob/master/share/vc4inc/vc4.qinc

.set first,     ra1
.set second,    rb1
.set addr,      ra2
.set used_qpus, rb2

mov first,      unif;
mov second,     unif;
mov addr,       unif;
mov used_qpus,  unif;

ldi r0, vpm_setup(0, 0, h32(0));
add vw_setup, r0, qpu_num;
add vpm, first, second;
read vw_wait;

#Check if we run on QPU #0, if not - signal semaphore (increase it), if yes - signal and wait on semaphore (decrease it after increasing)
ldi r0, 0;
add.setf r1, r0, qpu_num;
brr.anynz -, :program_end

#Make use of three pre-branching instructions
mov -, srel0; #Always signal semaphore (even if we run on QPU #0) to limit branching
ldi r2, 23;
shl r1, used_qpus, r2; #It will be needed later to set number or vectors for DMA transfer (as many as used QPUs)

#Wait on semaphore to be signalled by all other QPUs
mov r0, used_qpus;
:semaphore_wait
    sub.setf r0, r0, 1;
    brr.anynz -, :semaphore_wait
    mov -, sacq0;
    nop;
    nop;

mov r0, vdw_setup_0(0, 16, dma_h32(0, 0));
add vw_setup, r0, r1;
mov vw_setup, vdw_setup_1(0);
mov vw_addr, addr;
read vw_wait;

:program_end
mov.setf irq, nop;
nop ; thrend
nop;
nop;