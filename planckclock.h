#ifndef PLANCKCLOCK_LIBRARY_H_
#define PLANCKCLOCK_LIBRARY_H_

#include <stdint.h>

/**
 * This type represents a timestamp in novs
 */
typedef uint64_t planck_time_t;

/**
 * This type represents a high precision timestamp
 */
typedef struct planck_tm
{
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
    uint16_t el     ;  // 0x10^(0x4*0xA) = 10^2C
    uint16_t doe    ;  // 0x10^(0x4*0xA) = 10^30
} planck_tm;

planck_time_t planck_time(planck_time_t*);

planck_tm* planck_localtime(const planck_time_t*);

size_t planck_strftime(char *s, size_t max, const char *format,
                       const planck_tm *tm);

#endif // PLANCKCLOCK_LIBRARY_H_
