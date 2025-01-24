#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <arpa/inet.h>
#include <assert.h>
#include<time.h>
#include <math.h>

#define DBG_MSG(fmt, arg...) fprintf(stderr, "%s:%s:%d:" fmt "\n", __FILE__, __func__, __LINE__, ##arg);

int get_last_ip_octet(const char *ip_str) 
{
    unsigned int a, b, c, d;
    if (sscanf(ip_str, "%u.%u.%u.%u", &a, &b, &c, &d) == 4) {
        return d;
    } else {
        return -1; // 表示 IP 地址格式错误
    }
}

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

	/* 获取第一个子字符串 */
	//   token = strtok(str, "_");

	//   printf( "%s\n", token );


	//	printf("%d\n", strtoul(value, NULL, 10));
	//	printf("value = %s\n", value);


	//	char value[64] = {0};
	//	char data[1024] = {0};
	//	FILE *fid = NULL;
	//	int realFilesize = 0;
	//	int writeCrc = 0;
	//	fid = fopen("123.bin", "rb");
	//	if (NULL == fid)
	//	{
	//		printf("fail open file");
	//		return -1;
	//	}
	//
	//	fread(data, sizeof(char), 512, fid);
	//	fread(data, sizeof(char), strlen("encrpted_img"), fid);
	//	fread(data, 1, 4, fid);
	//	printf("data = %s\n", data);
	//    realFilesize = ntohl(*((unsigned int *)data));
	//    fread(data, 1, 4, fid);
	//    printf("realFilesize = %d\n", realFilesize);
	//
	//	fseek (fid, -4, 2);
	//	fread(value, 1, 4, fid);
	//	writeCrc = ntohl(*((unsigned int *)value));
	//	snprintf(value, sizeof(value), "%02X%02X%02X%02X", value[0]&0xFF, value[1]&0xFF, value[2]&0xFF, value[3]&0xFF);
	//	printf("111 value = %s\n", value);
	//
	//	snprintf(value, sizeof(value), "%02X%02X%02X%02X", (writeCrc >> 24) & 0xFF, (writeCrc >> 16) & 0xFF, (writeCrc >> 8)  & 0xFF, (writeCrc >> 0)  & 0xFF);
	//	printf("222 value = %s\n", value);
	//	fclose(fid);




	/* 继续获取其他的子字符串 */
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

	//	int n = 0;
	//	assert(n);



	// time_t timep;
	// struct tm *p;
	// time(&timep);
	// printf("time():%d\n", timep);
	// p = localtime(&timep);
	// timep = mktime(p);
	// printf("time()->localtime()->mktime():%d\n", timep);
	// return 0;

	char buf[64] = "24:8B:E0:E5:2A:78";
	char ip[64] = "192.168.1.101";
	printf("%c%c%c%c%c%c\n", buf[9],buf[10],buf[12],buf[13],buf[15],buf[16]);
	// printf("%c%c%c%c%c%c\n", buf[9],buf[10],buf[12],buf[13],buf[15],buf[16]);

	printf("%d\n", get_last_ip_octet(ip));

	printf("%f\n", 10*log10(23582/10000));

	return 0;
}

