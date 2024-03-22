#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <stdio.h>

#include "arena.h"
#include "versat.h"
#include "versat_accel.h"
#include "unitConfiguration.h"

#include <stdint.h>
#include <string.h>

typedef struct{
   int fd; 
} PhysicalAllocator;

PhysicalAllocator OpenPhysicalAllocator(){
   PhysicalAllocator res = {};
   res.fd = open("/dev/phys",O_RDWR | O_SYNC);
   return res;
}

bool PhysicalAllocatorError(PhysicalAllocator allocator){
   bool res = allocator.fd == 0;
   return res;
}

bool ClosePhysicalAllocator(PhysicalAllocator alloc){
   int result = close(alloc.fd);
   bool res = (result == -1);
   return res;
}

typedef struct{
   void* virtualMemBase;
   void* physicalAddressBase;
   int size;
} PhysicalBuffer;

bool PhysicalBufferError(PhysicalBuffer buffer){
   bool res = (buffer.virtualMemBase == NULL);
   return res;
}

PhysicalBuffer AllocatePhysicalBuffer(PhysicalAllocator allocator,int size){
   PhysicalBuffer res = {};
   int fd = allocator.fd;
   void* virtual = mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);

   if(virtual == MAP_FAILED){
      printf("MMap failed\n");
      return res;
   }

   void* physicalAddress;
   int result = ioctl(fd,0,&physicalAddress);

   if(result == -1){
      printf("IO failed\n");
      return res;
   }

   res.virtualMemBase = virtual;
   res.physicalAddressBase = physicalAddress;
   res.size = 4096;

   return res;
}

void* ConvertVirtToPhys(PhysicalBuffer buffer,const void* virtual){
   char* viewVirt = (char*) virtual;
   char* viewVirtBase = (char*) buffer.virtualMemBase;
   char* viewPhys = (char*) buffer.physicalAddressBase;

   int offset = viewVirt - viewVirtBase;
   // Somehow virtual is outside allocated buffer. Program should probably give some error before reaching here, but we double check it anyway.
   if(offset > buffer.size){
      return NULL;
   }

   void* res = (void*) (viewPhys + offset);
   return res;
}

typedef struct{
  char* str;
  int size;
} String;

#define STRING(str) (String){str,strlen(str)}

typedef struct {
  int initTime;
  int tests;
  int goodTests;
  int versatTimeAccum;
  int softwareTimeAccum;
  int earlyExit;
} TestState;

void clear_cache(){
  // Delay to ensure all data is written to memory
  for ( unsigned int i = 0; i < 10; i++)asm volatile("nop");
  // Flush VexRiscv CPU internal cache
  asm volatile(".word 0x500F" ::: "memory");
}

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

static uint32_t initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint32_t kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint32_t kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint32_t kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint32_t kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

// GLOBALS
static bool initVersat = false;

static void store_bigendian_32(uint8_t *x, uint32_t u) {
   x[3] = (uint8_t) u;
   u >>= 8;
   x[2] = (uint8_t) u;
   u >>= 8;
   x[1] = (uint8_t) u;
   u >>= 8;
   x[0] = (uint8_t) u;
}

void InitVersatSHA(){
   CryptoAlgosConfig* config = (CryptoAlgosConfig*) accelConfig;
   SHAConfig* sha = &config->SHA;

   *sha = (SHAConfig){0};
   ActivateMergedAccelerator(SHA);
   ConfigureSimpleVRead(&sha->MemRead,16,NULL);

   ACCEL_Constants_mem_iterA = 1;
   ACCEL_Constants_mem_incrA = 1;
   ACCEL_Constants_mem_perA = 16;
   ACCEL_Constants_mem_dutyA = 16;
   ACCEL_Constants_mem_startA = 0;
   ACCEL_Constants_mem_shiftA = 0;

   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_cMem0_mem_addr,ii,kConstants[0][ii]);
   }
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_cMem1_mem_addr,ii,kConstants[1][ii]);
   }
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_cMem2_mem_addr,ii,kConstants[2][ii]);
   }
   for(int ii = 0; ii < 16; ii++){
      VersatUnitWrite(TOP_cMem3_mem_addr,ii,kConstants[3][ii]);
   }

   ACCEL_TOP_Swap_enabled = 1;
}

static size_t versat_crypto_hashblocks_sha256(PhysicalBuffer buffer, const uint8_t* ptr, size_t inlen) {
   while (inlen >= 64) {
      ACCEL_TOP_MemRead_key_ext_addr = (iptr) ConvertVirtToPhys(buffer,(void*) ptr);
   
      // Loads data + performs work
      RunAccelerator(1);

      if(!initVersat){
         VersatUnitWrite(TOP_State_s0_reg_addr,0,initialStateValues[0]);
         VersatUnitWrite(TOP_State_s1_reg_addr,0,initialStateValues[1]);
         VersatUnitWrite(TOP_State_s2_reg_addr,0,initialStateValues[2]);
         VersatUnitWrite(TOP_State_s3_reg_addr,0,initialStateValues[3]);
         VersatUnitWrite(TOP_State_s4_reg_addr,0,initialStateValues[4]);
         VersatUnitWrite(TOP_State_s5_reg_addr,0,initialStateValues[5]);
         VersatUnitWrite(TOP_State_s6_reg_addr,0,initialStateValues[6]);
         VersatUnitWrite(TOP_State_s7_reg_addr,0,initialStateValues[7]);
         initVersat = true;
      }

      ptr += 64;
      inlen -= 64;
   }

   return inlen;
}

void VersatSHA(uint8_t *out, PhysicalBuffer buffer, size_t inlen) {
   uint8_t padded[128];
   uint64_t bytes = inlen;
   uint8_t *in = buffer.virtualMemBase;

   versat_crypto_hashblocks_sha256(buffer,in,inlen);
   in += inlen;
   inlen &= 63;
   in -= inlen;

#if 0
   for (size_t i = 0; i < inlen; ++i) {
      padded[i] = in[i];
   }
#endif
   in[inlen] = 0x80;

   if (inlen < 56) {
      for (size_t i = inlen + 1; i < 56; ++i) {
         in[i] = 0;
    }  
      in[56] = (uint8_t) (bytes >> 53);
      in[57] = (uint8_t) (bytes >> 45);
      in[58] = (uint8_t) (bytes >> 37);
      in[59] = (uint8_t) (bytes >> 29);
      in[60] = (uint8_t) (bytes >> 21);
      in[61] = (uint8_t) (bytes >> 13);
      in[62] = (uint8_t) (bytes >> 5);
      in[63] = (uint8_t) (bytes << 3);
      versat_crypto_hashblocks_sha256(buffer,in, 64);
   } else {
      for (size_t i = inlen + 1; i < 120; ++i) {
         in[i] = 0;
      }
      in[120] = (uint8_t) (bytes >> 53);
      in[121] = (uint8_t) (bytes >> 45);
      in[122] = (uint8_t) (bytes >> 37);
      in[123] = (uint8_t) (bytes >> 29);
      in[124] = (uint8_t) (bytes >> 21);
      in[125] = (uint8_t) (bytes >> 13);
      in[126] = (uint8_t) (bytes >> 5);
      in[127] = (uint8_t) (bytes << 3);
      versat_crypto_hashblocks_sha256(buffer,in, 128);
   }

   RunAccelerator(1);

   store_bigendian_32(&out[0*4],(uint32_t) VersatUnitRead(TOP_State_s0_reg_addr,0));
   store_bigendian_32(&out[1*4],(uint32_t) VersatUnitRead(TOP_State_s1_reg_addr,0));
   store_bigendian_32(&out[2*4],(uint32_t) VersatUnitRead(TOP_State_s2_reg_addr,0));
   store_bigendian_32(&out[3*4],(uint32_t) VersatUnitRead(TOP_State_s3_reg_addr,0));
   store_bigendian_32(&out[4*4],(uint32_t) VersatUnitRead(TOP_State_s4_reg_addr,0));
   store_bigendian_32(&out[5*4],(uint32_t) VersatUnitRead(TOP_State_s5_reg_addr,0));
   store_bigendian_32(&out[6*4],(uint32_t) VersatUnitRead(TOP_State_s6_reg_addr,0));
   store_bigendian_32(&out[7*4],(uint32_t) VersatUnitRead(TOP_State_s7_reg_addr,0));

   initVersat = false; // At the end of each run, reset
}

static char HexToInt(char ch){
   if('0' <= ch && ch <= '9'){
      return (ch - '0');
   } else if('a' <= ch && ch <= 'f'){
      return ch - 'a' + 10;
   } else if('A' <= ch && ch <= 'F'){
      return ch - 'A' + 10;
   } else {
      return 0x7f;
   }
}

// Make sure that buffer is capable of storing the whole thing. Returns number of bytes inserted
int HexStringToHex(char* buffer,const char* str){
   int inserted = 0;
   for(int i = 0; ; i += 2){
      char upper = HexToInt(str[i]);
      char lower = HexToInt(str[i+1]);

      if(upper >= 16 || lower >= 16){
         if(upper < 16){ // Upper is good but lower is not
            printf("Warning: HexString was not divisible by 2\n");
         }
         break;
      }

      buffer[inserted++] = upper * 16 + lower;
   }

   return inserted;
}

char GetHexadecimalChar(unsigned char value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'A' + (value - 10);
  }
}

char* GetHexadecimal(const char* text,char* buffer,int str_size){
  int i = 0;
  unsigned char* view = (unsigned char*) text;
  for(; i< str_size; i++){
    buffer[i*2] = GetHexadecimalChar(view[i] / 16);
    buffer[i*2+1] = GetHexadecimalChar(view[i] % 16);
  }

  buffer[i*2] = '\0';

  return buffer;
}

static TestState VersatCommonSHATests(String content){
  TestState result = {};

  int mark = MarkArena();

  InitVersatSHA();

  static const int HASH_SIZE = (256/8);

  PhysicalAllocator alloc = OpenPhysicalAllocator();
  if(PhysicalAllocatorError(alloc)){
    printf("Failed opening allocator\n");
    return (TestState){};
  }

  PhysicalBuffer buffer = AllocatePhysicalBuffer(alloc,4096);
  if(PhysicalBufferError(buffer)){
    printf("Failed allocating buffer\n");
    return (TestState){};
  }

  char* ptr = content.str;
  while(1){
    int testMark = MarkArena();

    ptr = SearchAndAdvance(ptr,STRING("LEN = "));
    if(ptr == NULL){
      break;
    }

    int len = ParseNumber(ptr);

    ptr = SearchAndAdvance(ptr,STRING("MSG = "));
    if(ptr == NULL){ // Note: It's only a error if any check after the first one fails, because we are assuming that if the first passes then that must mean that the rest should pass as well.
      result.earlyExit = 1;
      break;
    }

    unsigned char* message = PushArray(len,unsigned char);
    int bytes = HexStringToHex(buffer.virtualMemBase,ptr);

    unsigned char versat_digest[256];
    for(int i = 0; i < 256; i++){
      versat_digest[i] = 0;
    }

    printf("Versat: %s\n",versat_digest);
    VersatSHA(versat_digest,buffer,len / 8);
    char versat_buffer[2048];
    GetHexadecimal((char*) versat_digest,versat_buffer, HASH_SIZE);

    printf("Versat: %s\n",versat_buffer);

    result.tests += 1;
    PopArena(testMark);
  }

  PopArena(mark);

  ClosePhysicalAllocator(alloc);

  return result;
}

int main(int argc,const char* argv){
   int mem = -1;
   {
      mem = open("/dev/mem",O_RDWR | O_SYNC);
      if(mem == -1){
         puts("Open mem is -1\n");
         return -1;
      }
      void* res = mmap(0, versatAddressSpace, PROT_READ | PROT_WRITE, MAP_SHARED,mem,VERSAT_ADDRESS);   

      if(res == MAP_FAILED){
         puts("Mmap failed\n");
         printf("%d",errno);
         return -1;
      }

      versat_init((int) res);
   }

   InitArena(4096);

   String content = STRING("LEN = 128\nMSG = 0A27847CDC98BD6F62220B046EDD762B\n");
   printf("%.*s",content.size,content.str);
   VersatCommonSHATests(content); // 80C25EC1600587E7F28B18B1B18E3CDC89928E39CAB3BC25E4D4A4C139BCEDC4

   return 0;
}