/*
 * @*************************************:
 * @FilePath     : /user/C/dlopen_test/backtrace_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-06-26 19:31:44
 * @LastEditors  : dof
 * @LastEditTime : 2025-10-01 12:00:56
 * @Descripttion :
 * @compile      : gcc backtrace_test.c -o backtrace_test -rdynamic
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h> // 需要链接 -rdynamic
#include <unistd.h>

#define MAX_STACK_FRAMES 128

void print_stack_trace()
{
    void *buffer[MAX_STACK_FRAMES];
    int frames;

    // 获取调用栈
    frames = backtrace(buffer, MAX_STACK_FRAMES);
    printf("获取到 %d 个栈帧:\n", frames);

    // 获取符号信息
    char **symbols = backtrace_symbols(buffer, frames);
    if (symbols == NULL)
    {
        perror("backtrace_symbols");
        return;
    }

    // 打印调用栈
    for (int i = 0; i < frames; i++)
    {
        printf("#%d %s\n", i, symbols[i]);
    }

    free(symbols);
}

// 测试函数
void func_c()
{
    print_stack_trace();
}

void func_b()
{
    func_c();
}

void func_a()
{
    func_b();
}

int main()
{
    printf("=== 调用栈跟踪示例 ===\n");
    func_a();
    return 0;
}