#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct gps
{
	int id;
	char x[64];
	char y[64];
	char z[64];
} gps_t;

char *strrpl(char *s, const char *s1, const char *s2)
{
	char *ptr;

	while (ptr =  strstr(s, s1))
	{
		memmove(ptr + strlen(s2), ptr + strlen(s1), strlen(ptr) - strlen(s1) + 1);
		memcpy(ptr, &s2[0], strlen(s2));
	}

	return s;
}


int main(int argc, const char *argv[])
{
	FILE *fd;
	char buf[4096 * 16] = {0};
	char *token;
	char *token1;

	//	char x[64], y[64], z[64];
#if 1
	gps_t gps[] = {0};
	fd = fopen("123.kml", "rb");
	if (NULL == fd)
	{
		printf("fail open file");
	}

	while (fgets(buf, sizeof(buf), fd))
	{
		if (NULL != strstr(buf, "coordinates"))
		{
			token = strtok(buf, "<coordinates>");
			token = strtok(NULL, "<coordinates>");
			break;
		}
		memset(buf, '\0', sizeof(buf));
	}

	token = strtok(token, " ");
	int i = 0;
	int j = 0;
	while (token = strtok(NULL, " "))
	{
		//		printf("%5d = %s\n", i, token);
		strrpl(token, ",", "\n");
		//		printf("str = %s\n", token);
		sscanf(token, "%s %s %s", gps[i].x, gps[i].y, gps[i].z);
		gps[i].id = i;
		printf("id =%d %s	%s	%s\n", i, gps[i].x, gps[i].y, gps[i].z);
		i++;
	}

	//	for (j=0; j<i; j++)
	//	{
	//		printf("id=%5d	%10s	%10s	%10s\n", gps[j].id, gps[j].x, gps[j].y, gps[j].z);
	//	}

#endif

	//	char str[128] = "121.00628342,31.11101863,0";
	//	strrpl(str, ",", " ");
	//	printf("str = %s\n", str);
	//	sscanf(str, "%s %s %s", x, y, z);
	//	printf("%s\n%s\n%s\n", x, y, z);

	return 0;
}

