/*
 * @*************************************: 
 * @FilePath     : /user/C/ipc_test/server.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-07-19 14:54:51
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-15 17:38:27
 * @Descripttion :  内存型共享内存
 * @compile      :  
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define BUFSZ 32

enum
{
	mem_0,
	mem_1,
	mem_2,
	mem_3,
	mem_4,
	mem_5,
	mem_6,
	mem_7,
	mem_max
} mem_stat;

typedef struct
{
	unsigned int ulStats[mem_max];
} SHR_MEM;

int shmid;
SHR_MEM *shmadd = NULL;

void signal_handler(int signum)
{
	printf("signum = %d\n", signum);
	// 分离共享内存和当前进程
	int ret = 0;
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

	// 删除共享内存
	shmctl(shmid, IPC_RMID, NULL);

	system("ipcs -m"); // 查看共享内存
	exit(0);
}

int main(int argc, char *argv[])
{
	int ret;
	key_t key;

	signal(SIGINT, signal_handler);

	// 创建 唯一 key值
	key = ftok(".", 2015);
	if (key == -1)
	{
		perror("ftok");
	}
	printf("key = %d\n", key);

	// 创建共享内存
	shmid = shmget(key, BUFSZ, IPC_CREAT | 0666);
	if (shmid < 0)
	{
		perror("shmget");
		exit(-1);
	}
	system("ipcs -m"); // 查看共享内存

	// 映射
	shmadd = shmat(shmid, NULL, 0);
	if (shmadd < 0)
	{
		perror("shmat");
		_exit(-1);
	}

	char i = 0;
	while (1)
	{
		for (int j = 0; j < mem_max; j++)
		{
			printf("client %04d %d %d", i++, j, shmadd->ulStats[j]);
			printf("\n");
		}
		printf("\n");

		sleep(1);
	}

	return 0;
}