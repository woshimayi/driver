/*
 * @*************************************:
 * @FilePath: /user/C/string/memory_percent.c
 * @version:
 * @Author: dof
 * @Date: 2023-10-09 16:42:45
 * @LastEditors: dof
 * @LastEditTime: 2023-10-09 17:22:33
 * @Descripttion: linux 内存 cpu 占用率
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int memsize(int *pui_total, int *pui_free)
{
#define PROC_MEMINFO "/proc/meminfo"
	FILE *pf = NULL;
	char readBuf[256];
	char *pos, *pRem;
	int totoMem, availMem;

	pf = fopen(PROC_MEMINFO, "r");
	if (NULL == pf)
	{
		return -1;
	}
	if (0 > fread(readBuf, 1, sizeof(readBuf) - 1, pf)) // lint !e568 !e685
	{
		fclose(pf);
		return -1;
	}
	fclose(pf);

	pos = strstr(readBuf, "MemTotal:");
	if (NULL == pos)
	{
		return -1;
	}
	pos += strlen("MemTotal:");
	totoMem = strtol(pos, &pRem, 0);
	if (0 == totoMem)
	{
		return -1;
	}
	pos = strstr(pos, "MemAvailable:");
	if (NULL == pos)
	{
		return -1;
	}
	pos += strlen("MemAvailable:");
	availMem = strtol(pos, &pRem, 0);

	*pui_total = totoMem;
	*pui_free = availMem;
	return 0;
}

int get_MemoryRate(unsigned int *memusage)
{
	unsigned int ui_total = 0, ui_free = 0;
	memsize(&ui_total, &ui_free);
	printf("ui_total = %d, ui_free = %d\n", ui_total, ui_free);

	if (ui_total > (512 << 10))
	{
		ui_total = (1024 << 10);
	}
	else if (ui_total > (256 << 10))
	{
		ui_total = (512 << 10);
	}
	else if (ui_total > (128 << 10))
	{
		ui_total = (256 << 10);
	}
	else
	{
		ui_total = (128 << 10);
	}

	printf("memory percent = %d\n", (ui_total - ui_free) * 100 / ui_total);
	*memusage = (ui_total - ui_free) * 100 / ui_total;
	return 0;
}

int get_cpuusage(int *pi_total, int *pi_idle)
{
#define PROC_STAT "/proc/stat"
	int stat[8]; /* user , nice , sys, idle, iowait, irq, sirq, steal, guest */
	FILE *fp = NULL;
	if (NULL == (fp = fopen(PROC_STAT, "r")))
	{
		return -1;
	}
	fscanf(fp, "cpu %d %d %d %d %d %d %d %d",
		   &stat[0], &stat[1], &stat[2], &stat[3], &stat[4], &stat[5], &stat[6], &stat[7]);
	fclose(fp);

	*pi_total = stat[0] + stat[1] + stat[2] + stat[3] + stat[4] + stat[5] + stat[6];
	*pi_idle = stat[3];

	return 0;
}

int get_CPURate(unsigned int *cpuusage)
{
	int used = 0;
	int usage = 0;
	int crtTotal = 0;
	int crtIdel = 0;
	static int lastTotal = 0;
	static int lastIdel = 0;
	static int lastUsage = 1;
#define HI_CPUUSAGE_CAL_PERIOD (100 * 3) // jiffies tick

	get_cpuusage(&crtTotal, &crtIdel);
	if ((crtTotal - lastTotal) >= HI_CPUUSAGE_CAL_PERIOD)
	{
		used = (crtTotal - lastTotal) - (crtIdel - lastIdel);
		usage = (used * 100) / (crtTotal - lastTotal);
		lastIdel = crtIdel;
		lastTotal = crtTotal;
		lastUsage = usage;
	}
	else
	{
		usage = lastUsage;
	}

	*cpuusage = usage;
	return 0;
}

int main(int argc, char const *argv[])
{
	unsigned int cpu_rate = 0, memory_rate = 0;
	get_CPURate(&cpu_rate);
	get_MemoryRate(&memory_rate);
	printf(" cpu_rate = %d, memory_rate = %d\n", cpu_rate, memory_rate);
	return 0;
}
