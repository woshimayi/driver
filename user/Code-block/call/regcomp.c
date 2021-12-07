#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isValidMac(char *value)
{
    int r; //r=0:valid, else not valid
    char *reg = "^[0-9A-F]\\([0-9A-F]\\:[0-9A-F]\\)\\{5\\}[0-9A-F]$";
    r = ereg(reg, value);
    return r;

}


int main()
{

    return 0;
}


