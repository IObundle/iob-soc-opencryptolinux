#include "versat_accel.h"

void IntSet(volatile void* buffer,int value,int byteSize){
   volatile int* asInt = (int*) buffer;

   int nInts = byteSize / 4;

   for(int i = 0; i < nInts; i++){
      asInt[i] = value;
   }
}

#ifdef VERSAT_DEFINED_VRead
void ConfigureSimpleVRead(VReadConfig* inst, int numberItems,int* memory){
   IntSet(inst,0,sizeof(VReadConfig));

   // Memory side
   inst->incrA = 1;
   inst->perA = numberItems;
   inst->pingPong = 1;
   inst->ext_addr = (iptr) memory;
   inst->length = numberItems * sizeof(int);
   //inst->iterA = 1;
   //inst->dutyA = numberItems;
   //inst->int_addr = 0;

   // B - versat side
   inst->iterB = 1;
   inst->incrB = 1;
   inst->perB = numberItems;
   inst->dutyB = 1;
}

void ConfigureLeftSideMatrixVRead(VReadConfig* inst, int iterations){
   IntSet(inst,0,sizeof(VReadConfig));

   int numberItems = iterations * iterations;

   inst->incrA = 1;
   //inst->iterA = 1;
   inst->perA = numberItems;
   //inst->dutyA = numberItems;
   //inst->int_addr = 0;
   inst->pingPong = 0;
   inst->length = numberItems * sizeof(int);

   inst->iterB = iterations;
   inst->perB = iterations;
   inst->dutyB = iterations;
   inst->startB = 0;
   inst->shiftB = -iterations;
   inst->incrB = 1;
   inst->reverseB = 0;
   inst->iter2B = 1;
   inst->per2B = iterations;
   inst->shift2B = 0;
   inst->incr2B = iterations;
}

void ConfigureRightSideMatrixVRead(VReadConfig* inst, int iterations){
   IntSet(inst,0,sizeof(VReadConfig));

   int numberItems = iterations * iterations;

   inst->incrA = 1;
   //inst->iterA = 1;
   inst->perA = numberItems;
   //inst->dutyA = numberItems;
   //inst->int_addr = 0;
   inst->pingPong = 0;
   inst->length = numberItems * sizeof(int);

   inst->iterB = iterations;
   inst->perB = iterations;
   inst->dutyB = iterations;
   inst->startB = 0;
   inst->shiftB = -(iterations * iterations - 1);
   inst->incrB = iterations;
   inst->reverseB = 0;
   inst->iter2B = 1;
   inst->per2B = iterations;
   inst->shift2B = 0;
   inst->incr2B = 0;
}
#endif

#ifdef VERSAT_DEFINED_VWrite
void ConfigureSimpleVWrite(VWriteConfig* inst, int numberItems,int* memory){
   IntSet(inst,0,sizeof(VWriteConfig));

   // Write side
   inst->incrA = 1;
   inst->iterA = 1;
   inst->perA = numberItems;
   inst->dutyA = numberItems;
   inst->size = 8;
   inst->int_addr = 0;
   inst->pingPong = 1;
   inst->length = numberItems * sizeof(int);
   inst->ext_addr = (iptr) memory;

   // Memory side
   inst->iterB = numberItems;
   inst->perB = 1;
   inst->dutyB = 1;
   inst->incrB = 1;
}

void ConfigureMatrixVWrite(VWriteConfig* inst,int amountOfData){
   IntSet(inst,0,sizeof(VWriteConfig));

   inst->incrA = 1;
   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->size = 8;
   inst->int_addr = 0;
   inst->pingPong = 0;
   inst->length = amountOfData * sizeof(int);

   inst->iterB = amountOfData;
   inst->perB = 4;
   inst->dutyB = 1;
   inst->incrB = 1;
}
#endif

#ifdef VERSAT_DEFINED_Mem
void ConfigureLeftSideMatrix(MemConfig* inst,int iterations){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = iterations;
   inst->perA = iterations;
   inst->dutyA = iterations;
   inst->startA = 0;
   inst->shiftA = -iterations;
   inst->incrA = 1;
   inst->reverseA = 0;
   inst->iter2A = 1;
   inst->per2A = iterations;
   inst->shift2A = 0;
   inst->incr2A = iterations;
}

void ConfigureRightSideMatrix(MemConfig* inst, int iterations){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = iterations;
   inst->perA = iterations;
   inst->dutyA = iterations;
   inst->startA = 0;
   inst->shiftA = -(iterations * iterations - 1);
   inst->incrA = iterations;
   inst->reverseA = 0;
   inst->iter2A = 1;
   inst->per2A = iterations;
   inst->shift2A = 0;
   inst->incr2A = 0;
}

void ConfigureMemoryLinear(MemConfig* inst, int amountOfData, int start){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->startA = start;
}

void ConfigureMemoryLinearOut(MemConfig* inst, int amountOfData){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->in0_wr = 1;
}

void ConfigureMemoryReceive(MemConfig* inst, int amountOfData,int interdataDelay){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = amountOfData;
   inst->perA = interdataDelay;
   inst->dutyA = 1;
   inst->startA = 0;
   inst->shiftA = 0;
   inst->incrA = 1;
   inst->in0_wr = 1;
   inst->reverseA = 0;
   inst->iter2A = 0;
   inst->per2A = 0;
   inst->shift2A = 0;
   inst->incr2A = 0;
}
#endif

