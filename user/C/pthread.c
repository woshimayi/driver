#include <pthread.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

/**
 * 用户自定义signal
 */
#define   SIG_MY_MSG   SIGUSR1+100  

void userFun(int a)
{
    printf("signal user = %d\n", a);
    return ;
}

void *tprocess1(void *a)
{
    int i = 0;
    pthread_detach(pthread_self());
    while (1)
    {
        printf("%d 11111111 %d\n", i++, *(int *)a);
        sleep(1);
    }
    pthread_exit(0);
    return NULL;
}

/**
 * [tprocess2 线程接受其他进程signal, 进行中断处理]
 * @param  a [description]
 * @return   [description]
 */
void *tprocess2(void *a)
{
    int i = 0;
    char *buf = NULL;
    pthread_detach(pthread_self());
    while (1)
    {
        buf = getenv("POT_NTP");
        signal(SIG_MY_MSG, userFun);
        if (NULL != buf)
        {
            printf("buf = %s\n", buf);
            printf("%d 222222222 %d %s\n", i++, *(int *)a, buf);
        }
        sleep(1);
    }
    pthread_exit(0);
    return NULL;
}

int main()
{
    pthread_t t1;
    pthread_t t2;

    int a = 3;
    int b = 5;

    char str[64] = {0};
    snprintf(str, sizeof(str), "%d", getpid());
    printf("uhttp pid%s\n", str);
    setenv("UHTTPD_PID", str, 1);

    if (0 != pthread_create(&t1, NULL, (void*)tprocess1, (void *)&a))
    {
        perror("fail error 1");
    }
    if (0 != pthread_create(&t2, NULL, (void*)tprocess2, (void *)&b))
    {
        perror("fail error 1");
    }
    while (1)
    {
        // sleep(1);
        // printf("asdasd %d\n", a++);
        a = a ;
    }

    return 0;
}
