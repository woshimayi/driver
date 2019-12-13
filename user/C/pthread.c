#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void *tprocess1(void *a)
{
    int i = 0;
    while (1)
    {
        printf("%d 11111111 %d\n", i++, *(int *)a);
        sleep(1);
    }
    return NULL;
}

void *tprocess2(void *a)
{
    int i = 0;
    while (1)
    {
        printf("%d 222222222 %d\n", i++, *(int *)a);
        sleep(1);
    }
    return NULL;
}

int main()
{
    pthread_t t1;
    pthread_t t2;

    int a = 3;
    int b = 5;

    pthread_create(&t1, NULL, tprocess1, (void *)&a);
    pthread_create(&t2, NULL, tprocess2, (void *)&b);

    return 0;
}
