#include "bsp.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_periphs.h"
#include "iob_soc_opencryptolinux_system.h"
#include "clint.h"
#include "plic.h"
#include "printf.h"
#include "iob-eth.h"

#include "riscv-csr.h"
#include "riscv-interrupts.h"

#include "versat_crypto_tests.h"

#ifdef SIMULATION
#define WAIT_TIME 0.001
#else
#define WAIT_TIME 1
#endif

#define MTIMER_SECONDS_TO_CLOCKS(SEC)           \
    ((uint64_t)(((SEC)*(FREQ))))

// Machine mode interrupt service routine
static void irq_entry(void) __attribute__((interrupt("machine")));

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

void clear_cache(){
  // Delay to ensure all data is written to memory
  for ( unsigned int i = 0; i < 10; i++)asm volatile("nop");
  // Flush VexRiscv CPU internal cache
  asm volatile(".word 0x500F" ::: "memory");
}

// Send signal by uart to receive file by ethernet
uint32_t uart_recvfile_ethernet(const char *file_name) {

  uart16550_puts(UART_PROGNAME);
  uart16550_puts (": requesting to receive file by ethernet\n");

  //send file receive by ethernet request
  uart16550_putc (0x13);

  //send file name (including end of string)
  uart16550_puts(file_name); uart16550_putc(0);

  // receive file size
  uint32_t file_size = uart16550_getc();
  file_size |= ((uint32_t)uart16550_getc()) << 8;
  file_size |= ((uint32_t)uart16550_getc()) << 16;
  file_size |= ((uint32_t)uart16550_getc()) << 24;

  // send ACK before receiving file
  uart16550_putc(ACK);

  return file_size;
}

// copy src to dst
// return number of copied chars (excluding '\0')
int string_copy(char *dst, char *src) {
  if (dst == NULL || src == NULL) {
    return -1;
  }
  int cnt = 0;
  while (src[cnt] != 0) {
    dst[cnt] = src[cnt];
    cnt++;
  }
  dst[cnt] = '\0';
  return cnt;
}

// 0: same string
// otherwise: different
int compare_str(char *str1, char *str2, int str_size) {
  int c = 0;
  while (c < str_size) {
    if (str1[c] != str2[c]) {
      return str1[c] - str2[c];
    }
    c++;
  }
  return 0;
}

// Needed by crypto side to time algorithms.
// Does not need to return seconds or any time unit, we are comparing directly with the software implementation. 
// Only care about the relative differences
int GetTime(){
  return clint_getTime(CLINT0_BASE);
}

int main() {
  char pass_string[] = "Test passed!";
  uint_xlen_t irq_entry_copy;
  int i;

  // init uart
  uart16550_init(UART0_BASE, FREQ / (16 * BAUD));
  clint_setCmp(CLINT0_BASE, 0xffffffffffffffff, 0);
  printf_init(&uart16550_putc);
  // init eth
  eth_init(ETH0_BASE, &clear_cache);
  eth_wait_phy_rst();

#ifndef SIMULATION
  char buffer[5096];
  // Receive data from console via Ethernet
  uint32_t file_size = uart_recvfile_ethernet("../src/eth_example.txt");
  eth_rcv_file(buffer,file_size);
  uart16550_puts("\nFile received from console via ethernet:\n");
  for(i=0; i<file_size; i++)
    uart16550_putc(buffer[i]);
#endif

//#ifndef SIMULATION
  InitializeCryptoSide(VERSAT0_BASE);
//#endif

  printf("\n\n\nHello world!\n\n\n");

  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  csr_write_mie(0);

  // Setup the IRQ handler entry point
  csr_write_mtvec((uint_xlen_t)irq_entry);

  // Setup timer
  timestamp = clint_getTime(CLINT0_BASE);
  clint_setCmp(CLINT0_BASE, MTIMER_SECONDS_TO_CLOCKS(WAIT_TIME)+(uint32_t)timestamp, 0);

  // Enable MIE.MTI
  csr_set_bits_mie(MIE_MTI_BIT_MASK);

  // Global interrupt enable
  csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  printf("Waiting...\n");
  // Wait for interrupt
  __asm__ volatile("wfi");

  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);

  int testResult = 0;

  // Tests are too big and slow to perform during simulation.
  // Comment out the source files in sw_build.mk to also reduce binary size and speedup simulation.
#ifndef SIMULATION
  testResult |= VersatSHATests();
  testResult |= VersatAESTests();
  testResult |= VersatMcElieceTests();
#else
  testResult |= VersatSimpleSHATests();
  //testResult |= VersatSimpleAESTests();
#endif

  if(testResult){
    uart16550_sendfile("test.log", 12, "Test failed!");
  } else {
    uart16550_sendfile("test.log", 12, "Test passed!");
  }

  printf("Exit...\n");
  uart16550_finish();

  return 0;
}

#pragma GCC push_options
#pragma GCC optimize("align-functions=2")
static void irq_entry(void) {
  printf("Entered IRQ.\n");
  uint32_t this_cause = csr_read_mcause();
  timestamp = clint_getTime(CLINT0_BASE);
  if (this_cause & MCAUSE_INTERRUPT_BIT_MASK) {
    this_cause &= 0xFF;
    // Known exceptions
    switch (this_cause) {
    case RISCV_INT_POS_MTI:
      printf("Time interrupt.\n");
      // Timer exception, keep up the one second tick.
      clint_setCmp(CLINT0_BASE, MTIMER_SECONDS_TO_CLOCKS(WAIT_TIME)+(uint32_t)timestamp, 0);
      break;
    }
  }
}
#pragma GCC pop_options
