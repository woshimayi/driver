/*
	Name:
	Copyright:
	Author:
	Date: 12/02/20 10:23
	Description:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//传指针相当于拷贝指针地址
int fun(char *str,  int len)
{
    char str1[] = "qwertyuiopssssssssssssssssssssssssssssssssssssssssssssssssssss";
    //	strcpy(str , str1);
    memcpy(str, str1, len);  // 等同strcpy
    str[len - 1] = '\0';
    printf("%p %s %d\n", str1, str1, sizeof(str1));
    return 0;
}

//拷贝指针的引用
int fun1(char **str)
{
    char str1[] = "qwertyuiopaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    *str =  strdup(str1);
    printf("%p\n%p %s\n", str1, *str, *str);
    return 0;
}

int fun2(char *str[])
{
    char str1[] = "qwertyuiop";
    *str =  strdup(str1);
    printf("fun2 %p\n%p %s\n", str1, *str, *str);
    return 0;
}

int fun3(char str[][32])
{

    return 0;
}


int main(int argc, char *argv[])
{
    //	char str[32] = {0};
    //	fun(str, 32);
    //	printf("%p %s\n", str, str);

    char *str = NULL;
    fun1(&str);
    printf("main %p %s\n", str, str);

    return 0;
}

