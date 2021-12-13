/*
	Name:
	Copyright:
	Author:
	Date: 08/01/20 11:35
	Description:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>




typedef struct
{
	unsigned int  sec;   /**< Number of seconds since some arbitrary point. */
	unsigned int  nsec;  /**< Number of nanoseconds since some arbitrary point. */
} UtilTimestamp;


void oalTms_get(UtilTimestamp *tms)
{
	struct timespec ts;
	int rc;

	if (tms == NULL)
	{
		return;
	}

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (rc == 0)
	{
		tms->sec = ts.tv_sec;
		tms->nsec = ts.tv_nsec;
	}
	else
	{
		printf("clock_gettime failed, set timestamp to 0\n");
		tms->sec = 0;
		tms->nsec = 0;
	}
}



int main(int argc, char *argv[])
{
	UtilTimestamp ts;
	utilTms_get(&ts);
	printf("%s\n", ts.sec);

	return 0;
}
