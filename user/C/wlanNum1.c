#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	FILE *fd=NULL;
    char  tmp[100] = {0};
    int num = 0;

    fd = fopen("wlanNum", "r");
    if(fd != NULL)
    {
    	printf("qwe\n");
        fgets(tmp, sizeof(tmp), fd);
        printf("tmp: %s\n", tmp);
        sscanf(tmp, "%*s%d", &num);
    }
    fclose(fd);
    printf("num: %d", num);
	return 0;
}


