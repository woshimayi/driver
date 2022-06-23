/*
 * @*************************************: 
 * @FilePath: /user/C/string/array_kuozhan.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-20 17:05:23
 * @LastEditors: dof
 * @LastEditTime: 2022-06-20 19:24:19
 * @Descripttion: 
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *informParameters_TR98[] = {0};
// {
//    "InternetGatewayDevice.DeviceInfo.HardwareVersion",
//    "InternetGatewayDevice.DeviceInfo.SoftwareVersion",
//    "InternetGatewayDevice.ManagementServer.ConnectionRequestURL",
//    "InternetGatewayDevice.ManagementServer.ParameterKey",
//    //"InternetGatewayDevice.X_CMCC_UserInfo.Password",
//    "InternetGatewayDevice.DeviceInfo.X_CMCC_DeviceType",
//    ""  /* this pathname can change, copied from acsState.connReqIfNameFullPath*/
// };

int main(int argc, char const *argv[])
{
	int numParms = (sizeof(informParameters_TR98)/sizeof(char*));

	printf("size = %d\n", sizeof(informParameters_TR98) / sizeof(char *));
	informParameters_TR98[numParms-1] = strdup("aaaa");
	informParameters_TR98[numParms++] = strdup("ss");

	int i = 0;
	for (i = 0; i < numParms; i++)
	{
		printf("%s\n", informParameters_TR98[i]);
	}
	printf("size = %d\n", sizeof(informParameters_TR98) / sizeof(char *));
	return 0;
}

