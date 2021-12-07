#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    char *string = "ac";
    long lnumber;

    /* strtol converts string to long integer */
    lnumber = strtol(string, NULL, 16);
    printf("string = %x  long = %ld\n", string, lnumber);

    return 0;
}
/*Êä³ö£º
  string = 0x11 long = 17
*/
