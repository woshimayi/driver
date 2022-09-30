/*
 * @*************************************:
 * @FilePath: /user/C/ipc_test/server.c
 * @version:
 * @Author: dof
 * @Date: 2022-09-29 14:53:55
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 17:15:01
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUFSZ 512

int main(int argc, char *argv[])
{
	int shmid;
	int ret;
	key_t key;
	char *shmadd;

	//创建 唯一 key值
	key = ftok(".", 2015);
	if (key == -1)
	{
		perror("ftok");
	}
	printf("key = %d\n", key);

	//创建共享内存
	shmid = shmget(key, BUFSZ, IPC_CREAT | 0666);
	if (shmid < 0)
	{
		perror("shmget");
		exit(-1);
	}

	//映射
	shmadd = shmat(shmid, NULL, 0);
	if (shmadd < 0)
	{
		perror("shmat");
		_exit(-1);
	}

	int i = 0;
	while(1)
	{
		printf("client %s", shmadd);
		printf("\n");
		sleep(1);
	}

	return 0;
}