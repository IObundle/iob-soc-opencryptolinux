#ifndef INCLUDED_UNIT_CONFIGURATION
#define INCLUDED_UNIT_CONFIGURATION

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

   // B - versat side
   inst->iterB = 1;
   inst->incrB = 1;
   inst->perB = numberItems;
   inst->dutyB = 1;
}
#endif

#ifdef VERSAT_DEFINED_VWrite
// Not being used for now.
void ConfigureSimpleVWrite(VWriteConfig* inst, int numberItems,int* memory){
   IntSet(inst,0,sizeof(VWriteConfig));

   // Write side
   inst->incrA = 1;
   inst->perA = numberItems;
   //inst->int_addr = 0;
   inst->pingPong = 1;
   inst->length = numberItems * sizeof(int);
   inst->ext_addr = (iptr) memory;

   // Memory side
   inst->iterB = numberItems;
   inst->perB = 1;
   inst->dutyB = 1;
   inst->incrB = 1;
}
#endif

#ifdef VERSAT_DEFINED_Mem
void ConfigureSimpleMemoryWithStart(MemConfig* inst, int amountOfData, int start){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->startA = start;
}

void ConfigureSimpleMemoryAndCopyData(MemConfig* inst, int amountOfData, int start,MemAddr addr,int* data){
   ConfigureSimpleMemoryWithStart(inst,amountOfData,start);
   VersatMemoryCopy(addr.addr,data,amountOfData * sizeof(int));
}

void ConfigureSimpleMemory(MemConfig* inst, int amountOfData){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->in0_wr = 1;
}

void ConfigureMemoryReceive(MemConfig* inst, int amountOfData){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->in0_wr = 1;
}
#endif

#endif // INCLUDED_UNIT_CONFIGURATION