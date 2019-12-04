#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>

/*seconds: the seconds; mseconds: the micro seconds*/
void setTimer(int seconds, int mseconds)
{
        struct timeval temp_u;
        temp_u.tv_sec = seconds;
        temp_u.tv_usec = mseconds*1000;

        select(0, NULL, NULL, NULL, &temp_u);
        printf("timer\n");

        return ;
}

#if 0
void setTimer(int seconds, int nseconds)
{
        struct timespec temp_n;
        temp_n.tv_sec
        temp_n.tv_nsec

        select(0, NULL, NULL, NULL, &temp_n);
        printf("timer\n");

        return ;
}
#endif


int main()
{
    int i;

    for(i = 0 ; i < 100; i++)
            setTimer(1, 500);

    return 0;
}
