/*
 * @*************************************:
 * @FilePath: /user/C/string/array_order.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:06:56
 * @Descripttion:  分解字符串 按照分号
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


URL_SERVER *create_informUrl(const char *informURL)
{
	char *token = NULL;
	char   url1[1024] = {0};

	strncpy(url1, informURL, sizeof(url1));
	token = strtok(url1, ";");

	while (token != NULL)
	{
		//		printf("token = %s\n", token);
		strncpy(newNode->url1, token, sizeof(newNode->url1));
		token = strtok(NULL, ";");
	}

	return ;
}


int main()
{
	char informurl3[1024] =
	    "http://www.bbums.cn/reg/;http://nmg-ct.cnspeedtest.com:9806/nsupload/uploadfile;http://www.bbums.org.cn/reg123/;http://nmg-ct.cnspeedtest.com:9806/nsupload/upload";
	char informurl4[1024] =
	    "http://www.bbums.cn1/reg/;http://nmg-ct.cnspeedtest.com:98061/nsupload/uploadfile;http://www.bbums.org.cn/reg123/;http://nmg-ct.cnspeedtest.com:9806/nsupload/upload";



	return 0;
}


