#include <time.h>
#include <stdio.h>

#ifdef linux

#include <sys/time.h>
#include <sys/select.h>

/*seconds: the seconds; mseconds: the micro seconds*/
void setTimer(int seconds, int mseconds)
{
        struct timeval temp;

        temp.tv_sec = seconds;
        temp.tv_usec = mseconds*1000;

        select(0, NULL, NULL, NULL, &temp);
        printf("timer\n");

        return ;
}

#endif
