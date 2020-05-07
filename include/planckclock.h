#ifndef PLANCKCLOCK_LIBRARY_H_
#define PLANCKCLOCK_LIBRARY_H_

#include <stdint.h>     // For uint64_t
#include <time.h>       // for timeval

// region Planck Time Type Declarations

/**
 * This type represents a timestamp in novs, where one nov is 0x10^24.
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

// endregion

// region Conversion Functions

// region Self

/**
 * Outputs the corresponding time according to ptm_in.
 * @param ptm_in input time ptr
 * @return corresponding ptime_t
 */
ptime_t ptime_at_ptm(const ptm* ptm_in);

/**
 * Writes to ptm_out the corresponding time according to time_in.
 * @param ptm_out output time ptr
 * @param time_in input time
 */
void ptm_at_ptime(ptm* ptm_out, ptime_t time_in);

// endregion

// region timespec

/**
 * Outputs the corresponding time according to ts_in
 * @param ptm_out output corresponding time ptr
 * @param ts_in input timespec ptr
 * @return corresponding ptime_t
 */
ptime_t ptime_at_ts(ptm* ptm_out, const struct timespec* ts_in);

/**
 * Writes to ts_out the corresponding time according to ptime_in
 * @param ts_out output corresponding timespec ptr
 * @param ptime_in input time
 */
void ts_at_ptime(struct timespec* ts_out, ptime_t ptime_in);

/**
 * Writes to ts_out the corresponding time according to ptm_in
 * @param ts_out output corresponding timespec ptr
 * @param ptm_in input time ptr
 */
void ts_at_ptm(struct timespec* ts_out, const ptm* ptm_in);

// endregion

// endregion

// region Arithmetic

// region Self

ptime_t ptm_add_ptm(ptm* ptm_out, const ptm* ptm_in_a, const ptm* ptm_in_b);

ptime_t ptm_add_ptime(ptm* ptm_out, const ptm* ptm_in_a, ptime_t ptime_in_b);

ptime_t ptime_add_ptm(ptm* ptm_out, ptime_t ptime_in_a, const ptm* ptm_in_b);

ptime_t ptime_add_ptime(ptm* ptm_out, ptime_t ptime_in_a, ptime_t ptime_in_b);

ptime_t ptm_sub_ptm(ptm* ptm_out, const ptm* ptm_in_a, const ptm* ptm_in_b);

ptime_t ptm_sub_ptime(ptm* ptm_out, const ptm* ptm_in_a, ptime_t ptime_in_b);

ptime_t ptime_sub_ptm(ptm* ptm_out, ptime_t ptime_in_a, const ptm* ptm_in_b);

ptime_t ptime_sub_ptime(ptm* ptm_out, ptime_t ptime_in_a, ptime_t ptime_in_b);

// endregion

// region timespec

ptime_t ptm_add_ts(ptm* ptm_out, const ptm* ptm_in_a, const struct timespec* timespec_in_b);

ptime_t ptime_add_ts(ptm* ptm_out, ptime_t ptime_in_a, const struct timespec* timespec_in_b);

ptime_t ts_add_ptm(ptm* ptm_out, const struct timespec* timespec_in_a, const ptm* ptm_in_b);

ptime_t ts_add_ptime(ptm* ptm_out, const struct timespec* timespec_in_a, ptime_t ptime_in_b);

ptime_t ptm_sub_ts(ptm* ptm_out, const ptm* ptm_in_a, const struct timespec* timespec_in_b);

ptime_t ptime_sub_ts(ptm* ptm_out, ptime_t ptime_in_a, const struct timespec* timespec_in_b);

ptime_t ts_sub_ptm(ptm* ptm_out, const struct timespec* timespec_in_a, const ptm* ptm_in_b);

ptime_t ts_sub_ptime(ptm* ptm_out, const struct timespec* timespec_in_a, ptime_t ptime_in_b);

// endregion

// endregion

// region Formatting

unsigned long strfptm(char* str_out, unsigned long max, const char* format, const ptm* ptm_in);

unsigned long strfptime(char *str_out, unsigned long max, const char *format, ptime_t ptime_in);

// endregion

// region Other

/**
 * Outputs the current time.
 * @param ptm_out output ptm ptr
 * @return current ptime
 */
ptime_t ptime_now(ptm* ptm_out);

// endregion

#endif // PLANCKCLOCK_LIBRARY_H_
