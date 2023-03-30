/*
 * @*************************************: 
 * @FilePath: /user/C/fun_retuen.c
 * @version: 
 * @Author: dof
 * @Date: 2022-11-21 22:21:49
 * @LastEditors: dof
 * @LastEditTime: 2022-11-21 22:54:16
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE * sd_open(char * file)
{
	FILE *fd;
	// Use POSIX and C standard library functions to work with files.
	// First create a file.
	printf("Opening file");
	fd = fopen(file, "a+");
	if (fd == NULL)
	{
		printf("Failed to open file for writing");
		return;
	}
	return fd;
}

void sd_write(FILE * fd, char * data, int data_len)
{
	if (NULL == data && NULL == fd)
	{
		return;
	}

	fprintf(fd, "Hello %s \n", data);
	printf("File written");
}

void sd_close(FILE *fd)
{
	fclose(fd);
}

int main(int argc, char const *argv[])
{
	FILE *fd;
	fd = sd_open("./test.txt");

	for (int i=0; i < 10; i++)
	{
		printf("zzzz\n");
		sd_write(fd, "asasasa", sizeof("asasasa"));
	}

	sd_write(fd, "asasasa", sizeof("asasasa"));
	sd_close(fd);
	return 0;
}
