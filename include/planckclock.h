#ifndef PLANCKCLOCK_LIBRARY_H_
#define PLANCKCLOCK_LIBRARY_H_

#include <stdint.h>
#include <time.h>

/**
 * This type represents a timestamp in novs
 */
typedef uint64_t ptime_t;

/**
 * This type represents a high precision timestamp
 */
typedef struct ptm
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
} ptm;

enum PTIME_PWR
{
    UNIT        = 0x00,
    DUUNIT      = 0x02,
    UN          = 0x04,
    DUUN        = 0x06,
    DU          = 0x08,
    DUDU        = 0x0A,
    TRE         = 0x0C,
    DUTRE       = 0x0E,
    QUAT        = 0x10,
    DUQUAT      = 0x12,
    QUINT       = 0x14,
    DUQUINT     = 0x16,
    HEC         = 0x18,
    DUHEC       = 0x1A,
    SEP         = 0x1C,
    DUSEP       = 0x1E,
    OCT         = 0x20,
    DUOCT       = 0x22,
    NOV         = 0x24,
    DUNOV       = 0x26,
    DEC         = 0x28,
    DUDEC       = 0x2A,
    EL          = 0x2C,
    DUEL        = 0x2E,
    DOE         = 0x30,
    DUDOE       = 0x32,
};

ptime_t planck_time_at_planck_tm(const ptm* ptm);

void planck_tm_at_planck_time(ptm* ptm_out, ptime_t time);

/**
 * Gets the current time.
 * @param ptm_ph_out ptr to handle of output struct (may be null)
 * @return current time in novs
 */
ptime_t planck_time_now(ptm** ptm_ph_out);

/**
 * Stores in ts_out the time that ptime corresponds to.
 * @param ts_out
 * @param ptime
 * @return boolean whether ptime fits in ts_out
 */
int ts_at_planck_time(struct timespec* ts_out, const ptm* ptime);

/**
 * Gets the time at the given timespec.
 * @param ts timespec to get time at
 * @param ptm_ph_out ptr to handle of output struct (may be null)
 * @return current time in novs
 */
ptime_t planck_time_at_ts(struct timespec* ts, ptm** ptm_ph_out);

void planck_difftime_get_ts(const ptm* start, const ptm* end, struct timespec* ts_out);

unsigned long planck_strftime(char *s, unsigned long max, const char *format,
                              const ptm *tm);

#endif // PLANCKCLOCK_LIBRARY_H_
