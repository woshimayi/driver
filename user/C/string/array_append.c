/*
 * @*************************************: 
 * @FilePath: /user/C/string/array_append.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-01 15:10:10
 * @LastEditors: dof
 * @LastEditTime: 2022-07-01 17:30:10
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct
{
   unsigned char mode;
   char *fullPath;
}KEYCFG_ITEM;


typedef enum
{
   CONFIG_RST_NON             		=0x00,     /**< reset type is ignore */
   CONFIG_RST_KEY_SHORT             =0x01,     /**< reset type is short key */
   CONFIG_RST_KEY_LONG              =0x02,     /**< reset type is long key */
   CONFIG_RST_REMOTE	            =0x04,     /**< reset type is remote */
   CONFIG_RST_FATORY	            =0x08,     /**< reset type is factory */
   CONFIG_RST_KEY_SHORT_LONG_REMOTE =0x07,     /**< reset type is short key or long key or remote */
}ConfigRstType;

KEYCFG_ITEM keyCfgTableList[] =
	{
		{CONFIG_RST_KEY_SHORT, "InternetGatewayDevice.ManagementServer.URL"},
		{CONFIG_RST_KEY_SHORT, "InternetGatewayDevice.ManagementServer.LastConnectedURL"},
		{CONFIG_RST_KEY_SHORT, "InternetGatewayDevice.ManagementServer.Username"},
		{CONFIG_RST_KEY_SHORT, "InternetGatewayDevice.ManagementServer.Password"},
		{CONFIG_RST_KEY_SHORT, "InternetGatewayDevice.ManagementServer.ConnectionRequestUsername"},
		{CONFIG_RST_KEY_SHORT, "InternetGatewayDevice.ManagementServer.ConnectionRequestPassword"},
		{CONFIG_RST_KEY_SHORT, "BUCPE"},
		{CONFIG_RST_KEY_SHORT, "BUCPE"}
};


void  path(char ** argv)
{
	// char *str = NULL;
	// char *str1;

	argv[0] = strdup("InternetGatewayDevice.DeviceInfo.X_CMCC_TeleComAccount.Password");
	// printf("%d %p %s\n", strlen(str), str, str);
	// str1 = ;
	// str += strlen(str) + 2;
	argv[1] = strdup("InternetGatewayDevice.DeviceInfo.X_CMCC_TeleComAccount.Username");
	// printf("%d %p %s\n", strlen(str), str, str);
}

int main(int argc, char const *argv[])
{
	int i = 0;
	char *str[5] = {0};

	path(str);
	// printf("%s\n", str);
	// str += strlen(str) + 2;
	printf("%s\n%s\n%d\n", str[0], str[1], sizeof(str)/sizeof(char *));
	// free(str);
	for (i = 0; i < sizeof(str) / sizeof(char *); i++)
	{
		if (str[i] && strlen(str[i]))
		{
			free(str[i]);
		}
	}

	// KEYCFG_ITEM *bucpe;
	// for (i = 0; i < 2; i++)
	// {
	// 	bucpe = (KEYCFG_ITEM *)malloc(sizeof(KEYCFG_ITEM));
		
		
	// }

	// for (i = 0; i < sizeof(keyCfgTableList) / sizeof(KEYCFG_ITEM); i++)
	// {
	// 	printf("fullPath = %s\n", keyCfgTableList[i].fullPath);
	// }
	return 0;
}
