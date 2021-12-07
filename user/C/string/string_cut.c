/*
 * @*************************************: 
 * @FilePath: /user/C/string/string_cut.c
 * @version: 
 * @Author: dof
 * @Date: 2021-11-23 11:34:40
 * @LastEditors: dof
 * @LastEditTime: 2021-12-06 17:07:03
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>


int str_cut(char * input, const char * c, char **out)
{
	char *ptr;
	int i = 0;

	char tmp_out[][256] = {0};

	ptr = strtok(input, c);
	while (ptr != NULL)
	{
		printf("ptr = %s\n", ptr);
		strncpy(tmp_out[i++], ptr, 256);
		ptr = strtok(NULL, c);
	}

	out = &tmp_out;

	return 0;
}


int str_xxx(void) 
{
	int i = 0;
	char fileName[64] = {0}, *ch = "Dateline Standard Time.Eniwetok, Kwajalein";
	while (*ch != '\0' && *ch != ',')
	{
		if (*ch != ' ')
			fileName[i] = *ch;
		else
			fileName[i] = '_';
		ch ++;
		i ++;
	}
	printf("filename = %s\n", fileName);
	
	return 0;
}


typedef struct queue
{
	char *queue;
	char *data;
} _queue;



int  main()
{
	char cmd[256] = "bs /b/e egress_tm/dir=us,index=1 | grep queue_stat | awk -F '[{}]' '{print $4 $7}'";
	char * queue = "queue_id=0packets=1191,bytes=162806";
	char line[256] = {0};
	int n1=0, n2=0, n3=0;

	FILE *fd;
	fd = popen(cmd, "r");
	if (NULL != fd)
	{
		while (fgets(line, 256, fd) != NULL)
		{
			printf("line=%s\n", line);
			sscanf(line, "queue_id=%dpackets=%d,bytes=%d", &n1, &n2, &n3);
			printf("%d %d %d\n", n1, n2, n3);
		}
	}
//
//	printf("qeeue = %s\n", queue );
//	
//	char s1[256] = {0};
//	char s2[256] = {0};




//	char str[256] = "192.168.1.1/24,192.168.5.1/24,192.168.4.1/24,192.168.3.1/24,192.168.2.1/24";
//	char *buf[256] = {{0}};
//	
//	str_cut(str, ",", buf);
//
//	for (int i = 0; i<5; i++)
//	{
//		printf("out = %s\n", buf[i]);
//	}
//
//		return 0;
}
