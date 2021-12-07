#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define FIRWARE "/tmp/firware"





int main(int argc, const char *argv[])
{
    //	printf("##FIRWARE %s", FIRWARE);
    char *str = NULL;
    char buf[64] = {0};
    char pNum[] = "19";
    int a;

    printf("ss %d\n", atoi(pNum));
    printf("ss %d\n", buf);
    //	sscanf(str, "%s", buf);
    //	printf("buf = %s\n", buf);

    a = strtoul(pNum, NULL, 10); //最后的0，表示自动识别pNum是几进制
    printf("%d\n", a);



    return 0;
}

