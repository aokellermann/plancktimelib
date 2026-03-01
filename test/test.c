#include <stdio.h>
#include <string.h>
#include <time.h>

#include "planckclock.h"

static int failures = 0;
static int passed = 0;

#define ASSERT_EQ_INT(msg, expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s: expected %d, got %d\n", (msg), (expected), (actual)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

#define ASSERT_EQ_U16(msg, expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s: expected 0x%04X, got 0x%04X\n", (msg), (expected), (actual)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

#define ASSERT_EQ_U64(msg, expected, actual) \
    do { \
        if ((uint64_t)(expected) != (uint64_t)(actual)) { \
            printf("FAIL: %s: expected 0x%lX, got 0x%lX\n", (msg), \
                   (unsigned long)(expected), (unsigned long)(actual)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

#define ASSERT_EQ_LONG(msg, expected, actual) \
    do { \
        if ((long)(expected) != (long)(actual)) { \
            printf("FAIL: %s: expected %ld, got %ld\n", (msg), \
                   (long)(expected), (long)(actual)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

#define ASSERT_EQ_STR(msg, expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf("FAIL: %s: expected \"%s\", got \"%s\"\n", (msg), (expected), (actual)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

#define ASSERT_TRUE(msg, cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s\n", (msg)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

#define ASSERT_RANGE_U16(msg, val, lo, hi) \
    do { \
        if ((val) < (lo) || (val) > (hi)) { \
            printf("FAIL: %s: 0x%X not in [0x%X, 0x%X]\n", (msg), (val), (lo), (hi)); \
            ++failures; \
        } else { \
            ++passed; \
        } \
    } while (0)

/* ------------------------------------------------------------------ */
/* Conversion: ptime_at_ptm / ptm_at_ptime                           */
/* ------------------------------------------------------------------ */

void test_ptm_ptime_roundtrip(void)
{
    /* ptm_at_ptime sets nov..doe from ptime and zeroes lower fields */
    ptime_t p = 0x0505EAD29401C940ULL;
    ptm tm;
    ptm_at_ptime(&tm, p);

    ASSERT_EQ_U16("ptm_at_ptime zeroes units", 0, tm.units);
    ASSERT_EQ_U16("ptm_at_ptime zeroes un",    0, tm.un);
    ASSERT_EQ_U16("ptm_at_ptime zeroes du",    0, tm.du);
    ASSERT_EQ_U16("ptm_at_ptime zeroes tre",   0, tm.tre);
    ASSERT_EQ_U16("ptm_at_ptime zeroes quat",  0, tm.quat);
    ASSERT_EQ_U16("ptm_at_ptime zeroes quin",  0, tm.quin);
    ASSERT_EQ_U16("ptm_at_ptime zeroes hec",   0, tm.hec);
    ASSERT_EQ_U16("ptm_at_ptime zeroes sep",   0, tm.sep);
    ASSERT_EQ_U16("ptm_at_ptime zeroes oct",   0, tm.oct);

    /* ptime_at_ptm should recover the same value */
    ptime_t recovered = ptime_at_ptm(&tm);
    ASSERT_EQ_U64("ptime_at_ptm roundtrip", p, recovered);
}

void test_ptm_ptime_zero(void)
{
    ptm tm;
    ptm_at_ptime(&tm, 0);
    ASSERT_EQ_U16("zero nov", 0, tm.nov);
    ASSERT_EQ_U16("zero dec", 0, tm.dec);
    ASSERT_EQ_U16("zero el",  0, tm.el);
    ASSERT_EQ_U16("zero doe", 0, tm.doe);
    ASSERT_EQ_U64("ptime_at_ptm zero", 0ULL, ptime_at_ptm(&tm));
}

void test_ptm_ptime_max(void)
{
    ptime_t p = 0xFFFFFFFFFFFFFFFFULL;
    ptm tm;
    ptm_at_ptime(&tm, p);
    ASSERT_EQ_U16("max nov", 0xFFFF, tm.nov);
    ASSERT_EQ_U16("max dec", 0xFFFF, tm.dec);
    ASSERT_EQ_U16("max el",  0xFFFF, tm.el);
    ASSERT_EQ_U16("max doe", 0xFFFF, tm.doe);
    ASSERT_EQ_U64("ptime_at_ptm max", p, ptime_at_ptm(&tm));
}

/* ------------------------------------------------------------------ */
/* Conversion: ptime_at_ts / ts_at_ptm / ts_at_ptime                 */
/* ------------------------------------------------------------------ */

void test_ts_roundtrip(void)
{
    /* Convert a known timespec to ptm and back */
    struct timespec ts_in = {1000000, 500000000};  /* 1M sec + 500ms */
    ptm tm;
    ptime_at_ts(&tm, &ts_in);

    struct timespec ts_out;
    int ok = ts_at_ptm(&ts_out, &tm);
    ASSERT_EQ_INT("ts_at_ptm success", 1, ok);
    ASSERT_EQ_LONG("ts roundtrip sec", ts_in.tv_sec, ts_out.tv_sec);
    /* nsec loses precision through Planck time conversion; verify within 1ms */
    ASSERT_TRUE("ts roundtrip nsec close",
                ts_out.tv_nsec >= ts_in.tv_nsec - 1000000 &&
                ts_out.tv_nsec <= ts_in.tv_nsec + 1000000);
}

void test_ts_epoch(void)
{
    /* Unix epoch (0,0) should round-trip */
    struct timespec ts_in = {0, 0};
    ptm tm;
    ptime_at_ts(&tm, &ts_in);

    struct timespec ts_out;
    int ok = ts_at_ptm(&ts_out, &tm);
    ASSERT_EQ_INT("ts_at_ptm epoch success", 1, ok);
    ASSERT_EQ_LONG("epoch sec", 0, ts_out.tv_sec);
    ASSERT_EQ_LONG("epoch nsec", 0, ts_out.tv_nsec);
}

void test_ts_at_ptime(void)
{
    /* ts_at_ptime delegates through ptm_at_ptime then ts_at_ptm */
    struct timespec ts_in = {86400, 0};  /* 1 day */
    ptm tm;
    ptime_t p = ptime_at_ts(&tm, &ts_in);

    struct timespec ts_out;
    int ok = ts_at_ptime(&ts_out, p);
    ASSERT_EQ_INT("ts_at_ptime success", 1, ok);
    /* ptime truncates sub-nov precision, so sec should match or be close */
    ASSERT_TRUE("ts_at_ptime sec close",
                ts_out.tv_sec >= ts_in.tv_sec - 2 &&
                ts_out.tv_sec <= ts_in.tv_sec + 2);
}

/* ------------------------------------------------------------------ */
/* Arithmetic: ptm add/sub                                            */
/* ------------------------------------------------------------------ */

void test_ptm_add_ptm_basic(void)
{
    ptm a = {0}, b = {0}, result;
    a.units = 0x1234;
    b.units = 0x0001;
    int ok = ptm_add_ptm(&result, &a, &b);
    ASSERT_EQ_INT("add basic success", 1, ok);
    ASSERT_EQ_U16("add basic units", 0x1235, result.units);
}

void test_ptm_add_ptm_carry(void)
{
    /* Test carry from units into un */
    ptm a = {0}, b = {0}, result;
    a.units = 0xFFF0;
    b.units = 0x0020;
    int ok = ptm_add_ptm(&result, &a, &b);
    ASSERT_EQ_INT("add carry success", 1, ok);
    /* 0xFFF0 + 0x0020 = 0x10010, so units=0x0010 and carry propagates */
    ASSERT_EQ_U16("add carry units", 0x0010, result.units);
    ASSERT_EQ_U16("add carry un", 0x0001, result.un);
}

void test_ptm_add_ptm_overflow(void)
{
    /* Overflow at highest byte should return 0 */
    ptm a = {0}, b = {0}, result;
    a.doe = 0xFFFF;
    b.doe = 0x0001;
    int ok = ptm_add_ptm(&result, &a, &b);
    ASSERT_EQ_INT("add overflow returns 0", 0, ok);
}

void test_ptm_sub_ptm_basic(void)
{
    ptm a = {0}, b = {0}, result;
    a.units = 0x0010;
    b.units = 0x0005;
    int ok = ptm_sub_ptm(&result, &a, &b);
    ASSERT_EQ_INT("sub basic success", 1, ok);
    ASSERT_EQ_U16("sub basic units", 0x000B, result.units);
}

void test_ptm_sub_ptm_borrow(void)
{
    /* Borrow from un into units */
    ptm a = {0}, b = {0}, result;
    a.units = 0x0000;
    a.un    = 0x0001;
    b.units = 0x0001;
    int ok = ptm_sub_ptm(&result, &a, &b);
    ASSERT_EQ_INT("sub borrow success", 1, ok);
    ASSERT_EQ_U16("sub borrow units", 0xFFFF, result.units);
    ASSERT_EQ_U16("sub borrow un", 0x0000, result.un);
}

void test_ptm_sub_ptm_underflow(void)
{
    /* Subtracting larger from smaller should return 0 */
    ptm a = {0}, b = {0}, result;
    b.doe = 0x0001;
    int ok = ptm_sub_ptm(&result, &a, &b);
    ASSERT_EQ_INT("sub underflow returns 0", 0, ok);
}

void test_ptm_add_sub_identity(void)
{
    /* a + b - b == a */
    struct timespec ts = {1700000000, 123456789};
    ptm a, sum, roundtrip;
    ptime_at_ts(&a, &ts);

    ptm b = {0};
    b.nov = 0x0042;

    ptm_add_ptm(&sum, &a, &b);
    ptm_sub_ptm(&roundtrip, &sum, &b);

    ASSERT_EQ_U16("add-sub identity units", a.units, roundtrip.units);
    ASSERT_EQ_U16("add-sub identity un",    a.un,    roundtrip.un);
    ASSERT_EQ_U16("add-sub identity du",    a.du,    roundtrip.du);
    ASSERT_EQ_U16("add-sub identity tre",   a.tre,   roundtrip.tre);
    ASSERT_EQ_U16("add-sub identity quat",  a.quat,  roundtrip.quat);
    ASSERT_EQ_U16("add-sub identity quin",  a.quin,  roundtrip.quin);
    ASSERT_EQ_U16("add-sub identity hec",   a.hec,   roundtrip.hec);
    ASSERT_EQ_U16("add-sub identity sep",   a.sep,   roundtrip.sep);
    ASSERT_EQ_U16("add-sub identity oct",   a.oct,   roundtrip.oct);
    ASSERT_EQ_U16("add-sub identity nov",   a.nov,   roundtrip.nov);
    ASSERT_EQ_U16("add-sub identity dec",   a.dec,   roundtrip.dec);
    ASSERT_EQ_U16("add-sub identity el",    a.el,    roundtrip.el);
    ASSERT_EQ_U16("add-sub identity doe",   a.doe,   roundtrip.doe);
}

/* ------------------------------------------------------------------ */
/* Arithmetic: ptime variants                                         */
/* ------------------------------------------------------------------ */

void test_ptm_add_ptime(void)
{
    struct timespec ts = {500, 0};
    ptm a;
    ptime_at_ts(&a, &ts);
    ptime_t p = ptime_at_ptm(&a);

    ptm result;
    int ok = ptm_add_ptime(&result, &a, p);
    ASSERT_EQ_INT("ptm_add_ptime success", 1, ok);
    /* Adding p (nov-level) to a should roughly double the nov value */
    ASSERT_TRUE("ptm_add_ptime nov increased", result.nov > a.nov || result.dec > a.dec);
}

void test_ptime_add_ptm(void)
{
    struct timespec ts = {500, 0};
    ptm a;
    ptime_t p = ptime_at_ts(&a, &ts);

    ptm result;
    int ok = ptime_add_ptm(&result, p, &a);
    ASSERT_EQ_INT("ptime_add_ptm success", 1, ok);
}

void test_ptime_add_ptime(void)
{
    ptm result;
    int ok = ptime_add_ptime(&result, 0x100, 0x200);
    ASSERT_EQ_INT("ptime_add_ptime success", 1, ok);
    ptime_t p = ptime_at_ptm(&result);
    ASSERT_EQ_U64("ptime_add_ptime value", 0x300ULL, p);
}

void test_ptm_sub_ptime(void)
{
    struct timespec ts = {1000, 0};
    ptm a;
    ptime_at_ts(&a, &ts);

    ptm result;
    int ok = ptm_sub_ptime(&result, &a, 0x1);
    ASSERT_EQ_INT("ptm_sub_ptime success", 1, ok);
}

void test_ptime_sub_ptm(void)
{
    struct timespec ts = {100, 0};
    ptm a;
    ptime_t p = ptime_at_ts(&a, &ts);

    ptm one = {0};
    one.nov = 1;
    ptm result;
    int ok = ptime_sub_ptm(&result, p, &one);
    ASSERT_EQ_INT("ptime_sub_ptm success", 1, ok);
}

void test_ptime_sub_ptime(void)
{
    ptm result;
    int ok = ptime_sub_ptime(&result, 0x300, 0x100);
    ASSERT_EQ_INT("ptime_sub_ptime success", 1, ok);
    ptime_t p = ptime_at_ptm(&result);
    ASSERT_EQ_U64("ptime_sub_ptime value", 0x200ULL, p);
}

/* ------------------------------------------------------------------ */
/* Arithmetic: timespec variants                                      */
/* ------------------------------------------------------------------ */

void test_ptm_add_ts(void)
{
    struct timespec ts_a = {1000, 0}, ts_b = {500, 0};
    ptm a;
    ptime_at_ts(&a, &ts_a);

    ptm result;
    int ok = ptm_add_ts(&result, &a, &ts_b);
    ASSERT_EQ_INT("ptm_add_ts success", 1, ok);
    /* Result should be larger than a (ts_b adds a positive amount) */
    ASSERT_TRUE("ptm_add_ts result > a", result.doe > a.doe ||
                (result.doe == a.doe && result.el > a.el) ||
                (result.doe == a.doe && result.el == a.el && result.dec >= a.dec));
}

void test_ptime_add_ts(void)
{
    struct timespec ts_a = {1000, 0}, ts_b = {500, 0};
    ptm a;
    ptime_t p = ptime_at_ts(&a, &ts_a);

    ptm result;
    int ok = ptime_add_ts(&result, p, &ts_b);
    ASSERT_EQ_INT("ptime_add_ts success", 1, ok);
}

void test_ts_add_ptm(void)
{
    struct timespec ts = {1000, 0};
    ptm b = {0};
    b.nov = 1;

    ptm result;
    int ok = ts_add_ptm(&result, &ts, &b);
    ASSERT_EQ_INT("ts_add_ptm success", 1, ok);
}

void test_ts_add_ptime(void)
{
    struct timespec ts = {1000, 0};
    ptm result;
    int ok = ts_add_ptime(&result, &ts, 0x1);
    ASSERT_EQ_INT("ts_add_ptime success", 1, ok);
}

void test_ptm_sub_ts(void)
{
    struct timespec ts_a = {2000, 0}, ts_b = {500, 0};
    ptm a;
    ptime_at_ts(&a, &ts_a);

    ptm result;
    int ok = ptm_sub_ts(&result, &a, &ts_b);
    ASSERT_EQ_INT("ptm_sub_ts success", 1, ok);
    /* Result should be smaller than a */
    ASSERT_TRUE("ptm_sub_ts result < a", result.doe < a.doe ||
                (result.doe == a.doe && result.el < a.el) ||
                (result.doe == a.doe && result.el == a.el && result.dec <= a.dec));
}

void test_ptime_sub_ts(void)
{
    struct timespec ts_a = {2000, 0}, ts_b = {500, 0};
    ptm a;
    ptime_t p = ptime_at_ts(&a, &ts_a);

    ptm result;
    int ok = ptime_sub_ts(&result, p, &ts_b);
    ASSERT_EQ_INT("ptime_sub_ts success", 1, ok);
}

void test_ts_sub_ptm(void)
{
    struct timespec ts = {2000, 0};
    ptm b = {0};
    b.nov = 1;

    ptm result;
    int ok = ts_sub_ptm(&result, &ts, &b);
    ASSERT_EQ_INT("ts_sub_ptm success", 1, ok);
}

void test_ts_sub_ptime(void)
{
    struct timespec ts = {2000, 0};
    ptm result;
    int ok = ts_sub_ptime(&result, &ts, 0x1);
    ASSERT_EQ_INT("ts_sub_ptime success", 1, ok);
}

/* ------------------------------------------------------------------ */
/* Interval                                                           */
/* ------------------------------------------------------------------ */

void test_ts_interval_ptm_ptm(void)
{
    struct timespec ts_a = {0, 0}, ts_b = {10, 0};
    ptm a, b;
    ptime_at_ts(&a, &ts_a);
    ptime_at_ts(&b, &ts_b);

    struct timespec interval;
    int ok = ts_interval_ptm_ptm(&interval, &a, &b);
    ASSERT_EQ_INT("interval success", 1, ok);
    /* 10 seconds difference, allow +-1 for rounding */
    ASSERT_TRUE("interval ~10s", interval.tv_sec >= 9 && interval.tv_sec <= 11);
}

void test_ts_interval_same(void)
{
    struct timespec ts = {1000, 0};
    ptm a;
    ptime_at_ts(&a, &ts);

    struct timespec interval;
    int ok = ts_interval_ptm_ptm(&interval, &a, &a);
    ASSERT_EQ_INT("interval same success", 1, ok);
    ASSERT_EQ_LONG("interval same sec", 0, interval.tv_sec);
    ASSERT_EQ_LONG("interval same nsec", 0, interval.tv_nsec);
}

/* ------------------------------------------------------------------ */
/* Formatting: strfptm / strfptime                                    */
/* ------------------------------------------------------------------ */

void test_strfptm_full_format(void)
{
    ptm tm = {0};
    tm.units = 0xABCD;  /* Z='CD', Y='AB' */
    tm.un    = 0x1234;  /* X='34', W='12' */

    char buf[64];
    unsigned long n = strfptm(buf, sizeof(buf),
                              "%A%B%C%D%E%F%G%H%I%J%K%L%M%N%O%P%Q%R%S%T%U%V%W%X%Y%Z",
                              &tm);
    ASSERT_TRUE("strfptm full wrote chars", n == 52);
    /* Last 4 chars should be the units bytes */
    ASSERT_EQ_STR("strfptm units tail", "ABCD", &buf[48]);
}

void test_strfptm_with_literals(void)
{
    ptm tm = {0};
    tm.doe = 0x00FF;

    char buf[32];
    unsigned long n = strfptm(buf, sizeof(buf), "%A-%B", &tm);
    /* %A = doe high byte = 0x00 -> "00", then '-', then %B = doe low byte = 0xFF -> "FF" */
    ASSERT_TRUE("strfptm literal wrote", n == 5);
    ASSERT_EQ_STR("strfptm literal output", "00-FF", buf);
}

void test_strfptm_invalid_specifier(void)
{
    ptm tm = {0};
    char buf[32];
    unsigned long n = strfptm(buf, sizeof(buf), "%a", &tm);
    ASSERT_EQ_U64("strfptm invalid returns 0", 0ULL, (uint64_t)n);
}

void test_strfptm_max_limit(void)
{
    ptm tm = {0};
    char buf[3];
    /* max=3: loop runs while i+1 < 3, i.e. i <= 1. One specifier writes 2 chars
       at i=0,1 then i=2. Next: 2+1 < 3 is false, loop stops. Only 1 specifier fits. */
    unsigned long n = strfptm(buf, sizeof(buf), "%Z%Y", &tm);
    ASSERT_EQ_U64("strfptm truncates to 1 specifier", 2ULL, (uint64_t)n);
    ASSERT_EQ_STR("strfptm truncated output", "00", buf);
}

void test_strfptime(void)
{
    /* strfptime should produce same output as strfptm after ptm_at_ptime */
    ptime_t p = 0x0505EAD29401C940ULL;
    char buf1[64], buf2[64];
    ptm tm;
    ptm_at_ptime(&tm, p);
    unsigned long n1 = strfptm(buf1, sizeof(buf1), "%A%B%C%D", &tm);
    unsigned long n2 = strfptime(buf2, sizeof(buf2), "%A%B%C%D", p);
    ASSERT_EQ_U64("strfptime same length", (uint64_t)n1, (uint64_t)n2);
    ASSERT_EQ_STR("strfptime same output", buf1, buf2);
}

/* ------------------------------------------------------------------ */
/* Borrow test (preserved from original)                              */
/* ------------------------------------------------------------------ */

void test_sub_borrow(void)
{
    struct timespec ts_epoch = {0, 0};
    ptm ptm_epoch;
    ptime_at_ts(&ptm_epoch, &ts_epoch);

    struct timespec ts_289 = {289, 0};
    ptm ptm_289, ptm_dur;
    ptime_at_ts(&ptm_289, &ts_289);
    ptm_sub_ptm(&ptm_dur, &ptm_289, &ptm_epoch);

    /* 289 seconds ~ 0xF0 novs (each nov ~ 1.2 seconds) */
    ASSERT_RANGE_U16("289s duration nov ~0xF0", ptm_dur.nov, 0xEF, 0xF1);

    /* Verify monotonicity: each second should increment by ~0.83 novs */
    int i;
    uint16_t prev_nov = 0;
    for (i = 0; i <= 360; ++i)
    {
        struct timespec ts = {i, 0};
        ptm ptm_abs, ptm_d;
        ptime_at_ts(&ptm_abs, &ts);
        ptm_sub_ptm(&ptm_d, &ptm_abs, &ptm_epoch);
        ASSERT_TRUE("monotonicity", ptm_d.nov >= prev_nov);
        prev_nov = ptm_d.nov;
    }

    /* 360 seconds ~ 0x12C novs */
    ASSERT_RANGE_U16("360s duration nov ~0x12C", prev_nov, 0x12A, 0x12E);
}

/* ------------------------------------------------------------------ */
/* ptime_now                                                          */
/* ------------------------------------------------------------------ */

void test_ptime_now(void)
{
    ptm tm;
    ptime_t p = ptime_now(&tm);
    ASSERT_TRUE("ptime_now nonzero", p != 0);

    /* Should be convertible back to a reasonable timespec */
    struct timespec ts;
    int ok = ts_at_ptm(&ts, &tm);
    ASSERT_EQ_INT("ptime_now ts_at_ptm success", 1, ok);
    /* Should be after 2020 and before 2100 */
    ASSERT_TRUE("ptime_now sec reasonable", ts.tv_sec > 1577836800L && ts.tv_sec < 4102444800L);
}

/* ------------------------------------------------------------------ */
/* Commutativity of addition                                          */
/* ------------------------------------------------------------------ */

void test_add_commutative(void)
{
    struct timespec ts_a = {100, 0}, ts_b = {200, 0};
    ptm a, b, sum_ab, sum_ba;
    ptime_at_ts(&a, &ts_a);
    ptime_at_ts(&b, &ts_b);

    ptm_add_ptm(&sum_ab, &a, &b);
    ptm_add_ptm(&sum_ba, &b, &a);

    ASSERT_EQ_U16("commutative units", sum_ab.units, sum_ba.units);
    ASSERT_EQ_U16("commutative un",    sum_ab.un,    sum_ba.un);
    ASSERT_EQ_U16("commutative nov",   sum_ab.nov,   sum_ba.nov);
    ASSERT_EQ_U16("commutative dec",   sum_ab.dec,   sum_ba.dec);
    ASSERT_EQ_U16("commutative doe",   sum_ab.doe,   sum_ba.doe);
}

/* ------------------------------------------------------------------ */
/* Add zero is identity                                               */
/* ------------------------------------------------------------------ */

void test_add_zero_identity(void)
{
    struct timespec ts = {999, 123456789};
    ptm a, result;
    ptm zero = {0};
    ptime_at_ts(&a, &ts);

    ptm_add_ptm(&result, &a, &zero);
    ASSERT_EQ_U16("add zero units", a.units, result.units);
    ASSERT_EQ_U16("add zero un",    a.un,    result.un);
    ASSERT_EQ_U16("add zero du",    a.du,    result.du);
    ASSERT_EQ_U16("add zero tre",   a.tre,   result.tre);
    ASSERT_EQ_U16("add zero nov",   a.nov,   result.nov);
    ASSERT_EQ_U16("add zero dec",   a.dec,   result.dec);
    ASSERT_EQ_U16("add zero doe",   a.doe,   result.doe);
}

/* ------------------------------------------------------------------ */
/* Sub self is zero                                                   */
/* ------------------------------------------------------------------ */

void test_sub_self_is_zero(void)
{
    struct timespec ts = {86400, 0};
    ptm a, result;
    ptime_at_ts(&a, &ts);

    int ok = ptm_sub_ptm(&result, &a, &a);
    ASSERT_EQ_INT("sub self success", 1, ok);
    ASSERT_EQ_U16("sub self units", 0, result.units);
    ASSERT_EQ_U16("sub self un",    0, result.un);
    ASSERT_EQ_U16("sub self du",    0, result.du);
    ASSERT_EQ_U16("sub self tre",   0, result.tre);
    ASSERT_EQ_U16("sub self quat",  0, result.quat);
    ASSERT_EQ_U16("sub self quin",  0, result.quin);
    ASSERT_EQ_U16("sub self hec",   0, result.hec);
    ASSERT_EQ_U16("sub self sep",   0, result.sep);
    ASSERT_EQ_U16("sub self oct",   0, result.oct);
    ASSERT_EQ_U16("sub self nov",   0, result.nov);
    ASSERT_EQ_U16("sub self dec",   0, result.dec);
    ASSERT_EQ_U16("sub self el",    0, result.el);
    ASSERT_EQ_U16("sub self doe",   0, result.doe);
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== plancktimelib tests ===\n\n");

    /* Conversion: self */
    test_ptm_ptime_roundtrip();
    test_ptm_ptime_zero();
    test_ptm_ptime_max();

    /* Conversion: timespec */
    test_ts_roundtrip();
    test_ts_epoch();
    test_ts_at_ptime();

    /* Arithmetic: ptm */
    test_ptm_add_ptm_basic();
    test_ptm_add_ptm_carry();
    test_ptm_add_ptm_overflow();
    test_ptm_sub_ptm_basic();
    test_ptm_sub_ptm_borrow();
    test_ptm_sub_ptm_underflow();
    test_ptm_add_sub_identity();

    /* Arithmetic: ptime variants */
    test_ptm_add_ptime();
    test_ptime_add_ptm();
    test_ptime_add_ptime();
    test_ptm_sub_ptime();
    test_ptime_sub_ptm();
    test_ptime_sub_ptime();

    /* Arithmetic: timespec variants */
    test_ptm_add_ts();
    test_ptime_add_ts();
    test_ts_add_ptm();
    test_ts_add_ptime();
    test_ptm_sub_ts();
    test_ptime_sub_ts();
    test_ts_sub_ptm();
    test_ts_sub_ptime();

    /* Interval */
    test_ts_interval_ptm_ptm();
    test_ts_interval_same();

    /* Formatting */
    test_strfptm_full_format();
    test_strfptm_with_literals();
    test_strfptm_invalid_specifier();
    test_strfptm_max_limit();
    test_strfptime();

    /* Borrow (original test) */
    test_sub_borrow();

    /* Other */
    test_ptime_now();

    /* Properties */
    test_add_commutative();
    test_add_zero_identity();
    test_sub_self_is_zero();

    printf("\n=== Results: %d passed, %d failed ===\n", passed, failures);
    return failures;
}
