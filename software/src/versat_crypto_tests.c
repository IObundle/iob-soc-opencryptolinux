#include "versat_crypto_tests.h"

#include "stdbool.h"

#include "iob-uart16550.h"
#include "printf.h"
#include "iob-eth.h"

#include "versat_crypto.h"
#include "crypto/aes.h"
#include "crypto/sha2.h"

// McEliece
#include "api.h"
#include "arena.h"
void nist_kat_init(unsigned char *entropy_input, unsigned char *personalization_string, int security_strength);

#include "string.h"

void versat_init(int);

void InitializeCryptoSide(int versatAddress){
  versat_init(versatAddress);
  InitArena(16*1024*1024); // 16 megabytes should suffice. Arena memory used by crypto algorithms, both by software and Versat impl.
}

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

String PushFileFromEthernet(const char* filepath){
  uint32_t file_size = uart_recvfile_ethernet(filepath);
  char* testFile = PushArray(file_size + 1,char);
  eth_rcv_file(testFile,file_size);
  testFile[file_size] = '\0';

  return (String){.str=testFile,.size=file_size};
}

int VersatSHATests(){
  int mark = MarkArena();

  int start = GetTime();
  InitVersatSHA();
  int end = GetTime();

  printf("Init versat SHA took: %d\n",end - start);

  String content = PushFileFromEthernet("../../software/versat/tests/SHA256ShortMsg.rsp");

  static const int HASH_SIZE = (256/8);

  char* ptr = content.str;
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

    int start = GetTime();
    VersatSHA(versat_digest,message,len / 8);
    int middle = GetTime();
    sha256(software_digest,message,len / 8);
    int end = GetTime();

    printf("Versat   took: %d\n",middle - start);
    printf("Software took: %d\n",end - middle);

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
  PopArena(mark);

  return (goodTests == tests) ? 0 : 1;
}

int VersatAESTests(){
  int mark = MarkArena();

  InitVersatAES();

  String content = PushFileFromEthernet("../../software/versat/tests/AESECB256.rsp");

  char* ptr = content.str;
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

    int start = GetTime();
    VersatAES(versat_result,plain,key);
    int middle = GetTime();
    
    struct AES_ctx ctx;
    AES_init_ctx(&ctx,key);
    memcpy(software_result,plain,AES_BLK_SIZE);
    AES_ECB_encrypt(&ctx,software_result);
    int end = GetTime();

    printf("Versat   took: %d\n",middle - start);
    printf("Software took: %d\n",end - middle);

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
  PopArena(mark);

  return (goodTests == tests) ? 0 : 1;
}

int VersatMcElieceTests(){
  int mark = MarkArena();

  unsigned char* public_key = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES,unsigned char);
  unsigned char* secret_key = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES,unsigned char);

  String content = PushFileFromEthernet("../../software/versat/tests/McElieceRound4kat_kem.rsp");
  char* ptr = content.str;
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
  
    char* good_pk = ptr;

    ptr = SearchAndAdvance(ptr,STRING("SK = "));
    if(ptr == NULL){
      printf("McEliece early exit. Something wrong with testfile\n");
      break;
    }

    char* good_sk = ptr;

    nist_kat_init(seed, NULL, 256);
    VersatMcEliece(public_key, secret_key);

    // Software only implementation is slow and we are already comparing to KAT anyway and so, for McEliece, we skipping software implementation test of McEliece.
    //PQCLEAN_MCELIECE348864_CLEAN_crypto_kem_keypair(public_key, secret_key);

    unsigned char* public_key_hex = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES * 2 + 1,char);
    unsigned char* secret_key_hex = PushArray(PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES * 2 + 1,char);

    GetHexadecimal(public_key,public_key_hex,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES);
    GetHexadecimal(secret_key,secret_key_hex,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);

    bool good = true;
    for(int i = 0; i < PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES; i++){
      if(public_key_hex[i] != good_pk[i]){
        good = false;
        break;
      }
    }
    for(int i = 0; i < PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES; i++){
      if(secret_key_hex[i] != good_sk[i]){
        good = false;
        break;
      }
    }

    if(good){
      goodTests += 1;
    } else {
      printf("McEliece Test %02d: Error\n",tests);
      printf("  Expected Public (first 32 chars): %.32s\n",good_pk); 
      printf("  Expected Secret (first 32 chars): %.32s\n",good_sk); 
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
  PopArena(mark);

  return (goodTests == tests) ? 0 : 1;
}
