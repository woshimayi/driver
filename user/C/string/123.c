/*
 * @*************************************:
 * @FilePath: /user/C/123.c
 * @version:
 * @Author: dof
 * @Date: 2022-10-21 14:58:41
 * @LastEditors: dof
 * @LastEditTime: 2022-10-27 14:19:43
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>

// #define f(a,b) a##b
// #define g(a)   #a
// #define h(a)   g(a)

// int main(int argc, char const *argv[])
// {
// 	printf("h(f(1,2))-> %s, g(f(1,2))-> %s\n", h(f(1,2)), g(f(1,2)));
// 	return 0;
// }

int test(int count)
{
	int i = 0;
	int n = (count + 7) / 8;

	switch (count % 8)
	{
	case 0:
		do
		{
			i++;
		case 7:
			i++;
		case 6:
			i++;
		case 5:
			i++;
		case 4:
			i++;
		case 3:
			i++;
		case 2:
			i++;
		case 1:
			i++;
		} while (--n > 0);
	}
	return i;
}

int main(int argc, char const *argv[])
{
	printf("%d\n", test(5));
	if (-1)
	{
		printf("zzzzzx\n");
	}
	return 0;
}
