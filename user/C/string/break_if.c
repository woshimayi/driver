/*
 * @*************************************:
 * @FilePath: /user/C/string/break_if.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-18 09:36:57
 * @LastEditors: dof
 * @LastEditTime: 2022-07-18 10:53:48
 * @Descripttion: if 跳出后面的执行步骤，使用  do {} while(0);
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cmsUtl_strlen(const char *src)
{
   char emptyStr[1] = {0};
   char *str = (char *)src;
   
   if(src == NULL)
   {
      str = emptyStr;
   }	

   return strlen(str);
} 


int main(int argc, char const *argv[])
{
	unsigned ip = 0;
	char IP[32] = {0};
	char *MAC_1 = "asdfghjkl";
	char *MAC_2 = "ASDFGHJKL";
	do
	{
		if (0 == strcasecmp(MAC_1, MAC_2))
		{
			printf("qweqweqwe mac = %s %s\n", MAC_1, MAC_2);
		}
		
		// ip = 9;
		if (0 == cmsUtl_strlen(IP))
		{
			printf("qweqweqwe ip = %d\n", ip);
			break;
		}

		printf("ssss ip = %d\n", ip);
	} while (0);
	return 0;
}
