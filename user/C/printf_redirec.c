/*
 * @*************************************: 
 * @FilePath     : /user/C/printf_redirec.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-06-12 15:41:29
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-12 15:55:49
 * @Descripttion :  使用 freopen 重定向标准输出
 * @compile      :   
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test()
{
    printf("[%s:%d] 55555555\n", __FUNCTION__, __LINE__);
}

int main(int argc, char const *argv[])
{
    // 保存原始 stdout
    FILE *original_stdout = stdout;             // 如果不需要恢复 可以不加

    printf("111111111\n");
    printf("222222222\n");
    
    // freopen("/dev/null", "w", stdout);         // 重定向到 /dev/null 中
    // 重定向 stdout 到 /dev/null
    if (freopen("/dev/null", "w", stdout) == NULL) {
        perror("freopen 失败");
        return 1;
    }
    test();
    printf("333333333\n");
    test();

    // stdout = original_stdout;             // 恢复原始 stdout (可选) 不起作用
    printf("444444444\n");
    return 0;
}
