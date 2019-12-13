#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
//#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#define VOS_LOG_MAX_CACHE_LEN  (80)
#define VOS_LOG_MAX_CACHE_NUM  (32)

#ifndef PFM_SIM
    #define VOS_LOG_KEY_FILE_PREFIX  "/tmp/logKey"
#else
    #define VOS_LOG_KEY_FILE_PREFIX  "./logKey"
#endif

#define VOS_LAST_PTY_NAME_FILE "/tmp/last_pty"
#define VOS_LOOPBUF_DEV        "/dev/loopbuf"

typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef unsigned long  UINT64;

/*!\file vos_log.h
 * \brief Public header file for Broadcom DSL CPE Management System VosLogging API.
 * Applications which need to call VosLogging API functions must
 * include this file.
 *
 * Here is a general description of how to use this interface.
 *
 */

/*!\enum VosLogLevel
 * \brief VosLogging levels.
 * These correspond to LINUX log levels for convenience.  Other OS's
 * will have to map these values to their system.
 */
typedef enum
{
    VOS_LOG_LEVEL_PRINT  = 2,
    VOS_LOG_LEVEL_ERR    = 3, /**< Message at error level. */
    VOS_LOG_LEVEL_NOTICE = 5, /**< Message at notice level. */
    VOS_LOG_LEVEL_DEBUG  = 7  /**< Message at debug level. */
} VosLogLevel;


/*!\enum VosLogDestination
 * \brief identifiers for message logging destinations.
 */
typedef enum
{
    VOS_LOG_DEST_STDERR  = 1,  /**< Message output to stderr. */
    VOS_LOG_DEST_SYSLOG  = 2,  /**< Message output to syslog. */
    VOS_LOG_DEST_TELNET  = 3,   /**< Message output to telnet clients. */
    VOS_LOG_DEST_LOGCAT  = 4
} VosLogDestination;


typedef struct
{
    VosLogLevel logLevel;
    VosLogDestination logDestination;
    char cache[32][80];
    int location;
} VOS_LOG_SHARED_MEM_T;


/** Show application name in the log line. */
#define VOS_LOG_HDRMASK_APPNAME    0x0001

/** Show log level in the log line. */
#define VOS_LOG_HDRMASK_LEVEL      0x0002

/** Show timestamp in the log line. */
#define VOS_LOG_HDRMASK_TIMESTAMP  0x0004

/** Show location (function name and line number) level in the log line. */
#define VOS_LOG_HDRMASK_LOCATION   0x0008


/** Default log level is error messages only. */
#define DEFAULT_LOG_LEVEL        VOS_LOG_LEVEL_ERR

/** Default log destination is standard error */
#define DEFAULT_LOG_DESTINATION  VOS_LOG_DEST_STDERR

/** Default log header mask */
#define DEFAULT_LOG_HEADER_MASK (VOS_LOG_HDRMASK_APPNAME|VOS_LOG_HDRMASK_LEVEL|VOS_LOG_HDRMASK_TIMESTAMP|VOS_LOG_HDRMASK_LOCATION)


/** Maxmimu length of a single log line; messages longer than this are truncated. */
#define MAX_LOG_LINE_LENGTH      1024 * 10


/** Macros Definition.
 * Applications should use these macros for message logging, instead of
 * calling the log_log function directly.
 */
#if defined(VOS_LOG0)
    #define vosLog_print(args...)  log_log(VOS_LOG_LEVEL_PRINT, __FUNCTION__, __LINE__, args)
    #define vosLog_error(args...)
    #define vosLog_notice(args...)
    #define vosLog_debug(args...)

#elif defined(VOS_LOG2)
    #define vosLog_print(args...)  log_log(VOS_LOG_LEVEL_PRINT, __FUNCTION__, __LINE__, args)
    #define vosLog_error(args...)  log_log(VOS_LOG_LEVEL_ERR, __FUNCTION__, __LINE__, args)
    #define vosLog_notice(args...) log_log(VOS_LOG_LEVEL_NOTICE, __FUNCTION__, __LINE__, args)
    #define vosLog_debug(args...)

#else
    #define vosLog_print(args...)  log_log(VOS_LOG_LEVEL_PRINT, __FUNCTION__, __LINE__, args)
    #define vosLog_error(args...)  log_log(VOS_LOG_LEVEL_ERR, __FUNCTION__, __LINE__, args)
    #define vosLog_notice(args...) log_log(VOS_LOG_LEVEL_NOTICE, __FUNCTION__, __LINE__, args)
    #define vosLog_debug(args...)  log_log(VOS_LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, args)
#endif


/** Internal message log function; do not call this function directly.
 *
 * NOTE: Applications should NOT call this function directly from code.
 *       Use the macros defined in vos_log.h, i.e.
 *       vosLog_error, vosLog_notice, vosLog_debug.
 *
 * This function performs message logging based on two control
 * variables, "logLevel" and "logDestination".  These two control
 * variables are local to each process.  Each log message has an
 * associated severity level.  If the severity level of the message is
 * numerically lower than or equal to logLevel, the message will be logged to
 * either stderr or syslogd based on logDestination setting.
 * Otherwise, the message will not be logged.
 *
 * @param level (IN) The message severity level as defined in "sysvos_log.h".
 *                   The levels are, in order of decreasing importance:
 *                   VOS_LOG_EMERG (0)- system is unusable
 *                   VOS_LOG_ALERT (1)- action must be taken immediately
 *                   VOS_LOG_CRIT  (2)- critical conditions
 *                   VOS_LOG_ERR   (3)- error conditions
 *                   VOS_LOG_WARNING(4) - warning conditions
 *                   VOS_LOG_NOTICE(5)- normal, but significant, condition
 *                   VOS_LOG_INFO  (6)- informational message
 *                   VOS_LOG_DEBUG (7)- debug-level message
 * @param func (IN) Function name where the log message occured.
 * @param lineNum (IN) Line number where the log message occured.
 * @param fmt (IN) The message string.
 *
 */
void log_log(VosLogLevel level, const char *func, unsigned int lineNum, const char *fmt, ...);

/** Message log initialization.
 * This function initializes the message log utility.  The openlog
 * function is called to open a connection to syslogd for the
 * process.  The process name string identified by entityId will
 * be prepended to every message logged by the system logger syslogd.
 *
 * @param eid (IN) The entity ID of the calling process.
 */
void vosLog_init(int eid);

/** Message log cleanup.
 * This function performs all the necessary cleanup of the message
 * log utility. The closelog function is called to close the
 * descriptor being used to write to the system logger syslogd.
 *
 */
void vosLog_cleanup(void);

/** Set process message logging level.
 * This function sets the logging level of a process.
 *
 * @param level (IN) The logging level to set.
 */
void vosLog_setLevel(VosLogLevel level);

/** Get process message logging level.
 * This function gets the logging level of a process.
 *
 * @return The process message logging level.
 */
VosLogLevel vosLog_getLevel(void);

/** Set process message logging destination.
 * This function sets the logging destination of a process.
 *
 * @param dest (IN) The process message logging destination.
 */
void vosLog_setDestination(VosLogDestination dest);

/** Get process message logging destination.
 * This function gets the logging destination of a process.
 *
 * @return The process message logging destination.
 */
VosLogDestination vosLog_getDestination(void);

/** Set process message log header mask which determines which pieces of
 * info are included in each log line.
 *
 * @param mask (IN) Bitmask of VOS_LOG_HDRMASK_xxx
 */
void vosLog_setHeaderMask(unsigned int headerMask);

/** Get process message log header mask.
 *
 * @return The process message log header mask.
 */
unsigned vosLog_getHeaderMask(void);


void vosLog_printf(VosLogLevel logLevel, VosLogDestination logDestination, char newLine, const char *buf);


/** indicate first read */
#define BCM_SYSLOG_FIRST_READ           -2

/** indicates error */
#define BCM_SYSLOG_READ_BUFFER_ERROR    -1

/** indicates last line was read */
#define BCM_SYSLOG_READ_BUFFER_END      -3

/** max log buffer length */
#define BCM_SYSLOG_MAX_LINE_SIZE        255

/** redirect all printf to the tty
 *
 * @param ptr     the tty
 * @
 * @
 */
int vosLog_stdRedirect(const char *tty);
int vosLog_stdRevert(void);
int vosLog_saveLastPty(const char *pty_name);
int vosLog_stdToLastPty(void);



/** local definitions **/
#define LIGHT_RED_COLOR "\033[1;31m"
#define LIGHT_GREEN_COLOR "\033[1;32m"
#define BLUE_COLOR "\033[0;34m"
#define DEFAULT_COLOR "\033[0m"

/* default settings */

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

//static VOS_LOG_SHARED_MEM_T *logShareMem = NULL;

static int logHeaderMask; /**< Bitmask of which pieces of info we want
                              *   in the log line header.
                              */

//static char *sg_appName = NULL;


int g_vosStdin = -1;
int g_vosStdout = -1;
int g_vosStderr = -1;

void vosLog_printf(VosLogLevel logLevel, VosLogDestination logDestination, bool newLine, const char *buf)
{
    int logTelnetFd = -1;

    if (NULL == buf)
    {
        return;
    }
    if (logDestination == VOS_LOG_DEST_STDERR)
    {
        if (newLine)
        {
            fprintf(stderr, "%s\n", buf);
        }
        else
        {
            fprintf(stderr, "%s", buf);
        }
        fflush(stderr);
    }
    else
    {
    }
}


void vosLog_cache(const char *func, UINT32 lineNum)
{
    UINT32 location = 0;

    if (NULL == logShareMem)
    {
        return;
    }

    location = logShareMem->location;

    if (location < VOS_LOG_MAX_CACHE_NUM)
    {
        /* If use logShareMem->location++ directly,
         * oal_spawnProcess may cause memory write-overflow,
         * because child and parent process use the same logShareMem,
         * and logShareMem->location == VOS_LOG_MAX_CACHE_NUM at a time */

        UTIL_SNPRINTF(logShareMem->cache[location++],
                      VOS_LOG_MAX_CACHE_LEN, "<%s:%d>", func, lineNum);
    }

    logShareMem->location = (location % VOS_LOG_MAX_CACHE_NUM);
}


void log_log(VosLogLevel level, const char *func, UINT32 lineNum, const char *fmt, ...)
{
    va_list ap;
    char buf[MAX_LOG_LINE_LENGTH] = {0};
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


void vosLog_init(VosEntityId eid)
{
    SINT32 shmId = 0;
    int shmFlg = 0;
    char keyfile[BUFLEN_256] = {0};
    key_t key;
    FILE *fp = NULL;
    char appName[BUFLEN_64] = {0};

    sg_appName = VOS_STRDUP(appName[0] == '\0' ? "unknown" : appName);

    UTIL_SNPRINTF(keyfile, sizeof(keyfile), VOS_LOG_KEY_FILE_PREFIX"%s", appName);
    fp = fopen(keyfile, "a");
    if (fp != NULL)
    {
        fclose(fp);
    }
    else
    {
        printf("%s:%d:fopen %s error, %s\n", __FUNCTION__, __LINE__, keyfile, strerror(errno));
        return;
    }

    key = ftok(keyfile, 'a');

    shmId = shmget(key, 0, 0);
    if (-1 == shmId)
    {
        shmFlg = 0666;
        shmFlg |= (IPC_CREAT | IPC_EXCL);

        shmId = shmget(key, sizeof(VOS_LOG_SHARED_MEM_T), shmFlg);
        if (-1 == shmId)
        {
            printf("%s:%d:eid=%s, shmget fail\n", __FUNCTION__, __LINE__, appName);
            return;
        }
    }

    logShareMem = (VOS_LOG_SHARED_MEM_T *)shmat(shmId, NULL, 0);
    if ((VOS_LOG_SHARED_MEM_T *)(-1) == logShareMem)
    {
        printf("%s:%d:eid=%s, shmat fail\n", __FUNCTION__, __LINE__, appName);
        logShareMem = NULL;
        return;
    }

    logShareMem->logLevel = DEFAULT_LOG_LEVEL;
    logShareMem->logDestination = DEFAULT_LOG_DESTINATION;

    logHeaderMask  = DEFAULT_LOG_HEADER_MASK;
    gEid = (VosEntityId)eid;

    oalLog_init();

    return;
}  /* End of vosLog_init() */


void vosLog_cleanup(void)
{
    oalLog_cleanup();
    VOS_FREE(sg_appName);
    return;

}  /* End of vosLog_cleanup() */


void vosLog_setLevel(VosLogLevel level)
{
    if (logShareMem != NULL)
    {
        logShareMem->logLevel = level;
    }
}


VosLogLevel vosLog_getLevel(void)
{
    if (logShareMem != NULL)
    {
        return logShareMem->logLevel;
    }

    return DEFAULT_LOG_LEVEL;
}


void vosLog_setDestination(VosLogDestination dest)
{
    if (logShareMem != NULL)
    {
        logShareMem->logDestination = dest;
    }
}


VosLogDestination vosLog_getDestination(void)
{
    if (logShareMem != NULL)
    {
        return logShareMem->logDestination;
    }

    return DEFAULT_LOG_DESTINATION;
}


void vosLog_setHeaderMask(UINT32 headerMask)
{
    logHeaderMask = headerMask;
    return;
}


UINT32 vosLog_getHeaderMask(void)
{
    return logHeaderMask;
}


VOS_RET_E vosLog_security(VosLogSecurityLogIDs id, VosLogSecurityLogData *pdata, const char *fmt, ...)
{
    return (VOS_RET_SUCCESS);
}


VOS_RET_E vosSyslog_info(const char *fmt, ...)
{
    int len = 0;
    va_list ap;
    char buf[MAX_LOG_LINE_LENGTH] = {0};

    va_start(ap, fmt);

    len = snprintf(buf, MAX_LOG_LINE_LENGTH, "000000 ");
    len += vsnprintf(&buf[len], MAX_LOG_LINE_LENGTH - len, fmt, ap);
    va_end(ap);

    syslog(LOG_INFO, buf);
    return (VOS_RET_SUCCESS);
}

VOS_RET_E vosLog_stdRedirect(const char *tty)
{
    int fd = -1;

    if (NULL == tty || '\0' == tty[0])
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    fd = open(tty, O_RDWR);
    if (fd < 0)
    {
        return VOS_RET_RESOURCE_EXCEEDED;
    }
    if (g_vosStdin < 0)
    {
        g_vosStdin = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
    }
    if (g_vosStdout < 0)
    {
        g_vosStdout = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
    }
    if (g_vosStderr < 0)
    {
        g_vosStderr = dup(STDERR_FILENO);
        dup2(fd, STDERR_FILENO);
    }

    close(fd);

    return VOS_RET_SUCCESS;
}

VOS_RET_E vosLog_stdRevert(void)
{
    if (g_vosStdin >= 0)
    {
        dup2(g_vosStdin, STDIN_FILENO);
        g_vosStdin = -1;
    }
    if (g_vosStdout >= 0)
    {
        dup2(g_vosStdout, STDOUT_FILENO);
        g_vosStdout = -1;
    }
    if (g_vosStderr >= 0)
    {
        dup2(g_vosStderr, STDERR_FILENO);
        g_vosStderr = -1;
    }

    return VOS_RET_SUCCESS;
}

VOS_RET_E vosLog_saveLastPty(const char *pty_name)
{
    if (NULL == pty_name || '\0' == pty_name[0])
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    remove(VOS_LAST_PTY_NAME_FILE);
    symlink(pty_name, VOS_LAST_PTY_NAME_FILE);

    return VOS_RET_SUCCESS;
}

VOS_RET_E vosLog_stdToLastPty(void)
{
    return vosLog_stdRedirect(VOS_LAST_PTY_NAME_FILE);
}


