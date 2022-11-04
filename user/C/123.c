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

#define f(a,b) a##b  
#define g(a)   #a  
#define h(a)   g(a)  

int main(int argc, char const *argv[])
{
	printf("h(f(1,2))-> %s, g(f(1,2))-> %s\n", h(f(1,2)), g(f(1,2)));
	return 0;
}
