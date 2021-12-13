/*************************************************************************
	> File Name: exev.c
	> Author:
	> Mail:
	> Created Time: Wed 19 Dec 2018 07:40:50 PM CST
 ************************************************************************/

#include<stdio.h>
#include <unistd.h>
main()
{
	char *argv[] = {"ls", "-al", "/etc/passwd"};
	execv("/bin/ls", argv);
}
