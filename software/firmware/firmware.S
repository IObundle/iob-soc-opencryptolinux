#include "system.h"

.section .init
.global main
// The 'norelax' option is critical here.
// Without 'norelax' the global pointer will
// be loaded relative to the global pointer!
.option push
.option norelax
la gp, 0x800
.option pop

//set stack pointer
lui sp, %hi(1<<FIRM_ADDR_W)
addi sp, sp, %lo(1<<FIRM_ADDR_W)

//call main
jal ra, main

//reboot to run bootloader
li s5, 3
#ifdef RUN_EXTMEM
li s6, BOOTCTR_BASE | EXTRA_BASE
#else
li s6, BOOTCTR_BASE
#endif
sw s5, 0(s6)//cpu_rst_req=1, boot=1

ebreak
