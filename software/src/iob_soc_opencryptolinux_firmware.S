#include "iob_soc_opencryptolinux_system.h"
#include "iob_soc_opencryptolinux_conf.h"

.section .init
.globl _start
.type _start,@function

_start:
.option push
.option norelax
    la gp, __global_pointer$
.option pop

    //set stack pointer
    lui sp, %hi(1<<SRAM_ADDR_W)
    addi sp, sp, %lo(1<<SRAM_ADDR_W)

    //call main
    call main

    //reboot to run bootloader
    li s5, 3 //cpu_rst_req=1, boot=1
    li s6, BOOTCTR_BASE
    sw s5, 0(s6)//cpu_rst_req=1, boot=1

    ebreak
