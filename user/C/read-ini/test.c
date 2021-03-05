/*test.c*/
#include "ini.h"
#include <stdio.h>
int main(int argc, char const *argv[])
{
    char buff[100];
    int ret;

    ret = GetIniKeyString("city", "beijing", "./test.ini", buff);
    printf("ret:%d,%s\n", ret, buff);

    ret = GetIniKeyString("study", "highschool", "./test.ini", buff);
    printf("ret:%d,%s\n", ret, buff);

    ret = PutIniKeyString("study", "highschool", "zzzz", "./test.ini");
    printf("put ret:%d\n", ret);
    ret = GetIniKeyString("study", "highschool", "./test.ini", buff);
    printf("ret:%d,%s\n", ret, buff);
    return 0;
}