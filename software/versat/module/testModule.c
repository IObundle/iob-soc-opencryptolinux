#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>

#if 0

#include "arena.h"
#include "versat.h"
#include "versat_accel.h"
#include "versat_crypto.h"

#include <stdint.h>
#include <string.h>

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

static TestState VersatCommonSHATests(String content){
  TestState result = {};

  int mark = MarkArena();

  InitVersatSHA();

  static const int HASH_SIZE = (256/8);

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
    int bytes = HexStringToHex(message,ptr);

    ptr = SearchAndAdvance(ptr,STRING("MD = "));
    if(ptr == NULL){
      result.earlyExit = 1;
      break;
    }

    char* expected = ptr;

    unsigned char versat_digest[256];
    for(int i = 0; i < 256; i++){
      versat_digest[i] = 0;
    }

    VersatSHA(versat_digest,message,len / 8);
    char versat_buffer[2048];
    GetHexadecimal((char*) versat_digest,versat_buffer, HASH_SIZE);

    printf("Versat: %s\n",versat_buffer);

    result.tests += 1;
    PopArena(testMark);
  }

  PopArena(mark);

  return result;
}
#endif

int main(int argc,const char* argv){
   int versat = -1;
   {
      versat = open("/dev/versat",O_RDWR | O_SYNC);
      if(versat == -1){
         puts("Open versat is -1\n");
         return -1;
      }
   }

   void* res = mmap(0,4096,PROT_READ | PROT_WRITE,MAP_SHARED,versat,0);

   if(res == MAP_FAILED){
      printf("mmap failed\n");
      return -1;
   }

   printf("here %p\n",res);
   fflush(stdout);

   char* view = (char*) res;

   for(int i = 0; i < 8; i++){
      view[i] = i + 1;
   }

   for(int i = 0; i < 8; i++){
      printf("%c\n",view[i]);
   }

   {
      int fd = close(versat);
      if(fd == -1){
         puts("Open versat is -1\n");
         return -1;
      }
   }

#if 0
   int mem = -1;
   {
      mem = open("/dev/mem",O_RDWR | O_SYNC);
      if(mem == -1){
         puts("Open mem is -1\n");
         return -1;
      }
   }

   void* res = mmap(0, versatAddressSpace, PROT_READ | PROT_WRITE, MAP_SHARED,mem,VERSAT_ADDRESS);   

   if(res == MAP_FAILED){
      puts("Mmap failed\n");
      printf("%d",errno);
      return -1;
   }


   int* view = (int*) res;
   printf("VersatRead: %d\n",*view);

   versat_init((int) res);

   String content = STRING("LEN = 0\nMSG = 00\nMD = E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855");
   TestState result = VersatCommonSHATests(content);
#endif

   return 0;
}