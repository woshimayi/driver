/*
 * @*************************************: 
 * @FilePath: /call-cmake/main.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-20 17:08:43
 * @LastEditors: dof
 * @LastEditTime: 2023-09-06 10:59:12
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdlib.h>
#include <stdio.h>
#include "call_function.h"
#include "vos_log.h"

#include <fcntl.h>
#include <getopt.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int i = 23;
    
    vos_print("aaaaaaaa");
    vos_error("aaaaaaaa");
    vos_notic("aaaaaaaa");
    vos_debug("aaaaaaaa");


    Test4((void *)&i);

    printf("asdas\n");
    return 0;
}
