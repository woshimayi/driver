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

#define CMCC_MAC_ADDR_LEN 18
struct CWMP_OP tSecurity_MacLeafOP = {getNat, setNat};
struct CWMP_PRMT tSecurity_MacLeafInfo[] =
    {
        /*(name,		type,		    flag,			        op)*/
        {"Enable", eCWMP_tBOOLEAN, CWMP_WRITE | CWMP_READ, &tSecurity_MacLeafOP, (void *)&gArea_Cmcc_Common},
        {"Name", eCWMP_tSTRING, CWMP_WRITE | CWMP_READ, &tSecurity_MacLeafOP, (void *)&gArea_Cmcc_Common},
        {"Type", eCWMP_tSTRING, CWMP_WRITE | CWMP_READ, &tSecurity_MacLeafOP, (void *)&gArea_Cmcc_Common},
};
enum eSecurity_MacLeaf
{
    eMac_Enable,
    eMac_Name,
    eMac_Type,
};

struct CWMP_LEAF tNatLeaf[] =
    {
        {&tSecurity_MacLeafInfo[eMac_Enable]},
        {&tSecurity_MacLeafInfo[eMac_Name]},
        {&tSecurity_MacLeafInfo[eMac_Type]},
        {NULL}};

int getNat(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
    char *lastname = entity->info->name;
    unsigned int msgLen = 0;
    IgdNatParaTab stPara;
    IgdNatParaTab *pstPara = &stPara;
    unsigned int instnum = 0;
    char mac_addr[CMCC_MAC_ADDR_LEN] = {0};

    if ((name == NULL) || (type == NULL) || (data == NULL) || (entity == NULL))
        return -1;

    HI_OS_MEMSET_S((UINT8 *)pstPara, sizeof(IgdNatParaTab), 0, sizeof(IgdNatParaTab));

    instnum = getCMCC_MacFilterInstNum(name);
    if (instnum == 0)
        return ERR_9005;
    pstPara->ulIndex = instnum;

    msgLen = sizeof(stPara);
    pstPara->ulBitmap = MAC_FILTER_LIST_ATTR_MASK_ALL;
    CWMP_API_GET_ENTRY_PARA_INFO_FUNC(IGD_HG_NAT_TAB, (UINT8 *)pstPara, 0, msgLen);

    CWMP_LOG(LOG_DEBUG, "Enable=%u, Name=%s, Type=%s\n", pstPara->ulEnable, pstPara->aucFilterMAC, pstPara->aucFilterDestMAC);

    *type = entity->info->type;
    *data = NULL;
    if (strcmp(lastname, "Enable") == 0)
    {
        *data = booldup(pstPara->ulEnable);
    }
    else if (strcmp(lastname, "Name") == 0)
    {
        cwmp_sys_mactochar(pstPara->aucFilterMAC, mac_addr);
        *data = strdup((const char *)mac_addr);
    }
    else if (strcmp(lastname, "Type") == 0)
    {
        cwmp_sys_mactochar(pstPara->aucFilterDestMAC, mac_addr);
        *data = strdup((const char *)mac_addr);
    }
    else
    {
        return ERR_9005;
    }
    return 0;
}

int setNat(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
    char *lastname = entity->info->name;
    char *buf = data;
    unsigned int msgLen = 0;
    IgdNatParaTab stPara;
    IgdNatParaTab *pstPara = &stPara;
    unsigned int instnum = 0;

    if ((name == NULL) || (data == NULL) || (entity == NULL))
        return -1;
    if (entity->info->type != type)
        return ERR_9006;

    HI_OS_MEMSET_S((UINT8 *)pstPara, sizeof(IgdNatParaTab), 0, sizeof(IgdNatParaTab));
    instnum = getCMCC_MacFilterInstNum(name);
    if (instnum == 0)
        return ERR_9005;
    pstPara->ulIndex = instnum;
    printf("<%s:%d>Index[%#x]\n", __FUNCTION__, __LINE__, pstPara->ulIndex);

    IgdSecurMacFilterAttrConfTab stMacParainfo;
    IgdSecurMacFilterAttrConfTab *pstMacParainfo = &stMacParainfo;
    msgLen = sizeof(stMacParainfo);
    pstMacParainfo->ulBitmap = MAC_FILTER_ATTR_MASK_ALL;
    CWMP_API_GET_ENTRY_PARA_INFO_FUNC(IGD_SECUR_MAC_FILTER_ATTR_TAB, (UINT8 *)pstMacParainfo, 0, msgLen);

    if (strcmp(lastname, "Enable") == 0)
    {
        unsigned int *i = (unsigned int *)data;
        if (*i > true)
            return ERR_9007;
        pstPara->ulBitmap = NAT_ATTR_MASK_BIT0_ENABLE;
        pstPara->ucEnable = (*i == 0) ? 0 : 1;
    }
    else if (strcmp(lastname, "Name") == 0)
    {
        if (buf == NULL)
            return ERR_9007;
        if ((strlen(buf) == 0) || (strlen(buf) >= X_CMCC_URL_STR_LEN))
        {
            return ERR_9007;
        }
        HI_OS_STRCPY_S(pstPara->aucFilterMAC, buf);
        pstPara->ulBitmap |= NAT_ATTR_MASK_BIT2_NAME;
    }
    else if (strcmp(lastname, "Type") == 0)
    {
        if (buf == NULL)
            return ERR_9007;
        if ((strlen(buf) == 0) || (strlen(buf) >= X_CMCC_URL_STR_LEN))
        {
            return ERR_9007;
        }
        pstPara->ulBitmap = NAT_ATTR_MASK_BIT3_TYPE;
    }
    else
    {
        return ERR_9005;
    }

    CWMP_LOG(LOG_DEBUG, "Enable=%u, Name=%s, Type=%s\n", pstPara->ulEnable, pstPara->aucFilterMAC, pstPara->aucFilterDestMAC);

    msgLen = sizeof(stPara);
    CWMP_API_SET_ENTRY_PARA_INFO_FUNC(IGD_HG_NAT_TAB, (UINT8 *)pstPara, 0, msgLen);
    return 0;
}

struct CWMP_PRMT tCMCC_SecurityMacObjectInfo[] =
    {
        /*(name,			type,		flag,					op)*/
        {"0", eCWMP_tOBJECT, CWMP_READ | CWMP_WRITE | CWMP_LNKLIST, NULL, (void *)&gArea_Cmcc_Common}};

enum eCMCC_SecurityMacObject
{
    eCMCC_SecurityMac0
};

struct CWMP_LINKNODE tCMCC_SecurityMacObject[] =
    {
        /*info,  				leaf,			next,		sibling,		instnum)*/
        {&tCMCC_SecurityMacObjectInfo[eCMCC_SecurityMac0], tNatLeaf, NULL, NULL, 0},
};

struct CWMP_OP tCMCC_SecurityMac_OP = {NULL, objCMCC_SecurityMac};

int objCMCC_SecurityMac(char *name, struct CWMP_LEAF *e, int type, void *data)
{
    struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
    unsigned int ulloop;
    unsigned int msgLen = 0;
    int ret;
    IgdNatParaTab stPara;
    IgdNatParaTab *pstPara = &stPara;
    IgdNatParaTab *pstParaList = NULL;
    uint32_t *index;

    type = (type == eCWMP_tINITOBJ) ? eCWMP_tUPDATEOBJ : type;
    HI_OS_MEMSET_S((UINT8 *)pstPara, sizeof(IgdNatParaTab), 0, sizeof(IgdNatParaTab));

    switch (type)
    {
    case eCWMP_tINITOBJ:
    {
        unsigned int MaxInstNum = 0;
        struct CWMP_LINKNODE **table = (struct CWMP_LINKNODE **)data;

        if ((name == NULL) || (entity == NULL) || (data == NULL))
            return -1;

        CWMP_API_GET_ENTRY_CNT_FUNC(IGD_HG_NAT_TAB, &MaxInstNum);
        CWMP_LOG(LOG_DEBUG, "INIT IGD.X_CMCC_Security.MacFilter.obj  NUM=[%d]\n", MaxInstNum);
        if (MaxInstNum == 0)
            return 0;

        pstParaList = (IgdNatParaTab *)malloc(MaxInstNum * sizeof(IgdNatParaTab));
        if (NULL == pstParaList)
            return 0;
        HI_OS_MEMSET_S((UINT8 *)pstParaList, MaxInstNum * sizeof(IgdNatParaTab), 0, MaxInstNum * sizeof(IgdNatParaTab));
        msgLen = MaxInstNum * sizeof(IgdNatParaTab);

        CWMP_API_GET_ALL_ENTRY_FUNC(IGD_HG_NAT_TAB, pstParaList, msgLen, free(pstParaList));

        for (ulloop = 0; ulloop < MaxInstNum; ulloop++)
        {
            CWMP_LOG(LOG_DEBUG, "INIT IGD.X_CMCC_Security.MacFilter.obj  INDEX=[%d]\n", pstParaList[ulloop].ulIndex);
            if (create_Object(table, tCMCC_SecurityMacObject, sizeof(tCMCC_SecurityMacObject), 1, pstParaList[ulloop].ulIndex) < 0)
            {
                free(pstParaList);
                return -1;
            }
        }
        // add_objectNum( name, MaxInstNum );
        free(pstParaList);
        return 0;
    }
    case eCWMP_tADDOBJ:
    {
        if ((name == NULL) || (entity == NULL) || (data == NULL))
            return -1;

        msgLen = sizeof(stPara);
        CWMP_API_ADD_ENTRY_FUNC(IGD_HG_NAT_TAB, pstPara, msgLen);
        CWMP_LOG(LOG_DEBUG, "CM ADD IGD.X_CMCC_Security.UrlFilter.obj  INDEX=[%d]\n", pstPara->ulIndex);
        *(unsigned int *)data = pstPara->ulIndex;

        ret = add_Object(name, (struct CWMP_LINKNODE **)&entity->next, tCMCC_SecurityMacObject, sizeof(tCMCC_SecurityMacObject), data);
        HI_CWMP_LOG(CM_LOG_INFO_E, 1, "ADD %s .(inst:%d,ret:%x)", name, *(int *)data, ret);

        return ret;
    }

    case eCWMP_tDELOBJ:
    {
        pstPara->ulIndex = *(int *)data;
        msgLen = sizeof(stPara);
        CWMP_API_DEL_ENTRY_FUNC(IGD_HG_NAT_TAB, pstPara, msgLen);

        ret = del_Object(name, (struct CWMP_LINKNODE **)&entity->next, *(int *)data);
        HI_CWMP_LOG(CM_LOG_INFO_E, 1, "ADD %s .(inst:%d,ret:%x)", name, *(int *)data, ret);

        return ret;
    }
    case eCWMP_tUPDATEOBJ:
    {
        unsigned int num = 0, i, ulIndex = 0;
        struct CWMP_LINKNODE *old_table;

        CWMP_API_GET_ENTRY_CNT_FUNC(IGD_HG_NAT_TAB, &num);

        CWMPDBG(1, (stderr, "<%s:%d>[DEBUG]:table_count is %d\n", __FUNCTION__, __LINE__, num));
        if (num == 0)
            return 0;

        index = malloc(num * sizeof(uint32_t));
        if (NULL == index)
            return 0;
        (void)memset_s(index, num * sizeof(uint32_t), 0, num * sizeof(uint32_t));
        msgLen = num * sizeof(uint32_t);
        igdCmConfGetallIndex(IGD_HG_NAT_TAB, (UINT8 *)index, msgLen);

        old_table = (struct CWMP_LINKNODE *)entity->next;
        entity->next = NULL;

        for (i = 0; i < num; i++)
        {
            ulIndex = index[i];
            add_Object(name, (struct CWMP_LINKNODE **)&entity->next, tCMCC_SecurityMacObject, sizeof(tCMCC_SecurityMacObject), &ulIndex);
        }

        if (old_table)
        {
            destroy_ParameterTable((struct CWMP_NODE *)old_table);
        }

        free(index);
        return 0;
    }
    }

    return -1;
}
