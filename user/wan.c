#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DBG_MSG(fmt, arg...) fprintf(stderr, "%s:%s:%d:" fmt "\n", __FILE__, __func__, __LINE__, ##arg);

int main(int argc, const char *argv[])
{

	/*
	char str[256] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.5.WANIPConnection.1.qwer";

	char str1[256] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.5.WANPPPConnection.1.qwer";
	
	char str3[256] = {0};
	
	printf("%d\n", strlen(str1));
	
	int wanConId = 0;
	char * tmp = NULL;
	tmp = strstr(str, "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.");
	printf("tmp = %s\n", tmp);
	wanConId = atoi(tmp + 54);
//	str[72] = wanConId + '0'; // IP
	str1[73] = wanConId + '0'; // PPP
//	itoa(wanConId, str[strlen(str)-2], 1);

	str1[54] = '1';
	printf("wanaid = %d: %s\n %d\n", wanConId, str1, strlen(str1));
	strncpy(str3, str1, sizeof(str1));
	printf("str3 = %s\n", str3);
	
//	printf("InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.WANIPConnection.%d.%s", );
*/
	//	char str[128] = "port_0:4";
	//	char str1[128] = "GMT-14";
	//	printf("%d\n", str[7] - '0');
	//	printf("%s\n", &str1[3]);
	//	char buf[32] = {0};

	//	char str[128] = "EX6300";
	//	char str1[128] = "EX6300";

	//	printf("%d", strcmp(str, str1));
	//	int ret = -1;
	//	if (ret)
	//	{
	//		printf("ssssssssssssssss\n");
	//	}

	//   char str[80] = "V1.0.0.0_1.0.0";
	//   char str[80] = "V1.0.0.0";
	//   const char s[2] = "_";
	//   char *token;

	/* ��ȡ��һ�����ַ��� */
	//   token = strtok(str, "_");

	//   printf( "%s\n", token );

	char value[64] = {0};

	//	printf("%d\n", strtoul(value, NULL, 10));
	//	printf("value = %s\n", value);

	FILE *fd1 = NULL;
	fd1 = fopen("123.bin", "rb");
	if (NULL == fd1)
	{
		printf("fail open file");
		return -1;
	}

	fseek(fd1, -4, 2);
	fread(value, 1, 10, fd1);
	//	printf("ssssssssss %s\n", (unsight int*)value);
	snprintf(value, sizeof(value), "%02X%02X%02X%02X", value[0] & 0xFF, value[1] & 0xFF, value[2] & 0xFF, value[3] & 0xFF);
	printf("value = %s\n", value);
	fclose(fd1);

	//   while( token != NULL ) {
	//      printf( "%s\n", token );
	//      token = strtok(NULL, s);
	//   }

	//
	//	if (0 != a && 1 != a)
	//	{
	//		printf("aaaaaaaaaa");
	//	}
	//	else
	//	{
	//		printf("bbbbbbbbbb");
	//	}

	return 0;
}
