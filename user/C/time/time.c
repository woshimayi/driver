/*
 * @*************************************:
 * @FilePath: /user/C/time/time.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2024-03-22 16:11:15
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

// char *strftime_HHMMSS(char *buf, unsigned len, time_t *tp)
// {
// 	return strftime_fmt(buf, len, tp, "%H:%M:%S");
// }

// char *strftime_YYYYMMDDHHMMSS(char *buf, unsigned len, time_t *tp)
// {
// 	return strftime_fmt(buf, len, tp, "%Y-%m-%d %H:%M:%S");
// }

int getDateTime(unsigned int t)
{
	int rc = 0;
	time_t now;
	struct tm *tmp;
	if (t == 0)
	{
		now = time(NULL);
	}
	else
	{
		now = t;
	}

	tmp = localtime(&now);

	rc = mktime(tmp);
	if (rc == 0)
	{
		printf("get localtime failed\n");
		return -1;
	}
	char s[100] = {0};
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", tmp);
	printf("rc = %d s = %s %s\n", rc, s, asctime(tmp));
	return rc;
}

#if 0
int main()
{
#if 1
	char *str = NULL;
	getTheCurrentTime(str);
	//
	time_t t;
	char buf[1024];
	time(&t);
	// printf("%d\n", t.tm_sec);

	ctime_r(&t, buf);
	printf("111111111 %s\n", buf);

	unsigned int nt;
	struct tm *p;
	time_t t2;
	char s[100] = {0};
	nt = getDateTime(t);
	// p = localtime(&t2);
	
	printf("zzzzz nt = %d %s\n", nt, s);
#endif

	if (0)
	{
		// char time[100] = "2020/03/25 11:09:42 .761197";
		char time_format[100] = "%Y/%m/%d %H:%M:%S";
		// printf("origin  is %s\n", time);

		/* to tm */
		struct tm tm;
		strptime(time, time_format, &tm);
		printf("tm      is %04d-%02d-%02d %02d:%02d:%02d\n", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		/* to time_t */
		time_t tt;
		tt = mktime(&tm);
		printf("time_t  is %lu\n", tt);

		/* to timeval */
		struct timeval tv;
		tv.tv_sec = tt;
		// tv.tv_usec = atoi(time + 21);		// 21 = strlen("2020/03/25 11:09:42 .")
		printf("timeval is %ld.%06ld\n", tv.tv_sec, tv.tv_usec);

	}
	// printf("%s\n", str);
	return 0;
}
#endif

#include <time.h>
int main()
{
	char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep); /*取得当地时间*/
	// printf("%d%d%d ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);
	// printf("%s%d:%d:%d\n", wday[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec);

	timep = mktime(p);
	printf("time()->localtime()->mktime():%d\n", timep);

	char str[128] = {0};
	getTheCurrentTime(str);
	printf("str = %s\n", str);
}