#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{
	char *str =
	    "InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.X_BROADCOM_COM_MacFilterObj.X_BROADCOM_COM_MacFilterCfg.{i}.";
	printf("%d", strlen(str) + 1);

}
