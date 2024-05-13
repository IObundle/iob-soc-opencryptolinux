#include "bsp.h"
#include "clint.h"
#include "iob-eth.h"
#include "iob-spi.h"
#include "iob-spidefs.h"
#include "iob-spiplatform.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_periphs.h"
#include "iob_soc_opencryptolinux_system.h"
#include "printf.h"
#include <string.h>

#define PROGNAME "IOb-Bootloader"

#define DC1 17 // Device Control 1 (used to indicate end of bootloader)
#define EXT_MEM 0x80000000

#define NSAMPLES 16

// Ethernet utility functions
// NOTE: These functions are not compatible with malloc() and free().
//      These are specifically made for use with the current iob-eth.c drivers.
//      (These assume that there is only one block allocated at a time)
static void *mem_alloc(size_t size) {
  return (void *)(EXT_MEM | (1 << IOB_SOC_OPENCRYPTOLINUX_MEM_ADDR_W)) - size;
}
static void mem_free(void *ptr) {}

void clear_cache() {
  // Delay to ensure all data is written to memory
  for (unsigned int i = 0; i < 10; i++)
    asm volatile("nop");
  // Flush VexRiscv CPU internal cache
  asm volatile(".word 0x500F" ::: "memory");
}

// Send signal by uart to receive file by ethernet
uint32_t uart_recvfile_ethernet(char *file_name) {
  uart16550_puts(UART_PROGNAME);
  uart16550_puts(": requesting to receive file by ethernet\n");

  // send file receive by ethernet request
  uart16550_putc(0x13);

  // send file name (including end of string)
  uart16550_puts(file_name);
  uart16550_putc(0);

  // receive file size
  uint32_t file_size = uart16550_getc();
  file_size |= ((uint32_t)uart16550_getc()) << 8;
  file_size |= ((uint32_t)uart16550_getc()) << 16;
  file_size |= ((uint32_t)uart16550_getc()) << 24;

  // send ACK before receiving file
  uart16550_putc(ACK);

  return file_size;
}

void spi_read_all_regs(){
  unsigned int status_reg = 0, lock_reg = 0, flag_reg = 0;
  unsigned int non_vol_cfg = 0, vol_cfg = 0, enh_vol_cfg = 0;
  unsigned int ext_addr = 0;

  spiflash_readStatusReg(&status_reg);
  spiflash_readLockReg(&lock_reg);
  spiflash_readFlagReg(&flag_reg);
  spiflash_readNonVolConfigReg(&non_vol_cfg);
  spiflash_readVolConfigReg(&vol_cfg);
  spiflash_readEnhancedVolConfigReg(&enh_vol_cfg);
  spiflash_readExtendedAddrReg(&ext_addr);

  printf("\nStatus Reg: (%x)\n", status_reg);
  printf("Lock Reg: (%x)\n", lock_reg);
  printf("Flag Reg: (%x)\n", flag_reg);
  printf("NonVol Cfg Reg: (%x)\n", non_vol_cfg);
  printf("Vol Cfg Reg: (%x)\n", vol_cfg);
  printf("Enhanced Vol Cfg Reg: (%x)\n", enh_vol_cfg);
  printf("Extended Addr Reg: (%x)\n", ext_addr);
}

int main() {
  int run_linux = 0;
  int file_size;
  char *prog_start_addr;

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
  prog_start_addr = (char *)(EXT_MEM);

  while (uart16550_getc() != ACK) {
    uart16550_puts(PROGNAME);
    uart16550_puts(": Waiting for Console ACK.\n");
  }

// #ifndef VERILATOR
//   // Init printf
//   printf_init(&uart16550_putc);
//   // init spit flash controller
//   spiflash_init(SPI0_BASE);
//   printf("\nResetting flash registers...\n");
//   spiflash_resetmem();
//   // Read ID
//   // Manufacturer ID
//   unsigned int readid[5] = {0}, id_bytes = 0;
//   for (id_bytes = 0; id_bytes < 20; id_bytes += 4) {
//     spiflash_executecommand(COMMANS, 0, id_bytes, ((4 * 8) << 8) | READ_ID,
//                             &(readid[id_bytes / 4]));
//   }
//   printf("\nMANUFACTURER ID: (%x)\n", (readid[0] & 0xFF));
//   printf("DEVICE ID: (%x)\n", (readid[0] & 0xFFFF00) >> 8);
//   printf("UNIQUE ID:\n");
//   printf("\tData to follow: (%x)\n", (readid[0] & 0xFFFFFF00) >> 3*8);
//   printf("\tExt Device ID: (%x)\n", (readid[1] & 0xFFFF));
//   printf("\tCustom Factory data[0-1]: (%x)\n", readid[1] >> 2*8);
//   printf("\tCustom Factory data[5-2]: (%x)\n", readid[2]);
//   printf("\tCustom Factory data[9-6]: (%x)\n", readid[3]);
//   printf("\tCustom Factory data[13-10]: (%x)\n", readid[4]);
//
//   printf("Testing program flash\n");
//   char prog_data[NSAMPLES] = {0};
//   char *char_data = NULL;
//   unsigned int read_data[NSAMPLES] = {0};
//   int sample = 0;
//   int test_result = 0;
//   for (sample = 0; sample < NSAMPLES; sample++) {
//     prog_data[sample] = 0xFF - sample;
//   }
//
//   spi_read_all_regs();
//
//   spiflash_memProgram(prog_data, NSAMPLES, 0x0);
//   printf("After program\n");
//
//   spi_read_all_regs();
//
//   for (sample = 0; sample < NSAMPLES; sample = sample + 4) {
//     read_data[sample >> 2] = spiflash_readmem(0x0 + sample);
//   }
//   // check prog vs read data
//   char_data = (char *)read_data;
//   for (sample = 0; sample < NSAMPLES; sample++) {
//     if (prog_data[sample] != char_data[sample]) {
//       printf("Error: data[%x] = %02x != read_data[%x] = %02x\n", sample,
//              prog_data[sample], sample, char_data[sample]);
//       test_result = 1;
//     }
//   }
//   if (test_result) {
//     printf("Flash test failed!\n");
//   }
// #endif // ifndef VERILATOR

#ifndef IOB_SOC_OPENCRYPTOLINUX_INIT_MEM
  // Init ethernet and printf (for ethernet)
  printf_init(&uart16550_putc);
  eth_init(ETH0_BASE, &clear_cache);
  // Use custom memory alloc/free functions to ensure it allocates in external
  // memory
  eth_init_mem_alloc(&mem_alloc, &mem_free);
  // Wait for PHY reset to finish
  eth_wait_phy_rst();

  file_size = uart16550_recvfile("../iob_soc_opencryptolinux_mem.config",
                                 prog_start_addr);
  // compute_mem_load_txt
  int state = 0;
  int file_name_count = 0;
  int file_count = 0;
  char file_name_array[4][50];
  long int file_address_array[4];
  char hexChar = 0;
  int hexDecimal = 0;
  int i = 0;
  for (i = 0; i < file_size; i++) {
    hexChar = *(prog_start_addr + i);
    // uart16550_puts(&hexChar); /* Used for debugging. */
    if (state == 0) {
      if (hexChar == ' ') {
        file_name_array[file_count][file_name_count] = '\0';
        file_name_count = 0;
        file_address_array[file_count] = 0;
        file_count = file_count + 1;
        state = 1;
      } else {
        file_name_array[file_count][file_name_count] = hexChar;
        file_name_count = file_name_count + 1;
      }
    } else if (state == 1) {
      if (hexChar == '\n') {
        state = 0;
      } else {
        if ('0' <= hexChar && hexChar <= '9') {
          hexDecimal = hexChar - '0';
        } else if ('a' <= hexChar && hexChar <= 'f') {
          hexDecimal = 10 + hexChar - 'a';
        } else if ('A' <= hexChar && hexChar <= 'F') {
          hexDecimal = 10 + hexChar - 'A';
        } else {
          uart16550_puts(PROGNAME);
          uart16550_puts(": invalid hexadecimal character.\n");
        }
        file_address_array[file_count - 1] =
            file_address_array[file_count - 1] * 16 + hexDecimal;
      }
    }
  }

  for (i = 0; i < file_count; i++) {
    prog_start_addr = (char *)(EXT_MEM + file_address_array[i]);
    // Receive data from console via Ethernet
#ifndef SIMULATION
    file_size = uart_recvfile_ethernet(file_name_array[i]);
    eth_rcv_file(prog_start_addr, file_size);
#else
    file_size = uart16550_recvfile(file_name_array[i], prog_start_addr);
#endif
  }

  // Check if running Linux
  for (i = 0; i < file_count; i++) {
    if (!strcmp(file_name_array[i], "rootfs.cpio.gz")) {
#ifdef SIMULATION
      // Running Linux: setup required dependencies
      uart16550_sendfile("test.log", 12, "Test passed!");
      uart16550_putc((char)DC1);
#endif
      run_linux = 1;
      break;
    }
  }
#else // INIT_MEM = 1
#ifdef IOB_SOC_OPENCRYPTOLINUX_RUN_LINUX
  // Running Linux: setup required dependencies
  uart16550_sendfile("test.log", 12, "Test passed!");
  uart16550_putc((char)DC1);
#endif
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

#ifndef SIMULATION
  // Terminate console if running Linux on FPGA
  if (run_linux)
    uart16550_finish();
#endif
}
