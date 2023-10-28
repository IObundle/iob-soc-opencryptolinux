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
#define PLIC0_BASE 0xfc000000

#ifdef SIMULATION
#define WAIT_TIME 0.001
#else
#define WAIT_TIME 1
#endif

#define MTIMER_SECONDS_TO_CLOCKS(SEC)           \
    ((uint64_t)(((SEC)*(FREQ))))

// Machine mode interrupt service routine
static void irq_entry(void) __attribute__((interrupt("machine")));

char *send_string = "Sending this string as a file to console.\n"
                    "The file is then requested back from console.\n"
                    "The sent file is compared to the received file to confirm "
                    "correct file transfer via UART using console.\n"
                    "Generating the file in the firmware creates an uniform "
                    "file transfer between pc-emul, simulation and fpga without"
                    " adding extra targets for file generation.\n";

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

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


  // test file send
  char *sendfile = malloc(1000);
  int send_file_size = 0;
  send_file_size = string_copy(sendfile, send_string);
  uart16550_sendfile("Sendfile.txt", send_file_size, sendfile);

  // test file receive
  char *recvfile = malloc(10000);
  int file_size = 0;
  file_size = uart16550_recvfile("Sendfile.txt", recvfile);

  // compare files
  if (compare_str(sendfile, recvfile, send_file_size)) {
    printf("FAILURE: Send and received file differ!\n");
  } else {
    printf("SUCCESS: Send and received file match!\n");
  }

  free(sendfile);
  free(recvfile);

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
