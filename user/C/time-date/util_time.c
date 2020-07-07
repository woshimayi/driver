#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define UINT32  unsigned int
#define SINT32  int

#define NSECS_IN_SEC 1000000000

/** Number of nanoseconds in 1 milli-second. */
#define NSECS_IN_MSEC 1000000

/** Number of nanoseconds in 1 micro-second. */
#define NSECS_IN_USEC 1000

/** Number of micro-seconds in 1 second. */
#define USECS_IN_SEC  1000000

/** Number of micro-seconds in a milli-second. */
#define USECS_IN_MSEC 1000

/** Number of milliseconds in 1 second */
#define MSECS_IN_SEC  1000

/** Number of seconds in a minute */
#define SECS_IN_MINUTE   60

/** Number of seconds in an hour */
#define SECS_IN_HOUR     (SECS_IN_MINUTE * 60)

/** Number of seconds in a day */
#define SECS_IN_DAY      (SECS_IN_HOUR * 24)

/** Maximum value for a UINT64 */
#define MAX_UINT64 18446744073709551615ULL

/** Maximum value for a SINT64 */
#define MAX_SINT64 9223372036854775807LL

/** Minimum value for a SINT64 */
#define MIN_SINT64 (-1 * MAX_SINT64 - 1)

/** Maximum value for a UINT32 */
#define MAX_UINT32 4294967295U

/** Maximum value for a SINT32 */
#define MAX_SINT32 2147483647

/** Minimum value for a SINT32 */
#define MIN_SINT32 (-2147483648)

/** Maximum value for a UINT16 */
#define MAX_UINT16  65535

/** Maximum value for a SINT16 */
#define MAX_SINT16  32767

/** Minimum value for a SINT16 */
#define MIN_SINT16  (-32768)

//int snprintf(char *str, size_t size, const char *format, ...)
//{
//  va_list args;
//
//  va_start (args, format);
//
//  return vsprintf (str, format, args);
//}


typedef struct
{
    UINT32 sec;   /**< Number of seconds since some arbitrary point. */
    UINT32 nsec;  /**< Number of nanoseconds since some arbitrary point. */
} UtilTimestamp;


/** OS dependent timestamp functions go in this file.
 */
void oalTms_get(UtilTimestamp *tms)
{
    struct timespec ts;
    SINT32 rc;

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


int oalTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen)
{
    int          c;
    time_t       now;
    struct tm   *tmp;

    if (t == 0)
    {
        now = time(NULL);
    }
    else
    {
        now = t;
    }

    tmp = localtime(&now);
    memset(buf, 0, bufLen);
    c = strftime(buf, bufLen, "%Y-%m-%d T %H:%M:%S %z", tmp);
    if ((c == 0) || (c + 1 > bufLen))
    {
        /* buf was not long enough */
        return 1;
    }

    /* fix missing : in time-zone offset-- change -500 to -5:00 */
    buf[c + 1] = '\0';
    buf[c] = buf[c - 1];
    buf[c - 1] = buf[c - 2];
    buf[c - 2] = ':';
    return 0;
}



void utilTms_get(UtilTimestamp *tms)
{
    oalTms_get(tms);
}

void utilTms_delta(const UtilTimestamp *newTms,
                   const UtilTimestamp *oldTms,
                   UtilTimestamp *deltaTms)
{
    if (newTms->sec >= oldTms->sec)
    {
        if (newTms->nsec >= oldTms->nsec)
        {
            /* no roll-over in the sec and nsec fields, straight subtract */
            deltaTms->nsec = newTms->nsec - oldTms->nsec;
            deltaTms->sec = newTms->sec - oldTms->sec;
        }
        else
        {
            /* no roll-over in the sec field, but roll-over in nsec field */
            deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
            deltaTms->sec = newTms->sec - oldTms->sec - 1;
        }
    }
    else
    {
        if (newTms->nsec >= oldTms->nsec)
        {
            /* roll-over in the sec field, but no roll-over in the nsec field */
            deltaTms->nsec = newTms->nsec - oldTms->nsec;
            deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec + 1; /* +1 to account for time spent during 0 sec */
        }
        else
        {
            /* roll-over in the sec and nsec fields */
            deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
            deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec;
        }
    }
}


UINT32 utilTms_deltaInMilliSeconds(const UtilTimestamp *newTms,
                                   const UtilTimestamp *oldTms)
{
    UtilTimestamp deltaTms;
    unsigned int  ms;

    utilTms_delta(newTms, oldTms, &deltaTms);

    if (deltaTms.sec > MAX_UINT32 / MSECS_IN_SEC)
    {
        /* the delta seconds is larger than the UINT32 return value, so return max value */
        ms = MAX_UINT32;
    }
    else
    {
        ms = deltaTms.sec * MSECS_IN_SEC;

        if ((MAX_UINT32 - ms) < (deltaTms.nsec / NSECS_IN_MSEC))
        {
            /* overflow will occur when adding the nsec, return max value */
            ms = MAX_UINT32;
        }
        else
        {
            ms += deltaTms.nsec / NSECS_IN_MSEC;
        }
    }

    return ms;
}


void utilTms_addMilliSeconds(UtilTimestamp *tms, UINT32 ms)
{
    UINT32 addSeconds;
    UINT32 addNano;

    addSeconds = ms / MSECS_IN_SEC;
    addNano = (ms % MSECS_IN_SEC) * NSECS_IN_MSEC;

    tms->sec += addSeconds;
    tms->nsec += addNano;

    /* check for carry-over in nsec field */
    if (tms->nsec > NSECS_IN_SEC)
    {
        /* we can't have carried over by more than 1 second */
        tms->sec++;
        tms->nsec -= NSECS_IN_SEC;
    }

    return;
}


int utilTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen)
{
    return (oalTms_getXSIDateTime(t, buf, bufLen));
}

int utilTms_getDaysHoursMinutesSeconds(UINT32 t, char *buf, UINT32 bufLen)
{
    UINT32 days, hours, minutes, seconds;
    SINT32 r;
    int ret = 0;

    days = t / SECS_IN_DAY;
    t -= (days * SECS_IN_DAY);

    hours = t / SECS_IN_HOUR;
    t -= (hours * SECS_IN_HOUR);

    minutes = t / SECS_IN_MINUTE;
    t -= (minutes * SECS_IN_MINUTE);

    seconds = t;

    memset(buf, 0, bufLen);
    r = snprintf(buf, bufLen - 1, "%dD %dH %dM %dS", days, hours, minutes, seconds);
    if (r >= bufLen)
    {
        ret = 1;
    }

    return ret;
}

void sleep_ms(UINT32 ms)
{
    UtilTimestamp *oldtime = (UtilTimestamp *)malloc(sizeof(UtilTimestamp));
    UtilTimestamp *newtime = (UtilTimestamp *)malloc(sizeof(UtilTimestamp));
    oalTms_get(oldtime);
    int i = 0;
    while (1)
    {
        oalTms_get(newtime);
        i = utilTms_deltaInMilliSeconds(newtime, oldtime);
        if (ms == i)
        {
            break;
        }
    }
    return ;
}

int main()
{
    //	UtilTimestamp *oldtime = (UtilTimestamp*)malloc(sizeof(UtilTimestamp));
    //	UtilTimestamp *newtime = (UtilTimestamp*)malloc(sizeof(UtilTimestamp));
    //	oalTms_get(oldtime);
    //	printf("%d, %d\n", oldtime->nsec, oldtime->sec);
    //
    //	char date[256] = {0};
    //	utilTms_getDaysHoursMinutesSeconds(oldtime->sec, date, sizeof(date));
    //	printf("%s\n", date);
    //
    //	int i = 0;
    //	while(1)
    //	{
    //		oalTms_get(newtime);
    //		i = utilTms_deltaInMilliSeconds(newtime, oldtime);
    //		if(500 == i)
    //		{
    //			break;
    //		}
    //	}
    //	printf("i = %d\n", i);
    //
    //	utilTms_getXSIDateTime(0, date, sizeof(date));
    //	printf("date = %s\n", date);

    //	time_t rawtime;
    //    struct tm * timeinfo;
    //
    //    time ( &rawtime );
    //    timeinfo = localtime (&rawtime);
    //    printf( "The current date/time is: %s", asctime (timeinfo));
    //
    sleep_ms(500);

    return 0;
}
