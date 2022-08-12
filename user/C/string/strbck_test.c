/*
 * @*************************************: 
 * @FilePath: /user/C/string/strbck_test.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-25 17:45:37
 * @LastEditors: dof
 * @LastEditTime: 2022-07-25 17:49:39
 * @Descripttion: 字符串切割
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static char *strsplit(char **stringp, const char *delim)
{
	char *start = *stringp;
	char *p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}

	return start;
}



int main(int argc, char const *argv[])
{

	char *gps_data = "$GNRMC,013300.00,A,2240.84105,N,11402.70763,E,0.007,,220319,,,D*69\r\n";

	printf("%s\n", strsplit(&gps_data, ","));

	return 0;
}
