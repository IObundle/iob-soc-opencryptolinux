#ifndef H_ARENA_H
#define H_ARENA_H
#include <stdint.h>
#ifdef PC
#include <stdio.h>
#include <stdlib.h>
#else
#include "printf.h"
#endif

void InitArena(int size);
void* PushBytes(int size);
void* PushAndZeroBytes(int size);
int MarkArena();
void PopArena(int mark);

#endif // H_MEMORY_POOL_H
