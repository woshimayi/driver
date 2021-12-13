#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef unsigned long  UINT64;


int bua_getSecondsSince1970(UINT64 *currentTime)
{
	//    struct timeval tv;
	//    struct timezone tz;
	//
	//    gettimeofday (&tv, NULL);
	//
	//	*currentTime = tv.tv_sec;
	//    printf("current time get is %lu %lu\n", tv.tv_sec, *currentTime);


	FILE *fp = NULL;
	char timeStr[64] = {0};
	fp = popen("date \"+%s\"", "r");
	if (NULL != fp)
	{
		fgets(timeStr, 5, fp);
	}
	pclose(fp);

	//use 2016 to determine whether current time is right or not
	vosLog_debug("%d", atoi(timeStr));


	return 0;
}



int main()
{
	UINT64 currentTime = 0;
	bua_getSecondsSince1970(&currentTime);
	printf("currentTime       = %d\n", currentTime);
	return 0;
}


