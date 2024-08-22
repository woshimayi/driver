/*
 * @*************************************:
 * @FilePath     : /user/C/shell_generate_c/shell_exec.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-22 13:33:04
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-22 18:57:56
 * @Descripttion :  script execuite file
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "shell_include.h"

int main(int argc, char const *argv[])
{
    /* code */
    generate_file();
    system("cp /tmp/shell_include.sh /tmp/shell_include_tmp.sh");
    // system("pwd ../");
    char cmd[1024] = {0};
    // printf("argc = %d|%s|%s\n", argc, argv[0], argv[1]);
    for (int i=1; i < argc; i++)
    {
        if (1 == i)
        {
            snprintf(cmd, sizeof(cmd), "/tmp/shell_include_tmp.sh %s", argv[i]);
        }
        else
        {
            strcat(cmd, " ");
            strcat(cmd, argv[i]);
        }
    }
    // printf("cmd = %s\n", cmd);
    system(cmd);
    unlink("/tmp/shell_include.sh");
    unlink("/tmp/shell_include_tmp.sh");
    return 0;
}
