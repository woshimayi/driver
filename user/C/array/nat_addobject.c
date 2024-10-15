/*
 * @*************************************:
 * @FilePath: /user/C/array/nat_addobject.c
 * @version:
 * @Author: dof
 * @Date: 2024-06-04 19:19:55
 * @LastEditors: dof
 * @LastEditTime: 2024-06-04 19:32:31
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct CWMP_OP
{
    int (*getvalue)(char *name, struct CWMP_LEAF *entity, int *type, void **data);
    /*setvalue has another purpose to init/add/del/update objects for the instance objects*/
    int (*setvalue)(char *name, struct CWMP_LEAF *entity, int type, void *data);
};

struct CWMP_PRMT
{
    char *name;
    unsigned short type;
    unsigned short flag;
    struct CWMP_OP *op;
    void *area; // Area Info
};

typedef enum
{
    eCWMP_tNONE = 0,

    eCWMP_tSTRING,
    eCWMP_tINT,
    eCWMP_tUINT,
    eCWMP_tBOOLEAN,
    eCWMP_tDATETIME,
    eCWMP_tBASE64,
} eCWMP_TYPE;

#define CWMP_WRITE 0x01
#define CWMP_READ 0x02
#define CWMP_LNKLIST 0x04
#define CWMP_DENY_ACT 0x08
#define CWMP_FORCE_ACT 0x10
#define CWMP_ISPASSWORD 0x20
#define CWMP_ISVOIP 0x40
#define CWMP_ISWLAN 0x80
#define CWMP_HIDDEN 0x100

#define HI_TR069_OPRATOR_CMCC 0x2
#define HI_TR069_AREA_NONE 0
#define HI_TR069_OPTION_NONE 0

typedef unsigned int UINT32;

typedef struct
{
    int enable;
    char name[32];
    char type[32];
} __Nat_t;

#define CMCC_MAC_ADDR_LEN 18
struct CWMP_OP tSecurity_MacLeafOP = {NULL, NULL};
struct CWMP_PRMT tSecurity_MacLeafInfo[] =
    {
        /*(name,		type,		    flag,			        op)*/
        {"enable", eCWMP_tBOOLEAN, CWMP_WRITE | CWMP_READ, &tSecurity_MacLeafOP, NULL},
        {"name", eCWMP_tSTRING, CWMP_WRITE | CWMP_READ, &tSecurity_MacLeafOP, NULL},
        {"type", eCWMP_tUINT, CWMP_WRITE | CWMP_READ, &tSecurity_MacLeafOP, NULL},
};
enum eSecurity_MacLeaf
{
    eMac_Enable,
    eMac_Name,
    eMac_Type,
};

// struct CWMP_LEAF tNatLeaf[] =
//     {
//         {&tSecurity_MacLeafInfo[eMac_Enable]},
//         {&tSecurity_MacLeafInfo[eMac_Name]},
//         {&tSecurity_MacLeafInfo[eMac_Type]},
//         {NULL}};

#define STR(s) #s

#define PROCESS_HGCWMP  "ls"

int main(int argc, char const *argv[])
{
    __Nat_t nat = {0};

    // printf("%d\n", nat.tSecurity_MacLeafInfo[eMac_Enable].name);
    // printf("%s\n", tSecurity_MacLeafInfo[eMac_Enable].name);
    system(PROCESS_HGCWMP " &");
    return 0;
}
