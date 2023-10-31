#include "bsp.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_system.h"
#include "clint.h"

// defined here (and not in periphs.h) because it is the only peripheral used
// by the bootloader
#define UART0_BASE ((UART0<<(ADDR_W-4-N_SLAVES_W))|(0xf<<(ADDR_W-4)))

#define PROGNAME "IOb-Bootloader"

#define DC1 17 // Device Control 1 (used to indicate end of bootloader)
#define EXT_MEM 0x80000000

int main() {

  // init uart
  uart16550_init(UART0_BASE, FREQ / (16 * BAUD));

  // connect with console
  do {
    if (uart16550_txready())
      uart16550_putc((char)ENQ);
  } while (!uart16550_rxready());

  // welcome message
  uart16550_puts(PROGNAME);
  uart16550_puts(": connected!\n");

  uart16550_puts(PROGNAME);
  uart16550_puts(": DDR in use and program runs from DDR\n");

  // address to copy firmware to
  char *prog_start_addr;
  prog_start_addr = (char *)(EXT_MEM);

  while (uart16550_getc() != ACK) {
    uart16550_puts(PROGNAME);
    uart16550_puts(": Waiting for Console ACK.\n");
  }

#ifndef INIT_MEM
#ifdef RUN_LINUX

  // receive firmware from host
  int file_size = 0;
  char opensbi[] = "fw_jump.bin";
  char kernel[] = "Image";
  char dtb[] = "iob_soc.dtb";
  char rootfs[] = "rootfs.cpio.gz";
  file_size = uart16550_recvfile(opensbi, prog_start_addr);
  prog_start_addr = (char *)(EXT_MEM + 0x00400000);
  file_size = uart16550_recvfile(kernel, prog_start_addr);
  prog_start_addr = (char *)(EXT_MEM + 0x00F80000);
  file_size = uart16550_recvfile(dtb, prog_start_addr);
  prog_start_addr = (char *)(EXT_MEM + 0x01000000);
  file_size = uart16550_recvfile(rootfs, prog_start_addr);
  uart16550_puts(PROGNAME);
  uart16550_puts(": Loading firmware...\n");

#else

  // receive firmware from host
  int file_size = 0;
  char r_fw[] = "iob_soc_opencryptolinux_firmware.bin";
  file_size = uart16550_recvfile(r_fw, prog_start_addr);
  uart16550_puts(PROGNAME);
  uart16550_puts(": Loading firmware...\n");

  // sending firmware back for debug
  if (file_size)
    uart16550_sendfile(r_fw, file_size, prog_start_addr);
  else {
    uart16550_puts(PROGNAME);
    uart16550_puts(": ERROR loading firmware\n");
  }

#endif
#endif

#ifdef RUN_LINUX
  uart16550_putc((char)DC1);
#endif

  // Clear CPU registers, to not pass arguments to the next
  asm volatile("li a0,0");
  asm volatile("li a1,0");
  asm volatile("li a2,0");
  asm volatile("li a3,0");
  asm volatile("li a4,0");
  asm volatile("li a5,0");
  asm volatile("li a6,0");
  asm volatile("li a7,0");

  // run firmware
  uart16550_puts(PROGNAME);
  uart16550_puts(": Restart CPU to run user program...\n");
  uart16550_txwait();
}
