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
#include "plic.h"
#include "printf.h"
#include <string.h>

#include "riscv-csr.h"
#include "riscv-interrupts.h"

#include "versat_crypto_tests.h"

#ifdef SIMULATION
#define WAIT_TIME 0.001
#else
#define WAIT_TIME 1
#endif

#define MTIMER_SECONDS_TO_CLOCKS(SEC) ((uint64_t)(((SEC) * (FREQ))))

#define NSAMPLES 16

// Machine mode interrupt service routine
static void irq_entry(void) __attribute__((interrupt("machine")));

// Global to hold current timestamp
static volatile uint64_t timestamp = 0;

void clear_cache();
uint32_t uart_recvfile_ethernet(const char *file_name);
int string_copy(char *dst, char *src);
int compare_str(char *str1, char *str2, int str_size);
int GetTime();

#if 0

void clear_cache() {
  // Delay to ensure all data is written to memory
  for (unsigned int i = 0; i < 10; i++)
    asm volatile("nop");
  // Flush VexRiscv CPU internal cache
  asm volatile(".word 0x500F" ::: "memory");
}

// Send signal by uart to receive file by ethernet
uint32_t uart_recvfile_ethernet(const char *file_name) {

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

#endif

int main() {
  char pass_string[] = "Test passed!";
  uint_xlen_t irq_entry_copy;
  int i;
  int test_result = 0;

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
  eth_rcv_file(buffer, file_size);
  uart16550_puts("\nFile received from console via ethernet:\n");
  for (i = 0; i < file_size; i++)
    uart16550_putc(buffer[i]);
#endif

//#ifndef SIMULATION
  InitializeCryptoSide(VERSAT0_BASE);
//#endif

  printf("\n\n\nHello world!\n\n\n");

  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);

#ifdef SIMULATION
#ifndef VERILATOR
  unsigned int word = 0xA3A2A1A0;
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
  spiflash_executecommand(COMMANS, 0, 0, ((bytes * 8) << 8) | READ_ID,
  &readid);

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
    test_result = 1;
  }


  address = 0x0;
  read_mem = 1;
  printf("\nTesting dual output fast read\n");
  read_mem = spiflash_readfastDualOutput(address, 0);
  printf("\nRead from memory address (%x) the word: (%x)\n", address,
  read_mem); word = read_mem;

  read_mem = 2;
  printf("\nTesting quad output fast read\n");
  read_mem = spiflash_readfastQuadOutput(address, 0);
  if (read_mem == word) {
    printf("\nQuadFastOutput Read (%x) got same word as Expected (%x)\nSuccess\n", address, read_mem);
  } else {
    printf("\nQuadFastOutput Read (%x) Different word from memory\nRead: (%x), Read: (%x),Expected: (%x)\n", address, read_mem, word);
    test_result = 1;
  }

  read_mem = 3;
  printf("\nTesting dual input output fast read 0xbb\n");
  read_mem = spiflash_readfastDualInOutput(address, 0);
  if (read_mem == word) {
    printf("\nDualFastInOutput Read (%x) got same word as Expected "
           "(%x)\nSuccess\n",
           address, read_mem);
  } else {
    printf( "\nDualFastInOutput Read (%x) Different word from memory\nRead: (%x), Read: (%x),Expected: (%x)\n", address, read_mem, word);
    test_result = 1;
  }

  read_mem = 4;
  printf("\nTesting quad input output fast read 0xeb\n");
  read_mem = spiflash_readfastQuadInOutput(address, 0);
  if (read_mem == word) {
    printf("\nQuadFastInOutput Read (%x) got same word as Expected "
           "(%x)\nSuccess\n",
           address, read_mem);
  } else {
    printf("\nQuadFastInOutput Read (%x) Different word from memory\nRead: (%x), Read: (%x),Expected: (%x)\n", address, read_mem, word); 
    test_result = 1;
  }

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
  printf("\nAfter xip bit write, Volatile Configuration Register (8 bits):(%x)\n", volconfigReg);

  // Confirmation bit 0
  read_mem = 1;
  printf("\nTesting quad input output fast read with xip confirmation bit 0\n"); 
  read_mem = spiflash_readfastQuadInOutput(address, ACTIVEXIP);
  printf("\nRead from memory address (%x) the word: (%x)\n", address,
  read_mem); if (read_mem == word) {
    printf("\nQuadFastInOutput XIP Read (%x) got same word as Expected (%x)\nSuccess\n", address, read_mem);
  } else {
    printf("\nQuadFastInOutput XIP Read (%x) Different word from memory\nRead: (%x), Read: (%x),Expected: (%x)\n", address, read_mem, word);
    test_result = 1;
  }

  int xipEnabled = 10;
  xipEnabled = spiflash_terminateXipSequence();
  printf("\nAfter xip termination sequence: %d\n", xipEnabled);
  volconfigReg = 0;
  spiflash_readVolConfigReg(&volconfigReg);
  printf("\nAfter xip termination sequence, Volatile Configuration Register (8 bits):(%x)\n", volconfigReg);

  // XIP Bit 0 -> XIP ON
  if (((volconfigReg >> VOLCFG_XIP) & 0x1) == 0) {
    printf("\nAssuming Xip active, read from memory, confirmation bit 1\n");
    read_mem = 1;
    read_mem = spiflash_readMemXip(address, TERMINATEXIP);
    printf("\nRead from memory address (%x) the word: (%x)\n", address,
           read_mem);
  }

  printf("Testing program flash\n");
  char prog_data[NSAMPLES] = {0};
  char *char_data = NULL;
  unsigned int read_data[NSAMPLES] = {0};
  int sample = 0;
  for (sample = 0; sample < NSAMPLES; sample++) {
    prog_data[sample] = sample;
  }
  spiflash_memProgram(prog_data, NSAMPLES, 0x104);
  for (sample = 0; sample < NSAMPLES; sample = sample + 4) {
    read_data[sample>>2] = spiflash_readmem(0x104 + sample);
  }
  // check prog vs read data
  char_data = (char *)read_data;
  for (sample = 0; sample < NSAMPLES; sample++) {
    if (prog_data[sample] != char_data[sample]) {
      printf("Error: data[%x] = %08x != read_data[%x] = %08x\n", sample, prog_data[sample], sample, char_data[sample]);
      test_result = 1;
    }
  }

#endif // #ifndef VERILATOR
#endif // #ifdef SIMULATION

  // Tests are too big and slow to perform during simulation.
  // Comment out the source files in sw_build.mk to also reduce binary size and speedup simulation.
#ifndef SIMULATION
  test_result |= VersatSHATests();
  test_result |= VersatAESTests();
  test_result |= VersatMcElieceTests();
#else
  test_result |= VersatSimpleSHATests();
  test_result |= VersatSimpleAESTests();
#endif

  if (test_result) {
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
      clint_setCmp(CLINT0_BASE,
                   MTIMER_SECONDS_TO_CLOCKS(WAIT_TIME) + (uint32_t)timestamp,
                   0);
      break;
    }
  }
}
#pragma GCC pop_options
