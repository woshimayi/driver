/*
 * @*************************************: 
 * @FilePath: /user/C/string/xl2tp_log.c
 * @version: 
 * @Author: dof
 * @Date: 2023-12-06 19:14:47
 * @LastEditors: dof
 * @LastEditTime: 2023-12-07 10:30:23
 * @Descripttion: 
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void l2tp_log_1 (char * func, int line, int level, const char *fmt, ...)
{
   char buf[2048];
   va_list args;
   va_start (args, fmt);
   vsnprintf (buf, sizeof (buf), fmt, args);
   va_end (args);
   
	fprintf(stderr, "xl2tpd[%s:%d]: %s", func, line,  buf);
}



// #define l2tp_log(a, fmt, arg...) fprintf(stderr, "%d:[%s:%d]  " fmt "\n", a, __FUNCTION__, __LINE__,##arg)
#define l2tp_log(a, args...) l2tp_log_1(__FUNCTION__, __LINE__, a, args)


int main(int argc, char const *argv[])
{
	l2tp_log(2, "recv packet from\n");
	l2tp_log(2, " %s recv packet from %s, size = %d:%d\n", __FUNCTION__, "ssss", 4, 345);

	return 0;
}
