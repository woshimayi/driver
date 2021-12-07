#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef struct
{
    char Item;
    char *index;
    char Maxsta[][3];
} _OamSvaMaxUserConnect;

int main()
{
    _OamSvaMaxUserConnect *oamSvaMaxUserConnect = (_OamSvaMaxUserConnect *)malloc(sizeof(_OamSvaMaxUserConnect));
    char *endptr;
    int num = 0;
    oamSvaMaxUserConnect->index = "a";
    printf("%s\n", oamSvaMaxUserConnect->index);
    num = strtol(oamSvaMaxUserConnect->index, NULL, 16);
    printf("%d\n", num);

    strcpy(oamSvaMaxUserConnect->Maxsta[0], "a1");

    //	printf("%s\n", oamSvaMaxUserConnect->Maxsta[0]);
    printf("%d\n", strtol(oamSvaMaxUserConnect->Maxsta[0], NULL, 16));

    return 0;
}

