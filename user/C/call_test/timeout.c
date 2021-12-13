#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef unsigned long int 	UINT64;
typedef   signed long int 	SINT64;
typedef unsigned int    	UINT32;
typedef   signed int    	SINT32;
typedef unsigned short  	UINT16;
typedef   signed short  	SINT16;
typedef unsigned char   	UINT8;
typedef   signed char    	SINT8;


typedef unsigned char UBOOL8;
#define TRUE  1
#define FALSE 0


typedef struct
{
	int sec;   /**< Number of seconds since some arbitrary point. */
	int nsec;  /**< Number of nanoseconds since some arbitrary point. */
} CmsTimestamp;


/** OS independent timestamp structure.
 */
typedef struct
{
	UINT32 sec;   /**< Number of seconds since some arbitrary point. */
	UINT32 nsec;  /**< Number of nanoseconds since some arbitrary point. */
} UtilTimestamp;



#ifdef linux
/** OS dependent timestamp functions go in this file.
 */
void oalTms_get(CmsTimestamp *tms)
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
		printf("clock_gettime failed, set timestamp to 0");
		tms->sec = 0;
		tms->nsec = 0;
	}
}
#endif // linux

double xu_wallclock(void)
{
#ifdef __GNUC__
	struct timespec ctime;
	int error;

	//    error = clock_gettime(CLOCK_MONOTONIC_RAW, &ctime);

	return (1.0e+9 * (double)ctime.tv_sec + (double)ctime.tv_nsec);
#else
	return (double)time(NULL);
#endif
}


#if 0
int main()
{

	CmsTimestamp cmsTimestamp;
	oalTms_get(&cmsTimestamp);
	printf("%d %d\n", cmsTimestamp.nsec, cmsTimestamp.sec);

	CmsTimestamp cmsTimestamp1;
	oalTms_get(&cmsTimestamp1);
	printf("%d %d\n", cmsTimestamp1.nsec, cmsTimestamp1.sec);

	printf("%d %d\n", cmsTimestamp1.nsec - cmsTimestamp.nsec, cmsTimestamp1.sec - cmsTimestamp.sec);

	return 0;
}
#endif
