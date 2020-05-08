#include <stdio.h>
#include <time.h>

#include "planckclock.h"

int main()
{
    printf("Current unix time: %ld\n", time(NULL));

    ptm tm;
    ptime_t time_now = ptime_now(&tm);
    printf("ptime_t now: %lx\n", time_now);

    printf("ptm bytes: %04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x\n",
           tm.doe, tm.el, tm.dec, tm.nov, tm.oct, tm.sep, tm.hec,
           tm.quin, tm.quat, tm.tre, tm.du, tm.un, tm.units);

    char buf[sizeof(ptm) + 1];
    printf("planck_strftime bytes written: %lu\n",
           strfptm(buf, sizeof(ptm), "%C%D %E %F:%G %H ", &tm));
    printf("planck_strftime out: %s\n", buf);

    struct timespec ts_now;
    ts_at_ptm(&ts_now, &tm);
    printf("ts now: %ld.%09ld\n", ts_now.tv_sec, ts_now.tv_nsec);

    ptm ptm_next_nov, ptm_one_nov = {0};
    ptm_one_nov.nov = 1;
    ptm_add_ptm(&ptm_next_nov, &tm, &ptm_one_nov);
    char buf2[sizeof(ptm) * 2 + 1];
    strfptm(buf2, sizeof(ptm) * 2, "%A%B%C%D%E%F%G%H%I%J%K%L%M%N%O%P%Q%R%S%T%U%V%W%X%Y%Z", &ptm_next_nov);
    printf("time at next nov: %s\n", buf2);

    ptm ptm_sub_now;
    ptm_sub_ptm(&ptm_sub_now, &ptm_next_nov, &ptm_one_nov);
    strfptm(buf2, sizeof(ptm) * 2, "%A%B%C%D%E%F%G%H%I%J%K%L%M%N%O%P%Q%R%S%T%U%V%W%X%Y%Z", &ptm_sub_now);
    printf("time now: %s\n", buf2);

    struct timespec ts_next_nov;
    ts_interval_ptm_ptm(&ts_next_nov, &ptm_sub_now, &ptm_next_nov);
    printf("ts interval until next nov: %ld.%09ld\n", ts_next_nov.tv_sec, ts_next_nov.tv_nsec);

    return 0;
}