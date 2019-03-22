/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
# 
# 
# This program is the proprietary software of Broadcom Corporation and/or its 
# licensors, and may only be used, duplicated, modified or distributed pursuant 
# to the terms and conditions of a separate, written license agreement executed 
# between you and Broadcom (an "Authorized License").  Except as set forth in 
# an Authorized License, Broadcom grants no license (express or implied), right 
# to use, or waiver of any kind with respect to the Software, and Broadcom 
# expressly reserves all rights in and to the Software and all intellectual 
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE 
# NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY 
# BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE. 
# 
# Except as expressly set forth in the Authorized License, 
# 
# 1. This program, including its structure, sequence and organization, 
#    constitutes the valuable trade secrets of Broadcom, and you shall use 
#    all reasonable efforts to protect the confidentiality thereof, and to 
#    use this information only in connection with your use of Broadcom 
#    integrated circuit products. 
# 
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH 
#    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
#    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
#    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
#    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
#    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR 
#    PERFORMANCE OF THE SOFTWARE. 
# 
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR 
#    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, 
#    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
#    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN 
#    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
#    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE 
#    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS 
#    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY 
#    LIMITED REMEDY. 
#
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <syslog.h>
#include "mdm.h"
#include "cmc_api.h"
#include "../inc/tr69cdefs.h" /* defines for ACS state */
#include "../inc/appdefs.h"
#include "phl.h"
#include "bcmWrapper.h"
#include "util_psp.h"
#include "tr69c_api.h"
#include "../SOAPParser/RPCState.h"
#include "hal_util_sys.h"
#include "hal_util.h"
#include "ctMiddleware.h"

extern ACSState   acsState;
extern TransferInfo transferList;
extern void *g_msgHandle;
extern InformEvList informEvList;
extern UBOOL8 g_writeLog;

#define DPROXY_FILE "/var/dproxy.conf"

extern void clearInformEventList(void);
extern UINT32 addInformEventToList(UINT8 event);
extern void downloadStop(char *msg, int status);  /* in httpDownload.c */
extern void downloadDiagStop(char *msg, int status);

void downloadStop_nosendinform(char *msg, int status);

#define DPROXY_FILE "/var/dproxy.conf"

void setInformState(eInformState state)
{
    vosLog_debug("set informState=%d", state);

    if (informState != state)
    {
        informState = state;
        saveTR69StatusItems();
    }
}  /* End of setInformState() */

/*
* Save the TR69 state values across the reboot
*/
void saveTR69StatusItems(void)
{
    GWStateData gwState;
    ACSState    *a = &acsState;
    int  i = 0;
    char enableSim[BUFLEN_16] = {0};

    /* init strings */
    memset(&gwState, 0, sizeof(GWStateData));

    /* fill State Data structure from acsState data */
    if (a->downloadCommandKey)
    {
        UTIL_STRNCPY(gwState.downloadCommandKey, a->downloadCommandKey, sizeof(gwState.downloadCommandKey));
    }
    
    if (a->rebootCommandKey)
    {
        UTIL_STRNCPY(gwState.rebootCommandKey, a->rebootCommandKey, sizeof(gwState.rebootCommandKey));
    }
    
    if (a->newParameterKey)
    {
        UTIL_STRNCPY(gwState.newParameterKey, a->newParameterKey, sizeof(gwState.newParameterKey));
    }
    else if (a->parameterKey)/* otherwise keep old one */
    {
        UTIL_STRNCPY(gwState.newParameterKey, a->parameterKey, sizeof(gwState.newParameterKey));
    }
    
    if (a->dlFaultMsg)
    {
        UTIL_STRNCPY(gwState.dlFaultMsg, a->dlFaultMsg, sizeof(gwState.dlFaultMsg));
    }
    
    gwState.contactedState = informState;
    gwState.dlFaultStatus  = a->dlFaultStatus;
    gwState.startDLTime    = a->startDLTime;
    gwState.endDLTime      = a->endDLTime;
    gwState.retryCount    = a->retryCount;
    gwState.upgradeDownloadFlag = a->upgradeDownloadFlag;
   
    /* save inform event list */
    gwState.informEvCount = informEvList.informEvCnt;

    for (i = 0; i< informEvList.informEvCnt; i++)
    {
        gwState.informEvList[i] = informEvList.informEvList[i];
    }

    if (HAL_sysSetTr69cData("tr69c_acsState", &gwState, sizeof(GWStateData)) != VOS_RET_SUCCESS)
    {
        vosLog_error("Unable to save TR69 status");
    }

    UTIL_SNPRINTF(enableSim, sizeof(enableSim), "%d", g_writeLog);
    if (HAL_sysSetTr69cData("tr69c_simulate", enableSim, sizeof(enableSim)) != VOS_RET_SUCCESS)
    {
        vosLog_error("save tr69c_simulate failed");
    }

}  /* End of saveTR69StatusItems() */


/* save to scratch pad */
void retrieveTR69StatusItems(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    GWStateData *objValue;
    char enableSim[BUFLEN_16] = {0};
    UINT32 savedEvCount, i;
    UINT32 bufLen = 0;

    if (NULL == (objValue = (GWStateData *)VOS_MALLOC_FLAGS(sizeof(GWStateData), ALLOC_ZEROIZE)))
    {
        vosLog_error("malloc failed");
        return;
    }

    bufLen = sizeof(GWStateData);
    HAL_sysGetTr69cData("tr69c_acsState", objValue, &bufLen);
    if (0 == bufLen)
    {
        vosLog_debug("No existing state info found in scratch pad.");
        VOS_FREE(objValue);
        return;
    }
    else if (bufLen != sizeof(GWStateData))
    {
        vosLog_error("error while reading tr69c acs state data from scratch pad, count=%d", bufLen);
        VOS_FREE(objValue);
        return;
    }
    
    if (objValue->downloadCommandKey[0] != '\0')
    {
        VOS_MEM_REPLACE_STRING(acsState.downloadCommandKey, objValue->downloadCommandKey);
    }
    else
    {
        VOS_MEM_REPLACE_STRING(acsState.downloadCommandKey, NULL);
    }

    vosLog_debug("downloadCommandKey =%s", acsState.downloadCommandKey);

    if (objValue->rebootCommandKey[0] != '\0')
    {
        VOS_MEM_REPLACE_STRING(acsState.rebootCommandKey, objValue->rebootCommandKey);
    }
    else
    {
        VOS_MEM_REPLACE_STRING(acsState.rebootCommandKey, NULL);
    }
    
    vosLog_debug("rebootCommandKey = %s ", acsState.rebootCommandKey);

    /*
    * The last parameterKey was saved in the scratch pad in the newParameterKey field.
    * But now, we want to put that value in parameterKey so that it will be used
    * in the next Inform message.
    * But in 4.x code, the Inform message grabs the parameterKey value from the MDM,
    * not from the acsState structure, so this code below is pretty much useless.
    * See CR 17990.
    */
    
    if (objValue->newParameterKey[0] != '\0')
    {
        VOS_MEM_REPLACE_STRING(acsState.parameterKey, objValue->newParameterKey);
    }
    else
    {
        VOS_MEM_REPLACE_STRING(acsState.parameterKey, NULL);
    }
    
    vosLog_debug("acsState.parameterKey =%s", acsState.parameterKey);
    
    if (objValue->dlFaultMsg[0] != '\0')
    {
        VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg, objValue->dlFaultMsg);
    }
    else
    {
        VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg, NULL);
    }
    
    vosLog_debug("acsState.dlFaultMsg =%s", acsState.dlFaultMsg);
    acsState.dlFaultStatus = objValue->dlFaultStatus;
    acsState.startDLTime   = objValue->startDLTime;
    acsState.endDLTime     = objValue->endDLTime;
    acsState.retryCount    = objValue->retryCount;
    acsState.upgradeDownloadFlag = objValue->upgradeDownloadFlag;

    informState            = (eInformState)objValue->contactedState;

    /* retrieve inform states and inform event list from scratch pad.
    * TR69c may exit due to time out before an inform message could be sent out.
    * 64 events have been reserved for events; but tr69c only has max of 11 which 
    * should be enough as of now.
    */
    savedEvCount = (UINT32)objValue->informEvCount;
    informEvList.informEvCnt = objValue->informEvCount;

    /* we only do 11 events, so just copy 11 events over */   
    if (savedEvCount > MAXINFORMEVENTS)
    {
        vosLog_debug("informEvCount in scratchpad is %d, max kept is %d", 
                      savedEvCount, MAXINFORMEVENTS);
    }
    else
    {
        for (i = 0; i < savedEvCount; i++)
        {
            informEvList.informEvList[i] = (eInformEvent)objValue->informEvList[i];
        }
    }

    if (SF_FEATURE_SUPPORT_PLUGIN)
    {
        if (1 == acsState.upgradeDownloadFlag)
        {
            CMC_sendTr69cStartAlarmInfo(0, "104058");
        }
    }

    vosLog_debug("informState=%d", informState);
    vosLog_debug("retryCount=%d", acsState.retryCount);

    VOS_FREE(objValue);
    
    bufLen = sizeof(enableSim);
    ret = HAL_sysGetTr69cData("tr69c_simulate", enableSim, &bufLen);
    if (VOS_RET_SUCCESS == ret)
    {
        if (0 == bufLen)
        {
            vosLog_debug("no tr69c_simulate info");
            return;
        }        
    }
    else
    {
        vosLog_error("get tr69c_simulate info failed ret=%d", ret);
        return;
    }
    
    g_writeLog = (UBOOL8)atoi(enableSim);
    
    vosLog_debug("g_writeLog = %d", g_writeLog);
    return;
}

void wrapperReboot(eInformState rebootContactValue)
{
    addInformEventToList(INFORM_EVENT_REBOOT_METHOD);

    setInformState(rebootContactValue);    /* reset on BcmCfm_download error */
    saveTR69StatusItems();

    vosLog_notice("CPE is REBOOTING with rebootContactValue =%d", rebootContactValue);

    tr69SaveTransferList();
    wrapperReset();
}


#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
static void tr69_processCardRemoteReset(void)
{
    UINT32 cardtype = 0;
    CMC_cardGetCardType(&cardtype);
    if (1 == cardtype)
    {
        UTIL_DO_SYSTEM_ACTION("echo remote reset finished ! > /var/config/remote_resetstatus");
    }
}
#endif


/*
* Call library factory reset 
*/
UBOOL8 wrapperFactoryReset(void)
{
    vosLog_notice("invalidating config flash (restore to default)");

    if (SF_FEATURE_SUPPORT_CT_RESETTODEFAULT)
    {
        #ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
        if (SF_FEATURE_SUPPORT_CARD)
        {
            tr69_processCardRemoteReset();
        }
        #endif

        if (VOS_RET_SUCCESS != CMC_sysResetConfig(CMC_SYS_CONFIG_RESET_REMOTE))
        {
            vosLog_error("CMC_sysResetConfig error!\n");
            return FALSE;
        }
        //cmcMgm_saveCTDefaultConfigToFlash(MODE_CT_REMOTE_RESET_TO_DEFAULT);
    }
    else
    {
        //cmcMgm_invalidateConfigFlash();
    }

    wrapperReset();

    return TRUE;
}


/** Request modem reset.
 *  This was called sysMipsSoftReset in the old cfm code.
 *  In CMS, all management apps send a reboot message to smd and smd will
 *  do a proper shutdown of the system.
 */
void wrapperReset(void)
{
    vosLog_debug("Sending msg to smd requesting reboot/reset");
    
    if(strstr((const char *)PFM_PRODUCT_NAME, "96838") != 0)
    {
        sleep(5);
        HAL_Reboot_System();
    }
    else
    {
        UTIL_reboot(g_msgHandle);
    }
    
    return;         
}


/** Ask MDM to flush config to flash.
*/
void wrapperSaveConfigurations(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    /*
    * This function really should be called inside the lock that
    * was acquired by runRPC.  But I don't know the workings of the
    * tr69c state machine (yet) to make that happen.  For now, acquire
    * the lock again and write the config out.  The downside of this
    * approach is that some other management app may have done another
    * write after our write (highly unlikely).
    */
    if ((ret = CMC_sysSaveConfig()) != VOS_RET_SUCCESS)
    {
        vosLog_error("saveConfigToFlash failed, ret=%d", ret);
    }
    else
    {
        vosLog_debug("config saved to flash");
    }

    return;
}

#define UPGRADE_SERVER_NAME "/tmp/upgrade_socket_server"
/*set itms flag*/
void setITMSUpdateFlag(SINT32 updateflag)
{
    FILE *fp = NULL;
    int sockfd;
    struct sockaddr_un address;
    int result;
    char chUpgrade = '0';

    if((fp = fopen("/var/itmsupgrade", "w")) != NULL)
    {
        fprintf(fp, "upgrade=%d\n", updateflag);
        fclose(fp);
    }

    //create socket
    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    address.sun_family = AF_UNIX;
    UTIL_STRNCPY(address.sun_path, UPGRADE_SERVER_NAME, sizeof(address.sun_path) - 1);
    //connect to server
    result = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    if (result == -1)
    {
        vosLog_error("Fail to send upgrade msg to HTTPD.\n");
        return;
    }
    else
    {
        switch(updateflag)
        {
            case 0:
                chUpgrade = '0';
                break;
            case 1:
                chUpgrade = '1';
                break;
            case 2:
                chUpgrade = '2';
                break;
            default:
                chUpgrade = '0';
                break;
        }

        //send upgrade info to server socket
        write(sockfd, &chUpgrade, 1);
    }
    close(sockfd);
    return ;
}

void sendUpgradePopMsgToSSK(UBOOL8 flag)
{
    VosMsgHeader *msg;
    void *msgBuf;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    
    msgBuf = VOS_MALLOC_FLAGS(sizeof(VosMsgHeader), ALLOC_ZEROIZE);
    msg = (VosMsgHeader *)msgBuf;

    msg->type = VOS_MSG_TR69_UPGRADE_POP;
    msg->src = EID_TR69C;
    msg->dst = EID_SSK;
    msg->flags_request = 1;
    msg->wordData = (UINT32)flag;

    ret = vosMsg_send(g_msgHandle, msg);
    if (TRUE == ret)
    {
        vosLog_debug("ssk UpgradePopMsg is OK");
    }
    else
    {
        vosLog_debug("ssk UpgradePopMsg is not OK");
    }
    
}

/*升级强制弹出提示处理*/
void upgradePopInfo(int itmsupgrage)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char outIpAddr[BUFLEN_16] = {0};
	char cmd[128] = {0};
    vosLog_debug("Need DNSspoof,itmsupgrage=%d", itmsupgrage);

    if (itmsupgrage == 1 || itmsupgrage == 2)
    {
        UTIL_SNPRINTF(cmd, sizeof(cmd), "-f 1 -c %s", DPROXY_FILE);
        UTIL_DO_SYSTEM_ACTION("ifconfig br0:1 192.168.1.249 up");
        (UINT32)UTIL_sendRequestToSmd(g_msgHandle, VOS_MSG_RESTART_APP, EID_DNSPROXY, cmd, util_strlen(cmd) + 1);

        ret = CMC_lanGetIntfInfo(FALSE, "br0", outIpAddr, NULL, NULL, sizeof(outIpAddr));
        if(ret == VOS_RET_SUCCESS && outIpAddr[0] != '\0')
        {
            UTIL_DO_SYSTEM_ACTION("iptables -t nat  -D PREROUTING  -p tcp -d ! %s --dport 80 -j DNAT --to %s", outIpAddr, outIpAddr);
            UTIL_DO_SYSTEM_ACTION("iptables -t nat  -A PREROUTING  -p tcp -d ! %s --dport 80 -j DNAT --to %s", outIpAddr, outIpAddr);      
        }
        else
        {
            UTIL_DO_SYSTEM_ACTION("iptables -t nat  -D PREROUTING  -p tcp -d ! 192.168.1.1 --dport 80 -j DNAT --to 192.168.1.1");
            UTIL_DO_SYSTEM_ACTION("iptables -t nat  -A PREROUTING  -p tcp -d ! 192.168.1.1 --dport 80 -j DNAT --to 192.168.1.1");
        }

        if(SF_FEATURE_SUPPORT_CT_USERINFO && SF_FEATURE_LOCATION_GUANGDONG)
        {
            sendUpgradePopMsgToSSK(TRUE);
        }

        HAL_sysEnableFlowcache(0);
    }
    else if (0 == itmsupgrage)
    {
        UTIL_SNPRINTF(cmd, sizeof(cmd), "-f 0 -c %s", DPROXY_FILE);
        UTIL_DO_SYSTEM_ACTION("ifconfig br0:1 down");
        (UINT32)UTIL_sendRequestToSmd(g_msgHandle, VOS_MSG_RESTART_APP, EID_DNSPROXY, cmd, util_strlen(cmd) + 1);

        ret = CMC_lanGetIntfInfo(FALSE, "br0", outIpAddr, NULL, NULL, sizeof(outIpAddr));
        if(ret == VOS_RET_SUCCESS && outIpAddr[0] != '\0')
        {
            UTIL_DO_SYSTEM_ACTION("iptables -t nat  -D PREROUTING  -p tcp -d ! %s --dport 80 -j DNAT --to %s", outIpAddr, outIpAddr);    
        }
        else
        {
            UTIL_DO_SYSTEM_ACTION("iptables -t nat  -D PREROUTING  -p tcp -d ! 192.168.1.1 --dport 80 -j DNAT --to 192.168.1.1");
        }

        if(SF_FEATURE_SUPPORT_CT_USERINFO && SF_FEATURE_LOCATION_GUANGDONG)
        {
            sendUpgradePopMsgToSSK(FALSE);
        }

        HAL_sysEnableFlowcache(1);
    }
}

/* downloadComplete()
*  Called when image has been downloaded. If successful the *buf will point to the
*  image buffer. If *buf is NULL the download failed.
*/

VOS_RET_E downloadComplete(DownloadReq *r, char *buf)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    int updateflag = 2;
    
    vosLog_debug("r->efileType=%d r->fileSize=%d\n", r->efileType, r->fileSize);
    if ((eVendorConfig == r->efileType) && (0 == r->fileSize))
    {
        if (SF_FEATURE_SUPPORT_SYSLOG)
        {
            syslog(LOG_WARNING, "104057 invalid file format(empty configure file)");
        }
    }
    
    if (buf) 
    {
        /*invalid file, it's too small*/
        if (r->fileSize < 64)
        {
            vosLog_error("downloadComplete -- invalid image format, too small");
            downloadStop("Invalid image format", 9010);

            if (SF_FEATURE_SUPPORT_SYSLOG)
            {
                if (eVendorConfig == r->efileType)
                {
                    syslog(LOG_WARNING, "104054 Configuration file is not available");
                }
                else
                {
                    syslog(LOG_WARNING, "104056 Invalid file format, too small");
                }
            }
                
            ret = VOS_RET_INVALID_IMAGE;
        }

        downloadStop_nosendinform("Download finished, write flash failed",9010);
        tr69SaveTransferList();

        if (eVendorConfig == r->efileType)
        {
            tr69SaveConfigFileInfo(r);
        }
            setITMSUpdateFlag(updateflag);
            upgradePopInfo(updateflag);
        vosLog_notice("downloadComplete -- save flash image");

        if (eFirmwareUpgrade == r->efileType)
        {
            vosLog_debug("eFirmwareUpgrade");
            ret = HAL_flashWriteImage(buf, (UINT32)r->fileSize);
            if(SF_FEATURE_ISP_CT)
            {
                CMC_wanSetIpProtocolVersion(3);
                wrapperSaveConfigurations();
                sleep(15);
            }

            vosLog_debug("ret = %d", ret);
        }
        else 
        {
            ret = CMC_sysWriteBufToConfig(buf, (UINT32)r->fileSize);
        }

        if (ret != VOS_RET_SUCCESS)
        {
            vosLog_error("write of validated image failed, ret=%d", ret);
        }
        else
        {
            /*
            * In the modem, cmsImg_writeValidatedImage() will trigger a 
            * reboot.  On the desktop, we will still be here.
            */
            vosLog_debug("image written");
            downloadStop_nosendinform("Download successful", 0);
            tr69SaveTransferList();

            if (eFirmwareUpgrade == r->efileType && SF_FEATURE_SUPPORT_PLUGIN)
            {
                acsState.upgradeDownloadFlag = 0;
                saveTR69StatusItems();
            }

            if (!SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
				if(strstr((const char *)PFM_PRODUCT_NAME, "96838") != 0)
				{
					sleep(5);
					HAL_Reboot_System();
				}
				else
				{
                	UTIL_reboot(g_msgHandle);
				}
            }
            else
            {
                if (enblCTMiddleware != CTMDW_MODE_0)
                {
					//if(memcmp("18.96838GWOVS", (unsigned char *)PFM_PRODUCT_NAME, sizeof(PFM_PRODUCT_NAME)) == 0)
					if(strstr((const char *)PFM_PRODUCT_NAME, "96838") != 0)
					{
                        sleep(5);
						HAL_Reboot_System();
					}
					else
					{
	                	UTIL_reboot(g_msgHandle);
					}
                }
            }
        }
    }
    else
    {
        vosLog_debug("no buf to check or flash");
        ret = VOS_RET_INTERNAL_ERROR;
    }

    if (SF_FEATURE_SUPPORT_SYSLOG)
    {
        util_saveLogToFlash(g_msgHandle);
    }

    return ret;
}


VOS_RET_E downloaddiagComplete(int size, char *buf)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (buf)
    {
        //downloadDiagStop_nosendinform("Download successful", 0);
        tr69SaveTransferList();
        CMC_tr69cSetDownloadDiagState(COMPLETED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        downloadDiagStop("Download successful", 0);
    }
    else
    {
        CMC_tr69cSetDownloadDiagState(NORESPONSE);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        vosLog_debug("no buf to check or flash");
        ret = VOS_RET_INTERNAL_ERROR;
    }

    return ret;
}


/* Returns state of WAN interface to be used by tr69 client */
eWanState getWanState(void)
{
    /* add BCM shared library call here to determine status of outbound I/F*/
    /* mwang: even in the released 3.0.8 code, this function just returns
    * eWAN_ACTIVE. So in most places in the code, we will continue to use this
    * until we understand the implication of fixing it or removing it.
    * In the mean time, there is also a getRealWanState function below. */
    return eWAN_ACTIVE;
}

eWanState getRealWanState(const char *ifName)
{
    VosMsgHeader *msg;
    char *data;
    void *msgBuf;
    UINT32 msgDataLen = 0;
    eWanState realWanState;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (ifName != NULL)
    {
        msgDataLen = (UINT32)util_strlen(ifName) + 1;
    }

    msgBuf = VOS_MALLOC_FLAGS(sizeof(VosMsgHeader) + msgDataLen, ALLOC_ZEROIZE);
    msg = (VosMsgHeader *)msgBuf;

    msg->type = VOS_MSG_GET_WAN_CONN_STATUS;
    msg->src = EID_TR69C;
    msg->dst = EID_SSK;
    msg->flags_request = 1;

    if (ifName != NULL)
    {
        data = (char *) (msg + 1);
        msg->dataLength = msgDataLen;
        UTIL_STRNCPY(data, ifName, msgDataLen);
    }

   /*
    * ssk will reply with TRUE or FALSE for wan connection up.
    * Need to check for that instead of the usual VOS_RET_E enum.
    */
    ret = vosMsg_sendAndGetReply(g_msgHandle, msg);
    if (TRUE == ret)
    {
        vosLog_debug("ssk says boundIfName=%s is up", acsState.boundIfName);
        realWanState = eWAN_ACTIVE;
    }
    else
    {
        vosLog_debug("ssk says boundIfName=%s is not up", acsState.boundIfName);
        realWanState = eWAN_INACTIVE;
    }

    VOS_FREE(msgBuf);

    return realWanState;
}


void  addToParamPathList(char * path,void **list,char * value )
{
    ParamPathList *item = NULL;
    ParamPathList *temp = NULL;
    item = (ParamPathList*)(*list);
    
    temp = item;

    if (item)
    {
        ParamPathList *ppl = ((ParamPathList *)VOS_MALLOC_FLAGS(sizeof(ParamPathList), ALLOC_ZEROIZE));
        ppl->paramPath= VOS_STRDUP(path);
        if (value)
        {
            ppl->value= VOS_STRDUP(value);
        }
        
        while(temp->next)
        {
            temp = temp->next;
        }
        
        temp->next =ppl;
    }
    else
    {
        ParamPathList *ppl = ((ParamPathList *)VOS_MALLOC_FLAGS(sizeof(ParamPathList), ALLOC_ZEROIZE));
        ppl->paramPath = VOS_STRDUP(path);
        if (value)
        {
            ppl->value = VOS_STRDUP(value); 
        }
    
        ppl->next = NULL;
        item = ppl;
    }
    
    *list = item;
}

void freeParamPathList(void **aitem)
{  
    ParamPathList *item = NULL;
    ParamPathList *p = NULL;
    
    if (! *aitem) 
        return;
    
    item = (ParamPathList*)(*aitem);
       
    while(item)
    {
        VOS_FREE(item->paramPath);
        VOS_FREE(item->value);
        p = item->next;
        VOS_FREE(item);
        item =p;
    }
    
    VOS_FREE(item);
    *aitem = item;
}  

UINT32 getMdmParamValueChanges(void)
{
    UINT32 count = 0;

    CMC_phlGetNumOfParamValueChanged(&count);

    return count;
}

VOS_RET_E tr69SaveTransferList(void)
{
    UINT16 saveSz = 0, saveEntryCount = 0, i = 0, j = 0;
    DownloadReqInfo *pSaveList, *pSaved;
    DownloadReq *q;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    for (i = 0; i < TRANSFER_QUEUE_SIZE; i++)
    {
        q = &transferList.queue[i].request;
        if (eTransferNotYetStarted == q->state)
        {
            saveEntryCount++;
        }
    } /* for */ 

    vosLog_debug("saveEntryCount=%d", saveEntryCount);
    if (saveEntryCount > 0)
    {
        saveSz = (UINT16)sizeof(DownloadReqInfo) * saveEntryCount;

        pSaveList = (DownloadReqInfo *)VOS_MALLOC_FLAGS(saveSz, ALLOC_ZEROIZE);
        if (NULL == pSaveList)
        {
            return VOS_RET_RESOURCE_EXCEEDED;
        }

        for (i = 0, j = 0; i < TRANSFER_QUEUE_SIZE && j < saveEntryCount; i++)
        {
            q = &transferList.queue[i].request;
            pSaved = &pSaveList[j];
            if (eTransferNotYetStarted == q->state) 
            {
                if (q->commandKey)
                    UTIL_STRNCPY(pSaved->commandKey,(q->commandKey), sizeof(pSaved->commandKey));

                if (q->url)
                    UTIL_STRNCPY(pSaved->url,(q->url), sizeof(pSaved->url));

                if (q->user)
                    UTIL_STRNCPY(pSaved->user, q->user, sizeof(pSaved->user));

                if (q->pwd)
                    UTIL_STRNCPY(pSaved->pwd, q->pwd, sizeof(pSaved->pwd));
                
                if (q->fileName)
                    UTIL_STRNCPY(pSaved->fileName, q->fileName, sizeof(pSaved->fileName));

                pSaved->efileType = q->efileType;
                pSaved->fileSize=q->fileSize;
                pSaved->delaySec = q->delaySec;
                pSaved->state = q->state;
                pSaved->rpcMethod = transferList.queue[i].rpcMethod;

                vosLog_debug("tr69SaveTransferList(j %d): saving efileType %d,commandKey %s, url %s, user %s, pwd %s, filesize %d, filename %s, delaySec %d, state %d, rpcMethod %d\n",
                j,pSaved->efileType,pSaved->commandKey,pSaved->url,pSaved->user,
                pSaved->pwd,pSaved->fileSize,pSaved->fileName,pSaved->delaySec,pSaved->state,pSaved->rpcMethod);
                j++;
            } /* notyetStarted */        
        } /* for */

        if ((ret = HAL_sysSetTr69cData("tr69c_transfer", pSaveList, saveSz)) != VOS_RET_SUCCESS)
        {
            vosLog_error("Unable to save transferList in scratch PAD");
        }

        VOS_FREE(pSaveList);
    } /* saveEntryCount */

    return ret;
}

VOS_RET_E tr69RetrieveTransferListFromStore(DownloadReqInfo *list, UINT16 *size)
{
    char buf[sizeof(DownloadReqInfo) * TRANSFER_QUEUE_SIZE] = {0};
    UINT32 bufSz = sizeof(DownloadReqInfo) * TRANSFER_QUEUE_SIZE;
    VOS_RET_E ret = VOS_RET_INVALID_ARGUMENTS;

    HAL_sysGetTr69cData("tr69c_transfer", buf, &bufSz);

    if (0 == bufSz)
    {
        /* could not find tr69c_transfer in scratch pad or other error */
        vosLog_debug("could not find tr69c_transfer in scratch pad");
        *size = 0;
    }
    else
    {
        vosLog_debug("read %d bytes from scratch pad for tr69c_transfer", bufSz);
        *size = (UINT16) bufSz;
        memcpy((void*)list, buf, *size);
        ret = VOS_RET_SUCCESS;
    }
   
    return ret;
}

VOS_RET_E tr69SaveConfigFileInfo(DownloadReq *r)
{
    UINT16 saveSz = 0;
    DownloadVendorConfigInfo vendorConfig;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char *namePtr;
   
    saveSz = sizeof(DownloadVendorConfigInfo);
    memset(&vendorConfig,0,saveSz);

    utilTms_getXSIDateTime(0, vendorConfig.date, sizeof(vendorConfig.date));
    if ((r->fileName) && (r->fileName[0] != '\0'))
    {
        UTIL_STRNCPY(vendorConfig.name, r->fileName, sizeof(vendorConfig.name));
    }
    else
    {
        /* retrieve file name from URL such as: http://220.128.128.236/ACS/Save/pd128_fw/dev4.0sbcConf.xml */
        namePtr = strrchr(r->url, '/');
        if (namePtr != NULL)
        {
            UTIL_STRNCPY(vendorConfig.name, (namePtr + 1), sizeof(vendorConfig.name));
        }
    }

    if ((ret = HAL_sysSetTr69cData(VENDOR_CFG_INFO_TOKEN, &vendorConfig, saveSz)) != VOS_RET_SUCCESS)
    {
        vosLog_error("Unable to save vendorConfig info in scratch PAD");
    }

    return ret;
} /* tr69SaveConfigFileInfo */


void setVendorConfigObj(DownloadVendorConfigInfo *vendorConfig)
{
    vendorConfigUpdateMsgBody *pData;
    int msgDataLen = sizeof(vendorConfigUpdateMsgBody);
    VosMsgHeader *msg = NULL;
    char *msgBuf;

    /* send a message to SSK and have it edit the vendorConfigFile
    * table.   This is a dynamic instance.  TR69 and other applications
    * are not allowed to update the object. 
    */
    msgBuf = (char *)VOS_MALLOC_FLAGS(sizeof(VosMsgHeader) + msgDataLen, ALLOC_ZEROIZE);

    msg = (VosMsgHeader *)msgBuf;
    pData = (vendorConfigUpdateMsgBody *) (msg + 1);

    msg->type = VOS_MSG_VENDOR_CONFIG_UPDATE;
    msg->src = EID_TR69C;
    msg->dst = EID_SSK;
    msg->flags_request = 1;
    msg->dataLength = (UINT32)msgDataLen;
    UTIL_STRNCPY(pData->date, vendorConfig->date, sizeof(pData->date));
    
    if (vendorConfig->name[0] != '\0')
    {
        UTIL_STRNCPY(pData->name, vendorConfig->name, sizeof(pData->name));
    }

    if (vendorConfig->version[0] != '\0')
    {
        UTIL_STRNCPY(pData->version, vendorConfig->version, sizeof(pData->version));
    }

    if (vendorConfig->version[0] != '\0')
    {
        UTIL_STRNCPY(pData->name, vendorConfig->name, sizeof(pData->name));
    }

    if (vendorConfig->description[0] != '\0')
    {
        UTIL_STRNCPY(pData->description, vendorConfig->description, sizeof(pData->description));
    }

    if ((vosMsg_sendAndGetReply(g_msgHandle, msg)) != VOS_RET_SUCCESS)
    {
        vosLog_error("could not send VOS_MSG_VENDOR_CONFIG_UPDATE message to SSK");
    }
} /* setVendorConfigObj */


void retrieveClearTR69VendorConfigInfo(void)
{
    DownloadVendorConfigInfo *vendorConfig;
    UINT32 count = 0;
    vosLog_debug("entered");

    if ((vendorConfig = (DownloadVendorConfigInfo*)
        VOS_MALLOC_FLAGS(sizeof(DownloadVendorConfigInfo), ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("malloc failed");
        return;
    }

    count = sizeof(DownloadVendorConfigInfo);
    HAL_sysGetTr69cData(VENDOR_CFG_INFO_TOKEN, vendorConfig, &count);
    if (0 == count)
    {
        vosLog_debug("No existing VENDOR_CFG_INFO_TOKEN info found in scratch pad.");
        VOS_FREE(vendorConfig);
        return;
    }
    else if (count != sizeof(DownloadVendorConfigInfo))
    {
        vosLog_error("error while reading vendor config data from scratch pad, count=%d", count);
        VOS_FREE(vendorConfig);
        return;
    }
    
    vosLog_debug("pVendorConfig->name %s, date %s",vendorConfig->name,vendorConfig->date);

    setVendorConfigObj(vendorConfig);
    VOS_FREE(vendorConfig);

    /* we only record the config file once */
    if (HAL_sysSetTr69cData(VENDOR_CFG_INFO_TOKEN,NULL,0) != VOS_RET_SUCCESS)
    {
        vosLog_error("Unable to save VENDOR_CFG_INFO_TOKEN in scratch PAD");
    }
} /* retrieveTR69VendorConfigInfo */

