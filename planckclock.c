#include <time.h>
#include <gmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "planckclock.h"

#define UNS_PER_SECOND      "1.85488921611077614362073522501613984835872810662879e43"
#define UNS_SINCE_BIG_BANG  "8071833529780809902742760646451040575040216335127996084558318"
#define NOV_POWER_BASE_2    0x4 * 0x24
#define PLANCK_TIME_SIZE    8

planck_time_t planck_time_at_unix_timestamp(time_t ts)
{
    mpf_t input_unix_time_in_s, uns_per_s, uns_since_big_bang;
    mpf_init_set_si(input_unix_time_in_s, ts);
    mpf_init_set_str(uns_per_s, UNS_PER_SECOND, 10);
    mpf_init_set_str(uns_since_big_bang, UNS_SINCE_BIG_BANG, 10);

    mpf_t planck_time_since_unix_epoch, input_planck_time_raw_f;
    mpf_init(planck_time_since_unix_epoch);
    mpf_init(input_planck_time_raw_f);
    mpf_mul(planck_time_since_unix_epoch, input_unix_time_in_s, uns_per_s);
    mpf_add(input_planck_time_raw_f, uns_since_big_bang,
            planck_time_since_unix_epoch);

    // round down to nearest un
    mpf_floor(input_planck_time_raw_f, input_planck_time_raw_f);

    // divide to get correct power magnitude
    mpf_div_2exp(input_planck_time_raw_f,
                 input_planck_time_raw_f, NOV_POWER_BASE_2);

    return mpf_get_ui(input_planck_time_raw_f);
}

planck_time_t planck_time(planck_time_t *out)
{
    const planck_time_t time_now = planck_time_at_unix_timestamp(time(NULL));
    if (out)
        *out = time_now;
    return time_now;
}

planck_tm* planck_localtime(const planck_time_t* time)
{
    if (!time)
        return NULL;

    planck_tm* tm = calloc(1, sizeof(planck_tm));
    if (!tm)
        return NULL;

    memcpy(&tm->nov, time, PLANCK_TIME_SIZE);
    return tm;
}

size_t planck_strftime(char *s, size_t max, const char *format,
                       const planck_tm *tm)
{
    // Format: A-X for powers of 10^2

    size_t i = 0, j = 0, maxf = strlen(format);
    const unsigned char mask = 0xf;
    const void* tm_void = tm;
    while (i < max && j < maxf)
    {
        if (j != maxf - 1 && format[j] == '%')
        {
            const unsigned int c = format[j + 1];
            if (c >= 'A' && c <= 'X')
            {
                unsigned int c_from_zero = ('X' - c), c_hex = 0;
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