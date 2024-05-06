/*
 * @*************************************: 
 * @FilePath: /user/C/define_call.cpp
 * @version: 
 * @Author: dof
 * @Date: 2024-03-21 11:42:16
 * @LastEditors: dof
 * @LastEditTime: 2024-03-21 11:50:28
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int hi_ipc_call(char *pc_call_name)
{
	printf("pc_call_name = %s\n", pc_call_name);
	return 0;
}


#define HI_IPC_CALL(ipc_fun)                       \
    hi_ipc_call("hi_" ipc_fun "_call")


int main(int argc, char const *argv[])
{
    HI_IPC_CALL("zzzzzzz");    
    return 0;
}
