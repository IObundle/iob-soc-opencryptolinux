#ifndef CLINT_H
#define CLINT_H

#include <stdint.h>

#define MSIP_BASE 0
#define MTIMECMP_BASE 0x4000
#define MTIME_BASE 0xBFF8

#define MTIME_FREQ_HZ 100000

#define MTIMER_SECONDS_TO_CLOCKS(SEC)           \
    ((uint64_t)(((SEC)*(MTIME_FREQ_HZ))))

//Functions
static int clint_base;
void clint_init(int);

void clint_set_timer(unsigned long long);
void clint_set_timercmp(unsigned long long, int);
void clint_set_msip(unsigned long, int);

uint64_t clint_get_timer();
uint64_t clint_get_timercmp(int);
uint32_t clint_get_msip(int);

/** Set the raw time compare point in system timer clocks.
 * @param clock_offset Time relative to current mtime when
 * @note The time range of the 64 bit timer is large enough not to consider a wrap around of mtime.
 * An interrupt will be generated at mtime + clock_offset.
 * See http://five-embeddev.com/riscv-isa-manual/latest/machine.html#machine-timer-registers-mtime-and-mtimecmp
 */
void mtimer_set_raw_time_cmp(uint64_t clock_offset);

/** Read the raw time of the system timer in system timer clocks
 */
uint64_t mtimer_get_raw_time(void);

#endif // #ifdef CLINT_H
