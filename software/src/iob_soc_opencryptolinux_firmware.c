#include "bsp.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_periphs.h"
#include "iob_soc_opencryptolinux_system.h"
#include "iob_str.h"
#include "printf.h"
#include "iob-plic.h"
#include "iob-clint.h"

#include "riscv-csr.h"
#include "riscv-interrupts.h"
#include "iob-clint-timer.h"

// Machine mode interrupt service routine
static void irq_entry(void) __attribute__ ((interrupt ("machine")));

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

int main() {
    //init uart
    uart16550_init(UART0_BASE, FREQ/(16*BAUD));
    clint_init(CLINT0_BASE);
    plic_init(PLIC0_BASE);

    printf("\n\n\nHello world!\n\n\n");

    // Global interrupt disable
    csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
    csr_write_mie(0);
    csr_clr_bits_mcause(MCAUSE_INTERRUPT_ALL_SET_MASK);
    csr_write_mcause(0);

    // Setup timer for 1 second interval
    timestamp = mtimer_get_raw_time();
    mtimer_set_raw_time_cmp(MTIMER_SECONDS_TO_CLOCKS(0.002));

    // Setup the IRQ handler entry point
    csr_write_mtvec((uint_xlen_t) irq_entry);

    // Enable MIE.MTI and MIE.MEI
    csr_set_bits_mie(MIE_MTI_BIT_MASK);
    csr_set_bits_mie(MIE_MEI_BIT_MASK);

    // Global interrupt enable
    csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);

    // Enable PLIC interrupt for UART
    printf("Enabling external interrupt source 0 with ID = 1.\n");
    int target;
    target = plic_enable_interrupt(0);

    printf("HART id %d, waiting for timer interrupt...\n", target);
    // Wait for interrupt
    __asm__ volatile ("wfi");

    printf("Exit...\n");
    uart16550_finish();
    return 0;
}

#pragma GCC push_options
// Force the alignment for mtvec.BASE. A 'C' extension program could be aligned to to bytes.
#pragma GCC optimize ("align-functions=4")
static void irq_entry(void)  {
    printf("Entered IRQ.\n");
    uint_xlen_t this_cause = csr_read_mcause();
    if (this_cause &  MCAUSE_INTERRUPT_BIT_MASK) {
        this_cause &= 0xFF;
        // Known exceptions
        switch (this_cause) {
        case RISCV_INT_POS_MTI :
            printf("Time interrupt.\n");
            // Timer exception, keep up the one second tick.
            mtimer_set_raw_time_cmp(MTIMER_SECONDS_TO_CLOCKS(1));
            timestamp = mtimer_get_raw_time();
            float aux = (float)timestamp/MTIME_FREQ_HZ;
            printf("Current time passed:   %.3f seconds.\n", aux);
            printf("MTIMER register value: %lld.\n", timestamp);
            break;
        case RISCV_INT_POS_MEI :
            printf("External interrupt.\n");
            int source_id = 0;
            source_id = plic_claim_interrupt();
            printf("External interrupt ID received was: %d.\n", source_id);
            plic_complete_interrupt(source_id);
            plic_disable_interrupt(source_id);
            break;
        }

    }
}
#pragma GCC pop_options