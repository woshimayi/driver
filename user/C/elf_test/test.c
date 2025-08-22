/*
 * @*************************************: 
 * @FilePath     : /user/C/elf_test/test.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-06-09 19:15:17
 * @LastEditors  : dof
 * @LastEditTime : 2025-06-12 15:47:13
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>




int main(int argc, char const *argv[])
{
    char *str = (char *)malloc(128);
    snprintf(str, 128, "dddddd");
    printf("str = %s = %d\n", str, strlen(str));
    free(str);
    str = NULL;
    // printf("str = %s = %d\n", str, strlen(str));
    printf("sssssssss\n");
    return 0;
}
