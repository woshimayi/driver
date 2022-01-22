/*
 * @*************************************:
 * @FilePath: /user/C/string/systeminfo_.c
 * @version:
 * @Author: dof
 * @Date: 2022-01-20 18:42:48
 * @LastEditors: dof
 * @LastEditTime: 2022-01-20 18:44:49
 * @Descripttion:  获取系统信息
 * @**************************************:
 */
#include <stdio.h>

#include <linux/unistd.h> /* 包含调用 _syscallX 宏等相关信息*/

#include <linux/kernel.h> /* 包含sysinfo结构体信息*/

// _syscall1(int, sysinfo, struct sysinfo *, info);

// using namespace std;

int main(int argc, char *agrv[])

{

	struct sysinfo s_info;

	int error;

	error = sysinfo(&s_info);

	printf("\n\ncode error=%d\n", error);

	printf("Uptime = %ds\nLoad: 1 min%d / 5 min %d / 15 min %d\n"

		   "RAM: total %d / free %d /shared%d\n"

		   "Memory in buffers = %d\nSwap:total%d/free%d\n"

		   "Number of processes = %d\n",

		   s_info.uptime, s_info.loads[0],

		   s_info.loads[1], s_info.loads[2],

		   s_info.totalram, s_info.freeram,

		   s_info.totalswap, s_info.freeswap,

		   s_info.procs);

	return 0;
}
