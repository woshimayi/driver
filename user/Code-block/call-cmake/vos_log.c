/*
 * @*************************************: 
 * @FilePath: /call-cmake/vos_log.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-20 17:08:43
 * @LastEditors: dof
 * @LastEditTime: 2023-09-05 19:49:09
 * @Descripttion: 
 * @**************************************: 
 */
#include "vos_log.h"

void log_log(VosLogLevel level, const char *func, int linenum, const char *fmt, ...)
{
    char exec[256] = {0};
    char *tmp = NULL;
    va_list ap;
    va_start(ap, fmt);
    char buf[1024] = {0};
    vsprintf(buf, fmt, ap);
#ifdef __WIN32
    GetModuleFileName(0, exec, 1024); /*获取当前运行全路径*/
    tmp = strrchr(exec, '\\') + 1;
    printf("%s [%s %d]:", tmp, func, linenum);
#endif // __WIN32

#ifdef linux
    int rslt = readlink("/proc/self/exe", exec, 255);
    if (rslt < 0 || (rslt >= 255))
    {
        return;
    }
    exec[rslt] = '\0';
    printf("[%s %d]:", func, linenum);
#endif // __WIN32

    switch (level)
    {
    case VOS_LOG_LEVEL_PRINT:
        printf(buf);
        break;
    case VOS_LOG_LEVEL_ERR:
        RED printf(buf);
        WHITE
        break;
    case VOS_LOG_LEVEL_NOTICE:
        BLUE printf(buf);
        WHITE
        break;
    case VOS_LOG_LEVEL_DEBUG:
        YELLOW printf(buf);
        WHITE
        break;
    default:
        break;
    }
    printf("\n");
    va_end(ap);
}
