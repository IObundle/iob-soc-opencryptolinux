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

#include "stdbool.h"

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

#include "string.h"

typedef struct{
  char* str;
  int size;
} String;

#define STRING(str) (String){str,strlen(str)}

char* SearchAndAdvance(char* ptr,String str){
  char* firstChar = strstr(ptr,str.str);
  if(firstChar == NULL){
    return NULL;
  }

  char* advance = firstChar + str.size;
  return advance;
}

int ParseNumber(char* ptr){
  int count = 0;

  while(ptr != NULL){
    char ch = *ptr;

    if(ch >= '0' && ch <= '9'){
      count *= 10;
      count += ch - '0';
      ptr += 1;
      continue;
    }

    break;
  }

  return count;
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

  versat_init(VERSAT0_BASE);

  printf("\n\n\nHello world!\n\n\n");

  InitArena(16*1024*1024); // 16 megabytes should suffice

#if 1
  // SHA test
  {
    int mark = MarkArena();

    InitVersatSHA();

    uint32_t file_size = uart_recvfile_ethernet("../../tests/SHA256ShortMsg.rsp");
    char* testFile = PushArray(file_size,char);
    eth_rcv_file(testFile,file_size);

    static const int HASH_SIZE = (256/8);

    char* ptr = testFile;
    int goodTests = 0;
    int tests = 0;
    while(1){
      int testMark = MarkArena();

      ptr = SearchAndAdvance(ptr,STRING("LEN = "));
      if(ptr == NULL){
        break;
      }

      int len = ParseNumber(ptr);

      ptr = SearchAndAdvance(ptr,STRING("MSG = "));
      if(ptr == NULL){ // Note: It's only a error if any check after the first one fails, because we are assuming that if the first passes then that must mean that the rest should pass as well.
        printf("SHA early exit. Something wrong with testfile\n");
        break;
      }

      unsigned char* message = PushArray(len,unsigned char);
      int bytes = HexStringToHex(message,ptr);

      ptr = SearchAndAdvance(ptr,STRING("MD = "));
      if(ptr == NULL){
        printf("SHA early exit. Something wrong with testfile\n");
        break;
      }
    
      char* expected = ptr;

      unsigned char versat_digest[256];
      unsigned char software_digest[256];
      for(int i = 0; i < 256; i++){
        versat_digest[i] = 0;
        software_digest[i] = 0;
      }

      VersatSHA(versat_digest,message,len / 8);
      sha256(software_digest,message,len / 8);

      bool good = true;
      for(int i = 0; i < 256; i++){
        if(versat_digest[i] != software_digest[i]){
          good = false;
          break;
        }
      }

      if(good){
        goodTests += 1;
      } else {
        char versat_buffer[2048];
        char software_buffer[2048];
        GetHexadecimal((char*) versat_digest,versat_buffer, HASH_SIZE);
        GetHexadecimal((char*) software_digest,software_buffer, HASH_SIZE);

        printf("SHA Test %02d: Error\n",tests);
        printf("  Expected: %.64s\n",expected); 
        printf("  Software: %s\n",software_buffer);
        printf("  Versat:   %s\n",versat_buffer);
      }

      tests += 1;
      PopArena(testMark);
    }

    printf("\n\nSHA tests: %d passed out of %d\n\n",goodTests,tests);
  }
#endif

#if 1
  // AES test
  {
    int mark = MarkArena();

    InitVersatAES();

    uint32_t file_size = uart_recvfile_ethernet("../../tests/AESECB256.rsp");
    char* testFile = PushArray(file_size,char);
    eth_rcv_file(testFile,file_size);

    char* ptr = testFile;
    int goodTests = 0;
    int tests = 0;
    while(1){
      int testMark = MarkArena();

      ptr = SearchAndAdvance(ptr,STRING("COUNT = "));
      if(ptr == NULL){
        break;
      }

      int count = ParseNumber(ptr);

      ptr = SearchAndAdvance(ptr,STRING("KEY = "));
      if(ptr == NULL){
        printf("AES early exit. Something wrong with testfile\n");
        break;
      }

      unsigned char* key = PushArray(32 + 1,unsigned char);
      HexStringToHex(key,ptr);

      ptr = SearchAndAdvance(ptr,STRING("PLAINTEXT = "));
      if(ptr == NULL){
        printf("AES early exit. Something wrong with testfile\n");
        break;
      }
    
      unsigned char* plain = PushArray(16 + 1,unsigned char);
      HexStringToHex(plain,ptr);

      ptr = SearchAndAdvance(ptr,STRING("CIPHERTEXT = "));
      if(ptr == NULL){
        printf("AES early exit. Something wrong with testfile\n");
        break;
      }

      char* cypher = ptr;

      uint8_t versat_result[AES_BLK_SIZE] = {};
      uint8_t software_result[AES_BLK_SIZE] = {};

      VersatAES(versat_result,plain,key);
      
      struct AES_ctx ctx;
      AES_init_ctx(&ctx,key);
      memcpy(software_result,plain,AES_BLK_SIZE);
      AES_ECB_encrypt(&ctx,software_result);

      bool good = true;
      for(int i = 0; i < AES_BLK_SIZE; i++){
        if(versat_result[i] != software_result[i]){
          good = false;
          break;
        }
      }

      if(good){
        goodTests += 1;
      } else {
        char versat_buffer[2048];
        char software_buffer[2048];
        GetHexadecimal((char*) versat_result,versat_buffer, AES_BLK_SIZE);
        GetHexadecimal((char*) software_result,software_buffer, AES_BLK_SIZE);

        printf("AES Test %02d: Error\n",tests);
        printf("  Expected: %.32s\n",cypher); 
        printf("  Software: %s\n",software_buffer);
        printf("  Versat:   %s\n",versat_buffer);
      }

      tests += 1;
      PopArena(testMark);
    }

    printf("\n\nAES tests: %d passed out of %d\n\n",goodTests,tests);
  }
#endif

#if 1
  // McEliece test
  {
    int mark = MarkArena();

    unsigned char* public_key = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES,unsigned char);
    unsigned char* secret_key = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES,unsigned char);

    uint32_t file_size = uart_recvfile_ethernet("../../tests/McElieceRound4kat_kem.rsp");
    char* testFile = PushArray(file_size,char);
    eth_rcv_file(testFile,file_size);

    char* ptr = testFile;
    int goodTests = 0;
    int tests = 0;
    while(1){
      int testMark = MarkArena();

      ptr = SearchAndAdvance(ptr,STRING("COUNT = "));
      if(ptr == NULL){
        break;
      }

      int count = ParseNumber(ptr);

      ptr = SearchAndAdvance(ptr,STRING("SEED = "));
      if(ptr == NULL){
        printf("McEliece early exit. Something wrong with testfile\n");
        break;
      }

      unsigned char seed[49];
      HexStringToHex(seed,ptr);

      ptr = SearchAndAdvance(ptr,STRING("PK = "));
      if(ptr == NULL){
        printf("McEliece early exit. Something wrong with testfile\n");
        break;
      }
    
      char* pk = ptr;

      ptr = SearchAndAdvance(ptr,STRING("SK = "));
      if(ptr == NULL){
        printf("McEliece early exit. Something wrong with testfile\n");
        break;
      }

      char* sk = ptr;

      nist_kat_init(seed, NULL, 256);
      PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair(public_key, secret_key);

      unsigned char* public_key_hex = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES * 2 + 1,char);
      unsigned char* secret_key_hex = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES * 2 + 1,char);

      GetHexadecimal(public_key,public_key_hex,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES);
      GetHexadecimal(secret_key,secret_key_hex,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);

      bool good = true;
      for(int i = 0; i < PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES; i++){
        if(public_key_hex[i] != pk[i]){
          good = false;
          break;
        }
      }
      for(int i = 0; i < PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES; i++){
        if(secret_key_hex[i] != sk[i]){
          good = false;
          break;
        }
      }

      if(good){
        goodTests += 1;
      } else {
        printf("McEliece Test %02d: Error\n",tests);
        printf("  Expected Public (first 32 chars): %.32s\n",pk); 
        printf("  Expected Secret (first 32 chars): %.32s\n",sk); 
        printf("  Got Public (first 32 chars):      %.32s\n",public_key_hex);
        printf("  Got Secret (first 32 chars):      %.32s\n",secret_key_hex);
      }

      tests += 1;
      PopArena(testMark);

      // McEliece takes a decent amount of time
      if(tests >= 2){
        break;
      }
    }

    printf("\n\nMcEliece tests: %d passed out of %d\n\n",goodTests,tests);
  }
#endif

#if 0
  // McEliece
  {
    int mark = MarkArena();

    unsigned char* public_key = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES,unsigned char);
    unsigned char* secret_key = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES,unsigned char);

    unsigned char seed[48];

    HexStringToHex(seed,"061550234D158C5EC95595FE04EF7A25767F2E24CC2BC479D09D86DC9ABCFDE7056A8C266F9EF97ED08541DBD2E1FFA1");

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

    PopArena(mark);
  }
#endif

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
