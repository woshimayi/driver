/*
 * @*************************************:
 * @FilePath     : /user/C/dlopen_test/dladdr_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-06-26 17:44:39
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-26 18:00:12
 * @Descripttion :
 * @compile      :  gcc -o test test.c -ldl
 * @**************************************:
 */

#include <stdio.h>
#include "dlfcn.h" // 需要链接 -ldl

void my_function()
{
    printf("Hello from my_function!\n");
}

int main()
{
    Dl_info info;

    void (*func_ptr)() = my_function;

    if (dladdr(func_ptr, &info))
    {
        printf("Function name: %s\n", info.dli_sname); // 函数名
        printf("Library path: %s\n", info.dli_fname);  // 所在库路径
    }
    else
    {
        printf("Failed to get function info.\n");
    }

    return 0;
}