#include <stdio.h>
#include <stdlib.h>
#include <string.h>


double db = 0;

void ping(int str, double str1)
{
	FILE *fp = NULL;
	char cmd[48] = {0};
	char buf[128] = {0};
	int totalmm = 0;

	double totalfree = 0;

	double L = 0;

	char p[64];

	//        4 packets transmitted, 4 received, 0% packet loss, time 3003ms
	//        rtt min/avg/max/mdev = 6.927/8.696/9.942/1.107 ms
	sscanf("received, 43% packet loss, time 6008ms", "%*s%d%*s", &totalmm);
	printf("mm = %d\n", totalmm);
	sscanf("mdev = 7.694/8.347/9.092/0.565 ms", "%*[^/]/%[^/]", buf);
	L = atof(buf);
	printf("%.3f\n", L);
	str1 = L;
	str = totalmm;

}


#define DEFAULT_CROND_PATH "/etc/crontabs/root"

int main()
{
	char schedule_list[1024] =
	    "\"0-2-1\",\"0-3-3\",\"0-3-5\",\"0-48-6,0\",\"1-4-1,2,3,4,5\",\"0-1-1\",\"0-1-2\",\"1-4-3\",\"0-3-3\",\"0-3-5\",\"0-48-6,0\",\"1-4-1,2,3,4,5\",\"0-1-1\",\"0-1-2\",\"1-4-3\",\"2-3-4\",\"2-5-5\"";
	char *token;
	char *p;
	char tokenvalue[64] = {0};
	char buf[1024] = {0};

	token = strtok_r(schedule_list, ",", &p);
	while (NULL != token)
	{
		snprintf(tokenvalue, sizeof(tokenvalue) - 1, "%s", token);
		printf("tokenvalue=%s\n", tokenvalue);
		strncat(buf, tokenvalue, sizeof(tokenvalue));
		token = strtok_r(NULL, ",", &p);
	}
	printf("buf = %s\n", buf);


	char str[64] = "21";
	printf("%d\n", str - '\0');

	printf("rm %s\n", DEFAULT_CROND_PATH);

	return 0;
}
