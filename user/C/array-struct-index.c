/*
 * @*************************************:
 * @FilePath     : /user/C/array-struct-index.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-13 09:54:35
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-13 11:53:43
 * @Descripttion :
 * @compile      :
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
    int  oid;
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
    UINT32 dOprator;
    UINT32 dArea;
    UINT32 dOption;
} CwmpAreaProfile;

const CwmpAreaProfile gArea_Cmcc_Common = {
    HI_TR069_OPRATOR_CMCC,
    HI_TR069_AREA_NONE,
    HI_TR069_OPTION_NONE,
};

enum eCMCC_WLANShareLeaf
{
    eSSIDIndex,
    eWLANShare_PortIsolation,
    eWLANShare_STAIsolation,
    eWLANShare_EnableUserId,
    eWLANShare_UserId
};

struct CWMP_OP tCMCC_WLANShareLeafOP = {NULL, NULL};
struct CWMP_PRMT tCMCC_WLANShareLeafInfo[] =
    {
        /*(name,		type,		flag,			op,    areainfo)*/
        {eSSIDIndex, "SSIDIndex", eCWMP_tUINT, CWMP_WRITE | CWMP_READ, &tCMCC_WLANShareLeafOP, (void *)&gArea_Cmcc_Common},
        {eWLANShare_PortIsolation, "PortIsolation", eCWMP_tBOOLEAN, CWMP_WRITE | CWMP_READ, &tCMCC_WLANShareLeafOP, (void *)&gArea_Cmcc_Common},
        {eWLANShare_STAIsolation, "STAIsolation", eCWMP_tBOOLEAN, CWMP_WRITE | CWMP_READ, &tCMCC_WLANShareLeafOP, (void *)&gArea_Cmcc_Common},
        {eWLANShare_EnableUserId, "EnableUserId", eCWMP_tBOOLEAN, CWMP_WRITE | CWMP_READ, &tCMCC_WLANShareLeafOP, (void *)&gArea_Cmcc_Common},
        {eWLANShare_UserId, "UserId", eCWMP_tSTRING, CWMP_WRITE | CWMP_READ, &tCMCC_WLANShareLeafOP, (void *)&gArea_Cmcc_Common},
};

#define STR(s) #s
#define LASTNAME(s) s

int main(int argc, char const *argv[])
{
    struct CWMP_PRMT tCMCC_WLANtmp = {eSSIDIndex, "SSIDIndex", eCWMP_tUINT, CWMP_WRITE | CWMP_READ, &tCMCC_WLANShareLeafOP, (void *)&gArea_Cmcc_Common};
    int oid = tCMCC_WLANtmp.oid;
    int type = tCMCC_WLANtmp.type;
    printf("%ld\n", sizeof(tCMCC_WLANShareLeafInfo) / sizeof(tCMCC_WLANShareLeafInfo[0]));
    printf("%s\n", tCMCC_WLANShareLeafInfo[oid].name);

    void **data;

    switch (type)
    {
    case eCWMP_tSTRING:
        *data = uintdup( pstPara->ulSSIDIndex );
    break;
    case eCWMP_tINT:
    break;
    case eCWMP_tUINT:
    break;
    case eCWMP_tBOOLEAN:
    break;
    
    default:
        break;
    }


    return 0;
}
