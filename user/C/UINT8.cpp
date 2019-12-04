/*
	Name: 
	Copyright: 
	Author: 
	Date: 12/10/17 14:13
	Description: 
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	typedef char UINT8;
	typedef unsigned char UINT16;
	
	UINT8 num[12];
	UINT8 num1[10];
	
	char str[10] = "eeeeee";
	
	strncpy(num1, "sssss", sizeof("sssss"));
	strncpy(num, "sdfsd", sizeof("sdfsd"));
	strncpy(num, str, sizeof(str));
	
	strcat(num, ",");
	strcat(num, "sd");
		
	printf("%s %d\n", num, sizeof(num));
	printf("%s %d\n", num1, sizeof(num1));
	
	
	return 0;
}

