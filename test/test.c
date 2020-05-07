#include <stdio.h>
#include <time.h>

#include "planckclock.h"

int main()
{
    printf("Current unix time: %ld\n", time(NULL));

    planck_tm *tm;
    planck_time_t time_now = planck_time_now(&tm);
    printf("planck_time_t now: %lx\n", time_now);

    printf("planck_tm bytes: %04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x\n",
           tm->doe, tm->el, tm->dec, tm->nov, tm->oct, tm->sep, tm->hec,
           tm->quin, tm->quat, tm->tre, tm->du, tm->un, tm->units);

    char buf[sizeof(planck_tm) + 1];
    printf("planck_strftime bytes written: %lu\n",
           planck_strftime(buf, 24, "%C%D %E %F:%G %H ", tm));
    printf("planck_strftime out: %s\n", buf);

    struct timespec tv_now;
    ts_at_planck_time(&tv_now, tm);
    printf("tv now: %ld.%09ld\n", tv_now.tv_sec, tv_now.tv_nsec);

    ++time_now;
    printf("planck_time_t next nov: %lx\n", time_now);

    planck_tm next_nov;
    planck_tm_at_planck_time(&next_nov, time_now);
    printf("next nov bytes: %04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x\n",
           next_nov.doe, next_nov.el, next_nov.dec, next_nov.nov, next_nov.oct, next_nov.sep, next_nov.hec,
           next_nov.quin, next_nov.quat, next_nov.tre, next_nov.du, next_nov.un, next_nov.units);

    struct timespec ts_next_nov;
    ts_at_planck_time(&ts_next_nov, &next_nov);
    printf("tv at next nov: %ld.%09ld\n", ts_next_nov.tv_sec, ts_next_nov.tv_nsec);

    struct timespec ts_difference;
    planck_difftime_get_ts(tm, &next_nov, &ts_difference);
    printf("difference between novs: %ld.%09ld\n", ts_difference.tv_sec, ts_difference.tv_nsec);

    return 0;
}