#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "iob_clint.h"
#include "printf.h"

int main()
{
  unsigned long long elapsed;

  //init uart
  uart_init(UART16550_BASE,FREQ/(16*BAUD));
  clint_init(CLINT_BASE);

  uart_puts("\n\n\nHello world!\n\n\n");
  printf("Value of Pi = %f\n\n", 3.1415);

  uart_finish();
}
