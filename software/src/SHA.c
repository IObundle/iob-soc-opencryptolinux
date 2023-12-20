#include "versat_accel.h"
#include "unitConfiguration.h"

#include "iob-uart.h"
#include "iob_str.h"
#include "printf.h"

#include "stdlib.h"
#include "stdint.h"

#define nullptr NULL

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

static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen) {
   while (inlen >= 64) {
      ACCEL_TOP_MemRead_ext_addr = (iptr) in;

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

      in += 64;
      inlen -= 64;
   }

   return inlen;
}

void ClearCache(){
  int size = 1024 * 16;
  char* m = (char*) malloc(size); // Should not use malloc but some random fixed ptr in embedded. No use calling malloc since we can always read at any point in memory without worrying about memory protection.

  // volatile and asm are used to make sure that gcc does not optimize away this loop that appears to do nothing
  volatile int val = 0;
  for(int i = 0; i < size; i += 16){
    val += m[i];
    __asm__ volatile("" : "+g" (val) : :);
  }
  free(m);
}

void VersatSHA(uint8_t *out, const uint8_t *in, size_t inlen) {
   uint8_t padded[128];
   uint64_t bytes = inlen;

   versat_crypto_hashblocks_sha256(in, inlen);
   in += inlen;
   inlen &= 63;
   in -= inlen;

   for (size_t i = 0; i < inlen; ++i) {
      padded[i] = in[i];
   }
   padded[inlen] = 0x80;

   if (inlen < 56) {
      for (size_t i = inlen + 1; i < 56; ++i) {
         padded[i] = 0;
      }
      padded[56] = (uint8_t) (bytes >> 53);
      padded[57] = (uint8_t) (bytes >> 45);
      padded[58] = (uint8_t) (bytes >> 37);
      padded[59] = (uint8_t) (bytes >> 29);
      padded[60] = (uint8_t) (bytes >> 21);
      padded[61] = (uint8_t) (bytes >> 13);
      padded[62] = (uint8_t) (bytes >> 5);
      padded[63] = (uint8_t) (bytes << 3);
      versat_crypto_hashblocks_sha256(padded, 64);
   } else {
      for (size_t i = inlen + 1; i < 120; ++i) {
         padded[i] = 0;
      }
      padded[120] = (uint8_t) (bytes >> 53);
      padded[121] = (uint8_t) (bytes >> 45);
      padded[122] = (uint8_t) (bytes >> 37);
      padded[123] = (uint8_t) (bytes >> 29);
      padded[124] = (uint8_t) (bytes >> 21);
      padded[125] = (uint8_t) (bytes >> 13);
      padded[126] = (uint8_t) (bytes >> 5);
      padded[127] = (uint8_t) (bytes << 3);
      versat_crypto_hashblocks_sha256(padded, 128);
   }

   RunAccelerator(1);

   ClearCache();

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

void InitVersatSHA(){
   static bool initialized = false;

   if(initialized){
      return;
   }

   ConfigureSimpleVRead(&((SHAConfig*) accelConfig)->MemRead,16,nullptr);
   //ConfigureSimpleVRead(read,16,nullptr); // read address is configured before accelerator run

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

char GetHexadecimalChar(int value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'a' + (value - 10);
  }
}

unsigned char* GetHexadecimal(const unsigned char* text, int str_size){
  static unsigned char buffer[2048+1];
  int i;

  for(i = 0; i< str_size; i++){
    int ch = (int) ((unsigned char) text[i]);

    buffer[i*2] = GetHexadecimalChar(ch / 16);
    buffer[i*2+1] = GetHexadecimalChar(ch % 16);
  }

  buffer[(i)*2] = '\0';

  return buffer;
}

void VersatSHATest(){
   unsigned char msg_64[] = { 0x5a, 0x86, 0xb7, 0x37, 0xea, 0xea, 0x8e, 0xe9, 0x76, 0xa0, 0xa2, 0x4d, 0xa6, 0x3e, 0x7e, 0xd7, 0xee, 0xfa, 0xd1, 0x8a, 0x10, 0x1c, 0x12, 0x11, 0xe2, 0xb3, 0x65, 0x0c, 0x51, 0x87, 0xc2, 0xa8, 0xa6, 0x50, 0x54, 0x72, 0x08, 0x25, 0x1f, 0x6d, 0x42, 0x37, 0xe6, 0x61, 0xc7, 0xbf, 0x4c, 0x77, 0xf3, 0x35, 0x39, 0x03, 0x94, 0xc3, 0x7f, 0xa1, 0xa9, 0xf9, 0xbe, 0x83, 0x6a, 0xc2, 0x85, 0x09 };
   static const int HASH_SIZE = (256/8);
   
   InitVersatSHA();

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   VersatSHA(digest,msg_64,64);

   printf("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa\n");
   printf("%s\n",GetHexadecimal(digest, HASH_SIZE));
}