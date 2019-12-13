#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bua_isCurrentTimeRight()
{
    FILE *fp = NULL;
    char timeStr[6] = {0};
    fp = popen("date \"+%Y\"", "r");
    if (NULL != fp)
    {
        fgets(timeStr, 5, fp);
    }
    pclose(fp);

    printf("%s\n", timeStr);
    //use 2016 to determine whether current time is right or not
    if (2016 < atoi(timeStr))
    {
        return 1;
    }

    return 0;
}



int fun1(int a, int b)
{
    printf("a = %d b = %d\n", a, b);
    return 0;
}

int fun1_test(int a, int b, char *fun)
{
    printf("%s a = %d b = %d\n", fun, a, b);
    return 0;
}

typedef fun1_test

//#define fun1 fun1_test

int main()
{
    int i = 3;

    if (i < 3)
    {
        printf("dsd\n");
    }

    //	fun1(2, 3);
    //	fun1_test(2, 3, __FUNCTION__);







    //	char * str[128] = {0};
    //	char str1[128] = "reg";
    //	strcat(str1, "/");
    //	printf("%s\n", str1);

    //	printf("%d\n", bua_isCurrentTimeRight());

    //	char str[20]="0123456789";
    //	int a=strlen(str);
    //	int b=sizeof(str);
    //	printf("a=%d b=%d \n", a, b);

    //	char *str = "http://61.163.160.69:8084/webff/versions/3FE56752AOCK07.3FE56752AOCK07";
    //	char *p = NULL;
    //	p = strchr(str, '/');
    //	printf("%s\n", strchr(p, ':'));

    //	if (str)
    //	{
    //		printf("str = %s %d\n", str, strlen(str));
    //	}
    //
    //	int k = 1;
    //	while (1)
    //	{
    //		printf("%d\n", k);
    //		k = (k == 1)?7:1;
    //		sleep(1);
    //	}

    return 0;
}


