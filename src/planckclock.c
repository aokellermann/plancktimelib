#include <time.h>
#include <gmp.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "planckclock.h"

#define TIME_PER_NSECOND     "1.85488921611077614362073522501613984835872810662879e34"
#define TIME_SINCE_BIG_BANG  "8071833529780809902742760646451040575040216335127996084558318"
#define PLANCK_TIME_SIZE    8
#define NIBBLE_SIZE_BITS    (unsigned char)4

// region Misc Helper Functions

#define ts_add(a, b, result)			                \
  do {									                \
    (result)->tv_sec  = (a)->tv_sec  + (b)->tv_sec;		\
    (result)->tv_nsec = (a)->tv_usec + (b)->tv_nsec;	\
    if ((result)->tv_nsec >= 1000000000)				\
    {									                \
	    ++(result)->tv_sec;						        \
	    (result)->tv_nsec -= 1000000000;				\
    }									                \
  } while (0)

#define ts_sub(a, b, result)						    \
  do {									                \
    (result)->tv_sec  = (a)->tv_sec  - (b)->tv_sec;		\
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;	\
    if ((result)->tv_nsec < 0) {					    \
      --(result)->tv_sec;						        \
      (result)->tv_nsec += 1000000000;					\
    }									                \
  } while (0)

unsigned int hex_to_int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return 0xFF;
}

// endregion

// region Conversion Functions

// region Self

ptime_t ptime_at_ptm(const ptm* ptm_in)
{
    // Copy novs to return variable
    ptime_t ptime_now = 0;
    memcpy(&ptime_now, &ptm_in->nov, PLANCK_TIME_SIZE);
    return ptime_now;
}

void ptm_at_ptime(ptm* ptm_out, ptime_t time_in)
{
    memset(ptm_out, 0, (void*)&ptm_out->nov - (void*)ptm_out);
    memcpy(&ptm_out->nov, &time_in, PLANCK_TIME_SIZE);
}

// endregion

// region timespec

ptime_t ptime_at_ts(ptm* ptm_out, const struct timespec* ts_in)
{
    // Set up constants and input time
    mpf_t input_unix_time_in_ns, time_per_ns, time_since_big_bang;
    mpf_init_set_ui(input_unix_time_in_ns, ts_in->tv_sec);
    mpf_mul_ui(input_unix_time_in_ns, input_unix_time_in_ns, 1000000000);
    mpf_add_ui(input_unix_time_in_ns, input_unix_time_in_ns, ts_in->tv_nsec);
    mpf_init_set_str(time_per_ns, TIME_PER_NSECOND, 10);
    mpf_init_set_str(time_since_big_bang, TIME_SINCE_BIG_BANG, 10);

    // Add time from before unix epoch to time since unix epoch
    mpf_t time_since_unix_epoch, input_planck_time_raw_f;
    mpf_init(time_since_unix_epoch);
    mpf_mul(time_since_unix_epoch, input_unix_time_in_ns, time_per_ns);
    mpf_init(input_planck_time_raw_f);
    mpf_add(input_planck_time_raw_f, time_since_big_bang, time_since_unix_epoch);

    // Convert time to integer
    mpz_t input_planck_time_z;
    mpz_init(input_planck_time_z);
    mpz_set_f(input_planck_time_z, input_planck_time_raw_f);

    // Convert to hex string
    char* input_planck_time_str = mpz_get_str(NULL, 16, input_planck_time_z);

    // Allocate output struct and copy data
    int i, j = (int)strlen(input_planck_time_str) - 1;
    unsigned char byte;
    void* pt = ptm_out;
    memset(pt, 0, sizeof(ptm));
    for (i = 0; j >= 0; ++i, j -= 2)
    {
        byte = hex_to_int(input_planck_time_str[j]);
        if (j > 0)  // check if nibble to left is occupied
            byte += (hex_to_int(input_planck_time_str[j - 1]) << NIBBLE_SIZE_BITS);

        memset(&pt[i], byte, 1);
    }

    return ptime_at_ptm(ptm_out);
}

int ts_at_ptime(struct timespec* ts_out, ptime_t ptime_in)
{
    ptm ptm_in;
    ptm_at_ptime(&ptm_in, ptime_in);
    return ts_at_ptm(ts_out, &ptm_in);
}

int ts_at_ptm(struct timespec* ts_out, const ptm* ptm_in)
{
    mpz_t total_time, current_byte;
    mpz_init_set_ui(total_time, 0);
    mpz_init(current_byte);
    unsigned char pwr_base_2 = 0x0, byte;
    const void* void_ptime = ptm_in;
    int j;
    for (j = 0; j < (int)sizeof(ptm); ++j)
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

    // Get time per nsecond
    mpf_t time_per_ns;
    mpf_init_set_str(time_per_ns, TIME_PER_NSECOND, 10);

    // Get nseconds since unix epoch
    mpf_t nseconds_since_unix_epoch;
    mpf_init(nseconds_since_unix_epoch);
    mpf_set_z(nseconds_since_unix_epoch, time_since_unix_epoch);
    mpf_div(nseconds_since_unix_epoch, nseconds_since_unix_epoch, time_per_ns);

    // Return false if nseconds doesn't fit in uint64_t
    if (!mpf_fits_ulong_p(nseconds_since_unix_epoch))
        return 0;

    // Get seconds and remainder nseconds
    uint64_t nseconds_since_unix_epoch_ui = mpf_get_ui(nseconds_since_unix_epoch);
    uint64_t seconds = nseconds_since_unix_epoch_ui / 1000000000;
    uint64_t nseconds = nseconds_since_unix_epoch_ui % 1000000000;

    ts_out->tv_sec = seconds;
    ts_out->tv_nsec = nseconds;
    return 1;
}

// endregion

// endregion

// region Arithmetic

// region Self

int ptm_add_ptm(ptm* ptm_out, const ptm* ptm_in_a, const ptm* ptm_in_b)
{
    ptm psum = {0};
    uint16_t tsum;
    uint8_t s = 0, a, b;
    int i;
    void* vs = &psum;
    const void* va = ptm_in_a, * vb = ptm_in_b;
    for (i = 0; i < sizeof(ptm); ++i)
    {
        memcpy(&a, &va[i], 1);
        memcpy(&b, &vb[i], 1);
        tsum = s + a + b;
        s = 0;
        if (tsum > 0xFF)
        {
            // Overflow
            if (i == sizeof(ptm) - 1)
                return 0;

            // Set higher power to left byte
            s = tsum >> (unsigned int)8;

            // Get right byte
            tsum &= (unsigned int)0xFF;
        }

        // Write byte
        memset(&vs[i], tsum, 1);
    }

    memcpy(ptm_out, &psum, sizeof(ptm));
    return 1;
}

int ptm_add_ptime(ptm* ptm_out, const ptm* ptm_in_a, ptime_t ptime_in_b)
{
    ptm ptm_in_b;
    ptm_at_ptime(&ptm_in_b, ptime_in_b);
    return ptm_add_ptm(ptm_out, ptm_in_a, &ptm_in_b);
}

int ptime_add_ptm(ptm* ptm_out, ptime_t ptime_in_a, const ptm* ptm_in_b)
{
    ptm ptm_in_a;
    ptm_at_ptime(&ptm_in_a, ptime_in_a);
    return ptm_add_ptm(ptm_out, &ptm_in_a, ptm_in_b);
}

int ptime_add_ptime(ptm* ptm_out, ptime_t ptime_in_a, ptime_t ptime_in_b)
{
    ptm ptm_in_a, ptm_in_b;
    ptm_at_ptime(&ptm_in_a, ptime_in_a);
    ptm_at_ptime(&ptm_in_b, ptime_in_b);
    return ptm_add_ptm(ptm_out, &ptm_in_a, &ptm_in_b);
}

int ptm_sub_ptm(ptm* ptm_out, const ptm* ptm_in_a, const ptm* ptm_in_b)
{
    ptm pdif = {0};
    int16_t tdif;
    uint16_t s = 0;
    uint8_t a, b;
    int i;
    void* vd = &pdif;
    const void* va = ptm_in_a, * vb = ptm_in_b;
    for (i = sizeof(ptm) - 1; i >= 0; --i)
    {
        memcpy(&a, &va[i], 1);
        memcpy(&b, &vb[i], 1);
        tdif = s + a - b;
        s = 0;
        if (tdif < 0)
        {
            // Underflow (before big bang)
            if (i == 0)
                return 0;

            uint16_t utdif = -tdif;

            // Set lower power to right byte
            s = utdif << (unsigned int)8;

            // Get left byte
            utdif &= (unsigned int)0xFF00;
            tdif = utdif;
        }

        // Write byte
        memset(&vd[i], tdif, 1);
    }

    memcpy(ptm_out, &pdif, sizeof(ptm));
    return 1;
}

int ptm_sub_ptime(ptm* ptm_out, const ptm* ptm_in_a, ptime_t ptime_in_b)
{
    ptm ptm_in_b;
    ptm_at_ptime(&ptm_in_b, ptime_in_b);
    return ptm_sub_ptm(ptm_out, ptm_in_a, &ptm_in_b);
}

int ptime_sub_ptm(ptm* ptm_out, ptime_t ptime_in_a, const ptm* ptm_in_b)
{
    ptm ptm_in_a;
    ptm_at_ptime(&ptm_in_a, ptime_in_a);
    return ptm_sub_ptm(ptm_out, &ptm_in_a, ptm_in_b);
}

int ptime_sub_ptime(ptm* ptm_out, ptime_t ptime_in_a, ptime_t ptime_in_b)
{
    ptm ptm_in_a, ptm_in_b;
    ptm_at_ptime(&ptm_in_a, ptime_in_a);
    ptm_at_ptime(&ptm_in_b, ptime_in_b);
    return ptm_add_ptm(ptm_out, &ptm_in_a, &ptm_in_b);
}

// endregion

// region timespec

int ptm_add_ts(ptm* ptm_out, const ptm* ptm_in_a, const struct timespec* timespec_in_b)
{
    ptm ptm_in_b;
    ptime_at_ts(&ptm_in_b, timespec_in_b);
    return ptm_add_ptm(ptm_out, ptm_in_a, &ptm_in_b);
}

int ptime_add_ts(ptm* ptm_out, ptime_t ptime_in_a, const struct timespec* timespec_in_b)
{
    ptm ptm_in_a, ptm_in_b;
    ptm_at_ptime(&ptm_in_a, ptime_in_a);
    ptime_at_ts(&ptm_in_b, timespec_in_b);
    return ptm_add_ptm(ptm_out, &ptm_in_a, &ptm_in_b);
}

int ts_add_ptm(ptm* ptm_out, const struct timespec* timespec_in_a, const ptm* ptm_in_b)
{
    ptm ptm_in_a;
    ptime_at_ts(&ptm_in_a, timespec_in_a);
    return ptm_add_ptm(ptm_out, &ptm_in_a, ptm_in_b);
}

int ts_add_ptime(ptm* ptm_out, const struct timespec* timespec_in_a, ptime_t ptime_in_b)
{
    ptm ptm_in_a, ptm_in_b;
    ptm_at_ptime(&ptm_in_a, ptime_in_b);
    ptime_at_ts(&ptm_in_b, timespec_in_a);
    return ptm_add_ptm(ptm_out, &ptm_in_a, &ptm_in_b);
}

int ptm_sub_ts(ptm* ptm_out, const ptm* ptm_in_a, const struct timespec* timespec_in_b)
{
    ptm ptm_in_b;
    ptime_at_ts(&ptm_in_b, timespec_in_b);
    return ptm_sub_ptm(ptm_out, ptm_in_a, &ptm_in_b);
}

int ptime_sub_ts(ptm* ptm_out, ptime_t ptime_in_a, const struct timespec* timespec_in_b)
{
    ptm ptm_in_a, ptm_in_b;
    ptm_at_ptime(&ptm_in_a, ptime_in_a);
    ptime_at_ts(&ptm_in_b, timespec_in_b);
    return ptm_sub_ptm(ptm_out, &ptm_in_a, &ptm_in_b);
}

int ts_sub_ptm(ptm* ptm_out, const struct timespec* timespec_in_a, const ptm* ptm_in_b)
{
    ptm ptm_in_a;
    ptime_at_ts(&ptm_in_a, timespec_in_a);
    return ptm_sub_ptm(ptm_out, &ptm_in_a, ptm_in_b);
}

int ts_sub_ptime(ptm* ptm_out, const struct timespec* timespec_in_a, ptime_t ptime_in_b)
{
    ptm ptm_in_a, ptm_in_b;
    ptm_at_ptime(&ptm_in_a, ptime_in_b);
    ptime_at_ts(&ptm_in_b, timespec_in_a);
    return ptm_sub_ptm(ptm_out, &ptm_in_a, &ptm_in_b);
}

// endregion

// endregion

// region Formatting

unsigned long strfptm(char* str_out, unsigned long max, const char* format, const ptm* ptm_in)
{
    // Format: A-Z for powers of 10^2

    size_t i = 0, j = 0, maxf = strlen(format);
    const unsigned char mask = 0xf;
    const void* tm_void = ptm_in;
    while (i < max && j < maxf)
    {
        if (j != maxf - 1 && format[j] == '%')
        {
            const unsigned int c = format[j + 1];
            if (c >= 'A' && c <= 'Z')
            {
                unsigned int c_from_zero = ('Z' - c), c_hex = 0;
                memcpy(&c_hex, &tm_void[c_from_zero], 1);

                sprintf(&str_out[i], "%x%x", (c_hex >> 4) & mask, c_hex & mask);
                str_out[i] = toupper(str_out[i]);
                str_out[i + 1] = toupper(str_out[i + 1]);

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
            str_out[i] = format[j];
            ++i;
            ++j;
        }
    }
    str_out[i] = '\0';
    return i;
}

unsigned long strfptime(char *str_out, unsigned long max, const char *format, ptime_t ptime_in)
{
    ptm ptm_in;
    ptm_at_ptime(&ptm_in, ptime_in);
    return strfptm(str_out, max, format, &ptm_in);
}

// endregion

// region Other

ptime_t ptime_now(ptm* ptm_out)
{
    // Get current time
    struct timespec ts_now;
    clock_gettime(CLOCK_REALTIME, &ts_now);
    return ptime_at_ts(ptm_out, &ts_now);
}

// endregion
