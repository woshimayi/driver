/*
 * @*************************************:
 * @FilePath: /user/C/define_call.cpp
 * @version:
 * @Author: dof
 * @Date: 2024-03-21 11:42:16
 * @LastEditors: dof
 * @LastEditTime: 2024-06-13 15:36:56
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

#define HI_IPC_CALL(ipc_fun) \
    hi_ipc_call("hi_" ipc_fun "_call")

char *strERR_9000 = "Method not supported";
char *strERR_9001 = "Request denied";
char *strERR_9002 = "Internal error";
char *strERR_9003 = "Invalid arguments";
char *strERR_9004 = "Resources exceeded";
char *strERR_9005 = "Invalid parameter name";
char *strERR_9006 = "Invalid parameter type";
char *strERR_9007 = "Invalid parameter value";
char *strERR_9008 = "Attempt to set a non-writable parameter";
char *strERR_9009 = "Notification request rejected";
char *strERR_9010 = "Download failure";
char *strERR_9011 = "Upload failure";
char *strERR_9012 = "File transfer server authentication failure";
char *strERR_9013 = "Unsupported protocol for file transfer";
char *strERR_default = "fault";

#define SOAPERR(er) strERR_##er

// 定义行号和文件名
#line 12345 "abcdefg.xxxxx"

int main(int argc, char const *argv[])
{
    HI_IPC_CALL("zzzzzzz");
    printf("%s line: %d\n", __FILE__, __LINE__);
    printf("%s line: %d\n", __FILE__, __LINE__);
    printf("err = %s\n", SOAPERR(9012));
    return 0;
}
