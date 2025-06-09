/*
 * @*************************************:
 * @FilePath     : /user/C/elf_test/LD_PRELOAD_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-04-28 10:14:52
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-09 19:20:15
 * @Descripttion :
 * @compile      :
 *
 * 看到了吗？我们只用了十几行代码，就实现了一个能够监控任何程序内存分配的工具！
 *     这个例子的工作原理很简单：
 *          1:定义一个与系统函数同名的malloc
 *          2:用dlsym(RTLD_NEXT, "malloc")找到真正的 malloc
 *          3:函数在调用真正的 malloc 前后添加我们的代码（这里是打印日志）
 *          4:通过LD_PRELOAD让系统优先加载我们的库
 *
 *    这种技术经常用于：
 *                   调试内存问题
 *                   给程序添加日志
 *                   修改程序行为而不用改源码
 *                   临时修复运行中的服务
 *    当然，这项技术也常被黑客利用来劫持程序函数，所以理解它不仅能提升编程能力，也对安全防护很重要！
 *
 *    build:
 *          $ gcc -shared -fPIC LD_PRELOAD_test.c -o libmemtrace.so -ldl
 *          $ LD_PRELOAD=./libmemtrace.so ./my_program
 * @**************************************:
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

// 原始malloc函数指针
static void *(*real_malloc)(size_t) = NULL;

// 拦截 malloc 函数
void *malloc(size_t size)
{
    // 延迟初始化原始函数
    if (real_malloc == NULL)
    {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    // 调用原始malloc
    void *ptr = real_malloc(size);

    // 打印跟踪信息
    fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);

    return ptr;
}

// 定义一个函数指针类型，指向原始的 strlen 函数
// 这是为了让我们能够在自定义的 strlen 中调用原始的 strlen
typedef size_t (*orig_strlen_fct_ptr)(const char *s);

// 这是我们的自定义 strlen 函数。
// 它必须与标准库中的 strlen 具有相同的函数签名。
size_t strlen(const char *s)
{
    // 静态变量用于存储原始 strlen 的地址，只获取一次
    static orig_strlen_fct_ptr original_strlen = NULL;

    // 如果还没有获取原始 strlen 的地址
    if (original_strlen == NULL)
    {
        // 使用 dlsym 获取原始 strlen 的地址。
        // RTLD_NEXT 表示在当前库的搜索路径中查找下一个匹配的符号。
        // 这通常会找到标准 C 库中的 strlen。
        original_strlen = (orig_strlen_fct_ptr)dlsym(RTLD_NEXT, "strlen");

        // 检查 dlsym 是否成功。如果失败，可能是严重错误。
        if (original_strlen == NULL)
        {
            fprintf(stderr, "Error: Could not find original strlen using dlsym!\n");
            // 此时无法调用原始 strlen，只能简单返回或处理错误
            // 这里为了示例，直接返回0，但在实际应用中需要更严谨的错误处理
            return 0;
        }
        printf("[LD_PRELOAD]: 成功拦截 strlen！原始 strlen 地址: %p\n", (void *)original_strlen);
    }

    if (NULL == s)
    {
        printf("sssssssssssss\n");
        return 0;
    }
    // 在这里添加你自定义的逻辑
    // 例如：打印日志、修改行为等
    printf("[LD_PRELOAD]: 自定义 strlen 被调用，字符串是: \"%s\"\n", s);

    // 调用原始的 strlen 来完成实际的计算
    size_t length = original_strlen(s);

    printf("[LD_PRELOAD]: 原始 strlen 返回长度: %zu\n", length);

    // 返回计算出的长度
    return length;
}