/*
 * @*************************************:
 * @FilePath: /user/C/string/get_pid_name.c
 * @version:
 * @Author: dof
 * @Date: 2024-02-06 14:03:56
 * @LastEditors: dof
 * @LastEditTime: 2024-02-06 14:13:38
 * @Descripttion: 获取进程名称
 * @**************************************:
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

size_t get_executable_path(char *processname, size_t len)
{
    char *path_end;
    char processdir[PATH_MAX];
    if (readlink("/proc/self/exe", processdir, len) <= 0)
    {
        return -1;
    }
    path_end = strrchr(processdir, '/');
    if (path_end == NULL)
    {
        return -1;
    }
    ++path_end;
    strcpy(processname, path_end);
    *path_end = '\0';
    return (size_t)(path_end - processdir);
}

int main()
{
    char processname[1024];
    get_executable_path(processname, sizeof(processname));
    printf("processname:%s\n", processname);
}