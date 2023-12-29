/*
 * @*************************************: 
 * @FilePath: /user/C/string/xl2tp_log.c
 * @version: 
 * @Author: dof
 * @Date: 2023-12-06 19:14:47
 * @LastEditors: dof
 * @LastEditTime: 2023-12-21 11:39:41
 * @Descripttion: 
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define l2tp_log(a, fmt, arg...) printf("[VPN_DBG:%s(%d)]"fmt, __func__,__LINE__,##arg)


int main(int argc, char const *argv[])
{
	l2tp_log(2, "recv packet from\n");
	l2tp_log(2, " %s recv packet from %s, size = %d:%d\n", __FUNCTION__, "ssss", 4, 345);

	return 0;
}
