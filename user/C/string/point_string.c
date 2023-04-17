/*
 * @*************************************: 
 * @FilePath: /user/C/string/point_string.c
 * @version: 
 * @Author: dof
 * @Date: 2023-02-01 11:53:44
 * @LastEditors: dof
 * @LastEditTime: 2023-02-01 14:52:28
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	char *tmp = NULL;
	char tmpAlarmNumber[256] = "104007,104006";

	printf("%s\n", ('\0' == tmpAlarmNumber[0])?"":"," );

	// if((tmp = strstr(tmpAlarmNumber,",104006")) != NULL)
	// {
	// 	char *delstr = tmp+7;
	// 	while(*tmp != '\0')
	// 	{
	// 		*tmp = *delstr;
	// 		tmp++;
	// 		delstr++;
	// 	}
	// }
	
	return 0;
}




