#pragma once

#include <stdbool.h>

// These buffers are needed to load data into and from the versat accelerator.
// In order to avoid unneaded copies, we expose their interface to the programmer.
// Each time a buffer is returned by a function, that buffer is the next valid buffer 
// that the programmer is allowed to use while the previous buffer is going to be used.

typedef struct{
   void* mem;
   int size;
   int maxSize;
} VersatBuffer;

// Each Begin/End represents a pair of functions that need to be called before calling other functions 
// related to the crypto algorithm being used.
// Must call End before calling another Begin, unless calling Begin for the first time.

// SHA
bool InitVersat();

// To optimize performance try to send multiple buffers instead of a single buffer even if data fits into a single buffer.
// Versat can process data while CPU is filling the next buffer.

VersatBuffer* BeginSHA(); // Returns one buffer that can be used to load data.

// If buffer is not disivible by 64 bytes, prints an error.
VersatBuffer* ProcessSHA(VersatBuffer* input,int numberBlocks); // Returns a valid buffer that can be used by program to load more data
void EndSHA(VersatBuffer* input,unsigned char digest[32] /*[32]*/); // Fills digest
