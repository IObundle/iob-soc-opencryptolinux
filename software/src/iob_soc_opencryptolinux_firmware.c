#include "bsp.h"
#include "clint.h"
#include "iob-spi.h"
#include "iob-spidefs.h"
#include "iob-spiplatform.h"
#include "iob-uart16550.h"
#include "iob_soc_opencryptolinux_conf.h"
#include "iob_soc_opencryptolinux_periphs.h"
#include "iob_soc_opencryptolinux_system.h"
#include "plic.h"
#include "printf.h"
#include "iob-eth.h"
#include <string.h>

#include "riscv-csr.h"
#include "riscv-interrupts.h"

#ifdef SIMULATION
#define WAIT_TIME 0.001
#else
#define WAIT_TIME 1
#endif

#define MTIMER_SECONDS_TO_CLOCKS(SEC) ((uint64_t)(((SEC) * (FREQ))))

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
uint32_t uart_recvfile_ethernet(char *file_name) {

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

  printf("\n\n\nHello world!\n\n\n");

  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);

#ifdef SIMULATION
  unsigned int word = 0xFAFAB0CA;
  unsigned int address = 0x000100;
  unsigned int read_mem = 0xF0F0F0F0;
  printf("\nTest: %x, %x.\n", word, read_mem);
  // init spi flash controller
  spiflash_init(SPI0_BASE);
  printf("\nTesting SPI flash controller\n");
  // Reading Status Reg
  unsigned int reg = 0x00;
  spiflash_readStatusReg(&reg);
  printf("\nStatus reg (%x)\n", reg);

  // Testing Fast Read in single, dual, quad
  unsigned bytes = 4, readid = 0;
  unsigned frame = 0x00000000;
  unsigned commFastRead = 0x0b;
  unsigned fastReadmem0 = 0, fastReadmem1 = 0, fastReadmem2 = 0;
  unsigned dummycycles = 8;

  // Read ID
  bytes = 4;
  readid = 0;
  spiflash_executecommand(COMMANS, 0, 0, ((bytes * 8) << 8) | READ_ID, &readid);

  printf("\nREAD_ID: (%x)\n", readid);
  // Read from flash memory
  printf("\nReading from flash (address: (%x))\n", address);
  read_mem = spiflash_readmem(address);

  if (word == read_mem) {
    printf("\nMemory Read (%x) got same word as Programmed(%x)\nSuccess\n",
           read_mem, word);
  } else {
    printf("\nDifferent word from memory\nRead: (%x), Programmed: (%x)\n",
           read_mem, word);
  }

  address = 0x0;
  read_mem = 1;
  printf("\nTesting dual output fast read\n");
  read_mem = spiflash_readfastDualOutput(address + 1, 0);
  printf("\nRead from memory address (%x) the word: (%x)\n", address, read_mem);

  read_mem = 2;
  printf("\nTesting quad output fast read\n");
  read_mem = spiflash_readfastQuadOutput(address + 1, 0);
  printf("\nRead 2 from memory address (%x) the word: (%x)\n", address + 1,
         read_mem);

  read_mem = 3;
  printf("\nTesting dual input output fast read 0xbb\n");
  read_mem = spiflash_readfastDualInOutput(address + 1, 0);
  printf("\nRead 2 from memory address (%x) the word: (%x)\n", address + 2,
         read_mem);

  read_mem = 4;
  printf("\nTesting quad input output fast read 0xeb\n");
  read_mem = spiflash_readfastQuadInOutput(address + 1, 0);
  printf("\nRead 2 from memory address (%x) the word: (%x)\n", address + 3,
         read_mem);

  printf("\nRead Non volatile Register\n");
  unsigned nonVolatileReg = 0;
  bytes = 2;
  unsigned command_aux = 0xb5;
  spiflash_executecommand(COMMANS, 0, 0, ((bytes * 8) << 8) | command_aux,
                          &nonVolatileReg);
  printf("\nNon volatile Register (16 bits):(%x)\n", nonVolatileReg);

  printf("\nRead enhanced volatile Register\n");
  unsigned enhancedReg = 0;
  bytes = 1;
  command_aux = 0x65;
  frame = 0x00000000;
  spiflash_executecommand(COMMANS, 0, 0,
                          (frame << 20) | ((bytes * 8) << 8) | command_aux,
                          &enhancedReg);
  printf("\nEnhanced volatile Register (8 bits):(%x)\n", enhancedReg);

  // Testing xip bit enabling and xip termination sequence
  printf("\nTesting xip enabling through volatile bit and termination by "
         "sequence\n");
  unsigned volconfigReg = 0;

  printf("\nResetting flash registers...\n");
  spiflash_resetmem();

  spiflash_readVolConfigReg(&volconfigReg);
  printf("\nVolatile Configuration Register (8 bits):(%x)\n", volconfigReg);

  spiflash_XipEnable();

  volconfigReg = 0;
  spiflash_readVolConfigReg(&volconfigReg);
  printf(
      "\nAfter xip bit write, Volatile Configuration Register (8 bits):(%x)\n",
      volconfigReg);

  // Confirmation bit 0
  read_mem = 1;
  printf("\nTesting quad input output fast read with xip confirmation bit 0\n");
  read_mem = spiflash_readfastQuadInOutput(address + 1, ACTIVEXIP);
  printf("\nRead from memory address (%x) the word: (%x)\n", address + 1,
         read_mem);

  int xipEnabled = 10;
  xipEnabled = spiflash_terminateXipSequence();
  printf("\nAfter xip termination sequence: %d\n", xipEnabled);
  volconfigReg = 0;
  spiflash_readVolConfigReg(&volconfigReg);
  printf("\nAfter xip termination sequence, Volatile Configuration Register (8 "
         "bits):(%x)\n",
         volconfigReg);

  if (volconfigReg == 0xf3 || volconfigReg != 0xfb) {
    printf("\nAssuming Xip active, read from memory, confirmation bit 1\n");
    read_mem = 1;
    read_mem = spiflash_readMemXip(address + 1, TERMINATEXIP);
    printf("\nRead from memory address (%x) the word: (%x)\n", address + 1,
           read_mem);
  }
#endif // SIMULATION

  uart16550_sendfile("test.log", 12, "Test passed!");
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
      clint_setCmp(CLINT0_BASE,
                   MTIMER_SECONDS_TO_CLOCKS(WAIT_TIME) + (uint32_t)timestamp,
                   0);
      break;
    }
  }
}
#pragma GCC pop_options
