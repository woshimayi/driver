/*
 * @*************************************: 
 * @FilePath: /user/C/string/return_ptr.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-06 09:18:12
 * @LastEditors: dof
 * @LastEditTime: 2022-07-06 09:44:39
 * @Descripttion: 函数返回值是指针的情况
 * @**************************************: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0
static const *msg[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Webnesday",
	"Thursday",
	"Friday",
	"Saturday"};

char * get_a_day(int idx)
{
	static char buf[20];
	strcpy(buf, msg[idx]);
	return buf;
}

char * get_a_day_1(int idx)
{
	// static char buf[20];
	// strcpy(buf, msg[idx]);
	return msg[idx];
}

int main(int argc, char const *argv[])
{
	printf("%s %s\n", get_a_day(0), get_a_day(1));
	printf("%s %s\n", get_a_day_1(0), get_a_day_1(1));
	return 0;
}
#else

typedef struct 
{
	/* data */
	int number;
	char *msg;
} unit_t;

unit_t *alloc_UNIT(void)
{
	unit_t *p = (unit_t*)malloc(sizeof(unit_t));
	if (p == NULL)
	{
		printf("out of memory\n");
		exit(1);
	}

	p->number = 3;

	p->msg = malloc(20);
	strcpy(p->msg, "Hello world");

	return p;
}

void free_unit(unit_t * p)
{
	if (p->msg)
	{
		free(p->msg);
	}

	if (p)
	{
		free(p);
	}
}


int main(int argc, char const *argv[])
{
	/* code */
	unit_t *p = NULL;
	p = alloc_UNIT();
	printf("number: %d\v msg: %s\n", p->number, p->msg);
	free_unit(p);
	p = NULL;
	return 0;
}

#endif