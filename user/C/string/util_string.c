/*
 * @*************************************:
 * @FilePath: /user/C/string/util_string.c
 * @version:
 * @Author: dof
 * @Date: 2021-11-23 11:34:40
 * @LastEditors: dof
 * @LastEditTime: 2022-06-23 16:17:43
 * @Descripttion:
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * @brief ???????
 *
 * @param input
 * @param c
 * @param out
 * @return int
 */
int str_cut(char *input, const char *c, char **out)
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

/**
 * @brief ??? ? ???? ????
 *
 * @param ch
 * @param src
 * @param dst
 * @return char*
 */
char *str_charReplace(const char *ch, char src, char dst)
{
	int i = 0;
	char tmp[64] = {0};
	// char *ch = "Dateline Standard Time.Eniwetok, Kwajalein";
	while (*ch != '\0')
	{
		if (*ch != src)
			tmp[i] = *ch;
		else
			tmp[i] = dst;
		ch++;
		i++;
	}
	printf("tmp = %s\n", tmp);

	return strdup(tmp);
}

typedef struct queue
{
	char *queue;
	char *data;
} _queue;

/**
 * @brief strchr ??
 *
 * @param s1
 * @param ch
 * @return char*
 */
char *cmsUtl_strchr(const char *s1, const char ch)
{
	if (s1 == NULL)
	{
		return NULL;
	}

	return strchr(s1, ch);
}

/**
 * @brief ??????
 *
 * @param input
 * @return char*
 */
char *strupr(const char *input)
{
	if (input == NULL)
	{
		return NULL;
	}

	int len = strlen(input);
	char tmp[len + 1];
	char str[len + 1];
	strncpy(str, input, sizeof(str));
	int i = 0;

	while ('\0' != str[i])
	{
		tmp[i] = toupper(str[i]);
		i++;
	}
	tmp[i] = '\0';

	return strdup(tmp);
}

/**
 * @brief ??????
 *
 * @param input
 * @return char*
 */
char *strlwr(const char *input)
{
	if (input == NULL)
	{
		perror("input is null\n");
		return NULL;
	}

	int len = strlen(input);
	char tmp[len + 1];
	char str[len + 1];
	strncpy(str, input, sizeof(str));
	int i = 0;

	printf("input = %s\n", str);

	while ('\0' != str[i])
	{
		tmp[i] = tolower(str[i]);
		i++;
	}
	tmp[i] = '\0';

	return strdup(tmp);
}

/**
 * @name: ???????
 * @test: test font
 * @msg:
 * @param {char} *source   ??????????
 * @param {char} flag      ????????
 * @return ??????char*??????????char **pt
 */
char **split(const char *input, char flag)
{
	char **pt;
	int j, n = 0;
	int count = 1;
	int len = strlen(input);
	char source[len + 1];
	char tmp[len + 1];
	tmp[0] = '\0';
	strncpy(source, input, len + 1);

	int i = 0;
	for (i = 0; i < len; ++i)
	{
		if (source[i] == flag && source[i + 1] == '\0')
			continue;
		else if (source[i] == flag && source[i + 1] != flag)
			count++;
	}
	// ?????char*??????????
	pt = (char **)malloc((count + 1) * sizeof(char *));

	count = 0;
	for (i = 0; i < len; ++i)
	{
		if (i == len - 1 && source[i] != flag)
		{
			tmp[n++] = source[i];
			tmp[n] = '\0'; // ??????????
			j = strlen(tmp) + 1;
			pt[count] = (char *)malloc(j * sizeof(char));
			strcpy(pt[count++], tmp);
		}
		else if (source[i] == flag)
		{
			j = strlen(tmp);
			if (j != 0)
			{
				tmp[n] = '\0'; // ??????????
				pt[count] = (char *)malloc((j + 1) * sizeof(char));
				strcpy(pt[count++], tmp);
				// ??tmp
				n = 0;
				tmp[0] = '\0';
			}
		}
		else
			tmp[n++] = source[i];
	}
	// ??????
	pt[count] = NULL;

	return pt;
}

char str_msplit(char input, char *splitChar)
{
	int j, in = 0;
	// char buffer[100] = "Fred male 25,John male 62,Anna female 16";
	char *p[20];
	char *buf = input;

	// if?NULL == splitChar?
	// {


	// }

	// char split[strlen(splitChar) + 1] = {0};

	char *outer_ptr = NULL;
	char *inner_ptr = NULL;

	// strncpy(split, splitChar, strlen(splitChar) + 1);
	while ((p[in] = strtok_r(buf, ",", &outer_ptr)) != NULL)
	{
		buf = p[in];
		while ((p[in] = strtok_r(buf, " ", &inner_ptr)) != NULL)
		{
			in++;
			buf = NULL;
		}
		buf = NULL;
	}
	printf("Here we have %d strings\n", in);
	for (j = 0; j < in; j++)
	{
		printf(">%s<\n", p[j]);
	}
}

void utils_string2jsonlist(const char *source)
{
	char **pt = NULL;
	int j, n = 0;
	int count = 1;
	int len = strlen(source);
	char tmp[len + 1];
	tmp[0] = '\0';
	char flag = ',';
	int i = 0;

	for (i = 0; i < len; ++i)
	{
		if (source[i] == flag && source[i + 1] == '\0')
			continue;
		else if (source[i] == flag && source[i + 1] != flag)
			count++;
	}

	pt = (char **)malloc((count + 1) * sizeof(char *));

	count = 0;
	for (i = 0; i < len; ++i)
	{
		if (i == len - 1 && source[i] != flag)
		{
			tmp[n++] = source[i];
			tmp[n] = '\0';
			j = strlen(tmp) + 1;
			pt[count] = (char *)malloc(j * sizeof(char));
			strcpy(pt[count++], tmp);
		}
		else if (source[i] == flag)
		{
			j = strlen(tmp);
			if (j != 0)
			{
				tmp[n] = '\0';
				pt[count] = (char *)malloc((j + 1) * sizeof(char));
				strcpy(pt[count++], tmp);
				n = 0;
				tmp[0] = '\0';
			}
		}
		else
		{
			tmp[n++] = source[i];
		}
	}
	pt[count] = NULL;

	for (i = 0; i < count; i++)
	{
		printf("pt = %s\n", pt[i]);
		// jsonOut.Add(pt[i]);
		free(pt[i]);
		pt[i] = NULL;
	}

	if (pt)
	{
		free(pt);
		pt = NULL;
	}

}


unsigned int cmsUtl_strlen(const char *src)
{
   char emptyStr[1] = {0};
   char *str = (char *)src;
   
   if(src == NULL)
   {
      str = emptyStr;
   }
   printf("%d\n", strlen(str));

   return strlen(str);
} 


int main()
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
	// printf("qeeue = %c\n",  0?'2':'4');
	// char * str = NULL;
	// if (cmsUtl_strchr(str, 'd'))
	// {
	// 	printf("ssss\n");
	// }
	// printf("11111 %s\n", cmsUtl_strchr(str, 'd'));
	// printf("22222 %s\n", strchr(str, 'd'));
	//
	//	char a = 'd';
	//	char b = a;
	//	printf("a = %c %d b = %c %d\n", a, b);

	//	char startPort[16] = {0};
	//	char endPort[16] = {0};

	//	char s1[256] = {0};
	//	char s2[256] = {0};

	//Ð§¹ûÀàËÆ
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

	// char str[] = "00112233eeDD";
	// char *str = "00112233eeDD";
	// char tmp[128] = {0};
	// stringtoupper(str, &tmp);
	// printf("main tmp = %s\n", strupr(str));
	// printf("main tmp = %s\n", strlwr(str));

	// char *tmp = strupr(str);
	// printf("main tmp = %s\n", tmp);
	// printf("main tmp = %s\n", strlowr(str));
	// printf("main1 tmp = %s\n", &tmp);
	//		return 0;

	// char str1[] = "Dateline Standard Time.Eniwetok, Kwajalein";
	// char *str1 = "Dateline Standard Time.Eniwetok, Kwajalein";
	// char *ch = str_charReplace(str1, ' ', '_');
	// printf("ch = %s\n", ch);
	// if(ch)
	// {
	// 	printf("not null\n");
	// 	free(ch);
	// }

	// char str3[] = "192.168.1.1/24,192.168.5.1/24,192.168.4.1/24,192.168.3.1/24,192.168.2.1/24";
	// char **p3;

	// p3 = split(str3, ',');
	// for (int i = 0; p3[i] != NULL; i++)
	// 	printf("p3[%d] = %s\n", i, p3[i]);

	// if (p3)
	// 	free(p3);

	// char emptyStr[1] = {0};
	// printf("%lu\n", strlen(emptyStr));
	// printf("%d\n", atoi(""));
	// char str_1[1024] = "asd,qwe,ert,tyu,iop";
	// natived_utils_string2jsonlist(str_1);

	char *str = NULL;
	cmsUtl_strlen(str);
}
