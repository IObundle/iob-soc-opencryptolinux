#include <stdio.h>

#include "arena.h"
#include "versatCrypto.h"

#include <stdint.h>
#include <string.h>

#if 1

#undef SHA 
#undef AES

#define SHA
//#define AES

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


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

#ifdef SHA

#if 0
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

#endif

static TestState VersatCommonSHATests(Arena* test,String content){
  TestState result = {};

  int mark = MarkArena(test);

  static const int HASH_SIZE = (256/8);

  char* ptr = content.str;
  while(1){
    int testMark = MarkArena(test);

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

    VersatBuffer* buffer = BeginSHA();

    int bytes = HexStringToHex(buffer->mem,ptr);
    buffer->size = 0;//len / 8;

    printf("S: %d\n",buffer->size);
    unsigned char versat_digest[32] = {};
    EndSHA(buffer,versat_digest);

    char versat_buffer[2048];
    GetHexadecimal((char*) versat_digest,versat_buffer, HASH_SIZE);

    printf("Versat: %s\n",versat_buffer);

    result.tests += 1;
    PopArena(test,testMark);
  }

  PopArena(test,mark);

  return result;
}

#endif // SHA

#ifdef AES

// AES Sizes (bytes)
#define AES_BLK_SIZE (16)
#define AES_KEY_SIZE (32)

const uint8_t sbox[256] = {
   0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
   0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
   0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
   0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
   0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
   0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
   0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
   0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
   0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
   0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
   0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
   0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
   0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
   0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
   0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
   0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

const uint8_t mul2[] = {
   0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
   0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
   0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
   0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
   0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
   0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
   0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
   0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
   0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,
   0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,
   0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,
   0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,
   0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,
   0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,
   0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,
   0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5
};

const uint8_t mul3[] = {
   0x00,0x03,0x06,0x05,0x0c,0x0f,0x0a,0x09,0x18,0x1b,0x1e,0x1d,0x14,0x17,0x12,0x11,
   0x30,0x33,0x36,0x35,0x3c,0x3f,0x3a,0x39,0x28,0x2b,0x2e,0x2d,0x24,0x27,0x22,0x21,
   0x60,0x63,0x66,0x65,0x6c,0x6f,0x6a,0x69,0x78,0x7b,0x7e,0x7d,0x74,0x77,0x72,0x71,
   0x50,0x53,0x56,0x55,0x5c,0x5f,0x5a,0x59,0x48,0x4b,0x4e,0x4d,0x44,0x47,0x42,0x41,
   0xc0,0xc3,0xc6,0xc5,0xcc,0xcf,0xca,0xc9,0xd8,0xdb,0xde,0xdd,0xd4,0xd7,0xd2,0xd1,
   0xf0,0xf3,0xf6,0xf5,0xfc,0xff,0xfa,0xf9,0xe8,0xeb,0xee,0xed,0xe4,0xe7,0xe2,0xe1,
   0xa0,0xa3,0xa6,0xa5,0xac,0xaf,0xaa,0xa9,0xb8,0xbb,0xbe,0xbd,0xb4,0xb7,0xb2,0xb1,
   0x90,0x93,0x96,0x95,0x9c,0x9f,0x9a,0x99,0x88,0x8b,0x8e,0x8d,0x84,0x87,0x82,0x81,
   0x9b,0x98,0x9d,0x9e,0x97,0x94,0x91,0x92,0x83,0x80,0x85,0x86,0x8f,0x8c,0x89,0x8a,
   0xab,0xa8,0xad,0xae,0xa7,0xa4,0xa1,0xa2,0xb3,0xb0,0xb5,0xb6,0xbf,0xbc,0xb9,0xba,
   0xfb,0xf8,0xfd,0xfe,0xf7,0xf4,0xf1,0xf2,0xe3,0xe0,0xe5,0xe6,0xef,0xec,0xe9,0xea,
   0xcb,0xc8,0xcd,0xce,0xc7,0xc4,0xc1,0xc2,0xd3,0xd0,0xd5,0xd6,0xdf,0xdc,0xd9,0xda,
   0x5b,0x58,0x5d,0x5e,0x57,0x54,0x51,0x52,0x43,0x40,0x45,0x46,0x4f,0x4c,0x49,0x4a,
   0x6b,0x68,0x6d,0x6e,0x67,0x64,0x61,0x62,0x73,0x70,0x75,0x76,0x7f,0x7c,0x79,0x7a,
   0x3b,0x38,0x3d,0x3e,0x37,0x34,0x31,0x32,0x23,0x20,0x25,0x26,0x2f,0x2c,0x29,0x2a,
   0x0b,0x08,0x0d,0x0e,0x07,0x04,0x01,0x02,0x13,0x10,0x15,0x16,0x1f,0x1c,0x19,0x1a
};

void FillLookupTable(LookupTableAddr addr){
   VersatMemoryCopy(addr.addr,(void*) sbox,256 * sizeof(uint8_t));
}

void FillMul(LookupTableAddr addr,const uint8_t* mem,int size){
   VersatMemoryCopy(addr.addr,(void*) mem,size * sizeof(uint8_t)); // (int*) 
}

void FillRow(DoRowAddr addr){
   FillMul(addr.mul2_0,mul2,ARRAY_SIZE(mul2));
   FillMul(addr.mul2_1,mul2,ARRAY_SIZE(mul2));
   FillMul(addr.mul3_0,mul3,ARRAY_SIZE(mul3));
   FillMul(addr.mul3_1,mul3,ARRAY_SIZE(mul3));
}

void FillFirstLineKey(FirstLineKeyAddr addr){
   FillLookupTable(addr.b0);
   FillLookupTable(addr.b1);
}

void FillFourthLineKey(FourthLineKeyAddr addr){
   FillLookupTable(addr.b0);
   FillLookupTable(addr.b1);
}

void FillMixColumns(MixColumnsAddr addr){
   FillRow(addr.d0);
   FillRow(addr.d1);
   FillRow(addr.d2);
   FillRow(addr.d3);
}

void FillSBox(SBoxAddr addr){
   FillLookupTable(addr.s0);
   FillLookupTable(addr.s1);
   FillLookupTable(addr.s2);
   FillLookupTable(addr.s3);
   FillLookupTable(addr.s4);
   FillLookupTable(addr.s5);
   FillLookupTable(addr.s6);
   FillLookupTable(addr.s7);
}

void FillKeySchedule(KeySchedule256Addr addr){
   FillFirstLineKey(addr.s);
   FillFourthLineKey(addr.q);
}

void FillMainRound(MainRoundAddr addr){
   FillSBox(addr.subBytes);
   FillMixColumns(addr.mixColumns);
}

void FillRoundPairAndKey(RoundPairAndKeyAddr addr){
   FillMainRound(addr.round1);
   FillMainRound(addr.round2);
   FillKeySchedule(addr.key);
}

void byte_to_int(uint8_t *in, int *out, int size) {
   int i = 0;
   for(i=0; i<size; i++) {
      out[i] = (int) in[i];
   }
   return;
}

void int_to_byte(int *in, uint8_t *out, int size) {
   int i = 0;
   for(i=0; i<size; i++) {
      out[i] = (uint8_t) (in[i] & 0x0FF);
   }
   return;
}

void InitVersatAES() {
   CryptoAlgosConfig* config = (CryptoAlgosConfig*) accelConfig;
   ReadWriteAES256Config* aes = &config->ReadWriteAES256;

   *aes = (ReadWriteAES256Config){0};
   ActivateMergedAccelerator(ReadWriteAES256);

   CryptoAlgosAddr addr = ACCELERATOR_TOP_ADDR_INIT;

   FillRoundPairAndKey(addr.aes.mk0.roundPairAndKey);
   FillMainRound(addr.aes.round0);
   FillKeySchedule(addr.aes.key6);
   FillSBox(addr.aes.subBytes);

   aes->aes.rcon0.constant = 0x01;
   aes->aes.rcon1.constant = 0x02;
   aes->aes.rcon2.constant = 0x04;
   aes->aes.rcon3.constant = 0x08;
   aes->aes.rcon4.constant = 0x10;
   aes->aes.rcon5.constant = 0x20;
   aes->aes.rcon6.constant = 0x40;
}

void VersatAES(PhysicalBuffer* buffer,uint8_t *result, uint8_t *cypher, uint8_t *key) {
   Arena physArenaInst = {};
   Arena* physArena = &physArenaInst;
   physArena->ptr = buffer->virtualMemBase;
   physArena->allocated = buffer->size;

   int* cypher_int = PushArray(physArena,AES_BLK_SIZE * 2,int);
   int* key_int = PushArray(physArena,AES_KEY_SIZE * 2,int);
   int* result_int = PushArray(physArena,AES_BLK_SIZE * 2,int);

   byte_to_int(cypher, cypher_int, AES_BLK_SIZE);
   byte_to_int(key, key_int, AES_KEY_SIZE);

   CryptoAlgosConfig* config = (CryptoAlgosConfig*) accelConfig;
   ReadWriteAES256Config* con = (ReadWriteAES256Config*) &config->ReadWriteAES256;

   clear_cache();

   ConfigureSimpleVRead(&con->cypher,AES_BLK_SIZE,ConvertVirtToPhys(*buffer,cypher_int));
   ConfigureSimpleVRead(&con->key,AES_KEY_SIZE,ConvertVirtToPhys(*buffer,key_int));
   ConfigureSimpleVWrite(&con->results,AES_BLK_SIZE,ConvertVirtToPhys(*buffer,result_int));

   RunAccelerator(3);

   clear_cache();

   int_to_byte(result_int, result, AES_BLK_SIZE);

   return;
}

TestState VersatCommonAESTests(Arena* test,String content){
  TestState result = {};

  int mark = MarkArena(test);

  InitVersatAES();

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
    int testMark = MarkArena(test);

    ptr = SearchAndAdvance(ptr,STRING("COUNT = "));
    if(ptr == NULL){
      break;
    }

    int count = ParseNumber(ptr);

    ptr = SearchAndAdvance(ptr,STRING("KEY = "));
    if(ptr == NULL){
      result.earlyExit = 1;
      break;
    }

    uint8_t key[AES_KEY_SIZE] = {};
    HexStringToHex(key,ptr);

    ptr = SearchAndAdvance(ptr,STRING("PLAINTEXT = "));
    if(ptr == NULL){
      result.earlyExit = 1;
      break;
    }
  
    uint8_t plain[AES_BLK_SIZE] = {};
    HexStringToHex(plain,ptr);

    uint8_t versat_result[AES_BLK_SIZE] = {};

    VersatAES(&buffer,versat_result,plain,key);
    
    char versat_buffer[2048];
    GetHexadecimal((char*) versat_result,versat_buffer, AES_BLK_SIZE);
    printf("  Versat:   %s\n",versat_buffer);

    result.tests += 1;
    PopArena(test,testMark);
  }

  PopArena(test,mark);
  ClosePhysicalAllocator(alloc);

  return result;
}

#endif // AES

#ifdef McEliece

#include "controlbits.h"
#include "benes.h"
#include "crypto_declassify.h"
#include "crypto_uint64.h"
#include "params.h"
#include "pk_gen.h"
#include "sk_gen.h"
#include "root.h"
#include "uint64_sort.h"
#include "util.h"
#include "crypto_hash.h"
#include "decrypt.h"
#include "randombytes.h"

// Prevents "cast increases required alignment" warnings by gcc
// When mat is allocated in such a way that rows are guaranteed to be aligned
#define CAST_PTR(TYPE,PTR) ((TYPE) ((void*) (PTR)))

static McElieceConfig* vec;
static void* matAddr;

#define SBYTE (SYS_N / 8)
#define SINT (SBYTE / 4)

void PrintRow(unsigned char* view){
#if 0
    for(int i = 0; i < SBYTE; i++){
        printf("%02x",view[i]);
        if(i % 16 == 0 && i != 0){
            printf("\n");
        }
    }
#else
    for(int i = 0; i < 16; i++){
        printf("%02x",view[i]);
    }
    printf("\n");
    for(int i = SBYTE - 16; i < SBYTE; i++){
        printf("%02x",view[i]);
    }
    printf("\n");
#endif
}

void PrintFullRow(unsigned char* view){
    for(int i = 0; i < SBYTE; i++){
        printf("%02x",view[i]);
        if(i % 16 == 0 && i != 0){
            printf("\n");
        }
    }
}

void ReadRow(uint32_t* row){
    for (int i = 0; i < SINT; i++){
        row[i] = VersatUnitRead((iptr) matAddr,i);
    }
}

void VersatLoadRow(uint32_t* row){
    VersatMemoryCopy(matAddr,CAST_PTR(int*,row),SINT * sizeof(int));
}

void VersatPrintRow(){
    uint32_t values[SINT];
    ReadRow(values);
    PrintRow(CAST_PTR(uint8_t*,values));
}

uint8_t* GetVersatRowForTests(){  
    static uint8_t buffer[SBYTE];
    ReadRow(CAST_PTR(uint32_t*,buffer));
    return buffer;
}

void PrintSimpleMat(unsigned char** mat,int centerRow){
    PrintRow(mat[0]);
    PrintRow(mat[centerRow - 1]);
    PrintRow(mat[centerRow]);
    PrintRow(mat[centerRow + 1]);
    PrintRow(mat[PK_NROWS - 1]);
}

void PrintFullMat(unsigned char** mat){
    printf("Printing full mat:\n");
    for(int i = 0; i < PK_NROWS; i++){
        PrintFullRow(mat[i]);
    }
}

void VersatMcElieceLoop1(uint8_t *row, uint8_t mask,bool first){
    static uint8_t savedMask = 0;
    uint32_t *row_int = CAST_PTR(uint32_t*,row);

    ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);
    if(first){
        vec->mat.in0_wr = 0;
    } else {
        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);
        vec->mask.constant = mask_int;
        vec->mat.in0_wr = 1;
    }

    EndAccelerator();
    StartAccelerator();

    savedMask = mask;
}

// Can only be called if k != row. Care
void VersatMcElieceLoop2(unsigned char** mat,int timesCalled,int k,int row,uint8_t mask){
    static uint8_t savedMask = 0;

    int toRead =    k;
    int toCompute = ((toRead - 1    == row) ? toRead - 2    : toRead - 1);
    int toWrite =   ((toCompute - 1 == row) ? toCompute - 2 : toCompute - 1);

    if(toRead < PK_NROWS){
        int *toRead_int = CAST_PTR(int*,mat[toRead]);

        vec->mat.in0_wr = 0;

        ConfigureSimpleVReadShallow(&vec->row, SINT,toRead_int);        
    } else {
        vec->row.enableRead = 0;
        toRead = -9;
    }
    
    if(timesCalled >= 1 && toCompute >= 0 && toCompute < PK_NROWS){
        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);

        vec->mask.constant = mask_int;
    } else {
        ConfigureSimpleVWrite(&vec->writer, SINT, (int*) NULL);
        vec->writer.enableWrite = 0;
        toCompute = -9;
    }
    
    if(timesCalled >= 2 && toWrite >= 0){
        int* toWrite_int = CAST_PTR(int*,mat[toWrite]);
        ConfigureSimpleVWrite(&vec->writer, SINT,toWrite_int);
    } else {
        toWrite = -9;
        vec->writer.enableWrite = 0;
    }

    EndAccelerator();
    StartAccelerator();

    savedMask = mask;
}

static crypto_uint64 uint64_is_equal_declassify(uint64_t t, uint64_t u) {
    crypto_uint64 mask = crypto_uint64_equal_mask(t, u);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

static crypto_uint64 uint64_is_zero_declassify(uint64_t t) {
    crypto_uint64 mask = crypto_uint64_zero_mask(t);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

/* input: secret key sk */
/* output: public key pk */
int Versat_pk_gen(unsigned char *pk, unsigned char *sk, const uint32_t *perm, int16_t *pi) {
    int i, j, k;
    int row, c;

    int mark = MarkArena();

    // Init needed values for versat later on.  
    vec = (McElieceConfig*) accelConfig;
    matAddr = (void*) TOP_mat_addr;

    ActivateMergedAccelerator(McEliece);

    ConfigureSimpleVReadBare(&vec->row);

    vec->mat.iterA = 1;
    vec->mat.incrA = 1;
    vec->mat.iterB = 1;
    vec->mat.incrB = 1;
    vec->mat.perA = SINT + 1;
    vec->mat.dutyA = SINT + 1;
    vec->mat.perB = SINT + 1;
    vec->mat.dutyB = SINT + 1;

    uint64_t buf[ 1 << GFBITS ];

    unsigned char** mat = PushArray(PK_NROWS,unsigned char*);
    for(int i = 0; i < PK_NROWS; i++){
        mat[i] = PushArray(SYS_N / 8,unsigned char); // This guarantees that each row is properly aligned to a 32 bit boundary.
    }

    unsigned char mask;
    unsigned char b;

    gf* g = PushArray(SYS_T + 1,gf);
    gf* L = PushArray(SYS_N,gf); // support
    gf* inv = PushArray(SYS_N,gf);

    //

    g[ SYS_T ] = 1;

    for (i = 0; i < SYS_T; i++) {
        g[i] = load_gf(sk);
        sk += 2;
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        buf[i] = perm[i];
        buf[i] <<= 31;
        buf[i] |= i;
    }

    uint64_sort(buf, 1 << GFBITS);

    for (i = 1; i < (1 << GFBITS); i++) {
        if (uint64_is_equal_declassify(buf[i - 1] >> 31, buf[i] >> 31)) {
            PopArena(mark);
            return -1;
        }
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        pi[i] = buf[i] & GFMASK;
    }
    for (i = 0; i < SYS_N;         i++) {
        L[i] = bitrev(pi[i]);
    }

    // filling the matrix

    root(inv, g, L);

    for (i = 0; i < SYS_N; i++) {
        inv[i] = gf_inv(inv[i]);
    }

    for (i = 0; i < PK_NROWS; i++) {
        for (j = 0; j < SYS_N / 8; j++) {
            mat[i][j] = 0;
        }
    }

    for (i = 0; i < SYS_T; i++) {
        for (j = 0; j < SYS_N; j += 8) {
            for (k = 0; k < GFBITS;  k++) {
                b  = (inv[j + 7] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 6] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 5] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 4] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 3] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 2] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 1] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 0] >> k) & 1;

                mat[ i * GFBITS + k ][ j / 8 ] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = gf_mul(inv[j], L[j]);
        }
    }

    // gaussian elimination
    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

            uint32_t *out_int = CAST_PTR(uint32_t*,mat[row]);
            EndAccelerator();

            VersatLoadRow(out_int);
            bool first = true;
            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[ row ][ i ] ^ mat[ k ][ i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                VersatMcElieceLoop1(mat[k],mask,first);

                // We could fetch the value from Versat, but it's easier to calculate it CPU side.
                mat[row][i] ^= mat[k][i] & mask;
                first = false;
            }

            // Last run, use valid data to compute last operation
            VersatMcElieceLoop1(mat[PK_NROWS - 1],0,false); // TODO: Have a proper function instead of sending a "fake" adress

            EndAccelerator();

            if ( uint64_is_zero_declassify((mat[ row ][ i ] >> j) & 1) ) { // return if not systematic
               PopArena(mark);
               return -1;
            }

            ReadRow(out_int);

            int index = 0;
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[k][i] >> j;
                    mask &= 1;
                    mask = -mask;

                    VersatMcElieceLoop2(mat,index,k,row,mask);
                    index += 1;
                }
            }

            VersatMcElieceLoop2(mat,index++,PK_NROWS,row,0);
            VersatMcElieceLoop2(mat,index++,PK_NROWS + 1,row,0);
            vec->writer.enableWrite = 0;

            clear_cache();
        }
    }

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, mat[i] + PK_NROWS / 8, PK_ROW_BYTES);
    }

    PopArena(mark);
    return 0;
}

int VersatMcEliece
(
    unsigned char *pk,
    unsigned char *sk
) {
    int i;
    unsigned char seed[ 33 ] = {64};
    unsigned char r[ SYS_N / 8 + (1 << GFBITS)*sizeof(uint32_t) + SYS_T * 2 + 32 ];
    unsigned char *rp, *skp;

    gf f[ SYS_T ]; // element in GF(2^mt)
    gf irr[ SYS_T ]; // Goppa polynomial
    uint32_t perm[ 1 << GFBITS ]; // random permutation as 32-bit integers
    int16_t pi[ 1 << GFBITS ]; // random permutation

    randombytes(seed + 1, 32);

    while (1) {
        rp = &r[ sizeof(r) - 32 ];
        skp = sk;

        // expanding and updating the seed
        shake(r, sizeof(r), seed, 33);
        memcpy(skp, seed + 1, 32);
        skp += 32 + 8;
        memcpy(seed + 1, &r[ sizeof(r) - 32 ], 32);

        // generating irreducible polynomial
        rp -= sizeof(f);

        for (i = 0; i < SYS_T; i++) {
            f[i] = load_gf(rp + i * 2);
        }

        if (genpoly_gen(irr, f)) {
            continue;
        }

        for (i = 0; i < SYS_T; i++) {
            store_gf(skp + i * 2, irr[i]);
        }

        skp += IRR_BYTES;

        // generating permutation
        rp -= sizeof(perm);

        for (i = 0; i < (1 << GFBITS); i++) {
            perm[i] = load4(rp + i * 4);
        }

        if (Versat_pk_gen(pk, skp - IRR_BYTES, perm, pi)) {
            continue;
        }

        controlbitsfrompermutation(skp, pi, GFBITS, 1 << GFBITS);
        skp += COND_BYTES;

        // storing the random string s
        rp -= SYS_N / 8;
        memcpy(skp, rp, SYS_N / 8);

        // storing positions of the 32 pivots

        store8(sk + 32, 0xFFFFFFFF);

        break;
    }

    return 0;
}

#endif // McEliece

#endif

int main(int argc,const char* argv[]){
   if(!InitVersat()){
      return -1;
   }

   Arena testInst = InitArena(4096);
   Arena* test = &testInst;

#ifdef AES
   String content = STRING("COUNT = 0\nKEY = CC22DA787F375711C76302BEF0979D8EDDF842829C2B99EF3DD04E23E54CC24B\nPLAINTEXT = CCC62C6B0A09A671D64456818DB29A4D\n");
   printf("%.*s",content.size,content.str);
   VersatCommonAESTests(test,content); // DF8634CA02B13A125B786E1DCE90658B
#endif // AES

#ifdef SHA
   String content = STRING("LEN = 128\nMSG = 0A27847CDC98BD6F62220B046EDD762B\n");
   printf("%.*s",content.size,content.str);
   VersatCommonSHATests(test,content); // 80C25EC1600587E7F28B18B1B18E3CDC89928E39CAB3BC25E4D4A4C139BCEDC4
#endif // SHA

#ifdef McEliece

#endif // McEliece

   return 0;
}

/*

TODO: Currently versat DMA is being disabled because of physical memory 
      Affects AES.
      
*/ 
