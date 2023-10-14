#include "bsp.h"
#include "clint.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_system.h"
#include "iob_str.h"
#include "plic.h"
#include "printf.h"

#include "riscv-csr.h"
#include "riscv-interrupts.h"

#define UART0_BASE 0xf4000000
#define CLINT0_BASE 0xf8000000
#define PLIC0_BASE 0xf0000000

#define MTIMER_SECONDS_TO_CLOCKS(SEC)           \
    ((uint64_t)(((SEC)*(FREQ))))

// Machine mode interrupt service routine
static void irq_entry(void) __attribute__((interrupt("machine")));

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

int main() {
  char pass_string[] = "Test passed!";
  uint_xlen_t irq_entry_copy;

  // init uart
  uart16550_init(UART0_BASE, FREQ / (16 * BAUD));
  clint_setCmp(CLINT0_BASE, 0xffffffffffffffff, 0);
  printf_init(&uart16550_putc);

  printf("\n\n\nHello world!\n\n\n");

  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  csr_write_mie(0);

  // Setup the IRQ handler entry point
  csr_write_mtvec((uint_xlen_t)irq_entry);

  // Setup timer
  timestamp = clint_getTime(CLINT0_BASE);
  printf("%x\n", (uint32_t)timestamp);
  clint_setCmp(CLINT0_BASE, MTIMER_SECONDS_TO_CLOCKS(0.001)+(uint32_t)timestamp, 0);

  // Enable MIE.MTI
  csr_set_bits_mie(MIE_MTI_BIT_MASK);

  // Global interrupt enable
  csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  printf("Waiting...\n");
  // Wait for interrupt
  __asm__ volatile("wfi");

  printf("Exit...\n");
  uart16550_finish();

  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);

  return 0;
}

#pragma GCC push_options
#pragma GCC optimize("align-functions=2")
static void irq_entry(void) {
  printf("Entered IRQ.\n");
  uint32_t this_cause = csr_read_mcause();
  timestamp = clint_getTime(CLINT0_BASE);
  printf("%x\n", (uint32_t)timestamp);
  if (this_cause & MCAUSE_INTERRUPT_BIT_MASK) {
    this_cause &= 0xFF;
    // Known exceptions
    switch (this_cause) {
    case RISCV_INT_POS_MTI:
      printf("Time interrupt.\n");
      // Timer exception, keep up the one second tick.
      clint_setCmp(CLINT0_BASE, MTIMER_SECONDS_TO_CLOCKS(0.001)+(uint32_t)timestamp, 0);
      break;
    }
  }
}
#pragma GCC pop_options
