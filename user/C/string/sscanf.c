#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
* @param:
* @param:
* @param:
*/
int main(int argc, char *argv[])
{
    char str[128] = "sdfsd_yes_no=2&asda_flag=3";
    int yes = 0;
    int flag = 0;

    sscanf(str, "sdfsd_yes_no=%d&asda_flag=%d", &yes, &flag);
    printf("yes=%d falg=%d\n", yes, flag);
    return 0;
}
