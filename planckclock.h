#ifndef PLANCKCLOCK_LIBRARY_H_
#define PLANCKCLOCK_LIBRARY_H_

#include <stdint.h>
#include <sys/time.h>

/**
 * This type represents a timestamp in novs
 */
typedef uint64_t planck_time_t;

/**
 * This type represents a high precision timestamp
 */
typedef struct planck_tm
{
    uint16_t units  ;  // 0x10^(0x4*0x0) = 10^00
    uint16_t un     ;  // 0x10^(0x4*0x1) = 10^04
    uint16_t du     ;  // 0x10^(0x4*0x2) = 10^08
    uint16_t tre    ;  // 0x10^(0x4*0x3) = 10^0C
    uint16_t quat   ;  // 0x10^(0x4*0x4) = 10^10
    uint16_t quin   ;  // 0x10^(0x4*0x5) = 10^14
    uint16_t hec    ;  // 0x10^(0x4*0x6) = 10^18
    uint16_t sep    ;  // 0x10^(0x4*0x7) = 10^1C
    uint16_t oct    ;  // 0x10^(0x4*0x8) = 10^20
    uint16_t nov    ;  // 0x10^(0x4*0x9) = 10^24
    uint16_t dec    ;  // 0x10^(0x4*0xA) = 10^28
    uint16_t el     ;  // 0x10^(0x4*0xB) = 10^2C
    uint16_t doe    ;  // 0x10^(0x4*0xC) = 10^30
} planck_tm;

planck_time_t planck_time_at_planck_tm(const planck_tm* ptm);

void planck_tm_at_planck_time(planck_tm* ptm_out, planck_time_t time);

/**
 * Gets the current time.
 * @param ptm_ph_out ptr to handle of output struct (may be null)
 * @return current time in novs
 */
planck_time_t planck_time_now(planck_tm** ptm_ph_out);

/**
 * Stores in tv_out the time that ptime corresponds to.
 * @param tv_out
 * @param ptime
 * @return boolean whether ptime fits in tv_out
 */
int tv_at_planck_time(struct timeval* tv_out, const planck_tm* ptime);

/**
 * Gets the time at the given timeval.
 * @param tv timeval to get time at
 * @param ptm_ph_out ptr to handle of output struct (may be null)
 * @return current time in novs
 */
planck_time_t planck_time_at_tv(struct timeval* tv, planck_tm** ptm_ph_out);

void planck_difftime_get_tv(const planck_tm* start, const planck_tm* end, struct timeval* tv_out);

unsigned long planck_strftime(char *s, unsigned long max, const char *format,
                              const planck_tm *tm);

#endif // PLANCKCLOCK_LIBRARY_H_
