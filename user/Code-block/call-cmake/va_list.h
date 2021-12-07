#ifndef _VA_LIST_H_
#define _VA_LIST_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

typedef enum
{
    VOS_LOG_LEVEL_PRINT  = 2,
    VOS_LOG_LEVEL_ERR    = 3,
    VOS_LOG_LEVEL_NOTICE = 5,
    VOS_LOG_LEVEL_DEBUG  = 7
} VosLogLevel;

void simple_var_char(int i, ...);

void simple_var_char2(int i, ...);

void simple_var_int(int i, ...);

void simple_var_int2(int i, ...);

void err_sys(const char *fmt, ...);

#ifdef __WIN32
    #include <windows.h>
    // param: FOREGROUND_INTENSITY º”¡¡
    #define WHITE   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);
    #define BLUE    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE|FOREGROUND_INTENSITY);
    #define RED     SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_INTENSITY);
    #define GREEN   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY);
    #define YELLOW  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif // __WINDOWS__

#ifdef linux
    #define NONE          "\033[m"
    #define RED           "\033[0;32;31m"
    #define LIGHT_RED     "\033[1;31m"
    #define GREEN         "\033[0;32;32m"
    #define LIGHT_GREEN   "\033[1;32m"
    #define BLUE          "\033[0;32;34m"
    #define LIGHT_BLUE    "\033[1;34m"
    #define DARY_GRAY     "\033[1;30m"
    #define CYAN          "\033[0;36m"
    #define LIGHT_CYAN    "\033[1;36m"
    #define PURPLE        "\033[0;35m"
    #define LIGHT_PURPLE "\033[1;35m"
    #define BROWN         "\033[0;33m"
    #define YELLOW        "\033[1;33m"
    #define LIGHT_GRAY    "\033[0;37m"
    #define WHITE         "\033[1;37m"
#endif // __LINUX__


#define VOS_LOG log_level

//#ifdef (VOS_LOG == )
#define vos_print(args...) log_log(VOS_LOG_LEVEL_PRINT,  __FUNCTION__, __LINE__, args)
#define vos_error(args...) log_log(VOS_LOG_LEVEL_ERR,    __FUNCTION__, __LINE__, args)
#define vos_notic(args...) log_log(VOS_LOG_LEVEL_NOTICE, __FUNCTION__, __LINE__, args)
#define vos_debug(args...) log_log(VOS_LOG_LEVEL_DEBUG,  __FUNCTION__, __LINE__, args)

//#elif
//#endif // VOS_LOG


void log_log(VosLogLevel level, const char *func, int linenum, const char *fmt, ...);

#endif // _VA_LIST_H_

