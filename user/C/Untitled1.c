#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef int        uint32_t;
typedef uint32_t   UINT32;
static UINT32 logHeaderMask;

typedef struct
{
    VosLogLevel logLevel;
    VosLogDestination logDestination;
    char cache[32][80];
    UINT32 location;
} VOS_LOG_SHARED_MEM_T;
static VOS_LOG_SHARED_MEM_T *logShareMem = NULL;

typedef enum
{
    VOS_LOG_DEST_STDERR  = 1,  /**< Message output to stderr. */
    VOS_LOG_DEST_SYSLOG  = 2,  /**< Message output to syslog. */
    VOS_LOG_DEST_TELNET  = 3,   /**< Message output to telnet clients. */
    VOS_LOG_DEST_LOGCAT  = 4
} VosLogDestination;


typedef enum
{
    VOS_LOG_LEVEL_PRINT  = 2,
    VOS_LOG_LEVEL_ERR    = 3, /**< Message at error level. */
    VOS_LOG_LEVEL_NOTICE = 5, /**< Message at notice level. */
    VOS_LOG_LEVEL_DEBUG  = 7  /**< Message at debug level. */
} VosLogLevel;

typedef enum
{
    VOS_LOG_DEST_STDERR  = 1,  /**< Message output to stderr. */
    VOS_LOG_DEST_SYSLOG  = 2,  /**< Message output to syslog. */
    VOS_LOG_DEST_TELNET  = 3,   /**< Message output to telnet clients. */
    VOS_LOG_DEST_LOGCAT  = 4
} VosLogDestination;

typedef enum
{
    VOS_LOG_LEVEL_PRINT  = 2,
    VOS_LOG_LEVEL_ERR    = 3, /**< Message at error level. */
    VOS_LOG_LEVEL_NOTICE = 5, /**< Message at notice level. */
    VOS_LOG_LEVEL_DEBUG  = 7  /**< Message at debug level. */
} VosLogLevel;



#define vosLog_debug(args...)  log_log(VOS_LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, args)



void log_log(VosLogLevel level, const char *func, UINT32 lineNum, const char *fmt, ...)
{
    va_list ap;
    char buf[102400] = {0};
    int len = 0, maxLen;
    char *logLevelStr = NULL;
    UINT32 headerMask = logHeaderMask;

    if (NULL == logShareMem)
    {
        //printf("%s:%d:[%s]logShareMem is null\n", __FUNCTION__, __LINE__, func);
        return;
    }

    vosLog_cache(func, lineNum);

    maxLen = sizeof(buf);

    if (level <= logShareMem->logLevel)
    {
        va_start(ap, fmt);

        switch (level)
        {
            case VOS_LOG_LEVEL_ERR:
                len = snprintf(buf, maxLen, LIGHT_RED_COLOR);
                break;
            case VOS_LOG_LEVEL_NOTICE:
                len = snprintf(buf, maxLen, LIGHT_GREEN_COLOR);
                break;
            case VOS_LOG_LEVEL_DEBUG:
                len = snprintf(buf, maxLen, BLUE_COLOR);
                break;
            case VOS_LOG_LEVEL_PRINT:
                headerMask = 0;
                break;
            default:
                break;
        }

        if (headerMask & VOS_LOG_HDRMASK_APPNAME)
        {
            if (NULL != sg_appName)
            {
                len += snprintf(&(buf[len]), maxLen - len, "%s:", sg_appName);
            }
            else
            {
                len += snprintf(&(buf[len]), maxLen - len, "unknown:");
            }
        }

        if ((headerMask & VOS_LOG_HDRMASK_LEVEL) && (len < maxLen))
        {
            /*
                   * Only log the severity level when going to stderr
                   * because syslog already logs the severity level for us.
                   */
            if (logShareMem->logDestination == VOS_LOG_DEST_STDERR)
            {
                switch (level)
                {
                    case VOS_LOG_LEVEL_ERR:
                        logLevelStr = "error";
                        break;
                    case VOS_LOG_LEVEL_NOTICE:
                        logLevelStr = "notice";
                        break;
                    case VOS_LOG_LEVEL_DEBUG:
                        logLevelStr = "debug";
                        break;
                    default:
                        logLevelStr = "invalid";
                        break;
                }
                len += snprintf(&(buf[len]), maxLen - len, "%s:", logLevelStr);
            }
        }

        /*
            * VosLog timestamp for both stderr and syslog because syslog's
            * timestamp is when the syslogd gets the log, not when it was
            * generated.
            */
        if ((headerMask & VOS_LOG_HDRMASK_TIMESTAMP) && (len < maxLen))
        {
            UtilTimestamp ts;

            utilTms_get(&ts);
            len += snprintf(&(buf[len]), maxLen - len, "%u.%03u:",
                            ts.sec % 1000, ts.nsec / NSECS_IN_MSEC);
        }

        if ((headerMask & VOS_LOG_HDRMASK_LOCATION) && (len < maxLen))
        {
            len += snprintf(&(buf[len]), maxLen - len, "%s:%u:", func, lineNum);
        }

        if (len < maxLen)
        {
            len += vsnprintf(&buf[len], maxLen - len, fmt, ap);
        }

        if (VOS_LOG_LEVEL_PRINT == level)
        {
            vosLog_printf(logShareMem->logLevel, logShareMem->logDestination, FALSE, buf);
        }
        else
        {
            len += snprintf(&(buf[len]), maxLen - len, DEFAULT_COLOR);
            vosLog_printf(logShareMem->logLevel, logShareMem->logDestination, TRUE, buf);
        }

        va_end(ap);
    }
}  /* End of log_log() */



int main()
{
    vosLog_debug("sdfsdf");
    return 0;
}
