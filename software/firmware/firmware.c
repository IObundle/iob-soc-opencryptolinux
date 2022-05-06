#include "stdlib.h"
#include <stdio.h>
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "myclint.h"
#include "printf.h"

int main()
{
  unsigned long long elapsed;

  //init uart
  uart_init(UART_BASE,FREQ/BAUD);
  clint_init(CLINT_BASE);

  uart_puts("\n\n\nHello world!\n\n\n");
  printf("Value of Pi = %f\n\n", 3.1415);

  elapsed = clint_get_timer();
  printf("\nCLINT timer value: %d\n", (unsigned int) elapsed);
  uart_finish();
}
