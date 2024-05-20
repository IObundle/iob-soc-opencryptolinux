#include "versatCrypto.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <stdio.h>

#include "versat.h"
#include "versat_accel.h"
#include "unitConfiguration.h" 

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

typedef enum {
  BUFFER_FREE = 0,
  BUFFER_RESERVED, // Not being used by Versat but user copy data to it.
  BUFFER_BEING_USED // Being used by Versat. Only one buffer is allowed
} Buffer_State;

typedef struct{
  PhysicalAllocator alloc;
  PhysicalBuffer buffers[2];
  Buffer_State bufferState[2];
  VersatBuffer interface[2];
  MergeType lastTypeActivated;
  int lastBufferReserved;
  bool currentlyInside;

  union{
    struct{ // SHA only
         int bytesProcessed;
         bool initialStateValuesLoaded;
    };
  };
} VersatCrypto;

static VersatCrypto versat = {};

static PhysicalBuffer AllocateBuffer(PhysicalAllocator alloc){
  PhysicalBuffer buffer = {};
  int attempSize = 1024 * 1024; // Start at 1 Megabyte
  while(attempSize >= 4096){
    buffer = AllocatePhysicalBuffer(alloc,attempSize);      

    if(PhysicalBufferError(buffer)){
      attempSize /= 2;
    } else {
      break;
    }
  }

  return buffer;
}

static uint32_t initialStateValues[] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
static uint32_t kConstants0[] = {0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174};
static uint32_t kConstants1[] = {0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967};
static uint32_t kConstants2[] = {0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070};
static uint32_t kConstants3[] = {0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t* kConstants[4] = {kConstants0,kConstants1,kConstants2,kConstants3};

void InitVersatSHA(){
  printf("Init versat SHA\n");
  CryptoAlgosConfig* config = (CryptoAlgosConfig*) accelConfig;
  SHAConfig* sha = &config->SHA;

  *sha = (SHAConfig){0};
  ActivateMergedAccelerator(MergeType_SHA);
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

bool InitVersat(){
  int mem = open("/dev/mem",O_RDWR | O_SYNC);
  if(mem == -1){
    puts("Open mem failed\n");
    return false;
  }
  void* res = mmap(0, versatAddressSpace, PROT_READ | PROT_WRITE, MAP_SHARED,mem,VERSAT_ADDRESS);   

  if(res == MAP_FAILED){
    puts("Mmap failed\n");
    printf("%d",errno);
    close(mem);
    return false;
  }

  versat_init((iptr) res);
  close(mem);

  versat.alloc = OpenPhysicalAllocator();
  if(PhysicalAllocatorError(versat.alloc)){
    puts("Physical allocator failed\n");
    return false;
  }

  versat.buffers[0] = AllocateBuffer(versat.alloc);
  if(PhysicalBufferError(versat.buffers[0])){
    puts("Physical buffer failed\n");
    return false;
  }

  versat.buffers[1] = AllocateBuffer(versat.alloc);
  if(PhysicalBufferError(versat.buffers[1])){
    puts("Physical buffer failed\n");
    return false;
  }

  versat.interface[0].mem = versat.buffers[0].virtualMemBase;
  versat.interface[0].maxSize = versat.buffers[0].size;
  versat.interface[1].mem = versat.buffers[1].virtualMemBase;
  versat.interface[1].maxSize = versat.buffers[1].size;

  // Start off in the SHA state
  ActivateMergedAccelerator(MergeType_SHA);
  InitVersatSHA();

  return true;
}

static int GetVersatValidBuffer(){
  for(int i = 0; i < 2; i++){
    if(versat.bufferState[i] == BUFFER_FREE){
      return i;
    }
  }
  return -1;
}

void clear_cache();

static char GetHexadecimalChar(unsigned char value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'A' + (value - 10);
  }
}

static char* GetHexadecimal(const char* text,char* buffer,int str_size){
  int i = 0;
  unsigned char* view = (unsigned char*) text;
  for(; i< str_size; i++){
    buffer[i*2] = GetHexadecimalChar(view[i] / 16);
    buffer[i*2+1] = GetHexadecimalChar(view[i] % 16);
  }

  buffer[i*2] = '\0';

  return buffer;
}

static size_t versat_crypto_hashblocks_sha256(PhysicalBuffer* physBuffer,const uint8_t *in, size_t inlen){
  while (inlen >= 64) {
    char buffer[128];
    GetHexadecimal((char*) in,buffer, 64);
    printf("Called with %p %ld\n",in,inlen);
    printf("%s\n",buffer);

    ACCEL_TOP_MemRead_key_ext_addr = (iptr) ConvertVirtToPhys(*physBuffer,in);

    // Loads data + performs work
    clear_cache();
    RunAccelerator(1);

    if(!versat.initialStateValuesLoaded){
      VersatUnitWrite(TOP_State_s0_reg_addr,0,initialStateValues[0]);
      VersatUnitWrite(TOP_State_s1_reg_addr,0,initialStateValues[1]);
      VersatUnitWrite(TOP_State_s2_reg_addr,0,initialStateValues[2]);
      VersatUnitWrite(TOP_State_s3_reg_addr,0,initialStateValues[3]);
      VersatUnitWrite(TOP_State_s4_reg_addr,0,initialStateValues[4]);
      VersatUnitWrite(TOP_State_s5_reg_addr,0,initialStateValues[5]);
      VersatUnitWrite(TOP_State_s6_reg_addr,0,initialStateValues[6]);
      VersatUnitWrite(TOP_State_s7_reg_addr,0,initialStateValues[7]);
      versat.initialStateValuesLoaded = true;
    }

    in += 64;
    inlen -= 64;
  }

  return inlen;   
}

VersatBuffer* BeginSHA(){
  if(versat.currentlyInside){
    printf("Already called a Begin, call End beforehand");
    return NULL;
  }
  versat.currentlyInside = true;

  if(versat.lastTypeActivated != MergeType_SHA){
    ActivateMergedAccelerator(MergeType_SHA);
    InitVersatSHA();
  }

  versat.initialStateValuesLoaded = false;
  versat.bytesProcessed = 0;

  // Reserve initial buffer
  int bufferIndex = GetVersatValidBuffer();
  versat.lastBufferReserved = bufferIndex; 
  versat.bufferState[bufferIndex] = BUFFER_RESERVED;
  return &versat.interface[bufferIndex];
}

static int GetIndex(VersatBuffer* input){
  return versat.interface - input;
}

VersatBuffer* ProcessSHA(VersatBuffer* input,int numberBlocks){
  int bufferIndex = GetIndex(input);

  PhysicalBuffer* buffer = &versat.buffers[bufferIndex];
  Buffer_State* state = &versat.bufferState[bufferIndex];
  
  // Since versat currently does not allow to process SHA on the background, simple use and reuse the same buffer.
  *state = BUFFER_BEING_USED;
  versat_crypto_hashblocks_sha256(buffer,input->mem,numberBlocks * 64);
  versat.bytesProcessed += numberBlocks * 64;
  *state = BUFFER_FREE;
  
  return input;
}

static void store_bigendian_32(uint8_t *x, uint32_t u) {
   x[3] = (uint8_t) u;
   u >>= 8;
   x[2] = (uint8_t) u;
   u >>= 8;
   x[1] = (uint8_t) u;
   u >>= 8;
   x[0] = (uint8_t) u;
}

void EndSHA(VersatBuffer* input,unsigned char digest[32]){
  int index = GetIndex(input);
  PhysicalBuffer* buffer = &versat.buffers[index];

  uint8_t* in = (uint8_t*) input->mem;
  size_t inlen = input->size;
  
  uint64_t bytes = (uint64_t) versat.bytesProcessed + (uint64_t) inlen;

  printf("Bytes: %lx,Inlen: %lx,versat: %d\n",bytes,inlen,versat.bytesProcessed);
  versat_crypto_hashblocks_sha256(buffer,in, inlen);
  in += inlen;
  inlen &= 63;
  in -= inlen;

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

  // Flush last valid data
  clear_cache();
  printf("Before run %x\n",VersatUnitRead(TOP_State_s0_reg_addr,0));
  RunAccelerator(1);
  printf("After run %x\n",VersatUnitRead(TOP_State_s0_reg_addr,0));

  store_bigendian_32(&digest[0*4],(uint32_t) VersatUnitRead(TOP_State_s0_reg_addr,0));
  store_bigendian_32(&digest[1*4],(uint32_t) VersatUnitRead(TOP_State_s1_reg_addr,0));
  store_bigendian_32(&digest[2*4],(uint32_t) VersatUnitRead(TOP_State_s2_reg_addr,0));
  store_bigendian_32(&digest[3*4],(uint32_t) VersatUnitRead(TOP_State_s3_reg_addr,0));
  store_bigendian_32(&digest[4*4],(uint32_t) VersatUnitRead(TOP_State_s4_reg_addr,0));
  store_bigendian_32(&digest[5*4],(uint32_t) VersatUnitRead(TOP_State_s5_reg_addr,0));
  store_bigendian_32(&digest[6*4],(uint32_t) VersatUnitRead(TOP_State_s6_reg_addr,0));
  store_bigendian_32(&digest[7*4],(uint32_t) VersatUnitRead(TOP_State_s7_reg_addr,0));

  versat.currentlyInside = false;
}
