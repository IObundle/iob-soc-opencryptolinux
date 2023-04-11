#include "iob_soc_system.h"
#include "iob_soc_conf.h"

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
lui sp, %hi(1<<SRAM_ADDR_W)
addi sp, sp, %lo(1<<SRAM_ADDR_W)

//call main
jal ra, main

//reboot to run bootloader
li s5, 3 //cpu_rst_req=1, boot=1
#ifdef USE_EXTMEM //need to set MSB to address internal mem and boot controller
li s6, EXTRA_BASE | BOOTCTR_BASE
#else
li s6, BOOTCTR_BASE
#endif
sw s5, 0(s6)//cpu_rst_req=1, boot=1

ebreak
