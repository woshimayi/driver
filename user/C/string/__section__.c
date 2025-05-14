/*
 * @*************************************:
 * @FilePath     : /user/C/string/__section__.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-10-31 17:34:41
 * @LastEditors  : dof
 * @LastEditTime : 2025-03-13 11:18:33
 * @Descripttion : section 作用 
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>

int __attribute__((section(".my_data"))) global_var = 10;

void __attribute__((section(".my_text"))) my_function()
{
    printf("Hello from my_function!\n");
}

int main()
{
    int a = 10;
    printf("global_var = %d\n", global_var);
    global_var = 20;
    // asm("movl $200, %0" : "=m"(global_var));
    printf("global_var = %d\n", global_var);
    my_function();
    
    return 0;
}