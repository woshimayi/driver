/*
 * @*************************************: 
 * @FilePath: /user/C/array/wan_array_hash.c
 * @version: 
 * @Author: dof
 * @Date: 2024-06-03 16:05:06
 * @LastEditors: dof
 * @LastEditTime: 2024-06-12 13:37:47
 * @Descripttion:  wan hash 判断wan 是否存在
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>

#ifndef PP
#define PP(fmt,args...) printf("[speed :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args )
#endif


unsigned int wanNum[64] = {0};


void signal_handle()
{
	void *l_buffer[512];
	char **l_ptrace;

	printf("\r\n=========>>>catch signal <<<=========\r\n");
	printf("Dump stack start...\n");

	int size = backtrace(l_buffer, 512);
	l_ptrace = backtrace_symbols(l_buffer, size);
	if(NULL == l_ptrace)
	{
	  perror("backtrace_symbols");  
	  exit(1);
	} 

	for(int i = 0; i < size; i++)
	{
	 fprintf(stdout,"  [%02d] %s\n", i, l_ptrace[i]);
	}

	printf("Dump stack end...\n");
	free(l_ptrace);
	exit(1);
}

int main(int argc, char const *argv[])
{
    unsigned int wanCon[4] = {1, 1, 1, 2};

    printf("sizeof(wanCon) = %d\n", sizeof(wanCon)/sizeof(wanCon[0]));
    // for (int i = 0; i < sizeof(wanCon)/sizeof(wanCon[0]); i++)
    // {
    //     printf("%d\n", wanCon[i]);
    //     wanNum[wanCon[i]] = 1;
    // }

    for (int i = 0; i < sizeof(wanCon)/sizeof(wanCon[0]); i++)
    {
        for (int i = 0; i < sizeof(wanCon)/sizeof(wanCon[0]); i++)
        {
            if (wanNum[wanCon[i]])
            {
                printf("%d\n", wanCon[i]);
                PP();
                continue;
            }
            wanNum[wanCon[i]] = 1;
        }
    }

    signal_handle();

    return 0;
}
