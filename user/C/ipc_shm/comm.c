/*
 * @*************************************: 
 * @FilePath: /user/SourceCode/ipc/comm.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-29 13:21:33
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 13:32:32
 * @Descripttion: 
 * @**************************************: 
 */


#include "comm.h"

static int CommShm(int size, int flags)
{
	key_t key = ftok(PATHNAME, PROJ_ID);
	if (key < 0)
	{
		perror("ftok");
		return -1;
	}

	int shmid = 0;
	if (shmid = shmget(key, size, flags) < 0)
	{
		perror("shmget");
		return -2;
	}
	return shmid;
}

int DestoryShm(int shmid)
{
	if (shmctl(shmid, IPC_RMID, NULL) < 0)
	{
		perror("shmctrl");
		return -1;
	}
	return 0;
}

int CreateShm(int size)
{
	return CommShm(size, IPC_CREAT | IPC_EXCL | 0666);
}

int GetShm(int size)
{
	return CommShm(size, IPC_CREAT);
}