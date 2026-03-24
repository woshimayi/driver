#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>

// 函数声明
void *my_malloc(size_t size, const char *file, int line);
void my_free(void *ptr, const char *file, int line);
void *my_calloc(size_t nmemb, size_t size, const char *file, int line);
void *my_realloc(void *ptr, size_t size, const char *file, int line);
void mem_mgr_init(void);
void mem_leak_check(void);
void mem_report(void);

// 内存管理相关函数声明
void *sys_malloc(size_t size);
void sys_free(void *ptr);

// 宏定义 - 只保留一份
#undef malloc
#undef free
#undef calloc
#undef realloc

#define malloc(size) my_malloc(size, __FILE__, __LINE__)
#define free(ptr) my_free(ptr, __FILE__, __LINE__)
#define calloc(nmemb, size) my_calloc(nmemb, size, __FILE__, __LINE__)
#define realloc(ptr, size) my_realloc(ptr, size, __FILE__, __LINE__)

typedef struct MemRecord
{
    void *ptr;         // 分配的内存地址（关键索引）
    size_t size;       // 分配大小（字节）
    const char *file;  // 分配所在文件
    int line;          // 分配所在行号
    const char *func;  // 分配所在函数
    uint32_t seq;      // 分配顺序（第N次分配）
    time_t alloc_time; // 分配时间（秒级）
    int used;          // 1=已分配未释放, 0=已释放/未使用
} MemRecord;

#define MEM_RECORD_MAX 1024

// 全局内存管理器（单例）
typedef struct MemManager
{
    MemRecord records[MEM_RECORD_MAX]; // 静态数组存储记录
    uint32_t alloc_seq;                // 分配顺序计数器
    size_t total_alloc;                // 累计分配大小
    size_t total_free;                 // 累计释放大小
    pthread_mutex_t mutex;             // 线程锁
    int initialized;                   // 初始化标志
} MemManager;

MemRecord *alloc_free_slot(void);

static MemManager g_mem_mgr; // 全局内存管理器实例

// 系统内存函数包装
void *sys_malloc(size_t size)
{
    return malloc(size);
}

void sys_free(void *ptr)
{
    free(ptr);
}

// 初始化内存管理器
void mem_mgr_init(void)
{
    if (g_mem_mgr.initialized)
        return;

    memset(&g_mem_mgr, 0, sizeof(g_mem_mgr));
    pthread_mutex_init(&g_mem_mgr.mutex, NULL);
    g_mem_mgr.initialized = 1;

    // 初始化所有记录槽位为未使用
    for (int i = 0; i < MEM_RECORD_MAX; i++)
    {
        g_mem_mgr.records[i].used = 0;
        g_mem_mgr.records[i].ptr = NULL;
    }
}

// 分配一个空闲的记录槽位
MemRecord *alloc_free_slot(void)
{
    pthread_mutex_lock(&g_mem_mgr.mutex);
    for (int i = 0; i < MEM_RECORD_MAX; i++)
    {
        if (!g_mem_mgr.records[i].used)
        {
            pthread_mutex_unlock(&g_mem_mgr.mutex);
            return &g_mem_mgr.records[i];
        }
    }
    pthread_mutex_unlock(&g_mem_mgr.mutex);
    fprintf(stderr, "错误: 内存记录槽位已满！\n");
    return NULL;
}

// 根据指针查找记录
MemRecord *find_record_by_ptr(void *ptr)
{
    pthread_mutex_lock(&g_mem_mgr.mutex);
    for (int i = 0; i < MEM_RECORD_MAX; i++)
    {
        if (g_mem_mgr.records[i].used && g_mem_mgr.records[i].ptr == ptr)
        {
            pthread_mutex_unlock(&g_mem_mgr.mutex);
            return &g_mem_mgr.records[i];
        }
    }
    pthread_mutex_unlock(&g_mem_mgr.mutex);
    return NULL;
}

//--------------------------自定义内存函数实现--------------------------//

// 自定义malloc: 记录分配信息
void *my_malloc(size_t size, const char *file, int line)
{
    mem_mgr_init(); // 确保管理器已初始化

    // 调用系统malloc分配原始内存
    void *ptr = sys_malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "malloc失败: 大小%zu字节,文件%s, 行号%d\n", size, file, line);
        return NULL;
    }

    // 分配记录槽位并填写信息
    MemRecord *slot = alloc_free_slot();
    if (slot == NULL)
    {
        sys_free(ptr); // 无槽位时释放内存, 避免泄漏
        return NULL;
    }

    pthread_mutex_lock(&g_mem_mgr.mutex);
    slot->ptr = ptr;
    slot->size = size;
    slot->file = file;
    slot->line = line;
    slot->func = ""; // C语言没有标准方法获取函数名, 可以留空
    slot->seq = ++g_mem_mgr.alloc_seq;
    slot->alloc_time = time(NULL);
    slot->used = 1;
    g_mem_mgr.total_alloc += size;
    pthread_mutex_unlock(&g_mem_mgr.mutex);

    return ptr;
}

// 自定义free: 记录释放信息
void my_free(void *ptr, const char *file, int line)
{
    mem_mgr_init();

    if (ptr == NULL)
    {
        fprintf(stderr, "警告: 尝试释放NULL指针,文件%s, 行号%d\n", file, line);
        return;
    }

    MemRecord *record = find_record_by_ptr(ptr);
    if (record == NULL)
    {
        fprintf(stderr, "错误: 尝试释放未分配的内存 %p, 文件%s, 行号%d\n", ptr, file, line);
        return;
    }

    if (!record->used)
    {
        fprintf(stderr, "错误: 重复释放内存 %p, 文件%s, 行号%d\n", ptr, file, line);
        return;
    }

    pthread_mutex_lock(&g_mem_mgr.mutex);
    record->used = 0;
    g_mem_mgr.total_free += record->size;
    pthread_mutex_unlock(&g_mem_mgr.mutex);

    sys_free(ptr);
}

// 自定义calloc
void *my_calloc(size_t nmemb, size_t size, const char *file, int line)
{
    void *ptr = my_malloc(nmemb * size, file, line);
    if (ptr != NULL)
    {
        memset(ptr, 0, nmemb * size);
    }
    return ptr;
}

// 自定义realloc
void *my_realloc(void *ptr, size_t size, const char *file, int line)
{
    if (size == 0 && ptr != NULL)
    {
        my_free(ptr, file, line);
        return NULL;
    }

    if (ptr == NULL)
    {
        return my_malloc(size, file, line);
    }

    MemRecord *record = find_record_by_ptr(ptr);
    if (record == NULL)
    {
        fprintf(stderr, "错误: 尝试重新分配未分配的内存 %p, 文件%s, 行号%d\n", ptr, file, line);
        return NULL;
    }

    void *new_ptr = sys_malloc(size);
    if (new_ptr == NULL)
    {
        fprintf(stderr, "realloc失败: 新大小%zu字节, 文件%s, 行号%d\n", size, file, line);
        return NULL;
    }

    // 复制数据
    size_t copy_size = (size < record->size) ? size : record->size;
    memcpy(new_ptr, ptr, copy_size);

    // 释放旧内存并更新记录
    pthread_mutex_lock(&g_mem_mgr.mutex);
    record->ptr = new_ptr;
    record->size = size;
    record->file = file;
    record->line = line;
    record->alloc_time = time(NULL);
    g_mem_mgr.total_alloc += (size - record->size); // 更新总分配大小
    pthread_mutex_unlock(&g_mem_mgr.mutex);

    sys_free(ptr);
    return new_ptr;
}

// 内存泄漏检查
void mem_leak_check(void)
{
    mem_mgr_init();

    int leak_count = 0;
    size_t leak_size = 0;

    pthread_mutex_lock(&g_mem_mgr.mutex);
    printf("\n=== 内存泄漏检查报告 ===\n");

    for (int i = 0; i < MEM_RECORD_MAX; i++)
    {
        if (g_mem_mgr.records[i].used)
        {
            leak_count++;
            leak_size += g_mem_mgr.records[i].size;
            printf("泄漏 #%d:\n", leak_count);
            printf("  地址: %p\n", g_mem_mgr.records[i].ptr);
            printf("  大小: %zu 字节\n", g_mem_mgr.records[i].size);
            printf("  位置: %s:%d\n", g_mem_mgr.records[i].file, g_mem_mgr.records[i].line);
            printf("  分配顺序: %u\n", g_mem_mgr.records[i].seq);

            // 计算泄漏时间
            time_t now = time(NULL);
            double leak_seconds = difftime(now, g_mem_mgr.records[i].alloc_time);
            printf("  泄漏时长: %.0f 秒\n", leak_seconds);
        }
    }

    printf("\n总结:\n");
    printf("  总分配: %zu 字节\n", g_mem_mgr.total_alloc);
    printf("  总释放: %zu 字节\n", g_mem_mgr.total_free);
    printf("  净分配: %zu 字节\n", g_mem_mgr.total_alloc - g_mem_mgr.total_free);
    printf("  泄漏块数: %d\n", leak_count);
    printf("  泄漏总大小: %zu 字节\n", leak_size);

    pthread_mutex_unlock(&g_mem_mgr.mutex);

    if (leak_count == 0)
    {
        printf("恭喜！没有发现内存泄漏。\n");
    }
}

// 测试函数: 正常的内存分配与释放
void test_normal_allocation(void)
{
    printf("=== 测试1: 正常的内存分配与释放 ===\n");

    // 测试malloc和free
    int *arr = (int *)malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i * i;
    }
    printf("数组内容: ");
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
    free(arr);

    // 测试calloc
    char *str = (char *)calloc(20, sizeof(char));
    strcpy(str, "Hello, Memory!");
    printf("字符串: %s\n", str);
    free(str);

    // 测试realloc
    int *dynamic = (int *)malloc(5 * sizeof(int));
    for (int i = 0; i < 5; i++)
    {
        dynamic[i] = i;
    }
    dynamic = (int *)realloc(dynamic, 10 * sizeof(int));
    for (int i = 5; i < 10; i++)
    {
        dynamic[i] = i * 10;
    }
    printf("重新分配后的数组: ");
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", dynamic[i]);
    }
    printf("\n");
    free(dynamic);

    printf("测试1完成, 应无内存泄漏。\n\n");
}

// 测试函数: 故意制造内存泄漏
void test_memory_leak(void)
{
    printf("=== 测试2: 故意制造内存泄漏 ===\n");

    // 这些内存不会被释放
    int *leak1 = (int *)malloc(100 * sizeof(int));
    char *leak2 = (char *)malloc(256);
    void *leak3 = calloc(10, 50);

    // 使用这些指针避免编译器警告
    if (leak1)
        leak1[0] = 1;
    if (leak2)
        leak2[0] = 'A';
    if (leak3)
        memset(leak3, 0, 1);

    // 测试重复释放（应该会报错）
    printf("测试重复释放...\n");
    free(leak1); // 正常释放
    free(leak1); // 重复释放, 应该报错

    // 测试释放未分配的内存
    printf("测试释放未分配的内存...\n");
    int *fake_ptr = (int *)0x12345678;
    free(fake_ptr); // 应该报错

    printf("测试2完成, 应有2处内存泄漏（leak2和leak3）。\n\n");
}

// 测试函数: 测试异常情况
void test_error_cases(void)
{
    printf("=== 测试3: 测试异常情况 ===\n");

    // 测试释放NULL指针
    printf("测试释放NULL指针...\n");
    free(NULL); // 应该只给出警告, 不崩溃

    // 测试realloc异常情况
    printf("测试realloc异常...\n");
    int *ptr = (int *)malloc(10 * sizeof(int));
    int *new_ptr = (int *)realloc(ptr, 0); // realloc(ptr,0)相当于free(ptr)
    if (new_ptr == NULL)
    {
        printf("realloc(ptr,0)成功, ptr已被释放\n");
    }

    // 测试realloc未分配的内存
    int *bad_ptr = (int *)0x87654321;
    new_ptr = (int *)realloc(bad_ptr, 100); // 应该报错
    if (new_ptr == NULL)
    {
        printf("realloc未分配的内存失败（正确行为）\n");
    }

    printf("测试3完成。\n\n");
}

int main(void)
{
    printf("开始内存跟踪测试...\n\n");

    // 初始化内存管理器
    mem_mgr_init();

    test_normal_allocation();
    test_error_cases();
    test_memory_leak();

    // 手动检查泄漏
    printf("=== 手动检查内存泄漏 ===\n");
    mem_leak_check();

    printf("程序即将退出, 退出时会自动显示泄漏报告...\n");
    return 0;
}