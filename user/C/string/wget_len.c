#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void RUT_getQoeFileSizeInAdvance(int *qoeLen)
{
	char qoeLenString[16] = {0};
	char tmpstring1[32] = {0};
	char tmpstring2[32] = {0};
	char buf[256] = {0};
	int tmpData1 = 0;
	int tmpData2 = 0;
	char *p = NULL;
	//    UTIL_DO_SYSTEM_ACTION("killall wget");
	//    UTIL_DO_SYSTEM_ACTION("wget --spider '%s' -t 3 -T 3 -o /tmp/qoelen", qoeUrl);
	//UTIL_DO_SYSTEM_ACTION("wget --spider http://219.137.236.100:8000/bell/monitor.wan.bin -o /tmp/qoelen");
	FILE *fp = fopen("qoelen.cpp", "r");
	if (fp == NULL)
	{
		printf("Open '/tmp/qoelen' file failed. \n");
	}
	else
	{
		while (NULL != fgets(buf, sizeof(buf), fp))
		{
			printf("%s", buf);
			if (NULL == strstr(buf, "Length:"))
			{
				continue;
			}
			sscanf(buf, "Length: %s %s %s", qoeLenString, tmpstring1, tmpstring2);
			printf("\n\nqoeLenString = %s", qoeLenString);
			sscanf(qoeLenString, "%d,%d", &tmpData1, &tmpData2);
			*qoeLen = tmpData1 * 1000 + tmpData2;
			printf("\n\nThe length of the script named 'monitor.wan.bin' is %d Bytes.", *qoeLen);
			break;
		}

		if (0 == *qoeLen)
		{
			printf("The URL is invalid!!!");
		}
	}

	fclose(fp);
	//    UTIL_DO_SYSTEM_ACTION("rm /tmp/qoelen -rf");
}

int main()
{
	int len = 0;
	RUT_getQoeFileSizeInAdvance(&len);
	printf("%d\n", len);
	return 0;
}

