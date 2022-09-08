#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "printf.h"

int main()
{
  uint64_t elapsed;

  //init uart
  uart_init(UART16550_BASE,FREQ/(16*BAUD));

  uart_puts("\n\n\nHello world!\n\n\n");

  //test printf with floats
  printf("Value of Pi = %f\n\n", 3.1415);

  uart_finish();
}
