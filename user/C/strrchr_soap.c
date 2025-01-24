#include <stdio.h>
#include <string.h>

int main()
{
    const char *str[1024] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.Enable",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.ConnectionType",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_VLANMode",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_VLANIDMark",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_ServiceList",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_MulticastVlan",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_IPMode",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Enable",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Type",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Account",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Password",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION125.1.Enable",
							"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION125.1.Value"}
    const char ch = '.';
    char *ptr;
    char tmp[1024] = {0};
	
	
	for (int i=0; i<sizeof(str)/sizeof(str[0]); i++)
	{
		printf("str[%d] = %s\n", str[i]);
	}

    ptr = strrchr(str, ch);

    if (ptr != NULL)
    {
        printf("字符 '%c 出现的位置为 %ld. \n",ch,  ptr - str + 1);
        printf("|%c| 之后的字符串是 - |%s|\n", ch, ptr);
        strncpy(tmp, str, ptr-str+1);
        printf(" tmp = %s \n", tmp);
    }
    else
    {
        printf("没有找到字符 'd' \n");
    }
    return (0);
}
