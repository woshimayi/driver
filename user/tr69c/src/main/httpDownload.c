/*----------------------------------------------------------------------*
<:copyright-broadcom 
 
 Copyright (c) 2005 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
 *----------------------------------------------------------------------*
 * File Name  : httpDownload.c
 *
 * Description: download functions 
 * $Revision: 1.16 $
 * $Id: httpDownload.c,v 1.16 2006/02/17 21:15:09 dmounday Exp $
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

#include "cmc_api.h"
#include "../bcmLibIF/bcmWrapper.h"
#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include "../SOAPParser/RPCState.h"
#include "event.h"
#include "../webproto/protocol.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../main/informer.h"
#include "../main/httpProto.h"
#include "../SOAPParser/xmlTables.h"
#include "../SOAPParser/RPCState.h"
#include "hal_util_flash.h"
#include "../inc/tr69cdefs.h" /* defines for ACS state */
#include "aes.h"

#include "../ctMiddleware/ctMiddleware.h"


extern CTMDW_DOWNLOAD_STATE g_downloadState;
extern void ctmdw_sendUpgradeRet();

extern ACSState    acsState;
extern void *g_msgHandle;

/* interface name from socket used for image uploading */
char connIfName[UTIL_IFNAME_LENGTH]={0};
/*
  * display SOAP messages on serial console.
  * This flag is initialize, enabled or disabled in main.c,
  * and perform action in protocol.c
  */
extern UBOOL8   loggingSOAP; 

static SessionAuth getSessionAuth;
HttpTask    httpDownload;

static void downloadConnected(void *handle);


void downloadStop_nosendinform(char *msg, int status)
{
    vosLog_debug("msg = %p, status = %d", msg, status);
    
    if (httpDownload.wio)
    {
        wget_Disconnect(httpDownload.wio);
        httpDownload.wio = NULL;
    }
    
    if (httpDownload.authHdr != NULL)
    {       
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.authHdr);
    } 
    
    if (httpDownload.postMsg != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.postMsg);
        httpDownload.postLth = 0;
    }
    
    httpDownload.xfrStatus  = eDownloadDone;
    httpDownload.eHttpState = eClosed;
    httpDownload.eAuthState = sIdle;   
    httpDownload.callback = NULL;   

    setInformState(eACSDownloadReboot);
    addInformEventToList(INFORM_EVENT_DOWNLOAD_METHOD);
    addInformEventToList(INFORM_EVENT_TRANSER_COMPLETE);

    acsState.fault = 0;
    acsState.dlFaultStatus = status; 

    if (msg == NULL)
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,"Download successful"); 
    }
    else
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,msg);
    }

    acsState.endDLTime = time(NULL);
    saveTR69StatusItems();
    updateTransferState(acsState.downloadCommandKey,eTransferCompleted);
}

void downloadDiagStop_nosendinform(char *msg, int status)
{
    vosLog_debug("msg = %p, status = %d", msg, status);
    
    if (httpDownload.wio)
    {
        wget_Disconnect(httpDownload.wio);
        httpDownload.wio = NULL;
    }
    
    if (httpDownload.authHdr != NULL)
    {       
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.authHdr);
    } 
    
    if (httpDownload.postMsg != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.postMsg);
        httpDownload.postLth = 0;
    }
    
    httpDownload.xfrStatus  = eDownloadDone;
    httpDownload.eHttpState = eClosed;
    httpDownload.eAuthState = sIdle;   
    httpDownload.callback = NULL;   

    acsState.fault = 0;
    acsState.dlFaultStatus = status; 

    if (msg == NULL)
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,"Download successful"); 
    }
    else
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,msg);
    }

    acsState.endDLTime = time(NULL);
    saveTR69StatusItems();
    updateTransferState(acsState.downloadCommandKey,eTransferCompleted);
}

void downloadStop(char *msg, int status)
{
    vosLog_debug("Enter>, msg = %p, status= %d", msg, status);
    
    if (httpDownload.wio)
    {
        wget_Disconnect(httpDownload.wio);
        httpDownload.wio = NULL;
    }
    
    if (httpDownload.authHdr != NULL)
    {       
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.authHdr);
    } 
    
    if (httpDownload.postMsg != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.postMsg);
        httpDownload.postLth = 0;
    }
    
    httpDownload.xfrStatus  = eDownloadDone;
    httpDownload.eHttpState = eClosed;
    httpDownload.eAuthState = sIdle;   
    httpDownload.callback = NULL;   

    setInformState(eACSDownloadReboot);
    addInformEventToList(INFORM_EVENT_DOWNLOAD_METHOD);
    addInformEventToList(INFORM_EVENT_TRANSER_COMPLETE);

    acsState.fault = 0;
    acsState.dlFaultStatus = status; 

    if (msg == NULL)
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,"Download successful"); 
    }
    else
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,msg);
    }

    acsState.endDLTime = time(NULL);
    saveTR69StatusItems();
    sendDownloadFault();
}

VOS_RET_E downloadCompleteInform(void)
{
    //inform for tr69c
    VosMsgHeader msg = EMPTY_MSG_HEADER;
        
    msg.type = VOS_MSG_EMLUATE_COMPLTETE_INFORM;
    msg.src = EID_TR69C;
    msg.dst = EID_TR69C;
    msg.flags_request = 1;

    vosLog_debug("send msg [0x%08x] to [%u] from [%u], msg->data=[%u]", msg.type, msg.dst, msg.src, msg.wordData);

    if (VOS_RET_SUCCESS != vosMsg_send(g_msgHandle, &msg))
    {
        vosLog_error("fail to set Eid(%d) loglevel, ret=%d", EID_TR69C);
    }

    return VOS_RET_SUCCESS;
}

void downloadDiagStop(char *msg, int status)
{
    vosLog_debug("Enter>, msg = %p, status= %d", msg, status);
    
    if (httpDownload.wio)
    {
        wget_Disconnect(httpDownload.wio);
        httpDownload.wio = NULL;
    }
    
    if (httpDownload.authHdr != NULL)
    {       
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.authHdr);
    } 
    
    if (httpDownload.postMsg != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpDownload.postMsg);
        httpDownload.postLth = 0;
    }
    
    httpDownload.xfrStatus  = eDownloadDone;
    httpDownload.eHttpState = eClosed;
    httpDownload.eAuthState = sIdle;   
    httpDownload.callback = NULL;   

    acsState.fault = 0;
    acsState.dlFaultStatus = status; 

    if (msg == NULL)
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,"Download successful"); 
    }
    else
    {
       VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg,msg);
    }

    acsState.endDLTime = time(NULL);
    saveTR69StatusItems();

    if (isAcsConnected())
    {
//        transferCompletePending = 1;
        vosLog_debug("acs is connected -- set transferCompletePending");
    }
    else
    {
        vosLog_debug("acs is not connected -- sendInform(TransferComplete)");
//        transferCompletePending = 1;
        downloadCompleteInform();
    }

    updateTransferState(acsState.downloadCommandKey, eTransferCompleted);
    
}

void updateDownLoadKey(DownloadReq *dlr)
{
    vosLog_debug("Enter>, dlr = %p", dlr);

    if (acsState.downloadCommandKey)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(acsState.downloadCommandKey);
    }

    acsState.downloadCommandKey = dlr->commandKey;
    dlr->commandKey = NULL; 
}


VOS_RET_E TR69_tarDevGetFwInfo(char *fullPath, UINT32 len, UINT32 *fileSize)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    FILE *fp = NULL;
    char buffer[BUFLEN_80] = {0};
    char fileName[BUFLEN_80] = {0};
    char suffixName[BUFLEN_80] = {0};
    char pathName[BUFLEN_80] = {0};
    char *tmp = NULL;
    
    vosLog_debug("Enterd.\n");

    *fileSize = 0;
    
    fp = fopen("/proc/tardev", "r");
    if (NULL == fp)
    {
        vosLog_error("Fail to open tardev!");
        return VOS_RET_OPEN_FILE_ERROR;
    }

    /*# cat /proc/tardev 
        Sevice Pack(sp)     : bcm18.96838GWOVS
        Filter Rule(rule)   : haha,hehe,houhou
        Save Path(savepath) : /tmp
        Driver Status       : idle
        Tar Format          : true
        Wrote Bytes         : 34611200

        Filter Files:
        -----------------------------
        1) bcm18.96838GWOVS.main
        2) bcm18.96838GWOVS.haha
        3) bcm18.96838GWOVS.hehe
        4) bcm18.96838GWOVS.houhou
        */

    while(fgets(buffer, BUFLEN_80, fp))
    {
        if(NULL != (tmp = strchr(buffer, '\r')))
        {
            *tmp = '\0';
        }
        if(NULL != (tmp = strchr(buffer, '\n')))
        {
            *tmp = '\0';
        }

        vosLog_debug("buffer:%s>>.", buffer);

        if (NULL != (tmp = util_strstr(buffer, "Sevice Pack(sp)")))
        {
            tmp += sizeof("Sevice Pack(sp)");
            while (*tmp == ' ' || *tmp == ':')
            {
                tmp++;
            }
            UTIL_STRNCPY(fileName, tmp, sizeof(fileName));
        }

        if (NULL != (tmp = util_strstr(buffer, "Filter Rule(rule)")))
        {
            tmp += sizeof("Filter Rule(rule)");
            while (*tmp == ' ' || *tmp == ':')
            {
                tmp++;
            }
            UTIL_STRNCPY(suffixName, tmp, sizeof(fileName));
        }

        if (NULL != (tmp = util_strstr(buffer, "Save Path(savepath)")))
        {
            tmp += sizeof("Save Path(savepath)");
            while (*tmp == ' ' || *tmp == ':')
            {
                tmp++;
            }
            UTIL_STRNCPY(pathName, tmp, sizeof(pathName));
        }
    }

    vosLog_debug("fileName:%s;\n suffixName:%s;\n patheName:%s;\n", fileName, suffixName, pathName);

    if ('\0' == suffixName[0])
    {
        UTIL_SNPRINTF(fullPath, len, "%s/%s.main", pathName, fileName);
    }
    else
    {
        //only support one suffixName, add later if nessary!
        UTIL_SNPRINTF(fullPath, len, "%s/%s.%s", pathName, fileName, suffixName);
    }

    vosLog_debug("fullPath:%s", fullPath);
    
    if (access(fullPath, 0) == 0)
    {
        struct stat statBuf;

        if (stat(fullPath, &statBuf) < 0)
        {
            vosLog_error("Fail to get file size!");
            return VOS_RET_INTERNAL_ERROR;
        }

        *fileSize = statBuf.st_size;
    }
    else
    {
        vosLog_error("file %s is not exist!", fullPath);
        ret = VOS_RET_INTERNAL_ERROR;
    }
    
    return ret;
    
}


VOS_RET_E TR69_tarDevProcessUpgrate(tWget *wg, DownloadReq *dlreq)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 tmpLen = 1024 * 1024; // 1M
    SINT32 readLen = 0;
    SINT32 remainLen = wg->hdrs->content_length;
    char *buf = NULL;
    char fwFileName[BUFLEN_128] = {0};
    FILE *fp = NULL;

    vosLog_debug("Enterd>>.\n");

    //first setep: create tmp upgrate file to /tmp dir
    fp = fopen("/dev/tardev", "w");
    if (NULL == fp)
    {
        vosLog_error("Fail to open tardev!");
        return VOS_RET_OPEN_FILE_ERROR;
    }

    if(remainLen >0)
    {
        if(SF_FEATURE_LOCATION_JIANGXI || SF_FEATURE_LOCATION_GUANGDONG || SF_FEATURE_LOCATION_FUJIAN)
        {
            setITMSUpdateFlag(1);
            upgradePopInfo(1);	
            printf("*****Downloading Image.......*******\n");
        }
    }
    while (remainLen > 0)
    {
        if(remainLen >= tmpLen)
        {
            buf = readResponse(wg, &readLen, tmpLen);
        }
        else
        {
            buf = readResponse(wg, &readLen, remainLen);
        }
        
        vosLog_debug("buf:%p;tmpLen:%u;readLen:%u;remainLen:%d", buf, tmpLen,readLen, remainLen);

        if (NULL == buf)
        {
            vosLog_error("Fail to read msg");
            break;
        }

        if (fwrite(buf, 1, readLen, fp) != readLen)
        {
            vosLog_error("Fail to write tar dev");
            VOS_MEM_FREE_BUF_AND_NULL_PTR(buf);
            break;
        }
		
        VOS_MEM_FREE_BUF_AND_NULL_PTR(buf);

        remainLen -= readLen;
    }

    fclose(fp); //close fd.

    //second step, map file to memory and write upgrate fw to flash
    tmpLen = 0;
    if (VOS_RET_SUCCESS == TR69_tarDevGetFwInfo(fwFileName, sizeof(fwFileName), &tmpLen))
    {
        vosLog_debug("fileSize:%u.", tmpLen);
        buf = (char *)vomMem_mapFile2Mem(fwFileName, tmpLen);
        if (NULL != buf);
        {
            //modify size for write to flash;
            dlreq->fileSize = tmpLen;
            
            ret = downloadComplete(dlreq, buf);
        }
    }
    
    return ret;
}


VOS_RET_E TR69_commonProcessUpgrate(tWget *wg, DownloadReq *dlreq)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    SINT32 mlth = 0;
    char *rambuf = NULL;
    
    vosLog_debug("Enterd\n");
    
    rambuf = readResponse(wg, &mlth, 0);
    if ((0 == mlth)) 
    {
        /* if control falls thru to here send a fault status to ACS */
        vosLog_error("Download from failed.\n");
        downloadStop("Unable to connect to download server", 9010);
    } 
    else if ((rambuf != NULL )&& mlth)
    {
        dlreq->fileSize = mlth;
                
        if (SF_FEATURE_ISP_CU || SF_FEATURE_UPLINK_TYPE_EOC)
        {
            ret = downloadComplete(dlreq, rambuf);
        }
        else
        {
            UINT32 nEncodeSize = 0;
            aes_context handler;
            unsigned char szKey[32];
            unsigned char szNor_iv[16];
            unsigned char *pEncodeBuf = NULL;                
            const char *dslCpeConfig = "</DslCpeConfig>";
            const char *itfGatewayCfg = "</InternetGatewayDevice>";
            UINT32 i = 0;
            UINT32 len = 0;
            UINT32 len1 = 0;
            UBOOL8 likely = FALSE;
                   
            len = (UINT32)util_strlen(itfGatewayCfg);
            len1 = (UINT32)util_strlen(dslCpeConfig);
            if (dlreq->efileType == eVendorConfig && dlreq->fileSize > 0)
            {
                nEncodeSize = (dlreq->fileSize % 16) > 0 ? (dlreq->fileSize / 16 + 1) * 16 : dlreq->fileSize;
                pEncodeBuf = (unsigned char*)VOS_MALLOC_FLAGS(nEncodeSize, ALLOC_ZEROIZE);//new unsigned char[nEncodeSize];
                memset(pEncodeBuf,0,dlreq->fileSize+16);
        
                if (pEncodeBuf == NULL)
                {
                    vosLog_error("pEncodeBuf = new char[%d], alloc memory failed\n", dlreq->fileSize);
                }
                else
                {                 
                    memset(szKey, 0, sizeof(szKey));
                    memcpy(szKey, KEY_256, util_strlen(KEY_256) > sizeof(szKey) ? sizeof(szKey) : util_strlen(KEY_256));
                    memset(szNor_iv, 0, sizeof(szNor_iv));
                    memcpy(szNor_iv, NOR_IV, util_strlen(NOR_IV) > sizeof(szNor_iv) ? sizeof(szNor_iv) : util_strlen(NOR_IV));
        
                    aes_set_key(&handler, szKey, 256);
                    aes_cbc_decrypt(&handler, szNor_iv, (unsigned char *)rambuf, pEncodeBuf, nEncodeSize);
        
                    for (i = dlreq->fileSize - 50; i < dlreq->fileSize && !likely; i++)
                    {
                        if (util_strncmp((const char *)&(pEncodeBuf[i]), itfGatewayCfg, len) == 0)
                        {
                            likely = TRUE;
                            UTIL_SNPRINTF((char *)&(pEncodeBuf[i + len + 1]), len1 +1, dslCpeConfig);
                            pEncodeBuf[dlreq->fileSize - 2] = '\n';
                            pEncodeBuf[dlreq->fileSize - 1] = '\0';
                        }
                    }

                    ret = downloadComplete(dlreq, (char *)pEncodeBuf);
                }
            }
            else
            {
                ret = downloadComplete(dlreq, rambuf);
            }
        }
                  
        VOS_MEM_FREE_BUF_AND_NULL_PTR(rambuf);
    }

    return ret;
}


static void __downloadGetData(void *handle)
{
    vosLog_debug("Enter>, handle = %p", handle);

    int status = 9010;
    char *msg = "Download failed";
    tWget *wg = (tWget *)handle;
    HttpTask *ht = &httpDownload;
    RPCAction *action = (RPCAction *)ht->callback;
    DownloadReq *dlreq = &(action->ud.downloadReq);
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (wg->status == iWgetStatus_Ok)
    {
        if (wg->hdrs->status_code == 200 && 
        ((wg->hdrs->content_length > 0) ||
        (wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding, "chunked"))))
        {
            //int mlth = 0;
            //char *rambuf = NULL;
            UBOOL8 currLoggingSOAP = loggingSOAP;

            vosLog_debug("Starting image download from %s", dlreq->url);
            /* disable SOAP log so that image is not printed out at console */
            loggingSOAP = FALSE;

            /*
            * Notify smd that we are going to allocate a big buffer and
            * start downloading an image into it.  Smd will do the 
            * "killAllAps", or whatever it decides is best.
            */

            downloadStop("Download failed", 9010);

            if (SF_FEATURE_SUPPORT_TARDEV && (eFirmwareUpgrade == dlreq->efileType))
            {
                ret = TR69_tarDevProcessUpgrate(wg, dlreq);
            }
            else
            {
                ret = TR69_commonProcessUpgrate(wg, dlreq);
            }

            if (VOS_RET_SUCCESS != ret)
            {
                if (eFirmwareUpgrade == dlreq->efileType && SF_FEATURE_SUPPORT_PLUGIN)
                {
                    CMC_sendTr69cStartAlarmInfo(0, "104058");
                }

                vosLog_error("The image was bad.  Tell smd to resume normal operations again!");
                /*
                            * The image was bad.  Tell smd to resume normal operations
                            * again.
                            */
                if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                {
                   if (enblCTMiddleware == CTMDW_MODE_0)
                   {
                        g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FILE;
                        ctmdw_sendUpgradeRet();
                   }
                }

                /* reset SOAP log to current value */
                loggingSOAP = currLoggingSOAP;

                setITMSUpdateFlag(0);
                UTIL_doSystemAction("clear upgrade flag", "rm -f /var/itmsupgrade");
                upgradePopInfo(0);
            }

            if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                     g_downloadState = CTMDW_DOWNLOAD_STATE_OK;
                     ctmdw_sendUpgradeRet();
                }
            }

        }
        else if (wg->hdrs->status_code == 401)
        {
            if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                    g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT;
                    ctmdw_sendUpgradeRet();
                }
            }

            vosLog_error("File transfer server authentication failure from %s failed. Status = %d", dlreq->url, wg->hdrs->status_code);
            downloadStop("File transfer server authentication failure", 9012);
        }
        else if (wg->hdrs->status_code == 404)
        {
            if(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                    g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FILE;
                    ctmdw_sendUpgradeRet();
                }
            }

            vosLog_error("Download file cannot be found from %s failed. Status = %d", dlreq->url, wg->hdrs->status_code);
            downloadStop("Download file cannot be found", 9010);
        }
        else if (wg->hdrs->status_code >= 100 && wg->hdrs->status_code < 200)
        {
            return; /* ignore these status codes */
        }
        else
        {
            if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                    g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                    ctmdw_sendUpgradeRet();
                }
            }

            if (SF_FEATURE_SUPPORT_SYSLOG)
            {
                if(wg->hdrs->status_code == 200)
                {
                    if(dlreq->efileType == eVendorConfig)
                    {
                        syslog(LOG_WARNING, "104054 Configuration file is not available");
                    }
                    else
                    {
                        syslog(LOG_WARNING, "104054 Invalid file format(empty configure file)");
                    }
                    vosLog_error("Download file is empty!");
                    util_saveLogToFlash(g_msgHandle);

                    /*waiting the security log be flushed one second is not enough*/
                    sleep(3);
                }
            }
            vosLog_error("Download from %s failed. Status = %d\n", dlreq->url, wg->hdrs->status_code);
            downloadStop(msg, status);
        }
    }
    else
    {
        if(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUpgradeRet();
            }
        }

        if (eFirmwareUpgrade == dlreq->efileType && SF_FEATURE_SUPPORT_PLUGIN)
        {
            CMC_sendTr69cStartAlarmInfo(0, "104058");
        }

        /* if control falls thru to here send a fault status to ACS */
        downloadStop("Unable to connect to download server", 9010);
    }
}


typedef void *(*ThreadHandler)(void *);

static void downloadGetData(void *handle)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, (ThreadHandler)&__downloadGetData, handle);
	pthread_join(thread_id, NULL);
    return;
}


static void* downloadAsynDiagFunc()
{
    int mlth = 0;
    char *rambuf = NULL;
    UBOOL8 currLoggingSOAP = loggingSOAP;
    HttpTask *ht = &httpDownload;
    tWgetInternal *data = ht->wio;    
    tWget wg;

    vosLog_debug("Enter>, data = %p", data);
    
    wg.pc = data->pc;
    wg.hdrs = data->hdrs;
     
    /* disable SOAP log so that image is not printed out at console */
    loggingSOAP = FALSE;

    /*
    * Notify smd that we are going to allocate a big buffer and
    * start downloading an image into it.  Smd will do the 
    * "killAllAps", or whatever it decides is best.
    */
    //downloadDiagStop("Download failed1", 9010);
    CMC_tr69cSetDownloadBOMTime();
    rambuf = readResponseDiag(&wg, &mlth, 0);
                
    if ((mlth == 0))
    {
        /* if control falls thru to here send a fault status to ACS */
        vosLog_error("Download from failed.\n");
        downloadDiagStop("Unable to connect to download server", 9010);
        CMC_tr69cSetDownloadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    } 
    else if ((rambuf != NULL )&& mlth)
    {
        VOS_RET_E ret = VOS_RET_SUCCESS;

        CMC_tr69cSetDownloadEOMTime();
        ret = downloaddiagComplete(mlth, rambuf);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(rambuf);
        if (ret != VOS_RET_SUCCESS)
        {
            /*
            * The image was bad.  Tell smd to resume normal operations
            * again.
            */
            /* reset SOAP log to current value */
            loggingSOAP = currLoggingSOAP;
        }

    }
    else
    {
        vosLog_error("Download from failed. \n");
        downloadDiagStop("Download failed", 9010);
        CMC_tr69cSetDownloadDiagState(TRANSFER_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    }
               
    if (data->keepConnection == eCloseConnection)
    {
        freeData(data);
    }

    return NULL;
}


void downloadAsynDiagGetData(void)
{
    pthread_t thread_id; 
    pthread_attr_t attr; 
    pthread_attr_init(&attr);   
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread_id, &attr, downloadAsynDiagFunc, NULL); 
    pthread_attr_destroy(&attr); 
}


static void downloaddiagGetData(void *handle)
{
    vosLog_debug("==Enter==");
    int status = 9010;
    char *msg = "Download failed";
    tWget *wg = (tWget *)handle;
    HttpTask *ht = &httpDownload;
    CMC_TR69C_DOWNLOAD_DIAG_CFG_T *action = (CMC_TR69C_DOWNLOAD_DIAG_CFG_T *)ht->callback;

    if (wg->status == iWgetStatus_Ok)
    {
        if (wg->hdrs->status_code == 200 && 
        ((wg->hdrs->content_length > 0) ||
        (wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding,"chunked"))))
        {
            ht->wio->status = -9;        
            downloadAsynDiagGetData();
            stopListener(ht->wio->pc->fd);            
        }
        else if (wg->hdrs->status_code == 401)
        {
            vosLog_error("File transfer server authentication failure from %s failed. Status = %d", action->downloadurl, wg->hdrs->status_code);
            downloadDiagStop("File transfer server authentication failure", 9012);
            CMC_tr69cSetDownloadDiagState(LOGIN_FAILED);
            addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        }
        else if (wg->hdrs->status_code == 404)
        {
            vosLog_error("Download file cannot be found from %s failed. Status = %d", action->downloadurl, wg->hdrs->status_code);
            downloadDiagStop("Download file cannot be found", 9010);
            CMC_tr69cSetDownloadDiagState(NORESPONSE);
            addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        }
        else if (wg->hdrs->status_code >= 100 && wg->hdrs->status_code < 200)
        {
            return; /* ignore these status codes */
        }
        else
        {
            vosLog_error("Download from %s failed. Status = %d\n", action->downloadurl, wg->hdrs->status_code);
            downloadDiagStop(msg, status);
            CMC_tr69cSetDownloadDiagState(TRANSFER_FAILED);
            addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        }
    }
    else
    {
        /* if control falls thru to here send a fault status to ACS */
        downloadDiagStop("Unable to connect to download server", 9010);
        CMC_tr69cSetDownloadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    }
}


static void downloadGetAuth(void *handle)
{
    vosLog_debug("==Enter==");
    
    int status = 9010;
    char *msg = "Unable to connect to download URL";
    tWget *wg = (tWget *)handle;
    HttpTask *ht = &httpDownload;
    RPCAction *action = (RPCAction *)ht->callback;
    DownloadReq *dlreq = &(action->ud.downloadReq);
    SessionAuth *sa = &getSessionAuth;
        
    #ifdef DEBUG
    DBGPRINT((stderr, "downloadGetAuth(): starting authentication\n"));
    #endif
    
    if (wg->status == iWgetStatus_Ok)
    {
        if (wg->hdrs->status_code == 401)
        {
            int closeConn = 0;
            if (wg->hdrs->content_length > 0)
            {
                int mlth = 0;
                char *tmpBuf = readResponse(wg, &mlth, 0);
                if (tmpBuf != NULL)
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(tmpBuf);
            }
            ht->authHdr = generateAuthorizationHdrValue(sa, wg->hdrs->wwwAuthenticate,
                                                       "GET", ht->wio->uri, dlreq->user, dlreq->pwd);
            if (ht->authHdr == NULL)
            {
                if(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                {
                    if (enblCTMiddleware == CTMDW_MODE_0)
                    {
                        g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT;
                        ctmdw_sendUpgradeRet();
                    }
                }
                
                downloadStop("Download server has unknow authentication header format", status);
                return;
            }

            closeConn = ht->wio->hdrs->Connection && !strcasecmp(ht->wio->hdrs->Connection,"close");
            if (closeConn)
            {   
                /* end of data on 401 skip_Proto() */
                /* close connection and reconnect with Authorization header*/
                wget_Disconnect(ht->wio);
                ht->wio = wget_Connect(dlreq->url, downloadConnected, action);
                if (ht->wio == NULL) 
                {
                    downloadStop("Failed to connect download server", status);
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                    {
                        if (enblCTMiddleware == CTMDW_MODE_0)
                        {
                            g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                            ctmdw_sendUpgradeRet();
                        }
                    }
                } 
                else 
                {
                    wget_AddPostHdr(ht->wio,"Authorization", ht->authHdr);
                }
            }
            else
            {
                wget_AddPostHdr(ht->wio,"Authorization", ht->authHdr);
                // now just resend the last data with the Authorization header
                // HttpTask has to be the last argument in wget_GetData
                // since it is used later in do_send_request
                wget_GetData(ht->wio, downloadGetData, (void *)ht);
            }
        }
        else if (wg->hdrs->status_code == 404)
        {
            if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                    g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FILE;
                    ctmdw_sendUpgradeRet();
                }
            }

           downloadStop("Download file cannot be found", 9010);
        }
        else if (wg->hdrs->status_code >= 100 && wg->hdrs->status_code < 200)
        {
            return; /* ignore these status codes */
        }
        else if (200 == wg->hdrs->status_code)
        {
            vosLog_debug("wg->hdrs->status_code:%d", wg->hdrs->status_code);
            downloadGetData((void *)wg);
        }
        else
        {
            if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                   g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT;
                   ctmdw_sendUpgradeRet();
                }
            }
            
            downloadStop("Download server authentication failure", 9012);
        }
    }
    else
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUpgradeRet();
            }
        }

        downloadStop(msg, status);
    }
}

static int downloadIsAuthNeeded(RPCAction *action)
{
    vosLog_debug("==Enter==");
    
    int ret = TRUE;
    HttpTask *ht = &httpDownload;
    
    if (action->ud.downloadReq.user == NULL)
        ret = FALSE;
    else if (action->ud.downloadReq.user[0] == '\0')
        ret = FALSE;
    else if (action->ud.downloadReq.pwd == NULL)
        ret = FALSE;
    else if (action->ud.downloadReq.pwd[0] == '\0')
        ret = FALSE;

    // authHdr already has value by downloadGetAuth
    // so don't need to perform authentication again        
    if (ht->authHdr != NULL)
        ret = FALSE;

    return ret;
}

static void downloadConnected(void *handle) 
{
    vosLog_debug("==Enter==");
    
    tWget *wg = (tWget *)handle;
    RPCAction *action = (RPCAction *)wg->handle;
    HttpTask *ht = &httpDownload;
    
    if (wg->status != 0)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if(enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                ctmdw_sendUpgradeRet();
            } 
        }

        if (eFirmwareUpgrade == action->ud.downloadReq.efileType && SF_FEATURE_SUPPORT_PLUGIN)
        {
            CMC_sendTr69cStartAlarmInfo(0, "104058");
        }
        downloadStop("Unable to connect to download server", 9010);       
        return;      
    }
    
    // stick RPCAction to callback
    ht->callback = (CallBack)action;
    
    // HttpTask has to be the last argument in wget_GetData
    // since it is used later in do_send_request
    if (SF_FEATURE_ISP_CU && SF_FEATURE_LOCATION_HENAN)
    {
        wget_GetData(ht->wio, downloadGetData,(void *)ht);
    }
    else
    {
        if (downloadIsAuthNeeded(action) == FALSE)
        {    
            wget_GetData(ht->wio, downloadGetData,(void *)ht);
        }
        else
        {
            wget_GetData(ht->wio, downloadGetAuth,(void *)ht);
        }
    }
}



static void downloaddiagConnected(void *handle) 
{
    vosLog_debug("==Enter==");
    CMC_TR69C_DOWNLOAD_DIAG_CFG_T *action = (CMC_TR69C_DOWNLOAD_DIAG_CFG_T *)handle;
    HttpTask *ht = &httpDownload;

    CMC_tr69cSetDownloadResponseTime();

    // stick RPCAction to callback
    ht->callback = (CallBack)action;
    
    // HttpTask has to be the last argument in wget_GetData
    // since it is used later in do_send_request  
    CMC_tr69cSetDownloadROMTime();
    wget_GetData(ht->wio, downloaddiagGetData,(void *)ht);
}


/* this is called by the callback from the startTimer in doDownload. */
/* we have to use wget_Connect in case of authentication in which case */
/* we need to control the connection */
void downloadStart( void *handle) 
{
    vosLog_debug("==Enter==");
    
    RPCAction   *action = (RPCAction *)handle;
    HttpTask    *ht = &httpDownload;
    DownloadReq *dlreq = &action->ud.downloadReq;

    // update download command key to fix CSP# 41725
    updateDownLoadKey(dlreq);
    if (util_strstr(dlreq->url, "http:") == NULL &&
        util_strstr(dlreq->url, "https:") == NULL)
    {
        if(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUpgradeRet();
            }
        }

        if (eFirmwareUpgrade == dlreq->efileType && SF_FEATURE_SUPPORT_PLUGIN)
        {
            CMC_sendTr69cStartAlarmInfo(0, "104058");
        }
        downloadStop("Protocol not supported", 9013);
        return;
    }
    
    if (dlreq->efileType != eFirmwareUpgrade &&
        dlreq->efileType != eVendorConfig)
    {
        if(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUpgradeRet();
            }
        }

        downloadStop("Invalid download file type", 9010);       
        return;
    }
    
    memset(ht, 0, sizeof(struct HttpTask));
    memset(&getSessionAuth,0, sizeof(struct SessionAuth));
    
    ht->wio = wget_Connect(dlreq->url, downloadConnected, action); 
    if (ht->wio == NULL)
    {
        if(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUpgradeRet();
            }
        }

        if (eFirmwareUpgrade == dlreq->efileType && SF_FEATURE_SUPPORT_PLUGIN)
        {
            CMC_sendTr69cStartAlarmInfo(0, "104058");
        }
        vosLog_error("wget_Connect to download server failed: %s", wget_LastErrorMsg());
        downloadStop("Connect to download server failed", 9010);   
    }
    else
    {
        acsState.startDLTime = time(NULL);
        wget_ClearPostHdrs(ht->wio);
        if (ht->authHdr)
        {
            VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);
        }
    }
}


void downloaddiagStart( CMC_TR69C_DOWNLOAD_DIAG_CFG_T *handle)
{
    HttpTask    *ht = &httpDownload;

    vosLog_debug("Enter>");

    if (util_strstr(handle->downloadurl, "http:") == NULL)
    {
        downloadDiagStop("Protocol not supported", 9013);
        CMC_tr69cSetDownloadDiagState(NO_TRANSFER_MODE);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        return;
    }

    memset(ht, 0, sizeof(struct HttpTask));
    CMC_tr69cSetDownloadRequestTime();

    ht->wio = wget_DiagConnect(handle->downloadurl, downloaddiagConnected, handle); 
    if (ht->wio == NULL)
    {
        vosLog_error("wget_Connect to download server failed: %s", wget_LastErrorMsg());
        downloadDiagStop("Connect to download server failed", 9010);
        CMC_tr69cSetDownloadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    }
    else
    {
        wget_ClearPostHdrs(ht->wio);
        if (ht->authHdr)
        {
            VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);
        }
    }
}

