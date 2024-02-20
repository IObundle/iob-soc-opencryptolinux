#ifndef H_ARENA_H
#define H_ARENA_H
#include <stdint.h>

void InitArena(int size);
void* PushBytes(int size);
void* PushAndZeroBytes(int size);
int MarkArena();
void PopArena(int mark);

#define PushArray(N_ELEM,TYPE) (TYPE*) PushBytes((N_ELEM) * sizeof(TYPE))

#endif // H_MEMORY_POOL_H
