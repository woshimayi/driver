#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>


double xu_wallclock(void);

#define MAX_RUNTIME -10*365*365*24*60*60.0 /** Timing will fail if execution takes longer than 10 years */
#define timer_clear(tmr) (tmr = 0.0)
#define timer_start(tmr) do {if(tmr > MAX_RUNTIME){tmr -= xu_wallclock();}} while(0)
#define timer_stop(tmr) do {if(tmr < MAX_RUNTIME){tmr += xu_wallclock();}} while(0)



double xu_wallclock1(void)
{
#ifdef __GNUC__
    struct timeval ctime;

    gettimeofday(&ctime, NULL);

    return (1.0e+6 * (double)ctime.tv_sec + (double)ctime.tv_usec);
#else
    return (double)time(NULL);
#endif
}

double xu_wallclock(void)
{
#ifdef __GNUC__
    struct timespec ctime;
    int error;

    error = clock_gettime(CLOCK_MONOTONIC_RAW, &ctime);

    return (1.0e+9 * (double)ctime.tv_sec + (double)ctime.tv_nsec);
#else
    return (double)time(NULL);
#endif
}



int main()
{
    double timer;
    int t;

    t = 0;
    timer = 0.0;
    timer_start(timer);
    while(t < 1000)
    {
        t++;
    }
    timer_stop(timer);
    printf("timeflash:%.2lfns\n", timer);
}
