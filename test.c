#include <stdio.h>

#include "planckclock.h"

int main()
{
    const planck_time_t time_now = planck_time(NULL);
    printf("%lx\n", time_now);

    planck_tm *tm = planck_localtime(&time_now);
    printf("%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x%04x\n",
           tm->doe, tm->el, tm->dec, tm->nov, tm->oct, tm->sep, tm->hec,
           tm->quin, tm->quat, tm->tre, tm->du, tm->un);

    char buf[24];
    printf("%d\n", planck_strftime(buf, 24, "%C%D %E %F:%G %H ", tm));
    puts(buf);

    return 0;
}