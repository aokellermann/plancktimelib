#include <stdio.h>
#include <time.h>

#include "planckclock.h"

int main()
{
    printf("Current unix time: %ld\n", time(NULL));

    planck_tm *tm;
    const planck_time_t time_now = planck_time_now(&tm);
    printf("planck_time_t now: %lx\n", time_now);

    printf("planck_tm bytes: %04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x\n",
           tm->doe, tm->el, tm->dec, tm->nov, tm->oct, tm->sep, tm->hec,
           tm->quin, tm->quat, tm->tre, tm->du, tm->un, tm->units);

    char buf[sizeof(planck_tm) + 1];
    printf("planck_strftime bytes written: %lu\n",
           planck_strftime(buf, 24, "%C%D %E %F:%G %H ", tm));
    printf("planck_strftime out: %s\n", buf);

    struct timeval tv;
    tv_at_planck_time(&tv, tm);
    printf("tv_at_planck_time: %ld.%ld\n", tv.tv_sec, tv.tv_usec);

    return 0;
}