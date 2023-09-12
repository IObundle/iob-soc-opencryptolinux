#include "bsp.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_system.h"
#include "iob_str.h"
#include "printf.h"

#define UART0_BASE 0xf2000000
#define PLIC0_BASE 0xf4000000
#define CLINT0_BASE 0xf6000000

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

int main() {
  char pass_string[] = "Test passed!";

  // init uart
  uart16550_init(UART0_BASE, FREQ / (16 * BAUD));
  printf_init(&uart16550_putc);

  printf("\n\n\nHello world!\n\n\n");

  printf("Exit...\n");
  uart16550_finish();
  return 0;
}
