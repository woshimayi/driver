#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 获取内存使用率
double get_memory_usage()
{
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file)
    {
        perror("Failed to open /proc/meminfo");
        return -1.0;
    }

    unsigned long total_mem = 0;
    unsigned long free_mem = 0;
    unsigned long available_mem = 0;
    unsigned long buffers = 0;
    unsigned long cached = 0;
    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "MemTotal:", 9) == 0)
        {
            sscanf(line + 9, "%lu", &total_mem);
        }
        else if (strncmp(line, "MemFree:", 8) == 0)
        {
            sscanf(line + 8, "%lu", &free_mem);
        }
        else if (strncmp(line, "MemAvailable:", 13) == 0)
        {
            sscanf(line + 13, "%lu", &available_mem);
        }
        else if (strncmp(line, "Buffers:", 8) == 0)
        {
            sscanf(line + 8, "%lu", &buffers);
        }
        else if (strncmp(line, "Cached:", 7) == 0)
        {
            sscanf(line + 7, "%lu", &cached);
        }
    }
    fclose(file);

    if (total_mem == 0)
    {
        return -1.0;
    }

    // 计算已使用内存（更准确的方式）
    unsigned long used_mem = total_mem - free_mem - buffers - cached;

    // 如果有可用内存信息，使用更准确的计算
    if (available_mem > 0)
    {
        used_mem = total_mem - available_mem;
    }

    return (double)used_mem / total_mem * 100.0;
}

#include <sys/time.h>
#include <sys/resource.h>

typedef struct
{
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
} CPUStats;

// 读取 CPU 统计信息
int read_cpu_stats(CPUStats *stats)
{
    FILE *file = fopen("/proc/stat", "r");
    if (!file)
    {
        perror("Failed to open /proc/stat");
        return -1;
    }

    char line[256];
    if (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "cpu ", 4) == 0)
        {
            sscanf(line + 5, "%lu %lu %lu %lu %lu %lu %lu",
                   &stats->user, &stats->nice, &stats->system, &stats->idle,
                   &stats->iowait, &stats->irq, &stats->softirq);
        }
    }

    fclose(file);
    return 0;
}

// 获取 CPU 使用率
double get_cpu_usage()
{
    CPUStats stats1, stats2;

    // 第一次读取
    if (read_cpu_stats(&stats1) != 0)
    {
        return -1.0;
    }

    // 等待一段时间
    sleep(1);

    // 第二次读取
    if (read_cpu_stats(&stats2) != 0)
    {
        return -1.0;
    }

    // 计算两次的差值
    unsigned long total1 = stats1.user + stats1.nice + stats1.system +
                           stats1.idle + stats1.iowait + stats1.irq + stats1.softirq;
    unsigned long total2 = stats2.user + stats2.nice + stats2.system +
                           stats2.idle + stats2.iowait + stats2.irq + stats2.softirq;

    unsigned long idle1 = stats1.idle;
    unsigned long idle2 = stats2.idle;

    unsigned long total_diff = total2 - total1;
    unsigned long idle_diff = idle2 - idle1;

    if (total_diff == 0)
    {
        return 0.0;
    }

    // 计算 CPU 使用率
    double usage = 100.0 * (1.0 - (double)idle_diff / total_diff);
    return usage;
}

// 上面定义的所有函数...

int main()
{
    printf("系统资源监控\n");
    printf("====================\n");

    while (1)
    {
        // 获取内存使用率
        double memory_usage = get_memory_usage();
        if (memory_usage >= 0)
        {
            printf("内存使用率: %.2f%%\n", memory_usage);
        }
        else
        {
            printf("获取内存使用率失败\n");
        }

        // 获取 CPU 使用率
        double cpu_usage = get_cpu_usage();
        if (cpu_usage >= 0)
        {
            printf("CPU 使用率: %.2f%%\n", cpu_usage);
        }
        else
        {
            printf("获取 CPU 使用率失败\n");
        }

        printf("--------------------\n");
        sleep(2); // 每2秒更新一次
    }

    return 0;
}