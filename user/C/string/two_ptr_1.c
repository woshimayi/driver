/*
 * @*************************************: 
 * @FilePath: /user/C/string/two_ptr_1.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-05 19:38:49
 * @LastEditors: dof
 * @LastEditTime: 2022-07-05 23:14:02
 * @Descripttion:  二级指针，结构体 内申请内存，释放
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
	/* data */
	int number;
	char *msg;
} unit_t;

void alloc_UNIT(unit_t **pp)
{
	unit_t *p = (unit_t*)malloc(sizeof(unit_t));
	if (p == NULL)
	{
		printf("out of memory\n");
		exit(1);
	}

	p->number = 3;
	// p->msg = malloc(20);

	// strcpy(p->msg, "Hello world");

	*pp = p;
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
	alloc_UNIT(&p);
	printf("number: %d\n msg: %s\n", p->number, p->msg);
	free_unit(p);
	p = NULL;
	return 0;
}
