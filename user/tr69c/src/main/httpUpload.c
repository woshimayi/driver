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
 * File Name  : httpUpload.c
 *
 * Description: upload functions
 * $Revision: 1.16 $
 * $Id: httpUpload.c,v 1.16 2006/11/1 21:15:09 Peter Tran Exp $
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

#ifdef TESTBOX
    #include "../bcmLibIF/bcmTestWrapper.h"
#else
    #include "fwk.h"
    #include "vos_log.h"
    #include "../bcmLibIF/bcmWrapper.h"
#endif

#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include "../SOAPParser/RPCState.h"
#include "event.h"
#include "vos_mem.h"
#include "hal_util.h"

#include "../webproto/protocol.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../main/informer.h"
#include "../main/httpProto.h"
#include "../main/httpDownload.h"
#include "xml_parser_sm.h"
#include "../SOAPParser/xmlTables.h"
#include "../SOAPParser/RPCState.h"
#include "../inc/tr69cdefs.h" /* defines for ACS state */
#include "aes.h"
#include "../ctMiddleware/ctMiddleware.h"


extern CTMDW_DOWNLOAD_STATE g_downloadState;
extern void ctmdw_sendUploadRet();

extern void *g_msgHandle;


#ifdef DEBUG
    #define DBGPRINT(X) fprintf X
#else
    #define DBGPRINT(X)
#endif


extern ACSState     acsState;

static SessionAuth putSessionAuth;
HttpTask    httpUpload;

static void uploadConnected(void *handle);
static void uploadPutData(RPCAction *action);

extern int transferCompletePending;

void uploadStop(char *msg, int status)
{
    vosLog_debug("Enter>");

    if (httpUpload.wio)
    {
        wget_Disconnect(httpUpload.wio);
        httpUpload.wio = NULL;
    }

    if (httpUpload.authHdr != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpUpload.authHdr);
    }

    if (httpUpload.postMsg != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(httpUpload.postMsg);
        httpUpload.postLth = 0;
    }

    httpUpload.xfrStatus  = eUploadDone;
    httpUpload.eHttpState = eClosed;
    httpUpload.eAuthState = sIdle;
    httpUpload.callback = NULL;

    setInformState(eACSUpload);
    addInformEventToList(INFORM_EVENT_UPLOAD_METHOD);
    addInformEventToList(INFORM_EVENT_TRANSER_COMPLETE);

    acsState.fault = 0;
    acsState.dlFaultStatus = status;

    if (msg == NULL)
    {
        VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg, "Upload successful");
    }
    else
    {
        VOS_MEM_REPLACE_STRING(acsState.dlFaultMsg, msg);
    }

    acsState.endDLTime = time(NULL);
    saveTR69StatusItems();
    sendDownloadFault();
}

static void uploadCompletePut(void *handle)
{
    vosLog_debug("==Enter==");

    int status = 9011;
    char *msg = "Upload failed";
    tWget *wg = (tWget *)handle;

    if (wg->status == iWgetStatus_Ok)
    {
        switch (wg->hdrs->status_code)
        {
            case 401:
                status = 9012;
                msg = "File transfer server authentication failure";
                break;
            case 405:
            case 501:
                status = 9011;
                msg = "Upload server does not support PUT method";
                break;
            case 100:   // Continue status might be returned by Microsoft-IIS/5.1
            case 201:   // Created status is returned by Microsoft-IIS/5.1
            case 204:   // No content status is returned by Apache/2.2.2
            case 200:
                status = 0;
                msg = "Upload successful";
                break;
            default:
                status = 9011;
                msg = "Upload failed";
                break;
        }
    }

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        if (enblCTMiddleware == CTMDW_MODE_0)
        {
            if (0 == status)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_OK;
            }
            else if (9012 == status)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT;
            }
            else
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
            }

            ctmdw_sendUploadRet();
        }
    }

    uploadStop(msg, status);
}


static void uploaddiagCompletePut(void *handle)
{
    vosLog_debug("Enter>");

    int status = 9011;
    char *msg = "Upload failed";
    tWget *wg = (tWget *)handle;

    if (wg->status == iWgetStatus_Ok)
    {
        switch (wg->hdrs->status_code)
        {
            case 401:
                status = 9012;
                msg = "File transfer server authentication failure";
                break;
            case 405:
            case 501:
                status = 9011;
                msg = "Upload server does not support PUT method";
                break;
            case 100:   // Continue status might be returned by Microsoft-IIS/5.1
            case 201:   // Created status is returned by Microsoft-IIS/5.1
            case 204:   // No content status is returned by Apache/2.2.2
            case 200:
                status = 0;
                msg = "Upload successful";
                break;
            default:
                status = 9011;
                msg = "Upload failed";
                break;
        }
    }
    else
    {
        uploadStop("Unable to connect to upload server", 9011);
        CMC_tr69cSetUploadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        return;
    }

    uploadStop(msg, status);
}


static void uploadCompleteAuth(void *handle)
{
    vosLog_debug("==Enter==");

    int status = 9011;
    char *msg = "Unable to connect to upload URL";
    tWget *wg = (tWget *)handle;
    HttpTask *ht = &httpUpload;
    RPCAction *action = (RPCAction *)ht->callback;
    DownloadReq *ulreq = &(action->ud.downloadReq);
    SessionAuth *sa = &putSessionAuth;

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
                          "PUT", ht->wio->uri, ulreq->user, ulreq->pwd);
            if (ht->authHdr == NULL)
            {
                if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                {
                    if (enblCTMiddleware == CTMDW_MODE_0)
                    {
                        g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT;
                        ctmdw_sendUploadRet();
                    }
                }

                uploadStop("Upload server has unknow authentication header format", status);
                return;
            }

            closeConn = ht->wio->hdrs->Connection && !strcasecmp(ht->wio->hdrs->Connection, "close");
            /* end of data on 401 skip_Proto() */
            if (closeConn)
            {
                /* close connection and reconnect with Authorization header*/
                wget_Disconnect(ht->wio);
                ht->wio = wget_Connect(ulreq->url, uploadConnected, action);
                if (ht->wio == NULL)
                {
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                    {
                        if (enblCTMiddleware == CTMDW_MODE_0)
                        {
                            g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                            ctmdw_sendUploadRet();
                        }
                    }

                    uploadStop("Failed to connect upload server", status);
                }
                else
                {
                    wget_AddPostHdr(ht->wio, "Authorization", ht->authHdr);
                }
            }
            else
            {
                wget_AddPostHdr(ht->wio, "Authorization", ht->authHdr);
                /* now just resend the last data with the Authorization header */
                uploadPutData(action);
            }
        }
        else if (wg->hdrs->status_code == 200)
        {
            vosLog_debug("wg->hdrs->status_code:%d", wg->hdrs->status_code);
            uploadPutData(action);
        }
        else
        {
            if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            {
                if (enblCTMiddleware == CTMDW_MODE_0)
                {
                    g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_ACCOUNT;
                    ctmdw_sendUploadRet();
                }
            }

            uploadStop("Upload server authentication failure", 9012);
        }
    }
    else
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop(msg, status);
    }
}


static void uploadPutLog(void)
{
    int readptr = BCM_SYSLOG_FIRST_READ;
    char data[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char logHeader[BUFLEN_1024] = {0};
    char ipAddress[BUFLEN_64] = {0};
    VOS_RET_E   ret = VOS_RET_SUCCESS;
    CMC_SYS_DEVICE_INFO_T    deviceInfoObj;
    HttpTask *ht = &httpUpload;
    tProtoCtx *pc = ht->wio->pc;
    char dateLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char timeLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char hostLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char msgLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};

    vosLog_debug("Enter>");

    memset(&deviceInfoObj, 0, sizeof(deviceInfoObj));
    if (VOS_RET_SUCCESS != (ret = CMC_sysGetDeviceInfo(&deviceInfoObj)))
    {
        vosLog_error("Get device info fail! ret=%d", ret);
        return;
    }

    if ((ret = CMC_wanGetConnIpAddr(acsState.boundIfName, ipAddress, UTIL_WAN_IFNAME_MAX_LEN)) != VOS_RET_SUCCESS)
    {
        vosLog_error("Tr069 WAN intf is not up! ret=%d", ret);
        return;
    }

    UTIL_SNPRINTF(logHeader, sizeof(logHeader),
                  "Manufacturer:%s\r\nProductClass:%s\r\nSerialNumber:%s\r\nIP:%s\r\nHWVer:%s\r\nSWVer:%s\r\n\r\n",
                  deviceInfoObj.manufacturer, deviceInfoObj.productClass,
                  deviceInfoObj.serialNumber, ipAddress,
                  deviceInfoObj.hardwareVersion, deviceInfoObj.softwareVersion);

    proto_SendRaw(pc, logHeader, util_strlen(logHeader));

    if (SF_FEATURE_SUPPORT_SYSLOG)
    {
        /*try reading circular buffer*/
        while ((readptr != BCM_SYSLOG_READ_BUFFER_ERROR) && (readptr != BCM_SYSLOG_READ_BUFFER_END))
        {
            readptr = HAL_readLogPartialFlash(readptr, data);
            if ('\0' != data[0])
            {
                sscanf(data, "%s %s %s %[^\n]", dateLog, timeLog, hostLog, msgLog);
                memset(data, 0, sizeof(data));
                UTIL_SNPRINTF(data, BCM_SYSLOG_MAX_LINE_SIZE, "%s %s %s\r\n", dateLog, timeLog, msgLog);
            }

            proto_SendRaw(pc, data, util_strlen(data));
        }

        readptr = BCM_SYSLOG_FIRST_READ;
    }

    while ((readptr != BCM_SYSLOG_READ_BUFFER_ERROR) && (readptr != BCM_SYSLOG_READ_BUFFER_END))
    {
        readptr = HAL_readLogPartial(readptr, data);

        if ('\0' != data[0])
        {
            sscanf(data, "%s %s %s %[^\n]", dateLog, timeLog, hostLog, msgLog);
            memset(data, 0, sizeof(data));
            UTIL_SNPRINTF(data, BCM_SYSLOG_MAX_LINE_SIZE, "%s %s %s\r\n", dateLog, timeLog, msgLog);
        }

        proto_SendRaw(pc, data, strlen(data));
    }
}

static void uploadPutConfig(void)
{
    vosLog_debug("Enter>");

    UINT32 cnt = 0;
    UINT32 size = 0;
    char *data;
    HttpTask *ht = &httpUpload;
    tProtoCtx *pc = ht->wio->pc;
    UBOOL8 sendUploadStop = TRUE;
    VOS_RET_E ret;

    HAL_flashGetConfigSize(&size);

    if ((data = VOS_MALLOC_FLAGS(size, ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("could not allocate %u bytes for config buf", size);
    }
    else
    {
        if ((ret = HAL_flashReadConfig(data, &cnt)) != VOS_RET_SUCCESS)
        {
            vosLog_error("could not read config into buf, ret=%d", ret);
        }
        else
        {
            if (SF_FEATURE_UPLINK_TYPE_EOC || SF_FEATURE_ISP_CU)
            {
                if (cnt == (UINT32)ht->content_len)
                {
                    proto_SendRaw(pc, data, ht->content_len);
                    /* successful, so don't send uploadStop */
                    sendUploadStop = FALSE;
                }
            }
            else
            {
                if (cnt == (UINT32)ht->content_len)
                {
                    UINT32 nEncodeSize = 0;
                    aes_context handler;
                    unsigned char szKey[32];
                    unsigned char szNor_iv[16];
                    unsigned char *pEncodeBuf = NULL;

                    nEncodeSize = (cnt % 16) > 0 ? (cnt / 16 + 1) * 16 : cnt;
                    pEncodeBuf = (unsigned char *)VOS_MALLOC_FLAGS(nEncodeSize, ALLOC_ZEROIZE); //new unsigned char[nEncodeSize];
                    memset(pEncodeBuf, 0, (int)nEncodeSize);
                    if (pEncodeBuf == NULL)
                    {
                        vosLog_error("pEncodeBuf = new char[%d], alloc memory failed\n", nEncodeSize);
                    }
                    else
                    {
                        memset(szKey, 0, sizeof(szKey));
                        memcpy(szKey, KEY_256, util_strlen(KEY_256) > sizeof(szKey) ? sizeof(szKey) : util_strlen(KEY_256));
                        memset(szNor_iv, 0, sizeof(szNor_iv));
                        memcpy(szNor_iv, NOR_IV, util_strlen(NOR_IV) > sizeof(szNor_iv) ? sizeof(szNor_iv) : util_strlen(NOR_IV));

                        aes_set_key(&handler, szKey, 256);
                        aes_cbc_encrypt(&handler, szNor_iv, (unsigned char *)data, pEncodeBuf, nEncodeSize);
                        proto_SendRaw(pc, (const char *)pEncodeBuf, nEncodeSize);
                        /* successful, so don't send uploadStop */
                        sendUploadStop = FALSE;
                        VOS_MEM_FREE_BUF_AND_NULL_PTR(pEncodeBuf);
                    }
                }
            }
        }
    }

    if (sendUploadStop)
    {
        uploadStop("Invalid upload data", 9011);
    }

    if (data != NULL)
    {
        VOS_FREE(data);
    }
}


static void uploadDiagPutConfig(CMC_TR69C_UPLOAD_DIAG_CFG_T *action)
{
    vosLog_debug("Enter>");

    UINT32 cnt = 0;
    UINT32 size = 0;
    char *data;
    HttpTask *ht = &httpUpload;
    tProtoCtx *pc = ht->wio->pc;
    UBOOL8 sendUploadStop = TRUE;
    VOS_RET_E ret;

    HAL_flashGetConfigSize(&size);

    if ((data = VOS_MALLOC_FLAGS(size, ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("could not allocate %u bytes for config buf", size);
    }
    else
    {
        if ((ret = HAL_flashReadConfig(data, &cnt)) != VOS_RET_SUCCESS)
        {
            vosLog_error("could not read config into buf, ret=%d", ret);
        }
        else
        {
            CMC_tr69cSetUploadBOMTime();
            if (cnt == (UINT32)ht->content_len)
            {
                proto_SendRaw(pc, data, (int)action->testfilelength);
                /* successful, so don't send uploadStop */
                sendUploadStop = FALSE;
                CMC_tr69cSetTotalBytesSent((int)(action->testfilelength + 16));
            }
        }
    }

    if (sendUploadStop)
    {
        uploadStop("Invalid upload data", 9011);
        CMC_tr69cSetUploadDiagState(TRANSFER_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    }

    CMC_tr69cSetUploadEOMTime();

    CMC_tr69cSetUploadDiagState(COMPLETED);
    addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);

    if (data != NULL)
    {
        VOS_FREE(data);
    }
}


static UINT32 uploadGetLogSize(void)
{
    UINT32 cnt = 0;
    UINT32 test_head = 0;
    int readptr = BCM_SYSLOG_FIRST_READ;
    char buffer[BCM_SYSLOG_MAX_LINE_SIZE];
    char logHeader[BUFLEN_1024] = {0};
    CMC_SYS_DEVICE_INFO_T    pDeviceInfoObj;
    char ipAddress[BUFLEN_32] = {0};
    VOS_RET_E  ret = VOS_RET_SUCCESS;
    char dateLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char timeLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char hostLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};
    char msgLog[BCM_SYSLOG_MAX_LINE_SIZE] = {0};

    vosLog_debug("Enter>");

    /*try reading circular buffer*/
    if (SF_FEATURE_SUPPORT_SYSLOG)
    {
        while ((readptr != BCM_SYSLOG_READ_BUFFER_ERROR) && (readptr != BCM_SYSLOG_READ_BUFFER_END))
        {
            readptr = HAL_readLogPartialFlash(readptr, buffer);
            if ('\0' != buffer[0])
            {
                sscanf(buffer, "%s %s %s %[^\n]", dateLog, timeLog, hostLog, msgLog);
                memset(buffer, 0, sizeof(buffer));
                UTIL_SNPRINTF(buffer, BCM_SYSLOG_MAX_LINE_SIZE, "%s %s %s\n", dateLog, timeLog, msgLog);
            }

            cnt += util_strlen(buffer);
        }
        readptr = BCM_SYSLOG_FIRST_READ;
    }

    while ((readptr != BCM_SYSLOG_READ_BUFFER_ERROR) && (readptr != BCM_SYSLOG_READ_BUFFER_END))
    {
        readptr = HAL_readLogPartial(readptr, buffer);
        if ('\0' != buffer[0])
        {
            sscanf(buffer, "%s %s %s %[^\n]", dateLog, timeLog, hostLog, msgLog);
            memset(buffer, 0, sizeof(buffer));
            UTIL_SNPRINTF(buffer, BCM_SYSLOG_MAX_LINE_SIZE, "%s %s %s\n", dateLog, timeLog, msgLog);
        }

        cnt += util_strlen(buffer);
    }

    if ((ret = CMC_sysGetDeviceInfo(&pDeviceInfoObj)) == VOS_RET_SUCCESS)
    {
        vosLog_debug("Get device info success!");
    }
    else
    {
        vosLog_error("Get device info fail! ret=%d", ret);
        return 0;
    }

    if ((ret = CMC_wanGetConnIpAddr(acsState.boundIfName, ipAddress, BUFLEN_32)) == VOS_RET_SUCCESS)
    {
        vosLog_debug("Tr069 WAN intf is up!");
    }
    else
    {
        vosLog_error("Tr069 WAN intf is not up! ret=%d", ret);
        return 0;
    }

    UTIL_SNPRINTF(logHeader, sizeof(logHeader),
                  "Manufacturer:%s\nProductClass:%s\nSerialNumber:%s\nIP:%s\nHWVer:%s\nSWVer:%s\n\n",
                  pDeviceInfoObj.manufacturer, pDeviceInfoObj.productClass,
                  pDeviceInfoObj.serialNumber, ipAddress,
                  pDeviceInfoObj.hardwareVersion, pDeviceInfoObj.softwareVersion);
    test_head = util_strlen(logHeader) - 1;

    return cnt + test_head;
}


static UINT32 uploadGetConfigSize(void)
{
    vosLog_debug("==Enter==");

    UINT32 cnt = 0;
    UINT32 size = 0;
    char *data;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    HAL_flashGetConfigSize(&size);

    if ((data = VOS_MALLOC_FLAGS(size, ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("could not allocate %u bytes for config buf", size);
        return 0;
    }

    if ((ret = HAL_flashReadConfig(data, &cnt)) != VOS_RET_SUCCESS)
    {
        vosLog_error("could not read configuration from flash into buf, ret=%d", ret);
    }

    VOS_FREE(data);

    return cnt;
}

static void uploadPutData(RPCAction *action)
{
    vosLog_debug("==Enter==");

    UINT32 size = 0;
    HttpTask *ht = &httpUpload;


    if (ht->wio == NULL)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("Unable to connect to upload server", 9011);
        if (SF_FEATURE_SUPPORT_SYSLOG)
        {
            syslog(LOG_ERR, "104061 upload file failed!");
            util_saveLogToFlash(g_msgHandle);
        }

        return;
    }

    if (ht->wio->pc == NULL)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("Unable to connect to upload server", 9011);
        if (SF_FEATURE_SUPPORT_SYSLOG)
        {
            syslog(LOG_ERR, "104061 upload file failed!");
            util_saveLogToFlash(g_msgHandle);
        }

        return;
    }

    // Get the body data length
    switch (action->ud.downloadReq.efileType)
    {
        case eVendorLog:
            size = uploadGetLogSize();
            break;
        default:
            size = uploadGetConfigSize();
            break;
    }

    if (size != 0)
    {
        // Send the PUT header with content length
        ht->content_len = (int)size;
        ht->postMsg     = NULL;
        ht->postLth     = (int)size;
        ht->callback    = (CallBack)action; // stick RPCAction to callback

        // HttpTask has to be the last argument in wget_PutData
        // since it is used later in do_send_request
        wget_PutData(ht->wio, ht->postMsg, ht->postLth, "text/xml",
                     uploadCompletePut, (void *)ht);

        // Send the body data
        switch (action->ud.downloadReq.efileType)
        {
            case eVendorLog:
                uploadPutLog();
                break;
            default:
                uploadPutConfig();
                break;
        }
    }
    else
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("No upload data", 9011);
    }
}


static void uploaddiagPutData(CMC_TR69C_UPLOAD_DIAG_CFG_T *action)
{
    vosLog_debug("==Enter==");

    UINT32 size = 0;
    HttpTask *ht = &httpUpload;

    if (ht->wio == NULL)
    {
        uploadStop("Unable to connect to upload server", 9011);
        CMC_tr69cSetUploadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        return;
    }

    if (ht->wio->pc == NULL)
    {
        uploadStop("Unable to connect to upload server", 9011);
        CMC_tr69cSetUploadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        return;
    }

    // Get the body data length
    size = uploadGetConfigSize();

    if (size != 0)
    {
        // Send the PUT header with content length
        ht->content_len = (int)size;
        ht->postMsg     = NULL;
        ht->postLth     = (int)size;
        ht->callback    = (CallBack)action; // stick RPCAction to callback

        // HttpTask has to be the last argument in wget_PutData
        // since it is used later in do_send_request
        CMC_tr69cSetUploadROMTime();
        wget_PutData(ht->wio, ht->postMsg, ht->postLth, "text/xml",
                     uploaddiagCompletePut, (void *)ht);

        // Send the body data
        uploadDiagPutConfig(action);
    }
    else
    {
        uploadStop("No upload data", 9011);
        CMC_tr69cSetUploadDiagState(INCORRECT_SIZE);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    }
}


static void uploadPutAuth(RPCAction *action)
{
    vosLog_debug("==Enter==");

    int      size = 2;
    char     *msg;
    HttpTask *ht = &httpUpload;

    if (ht->wio == NULL)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("Unable to connect to upload server", 9011);
        return;
    }

    msg = (char *)VOS_MALLOC_FLAGS((UINT32)size, ALLOC_ZEROIZE);
    if (msg == NULL)
    {
        uploadStop("Unable to malloc", VOS_RET_INTERNAL_ERROR);
        return;
    }
    msg[0] = '\n';
    ht->content_len = size;
    ht->postMsg     = msg;
    ht->postLth     = size;
    ht->callback    = (CallBack)action; // stick RPCAction to callback

    // Send the newline file to test for authentication method
    // HttpTask has to be the last argument in wget_PutData
    // since it is used later in do_send_request
    wget_PutData(ht->wio, ht->postMsg, ht->postLth, "text/xml",
                 uploadCompleteAuth, (void *)ht);
}

static int uploadIsAuthNeeded(RPCAction *action)
{
    vosLog_debug("==Enter==");

    int ret = TRUE;
    HttpTask *ht = &httpUpload;

    if (action->ud.downloadReq.user == NULL)
        ret = FALSE;
    else if (action->ud.downloadReq.user[0] == '\0')
        ret = FALSE;
    else if (action->ud.downloadReq.pwd == NULL)
        ret = FALSE;
    else if (action->ud.downloadReq.pwd[0] == '\0')
        ret = FALSE;

    // authHdr already has value by uploadPutAuth
    // so don't need to perform authentication again
    if (ht->authHdr != NULL)
        ret = FALSE;

    return ret;
}

static void uploadConnected(void *handle)
{
    vosLog_debug("==Enter==");

    tWget *wg = (tWget *)handle;
    RPCAction *action = (RPCAction *)wg->handle;

    if (wg->status != 0)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("Unable to connect to upload server", 9011);
        return;
    }

    if (uploadIsAuthNeeded(action) == FALSE)
    {
        uploadPutData(action);
    }
    else
    {
        uploadPutAuth(action);
    }
}


static void uploaddiagConnected(void *handle)
{
    vosLog_debug("==Enter==");

    //tWget *wg = (tWget *)handle;
    CMC_TR69C_UPLOAD_DIAG_CFG_T *action = (CMC_TR69C_UPLOAD_DIAG_CFG_T *)handle;

    /*if (wg->status != 0)
    {
        uploadStop("Unable to connect to upload server", 9011);
        return;
    }*/
    CMC_tr69cSetUploadResponseTime();
    uploaddiagPutData(action);
}


/* this is called by the callback from the startTimer in doUpload. */
/* we have to use wget_Connect in case of authentication in which case */
/* we need to control the connection */
void uploadStart(void *handle)
{
    RPCAction   *action = (RPCAction *)handle;
    HttpTask    *ht = &httpUpload;
    DownloadReq *ulreq = &action->ud.downloadReq;

    vosLog_debug("==Enter==");

    if (isTransferInProgress())
    {
#ifdef DEBUG
        printf("------------> transfer in progress, delay request command key %s for 5 seconds......\n",
               ulreq->commandKey);
#endif /* DEBUG */

        /* delay by five second */
        utilTmr_set(tmrHandle, uploadStart, (void *)action, 5000, "upload");
        return;
    }
    else
    {
        if (ulreq->state == eTransferRejected)
        {
#ifdef DEBUG
            printf("@@@@@@@@@@@@@@@@@This upload request was rejected, return 9004 error code, commandKey %s\n",
                   ulreq->commandKey);
#endif /* DEBUG */

            if (transferCompletePending)
            {
#ifdef DEBUG
                printf("^^^^^ transferCompletePending..  give tr69c a chance to send the transfer complete out first\n");
#endif /* DEBUG */

                utilTmr_set(tmrHandle, uploadStart, (void *)action, 1000, "upload");
            }
            else
            {
                if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                {
                    if (enblCTMiddleware == CTMDW_MODE_0)
                    {
                        g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                        ctmdw_sendUploadRet();
                    }
                }

                updateDownLoadKey(ulreq);
                uploadStop("Resources exceeded", 9004);
            }

            return;
        } /* eTransferRejected */
        updateTransferState(ulreq->commandKey, eTransferInProgress);
    }

    updateDownLoadKey(ulreq);
    if (strstr(ulreq->url, "http:") == NULL &&
            strstr(ulreq->url, "https:") == NULL)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("Protocol not supported", 9013);
        return;
    }

    if (ulreq->efileType != eVendorConfig &&
            ulreq->efileType != eVendorLog)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_FAILED;
                ctmdw_sendUploadRet();
            }
        }

        uploadStop("Invalid upload file type", 9011);
        return;
    }

    memset(ht, 0, sizeof(struct HttpTask));
    memset(&putSessionAuth, 0, sizeof(struct SessionAuth));

    ht->wio = wget_Connect(ulreq->url, uploadConnected, action);
    if (ht->wio == NULL)
    {
        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
        {
            if (enblCTMiddleware == CTMDW_MODE_0)
            {
                g_downloadState = CTMDW_DOWNLOAD_STATE_ERR_CONNECT;
                ctmdw_sendUploadRet();
            }
        }

        vosLog_error("wget_Connect to upload server failed: %s", wget_LastErrorMsg());
        uploadStop("Connect to upload server failed", 9011);
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


void uploaddiagStart(CMC_TR69C_UPLOAD_DIAG_CFG_T *handle)
{
    CMC_TR69C_UPLOAD_DIAG_CFG_T   *action = (CMC_TR69C_UPLOAD_DIAG_CFG_T *)handle;
    HttpTask    *ht = &httpUpload;

    vosLog_debug("==Enter==");

    if (strstr(action->uploadurl, "http:") == NULL)
    {
        uploadStop("Protocol not supported", 9013);
        CMC_tr69cSetUploadDiagState(NO_TRANSFER_MODE);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
        return;
    }

    memset(ht, 0, sizeof(struct HttpTask));
    //memset(&putSessionAuth,0, sizeof(struct SessionAuth));
    CMC_tr69cSetUploadRequestTime();

    ht->wio = wget_Connect(action->uploadurl, uploaddiagConnected, action);
    if (ht->wio == NULL)
    {
        vosLog_error("wget_Connect to upload server failed: %s", wget_LastErrorMsg());
        uploadStop("Connect to upload server failed", 9011);
        CMC_tr69cSetUploadDiagState(INIT_CONNECTION_FAILED);
        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
    }
    else
    {
        //acsState.startDLTime = time(NULL);
        wget_ClearPostHdrs(ht->wio);
        if (ht->authHdr)
        {
            VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);
        }
    }
}


