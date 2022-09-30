/*
 * @*************************************:
 * @FilePath: /user/C/ipc_shm/client.c
 * @version:
 * @Author: dof
 * @Date: 2022-09-29 13:21:45
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 14:26:43
 * @Descripttion:
 * @**************************************:
 */

#include <unistd.h>

#include "comm.h"

int main(int argc, char const *argv[])
{
	int shmid = GetShm(4096);
	sleep(1);
	char *addr = shmat(shmid, NULL, 0);

	sleep(2);
	int i = 0;
	while (i < 26)
	{
		addr[i] = 'A' + i;
		i++;
		addr[i] = 0;
		sleep(1);
	}
	shmdt(addr);
	sleep(2);
	return 0;
}
