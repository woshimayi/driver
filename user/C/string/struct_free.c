/*
 * @*************************************: 
 * @FilePath: /user/C/string/struct_free.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-27 09:57:08
 * @LastEditors: dof
 * @LastEditTime: 2022-06-27 15:17:03
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct wget
{
	/* data */
	unsigned short    port;
    char              *uri;
} _wget;


void fun(char **url)
{
	*url = strdup("www.baidu.com");
}

int main(int argc, char const *argv[])
{
	_wget *w = (_wget *)malloc(sizeof(_wget));
	if ( NULL == w)
	{
		printf("malloc fali");
		return 0;
	}

	// wget->port = 80;
	// w->uri = strdup("www.baidu.com");
	fun(&(w->uri));
	// strcpy(w->uri, "www.baidu.com");

	free(w);

	return 0;
}
