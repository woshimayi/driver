/*
 * @*************************************:
 * @FilePath     : /user/C/ipc_mmap/mmap_client.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-15 17:45:54
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-15 18:09:04
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct
{
	char name[4];
	int age;
} people;

int main(int argc, char **argv) // map a normal file as shared mem:
{
	int fd, i;
	people *p_map;
	char temp;

	fd = open(argv[1], O_CREAT | O_RDWR | O_TRUNC, 00777);
	lseek(fd, sizeof(people) * 10 - 1, SEEK_SET);
	write(fd, "", 1);

	p_map = (people *)mmap(NULL, sizeof(people) * 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	temp = 'a';
	for (i = 0; i < 10; i++)
	{
		memcpy((*(p_map + i)).name, &temp, 2);
		temp += 1;
		(*(p_map + i)).age = 20 + i;
	}
	printf("initialize over \n");
	sleep(1);
	munmap(p_map, sizeof(people) * 10);
	printf("umap ok \n");

	return 0;
}