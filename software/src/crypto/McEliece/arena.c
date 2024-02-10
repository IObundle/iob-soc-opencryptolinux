#include "arena.h"

#include <stdlib.h>

#include "printf.h"

typedef struct{
  char* ptr;
  int used;
  int allocated;
} Arena;

static Arena arena = {};

void InitArena(int size){
  if(arena.ptr){
    printf("Arena already initialized\n");
    return;
  }

  arena.ptr = (char*) malloc(size * sizeof(char));
  arena.allocated = size;
}

void* PushBytes(int size){
  char* ptr = &arena.ptr[arena.used];

  size = (size + 3) & (~3); // Align to 4 byte boundary
  arena.used += size;

  if(arena.used > arena.allocated){
    printf("Arena overflow\n");
    printf("Size: %d,Used: %d, Allocated: %d\n",size,arena.used,arena.allocated);
  }

  return ptr;
}

void* PushAndZeroBytes(int size){
  char* ptr = PushBytes(size);

  for (int i = 0; i < size; i++) {
    ptr[i] = 0;
  }

  return ptr;
}

int MarkArena(){
  return arena.used;
}

void PopArena(int mark){
  arena.used = mark;
}
