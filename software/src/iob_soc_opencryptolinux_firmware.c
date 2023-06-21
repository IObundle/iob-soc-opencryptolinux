#include <stdio.h>
#include <stdint.h>
#include "bsp.h"
#include "iob_soc_opencryptolinux_system.h"
#include "iob_soc_opencryptolinux_periphs.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob-uart.h"
#include "iob-clint.h"
#include "printf.h"
#include "iob_str.h"

#include "riscv-csr.h"
#include "riscv-interrupts.h"
#include "iob-clint-timer.h"

// Machine mode interrupt service routine
static void irq_entry(void) __attribute__ ((interrupt ("machine")));

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

int main() {
    char pass_string[] = "Test passed!";
    char fail_string[] = "Test failed!";

    //init uart
    uart_init(UART0_BASE, FREQ/(16*BAUD));
    clint_init(CLINT0_BASE);

    printf("\n\n\nHello world!\n\n\n");

    uart_sendfile("test.log", iob_strlen(pass_string), pass_string);

    uart_finish();
    return 0;

}