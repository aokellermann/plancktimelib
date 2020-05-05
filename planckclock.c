#include <time.h>
#include <gmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>

#include "planckclock.h"

#define TIME_PER_USECOND     "1.85488921611077614362073522501613984835872810662879e37"
#define TIME_SINCE_BIG_BANG  "8071833529780809902742760646451040575040216335127996084558318"
#define PLANCK_TIME_SIZE    8

int hex_to_int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

planck_tm* inner_planck_time_at_tv(const struct timeval* tv)
{
    // Set up constants and input time
    mpf_t input_unix_time_in_us, time_per_us, time_since_big_bang;
    mpf_init_set_ui(input_unix_time_in_us, tv->tv_sec);
    mpf_mul_ui(input_unix_time_in_us, input_unix_time_in_us, 1000000);
    mpf_add_ui(input_unix_time_in_us, input_unix_time_in_us, tv->tv_usec);
    mpf_init_set_str(time_per_us, TIME_PER_USECOND, 10);
    mpf_init_set_str(time_since_big_bang, TIME_SINCE_BIG_BANG, 10);

    // Add time from before unix epoch to time since unix epoch
    mpf_t time_since_unix_epoch, input_planck_time_raw_f;
    mpf_init(time_since_unix_epoch);
    mpf_mul(time_since_unix_epoch, input_unix_time_in_us, time_per_us);
    mpf_init(input_planck_time_raw_f);
    mpf_add(input_planck_time_raw_f, time_since_big_bang, time_since_unix_epoch);

    // Convert time to integer
    mpz_t input_planck_time_z;
    mpz_init(input_planck_time_z);
    mpz_set_f(input_planck_time_z, input_planck_time_raw_f);

    // Convert to hex string
    char* input_planck_time_str = mpz_get_str(NULL, 16, input_planck_time_z);

    // Allocate output struct and copy data
    void* pt = calloc(1, sizeof(planck_tm));
    int i, j = (int)strlen(input_planck_time_str) - 1;
    unsigned char byte;
    for (i = 0; j >= 0; ++i, j -= 2)
    {
        byte = hex_to_int(input_planck_time_str[j]);
        if (j > 0)  // check if nibble to left is occupied
            byte += (hex_to_int(input_planck_time_str[j - 1]) << 4);

        memset(&pt[i], byte, 1);
    }

    return pt;
}

planck_time_t planck_time_at_planck_tm(const planck_tm* ptm)
{
    // Copy novs to return variable
    planck_time_t ptime_now = 0;
    memcpy(&ptime_now, &ptm->nov, PLANCK_TIME_SIZE);
    return ptime_now;
}

void planck_tm_at_planck_time(planck_tm* ptm_out, planck_time_t time)
{
    memset(ptm_out, 0, (void*)&ptm_out->nov - (void*)ptm_out);
    memcpy(&ptm_out->nov, &time, PLANCK_TIME_SIZE);
}

int tv_at_planck_time(struct timeval* tv_out, const planck_tm* ptime)
{
    mpz_t total_time, current_byte;
    mpz_init_set_ui(total_time, 0);
    mpz_init(current_byte);
    unsigned char pwr_base_2 = 0x0, byte;
    const void* void_ptime = ptime;
    int j;
    for (j = 0; j < (int)sizeof(planck_tm); ++j)
    {
        // Get byte
        memcpy(&byte, &void_ptime[j], 1);

        // Get place value
        mpz_set_ui(current_byte, byte);
        mpz_mul_2exp(current_byte, current_byte, pwr_base_2);

        // Add to total
        mpz_add(total_time, total_time, current_byte);

        pwr_base_2 += 8;
    }

    // Get time since unix epoch
    mpz_t time_before_unix_epoch, time_since_unix_epoch;
    mpz_init_set_str(time_before_unix_epoch, TIME_SINCE_BIG_BANG, 10);
    mpz_init_set(time_since_unix_epoch, total_time);
    mpz_sub(time_since_unix_epoch, time_since_unix_epoch, time_before_unix_epoch);

    // Get time per usecond
    mpf_t time_per_us;
    mpf_init_set_str(time_per_us, TIME_PER_USECOND, 10);

    // Get useconds since unix epoch
    mpf_t useconds_since_unix_epoch;
    mpf_init(useconds_since_unix_epoch);
    mpf_set_z(useconds_since_unix_epoch, time_since_unix_epoch);
    mpf_div(useconds_since_unix_epoch, useconds_since_unix_epoch, time_per_us);

    // Return false if useconds doesn't fit in uint64_t
    if (!mpf_fits_ulong_p(useconds_since_unix_epoch))
        return 0;

    // Get seconds and remainder useconds
    uint64_t useconds_since_unix_epoch_ui = mpf_get_ui(useconds_since_unix_epoch);
    uint64_t seconds = useconds_since_unix_epoch_ui / 1000000;
    uint64_t useconds = useconds_since_unix_epoch_ui % 1000000;

    tv_out->tv_sec = seconds;
    tv_out->tv_usec = useconds;
    return 1;
}

planck_time_t planck_time_now(planck_tm** ptm_ph_out)
{
    // Get current time
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    return planck_time_at_tv(&tv_now, ptm_ph_out);
}

planck_time_t planck_time_at_tv(struct timeval* tv, planck_tm** ptm_ph_out)
{
    planck_tm* ptm_now = inner_planck_time_at_tv(tv);
    planck_time_t ptime_now = planck_time_at_planck_tm(ptm_now);

    // If handle ptr is not null, set. Otherwise free ptm_now.
    if (ptm_ph_out)
        *ptm_ph_out = ptm_now;
    else
        free(ptm_now);

    return ptime_now;
}

void planck_difftime_get_tv(const planck_tm* start, const planck_tm* end, struct timeval* tv_out)
{
    struct timeval tv_start, tv_end;
    tv_at_planck_time(&tv_start, start);
    tv_at_planck_time(&tv_end, end);
    timersub(&tv_end, &tv_start, tv_out);
}

size_t planck_strftime(char *s, size_t max, const char *format,
                       const planck_tm *tm)
{
    // Format: A-Z for powers of 10^2

    size_t i = 0, j = 0, maxf = strlen(format);
    const unsigned char mask = 0xf;
    const void* tm_void = tm;
    while (i < max && j < maxf)
    {
        if (j != maxf - 1 && format[j] == '%')
        {
            const unsigned int c = format[j + 1];
            if (c >= 'A' && c <= 'Z')
            {
                unsigned int c_from_zero = ('Z' - c), c_hex = 0;
                memcpy(&c_hex, &tm_void[c_from_zero], 1);

                sprintf(&s[i], "%x%x", (c_hex >> 4) & mask, c_hex & mask);
                s[i] = toupper(s[i]);
                s[i + 1] = toupper(s[i + 1]);

                i += 2;
                j += 2;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            s[i] = format[j];
            ++i;
            ++j;
        }
    }
    s[i] = '\0';
    return i;
}