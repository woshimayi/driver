#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sighandler(int);

int main()
{
    signal(SIGINT, sighandler);

    while (1)
    {
        printf("��ʼ����һ����...\n");
        sleep(1);
    }

    return (0);
}

void sighandler(int signum)
{
    printf("�����ź� %d������...\n", signum);
    exit(1);
}
