/*
 * @*************************************:
 * @FilePath: /user/C/ipc_test/client.c
 * @version:
 * @Author: dof
 * @Date: 2022-09-29 14:54:13
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:03:49
 * @Descripttion: shmat 共享内存
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

	//创建key值
	key = ftok(".", 2015);
	if (key == -1)
	{
		perror("ftok");
	}
	printf("key = %d\n", key);

	system("ipcs -m"); //查看共享内存

	//打开共享内存
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
		exit(-1);
	}

	// 写入共享内存数据
	int i = 0;
	char str[128] = {0};
	while (1)
	{
		printf("input string: ");
		scanf("%s", str);
		strcpy(shmadd, str);
	}

	//分离共享内存和当前进程
	ret = shmdt(shmadd);
	if (ret < 0)
	{
		perror("shmdt");
		exit(1);
	}
	else
	{
		printf("deleted shared-memory\n");
	}

	//删除共享内存
	shmctl(shmid, IPC_RMID, NULL);

	system("ipcs -m"); //查看共享内存

	return 0;
}