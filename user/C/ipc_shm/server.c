/*
 * @*************************************: 
 * @FilePath: /user/C/ipc_shm/server.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-29 13:21:57
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 14:26:58
 * @Descripttion: 共享内存服务器
 * @**************************************: 
 */



#include <unistd.h>

#include "comm.h"

int main(int argc, char const *argv[])
{
	int shmid = CreateShm(4096);

	char *addr = shmat(shmid, NULL, 0);
	sleep(1);

	int i = 0;
	while(i++ < 26)
	{
		printf("client %s\n", addr);
		sleep(1);
	}

	shmdt(addr);
	sleep(2);
	DestoryShm(shmid);
	return 0;
}
