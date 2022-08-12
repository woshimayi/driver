/***********************************************************************
 *
 *  Copyright (c) 2017  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2017:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 * 
 ************************************************************************/

/*
 * dhcp_common.c
 *
 *  Created on:  Sep. 2017
 *      Author: Fuguo Xu <fuguo.xu@broadcom.com>
 */
 

/*
 * the function in this file is used for both DHCPv4 and DHCPv6
 */


#include "cms_util.h"
#include "dhcp_config_key.h"


/* For debug */
static void show_Option(const char *option, int len) 
{
    int i;

    for (i = 0; i < len; i++) 
    {
        printf("%02x ", option[i]);
        if ((i + 1) % 16 == 0) 
        {
            printf("\n");
        }
    }
    printf("\n\n");
}

CmsRet cmsDhcp_mkCfgDir(DhcpVersion dhcpVer, const char *ifName)
{
    char cmd[BRCM_UDHCPC_CONFIG_FILE_NAME_LEN + 16];
    
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/%s", 
        (DHCP_V4 == dhcpVer) ? BRCM_UDHCPC_CONFIG_DIR : BRCM_UDHCP6C_CONFIG_DIR, ifName);

    if(system(cmd) < 0 )
    {
        cmsLog_error("cmd %s failed", cmd);
    }

    return CMSRET_SUCCESS;
}

CmsRet cmsDhcp_readOption(DhcpVersion dhcpVer, const char *ifName, int code, char *option, int *len)
{
    char file[BRCM_UDHCPC_CONFIG_FILE_NAME_LEN] = {0};
    char buffer[VDR_MAX_DHCP_OPTION_LEN] = {0}, *buffPtr = NULL;
    FILE *in;
    int rlen, cpLen, vlen = 0;
    CmsRet ret = CMSRET_SUCCESS;

    if (NULL == ifName || NULL == option || NULL == len)
    {
        cmsLog_error("param invalid!");
        return CMSRET_INVALID_ARGUMENTS;
    }

    /* Full name example: /var/udhcpc/veip0.1/option122.out */
    snprintf(file, sizeof(file), "%s/%s/%s%d%s", 
                   (DHCP_V4 == dhcpVer) ? BRCM_UDHCPC_CONFIG_DIR : BRCM_UDHCP6C_CONFIG_DIR, ifName, 
                    BRCM_UDHCPC_CONFIG_FILE_BASE_NAME, code,
                    BRCM_UDHCPC_CONFIG_OUT_FILE_SUFFIX); /* output file from dhcp */
    if (!(in = fopen(file, "r")))
    {
        cmsLog_error("unable to open option file: %s", file);
        return CMSRET_INTERNAL_ERROR;
    }

    rlen = fread(buffer, 1, VDR_MAX_DHCP_OPTION_LEN, in);

    /* decrypt */
    dhcpEncryptCfgFile(buffer, rlen, BRCM_DHCP_CONFIG_KEY);

    if (DHCP_V4 == dhcpVer)
        vlen = buffer[VDR_OPTION_LEN_OFFSET] + VDR_OPTION_SUBCODE_OFFSET;
    else if (DHCP_V6 == dhcpVer)
    {
        buffPtr = &buffer[VDR_OPTION_V6_LEN_OFFSET];
        vlen = ntohs(*((uint16_t *)buffPtr)) + VDR_OPTION_V6_SUBCODE_OFFSET;
    }

    /* verify */
    if (rlen != vlen)
    {
        cmsLog_error("Invalid option file: %s", file);
        ret = CMSRET_INTERNAL_ERROR;
        goto exit;
    }

    if(cmsLog_getLevel() == LOG_LEVEL_DEBUG) 
    {
        cmsLog_debug("Option%d:\n", code);
        show_Option(buffer, rlen);
    }

    cpLen = rlen < (*len) ? rlen : (*len);
    memcpy(option, buffer, cpLen);
    *len = cpLen;

exit:
    fclose(in);
    return ret;
}

CmsRet cmsDhcp_saveOption(DhcpVersion dhcpVer, const char *ifName, int code, const char *option, int len)
{
    char file[BRCM_UDHCPC_CONFIG_FILE_NAME_LEN] = {0};
    char buffer[VDR_MAX_DHCP_OPTION_LEN] = {0};
    FILE *out;
    CmsRet ret = CMSRET_SUCCESS;
    unsigned int cnt = 1;
    int wLen = len < VDR_MAX_DHCP_OPTION_LEN ? len : VDR_MAX_DHCP_OPTION_LEN;

    if(cmsLog_getLevel() == LOG_LEVEL_DEBUG)
    {
        cmsLog_debug("Option%d:\n", code);
        show_Option(option, len);
    }

    /* encrypt */
    memcpy(buffer, option, wLen);
    dhcpEncryptCfgFile(buffer, wLen, BRCM_DHCP_CONFIG_KEY);

    /* Full name example: /var/udhcpc/veip0.1/option43.in */
    snprintf(file, sizeof(file), "%s/%s/%s%d%s", 
                   (DHCP_V4 == dhcpVer) ? BRCM_UDHCPC_CONFIG_DIR : BRCM_UDHCP6C_CONFIG_DIR, ifName, 
                    BRCM_UDHCPC_CONFIG_FILE_BASE_NAME, code,
                    BRCM_UDHCPC_CONFIG_IN_FILE_SUFFIX); /* input file for dhcp */
    if (!(out = fopen(file, "w"))) 
    {
        cmsLog_error("unable to open option file: %s", file);
        return CMSRET_INTERNAL_ERROR;
    }

    if (fwrite(buffer, wLen, cnt, out) != cnt)
    {
        cmsLog_error("write option file: %s fail", file);
        ret = CMSRET_INTERNAL_ERROR;
        goto exit;
    }

exit:
    fclose(out);
    return ret;
}

CmsRet cmsDhcp_encapsulateSubOption(uint16_t code, DhcpSubOptionTable *subOptTable, 
                  int subOptTableLen, const void *generalParm, char* optData, int *dataLen,
                  const DhcpOptionCodeLen codeLen, const DhcpOptionSizeLen sizeLen)
{
    char *dataPtr, *subCodeLenPtr, *valPtr;
    uint16_t sub_code;
    int subLen = 0, totalLen = 0, cnt = 0;
    int loop, value, i;
    char type;
    char valNew[VDR_MAX_DHCP_SUB_OPTION_LEN] = {0};
    CmsRet ret = CMSRET_SUCCESS;

    dataPtr = optData;
    for(loop = 0; loop < subOptTableLen; loop++)
    {
        sub_code = subOptTable[loop].subCode;
        type = subOptTable[loop].type; 
        if (OPTION_CODE_LEN1  == codeLen)
            *dataPtr++ = sub_code;
        else if (OPTION_CODE_LEN2  == codeLen)
        {
            *((uint16_t *)dataPtr) = htons(sub_code);
            dataPtr += 2;
        }
        else
        {
            cmsLog_error("Unsupported codeLen=%d\n", codeLen);
            ret = CMSRET_INTERNAL_ERROR;
            goto exit;
        }

        if (OPTION_SIZE_LEN1 == sizeLen)
            subCodeLenPtr = dataPtr++;
        else if (OPTION_SIZE_LEN2 == sizeLen)
        {
            subCodeLenPtr = dataPtr;
            dataPtr += 2;
        }
        else
        {
            cmsLog_error("Unsupported sizeLen=%d\n", sizeLen);
            ret = CMSRET_INTERNAL_ERROR;
            goto exit;
        } 

        if (NULL == subOptTable[loop].valFn)
            valPtr = subOptTable[loop].valDef;
        else
        {
            int len = sizeof(valNew);
            memset(valNew, 0, len);
            if (!subOptTable[loop].valFn(generalParm, valNew, &len))
            {
                valPtr = valNew;
            }
            else
            {
                cmsLog_error("option %d, subOption %d: update value fail!", code, sub_code);
                valPtr = subOptTable[loop].valDef;
            }
        }         
        
        if (OPTION_CHAR_STRING == type)
        {
            subLen = strlen(valPtr);
            strncpy(dataPtr, valPtr, subLen);
            dataPtr += subLen;
        }
        else if(OPTION_HEX_STRING == type) 
        {
            cnt = 0;
            while ( sscanf(valPtr, "%2x%n", &value, &i) == 1 )
            {
                *dataPtr++ = value;
                valPtr += i;
                cnt++;
            }
            subLen = cnt;
        }
        else
        {
            cmsLog_error("Unsupported type %d\n", type);
            ret = CMSRET_INTERNAL_ERROR;
            goto exit;
        }

        if (OPTION_SIZE_LEN1 == sizeLen)            
            *subCodeLenPtr = subLen;
        else if (OPTION_SIZE_LEN2 == sizeLen)
            *(uint16_t *)subCodeLenPtr = htons(subLen);        
        
        totalLen += codeLen + sizeLen + subLen;
    }

    *dataLen = totalLen;

exit:
    if (ret)
    {
        cmsLog_error("Encapsulate fail! code = %d\n", code);
    }
    return ret;    
}

 
CmsRet cmsDhcp_getSubOptionData(DhcpVersion dhcpVer, const char *optionData, int dataLen,
                                    int subCode, char *subOptionData, int *subDataLen)
{
    const char *ptr = optionData;
    uint16_t subCodeTmp = 0, subLenTmp = 0;

    if (!optionData || !subOptionData || !subDataLen)
    {
        cmsLog_error("param invalid\n");
        return CMSRET_INVALID_ARGUMENTS;
    }

    while (ptr < (optionData + dataLen))
    {
        if (DHCP_V4 == dhcpVer)
        {
            subCodeTmp = *ptr;
            ptr += 1;
            subLenTmp = *ptr;
            ptr += 1;
        }
        else /* DHCPv6 */
        {
            subCodeTmp = ntohs(*(uint16_t *)ptr);
            ptr += 2;
            subLenTmp = ntohs(*(uint16_t *)ptr);
            ptr += 2;
        }

        if (subCodeTmp == subCode)
        {
            memcpy(subOptionData, ptr, (subLenTmp < (*subDataLen)) ? subLenTmp : (*subDataLen));
            *subDataLen = subLenTmp;
            return CMSRET_SUCCESS;
        }

        ptr += subLenTmp;
    }

   return CMSRET_METHOD_NOT_SUPPORTED; /* not found the given sub-option */
}


