/*
 * @*************************************:
 * @FilePath     : /user/C/dlopen_test/backtrace_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-06-26 19:31:44
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-26 19:35:33
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h> // 需要链接 -rdynamic

void my_function()
{
    printf("Hello from my_function!\n");
    
    void *buffer[1];
    int num_ptrs = backtrace(buffer, 1); // 获取当前调用栈
    char **symbols = backtrace_symbols(buffer, num_ptrs);
    
    if (symbols)
    {
        printf("Function name (mangled): %s\n", symbols[0]); // 可能包含额外信息
        free(symbols);
    }
}

int main()
{
    void (*func_ptr)() = my_function;

    func_ptr();

    return 0;
}