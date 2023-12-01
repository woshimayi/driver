/*
 * @*************************************:
 * @FilePath: /user/C/string/cpu_mem_percent.c
 * @version:
 * @Author: dof
 * @Date: 2023-11-30 15:14:33
 * @LastEditors: dof
 * @LastEditTime: 2023-11-30 15:31:16
 * @Descripttion: cpu 内存占用率
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 函数：获取CPU利用率
float get_cpu_usage()
{
	FILE *file = fopen("/proc/stat", "r");
	if (!file)
	{
		perror("Error opening file");
		return -1;
	}

	char buffer[1024];
	fgets(buffer, sizeof(buffer), file); // 读取第一行

	fclose(file);

	char cpu[5];
	unsigned long user, nice, system, idle, iowait, irq, softirq;

	sscanf(buffer, "%s %lu %lu %lu %lu %lu %lu %lu", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq);

	unsigned long total_non_idle = user + nice + system + irq + softirq;
	unsigned long total = total_non_idle + idle;

	return ((float)total_non_idle / total) * 100;
}

// 函数：获取内存利用率
float get_memory_usage()
{
	FILE *file = fopen("/proc/meminfo", "r");
	if (!file)
	{
		perror("Error opening file");
		return -1;
	}

	char buffer[1024];
	char mem_total_label[32];
	unsigned long mem_total;

	// 读取总内存
	fgets(buffer, sizeof(buffer), file);
	sscanf(buffer, "%s %lu", mem_total_label, &mem_total);

	unsigned long mem_free, mem_available;
	// 读取空闲内存和可用内存
	for (int i = 0; i < 2; i++)
	{
		fgets(buffer, sizeof(buffer), file);
	}
	sscanf(buffer, "%s %lu", mem_total_label, &mem_free);

	fgets(buffer, sizeof(buffer), file);
	sscanf(buffer, "%s %lu", mem_total_label, &mem_available);

	fclose(file);

	float used_memory = (float)(mem_total - mem_free - mem_available) / mem_total * 100;

	return used_memory;
}

int main()
{
	printf("CPU利用率: %.2f%%\n", get_cpu_usage());
	printf("内存利用率: %.2f%%\n", get_memory_usage());

	printf("CPU利用率: %d\n", get_cpu_usage() < 80);
	printf("内存利用率: %d\n", get_memory_usage() < 80);
	return 0;
}
