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
 * File Name  : informer.c
 *
 * Description: Inform logic for tr69
 * $Revision: 1.51 $
 * $Id: informer.c,v 1.51 2006/02/17 21:22:41 dmounday Exp $
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/if.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "fwk.h"
#include "vos_msg.h"
#include "cmc_api.h"
#include "../inc/tr69cdefs.h"
#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include "../SOAPParser/RPCState.h"
#include "xml_parser_sm.h"
#include "../SOAPParser/xmlTables.h"
#include "event.h"

#include "../webproto/protocol.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../main/informer.h"
#include "../main/httpProto.h"
#include "../main/httpDownload.h"
#include "acsListener.h"
#include "../bcmLibIF/bcmWrapper.h"
#include "phl.h"
#include "tr69c_api.h"
#include "mdm.h"

#include "../inc/tr69cdefs.h"           /* defines for ACS state */

#include "../ctMiddleware/ctMiddleware.h"


typedef struct TR69C_ALARM
{
    CMC_TR69C_AlARM_T alarmCfg;
    struct TR69C_ALARM *next;
} TR69C_ALARM_T;


typedef struct TR69C_MONITOR
{
    CMC_TR69C_ALARM_MONITOR_T monitorCfg;
    struct TR69C_MONITOR *next;
} TR69C_MONITOR_T;


/** external functions **/
extern void main_cleanup(SINT32 code);

/** external data **/
extern ACSState      acsState;
extern int transferCompletePending;
extern int sendGETRPC;                  /* send a GetRPCMetods */
extern int tr69cTerm;                   /* TR69C termination flag */

extern RPCAction *simRpcAction;
extern RPCAction *acsRpcAction;

extern void *g_msgHandle;
extern LimitNotificationQInfo limitNotificationList;

/** public data **/
eInformState   informState = eACSNeverContacted;
eSessionState  sessionState = eSessionEnd;
InformEvList   informEvList;
InformEvList   g_tr69cInformEvListCopy;

/** private data **/
static CookieHdr *glbCookieHdr = NULL;
HttpTask      httpTask;                 /* http io desc */
HttpTask      simHttpTask;
static SessionAuth   sessionAuth;
static int  sentACSNull;                /* set if last msg to ACS was NULL, cleared if non-null */
/* response received from ACS */
static UBOOL8 rebootingFlag = FALSE;    /*set if we are doing system reboot or factoryreset*/

extern UBOOL8 g_writeLog;

extern int g_dns_resolve_ret;

int monitorAllNumber = 0;
int monitorStatus = 0;
int alarmAllNumber = 0;
int alarmStatus = 0;
int send_alarm_number = 0;
int send_monitor_number = 0;
int send_clean_alarm_number = 0;
int glbflag = 3;

CT_ALARMORMONITOR_SEND alarmSend[50];
CT_ALARMORMONITOR_SEND monitorSend[50];
CT_ALARMORMONITOR_SEND cleanalarmSend[50];

TR69C_ALARM_T *pAlarmHead = NULL;
TR69C_MONITOR_T *pMonitorHead = NULL;

/** private functions **/
static int  getInformEvCnt(void);
static void closeACSConnection(HttpTask *ht);
static void acsDisconnect(HttpTask *ht, AcsStatus acsStatus);
static void updateAuthorizationHdr(HttpTask *ht);
static void nullHttpTimeout(void *handle);
static void postComplete(void *handle);
static void Connected(void *handle);
static void sendInformData(void);
static void startACSComm(void *handle);
static int  getRandomNumber(int from, int to);
static int  getDelayTime(int acsConnectFails);

void backupInformEvList();

int ReadRandomInformEnaleFromFile();
void WriteRandomInformEnableTofile(int RandomEnable);


int ReadRandomInformEnaleFromFile()
{
    FILE *fp = NULL;
    int EnableFlag = 0;

    fp = fopen("/var/config/RandomInform", "r");
    if (NULL == fp)
    {
        printf("open file error");
        return 0;
    }
    else
    {
        fscanf(fp, "%d", &EnableFlag);
    }

    fclose(fp);
    return EnableFlag;
}


void WriteRandomInformEnableTofile(int RandomEnable)
{
    FILE *fp;

    fp = fopen("/var/config/RandomInform", "w");
    if (NULL == fp)
    {
        printf("open file error");
        return;
    }
    else
    {
        fprintf(fp, "%d", RandomEnable);
    }

    fclose(fp);
}


unsigned int new_random(void)
{
    int fd;
    unsigned int n = 0;

    fd = open("/dev/urandom", O_RDONLY);

    if (fd > 0)
    {
        read(fd, &n, sizeof(n));
        close(fd);
    }

    return n;
}


int getRangeNumber(int maxNum)
{
    int num = 0;
    int max;
    unsigned int ticks;
    struct timeval tv;
    int fd;
    unsigned int r;
    int i;

    max = maxNum;

    if (maxNum < 2)
    {
        return 1;
    }

    gettimeofday(&tv, NULL);
    ticks = tv.tv_sec + tv.tv_usec;

    fd = open("/dev/urandom", O_RDONLY);

    if (fd == -1)
    {
        return 1;
    }

    for (i = 0; i < 512; i++)
    {
        read(fd, &r, sizeof(r));
        ticks += r;
    }

    close(fd);
    srand(ticks);
    srand((unsigned)time(NULL));

    if (max > 40)
    {
        num = new_random() % (max - 40) + 30; //减小10秒，防止出现时间超出最大。
    }
    else
    {
        num = 30;
    }

    printf("Delay seconds:%d\n", num);
    return num;
}


#define BRCTL_GET_BRIDGE_USERNAME 34
#define SIOCDEVPRIVATE 0x89F0 /* to 89FF */
char bridgeUserName[64] = {0};
static int getBridgeUsernameIsOk(void);
static void getBridgeUsername(void *handle);
int getRandomNumber(int from, int to)
{
    int num = 0;

    srand((unsigned int)time((time_t *)NULL));
    num = (rand() % (to - from)) + from;

    return num;
}


int getDelayTime(int acsConnectFails)
{
    int delayTime = 0;

    vosLog_debug("acsConnectFails = %d", acsConnectFails);

    /* the following implementation is based on section
     * 3.2.1.1 Session Retry Policy in "TR-069 Amendment 1"
     * Table 3 - Session Retry Wait Intervals
     */
    switch (acsConnectFails)
    {
        case 1:
            delayTime = getRandomNumber(5, 10);
            break;
        case 2:
            delayTime = getRandomNumber(10, 20);
            break;
        case 3:
            delayTime = getRandomNumber(20, 40);
            break;
        case 4:
            delayTime = getRandomNumber(40, 80);
            break;
        case 5:
            delayTime = getRandomNumber(80, 160);
            break;
        case 6:
            delayTime = getRandomNumber(160, 320);
            break;
        case 7:
            delayTime = getRandomNumber(320, 640);
            break;
        case 8:
            delayTime = getRandomNumber(640, 1280);
            break;
        case 9:
            delayTime = getRandomNumber(1280, 2560);
            break;
        default:
            delayTime = getRandomNumber(2560, 5120);
            break;
    }

    return delayTime;
}

void retrySessionConnection(void)
{
    HttpTask *ht = &httpTask;
    UINT32 retryCount = (UINT32)acsState.retryCount;

    if (ht->wio != NULL)
    {
        wget_Disconnect(ht->wio);
        ht->wio = NULL;
        ht->xfrStatus = ePostError;
        ht->eHttpState = eClosed;
        ht->eAuthState = sIdle;
    }

    if (acsState.retryCount > 0)
    {
        /* the following implementation is based on section
         * 3.2.1.1 Session Retry Policy in "TR-069 Amendment 1"
         * Table 3 - Session Retry Wait Intervals
         * delay time for acsConnectFails that is greater than 10
         * is the same with acsConnectFails that is equal to 10
         */
        if (acsState.retryCount > 10)
        {
            retryCount = 10;
        }

        /* retry ACS connect errors with increasing retry time interval */
        UINT32 backOffTime = (UINT32)getDelayTime(retryCount) * 1000;

        vosLog_error("ACS connect failed, retryCount = %d, backOffTime = %dms", retryCount, backOffTime);
        utilTmr_replaceIfSooner(tmrHandle, sendInform, NULL, backOffTime, "conn_fail_backoff_inform");
    }
    else
    {
        utilTmr_replaceIfSooner(tmrHandle, sendInform, NULL, 0, "conn_backoff_retryReset");
        vosLog_debug("ACS connect retry, no more backoff, retryCount = %d", retryCount);
    }
}


HttpTask *getHttpTask()
{
    if (simRpcAction != NULL)
    {
        vosLog_debug("simHttpTask");
        return &simHttpTask;
    }
    else
    {
        vosLog_debug("httpTask");
        return &httpTask;
    }
}

UBOOL8 isAcsConnected(void)
{
    HttpTask *tempHttp = getHttpTask();

    return ((tempHttp->wio != NULL) && (tempHttp->wio->pc != NULL));
}


tProtoCtx *getAcsConnDesc(void)
{
    HttpTask *tempHttp = getHttpTask();

    if (!isAcsConnected())
    {
        return NULL;
    }

    if (SF_FEATURE_SUPPORT_TR69C_SSL)
    {
        if (tempHttp->wio->pc->type == iSsl && tempHttp->wio->pc->ssl != NULL)
        {
            if (tempHttp->wio->pc->sslConn > 0)
            {
                return (tempHttp->wio->pc);
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return (tempHttp->wio->pc);
        }
    }
    else
    {
        return (tempHttp->wio->pc);
    }
}


void clearInformEventListNoBackup(void)
{
    int count = 0;
    int i = 0;

    vosLog_debug("cleared informEvList");

    count = informEvList.informEvCnt;

    for (i = 0; i < count; i++)
    {
        informEvList.informEvList[i] = 0;
    }

    informEvList.informEvCnt = 0;
}


void clearInformEventList(void)
{
    int count = 0;
    int count_copy = 0;
    int i = 0;
    int i_copy = 0;
    UBOOL8 found = FALSE;
    InformEvList tempInformEvList;
    UINT32 tempIndex = 0;

    memset(&tempInformEvList, 0, sizeof(tempInformEvList));

    vosLog_debug("******cleared informEvList");

    count_copy = g_tr69cInformEvListCopy.informEvCnt;
    count = informEvList.informEvCnt;

    for (i = 0; i < count; i++)
    {
        found = FALSE;

        for (i_copy = 0; i_copy < count_copy; i_copy++)
        {
            if (g_tr69cInformEvListCopy.informEvList[i_copy] == informEvList.informEvList[i])
            {
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            vosLog_debug("******Event %d was been added after information sent, retain it.\n", informEvList.informEvList[i]);

            tempInformEvList.informEvList[tempIndex] = informEvList.informEvList[i];
            tempIndex ++;
            tempInformEvList.informEvCnt = tempIndex;
        }
    }

    informEvList = tempInformEvList;

    if (informEvList.informEvCnt > 0)
    {
        vosLog_debug("*******There are still %d events in the list.\n", informEvList.informEvCnt);
    }
}


int getInformEvCnt(void)
{
    return informEvList.informEvCnt;
}

static void closeACSConnection(HttpTask *ht)
{
    vosLog_debug("=====>ENTER");
    if (ht->wio)
    {
        wget_Disconnect(ht->wio);
        ht->wio = NULL;
    }

#ifdef later
    /*
     * mwang: free these pointers to make resource checker happy.
     * I decided not to do this yet because freeing sessionAuth here may
     * have some unintended consequences.
     */
    resetSessionAuth(&sessionAuth);

    /*
     * Freeing the acsRpcAction should be OK, since we are done with this transaction,
     * but leave out for now also.
     */

    freeRPCAction(acsRpcAction);
    acsRpcAction = NULL;
#endif
}

static void acsDisconnect(HttpTask *ht, AcsStatus acsStatus)
{
    static AcsStatus acsStatusPrev;

    vosLog_debug("=====>ENTER");
    utilTmr_cancel(tmrHandle, nullHttpTimeout, (void *)ht);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);

    closeACSConnection(ht);
    ht->xfrStatus = acsStatus;
    ht->eHttpState = eClosed;
    ht->eAuthState = sIdle;

    switch (acsStatus)
    {
        case eAuthError:
            ++acsState.retryCount;
            if (acsState.informEnable)
            {
                if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
                {
                    /*Auth Failed. 20100125 houweilin*/
                    CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_VERIFY_FAIL, CMC_TR69C_REMOTE_INFORM_STATUS);
                    vosLog_error("Auth failed!");
                }

                cancelPeriodicInform();
            }

            vosLog_notice("Failed authentication with ACS");
            /*save retryCount to scratchpad*/
            saveTR69StatusItems();
            break;
        case eConnectError:
        case eGetError:
        case ePostError:
            ++acsState.retryCount;
            if (acsState.informEnable)
            {
                cancelPeriodicInform();
            }

            vosLog_error("ACS Disconnect with error %d", acsStatus);

#ifdef ALLOW_DISCONNECT_ERROR
            vosLog_notice("Continue processing even though ACS Disconnect with error");
            saveTR69StatusItems();
            saveConfigurations();
            rebootingFlag = factoryResetCompletion();  /*this will cause a reboot if factoryResetFlag==1*/
            if (rebootingFlag)
            {
                rebootCompletion();
            }
            else
            {
                rebootingFlag = rebootCompletion();
            }
#endif
            break;
        case eAcsDone:
        default:
            /* should NOT clear acsState.retryCount here, only clear
             * it after receiving InformResponse to make this value
             * can be shown correctly in Inform.
             */
            if (acsStatusPrev != eAcsDone)
            {
                resetPeriodicInform(acsState.informInterval);
            }

            vosLog_debug("ACS Disconnect: ok");
            /* if no error then run thru pending disconnect actions */
            if (acsStatus == eAcsDone)
            {
                clearInformEventList();
            }

            saveTR69StatusItems();
            saveConfigurations();
            rebootingFlag = factoryResetCompletion(); /* this will cause a reboot if factoryResetFlag==1*/
            if (rebootingFlag)
            {
                rebootCompletion();
            }
            else
            {
                rebootingFlag = rebootCompletion();
            }
            break;
    }

    if (tr69cTerm)
    {
        vosLog_notice("TR69C terminated due to tr69cTerm flag");
        tr69cTerm = 0;         /* Clear tr69c termination flag */

        /* this function calls all cleanup functions and exits with specified code */
        main_cleanup(0);
    }

    /*
     * instead of checking all these flags, just find out if there is event in
     * informEventList
     */
    if ((getInformEvCnt() != 0)
            || transferCompletePending
            || sendGETRPC
            || (acsStatus == eAuthError)
            || (acsStatus == eConnectError)
            || (acsStatus == eGetError)
            || (acsStatus == ePostError))   /*session retry policy*/
    {
        retrySessionConnection();
    }

    acsStatusPrev = acsStatus;
}  /* End of acsDisconnect() */

static void updateAuthorizationHdr(HttpTask *ht)
{
    vosLog_debug("=====>ENTER");
    if (ht->authHdr)
    {
        if (eAuth == sessionAuth.qopType)
        {
            VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);
            ht->authHdr = generateNextAuthorizationHdrValue(&sessionAuth,
                          acsState.acsUser, acsState.acsPwd);
            wget_AddPostHdr(ht->wio, "Authorization", ht->authHdr); /* replace header */
        }
        else
        {
            /* must be basic or not "Auth" */
            wget_AddPostHdr(ht->wio, "Authorization", ht->authHdr); /* replace header */
        }
    }
}  /* End of updateAuthorizationHdr() */

static void nullHttpTimeout(void *handle)
{
    HttpTask    *ht = (HttpTask *)handle;

    vosLog_debug("=====>ENTER");
    acsDisconnect(ht, eAcsDone);
}

void sendNullHttp(UBOOL8 disconnect)
{
    HttpTask *ht = &httpTask;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (ht->wio)
    {
        vosLog_debug("sendNullHttp(%s) to ACS", ht->eHttpState == eClose ? "close" : "keepOpen");
    }

    if (ht->postMsg)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
    }

    sendToAcs(0, NULL);
    /*send empty POST only count when holdrequests is false*/
    if (acsState.holdRequests == FALSE)
    {
        sentACSNull = 1;
    }

    if (disconnect == TRUE)
    {
        ret = utilTmr_set(tmrHandle, nullHttpTimeout, (void *)ht, ACSRESPONSETIME / 2, "null_http"); /* half of max */
        if (ret != VOS_RET_SUCCESS)
        {
            vosLog_error("could not set NULL httpd timer, ret=%d", ret);
        }
    }
}

#ifdef SUPPORT_ACS_CISCO
void informRspTimeout(void *handle)
{
    informState = eACSInformed;
    setInformState(eACSInformed);
    if (transferCompletePending)
    {
        sendTransferComplete();
        transferCompletePending = 0;
    }
    else
    {
        sendNullHttp(TRUE);
    }

    /* need this to get CISCO notifications to work.Since we never get a */
    /* InformResponse for the Cisco tool */
    resetNotification();
}
#endif

static void freeMsg(char *buf, int *bufSz)
{
    if (buf != NULL)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(buf);
    }

    *bufSz = 0;
}

static char *readLengthMsgDiag(tWget *wg, int readLth, int *mlth, int doFlushStream)
{
#define MaxPacketLenth (1024*10)
    int bufCnt                = 0;
    int readCnt               = 0;
    int bufLth                = readLth;
    char *soapBuf             = NULL;
    int headtotal             = 0;
    int buf_cap               = 0;
    *mlth                     = 0;
    UINT32 testBytesReceived  = 0;
    UINT32 totalBytesReceived = 0;
    if ((soapBuf = (char *) VOS_MALLOC_FLAGS(MaxPacketLenth + 1, 0)) != NULL)
    {
        while (bufCnt < readLth)
        {
            buf_cap = bufLth > MaxPacketLenth ? MaxPacketLenth : bufLth;
            if ((readCnt = proto_Readn(wg->pc, soapBuf, buf_cap)) > 0)
            {
                bufCnt             += readCnt;
                bufLth             -= readCnt;
                headtotal          += 38;
                testBytesReceived   = bufCnt;
                totalBytesReceived  = headtotal + bufCnt;
            }
            else
            {
                /* read error */
                *mlth = 0;
                VOS_MEM_FREE_BUF_AND_NULL_PTR(soapBuf);
                break;
            }
        }
        vosLog_debug("soapBuf bufCnt=%d readLth=%d\n", bufCnt, readLth);
        *mlth = bufCnt;

        if (doFlushStream)
        {
            proto_Skip(wg->pc);
        }

        CMC_tr69cSetTestBytesReceived(testBytesReceived);
        CMC_tr69cSetTotalBytesReceived(totalBytesReceived);
    }
    return soapBuf;

}
static char *readLengthMsg(tWget *wg, int readLth, int *mlth, UBOOL8 doFlushStream)
{
    int bufCnt = 0;
    int bufCnt_1M = 0;
    int readCnt = 0;
    int bufLth = readLth;
    char *soapBuf = NULL;

    *mlth = 0;

    /*
     * This is the path taken when we do image download.  Don't zeroize
     * the buffer that is allocated here because that will force linux
     * to immediately assign physical pages to the buffer.  Intead, just
     * let the buffer fill in as the transfer progresses.  This will give
     * smd and the kernel more time to make physical pages available.
     */
    if ((soapBuf = (char *) VOS_MALLOC_FLAGS((UINT32)readLth + 1, 0)) != NULL)
    {
        while (bufCnt < readLth)
        {
            if ((readCnt = proto_Readn(wg->pc, soapBuf + bufCnt, bufLth)) > 0)
            {
                bufCnt += readCnt;
                bufLth -= readCnt;
            }
            else
            {
                if (readCnt == -99)
                {
                    /* read error */
                    vosLog_error("download interrupted");
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(soapBuf);

                    *mlth = 0;
                }
                else if (readCnt == 0)
                {
                    vosLog_error("reach end of stream.");
                }

                break;
            }

            /* send HEARTBEAT to smd */
            if ((bufCnt / (1024 * 1024)) > bufCnt_1M)
            {
                UTIL_sendHeartbeat(g_msgHandle);
                bufCnt_1M = bufCnt / (1024 * 1024);
            }

            vosLog_debug("readCnt:%u;bufCnt:%u;errno:%d.", readCnt, bufCnt, errno);
        }

        vosLog_debug("soapBuf bufCnt=%d readLth=%d\n", bufCnt, readLth);
        if (readCnt != -99)
        {
            *mlth = bufCnt;
            soapBuf[bufCnt] = '\0';
        }
        if (doFlushStream)
        {
            /* If we are not processing a chunked message,
             * skip(flush) anything else
             */
            proto_Skip(wg->pc);
        }
    }

    return soapBuf;
}

static char *readChunkedMsg(tWget *wg, int *mlth, int maxSize)
{
    char *soapBuf = NULL;
    char chunkedBuf[128] = {0};

    *mlth = 0;

    /*read chunked size of first chunk*/
    if (proto_Readline(wg->pc, chunkedBuf, sizeof(chunkedBuf)) > 0)
    {
        int  chunkedSz = 0, readSz = 0;
        char *newBuf = NULL, *readBuf = NULL;

        sscanf(chunkedBuf, "%x", &chunkedSz);
        while (chunkedSz > 0)
        {
            /* read chunked data*/
            UBOOL8 doFlushStream = FALSE;
            readBuf = readLengthMsg(wg, chunkedSz, &readSz, doFlushStream);
            if (chunkedSz != readSz)
            {
                vosLog_error("===> readChunkedMsg, chunked size = %d, read size = %d\n", chunkedSz, readSz);
            }

            if (readBuf == NULL)
            {
                freeMsg(soapBuf, mlth);
                break;
            }

            if ((*mlth + readSz) > maxSize)
            {
                vosLog_error("reading more data than maxSize (%d)", maxSize);
                freeMsg(soapBuf, mlth);
                freeMsg(readBuf, &readSz);
                break;
            }

            if (NULL == soapBuf)
            {
                /* allocate the first chunk since VOS_REALLOC
                 * does not accept soapBuf as NULL pointer.
                 */
                newBuf = soapBuf = VOS_MALLOC_FLAGS((UINT32)(*mlth + readSz + 1), ALLOC_ZEROIZE);
            }
            else
            {
                /* reallocate soap message size*/
                newBuf = VOS_REALLOC(soapBuf, (UINT32)(*mlth + readSz + 1));
            }

            if (NULL == newBuf)
            {
                freeMsg(soapBuf, mlth);
                freeMsg(readBuf, &readSz);
                break;
            }

            /* point soap message to new allocated memory*/
            soapBuf = newBuf;
            /* append chunked data to soap message*/
            UTIL_STRNCPY(soapBuf + *mlth, readBuf, readSz + 1);
            /* increase soap message size*/
            *mlth += readSz;
            /* free chunked data*/
            freeMsg(readBuf, &readSz);
            chunkedSz = 0;
            /*flush off trailing crlf*/
            do
            {
                chunkedBuf[0] = '\0';
                readSz = proto_Readline(wg->pc, chunkedBuf, sizeof(chunkedBuf));
            }
            while (readSz > 0 && isxdigit(chunkedBuf[0]) == 0);

            /* read chunked size of next chunk*/
            if (isxdigit(chunkedBuf[0]) != 0)
            {
                sscanf(chunkedBuf, "%x", &chunkedSz);
            }
            else
            {
                freeMsg(soapBuf, mlth);
            }
        }

        /* skip(flush) anything else*/
        proto_Skip(wg->pc);
    }

    return soapBuf;
}


void writeLog(const char *buf, int bufLen)
{
    FILE *fd = NULL;

    if ((fd = fopen(UTIL_CLI_TR69_RESULT_FILE, "a+")) == NULL)
    {
        vosLog_error("open logTr69 failed");
    }
    else
    {
        if (g_writeLog)
        {
            fwrite(buf, bufLen, 1, fd);
        }

        fclose(fd);
    }
}

/*
 * tWget *wg is an connected web descriptor,
 *      *mlth is pointer to location of result read length,
 *      maxBufferSize is maximum size to read if non-zero. No limit if maxSize is 0.
 * Returns:
 *     pointer to response buffer or NULL.
 *      *mlth contain size of buffer. Undefined if return is NULL.
 */
char *readResponse(tWget *wg, int *mlth, int maxBufferSize)
{
    char *soapBuf = NULL;

    if (wg->hdrs->content_length > 0)
    {
        UBOOL8 doFlushStream = FALSE;

        /* liqingyang: we allow to read length step by step.
               * if maxBufferSize = 0, we read all content, or read maxBufferSize.
               */
        int maxSize = (maxBufferSize > 0) ? maxBufferSize : wg->hdrs->content_length;

        if (maxSize > wg->hdrs->content_length)
        {
            vosLog_error("Max length is larger than content length");
            return NULL;
        }

        if (maxSize == wg->hdrs->content_length)
        {
            doFlushStream = TRUE;
        }
        /* liqingyang */

        /* this is the path taken by image downloads */
        vosLog_debug("calling readLengthMsg with content_length=%d", maxSize);
        soapBuf = readLengthMsg(wg, maxSize, mlth, doFlushStream);
    }
    else if (wg->hdrs->TransferEncoding && !util_strcasecmp(wg->hdrs->TransferEncoding, "chunked"))
    {
        int maxSize = (maxBufferSize > 0) ? maxBufferSize : MAXWEBBUFSZ;
        vosLog_debug("calling readChunkedMsg with maxSize=%d", maxSize);
        soapBuf = readChunkedMsg(wg, mlth, maxSize);
    }

    if (soapBuf != NULL)
    {
        writeLog(soapBuf, strlen(soapBuf));
    }

    return soapBuf;
}  /* End of readResponse() */

char *readResponseDiag(tWget *wg, int *mlth, int maxBufferSize)
{
    char *soapBuf = NULL;

    if (wg->hdrs->content_length > 0)
    {
        int doFlushStream = TRUE;
        soapBuf = readLengthMsgDiag(wg, wg->hdrs->content_length, mlth, doFlushStream);
    }
    else if (wg->hdrs->TransferEncoding && !strcasecmp(wg->hdrs->TransferEncoding, "chunked"))
    {
        int maxSize = (maxBufferSize > 0) ? maxBufferSize : MAXWEBBUFSZ;
        soapBuf = readChunkedMsg(wg, mlth, maxSize);
    }

    return soapBuf;
}  /* End of readResponse() */

static void addCookie(CookieHdr *hdr, CookieHdr *cookie)
{
    CookieHdr *curr = hdr;
    CookieHdr *prev = hdr;

    /*does cookie already exist in cookie list*/
    for (curr = hdr, prev = hdr; curr != NULL; prev = curr, curr = curr->next)
    {
        if (0 == util_strcmp(curr->name, cookie->name)
                && 0 == util_strcmp(curr->value, cookie->value))
        {
            break;
        }
    }

    /*if cookie is not in cookie list then add it*/
    if (NULL == curr)
    {
        curr = (CookieHdr *) VOS_MALLOC_FLAGS(sizeof(CookieHdr), ALLOC_ZEROIZE);
        curr->name = VOS_STRDUP(cookie->name);
        curr->value = VOS_STRDUP(cookie->value);
        curr->next = NULL;
        prev->next = curr;
    }
}

static void copyCookies(CookieHdr **dst, CookieHdr *src)
{
    CookieHdr *cp = src;

    /* create cookie head*/
    if (*dst == NULL && cp != NULL)
    {
        *dst = (CookieHdr *) VOS_MALLOC_FLAGS(sizeof(CookieHdr), ALLOC_ZEROIZE);
        (*dst)->name = VOS_STRDUP(cp->name);
        (*dst)->value = VOS_STRDUP(cp->value);
        (*dst)->next = NULL;
        cp = cp->next;
    }

    /* add cookie to cookie head*/
    while (cp != NULL)
    {
        addCookie(*dst, cp);
        cp = cp->next;
    }
}  /* End of copyCookies() */

static void freeCookies(CookieHdr **p)
{
    CookieHdr *next = *p;

    while (next)
    {
        CookieHdr *temp;
        temp = next->next;
        if (next->name)
        {
            VOS_MEM_FREE_BUF_AND_NULL_PTR(next->name);
        }

        if (next->value)
        {
            VOS_MEM_FREE_BUF_AND_NULL_PTR(next->value);
        }

        VOS_MEM_FREE_BUF_AND_NULL_PTR(next);
        next = temp;
    }
}  /* End of freeCookies() */

static void handleSoapMessage(char *soapmsg, int len)
{
    vosLog_debug("soapmsg = %p, len = %d", soapmsg, len);

    if (soapmsg)
    {
        eParseStatus   status;
        ParseHow       parseReq;
#ifdef DUMPSOAPIN
        fprintf(stderr, "SoapInMsg=>>>>>\n%s\n<\n", soapmsg);
#endif
        if (acsRpcAction)
        {
            freeRPCAction(acsRpcAction);
            acsRpcAction = newRPCAction();
        }

        parseReq.topLevel = envelopeDesc;
        parseReq.nameSpace = nameSpaces;
        status = parseGeneric(NULL, soapmsg, len, &parseReq);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(soapmsg);

        if (status == NO_ERROR)
        {
            if (runRPC() == eRPCRunFail)
            {
                /* couldn't run the RPC: so just disconnect */
                acsDisconnect(&httpTask, eAcsDone);
            }
        }
        else
        {
            vosLog_debug("ACS Msg. Parse Error %80s", soapmsg);
            acsDisconnect(&httpTask, eAcsDone);  /* force disconnect on parse error*/
        }
    }
    else
    {
        /* no response */
        vosLog_debug("status = 200, no Soapmsg. sentACSNull=%d", sentACSNull);
        if (!sentACSNull)
        {
            sendNullHttp(FALSE);
        }

        acsDisconnect(&httpTask, eAcsDone);
    }
}

static void handleSoapMessageCallBack(void *handle)
{
    HttpTask *ht = &httpTask;
    tWget *wg = (tWget *)handle;
    char  *soapmsg = (char *)wg->handle;

    ht->eHttpState = eConnected;
    /* copy cookies from previous connection to the new one*/
    if (glbCookieHdr != NULL)
    {
        copyCookies(&(ht->wio->cookieHdrs), glbCookieHdr);
    }

    handleSoapMessage(soapmsg, util_strlen(soapmsg));
}

/*
 * Data has been posted to the server or an
 * error has ocurred.
 */
static void postComplete(void *handle)
{
    tWget       *wg = (tWget *)handle;
    HttpTask    *ht = &httpTask;
    SessionAuth *sa = &sessionAuth;
    int skipResult = 1;

    vosLog_debug("Enter>, wg = %p", wg);

    utilTmr_cancel(tmrHandle, nullHttpTimeout, (void *)ht);

    if (iWgetStatus_Ok == wg->status)
    {
        if (ht->wio->hdrs->Connection
                && !util_strcasecmp(ht->wio->hdrs->Connection, "close"))
        {
            ht->eHttpState = eClose;
        }

        vosLog_debug("Connection = %s", ht->eHttpState == eClose ? "close" : "keep-alive");
        vosLog_debug("wg->hdrs->status_cod = %d", wg->hdrs->status_code);
        if (401 == wg->hdrs->status_code)
        {
            /* need to send authenticate */
            char *hdrvalue;
            if (wg->hdrs->content_length > 0
                    || (wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding, "chunked")))
            {
                int   mlth;
                char *tmpBuf;
                if ((tmpBuf = readResponse(wg, &mlth, 0)))
                {
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(tmpBuf);
                }
            }

            sentACSNull = 0;
            VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr); /* free in case of reauth requested during connection */
            if (ht->eAuthState == sAuthenticating)
            {
                ht->eAuthState = sAuthFailed;
                VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                ht->postLth = 0;
                /* disconnect and delay */
                acsDisconnect(ht, eAuthError);
                return;
            }
            else
            {
                ht->eAuthState = sAuthenticating;
            }

            vosLog_debug("WWW-Authenticate= %s", wg->hdrs->wwwAuthenticate);
            if (!(hdrvalue = generateAuthorizationHdrValue(sa, wg->hdrs->wwwAuthenticate,
                             "POST", ht->wio->uri,
                             acsState.acsUser,
                             acsState.acsPwd)))
            {
                vosLog_error("WWWAuthenticate header parsing error: %s",
                             wg->hdrs->wwwAuthenticate);
                VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                acsDisconnect(ht, eAuthError);
                return;
            }

            ht->authHdr = hdrvalue;
            if (skipResult == 0 || ht->eHttpState == eClose)
            {
                /* end of data on 401 skip_Proto() */
                /* save cookies of the current connection*/
                if (wg->hdrs->setCookies != NULL)
                {
                    copyCookies(&glbCookieHdr, wg->hdrs->setCookies);
                }

                /* close connection and reconnect with Authorization header */
                closeACSConnection(ht);
                ht->wio = wget_Connect(acsState.acsURL, Connected, NULL);

                if (ht->wio == NULL)
                {
                    vosLog_error("ACS Connect failed: %s", wget_LastErrorMsg());
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);
                    return;
                }
                ht->eHttpState = eConnected;
                /* copy cookies from previous connection to the new one*/
                if (glbCookieHdr != NULL)
                {
                    copyCookies(&(ht->wio->cookieHdrs), glbCookieHdr);
                }
                /* return here since sendInformData will be called in Connected*/
                return;
            }
            else if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)  /*lixuefei 20090723*/
            {
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_VERIFY_FAIL, CMC_TR69C_REMOTE_INFORM_STATUS);
            }

            /* now just resend the last data with the Authorization header */
            sendInformData();
            /* now we just return to wait on the response from the server */
#ifdef FORCE_NULL_AFTER_INFORM
            vosLog_debug("set Timer to Force Null send to ACS (Cisco tool)");
            utilTmr_set(tmrHandle, informRspTimeout, ht, 1 * 1000,
                        "informRspTimeout"); /******** ?????????????CISCO TOOL ???????? send null if server doesn't respond */
#endif
        }
        else if (wg->hdrs->status_code  >= 300 &&  wg->hdrs->status_code <= 307)
        {
            if (wg->hdrs->locationHdr != NULL)
            {
                /* Redirect status with new location */
                /* repost msg to new URL */
                closeACSConnection(ht);
                ht->eHttpState = eStart;
                vosLog_debug("Redirect to %s", wg->hdrs->locationHdr);
                ht->wio = wget_Connect(wg->hdrs->locationHdr, Connected,  NULL);

                if (ht->wio == NULL)
                {
                    vosLog_error("Redirect failed: %s", wget_LastErrorMsg());
                    acsDisconnect(ht, eConnectError);
                }
            }
            else
            {
                vosLog_error("Redirect failed: location header is empty");
                acsDisconnect(ht, eConnectError);
            }
        }
        else
        {
            /* If status != 401 after inform, and eAuthState == sIdle,
             * we're in the wrong authentication state.  The correct state should be from
             * sAuthenticating to sAutenticated so disconnect unless
             * ACS does not have any authentications and returns status_code == 200
             * right after CPE sends the first Inform message
             */
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                if (404 == wg->hdrs->status_code)
                {
                    CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_INTERRUPT, CMC_TR69C_REMOTE_INFORM_STATUS);    //inform 404
                }
            }

            if (ht->eAuthState == sIdle && wg->hdrs->status_code != 200)
            {
                ht->eAuthState = sAuthFailed;
                /* disconnect and delay */
                acsDisconnect(ht, eAuthError);
                vosLog_debug("Error: From sIdle -> sAuthenticated w/t sAuthenticating");
                return;
            }

            ht->eAuthState = sAuthenticated;
            if (informState == eACSNeverContacted)
            {
                informState = eACSContacted;
            }

            if ((wg->hdrs->status_code == 200) &&
                    ((wg->hdrs->content_length > 0) ||
                     (wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding, "chunked"))))
            {
                /* allocate buffer and parse the response */
                int     mlth;
                char    *soapmsg;

                /* msg posted - free buffer */
                VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                soapmsg = readResponse(wg, &mlth, 0);

                /* if TCP connection is closed by ACS*/
                if (ht->eHttpState == eClose)
                {
                    /* save cookies of the current connection*/
                    if (wg->hdrs->setCookies != NULL)
                    {
                        copyCookies(&glbCookieHdr, wg->hdrs->setCookies);
                    }
                    /*close connection and reconnect since ACS asks for it*/
                    closeACSConnection(ht);
                    ht->wio = wget_Connect(acsState.acsURL, handleSoapMessageCallBack, soapmsg);
                    if (ht->wio == NULL)
                    {
                        vosLog_error("ACS Connect failed: %s", wget_LastErrorMsg());
                        VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                        VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->authHdr);
                        return;
                    }
                    /*return here since handleSoapMessage will be called in handleSoapMessageCallBack*/
                    return;
                }

                handleSoapMessage(soapmsg, mlth);
            }
            else if ((wg->hdrs->status_code == 204)
                     || (wg->hdrs->status_code == 200 && !wg->hdrs->TransferEncoding))
            {
                /* only terminate session if no pending*/
                if (transferCompletePending == 0 && sendGETRPC == 0)
                {
                    /* empty ACS message -- ACS is done */
                    vosLog_debug("empty ACS msg - sentACSNull=%d", sentACSNull);
                    /* msg posted - free buffer */
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                    if (!sentACSNull)
                    {
                        sendNullHttp(FALSE);
                    }
                    if (wg->hdrs->status_code == 200 && ht->eHttpState == eClose)
                    {
                        closeACSConnection(ht);
                    }
                    else
                    {
                        acsDisconnect(ht, eAcsDone);
                        /* TR-069 session is finished*/
                        sessionState = eSessionEnd;
                        /*free cookies since tr-069 session is finished*/
                        if (glbCookieHdr != NULL)
                        {
                            freeCookies(&glbCookieHdr);
                            glbCookieHdr = NULL;
                        }
                    }
                }
                else
                {
                    if (1 == transferCompletePending)
                    {
                        /* transfer complete is pending so send it when receive empty message*/
                        sendTransferComplete();
                        transferCompletePending = 0;
                        /* setACSContactedState to eACSInformed for clearing*/
                        /* previous state which is eACSDownloadReboot or eACSUpload*/
                        setInformState(eACSInformed);
                    }
                    else if (1 == sendGETRPC)
                    {
                        sendGetRPCMethods();
                        sendGETRPC = 0;
                    }
                }
            }
            else if (100 == wg->hdrs->status_code)
            {
                /* 100 Continue: Just ignore this status */
                /* msg posted - free buffer */
                VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
            }
            else if (wg->hdrs->status_code >= 300
                     && wg->hdrs->status_code <= 307
                     && wg->hdrs->locationHdr)
            {
                /* Redirect status with new location */
                /* repost msg to new URL */
                closeACSConnection(ht);
                ht->eHttpState = eStart;
                vosLog_debug("Redirect to %s", wg->hdrs->locationHdr);
                ht->wio = wget_Connect(wg->hdrs->locationHdr, Connected, NULL);

                if (NULL == ht->wio)
                {
                    vosLog_error("Redirect failed: %s", wget_LastErrorMsg());
                    acsDisconnect(ht, ePostError);
                }
            }
            else
            {
                /* msg posted - free buffer */
                VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
                vosLog_error("Unknown status_code=%d received from ACS or encoding",
                             wg->hdrs->status_code);
                acsDisconnect(ht, ePostError);
            }
        }
    }
    else
    {
        vosLog_error("Post to ACS failed, Status = %d %s\n", wg->status, wg->msg);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
        acsDisconnect(ht, ePostError);
    }
}  /* End of postComplete() */

/*
 *  This routine store an events to the informEvList.
 *  sendInformData will look at this informEvList and
 *  compose the inform message when it's ready to be sent
 */
/*  Replace setupEventList, putInformEventMmethod and putInformEvent */

UINT32 addInformEventToList(UINT8 event)
{
    int   i;

    vosLog_debug("entered with event=%d", event);

    /*
    * Check for duplicate event codes in the informEvList.
    * We only need each event to appear once in the List/array.
    * Single cumulative behavior cannot be repeated, but multiple can.
    * At this point, multiple ones are added according to ACSstate.-- May need to change this to rely less on global var.
    */
    for (i = 0; i < informEvList.informEvCnt; i++)
    {
        if (informEvList.informEvList[i] == event)
        {
            vosLog_debug("event %d is already in informEvList, do nothing", event);
            return 0;
        }
    }

    if (informEvList.informEvCnt < MAXINFORMEVENTS)
    {
        informEvList.informEvList[informEvList.informEvCnt] = event;
        vosLog_debug("adding event %d to informEvList, count is now %d", event, informEvList.informEvCnt);
        informEvList.informEvCnt++;
        return informEvList.informEvCnt;
    }
    else
    {
        vosLog_error("too many events in informEvList, count=%d", informEvList.informEvCnt);
    }

    return 0;
}


/*
 * The connection to the ACS has been completed or
 * an error has ocurred.
 */
static void Connected(void *handle)
{
    HttpTask *ht = &httpTask;
    tWget    *wg = (tWget *)handle;

    vosLog_debug("=====>ENTER");

    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
    {
        CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NO_RESPONSE, CMC_TR69C_REMOTE_INFORM_STATUS);  //inform success
    }

    if (wg->status != 0)
    {
        VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
        acsDisconnect(ht, eConnectError);
        vosLog_error("ACS Connect Status = %d %s", wg->status, wg->msg);
        return;
    }

    if (ht->wio == NULL)
    {
        ht->eHttpState = eClosed;
        vosLog_error("Error -- pointer to IO desc is NULL");
        return;
    }

    ht->eHttpState = eConnected;

    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
    {
        CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_INTERRUPT, CMC_TR69C_REMOTE_INFORM_STATUS);  //inform halt
    }

    sendInformData();
}  /* End of Connected() */

/*
 * Send the current buffer to the ACS. This is an async call. The
 * return status only indicates that the connection has started.
 * The httpTask structure represents the only connection to an
 * ACS. If the connection is up then it is reused; otherwise,
 * a new connection is attempted.
 */
void sendToAcs(int contentLen, char *buf)
{
    HttpTask *ht = getHttpTask();

    vosLog_debug("=====>ENTER");

    if (ht->postMsg)
    {
        vosLog_error("postMsg buffer not null");
        VOS_MEM_FREE_BUF_AND_NULL_PTR(ht->postMsg);
    }

    if (getAcsConnDesc() == NULL)
    {
        vosLog_error("Try to send message to ACS while connection is NULL!");
        return;
    }

    ht->content_len = contentLen;
    ht->postMsg     = buf;
    ht->postLth     = buf ? util_strlen(buf) : 0;
    wget_ClearPostHdrs(ht->wio);
#ifdef GENERATE_SOAPACTION_HDR
    wget_AddPostHdr(ht->wio, "SOAPAction", "");
#endif
    updateAuthorizationHdr(ht);

    if (ht->eHttpState == eClose)
    {
        wget_PostDataClose(ht->wio, ht->postMsg, ht->postLth, "text/xml",
                           postComplete, (void *) ht);
    }
    else
    {
        wget_PostData(ht->wio, ht->postMsg, ht->postLth, "text/xml",
                      postComplete, (void *)ht);
    }
}  /* End of sendToAcs() */

/*
 * Send an Inform message. The possible handle values are
 * eIEConnectionRequest
 * eIETransferComplete
 * eIEJustBooted.
 * The other possible events are set by xxxPending flags.
 * eIEPeriodix
 * eIEDiagnostics
 *
 */
void sendInform(void *dummy)
{
    int numOfValueChanged = (int)getMdmParamValueChanges();
    UtilTimestamp now;
    UINT32 deltaMs;
    int timeToSend = 1;
    LimitNotificationInfo *ptr;
    UINT32 eventCount, notificationEvent = 0;
    int checkwanresult = 0;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    eventCount = (UINT32)informEvList.informEvCnt;
    if (0 == eventCount)
    {
        /* inform has been sent, there is nothing to send now */
        vosLog_debug("inform has been sent, there is nothing to send");
        return;
    }

    if (rebootingFlag)
    {
        /* system is rebooting, we don't send inform */
        vosLog_debug("system is rebooting, we don't send inform");
        return;
    }

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        if (enblCTMiddleware == CTMDW_MODE_0)
        {
            vosLog_debug("CTMiddleware is enabled, sendInform return!");
            return;
        }
    }

    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
    {
        /*init the inform status:unresponse*/
        if (acsState.acsURL != NULL && acsState.acsURL[0] != '\0')
        {
            CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NO_RESPONSE, CMC_TR69C_REMOTE_INFORM_STATUS);
        }
    }

    if (isAcsConnected())
    {
#if 0
        /* this change doesn't seem to be needed anymore.   We should just return if there is
           a session going on.   When the session ends, we can try to call inform again.
         */
        if (informState == eACSNeverContacted)
        {
            /* need to close connection to avoid system crash problem
             * when system is booted up without URL, then its value is changed
             * from WEB UI, then connection request is performed, then system is crashed
             * ==> call closeACSConnection to fix CR #16299
             */
            closeACSConnection(&httpTask);
        }
        else
        {
            /* inform needs to be sent, however, there is an outstanding
             * session going on.  Inform needs to sent in the next session.
             * (TR69 amendment 2, section 3.7) Inform message must not occur more
             * than once during a session unless this is retry.
             * Interop with Commtrend ACS, it is slow in sending empty http in Ping Test.
             */
            utilTmr_set(tmrHandle, sendInform, NULL, 5 * MSECS_IN_SEC, "sendInformWhenSessionEnds");
            return;
        }
#else
        vosLog_debug(">>>>>>There is already one AcsCon exist, retry after 20000 ms.\n");
        utilTmr_replaceIfSooner(tmrHandle, sendInform, NULL, 20000, "sendInformWhenSessionEnds");
        return;
#endif
    }

    sentACSNull = 0;

    if (acsState.acsURL != NULL && acsState.acsURL[0] != '\0')
    {
        HttpTask *ht = &httpTask;
        UBOOL8 changed = FALSE;
        int i;

        /* Before inform is sent for valueChanged, we need to see if this is a notification that has limitNotification
         * set by ACS.  If it is, inform should not be sent out; set a timer to send later.
         * If this notification is sent for other purposes or
         * valueChanged of a different parameter, the limitNotification parameter is sent anyway in the same inform.
         * At this point, we cancel the pending notification.
         */
        if (limitNotificationList.count != 0)
        {

            /* are we sending notification for value changes? */
            for (i = 0; i < (int)eventCount; i++)
            {
                if (informEvList.informEvList[i] == eIEValueChanged)
                {
                    notificationEvent = 1;
                    break;
                }
            }

            if (notificationEvent && (limitNotificationList.count >= numOfValueChanged))
            {
                /* there is a notification that needs to be sent, just to see this is one of those
                 * in limit notification list
                 */
                ptr = limitNotificationList.limitEntry;
                utilTms_get(&now);

                for (; ptr != NULL; ptr = ptr->next)
                {

                    ret = CMC_phlIsParamValueChanged(ptr->parameterFullPathName, &changed);

                    if (ret != VOS_RET_SUCCESS)
                    {
                        continue;
                    }

                    if (changed)
                    {
                        deltaMs = utilTms_deltaInMilliSeconds(&now, &ptr->lastSent);
                        if (deltaMs < (UINT32)(ptr->limitValue))
                        {
                            /* it's not time to send inform yet for this event.
                            * If there is another parameter that can send, then this event can go with it.
                            */
                            if (ptr->notificationPending == 0)
                            {
                                vosLog_debug("timerSet: deltaMs %d, limValue %d, toBeSent in %d ms",
                                             deltaMs, ptr->limitValue, (ptr->limitValue - deltaMs));

                                utilTmr_set(tmrHandle, ptr->func, NULL, ((UINT32)ptr->limitValue - deltaMs), "limitNotification");
                                ptr->notificationPending = 1;
                            }

                            timeToSend = 0;
                            continue;
                        }
                        else
                        {
                            /* it's time to send! */
                            timeToSend = 1;
                            break;
                        }
                    } /* param changed */
                } /* loop notifcation list */
            } /* notification pending */
            if (!timeToSend)
            {
                return;
            }
        } /* limit Notification != NULL */

        vosLog_debug("Connect to ACS at %s", acsState.acsURL);
        ht->eHttpState = eStart;

        ht->wio = wget_Connect(acsState.acsURL, Connected, NULL);

        if (ht->wio == NULL)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                /*wan not selected, don't send inform.*/
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NOT_REPORT, CMC_TR69C_REMOTE_INFORM_STATUS);
                ret = CMC_wanGetTr69cWanConnState(&checkwanresult);

                if (ret != VOS_RET_SUCCESS)
                {
                    CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_WAN_CONN, CMC_TR69C_REMOTE_STATUS_MESSAGE);
                }
                else if (0 == checkwanresult)
                {
                    CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_EFFECT_WAN_CONN, CMC_TR69C_REMOTE_STATUS_MESSAGE);
                }
                else
                {
                    if (1 == g_dns_resolve_ret)
                    {
                        CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_DNS_FAILED, CMC_TR69C_REMOTE_STATUS_MESSAGE);
                    }
                    else if (2 == g_dns_resolve_ret)
                    {
                        CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NO_RESPONSE, CMC_TR69C_REMOTE_INFORM_STATUS);
                    }
                    else
                    {
                        CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_DNS_INFORM, CMC_TR69C_REMOTE_STATUS_MESSAGE);
                    }
                }
            }

            vosLog_debug("ACS Connect Failed: %s", wget_LastErrorMsg());
            vosLog_debug("set delayed inform timer for %d ", CHECKWANINTERVAL);
            /* mwang: this looks a bit wrong.  The handle is used to
            * look up this timer.  But handle is this function is actually
            * an eInformEvent enum.  So are we saying if we get here,
            * the eInformEvent enum will always be the same?
            */
            ret = utilTmr_replaceIfSooner(tmrHandle, sendInform, NULL, CHECKWANINTERVAL, "WANCHECK_inform");

            if (ret != VOS_RET_SUCCESS)
            {
                vosLog_error("setting delayed wan inform timer failed, ret=%d", ret);
            }
        }
        else if (ht->wio->status == -6)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NOT_REPORT, CMC_TR69C_REMOTE_INFORM_STATUS);
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_ACS_CFG, CMC_TR69C_REMOTE_STATUS_MESSAGE);
            }

            vosLog_error("acsUrl length is 0.");
        }
        else if (ht->wio->status == -5)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NOT_REPORT, CMC_TR69C_REMOTE_INFORM_STATUS);
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_DNS_FAILED, CMC_TR69C_REMOTE_STATUS_MESSAGE);
            }
            vosLog_error("acsUrl format is error.");
        }
        else
        {
            /* go through the whole notification list, cancel timers because they will all be sent now */
            /* in this case, everything will be sent out, so we cancel all timers and update lastSent field */
            ptr = limitNotificationList.limitEntry;
            while (ptr != NULL)
            {
                /* Everything will be sent, so we update our list.
                 * To simplify the whole implementation, I just update the lastSent for everyone in the list.
                 * LastSent is basically the last notification sent, not necessarily the notification of this
                 * parameter.
                 */
                if (ptr->notificationPending)
                {
                    vosLog_debug("timerCancel for ptr->parameterFullPathName %s", ptr->parameterFullPathName);
                    utilTmr_cancel(tmrHandle, ptr->func, NULL);
                    ptr->notificationPending = 0;
                }

                utilTms_get(&ptr->lastSent);
                ptr = ptr->next;
            } /* loop */
        } /* inform sent */
    } /* acs */
    else
    {
        if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
        {
            CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_NOT_REPORT, CMC_TR69C_REMOTE_INFORM_STATUS);
            ret = CMC_wanGetTr69cWanConnState(&checkwanresult);

            if (ret != VOS_RET_SUCCESS)
            {
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_WAN_CONN, CMC_TR69C_REMOTE_STATUS_MESSAGE);
            }
            else if (0 == checkwanresult)
            {
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_EFFECT_WAN_CONN, CMC_TR69C_REMOTE_STATUS_MESSAGE);
            }
            else
            {
                CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_NOT_REPORT_NO_ACS_CFG, CMC_TR69C_REMOTE_STATUS_MESSAGE);
            }
        }

        vosLog_debug("acsURL is NULL!");
    }
}  /* End of sendInform() */


static void sendInformData(void)
{
    RPCAction *a = newRPCAction();
    HttpTask  *ht = &httpTask;
    vosLog_debug("=====>ENTER");

    if (ht->eAuthState == sAuthenticating)
    {
        /*second Inform is ready to send out for authentication*/
        sessionState = eSessionAuthenticating;
    }
    else
    {
        /*first Inform is ready to send out to start TR-069 session*/
        sessionState = eSessionStart;
    }

    buildInform(a, &informEvList);
    backupInformEvList();
    freeRPCAction(a);
}  /* End of sendInformData() */


/*We need clean informEvList soon, but at that time, we should know which one
 we have not sent yet. So make a copy here.
*/
void backupInformEvList()
{
    vosLog_debug("******Send information to sever finished, backup \"informEvList\".\n");
    g_tr69cInformEvListCopy = informEvList;
}


/** Send msg to smd requesting a delayed msg.
 *
 * The delayed msg contains a special id (PERIODIC_INFORM_TIMEOUT_ID) in the
 * wordData field of the message header.  When I get a DELAYED_MSG with this
 * special id, I know it is time to do a periodic inform.
 * Requesting a delayed message will also cancel any previous requests with the
 * same id.
 */
void requestPeriodicInform(UINT32 interval)
{
    char buf[sizeof(VosMsgHeader) + sizeof(RegisterDelayedMsgBody)] = {0};
    VosMsgHeader *msg;
    RegisterDelayedMsgBody *body;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    msg = (VosMsgHeader *) buf;
    body = (RegisterDelayedMsgBody *)(msg + 1);

    msg->type = VOS_MSG_REGISTER_DELAYED_MSG;
    msg->src = EID_TR69C;
    msg->dst = EID_SMD;
    msg->flags_request = 1;
    msg->wordData = PERIODIC_INFORM_TIMEOUT_ID;
    msg->dataLength = sizeof(RegisterDelayedMsgBody);

    body->delayMs = interval * MSECS_IN_SEC;  /* tr69c uses seconds, smd uses ms */

    ret = vosMsg_sendAndGetReply(g_msgHandle, msg);
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("request failed, ret = %d", ret);
    }
    else
    {
        vosLog_debug("set next periodic inform for %u seconds in future", interval);
    }

    return;
}

void cancelPeriodicInform()
{
    VosMsgHeader msg;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    /*cancel local periodic inform timer in tr69c*/
    utilTmr_cancel(tmrHandle, periodicInformTimeout, (void *)NULL);

    memset(&msg, 0, sizeof(VosMsgHeader));

    msg.type = VOS_MSG_UNREGISTER_DELAYED_MSG;
    msg.src = EID_TR69C;
    msg.dst = EID_SMD;
    msg.flags_request = 1;
    msg.wordData = PERIODIC_INFORM_TIMEOUT_ID;

    /*send message from tr69c to smd so that smd will not wake up
    *tr69c when periodic inform is expired
    */
    ret = vosMsg_sendAndGetReply(g_msgHandle, &msg);
    if (ret != VOS_RET_SUCCESS && ret != VOS_RET_NO_MORE_INSTANCES)
    {
        vosLog_error("request failed, ret = %d", ret);
    }
}

/** Periodic Inform Timer callback.
 *
 * If peridic inform is enabled and informInterval > 0 then always create another
 * timer event.
 */
void periodicInformTimeout(void *handle __attribute__((unused)))
{
    addInformEventToList(INFORM_EVENT_PERIODIC);
    sendInform(NULL);

    resetPeriodicInform((UINT32)acsState.informInterval);
#ifdef USE_DMALLOC
    dmalloc_log_unfreed();
#endif // USE_DMALLOC
}  /* End of periodicInformTimeout() */


/*
* Called from setter function to update next inform time
*/
void resetPeriodicInform(UINT32 interval)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    int Inform_flag = 0;

    if (acsState.informEnable)
    {
        if (SF_FEATURE_ISP_CU)
        {
            int secondInformDelay;

            if (acsState.randomInformEnable)
            {
                Inform_flag = ReadRandomInformEnaleFromFile();
                if (Inform_flag == 1 && glbflag == 1)
                {
                    secondInformDelay = getRangeNumber(acsState.informInterval) % 60 + 30;
                    ret = utilTmr_replaceIfSooner(tmrHandle, periodicInformTimeout, (void *)NULL, (UINT32)secondInformDelay * MSECS_IN_SEC,
                                                  "periodic_inform");
                    glbflag--;
                    Inform_flag = 0;
                    WriteRandomInformEnableTofile(Inform_flag);
                }
                else
                {
                    ret = utilTmr_replaceIfSooner(tmrHandle, periodicInformTimeout, (void *)NULL, interval * MSECS_IN_SEC,
                                                  "periodic_inform");
                }
            }
            else
            {
                ret = utilTmr_replaceIfSooner(tmrHandle, periodicInformTimeout, (void *)NULL, interval * MSECS_IN_SEC,
                                              "periodic_inform");
            }

            if (glbflag)
            {
                glbflag --;
            }
        }
        else
        {
            ret = utilTmr_replaceIfSooner(tmrHandle, periodicInformTimeout, (void *)NULL, interval * MSECS_IN_SEC,
                                          "periodic_inform");
        }

        if (ret != VOS_RET_SUCCESS)
        {
            vosLog_error("could not set timer, ret = %d", ret);
        }
    }
    else
    {
        utilTmr_cancel(tmrHandle, periodicInformTimeout, (void *)NULL);
    }
}

void scheduleInformTimeout(void *handle __attribute__((unused)))
{
    addInformEventToList(INFORM_EVENT_SCHEDULED);
    addInformEventToList(INFORM_EVENT_SCHEDULE_METHOD);
    sendInform(NULL);
}

void setScheduleInform(UINT32 interval)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    ret = utilTmr_replaceIfSooner(tmrHandle, scheduleInformTimeout, (void *)NULL, interval * MSECS_IN_SEC,
                                  "schedule_inform");

    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("could not set timer, ret = %d", ret);
    }
}

/*
* This can only be called after doDownload() has already sent the Download response.
* It is called by the httpDownload functions if an error occurs.
* Thus we need to send a TransferComplete message here.
* If the ACS is not connected then an Inform needs to be started.
*/
void sendDownloadFault()
{
    if (isAcsConnected())
    {
        transferCompletePending = 1;
        vosLog_debug("acs is connected -- set transferCompletePending");
    }
    else
    {
        vosLog_debug("acs is not connected -- sendInform(TransferComplete)");
        transferCompletePending = 1;
        addInformEventToList(INFORM_EVENT_TRANSER_COMPLETE);
        sendInform(NULL);
    }

    updateTransferState(acsState.downloadCommandKey, eTransferCompleted);
}  /* End of sendDownloadFault() */



/** This is only called at tr69c startup from initInformer.
 *
 */
void startACSComm(void *handle  __attribute__((unused)))
{
    /* Initialize server socket for receiving connection requests from ACS. */
    startACSListener();
    sendInform(NULL);
    resetPeriodicInform((UINT32)acsState.informInterval);

    if (SF_FEATURE_SUPPORT_PPPOE_SNOOPING)
    {
        utilTmr_set(tmrHandle, getBridgeUsername, NULL, 20 * 1000, "getBridgeName");
    }
}  /* End of startACSComm() */


static UBOOL8 isAlarmOrMonitorInList(int isAlarm)
{
    int i = 0;

    vosLog_debug("isAlarm = %d", isAlarm);

    for (i = 0; i < informEvList.informEvCnt; i++)
    {
        if (isAlarm)
        {
            if ((INFORM_EVENT_CT_ALARM == informEvList.informEvList[i])
                    || (INFORM_EVENT_CLEAR_CT_ALARM == informEvList.informEvList[i]))
            {
                vosLog_debug("isAlarm event is not inform");
                break;
            }
        }
        else
        {
            if (INFORM_EVENT_CT_MONITOR == informEvList.informEvList[i])
            {
                vosLog_debug("isAlarm event is not inform");
                break;
            }
        }
    }

    if (i == informEvList.informEvCnt)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/* chenlianzhong +: 2011-2-15 增加设备监控和告警*/
void InsertSend(int alert, SINT64 max, SINT64 min, char *plist, int *inalarmstatus)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char *paramValue = NULL;

    vosLog_debug("max = %lld, min =%lld", max, min);

    if (IS_EMPTY_STRING(plist))
    {
        vosLog_notice("pList is NULL");
        return;
    }

    if (!alert && SF_FEATURE_SUPPORT_MONITOR_COLLECTOR)
    {
        char *p = NULL;
        char *paralist = VOS_STRDUP(plist);

        if (!isAlarmOrMonitorInList(0))
        {
            p = strtok(paralist, ",");

            if (send_monitor_number < 50 && VOS_RET_SUCCESS == tr69c_getParamValue(p, &paramValue))
            {
                UTIL_STRNCPY(monitorSend[send_monitor_number].paralist,
                             p, sizeof(monitorSend[send_monitor_number].paralist));

                UTIL_STRNCPY(monitorSend[send_monitor_number].value,
                             paramValue, sizeof(monitorSend[send_monitor_number].value));
                send_monitor_number++;
            }
            VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);

            while ((p = strtok(NULL, ",")))
            {
                if (send_monitor_number < 50 && VOS_RET_SUCCESS ==  tr69c_getParamValue(p, &paramValue))
                {
                    UTIL_STRNCPY(monitorSend[send_monitor_number].paralist,
                                 p, sizeof(monitorSend[send_monitor_number].paralist));

                    UTIL_STRNCPY(monitorSend[send_monitor_number].value,
                                 paramValue, sizeof(monitorSend[send_monitor_number].value));
                    send_monitor_number++;
                }
                VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
            }

            VOS_MEM_FREE_BUF_AND_NULL_PTR(paralist);

        }
        return;
    }

    ret = tr69c_getParamValue(plist, &paramValue);
    if (VOS_RET_SUCCESS == ret)
    {
        SINT64 valueInt;

        util_strtol64(paramValue, NULL, 0, &valueInt);
        vosLog_debug("valueInt = %lld", valueInt);
        if (1 == alert)
        {
            if (SF_FEATURE_SUPPORT_TR69C_ALARM)
            {
                if ((valueInt > max) || (valueInt < min))
                {
                    if (send_alarm_number < 50)
                    {
                        UTIL_STRNCPY(alarmSend[send_alarm_number].paralist,
                                     plist, sizeof(alarmSend[send_alarm_number].paralist));
                        UTIL_STRNCPY(alarmSend[send_alarm_number].value, paramValue,
                                     sizeof(alarmSend[send_alarm_number].value));
                        send_alarm_number++;
                        *inalarmstatus = 1;
                    }
                }
                else
                {
                    if (1 == *inalarmstatus)
                    {
                        UTIL_STRNCPY(cleanalarmSend[send_clean_alarm_number].paralist,
                                     plist, sizeof(cleanalarmSend[send_clean_alarm_number].paralist));
                        UTIL_STRNCPY(cleanalarmSend[send_clean_alarm_number].value,
                                     paramValue, sizeof(cleanalarmSend[send_clean_alarm_number].value));
                        send_clean_alarm_number++;
                        *inalarmstatus = 0;
                    }
                }
            }
        }
        else
        {
            vosLog_debug("monitor ");
            if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
            {
                if (send_monitor_number < 50)
                {
                    UTIL_STRNCPY(monitorSend[send_monitor_number].paralist,
                                 plist, sizeof(monitorSend[send_monitor_number].paralist));
                    UTIL_STRNCPY(monitorSend[send_monitor_number].value,
                                 paramValue, sizeof(monitorSend[send_monitor_number].value));
                    send_monitor_number++;
                }
            }
        }

        VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
    }
    else
    {
        vosLog_error("tr69c_getParamValue failed, ret = %d", ret);
    }
}


void getNumberAndStatus(int alert, int *allnumber, int *status)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (1 == alert)
    {
        if (SF_FEATURE_SUPPORT_TR69C_ALARM)
        {
            CMC_TR69C_ALARM_CFG_T pAlarm;

            memset(&pAlarm, 0, sizeof(pAlarm));
            if ((ret = CMC_tr69cGetAlarm(&pAlarm)) != VOS_RET_SUCCESS)
            {
                vosLog_error("Could not get MDMOID_ALARM, ret = %d", ret);
                return;
            }

            *allnumber = (int)pAlarm.NumOfEntries;
            *status = pAlarm.enable;
            vosLog_debug("alarmAllNumber = %d ,alarmStatus = %d", *allnumber, *status);
        }
    }
    else
    {
        if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
        {
            CMC_TR69C_MONITOR_CFG_T monitorObj;
            memset(&monitorObj, 0, sizeof(monitorObj));

            ret = CMC_tr69cGetMonitor(&monitorObj);
            if (VOS_RET_SUCCESS == ret)
            {
                *allnumber = (int)monitorObj.NumOfEntries;
                *status = monitorObj.enable;
            }
            else
            {
                vosLog_error("CMC_tr69cGetMonitor failed! (ret = %d)", ret);
            }
        }
    }
}


void dealMonitorTimer(void *id)
{
    TR69C_MONITOR_T *pMonitor = pMonitorHead->next;
    UINT32 MonitorId = *(UINT32 *)id;
    UBOOL8 found = FALSE;

    vosLog_debug("Enter, MonitorId = %d", MonitorId);

    if (!isAlarmOrMonitorInList(0))
    {
        memset(monitorSend, 0, sizeof(CT_ALARMORMONITOR_SEND) * 50);
        send_monitor_number = 0;
    }

    while (pMonitor != NULL)
    {
        if (pMonitor->monitorCfg.instanceId == MonitorId)
        {
            found = TRUE;
            InsertSend(0, 0, 0, pMonitor->monitorCfg.paramList, NULL);
            break;
        }

        pMonitor = pMonitor->next;
    }

    vosLog_debug("send_monitor_number = %d", send_monitor_number);
    if (send_monitor_number > 0)
    {
        addInformEventToList(INFORM_EVENT_CT_MONITOR);
        sendInform(NULL);
    }

    if (found)
    {
        vosLog_debug("found = %d", found);
        utilTmr_set(tmrHandle, dealMonitorTimer, (void *)&pMonitor->monitorCfg.instanceId,
                    (UINT32)pMonitor->monitorCfg.time * 1000, "monitor_fail_backoff_inform");
    }
}


void dealAlarmTimer(void *id)
{
    TR69C_ALARM_T *pAlarm = pAlarmHead->next;
    UINT32 alarmId = *(UINT32 *)id;
    UBOOL8 found = FALSE;

    vosLog_debug("Enter>, pAlarmHead = %p, alarmId =%d", pAlarmHead, alarmId);

    if (!isAlarmOrMonitorInList(1))
    {
        memset(alarmSend, 0, sizeof(CT_ALARMORMONITOR_SEND) * 50);
        send_alarm_number = 0;
        send_clean_alarm_number = 0;
    }

    while (pAlarm != NULL)
    {
        if (pAlarm->alarmCfg.instanceId == alarmId)
        {
            found = TRUE;
            InsertSend(1, pAlarm->alarmCfg.limitMax, pAlarm->alarmCfg.limitMin, pAlarm->alarmCfg.paramlist,
                       &pAlarm->alarmCfg.inAlarmStatus);
            break;
        }

        pAlarm = pAlarm->next;
    }

    if (send_alarm_number > 0)
    {
        addInformEventToList(INFORM_EVENT_CT_ALARM);
        sendInform(NULL);
    }

    vosLog_debug("send_alarm_number = %d, send_clean_alarm_number=%d", send_alarm_number, send_clean_alarm_number);
    if (send_clean_alarm_number > 0)
    {
        addInformEventToList(INFORM_EVENT_CLEAR_CT_ALARM);
        sendInform(NULL);
    }

    if (found)
    {
        utilTmr_set(tmrHandle, dealAlarmTimer, (void *)&pAlarm->alarmCfg.instanceId, (UINT32)pAlarm->alarmCfg.time * 1000,
                    "alarm_fail_backoff_inform");
    }
}


void changeAlarm(UINT32 alarmId)
{
    TR69C_ALARM_T *preAlarm = pAlarmHead;
    TR69C_ALARM_T *pAlarm = preAlarm->next;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UBOOL8 found = FALSE;
    int status = 0;

    vosLog_debug("Enter>, alarmId = %d ", alarmId);

    getNumberAndStatus(1, &alarmAllNumber, &alarmStatus);

    while (pAlarm && alarmStatus)
    {
        if (pAlarm->alarmCfg.instanceId == alarmId)
        {
            found = TRUE;

            PUSH_INSTANCE_ID(&iidStack, alarmId);
            utilTmr_cancel(tmrHandle, dealAlarmTimer, (void *) & (pAlarm->alarmCfg.instanceId));

            status = pAlarm->alarmCfg.inAlarmStatus;

            ret = CMC_tr69cGetAlarmCfg(&(pAlarm->alarmCfg), &iidStack);
            if (VOS_RET_SUCCESS == ret)
            {
                vosLog_debug("edit ");
                if (!IS_EMPTY_STRING(pAlarm->alarmCfg.paramlist))
                {
                    utilTmr_set(tmrHandle, dealAlarmTimer, (void *) & (pAlarm->alarmCfg.instanceId), (UINT32)pAlarm->alarmCfg.time * 1000,
                                "alarm_fail_backoff_inform");
                }

                pAlarm->alarmCfg.inAlarmStatus = status;
            }
            else
            {
                vosLog_debug("del");
                preAlarm->next = pAlarm->next;
                VOS_MEM_FREE_BUF_AND_NULL_PTR(pAlarm);
            }

            break;
        }

        preAlarm = pAlarm;
        pAlarm = pAlarm->next;

    }

    vosLog_debug("alarmStatus = %d", alarmStatus);
    if (!found && alarmStatus)
    {
        PUSH_INSTANCE_ID(&iidStack, alarmId);
        pAlarm = (TR69C_ALARM_T *)VOS_MALLOC(sizeof(TR69C_ALARM_T));
        if (VOS_RET_SUCCESS == (ret = CMC_tr69cGetAlarmCfg(&(pAlarm->alarmCfg), &iidStack)))
        {
            preAlarm->next = pAlarm;

            vosLog_debug("pAlarm->alarmCfg.instanceId = %d", pAlarm->alarmCfg.instanceId);
            if (!IS_EMPTY_STRING(pAlarm->alarmCfg.paramlist))
            {
                utilTmr_set(tmrHandle, dealAlarmTimer, (void *) & (pAlarm->alarmCfg.instanceId), (UINT32)pAlarm->alarmCfg.time * 1000,
                            "alarm_fail_backoff_inform");
            }
            else
            {
                vosLog_debug("alarmCfg.paramlis is NULL");
            }
        }
        else
        {
            vosLog_error("CMC_tr69cGetAlarmCfg failed, ret = %d", ret);
        }
    }
}


void changeMonitor(UINT32 monitorId)
{
    TR69C_MONITOR_T *preMonitor = pMonitorHead;
    TR69C_MONITOR_T *pMonitor = preMonitor->next;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UBOOL8 found = FALSE;

    vosLog_debug("Enter>, monitorId = %d ", monitorId);

    getNumberAndStatus(0, &monitorAllNumber, &monitorStatus);

    while (pMonitor && monitorStatus)
    {
        if (pMonitor->monitorCfg.instanceId == monitorId)
        {
            found = TRUE;

            PUSH_INSTANCE_ID(&iidStack, monitorId);
            utilTmr_cancel(tmrHandle, dealMonitorTimer, (void *) & (pMonitor->monitorCfg.instanceId));

            ret = CMC_tr69cGetMonitorCfg(&(pMonitor->monitorCfg), &iidStack);
            if (VOS_RET_SUCCESS == ret)
            {
                vosLog_debug("edit ");
                if (!IS_EMPTY_STRING(pMonitor->monitorCfg.paramList))
                {
                    utilTmr_set(tmrHandle, dealMonitorTimer, (void *) & (pMonitor->monitorCfg.instanceId),
                                (UINT32)pMonitor->monitorCfg.time * 1000, "alarm_fail_backoff_inform");
                }
                else
                {
                    vosLog_debug("monitorCfg.paramList is NULL");
                }
            }
            else
            {
                vosLog_debug("del");
                preMonitor->next = pMonitor->next;
                VOS_MEM_FREE_BUF_AND_NULL_PTR(pMonitor);
            }

            break;
        }

        preMonitor = pMonitor;
        pMonitor = pMonitor->next;

    }

    vosLog_debug("monitorStatus = %d", monitorStatus);

    if (!found && monitorStatus)
    {
        PUSH_INSTANCE_ID(&iidStack, monitorId);
        pMonitor = (TR69C_MONITOR_T *)VOS_MALLOC(sizeof(TR69C_MONITOR_T));
        if (VOS_RET_SUCCESS == (ret = CMC_tr69cGetMonitorCfg(&(pMonitor->monitorCfg), &iidStack)))
        {
            preMonitor->next = pMonitor;

            vosLog_debug("pMonitor->monitorCfg.instanceId =%d", pMonitor->monitorCfg.instanceId);
            if (!IS_EMPTY_STRING(pMonitor->monitorCfg.paramList))
            {
                utilTmr_set(tmrHandle, dealMonitorTimer, (void *) & (pMonitor->monitorCfg.instanceId),
                            (UINT32)pMonitor->monitorCfg.time * 1000, "alarm_fail_backoff_inform");
            }
            else
            {
                vosLog_debug("monitorCfg.paramList is NULL");
            }
        }
        else
        {
            vosLog_error("CMC_tr69cGetMonitorCfg failed ret = %d", ret);
        }
    }
}


void startAlarm()
{
    TR69C_ALARM_T *pAlarm = NULL;
    TR69C_ALARM_T *pHead = NULL;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    memset(alarmSend, 0, sizeof(CT_ALARMORMONITOR_SEND) * 50);
    send_alarm_number = 0;
    send_clean_alarm_number = 0;

    getNumberAndStatus(1, &alarmAllNumber, &alarmStatus);

    pAlarmHead = (TR69C_ALARM_T *)VOS_MALLOC(sizeof(TR69C_ALARM_T));
    if (NULL == pAlarmHead)
    {
        vosLog_error("VOS_MALLOC failed ");
    }

    pHead = pAlarmHead;
    pHead->next = NULL;

    if (alarmStatus != 0 && alarmAllNumber > 0)
    {
        pAlarm = (TR69C_ALARM_T *)VOS_MALLOC(sizeof(TR69C_ALARM_T));
        if (NULL == pAlarm)
        {
            vosLog_error("VOS_MALLOC failed ");
            return;
        }

        while (VOS_RET_SUCCESS == (ret = CMC_tr69cGetNextAlarmEntery(&(pAlarm->alarmCfg), &iidStack)))
        {
            if (!IS_EMPTY_STRING(pAlarm->alarmCfg.paramlist))
            {
                utilTmr_set(tmrHandle, dealAlarmTimer, (void *) & (pAlarm->alarmCfg.instanceId), (UINT32)pAlarm->alarmCfg.time * 1000,
                            "alarm_fail_backoff_inform");
            }
            else
            {
                vosLog_debug("alarmCfg.paramlist is NULL");
            }

            pAlarm->next = pHead->next;
            pHead->next = pAlarm;
            pAlarm = (TR69C_ALARM_T *)VOS_MALLOC(sizeof(TR69C_ALARM_T));
            if (NULL == pAlarm)
            {
                vosLog_error("VOS_MALLOC failed ");
            }
        }

        VOS_MEM_FREE_BUF_AND_NULL_PTR(pAlarm);
    }
    else if (!alarmStatus)
    {
        while (pHead->next)
        {
            pAlarm = pHead->next;
            pHead->next = pAlarm->next;
            utilTmr_cancel(tmrHandle, dealAlarmTimer, (void *) & (pAlarm->alarmCfg.instanceId));
            VOS_MEM_FREE_BUF_AND_NULL_PTR(pAlarm);
        }
    }
}


void startMonitor()
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    TR69C_MONITOR_T *pHead = NULL;
    TR69C_MONITOR_T *pMonitor = NULL;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;

    vosLog_debug("Enter>");

    memset(monitorSend, 0, sizeof(CT_ALARMORMONITOR_SEND) * 50);
    send_monitor_number = 0;

    getNumberAndStatus(0, &monitorAllNumber, &monitorStatus);
    pMonitorHead = (TR69C_MONITOR_T *)VOS_MALLOC(sizeof(TR69C_MONITOR_T));

    if (NULL == pMonitorHead)
    {
        vosLog_error("VOS_MALLOC failed ");
    }

    pHead = pMonitorHead;
    pHead->next = NULL;

    if (monitorStatus != 0 && monitorAllNumber > 0)
    {
        pMonitor = (TR69C_MONITOR_T *)VOS_MALLOC(sizeof(TR69C_MONITOR_T));
        if (NULL == pMonitor)
        {
            vosLog_error("VOS_MALLOC failed");
            return;
        }

        while (VOS_RET_SUCCESS == (ret = CMC_tr69cGetNextMonitorEntery(&(pMonitor->monitorCfg), &iidStack)))
        {
            if (!IS_EMPTY_STRING(pMonitor->monitorCfg.paramList))
            {
                utilTmr_set(tmrHandle, dealMonitorTimer, (void *) & (pMonitor->monitorCfg.instanceId),
                            (UINT32)pMonitor->monitorCfg.time * 1000, "alarm_fail_backoff_inform");
            }
            else
            {
                vosLog_debug("monitorCfg.paramList is NULL");
            }

            pMonitor->next = pHead->next;
            pHead->next = pMonitor;
            pMonitor = (TR69C_MONITOR_T *)VOS_MALLOC(sizeof(TR69C_MONITOR_T));
            if (NULL == pMonitor)
            {
                vosLog_error("VOS_MALLOC failed ");
            }
        }

        VOS_MEM_FREE_BUF_AND_NULL_PTR(pMonitor);
    }
    else if (!monitorStatus)
    {
        while (pHead->next)
        {
            pMonitor = pHead->next;
            pHead->next = pMonitor->next;
            utilTmr_cancel(tmrHandle, dealMonitorTimer, (void *) & (pMonitor->monitorCfg.instanceId));
            VOS_MEM_FREE_BUF_AND_NULL_PTR(pMonitor);
        }
    }
}


/* called once when tr69c starts - must be called AFTER all instances are initialized */
void initInformer(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    /* chenlianzhong +: 2011-2-15 增加电信规范要求的设备监控和设备告警功能*/
    if (SF_FEATURE_SUPPORT_TR69C_ALARM)
    {
        startAlarm();
    }

    if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
    {
        startMonitor();
    }

    /* start trying ACSComm in a moment (ACSINFORMDELAY=500ms) */
    ret = utilTmr_set(tmrHandle, startACSComm, NULL, ACSINFORMDELAY, "startACSComm");
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("could not set timer, ret=%d", ret);
    }

    initTransferList();
}  /* End of initInformer() */


void processIptvStbMac(void *handle)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char stbMac[BUFLEN_128] = {0};
    CMC_MCAST_IPTV_STB_MAC_T  iptvStbMac;
    UINT32 i = 1;
    char lines[BUFLEN_128] = {0};
    FILE *pfile = NULL;
    char mac[BUFLEN_32] = {0};
    char lanIfc[BUFLEN_8][BUFLEN_32] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};
    UINT32 portIndex = 0;
    char brName[BUFLEN_4] = {0};
    char brId[BUFLEN_32] = {0};
    char stp[BUFLEN_8] = {0};
    char isLocal[BUFLEN_8] = {0};
    char macbuf[BUFLEN_128] = {0};
    UBOOL8 isBr1Intf = FALSE;
    char *ptr = NULL;
    static UINT32 timer_counter = 0;

    pfile = popen("brctl show", "r");
    if (NULL != pfile)
    {
        while (fgets(lines, 128, pfile) != NULL)
        {
            if ((0 == util_strncasecmp(lines, "bridge", 6)) || (0 == util_strncasecmp(lines, "br0", 3)))
            {
                continue;
            }
            else if (0 == util_strncasecmp(lines, "br1", 3))
            {
                isBr1Intf = TRUE;
                if (4 == sscanf(lines, "%s %s %s %s", brName, brId, stp, lanIfc[i]))
                {
                    i++;
                }
            }

            if (isBr1Intf && (0 != util_strncasecmp(lines, "br1", 3)))
            {
                if (1 == sscanf(lines, "%s", lanIfc[i]))
                {
                    i++;
                }
            }

            if (i > 7)
            {
                break;
            }

            memset(lines, 0, sizeof(lines));
        }
        pclose(pfile);
        pfile = NULL;

        pfile = popen("brctl showmacs br1", "r");
        if (NULL != pfile)
        {
            while (fgets(lines, 128, pfile) != NULL)
            {
                if (0 == util_strncasecmp(lines, "port", 4))
                {
                    continue;
                }
                else
                {
                    if (3 == sscanf(lines, "%d %s %s", &portIndex, mac, isLocal))
                    {
                        if ((0 != util_strncasecmp(lanIfc[portIndex], "veip", 4)) && (0 != util_strncasecmp(lanIfc[portIndex], "epon", 4)) &&
                                (0 != util_strcmp(isLocal, "yes")))
                        {
                            vosLog_debug("index=%d, ifname=%s, isLocal=%s, mac=%s", portIndex, lanIfc[portIndex], isLocal, mac);
                            UTIL_STRNCAT(macbuf, mac, sizeof(macbuf));
                            UTIL_STRNCAT(macbuf, ";", sizeof(macbuf));
                        }
                    }
                }

                memset(lines, 0, sizeof(lines));
            }

            if (NULL != (ptr = strrchr(macbuf, ';')))
            {
                *ptr = '\0';
            }

            vosLog_debug("macbuf:%s", macbuf);

            pclose(pfile);
            pfile = NULL;
        }

        if (macbuf[0] != '\0')
        {
            timer_counter = 0;
            UTIL_STRNCPY(stbMac, macbuf, sizeof(stbMac));
            memset((void *)&iptvStbMac, 0, sizeof(iptvStbMac));
            UTIL_STRNCPY(iptvStbMac.STBMAC, stbMac, sizeof(iptvStbMac.STBMAC));

            ret = CMC_igmpSetIptvStbMac(&iptvStbMac);
            if (ret != VOS_RET_SUCCESS)
            {
                vosLog_error("process ping state changed msg failed,ret=%d", ret);
            }
            else
            {
                addInformEventToList(INFORM_EVENT_CT_STB_BIND);
                sendInform(NULL);
            }
        }
        else
        {
            if (timer_counter < 15)
            {
                timer_counter++;
                ret = utilTmr_set(tmrHandle, processIptvStbMac, NULL, 2 * 1000, "timer_response");
                if (ret != VOS_RET_SUCCESS)
                {
                    vosLog_error("could not set response timer, ret = %d", ret);
                }
            }
            else
            {
                timer_counter = 0;
            }
        }
    }
}


static int getBridgeUsernameIsOk(void)
{
    static SINT32 sfd = -1;
    struct ifreq ifr;
    char fe[64] = {0};
    unsigned long args[2] = { BRCTL_GET_BRIDGE_USERNAME, (unsigned long) fe};

    memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    UTIL_STRNCPY(ifr.ifr_name, "br0", sizeof(ifr.ifr_name));

    ifr.ifr_data = (char *) args;

    if (sfd < 0)
    {
        if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            return 0;
        }
    }

    ioctl(sfd, SIOCDEVPRIVATE, &ifr);

    if (0 != fe[0])
    {
        UTIL_STRNCPY(bridgeUserName, fe, sizeof(bridgeUserName));
        vosLog_error("Get PPPOE username from bridge WAN link : %s\n", bridgeUserName);
        vosLog_error("Send inform of BAND1\n");

        addInformEventToList(INFORM_BRIDGE_USERNAME);
        sendInform(NULL);

        close(sfd);
        sfd = -1;

        return 1;
    }

    return 0;
}


static void getBridgeUsername(void *handle)
{
    utilTmr_cancel(tmrHandle, getBridgeUsername, NULL);
    if (0 == getBridgeUsernameIsOk())
    {
        utilTmr_set(tmrHandle, getBridgeUsername, NULL, 20 * 1000, "getBridgeName");
    }
}
