#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

// #undef system
// #define system  UTIL_DO_SYSTEM_ACTION
// #define UTIL_DO_SYSTEM_ACTION(fmt, ...)    UTIL_doSystemAction(__FUNCTION__, fmt, ##__VA_ARGS__)
// #define UTIL_COMM_SYSTEM_EX(pszCmd, pszResult, iCount)	UTIL_commSystemEx(pszCmd, pszResult, iCount)

// int UTIL_doSystemAction(const char *from, const char *fmt, ...);
// int UTIL_commSystemEx(const char *pszCmd, char *pszResult, int iCount);

// int UTIL_commSystemEx(const char *pszCmd, char *pszResult, int iCount)
// {

//     FILE   *stream;

//     //memset( pszResult, '\0', sizeof(pszResult) );
//     memset(pszResult, '\0', iCount);
//     stream = popen(pszCmd, "r");
//     if (!stream)
//     {
//         pclose(stream);
//         return 0;
//     }

//     iCount = fread(pszResult, sizeof(char), iCount, stream);
//     if (!(iCount))
//     {
//         pclose(stream);
//         return 0;
//     }

//     pclose(stream);
//     return 1;
// }

// int util_doSystem(const char *cmdStr)
// {
//     char cmdBuf[512] = {0};
//     char idBuf[512]  = {0};
//     char *cmd        = (char *)cmdStr;
//     char *idStr      = NULL;
//     char *p          = idBuf;
//     char found       = 0;

//     strncpy(idBuf, cmdStr, sizeof(idBuf) - 1);
//     while (' ' == *p)
//     {
//         p++;
//     }
//     idStr = p;

//     p = strchr(p, ' ');
//     if (NULL != p)
//     {
//         *p = '\0';
// snprintf(cmdBuf, sizeof(cmdBuf) - 1, __ROOTFS_PATH"etc/ts/%s", idStr);
// if (0 == access(cmdBuf, 0))
// {
//     found = 1;
// }
// else
// {
//     snprintf(cmdBuf, sizeof(cmdBuf) - 1, __ROOTFS_PATH"tmp/etc/ts/%s", idStr);
//     if (0 == access(cmdBuf, 0))
//     {
//         found = 1;
//     }
// }
// }

//     if (found)
//     {
//         snprintf(cmdBuf, sizeof(cmdBuf) - 1, "TS_SOCKET=/tmp/%s.ts_socket ts -n %s", idStr, cmdStr);
//         cmd = cmdBuf;
//     }

// #ifndef DESKTOP_LINUX
// #undef system
//     return system((const char *)cmd);
// #endif
// }



int UTIL_doSystemAction(const char *fun, int line, const char *fmt, ...)
{
    int ret = 0;
    int len = 0;
    char buf[256] = {0};
    char *cmd = NULL;
    char *allocBuf = NULL;
    va_list paraList;
    
    va_start(paraList, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    if (len < sizeof(buf))
    {
        cmd = buf;
    }
    else
    {
        len++;
        allocBuf = (char *)malloc(len);
        va_start(paraList, fmt);
        vsnprintf(allocBuf, len, fmt, paraList);
        va_end(paraList);
        
        cmd = allocBuf;
    }
    
    printf("[%s:%d] %s\n", fun, line, cmd);
    
    ret = system(cmd);
    
    free(allocBuf);
    return ret;
}
#define UTIL_DO_SYSTEM_ACTION(fmt, ...)    UTIL_doSystemAction(__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

int main(int argc, char const *argv[])
{
    UTIL_DO_SYSTEM_ACTION("ls -l %s", "./");
    return 0;
}
