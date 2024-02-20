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

#include "SHA_AES.h"
#include "crypto/aes.h"
#include "crypto/sha2.h"

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

void versat_init(int);

// McEliece
#include "api.h"
#include "arena.h"
void nist_kat_init(unsigned char *entropy_input, unsigned char *personalization_string, int security_strength);

#include "versat_accel.h"

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

  versat_init(VERSAT0_BASE);

  printf("\n\n\nHello world!\n\n\n");

#if 0
  // SHA test
  {
    unsigned char msg_64[] = { 0x5a, 0x86, 0xb7, 0x37, 0xea, 0xea, 0x8e, 0xe9, 0x76, 0xa0, 0xa2, 0x4d, 0xa6, 0x3e, 0x7e, 0xd7, 0xee, 0xfa, 0xd1, 0x8a, 0x10, 0x1c, 0x12, 0x11, 0xe2, 0xb3, 0x65, 0x0c, 0x51, 0x87, 0xc2, 0xa8, 0xa6, 0x50, 0x54, 0x72, 0x08, 0x25, 0x1f, 0x6d, 0x42, 0x37, 0xe6, 0x61, 0xc7, 0xbf, 0x4c, 0x77, 0xf3, 0x35, 0x39, 0x03, 0x94, 0xc3, 0x7f, 0xa1, 0xa9, 0xf9, 0xbe, 0x83, 0x6a, 0xc2, 0x85, 0x09 };
    static const int HASH_SIZE = (256/8);

    InitVersatSHA();

    unsigned char versat_digest[256];
    unsigned char software_digest[256];
    for(int i = 0; i < 256; i++){
      versat_digest[i] = 0;
      software_digest[i] = 0;
    }

    VersatSHA(versat_digest,msg_64,64);
    sha256(software_digest,msg_64,64);

    char versat_buffer[2048];
    char software_buffer[2048];
    GetHexadecimal((char*) versat_digest,versat_buffer, HASH_SIZE);
    GetHexadecimal((char*) software_digest,software_buffer, HASH_SIZE);

    printf("Good:     %s\n","42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa"); 
    printf("Versat:   %s\n",versat_buffer);
    printf("Software: %s\n",software_buffer);
  }

  {
    uint8_t key[128] = {};
    uint8_t plain[128] = {};

    int keyIndex = HexStringToHex((char*) key,"cc22da787f375711c76302bef0979d8eddf842829c2b99ef3dd04e23e54cc24b");
    int plainIndex = HexStringToHex((char*) plain,"ccc62c6b0a09a671d64456818db29a4d");

    uint8_t versat_result[AES_BLK_SIZE] = {};
    uint8_t software_result[AES_BLK_SIZE] = {};

    InitVersatAES();

    VersatAES(versat_result,plain,key);
    VersatAES(software_result,plain,key);

    char versat_buffer[2048];
    char software_buffer[2048];
    GetHexadecimal((char*) versat_result,versat_buffer, AES_BLK_SIZE);
    GetHexadecimal((char*) software_result,software_buffer, AES_BLK_SIZE);
    printf("Good:     %s\n","df8634ca02b13a125b786e1dce90658b");
    printf("Versat:   %s\n",versat_buffer);
    printf("Software: %s\n",software_buffer);
  }  
#endif

  // McEliece
  {
    unsigned char public_key[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES];
    unsigned char secret_key[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES];

    unsigned char seed[48];

    HexStringToHex(seed,"061550234D158C5EC95595FE04EF7A25767F2E24CC2BC479D09D86DC9ABCFDE7056A8C266F9EF97ED08541DBD2E1FFA1");

    InitArena(4*1024*1024); // 4 megabytes should suffice

    nist_kat_init(seed, NULL, 256);
        
    // McEliece Key Pair
    PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair(public_key, secret_key);

    unsigned char public_key_buffer[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES * 2 + 1];
    unsigned char secret_key_buffer[PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES * 2 + 1];

    GetHexadecimal(public_key,public_key_buffer,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES);
    GetHexadecimal(secret_key,secret_key_buffer,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);

    public_key_buffer[100] = '\0';
    secret_key_buffer[100] = '\0';

    printf("%s\n",public_key_buffer);
    printf("%s\n","C5ED9AF0EEA0D4ADEA66D1A2A2F614E05500FB4344221FAA9135B50600BB8C5652C79FA603A2BC60EE8481D457C2CD81B21C\n");
    printf("%s\n",secret_key_buffer);
    printf("%s\n","5B815C890117893D8BB8E886F63A78CE2D5F58342D703348CB95539E14B9A719FFFFFFFF00000000F7066E0E5103160E7600\n");
  }

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
      clint_setCmp(CLINT0_BASE, MTIMER_SECONDS_TO_CLOCKS(WAIT_TIME)+(uint32_t)timestamp, 0);
      break;
    }
  }
}
#pragma GCC pop_options
