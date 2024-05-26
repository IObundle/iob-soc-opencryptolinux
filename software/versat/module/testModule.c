#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <argp.h>

#include "arena.h"
#include "versatCrypto.h"

#if 1

#undef SHA 
#undef AES
#undef McEliece

//#define SHA
//#define AES
//#define McEliece

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

void nist_kat_init(unsigned char *entropy_input, unsigned char *personalization_string, int security_strength);

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
   buffer->size = len / 8;

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

TestState VersatCommonAESTests(Arena* test,String content){
  TestState result = {};

  int mark = MarkArena(test);

  printf("1\n");
  fflush(stdout);

  char* ptr = content.str;
  while(1){
   int testMark = MarkArena(test);

   printf("2\n");
   fflush(stdout);
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

   uint8_t key[AES_256_KEY_SIZE] = {};
   HexStringToHex(key,ptr);

   ptr = SearchAndAdvance(ptr,STRING("PLAINTEXT = "));
   if(ptr == NULL){
    result.earlyExit = 1;
    break;
   }
  
   uint8_t plain[AES_256_BLK_SIZE] = {};
   HexStringToHex(plain,ptr);

   uint8_t versat_result[AES_256_BLK_SIZE * 2 + 1] = {};

   printf("3\n");
   fflush(stdout);
   VersatBuffer* buffer = BeginAES_ECB(key,true,false);
   HexStringToHex(buffer->mem,ptr);
   buffer->size = 16;

   printf("4\n");
   fflush(stdout);
   int outputSize = 0;
   int fileSize = EndAES(buffer,versat_result,&outputSize);

   printf("AES: %d %d\n",outputSize,fileSize);

   char versat_buffer[2048];
   GetHexadecimal((char*) versat_result,versat_buffer, AES_256_BLK_SIZE);
   printf("  Versat:   %s\n",versat_buffer);

   result.tests += 1;
   PopArena(test,testMark);
  }

  PopArena(test,mark);

  return result;
}

#endif // AES

#ifdef McEliece
#include "api.h"
#endif // McEliece

#endif

void ConfigEnableDMA(bool value);
int VersatMcEliece(unsigned char *pk,unsigned char *sk,Arena* temp);

#include <argp.h>
  
typedef struct{
  const char* algorithm;
  bool encrypt;
  bool decrypt;
  
  const char* key;
  const char* seed;
  const char* iv;
  const char* inputFile;
  const char* outputFile;
} Options;

error_t parse(int key, char *arg, struct argp_state *state){
  Options* options = (Options*) state->input;
  
  switch(key){
  case ARGP_KEY_INIT: printf("Init\n"); break;
  case ARGP_KEY_SUCCESS: printf("Success\n"); break;
  case ARGP_KEY_ERROR: printf("Error\n"); break;
  case ARGP_KEY_ARGS: printf("Args\n"); break;
  case ARGP_KEY_END: printf("End\n"); break;
  case ARGP_KEY_NO_ARGS: printf("No Args\n"); break;
  case ARGP_KEY_FINI: printf("Fini\n"); break;
    
  case ARGP_KEY_ARG: options->algorithm = arg; break;
  case 'e': options->encrypt = true; break;
  case 'd': options->decrypt = true; break;
  case 'k': options->key = arg; break;
  case 'v': options->iv = arg; break;
  case 's': options->seed = arg; break;
  case 'i': options->inputFile = arg; break;
  case 'o': options->outputFile = arg; break;
  default: return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

typedef enum {
  AlgorithmType_SHA,
  AlgorithmType_AES,
  AlgorithmType_McEliece
} AlgorithmType;

typedef enum {
  AESType_CBC,
  AESType_ECB,
  AESType_CTR
} AESType;

bool CheckCorrectHex(const char* str,int expectedLength){
  for(int i = 0; i < expectedLength; i++){
   if(str[i] == '\0'){
    return false;
   }
   bool check = ((str[i] >= '0' && str[i] <= '9') ||
            (str[i] >= 'a' && str[i] <= 'f') ||
            (str[i] >= 'A' && str[i] <= 'F'));
   
   if(!check){
    return false;
   }
  }

  return (str[expectedLength] == '\0');
}

bool FillBufferFromFile(VersatBuffer* buffer,int fileFd){
  while(1){
    ssize_t bytesRead = read(fileFd,buffer->mem + buffer->size,buffer->maxSize - buffer->size);

    if(bytesRead == -1){
      printf("There was an error reading the file\n");
      exit(-1);
    }

    if(bytesRead == 0){
      return true;
    }

    buffer->size += bytesRead;

    if(bytesRead >= buffer->size){
      return false;
    }
  }
}

int main(int argc,char** argv){
  if(!InitVersat()){
    return -1;
  }

  const struct argp_option options[] = {
    {.name = NULL,.key = 0,.arg = NULL,.flags = 0,.doc = "Configuration:",.group = 0},
    {.name = "Encrypt",.key = 'e',.arg = NULL,.flags = 0,.doc = "",.group = 0},
    {.name = "Decrypt",.key = 'd',.arg = NULL,.flags = 0,.doc = "",.group = 0},
    {.name = "Key",.key = 'k',.arg = "hex",.flags = 0,.doc = "Key used for AES in Hex",.group = 0},
    {.name = "IV",.key = 'v',.arg = "hex",.flags = 0,.doc = "IV or initial counter for AES in Hex",.group = 0},
    {.name = "Seed",.key = 's',.arg = "hex",.flags = 0,.doc = "Seed for McEliece in Hex",.group = 0},
    {.name = NULL,.key = 0,.arg = NULL,.flags = 0,.doc = "Input/Output:",.group = 0},
    {.name = "InputFile",.key = 'i',.arg = "name",.flags = 0,.doc = "",.group = 0},
    {.name = "OutputFile",.key = 'o',.arg = "name",.flags = 0,.doc = "",.group = 0},
    {0}};

  const struct argp parser = {
    .options = options,
    .parser = parse,
    .args_doc = "ALGORITHM",
    .doc = "\nSupported algorithms: SHA256 AES-128-CBC AES-128-ECB AES-128-CTR AES-256-CBC AES-256-ECB AES-256-CTR McEliece\n\nSHA outputs digest on terminal, no output file used\n\nFor McEliece the session key is outputted on the terminal while output file stores the chipertext",
    .children = NULL,
    .help_filter = NULL,
    .argp_domain = NULL
  };

  Options OPT = {};
  
  int index = 0;
  error_t result = argp_parse(&parser,argc,argv,0,&index,&OPT);
  
  if(!OPT.algorithm){
    printf("Need to speficy an algorithm\n");
    return -1;
  }

  AlgorithmType algorithm;
  AESType type;
  bool is256 = false;
  if(strcmp(OPT.algorithm,"SHA256") == 0){
    algorithm = AlgorithmType_SHA;
  } else if(strcmp(OPT.algorithm,"AES-128-CBC") == 0){
    algorithm = AlgorithmType_AES;
    type = AESType_CBC;
  } else if(strcmp(OPT.algorithm,"AES-256-CBC") == 0){
    algorithm = AlgorithmType_AES;
    type = AESType_CBC;
    is256 = true;
  } else if(strcmp(OPT.algorithm,"AES-128-CTR") == 0){
    algorithm = AlgorithmType_AES;
    type = AESType_CTR;
  } else if(strcmp(OPT.algorithm,"AES-256-CTR") == 0){
    algorithm = AlgorithmType_AES;
    type = AESType_CTR;
    is256 = true;
  } else if(strcmp(OPT.algorithm,"AES-128-ECB") == 0){
    algorithm = AlgorithmType_AES;
    type = AESType_ECB;
  } else if(strcmp(OPT.algorithm,"AES-256-ECB") == 0){
    algorithm = AlgorithmType_AES;
    type = AESType_ECB;
    is256 = true;
  } else if(strcmp(OPT.algorithm,"McEliece") == 0){
    algorithm = AlgorithmType_McEliece;
  } else {
    printf("Algorithm '%s' not supported\n",OPT.algorithm);
    return -1;
  }

  switch(algorithm){
  case AlgorithmType_SHA:{
    if(OPT.inputFile == NULL){
      printf("Need input file for SHA\n");
      return -1;
    }
  } break;
  case AlgorithmType_McEliece:{
    if(OPT.outputFile == NULL){
      printf("Need output file for McEliece\n");
      return -1;
    }

    if(OPT.seed == NULL){
      printf("Need seed for McEliece\n");
      return -1;
    }

    if(!CheckCorrectHex(OPT.seed,96)){
      printf("McEliece seed needs to be 96 hexadecimal characters\n");
      return -1;
    }
  } break;
  case AlgorithmType_AES:{
    if(OPT.inputFile == NULL || OPT.outputFile == NULL){
      printf("Need both input file and output file for AES\n");
      return -1;
    }

    if(OPT.key == NULL){
      printf("Need key for AES\n");
      return -1;
    }

    if(is256){
      if(!CheckCorrectHex(OPT.key,64)){
        printf("AES key needs to be 64 hexadecimal characters\n");
        return -1;
      } 
    } else if(!CheckCorrectHex(OPT.key,32)){
      printf("AES key needs to be 32 hexadecimal characters\n");
      return -1;
    }

    if(type == AESType_CTR || type == AESType_CBC){
      if(OPT.iv == NULL){
        printf("Need IV for selected AES algorithm\n");
        return -1;
      }

      if(!CheckCorrectHex(OPT.iv,32)){
        printf("IV needs to be an hexadecimal number with 32 characters\n");
        return -1;
      }
    }
    
    if(OPT.encrypt == false && OPT.decrypt == false){
      printf("Need either a encrypt or a decrypt option set for AES\n");
      return -1;
    }
    if(OPT.encrypt == true && OPT.decrypt == true){
      printf("Cannot have both encrypt and decrypt options set\n");
      return -1;
    }
  } break;
  } 

  ConfigEnableDMA(false);

  Arena testInst = InitArena(4 * 1024 * 1024);
  Arena* test = &testInst;

  globalArena = test; // Global arena is only used for allocations that are 100% stack based. There should not be any conflict having test and global acting the same

  int inputFd = -1;
  int outputFd = -1;

  if(algorithm == AlgorithmType_AES || algorithm == AlgorithmType_SHA){
    inputFd = open(OPT.inputFile,O_RDONLY,0);

    if(inputFd == -1){
      printf("Problem opening file: %s\n",OPT.inputFile);
      return -1;
    }
  }

  if(algorithm == AlgorithmType_AES || algorithm == AlgorithmType_McEliece){
    outputFd = open(OPT.outputFile,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if(outputFd == -1){
      printf("Problem opening file: %s\n",OPT.outputFile);
      return -1;
    }
  }

  switch(algorithm){
  case AlgorithmType_AES:{
    VersatBuffer* buffer = NULL;

    uint8_t key[AES_MAX_KEY_SIZE] = {};
    uint8_t ivOrCounter[AES_MAX_BLK_SIZE] = {};
    HexStringToHex(key,OPT.key);

    switch(type){
    case AESType_ECB:{
      buffer = BeginAES_ECB(key,is256,OPT.decrypt);
    }break;
    case AESType_CTR:{
      HexStringToHex(ivOrCounter,OPT.iv);
      buffer = BeginAES_CTR(key,ivOrCounter,is256);
    }break;
    case AESType_CBC:{
      HexStringToHex(ivOrCounter,OPT.iv);
      buffer = BeginAES_CBC(key,ivOrCounter,is256,OPT.decrypt);
    }break;
    }

    int mark = MarkArena(test);
    while(1){
      PopArena(test,mark);

      uint8_t* outputSpace = PushBytes(test,buffer->maxSize);
      bool end = false;
      if(FillBufferFromFile(buffer,inputFd)){
        end = true;
      }

      int outputOffset = 0;
      int totalFileSize = 0;

      if(end){
        totalFileSize = EndAES(buffer,outputSpace,&outputOffset);
      } else {
        buffer = ProcessAES(buffer,outputSpace,&outputOffset);
      }

      ssize_t res = write(outputFd,outputSpace,outputOffset);

      if(res < 0){
        printf("There was an error writing to the file\n");
        return -1;
      }

      if(end){
        ftruncate(outputFd,totalFileSize);
        break;
      }
    }
  } break;
  }

#if 0
  int fileToRead = open("test.txt",O_RDONLY,0);
  if(fileToRead == -1){
    printf("Failed to open input file: %s",path);
    return -1;
  }

  bool AES = true;

  if(AES){
    uint8_t key[AES_256_KEY_SIZE] = {};
    VerstBuffer* buffer = BeginAES_ECB(key,false,false);

    while(1){
      int amountRead = read(fileToRead,buffer->mem,buffer->maxSize);

      if(amountRead == 0){
        break;
      }

      buffer->size += amountRead;

      int outputOffset = 0;
      buffer = ProcessAES(buffer,output,&outputOffset);
    }
  }
#endif

#ifdef AES
{
  String content = STRING("COUNT = 0\nKEY = CC22DA787F375711C76302BEF0979D8EDDF842829C2B99EF3DD04E23E54CC24B\nPLAINTEXT = CCC62C6B0A09A671D64456818DB29A4D\n");
  printf("%.*s",content.size,content.str);
  VersatCommonAESTests(test,content);
  printf("Good: DF8634CA02B13A125B786E1DCE90658B\n");
}
#endif // AES

#ifdef SHA
{
  String content = STRING("LEN = 128\nMSG = 0A27847CDC98BD6F62220B046EDD762B\n");
  printf("%.*s",content.size,content.str);
  VersatCommonSHATests(test,content); // 80C25EC1600587E7F28B18B1B18E3CDC89928E39CAB3BC25E4D4A4C139BCEDC4
  printf("Good: 80C25EC1600587E7F28B18B1B18E3CDC89928E39CAB3BC25E4D4A4C139BCEDC4\n");
}
#endif // SHA

#ifdef McEliece
   unsigned char seed[49];
   HexStringToHex(seed,"061550234D158C5EC95595FE04EF7A25767F2E24CC2BC479D09D86DC9ABCFDE7056A8C266F9EF97ED08541DBD2E1FFA1");

   nist_kat_init(seed, NULL, 256);

   unsigned char* public_key = PushArray(test,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES,unsigned char);
   unsigned char* secret_key = PushArray(test,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES,unsigned char);

   VersatMcEliece(public_key, secret_key,test);

   unsigned char* public_key_hex = PushArray(test,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES * 2 + 1,char);
   unsigned char* secret_key_hex = PushArray(test,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES * 2 + 1,char);

   GetHexadecimal(public_key,public_key_hex,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_PUBLICKEYBYTES);
   GetHexadecimal(secret_key,secret_key_hex,PQCLEAN_MCELIECE348864_CLEAN_CRYPTO_SECRETKEYBYTES);

   // C5ED9AF0EEA0D4ADEA66D1A2A2F614E0
   // 5B815C890117893D8BB8E886F63A78CE

   printf("  Got Public (first 32 chars):      %s\n",public_key_hex);
   printf("  Got Secret (first 32 chars):      %s\n",secret_key_hex);

#endif // McEliece

  return 0;
}

/*

TODO: Currently versat DMA is being disabled because of physical memory 
    Affects AES.
    
*/ 
