#include <stdio.h>

#include "planckclock.h"

int main()
{
    planck_tm* tm;
    const planck_time_t time_now = planck_time_now(&tm);
    printf("%lx\n", time_now);

    printf("%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x\n",
           tm->doe, tm->el, tm->dec, tm->nov, tm->oct, tm->sep, tm->hec,
           tm->quin, tm->quat, tm->tre, tm->du, tm->un, tm->units);

    char buf[sizeof(planck_tm) + 1];
    printf("%lu\n", planck_strftime(buf, 24, "%C%D %E %F:%G %H ", tm));
    puts(buf);

    return 0;
}