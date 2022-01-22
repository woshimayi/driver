/*
 * @*************************************: 
 * @FilePath: /user/C/time/time.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2022-01-20 19:29:55
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


static void getTheCurrentTime(char *time_now)
{
	time_t now;
	struct tm *timenow;

	time(&now);
	timenow = localtime(&now);
	snprintf(time_now, 32, "%d%02d%02d%02d%02d%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday,
	         timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
}


int getDateTime(unsigned int t)
{
	int rc = 0;
	time_t now;
	struct tm   *tmp;
	if(t == 0)
	{
		now = time(NULL);
	}
	else
	{
		now = t;
	}
	
	tmp = localtime(&now);

	rc = mktime(tmp);
	if(rc == 0)
	{
		printf("get localtime failed\n");
		return -1;
	}
	return rc;
}



int main()
{
	char *str = NULL;
	// getTheCurrentTime(str);
	//
	time_t t;
	char buf[1024];
	time(&t);
	// printf("%d\n", t.tm_sec);

	ctime_r(&t, buf);
	printf("%s\n", buf);

	unsigned int nt;
	int i = getDateTime(nt);
	printf("zzzzz nt = %ld i = %ld\n", nt, i);

	// printf("%s\n", str);
	return 0;
}