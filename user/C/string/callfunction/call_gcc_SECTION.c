/*
 * @*************************************:
 * @FilePath: /user/C/string/call_gcc_SECTION.c
 * @version:
 * @Author: dof
 * @Date: 2021-12-19 14:59:53
 * @LastEditors: dof
 * @LastEditTime: 2021-12-19 15:14:58
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#define SEC __attribute__((__section__("ss"), aligned(sizeof(void *))))

void func_1(int a, int b)
{
	printf("%s %d %d\n", __func__, __LINE__, a + b);
}
void func_2(int a, int b)
{
	printf("%s %d %d\n", __func__, __LINE__, a * b);
}

// 编译器会自动提供__start_ss，__stop_ss标志段ss的起止地址
extern size_t __start_ss;
extern size_t __stop_ss;

typedef struct
{
	void (*p)(int, int);
} node_t;

// 结构体变量a位于自定义段ss
SEC node_t a =
{
	.p = func_1,
};

SEC node_t b =
{
	.p = func_2,
};

int main(int argc, char **argv)
{
	int a = 3, b = 4;
	node_t *p;
	// 遍历段ss，执行node_t结构中的p指向的函数
	for (p = (node_t *)&__start_ss; p < (node_t *)&__stop_ss; p++)
	{
		p->p(a, b);
		a += 1;
		b += 2;
	}
}