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

char *cmsUtl_strchr(const char *s1, const char ch)
{
//	char emptyStr = '\0';
//	char *str1 = (char *)s1;
//	char  str2 = ch;
	
	if (s1 == NULL)
	{
	   return NULL;
	}
//	if (0 > str2 || 127 > str2)
//	{
//	   str2 = '\0';
//	}
	
	return strchr(s1, ch);
}



int  main()
{
//	char cmd[256] = "bs /b/e egress_tm/dir=us,index=1 | grep queue_stat | awk -F '[{}]' '{print $4 $7}'";
//	char * queue = "queue_id=0packets=1191,bytes=162806";
//	char line[256] = {0};
//	int n1=0, n2=0, n3=0;
//
//	FILE *fd;
//	fd = popen(cmd, "r");
//	if (NULL != fd)
//	{
//		while (fgets(line, 256, fd) != NULL)
//		{
//			printf("line=%s\n", line);
//			sscanf(line, "queue_id=%dpackets=%d,bytes=%d", &n1, &n2, &n3);
//			printf("%d %d %d\n", n1, n2, n3);
//		}
//	}
//
	printf("qeeue = %c\n",  0?'2':'4');
	char * str = NULL;
	if (cmsUtl_strchr(str, 'd'))
	{
		printf("ssss\n");
	}
	printf("11111 %s\n", cmsUtl_strchr(str, 'd'));
	printf("22222 %s\n", strchr(str, 'd'));
//	
//	char a = 'd';
//	char b = a;
//	printf("a = %c %d b = %c %d\n", a, b);
	
//	char startPort[16] = {0};
//	char endPort[16] = {0};
	
	
	
//	char s1[256] = {0};
//	char s2[256] = {0};

	//–ßπ˚¿‡À∆ 
//	char * str = "!234";
//	char  str[16] = "!234!asdasd!werwer!234234!";
//		
//	char *p;
//	p = strchr(str, '!');
//	printf("1 %s\n", p+1);
//	while (p != NULL)
//	{
//		printf("2 %s\n", p);
//		p = strchr(p+1, '!');
////		*p = '-';
//	}




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
