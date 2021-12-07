#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#ifdef linux

#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>

/*seconds: the seconds; mseconds: the micro seconds*/
void *setTimer(void * args)
{
    struct timeval temp;

	while(1)
	{
	    temp.tv_sec = 15;
	    temp.tv_usec = 0;
		printf("ssssssssss\n");
	    select(0, NULL, NULL, NULL, &temp);
	    printf("timer\n");
	}
}

#endif



int main(int argc, const char *argv[])
{
	pthread_t pid;
	if (pthread_create(&pid, NULL, setTimer, NULL))
	{
		printf("fail \n");
	}
	while(1)
	{
		sleep(1);
		printf("ssssssssssssss\n");
	}
	return 0;
}


