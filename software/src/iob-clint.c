#include "iob-clint.h"

void clint_init(int base_address){
  clint_base = base_address;
}

void clint_set_timer(unsigned long long time){
  (*(volatile uint32_t *) (clint_base + MTIME_BASE))     = (uint32_t)(time & 0x0FFFFFFFFUL);
  (*(volatile uint32_t *) (clint_base + MTIME_BASE + 4)) = (uint32_t)(time >> 32);
}

uint64_t clint_get_timer(){
  uint64_t read_time;

  read_time  = (uint64_t)(*(volatile uint32_t *) (clint_base + MTIME_BASE));
  read_time |= (uint64_t)(*(volatile uint32_t *) (clint_base + MTIME_BASE + 4)) << 32;

  return read_time;
}

void clint_set_timercmp(unsigned long long time, int hart){
  (*(volatile uint32_t *) (clint_base + MTIMECMP_BASE + 8*hart + 4)) = 0xFFFFFFFF;
  (*(volatile uint32_t *) (clint_base + MTIMECMP_BASE + 8*hart))     = (uint32_t)(time & 0x0FFFFFFFFUL);
  (*(volatile uint32_t *) (clint_base + MTIMECMP_BASE + 8*hart + 4)) = (uint32_t)(time >> 32);
}

uint64_t clint_get_timercmp(int hart){
  uint64_t read_time;

  read_time  = (uint64_t)(*(volatile uint32_t *) (clint_base + MTIMECMP_BASE + 8*hart));
  read_time |= (uint64_t)(*(volatile uint32_t *) (clint_base + MTIMECMP_BASE + 8*hart + 4)) << 32;

  return read_time;
}

void clint_set_msip(unsigned long msip_value, int hart){
  (*(volatile uint32_t *) (clint_base + MSIP_BASE + 4*hart)) = msip_value;
}

uint32_t clint_get_msip(int hart){
  uint32_t msip_value;

  msip_value = (*(volatile uint32_t *) (clint_base + MSIP_BASE + 4*hart));

  return msip_value;
}

void mtimer_set_raw_time_cmp(uint64_t clock_offset) {
  // First of all set 
  uint64_t new_mtimecmp = mtimer_get_raw_time() + clock_offset;
#if (__riscv_xlen == 64)
  // Single bus access
  volatile uint64_t *mtimecmp = (volatile uint64_t*)(clint_base+MTIMECMP_BASE);
  *mtimecmp = new_mtimecmp;
#else
  volatile uint32_t *mtimecmpl = (volatile uint32_t *)(clint_base+MTIMECMP_BASE);
  volatile uint32_t *mtimecmph = (volatile uint32_t *)(clint_base+MTIMECMP_BASE+4);
  // AS we are doing 32 bit writes, an intermediate mtimecmp value may cause spurious interrupts.
  // Prevent that by first setting the dummy MSB to an unacheivable value
  *mtimecmph = 0xFFFFFFFF;  // cppcheck-suppress redundantAssignment
  // set the LSB
  *mtimecmpl = (uint32_t)(new_mtimecmp & 0x0FFFFFFFFUL);
  // Set the correct MSB
  *mtimecmph = (uint32_t)(new_mtimecmp >> 32); // cppcheck-suppress redundantAssignment
#endif
}
 
/** Read the raw time of the system timer in system timer clocks
 */
uint64_t mtimer_get_raw_time(void) {
#if ( __riscv_xlen == 64)
  // Directly read 64 bit value
  volatile uint64_t *mtime = (volatile uint64_t *)(clint_base+MTIME_BASE);
  return *mtime;
#else
  volatile uint32_t * mtimel = (volatile uint32_t *)(clint_base+MTIME_BASE);
  volatile uint32_t * mtimeh = (volatile uint32_t *)(clint_base+MTIME_BASE+4);
  uint32_t mtimeh_val;
  uint32_t mtimel_val;
  do {
    // There is a small risk the mtimeh will tick over after reading mtimel
    mtimeh_val = *mtimeh;
    mtimel_val = *mtimel;
    // Poll mtimeh to ensure it's consistent after reading mtimel
    // The frequency of mtimeh ticking over is low
  } while (mtimeh_val != *mtimeh);
  return (uint64_t) ( ( ((uint64_t)mtimeh_val)<<32) | mtimel_val);
#endif
} 
