/*
 * @*************************************:
 * @FilePath: /user/C/ipc_test/client.c
 * @version:
 * @Author: dof
 * @Date: 2022-09-29 14:54:13
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:03:49
 * @Descripttion: shmat 共享内存  内存型共享内存
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

SHR_MEM *shmadd = NULL;
int shmid;

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

	// 创建key值
	key = ftok(".", 2015);
	if (key == -1)
	{
		perror("ftok");
	}
	printf("key = %d\n", key);

	system("ipcs -m"); // 查看共享内存

	// 打开共享内存
	shmid = shmget(key, BUFSZ, IPC_CREAT | 0666);
	if (shmid < 0)
	{
		perror("shmget");
		exit(-1);
	}

	// 映射
	shmadd = shmat(shmid, NULL, 0);
	if (shmadd < 0)
	{
		perror("shmat");
		exit(-1);
	}

	// 写入共享内存数据
	int i = 0;
	int id = 0;
	char str[128] = {0};
	while (1)
	{
		printf("input id: ");
		scanf("%d", &id);
		if (id < 0 || id >= mem_max)
		{
			continue;
		}
		printf("input str: ");
		scanf("%s", str);
		shmadd->ulStats[id] = atoi(str);
	}

	return 0;
}