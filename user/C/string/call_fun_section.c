/*
 * @*************************************:
 * @FilePath: /user/C/string/call_fun_section.c
 * @version:
 * @Author: dof
 * @Date: 2021-12-19 15:15:59
 * @LastEditors: dof
 * @LastEditTime: 2021-12-19 15:15:59
 * @Descripttion:
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>

typedef void (*callback)(void);

typedef struct
{
	const char *name;
	callback fn;
} callback_t;

void f0();
void f1();

callback_t callbacks[] =
{
	{"cmd0", f0},
	{"cmd1", f1},
};

void f0() // 回调函数0
{
	printf("cmd0");
}

void f1() // 回调函数1
{
	printf("cmd1");
}

void do_callback(const char *name)
{
	size_t i;
	for (i = 0; i < sizeof(callbacks) / sizeof(callbacks[0]); i++)
	{
		if (!strcmp(callbacks[i].name, name))
		{
			callbacks[i].fn();
		}
	}
}

int main()
{
	do_callback("cmd1");
	getchar();
	return 0;
}