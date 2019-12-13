#include "va_list.h"
#include <windows.h>

void simple_var_char(int i, ...)
{
    va_list arg_ptr;
    char *s = NULL;
    va_start(arg_ptr, i);
    s = va_arg(arg_ptr, char *);
    va_end(arg_ptr);
    printf("char*=%d %s\n", i, s);
    return;
}
void simple_var_char2(int i, ...)
{
    va_list arg_ptr;
    char *s = NULL;
    va_start(arg_ptr, i);
    s = va_arg(arg_ptr, char *);
    char *data = va_arg(arg_ptr, char *);
    va_end(arg_ptr);
    printf("char*=%d %s data=%s\n", i, s, data);
    return;
}

void simple_var_int(int i, ...)
{
    va_list arg_ptr;
    int j = 0;
    va_start(arg_ptr, i);
    j = va_arg(arg_ptr, int);
    va_end(arg_ptr);
    printf("int=%d %d\n", i, j);
    return;
}

void simple_var_int2(int i, ...)
{
    va_list arg_ptr;
    int j = 0;
    va_start(arg_ptr, i);
    j = va_arg(arg_ptr, int);
    int n = va_arg(arg_ptr, int);
    va_end(arg_ptr);
    printf("int=%d %d %d\n", i, j, n);
    return;
}

void err_sys(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    char    buf[1024] = {0};
    vsprintf(buf, fmt, ap);
    printf(buf);
    va_end(ap);
    exit(1);
}

void log_log(const char *func, int linenum, const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    char    buf[1024] = {0};
    vsprintf(buf, fmt, ap);
    printf("[%s %d]:", func, linenum);
    RED printf(buf);
    WHITE
    printf("\n");
    va_end(ap);
    exit(1);
}
