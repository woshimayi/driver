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
//#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <sched.h>

#include "tr69cdefs.h"
#include "appdefs.h"
#include "utils.h"
#include "bcmWrapper.h"
#include "event.h"
#include "informer_public.h"
#include "tr69c_api.h"
#include "RPCState.h"
#include "informer.h"
#include "ctMiddleware.h"
#include "tr69c_cli.h"

//#include "fwk.h" 
//#include "object.h"
//#include "cmc_api.h"
//#include "vos_msg.h"
//#include "util_tms.h"
//#include "util_prctl.h"
//#include "util_misc.h"


#define ACS_TR69C  "tr69"
//#define CPEVARNAMEINSTANCE
//#undef CPEVARNAMEINSTANCE
#define MDWExitTimeout 5000


extern int sendGETRPC;
extern int wanChangeNotification;
extern VOS_RET_E HAL_sysGetBoardId(char *boardId, UINT32 boardIdLen);

ACSState  acsState;
int tr69cTerm = 0;
void *g_msgHandle = NULL;
void *tmrHandle = NULL;
LimitNotificationQInfo limitNotificationList;
int g_TR069WANIPChanged = -1;
/*
 * Normally, the connection request server socket is opened by the smd, and inherited
 * by tr69c when smd forks/exec's tr69c.  But for unittests, we may want to start tr69c
 * by itself, so we need to be able to tell tr69c to open its own server socket.
 */
UBOOL8 openConnReqServerSocket=FALSE;
/*
  * display SOAP messages on serial console.
  * This flag is initialize, enabled or disabled in main.c,
  * and perform action in protocol.c
  */
UBOOL8 loggingSOAP = FALSE; 

UBOOL8 g_processAbnormal = FALSE;
UBOOL8 g_totalAssociationsEnable = FALSE;

//extern void proto_Init(void);
extern void freeAllListeners(void);
void main_cleanup(SINT32 code);
static void acsState_cleanup(void);
void registerSmdMessageListener(void);
void registerInterestInEvent(VosMsgType msgType, UBOOL8 positive, void *msgData, UINT32 msgDataLen);
void readMessageFromSmd(void *handle);
void adjustCTMDWHW();
void StartChinaTelecomMDW();
static void updateTr69cCfgInfo(void);
static UTIL_CPU_OCCUPY_T cpu_stat_o;
UBOOL8 exceedCpuUsageLimit = FALSE;
UBOOL8 exceedMemUsageLimit = FALSE;
UBOOL8 wirelessHWFailure = FALSE;
static UINT32 confirmWirelessHWFailure = 0;
static UINT32 accountCpuLimitTime = 0;
char alarmNumber[BUFLEN_8] = {0}; //ALARM or CLEAR ALARM with the alarm number


extern void WriteRandomInformEnableTofile(int RandomEnable);


void tr69c_scanMemUsage(void *handle)
{
    UTIL_MEM_OCCUPY_T mem_stat;
    UINT32 memUsage = 0;

    memset(&mem_stat, 0, sizeof(UTIL_MEM_OCCUPY_T));
    if(0 != UTIL_getMemOccupy(&mem_stat))
    {
        return;
    }

    memUsage = (UINT32)(mem_stat.memTotal-mem_stat.memFree-mem_stat.buffers-mem_stat.cached)*100/mem_stat.memTotal;

    if (98 <= memUsage)
    {
        if (!exceedMemUsageLimit)
        {
            vosLog_error("not enough memory, send alarm 104059");
            CMC_tr69cSetmodifyAlarm("104059");
        }
        exceedMemUsageLimit = TRUE;
        utilTmr_set(tmrHandle, tr69c_scanMemUsage, NULL, 1000, "quickly scan mem usage");
    }
    else
    {
        if (exceedMemUsageLimit)
        {
            vosLog_error("enough memory, clear alarm 104059");
            CMC_clearAlarmNumber("104059");
        }
        exceedMemUsageLimit = FALSE;
        utilTmr_set(tmrHandle, tr69c_scanMemUsage, NULL, 2*1000, "scan mem  usage");
    }

    return;
}


void tr69c_judgeWirelessHwFailure(void *handle)
{
    FILE *fp = NULL;
    char buf[BUFLEN_256] = {0};
    UBOOL8 findWl = FALSE;

    UTIL_DO_SYSTEM_ACTION("ifconfig | grep wl > /tmp/resultIfconfigForWl");
    fp = fopen("/tmp/resultIfconfigForWl", "r");

    if (NULL == fp)
    {
        vosLog_error("fopen /tmp/resultIfconfigForWl failed");
        utilTmr_set(tmrHandle, tr69c_judgeWirelessHwFailure, NULL, 2*1000, "judge wireless hardware");
        return;
    }

    while(fgets(buf, sizeof(buf), fp))
    {
        if (util_strstr(buf, "wl"))
        {
            findWl = TRUE;
            break;
        }
    }

    if (findWl)
    {
        if (wirelessHWFailure)
        {
            CMC_clearAlarmNumber("104012");
        }
        wirelessHWFailure = FALSE;
        confirmWirelessHWFailure = 0;
    }
    else
    {
        if (!wirelessHWFailure && 5 == confirmWirelessHWFailure)
        {
            vosLog_error("wireless hardware is failure, send alarm 104012");
            wirelessHWFailure = TRUE;
            CMC_tr69cSetmodifyAlarm("104012");
        }
        confirmWirelessHWFailure++;
    }

    fclose(fp);
    utilTmr_set(tmrHandle, tr69c_judgeWirelessHwFailure, NULL, 2*1000, "judge wireless hardware");
}


void tr69c_scanCpuUsage(void *handle)
{
    UTIL_CPU_OCCUPY_T cpu_stat;
    UINT32 cpuUsage = 0;

    memset(&cpu_stat, 0, sizeof(UTIL_CPU_OCCUPY_T));

    if(0 != UTIL_getCpuOccupy((UTIL_CPU_OCCUPY_T *)&cpu_stat))
    {
        return;
    }
    cpuUsage = UTIL_calCpuOccupy((UTIL_CPU_OCCUPY_T *)&cpu_stat_o, (UTIL_CPU_OCCUPY_T *)&cpu_stat);
    cpu_stat_o = cpu_stat;


    if (150 <= cpuUsage)
    {
        accountCpuLimitTime++;
        if (151 <= accountCpuLimitTime && !exceedCpuUsageLimit)
        {
            vosLog_error("CPU utilization rate for five minutes more than 95% minutes, send alarm 104030");
            CMC_tr69cSetmodifyAlarm("104030");
            exceedCpuUsageLimit = TRUE;
        }
    }
    else
    {
        accountCpuLimitTime = 0;

        if (exceedCpuUsageLimit)
        {
            CMC_clearAlarmNumber("104030");
        }
        exceedCpuUsageLimit = FALSE;
    }

    utilTmr_set(tmrHandle, tr69c_scanCpuUsage, NULL, 2*1000, "scan cpu usage");
}


static void tr69c_loidRegister(VosMsgHeader *msg)
{
    /* We don't allow other app send register msg to tr69c Directly */
    vosLog_debug("Register: msg->src = %u", msg->src);
    if (EID_CMC != msg->src)
    {
        vosLog_notice("VOS_MSG_CT_USERINFO_INFORM msg is not from cmc, ignore it!");
        return ;
    }

    addInformEventToList(INFORM_EVENT_CT_USERINFO);
    sendInform(NULL);

    return ;
}

static void initLoggingFromConfig(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VosLogLevel logLevel = DEFAULT_LOG_LEVEL;
    VosLogDestination logDest = VOS_LOG_DEST_STDERR;

    vosLog_debug("Enter>, get tr69c logLevel!");
    if (VOS_RET_SUCCESS != (ret = CMC_logGetLevel(EID_TR69C, &logLevel)))
    {
        vosLog_error("get CMC_logGetLevel fail, ret = %d", ret);
        return;
    }

    vosLog_debug("set tr69c logLevel: logLevel = [%u]", logLevel);
    vosLog_setLevel(logLevel);

    vosLog_debug("Enter>, get tr69c logDestination!");
    if (VOS_RET_SUCCESS != (ret = CMC_logGetDest(EID_TR69C, &logDest)))
    {
        vosLog_error("get CMC_logGetDest fail, ret = %d", ret);
        return;
    }

    vosLog_debug("set tr69c logDestination: logDest = [%u]", logDest);
    vosLog_setDestination(logDest);

    return;
}


void delayedTermFunc(void *handle __attribute__((unused)))
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    vosLog_notice("got terminal signal, cancel periodic timer and trigger final inform");

    /*
    * Terminal signal is handled differently than normal exit from
    * the event loop.  Under normal exit, we still allow smd to wake us
    * up sometime in the future for periodic inform or ACS changed.
    * But if we get a terminal signal, that means the user wants us to
    * completely exit and never come back, so clean up all delayed actions
    * in smd.
    */

    /* cancel any periodic inform I have in the smd */
    cancelPeriodicInform();

    /* cancel interest in various notifications that we might have registered for. */
    registerInterestInEvent(VOS_MSG_ACS_CONFIG_CHANGED, FALSE, NULL, 0);
    registerInterestInEvent(VOS_MSG_TR69_ACTIVE_NOTIFICATION, FALSE, NULL, 0);
    
    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        registerInterestInEvent(VOS_MSG_MDW_MODE_CHANGED, FALSE, NULL, 0);
        registerInterestInEvent(VOS_MSG_MDW_ACTIVE_NOTIFICATION, FALSE, NULL, 0);
        registerInterestInEvent(VOS_MSG_MDW_ACTIVE_NOTIFICATION_BAK, FALSE, NULL, 0);
        registerInterestInEvent(VOS_MSG_ALL_ACTIVE_NOTIFICATION, FALSE, NULL, 0);
        registerInterestInEvent(VOS_MSG_ALL_ACTIVE_NOTIFICATION_BAK, FALSE, NULL, 0);
    }

    if (SF_FEATURE_SUPPORT_TR69C_ALARM)
    {
        registerInterestInEvent(VOS_MSG_CT_ALARM_CHANGED, FALSE, NULL, 0);
        registerInterestInEvent(VOS_MSG_CT_ALARM_STATE_CHANGED, FALSE, NULL, 0);
    }

    if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
    {
        registerInterestInEvent(VOS_MSG_CT_MONITOR_CHANGED, FALSE, NULL, 0);
        registerInterestInEvent(VOS_MSG_CT_MONITOR_STATE_CHANGED, FALSE, NULL, 0);
    }

    registerInterestInEvent(VOS_MSG_WAN_CONNECTION_UP, FALSE, NULL, 0);

    /*
    * Set the timer to send out an inform to ACS.  The purpose is to
     * create an ACS disconnect event so that the tr69c termination
     * flag will be examined and acted on.
    * mwang: again, passing in a handle of NULL to sendInform does not make sense.
    * See comments in acsDisconnect().
    */
    ret = utilTmr_set(tmrHandle, sendInform, NULL, DELAYED_TERMINAL_ACTION_DELAY, "terminal_inform");
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("setting terminal sendInform timer failed, ret=%d", ret);
    }

    return;
}

void manageableDeviceNotificationLimitFunc(void *handle __attribute__((unused)))
{
    vosLog_notice("Manageable Device Limit Notification: time to send notification!");

    addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
    sendInform(NULL);
}


void handleNotificationLimit(char *notificationLimitName, int notificationLimitValue, UtilEventHandler limitInformFunc)
{
    LimitNotificationInfo *entry, *ptr;
    UBOOL8 found = FALSE;
    int i;

    vosLog_debug("notificationLimitName %s, limitValue %d, limitInformFunc %p",
                notificationLimitName, notificationLimitValue,limitInformFunc);

    /* loop through the notificationList queue to look for this parameter name (notificationLimitName).
    * We register this limitName, and value to the queue if it doesn't exist.
    * If there is such an item already, this mean notificationLimitValue is changed, just update the limitValue.
    */

    if ((entry = VOS_MALLOC_FLAGS(sizeof(LimitNotificationInfo), ALLOC_ZEROIZE)) != NULL)         
    {
        entry->parameterFullPathName = VOS_STRDUP(notificationLimitName);
        entry->limitValue = notificationLimitValue * MSECS_IN_SEC;
        entry->func = limitInformFunc;
    }
    else
    {
        return;
    }

    if (limitNotificationList.count != 0)
    {
        ptr = limitNotificationList.limitEntry;
        for (i = 0; i < (limitNotificationList.count) && (ptr!=NULL); i++)
        {
            if (util_strcmp(ptr->parameterFullPathName, notificationLimitName) == 0) 
            {
                ptr->limitValue = notificationLimitValue * MSECS_IN_SEC;
                found = TRUE;
                VOS_FREE(entry->parameterFullPathName);
                VOS_FREE(entry);
                break;
            } /* found entry */
            else
            {
                ptr = ptr->next;
            }
        } /* walk through limitNotificationList */
        
        if (!found)
        {
            entry->next = limitNotificationList.limitEntry;
            limitNotificationList.limitEntry = entry;
            limitNotificationList.count += 1;
        }
    } /* limitNotification list not empty */
    else
    {
        limitNotificationList.limitEntry = entry;
        limitNotificationList.count = 1;
    } /* list is empty */
} /* handleNotificationLimit */


void tr69c_sigTermHandler(int sig __attribute__((unused)))
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    /*
    * Try not to do too much in a signal handler.
    * Calling utilTmr_set() will result in a malloc.
    */
    vosLog_debug("got sig %d", sig);

    tr69cTerm = 1;

    ret = utilTmr_set(tmrHandle, delayedTermFunc, 0, DELAYED_TERMINAL_ACTION_DELAY, "sig_delayed_proc");
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("setting delayed signal processing timer failed, ret=%d", ret);
    }

    return;
}


static void openFireWallForTr69(void)
{
    if (SF_FEATURE_CUSTOMER_3BB)
    {    
        CMC_TR69C_ADVANCE_CFG_T acsCfg;
        
        VOS_RET_E ret = VOS_RET_SUCCESS;
        if ((ret = CMC_tr69cGetManagementServer(&acsCfg)) != VOS_RET_SUCCESS)
        {
            vosLog_error("get of MDMOID_MANAGEMENT_SERVER failed, ret = %d", ret);
        }
        
        UTIL_DO_SYSTEM_ACTION("iptables -t nat -D %s -p tcp --dport %u -j ACCEPT", UTIL_APP_NAT_PRE_CHAIN, acsCfg.connectionRequestPort);
        UTIL_DO_SYSTEM_ACTION("iptables -t nat -I %s -p tcp --dport %u -j ACCEPT", UTIL_APP_NAT_PRE_CHAIN, acsCfg.connectionRequestPort);
    }
    else
    {

        /* anti scan from lan side */
        UTIL_DO_SYSTEM_ACTION("iptables -t filter -I %s -p tcp --dport %u -i br0 -j DROP", UTIL_ANTI_SCAN_IN_CHAIN, TR69C_CONN_REQ_PORT);

        UTIL_DO_SYSTEM_ACTION("iptables -t nat -D %s -p tcp --dport %u -j ACCEPT", UTIL_APP_NAT_PRE_CHAIN, TR69C_CONN_REQ_PORT);
        UTIL_DO_SYSTEM_ACTION("iptables -t nat -I %s -p tcp --dport %u -j ACCEPT", UTIL_APP_NAT_PRE_CHAIN, TR69C_CONN_REQ_PORT);
    }   

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        UTIL_DO_SYSTEM_ACTION("iptables -t nat -D %s -p tcp --dport %u -j ACCEPT", UTIL_APP_NAT_PRE_CHAIN, 9080);
        UTIL_DO_SYSTEM_ACTION("iptables -t nat -I %s -p tcp --dport %u -j ACCEPT", UTIL_APP_NAT_PRE_CHAIN, 9080);
    }
}


/*
 * Initialize all the various tasks 
 */
static void initTasks(void)
{
    UBOOL8 init = TRUE;

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        init = FALSE;
#if 0
        if (enblCTMiddleware == 1)
        {
            /* init CT middleware client*/
            initCTMdwClient();
    
            /* initialize tr69c listener for any future messages from smd */
            registerSmdMessageListener();
        }
#else
        if (CTMDW_MODE_0 == enblCTMiddleware)
        {
            /* init CT middleware client*/
            initCTMdwClient();    
    
            wanChangeNotification = 1;
            ctmdw_sendInform();
            ctmdw_sendChangeInform();
            wanChangeNotification = 0;
            ctmdw_resetNotification(2);
    
            /* initialize tr69c listener for any future messages from smd */
            registerSmdMessageListener();
        }
        else if (CTMDW_MODE_2 == enblCTMiddleware)
        {
            /* INIT Protocol http, ssl */
            proto_Init();
    
            /* init CT middleware client*/
            initCTMdwClient();
    
            wanChangeNotification = 1;
            ctmdw_sendInform();            
            wanChangeNotification = 0;    
    
            /* initialize tr69c listener for any future messages from smd */
            registerSmdMessageListener();
    
            /* Just booted so send initial Inform */
            initInformer();
        }
#endif
        else
        {
            init = TRUE;
        }
    }

    if (init)
    {
        /* INIT Protocol http, ssl */
        proto_Init();
        
        /* initialize tr69c listener for any future messages from smd */
        registerSmdMessageListener();
        
        /* Just booted so send initial Inform */
        initInformer();
    }
}  /* End of initTasks() */


void registerSmdMessageListener(void)
{
    SINT32 fd;

    vosMsg_getEventHandle(g_msgHandle, &fd);

#ifdef DESKTOP_LINUX
    if (UTIL_INVALID_FD == fd)
    {
        /* when running on desktop, we might be in standalone mode for
        * unittests.  In this scenario, we don't have a fd to smd. */
        return;
    }
#endif

    if (fd != UTIL_INVALID_FD)
    {
        vosLog_debug("registering fd=%d with listener for messages from smd", fd);
        setListener(fd, readMessageFromSmd, NULL);
    }
    else
    {
        vosLog_error("Invalid msg comm fd %d", fd);
    }

    return;
}

void unregisterSmdMessageListener(void)
{
    SINT32 fd;

    vosMsg_getEventHandle(g_msgHandle, &fd);

#ifdef DESKTOP_LINUX
    if (UTIL_INVALID_FD == fd)
    {
        /* when running on desktop, we might be in standalone mode for
        * unittests.  In this scenario, we don't have a fd to smd. */
        return;
    }
#endif

    if (fd != UTIL_INVALID_FD)
    {
        vosLog_debug("unregistering fd=%d with listener for messages from smd", fd);
        stopListener(fd);
    }
    else
    {
        vosLog_error("Invalid msg comm fd %d", fd);
    }

    return;
}


/** Register or unregister our interest for some event events with smd.
 *
 * If a management entity changes the ACS config, it will send out
 * this notification and we will get it, possibly waking us up.
 * 
 * @param msgType (IN) The notification message/event that we are
 *                     interested in or no longer interested in.
 * @param positive (IN) If true, then register, else unregister.
 * @param data     (IN) Any optional data to send with the message.
 * @param dataLength (IN) Length of the data
 */
void registerInterestInEvent(VosMsgType msgType, UBOOL8 positive, void *msgData, UINT32 msgDataLen)
{
    VosMsgHeader *msg;
    char *data;
    void *msgBuf;
    char *action = (positive) ? "REGISTER" : "UNREGISTER";
    VOS_RET_E ret;

    if (msgData != NULL && msgDataLen != 0)
    {
        /* for msg with user data */
        msgBuf = VOS_MALLOC_FLAGS(sizeof(VosMsgHeader) + msgDataLen, ALLOC_ZEROIZE);
    } 
    else
    {
        msgBuf = VOS_MALLOC_FLAGS(sizeof(VosMsgHeader), ALLOC_ZEROIZE);
    }
   
    msg = (VosMsgHeader *)msgBuf;

    /* fill in the msg header */
    msg->type = (positive) ? VOS_MSG_REGISTER_EVENT_INTEREST : VOS_MSG_UNREGISTER_EVENT_INTEREST;
    msg->src = EID_TR69C;
    msg->dst = EID_SMD;
    msg->flags_request = 1;
    msg->wordData = msgType;

    if (msgData != NULL && msgDataLen != 0)
    {
        data = (char *) (msg + 1);
        msg->dataLength = msgDataLen;
        memcpy(data, (char *)msgData, msgDataLen);
    }      


    ret = vosMsg_sendAndGetReply(g_msgHandle, msg);
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("%s_EVENT_INTEREST for 0x%x failed, ret=%d", action, msgType, ret);
    }
    else
    {
        vosLog_debug("%s_EVENT_INTEREST for 0x%x succeeded", action, msgType);
    }

    VOS_FREE(msgBuf);

    return;
}


/** Make sure we have enough information to start, specifically:
 *  - The acsURL needs to be set
 *  - The interface we are bound to must be up/connected.
 *
 * Side effect: if we need the WAN connection to come up before starting
 * and the WAN connection currently is not up, send a message to smd
 * requesting WAN connection up notification.
 *
 * @return TRUE if we can start, FALSE otherwise.
 */
static UBOOL8 checkStartupPreReqs(void)
{
    UBOOL8 sts = FALSE;
    int tr69cWanState = eWAN_INACTIVE;
    VOS_RET_E ret = VOS_RET_SUCCESS;
 
    if (0 == util_strcmp(acsState.boundIfName, MDMVS_LAN))
    {
        vosLog_debug("boundIfName is %s, assume LAN is up", MDMVS_LAN);
        sts = TRUE;
    }
    else if (0 == util_strcmp(acsState.boundIfName, MDMVS_LOOPBACK))
    {
        vosLog_debug("boundIfName is %s, Loopback is always up", MDMVS_LOOPBACK);
        sts = TRUE;
    }
    else if (0 == util_strcmp(acsState.boundIfName, MDMVS_ANY_WAN))
    {
        ret = CMC_wanGetTr69cWanConnState(&tr69cWanState);
        if ((eWAN_ACTIVE == tr69cWanState) && (VOS_RET_SUCCESS == ret))
        {
            vosLog_debug("BoundIfName is %s, and one or more WAN connection is up", MDMVS_ANY_WAN);
            sts = TRUE;
        }
        else
        {
            vosLog_debug("register interest for any WAN connection up");
            registerInterestInEvent(VOS_MSG_WAN_CONNECTION_UP, TRUE, NULL, 0);//fangzhen
        }
    }
    else
    {
        /* boundifname must be a specific wan connection */
        vosLog_debug("boundIfName=%s", acsState.boundIfName);
        ret = CMC_wanGetTr69cWanConnState(&tr69cWanState);
        if ((eWAN_ACTIVE == tr69cWanState) && (VOS_RET_SUCCESS == ret))
        {
            vosLog_debug("WAN connection %s is up", acsState.boundIfName);
            if (!g_TR069WANIPChanged)
            {
                updateTr69cCfgInfo();
            }
            
            sts = TRUE;
        }
        else
        {
            vosLog_debug("register for WAN_CONNECTION_UP event on %s", acsState.boundIfName);
            registerInterestInEvent(VOS_MSG_WAN_CONNECTION_UP, TRUE, acsState.boundIfName, util_strlen(acsState.boundIfName)+1);
        }
    }
    
    return sts;
}


/* Copy settings from the MDM into acsState.
 * As a side effect, global variable.
 */
static void updateTr69cCfgInfo(void)
{
    UrlProto urlProto;
    char *urlAddr, *urlPath;
    UINT16 urlPort=0;
    CMC_TR69C_ADVANCE_CFG_T acsCfg;
    CMC_TR69C_APP_CFG_T tr69cCfg;
    CMC_TR69C_MIDDLEWARE_CFG_T mdwCfg;

    UBOOL8 connReqURLchanged = FALSE;
    VOS_RET_E ret;
    char cmd[128];
    struct in_addr addr;
    SINT32 retval;
    char ipAddr[128] = "0.0.0.0";
    char *pIpAddr = NULL;
    UBOOL8 IsSpecialWan = TRUE;
    char gwIpAddress[BUFLEN_32] = {0};
    char maskAddress[BUFLEN_32] = {0};
    char connReqIpAddrFullPath[BUFLEN_256] = {0};
    UBOOL8 check =FALSE;
    UBOOL8 isIpv4 = TRUE;
    UBOOL8 acsChange = FALSE;

    vosLog_debug("Entered>");

    memset(&acsCfg, 0, sizeof(acsCfg));
    memset(&tr69cCfg, 0, sizeof(tr69cCfg));
    memset(&mdwCfg, 0, sizeof(mdwCfg));

   /*
    * Fill in our deviceInfo params only if has not been done before.
    * Once we've filled it in, no need to do it again.  It will not
    * change while the system is still up.
    */
    if (NULL == acsState.manufacturer)
    {
        CMC_SYS_DEVICE_INFO_T deviceInfoObj;

        memset(&deviceInfoObj, 0, sizeof(deviceInfoObj));
        if ((ret = CMC_sysGetDeviceInfo(&deviceInfoObj)) != VOS_RET_SUCCESS)
        {
            vosLog_error("could not get device info object!, ret=%d", ret);
        }
        else
        {
            vosLog_debug("%s/%s/%s/%s", deviceInfoObj.manufacturer, deviceInfoObj.manufacturerOUI, deviceInfoObj.productClass, deviceInfoObj.serialNumber);

            VOS_MEM_REPLACE_STRING(acsState.manufacturer, deviceInfoObj.manufacturer);
            VOS_MEM_REPLACE_STRING(acsState.manufacturerOUI, deviceInfoObj.manufacturerOUI);
            VOS_MEM_REPLACE_STRING(acsState.productClass, deviceInfoObj.productClass);
            VOS_MEM_REPLACE_STRING(acsState.serialNumber, deviceInfoObj.serialNumber);
        }
    }

    /*get tr69c config object */
    if ((ret = CMC_tr69cGetConfig(&tr69cCfg)) != VOS_RET_SUCCESS)
    {
        vosLog_error("get of TR69C_CFG failed, ret=%d", ret);
    }
    else
    {
        loggingSOAP = tr69cCfg.loggingSoap;
        acsState.noneConnReqAuth = (tr69cCfg.connReqAuth == TRUE) ? 0 : 1;
    }

    /*Get managment server object.*/
    if ((ret = CMC_tr69cGetManagementServer(&acsCfg)) != VOS_RET_SUCCESS)
    {
        vosLog_error("get of MDMOID_MANAGEMENT_SERVER failed, ret = %d", ret);
    }

   /*
    * Check that boundIfName and acsURL are consistent.  But do not change
    * boundIfName for the user.  The user must do that himself.
    */
   if ((acsCfg.url[0] != '\0') &&
       (util_parseUrl(acsCfg.url, &urlProto, &urlAddr, &urlPort, &urlPath) == VOS_RET_SUCCESS))
   {
      vosLog_debug("^^^^acsCfg.URL:%s^^urlProto:%d^^urlAddr:%s^^urlPort:%d^^^^",acsCfg.url,urlProto,urlAddr,urlPort);

      if (CMC_lanIsLanSideIpAddr(urlAddr, &check) && util_strcmp(acsCfg.boundIfName, MDMVS_LAN))
      {
         vosLog_error("ACS URL is on LAN side (%s), but boundIfName is not set to LAN (%s)",
                      urlAddr, acsCfg.boundIfName);
      }
      else if ((util_strcmp(urlAddr, "127.0.0.1") == 0) && util_strcmp(acsCfg.boundIfName, MDMVS_LOOPBACK))
      {
         vosLog_error("ACS URL is on Loopback (%s), but boundIfName is not set to LOOPBACK (%s)",
                      urlAddr, acsCfg.boundIfName);
      }
      
      if ((0 == util_strcmp(acsCfg.boundIfName, MDMVS_ANY_WAN)) 
      || (0 == util_strcmp(acsCfg.boundIfName, MDMVS_LAN))
      || (0 == util_strcmp(acsCfg.boundIfName, MDMVS_LOOPBACK)))
      {
        IsSpecialWan = FALSE;
      }
      
      UTIL_STRNCPY(ipAddr, urlAddr, sizeof(ipAddr));
      VOS_MEM_FREE_BUF_AND_NULL_PTR(urlAddr);
      VOS_MEM_FREE_BUF_AND_NULL_PTR(urlPath);
   }

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        if ((ret = CMC_tr69cGetMiddlewareMgt(&mdwCfg)) != VOS_RET_SUCCESS)
        {
            vosLog_error("get of Middleware Management failed, ret=%d", ret);
        }
        else
        {
            acsState.enblChinaTelcomMDW = mdwCfg.tr069Enable;
            enblCTMiddleware = (int)acsState.enblChinaTelcomMDW;/*更新tr69配置参数不一致*/
            VOS_MEM_REPLACE_STRING(acsState.MWSURL, mdwCfg.mwUrl);
            vosLog_debug("enable = %d, url = %s\n", acsState.enblChinaTelcomMDW, acsState.MWSURL);
        }
    }

   /* case 1) : tr69c is alive, if URL is changed, we need to update acsState.acsURL, and reset retryCount
    *              to 0.
    *              Because lastConnectedURL can't be changed before cpe received InformResponse from ACS
    *              successfully, we can't reset retryCount at case 3.
    *
    * case 2) : tr69c is exit normally(such as we only have periodic inform, no retry, and out of select time
    *              out) or cpe just boot, when tr69c is launch again for some reason(value change, periodicinform), 
    *             we need to initialize acsState.acsURL.
    *
    * case 3) : we will add  INFORM_EVENT_BOOTSTRAP in this case, because acsState.acsURL will lose
    *               its value when tr69c exit.(we will set the value of acsCfg->lastConnectedURL when we
    *               receive Informresponse)
    *
    */
    
    //if (acsCfg.url[0] != '\0')
    {
        /*case 1: tr69c is alive, */
        if (acsState.acsURL != NULL)
        {
            if (util_strcmp(acsState.acsURL, acsCfg.url) != 0)
            {
                VOS_MEM_REPLACE_STRING(acsState.acsURL, acsCfg.url);
                acsState.retryCount = 0; /*reset retryCount when ACS URL is changed.*/
                acsChange = TRUE;
                vosLog_debug("acsURL changed, add eIEBootStrap inform event");
            }
        }
        else /*case 2 : cpe just boot or tr69c exit normally*/
        {
            acsState.acsURL = VOS_STRDUP(acsCfg.url);
        }

        vosLog_debug("acsState.acsURL=%s", acsState.acsURL);
   
        /*case 3 */
        if ((acsCfg.lastConnectedUrl[0] == '\0') 
         || (util_strcmp(acsCfg.lastConnectedUrl, acsCfg.url) != 0))
        {
            addInformEventToList(INFORM_EVENT_BOOTSTRAP);
            vosLog_debug("setting acsURL for the first time, add eIEBootStrap event to inform list. lastConnectedURL=%s, acsCfg.URL=%s", 
                          acsCfg.lastConnectedUrl, acsCfg.url);

            /* when bootstrap is sent, parameters on table 5 of TR98 specification
            * needs to be reset back to default Active Notification.
            */
            setDefaultActiveNotification();
        }
    }

    /*ACS username*/
    if (acsCfg.username[0] != '\0')
    {
        REPLACE_STRING_IF_NOT_EQUAL(acsState.acsUser, acsCfg.username);
    }
   
    /* ACS password*/
    if (acsCfg.password[0] != '\0')
    {
        REPLACE_STRING_IF_NOT_EQUAL(acsState.acsPwd, acsCfg.password);
    }

    /*connectionRequestURL*/
    if (acsState.connReqURL != NULL && acsCfg.connectionRequestUrl[0] != '\0')
    {
        if (util_strcmp(acsState.connReqURL, acsCfg.connectionRequestUrl) != 0)
        {
            VOS_MEM_REPLACE_STRING(acsState.connReqURL, acsCfg.connectionRequestUrl);
            connReqURLchanged = TRUE;
        }
        
        if ((acsState.connReqIpAddr != NULL) && (acsState.connReqIpAddrFullPath == NULL))
        {
            /* if a path had not been built because External IP address was not up
            due to layer 2 link down.  Try to build it.
            */
            connReqURLchanged = TRUE;
        }
    }
    else if (acsCfg.connectionRequestUrl[0] != '\0')
    {
        acsState.connReqURL = VOS_STRDUP(acsCfg.connectionRequestUrl);
        connReqURLchanged = TRUE;
    }

   /* connReqIpAddr, connReqIfNameFullPath, connReqPath*/
   if (connReqURLchanged)
   {
      /*
       * ConnectionRequestURL has changed or has been set to the acsState
       * for the first time.  Update the 3 other variables associated with
       * connReqURL.
       */
      VOS_FREE(acsState.connReqIpAddr);
      VOS_FREE(acsState.connReqPath);

      /*
       * parseUrl should always succeed since our own STL handler function
       * built this URL.  Note this algorithm assumes the IP address portion
       * is always in dotted decimal format, not a DNS name.  I think this is
       * a safe assumption.
       */
      util_parseUrl(acsCfg.connectionRequestUrl, &urlProto, &acsState.connReqIpAddr, &urlPort, &acsState.connReqPath);
      vosLog_debug("connReqIPAddr=%s connReqPath=%s", acsState.connReqIpAddr, acsState.connReqPath);

      if (!util_strcmp(acsState.connReqIpAddr, "127.0.0.1"))
      {
         /* if connReqIPAddress is loopback, just force full path to the
          * first LAN device.  Not a very good solution, but I'm not sure what
          * else to do.
          */
         acsState.connReqIpAddrFullPath = VOS_STRDUP("InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1.IPInterfaceIPAddress");
      } 
    }

    if (connReqURLchanged)
    {
      g_TR069WANIPChanged = 1;
    }

    ret = CMC_phlGetFullPathByIpAddr(acsState.connReqIpAddr, connReqIpAddrFullPath, (UINT32)sizeof(connReqIpAddrFullPath));
    if (VOS_RET_SUCCESS == ret)
    {
        REPLACE_STRING_IF_NOT_EQUAL(acsState.connReqIpAddrFullPath, connReqIpAddrFullPath);
        if (NULL == acsState.connReqIpAddrFullPath)
        {
            vosLog_error("could not build full path to %s", acsState.connReqIpAddr);
        }
        else
        {
            vosLog_debug("connReqIpAddrFullPath = %s", acsState.connReqIpAddrFullPath);
        }
    }
    else
    {
        vosLog_notice("CMC_phlGetFullPathByIpAddr failed");
    }

    /*connectionRequestUsername*/
    if (acsCfg.connectionRequestUsername[0] != '\0')
    {
        REPLACE_STRING_IF_NOT_EQUAL(acsState.connReqUser, acsCfg.connectionRequestUsername);
    }

    /*connectionRequestPassword*/
    if (acsCfg.connectionRequestPassword[0] != '\0')
    {
        REPLACE_STRING_IF_NOT_EQUAL(acsState.connReqPwd, acsCfg.connectionRequestPassword);
    }

    /* boundIfName, it should never be NULL*/
    if (util_strcmp(acsState.boundIfName, acsCfg.boundIfName) != 0 || acsChange)
    {
        char gwIp[32] = {0};
        
        REPLACE_STRING_IF_NOT_EQUAL(acsState.boundIfName, acsCfg.boundIfName);
        pIpAddr = VOS_MALLOC_FLAGS(64, ALLOC_ZEROIZE);
        
        retval = inet_pton(AF_INET, ipAddr, &addr);
        if (retval <= 0)         //(retval > 0)fangzhen
        {
            UINT32 bufLen = 16;
            char tr69IfName[32] = {0};
            char *ip = NULL;
            char *tokTemp = NULL;
            
            if (0 == util_strcmp(acsCfg.boundIfName, MDMVS_ANY_WAN))
            {
                if (VOS_RET_SUCCESS == CMC_tr69cGetIfName(tr69IfName, sizeof(tr69IfName)))
                {
                    if (tr69IfName[0] != '\0')
                    {
                        if (VOS_RET_SUCCESS != CMC_dnsGetHostIp(ipAddr, tr69IfName, pIpAddr, bufLen, &isIpv4))
                        {
                           vosLog_error("CMC_dnsGetHostIp failed");
                        }
                    }
                }
            }
            else
            {
                if (VOS_RET_SUCCESS != CMC_dnsGetHostIp(ipAddr, acsCfg.boundIfName, pIpAddr, bufLen, &isIpv4))
                {
                   vosLog_error("CMC_dnsGetHostIp failed");
                }
            }
            if (!IS_EMPTY_STRING(pIpAddr))
            {
                ip = strtok_r(pIpAddr, "\t", &tokTemp);
                if (ip)
                {
                    UTIL_STRNCPY(ipAddr, ip, sizeof(ipAddr));
                }
            }
            VOS_FREE(pIpAddr);
        }
        

        /* 支持多ip则是在增加静态路由时候是添加一个网段   route add -net xxx 
         * 否则是在增加静态路由时候是添加一个具体host     route add -host xxx
         */
        if (util_isValidIpAddress(AF_INET, ipAddr))
        {
#if 1//def SUPPORT_TR069_SERVER_MULTI_IP
           char ipAddr_24[64] = {0};
           char *pPoint = ipAddr;
           int i=0;
           for(; i<3; i++)
           {
              pPoint = strchr(pPoint, '.');
              pPoint++;
           }
           if ((util_strncmp(ipAddr,gwIpAddress,pPoint - ipAddr) != 0) && (util_strncmp(ipAddr,"192.168.1.249",util_strlen("192.168.1.249")) != 0))
           {
              UTIL_STRNCPY(ipAddr_24, ipAddr, pPoint - ipAddr + 1);
              UTIL_STRNCAT(ipAddr_24, "0", sizeof(ipAddr_24));
              vosLog_debug("^^^^^^^^^ipAddr_24:%s^^^^^^^^IsSpecialWan:%d^^^^^^^^^^^^^^^",ipAddr_24,IsSpecialWan);
              UTIL_SNPRINTF(cmd, sizeof(cmd), "route delete -net %s netmask 255.255.255.0 > /var/err",  ipAddr_24);
              UTIL_doSystemAction("tr69c", cmd);

              if (IsSpecialWan)
              {
                  vosLog_debug("^^^^ipAddr_24:%s^^gwIpAddress:%s^^maskAddress:%s^^^^",ipAddr_24,gwIpAddress,maskAddress);
                  //CMC_routeAddItemForTr69c(ipAddr_24, ipAddr, &acsCfg, gwIpAddress, sizeof(gwIpAddress), maskAddress, sizeof(maskAddress));//fangzhen dns
                  //CMC_routeAddItemForTr69c("80.80.80.80", ipAddr, &acsCfg, gwIpAddress, sizeof(gwIpAddress), maskAddress, sizeof(maskAddress));//fangzhen dns
              }
           }

           vosLog_debug("ipAddr = %s", ipAddr);

           CMC_tr69cGetGwIp(gwIp, sizeof(gwIp));
           UTIL_DO_SYSTEM_ACTION("ip route add %s via %s dev %s table %s", ipAddr, gwIp, acsCfg.boundIfName, ACS_TR69C);
           if (SF_FEATURE_CUSTOMER_3BB)
           {
               UTIL_DO_SYSTEM_ACTION("echo %s > /var/acsIp",ipAddr);
           }
          
#else
           sprintf(cmd, "route delete -host %s > /var/err",  ipAddr);
           UTIL_doSystemAction("tr69c", cmd);

           if (IsSpecialWan)
           {
              if (IsNotUseGw)
                 sprintf(cmd, "route add -host %s dev %s > /var/err",  ipAddr, acsCfg->X_BROADCOM_COM_BoundIfName);
              else
                 sprintf(cmd, "route add -host %s gw %s dev %s > /var/err",  ipAddr, gwIpAddress, acsCfg->X_BROADCOM_COM_BoundIfName);

              UTIL_doSystemAction("tr69c", cmd);
           }
#endif
        }
    }
    if (SF_FEATURE_CUSTOMER_3BB)
    {
        CMC_mangSetRule();
    }
    if (SF_FEATURE_ISP_CU)
    {
        acsState.randomInformEnable = acsCfg.randomInformEnable;
    }

    /*Periodic Inform Interval*/
    if (acsState.informInterval != (SINT32)acsCfg.periodicInformInterval)
    {
        acsState.informInterval = (SINT32)acsCfg.periodicInformInterval;
        if (TRUE == acsState.informEnable)
        {
            cancelPeriodicInform();
            resetPeriodicInform(acsCfg.periodicInformInterval);
        }
    }

    /*Periodic Inform Enable*/
    if (acsState.informEnable != acsCfg.periodicInformEnable)
    {
        acsState.informEnable = acsCfg.periodicInformEnable;
        if (TRUE == acsState.informEnable)
        {
            resetPeriodicInform(acsCfg.periodicInformInterval);
            vosLog_debug("periodic inform is now enabled, add event INFORM_EVENT_PERIODIC");
        }
        else
        {
            cancelPeriodicInform();
        }
    }

    /* ManageableDeviceNotificationLimit.  Update limitNotificationList. */
    if (acsCfg.manageableDeviceNotificationLimit != 0)
    {
        handleNotificationLimit("InternetGatewayDevice.ManagementServer.ManageableDeviceNumberOfEntries",
                      acsCfg.manageableDeviceNotificationLimit, manageableDeviceNotificationLimitFunc);
    }
    
    return;
}

void readMessageFromSmd(void *handle)
{
    VosMsgHeader *msg;
    VOS_RET_E ret;
    //int count = 0;
    int timeout;

    char path[1024*5] = {0};

    int ctstatus = 1;
    FILE *fd;
    char line[6];

    if (NULL == handle)
    {
        timeout = 0;
    }
    else
    {
        timeout = *(int*)handle;
    }

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        if ((fd = fopen("/var/ct/status", "r")) == NULL)
        {
           vosLog_debug("ctadmin should be not launched successfully.");
           ctstatus = 0;    
        }
        else
        {
           fgets(line, 6, fd);
           sscanf(line, "%d", &ctstatus);
           fclose(fd);
        }
    }
   
    /*
     * At startup, call receiveWithTimeout with a timeout of BOOTUP_MSG_RECEIVE_TIMEOUT.
     * After the first message is used, timeout is 0.
     * There should already be a message waiting for me.
     */
    while ((ret = vosMsg_receiveWithTimeout(g_msgHandle, &msg, (UINT32)timeout)) == VOS_RET_SUCCESS)
    {
        timeout = 0;

        vosLog_debug("tr69c msg->type:%x", msg->type);

        switch(msg->type)
        {
            case VOS_MSG_SYSTEM_BOOT:
                vosLog_debug("got SYSTEM_BOOT, adding eIEBoot to informEvList");

                /* according to TR69c spec, after reboot event, the session retry count
                must be set to 0. */
                acsState.retryCount = 0;

                if (SF_FEATURE_ISP_CU)
                {
                    WriteRandomInformEnableTofile(1);
                }

                /* system just booted up */
                addInformEventToList(INFORM_EVENT_BOOT);
                if (SF_FEATURE_LOCATION_XINJIANG)
                {
                    addInformEventToList(INFORM_EVENT_CT_USERINFO);
                }

                if (SF_FEATURE_SUPPORT_PLUGIN)
                {
                    //CMC_tr69cSetmodifyAlarm("104001");
                }

                if (SF_FEATURE_LOCATION_SHANGHAI)
                {
                    CMC_tr69cSetmodifyAlarm("104001");
                }

                break;

                case VOS_MSG_ACS_CONFIG_CHANGED:
                    vosLog_debug("got ACS config changed");
                    /* updateTR69cCfgInfo adds inform event to list if necessary */
                    //clearModemConnectionURL();
                    updateTr69cCfgInfo();
                    sendInform(NULL);

                    if (SF_FEATURE_SUPPORT_SYSLOG)
                    {
                        vosSyslog_info("itms config changed!");
                        util_saveLogToFlash(g_msgHandle);
                    }

                    break;

                case VOS_MSG_TR69_ACTIVE_NOTIFICATION:
                case VOS_MSG_MDW_ACTIVE_NOTIFICATION_BAK:
                    vosLog_debug("got tr69 active notification");
                    /* wan connection IP address may be changed.  This means the ConnectionURL is changed */
                    updateTr69cCfgInfo();

                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE && (CTMDW_MODE_0 == enblCTMiddleware) && ctstatus)
                    {
                        ctmdw_sendChangeInform();
                        ctmdw_resetNotification(0);
                    }
                    else
                    {
                        addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_MDW_ACTIVE_NOTIFICATION:
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                    {
                        if ((enblCTMiddleware != CTMDW_MODE_1) && ctstatus)
                        {
                            vosLog_debug("got Middleware active notification");
                            ctmdw_sendInform();
                            ctmdw_resetNotification(1);
                        }
                    }

                    break;

                case VOS_MSG_ALL_ACTIVE_NOTIFICATION:
                case VOS_MSG_ALL_ACTIVE_NOTIFICATION_BAK:
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                    {
                        if ((enblCTMiddleware != CTMDW_MODE_1) && ctstatus)
                        {
                            vosLog_debug("got Middleware active notification");
                            ctmdw_sendInform();
                            ctmdw_sendChangeInform();
                            ctmdw_resetNotification(2);
                        }
                    }
                    break;

                case VOS_MSG_MDW_MODE_CHANGED:
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                    {
                        if ((fd = fopen("/var/ct/mode", "r")) == NULL)
                        {
                            vosLog_debug("tr69c should be not launched successfully, so skip changing mode.");
                            break;
                        }
                        else
                        {
                            fclose(fd);
                        }

                        if (msg->wordData == 0)
                        {
                            ret = utilTmr_set(tmrHandle, stopCTMDWTimer, 0, 2000, "stop_ctmdw_timer");
                            if (ret != VOS_RET_SUCCESS)
                            {
                                vosLog_error("setting stop ctmdw timer failed, ret=%d", ret);
                            }
                        }
                        else if (msg->wordData == 1)
                        {
                            ret = utilTmr_set(tmrHandle, ctmdw_startCTMDWClient, 0, 5000, "start_ctmdw_proc");
                            if (ret != VOS_RET_SUCCESS)
                            {
                                vosLog_error("setting start ctmdw processing timer failed, ret=%d", ret);
                            }
                        }
                    }
                    break;

                case VOS_MSG_DELAYED_MSG:
                    if (msg->wordData == PERIODIC_INFORM_TIMEOUT_ID)
                    {
                        vosLog_debug("got delayed msg, periodic inform while running");
                        periodicInformTimeout(NULL);
                    }
                    else
                    {
                        vosLog_error("unrecognized wordData 0x%x in DELAYED_MSG", msg->wordData);
                    }
                    break;

                case VOS_MSG_WAN_CONNECTION_UP:
                    vosLog_debug("got WAN_CONNECTION_UP msg");
                    /* wan connection IP address may be changed.  This means the ConnectionURL is changed */
                    g_TR069WANIPChanged = 0;
                    /* wan connection IP address may be changed.  This means the ConnectionURL is changed */
                    updateTr69cCfgInfo();
                    if (1 == g_TR069WANIPChanged)
                    {
                        addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
                        sendInform(NULL);
                    }

                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                    {
                        if ((enblCTMiddleware == CTMDW_MODE_0) && ctstatus)
                        {
                            wanChangeNotification = 1;
                            ctmdw_sendInform();
                            ctmdw_sendChangeInform();
                            wanChangeNotification = 0;
                            ctmdw_resetNotification(2);
                            //ctmdw_sendMode2Inform();
                        }
                        else if (enblCTMiddleware == CTMDW_MODE_2)
                        {
                            wanChangeNotification = 1;
                            ctmdw_sendInform();
                            wanChangeNotification = 0;
                            //ctmdw_sendMode2Inform();
                        }
                    }
                    break;

                case VOS_MSG_SET_LOG_LEVEL:
                    vosLog_setLevel(msg->wordData);
                    break;

                case VOS_MSG_SET_LOG_DESTINATION:
                    vosLog_setDestination(msg->wordData);
                    break;

                case VOS_MSG_PING_STATE_CHANGED:
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE && (CTMDW_MODE_0 == enblCTMiddleware))
                    {
                        ctmdw_sendOperation();
                    }
                    else
                    {
                        TR69C_PING_STATUS_MSG_T *pingInfo = (TR69C_PING_STATUS_MSG_T *) (msg + 1);
                        if ((util_strcmp(pingInfo->diagnosticsState, MDMVS_COMPLETE) == 0) 
                        || (util_strcmp(pingInfo->diagnosticsState, MDMVS_ERROR_CANNOTRESOLVEHOSTNAME) == 0))
                        {
                            addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
                            sendInform(NULL);

                            vosLog_debug("got VOS_MSG_PING_STATE_CHANGED");
                        }
                    }
                    break;

                case VOS_MSG_TRACERT_STATE_CHANGED:
                {
                    VOS_TRACEROUTE_MSGBODY *traceRtInfo = (VOS_TRACEROUTE_MSGBODY *) (msg + 1);

                    if (util_strcmp(traceRtInfo->diagnosticsState, MDMVS_COMPLETE) == 0 ||
                    util_strcmp(traceRtInfo->diagnosticsState, MDMVS_ERROR_CANNOTRESOLVEHOSTNAME) == 0 || 
                    util_strcmp(traceRtInfo->diagnosticsState, MDMVS_ERROR_MAXHOPCOUNTEXCEEDED) == 0)
                    {
                        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
                        sendInform(NULL);
                        vosLog_debug("got CMS_MSG_TRACERT_STATE_CHANGED = %s", traceRtInfo->diagnosticsState);
                    }
                }
                break;

                case VOS_MSG_DIAG:
                    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE && (CTMDW_MODE_0 == enblCTMiddleware))
                    {
                        ctmdw_sendOperation();
                    }
                    else
                    {
                        vosLog_debug("got VOS_MSG_DIAG");
                        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_CT_JS_NAME_CHANGE:
                {
                        vosLog_debug("got VOS_MSG_CT_JS_NAME_CHANGE");
                        if (600 <= getTime())
                        {
                            vosLog_debug("device set NAME-CHANGE and timer out, so send event to ITMS!");
                            addInformEventToList(INFORM_EVENT_NAME_CHANGE);
                            sendInform(NULL);
                        }
                        else
                        {
                            vosLog_debug("device set NAME-CHANGE and timer in, so drop event! time = %d", getTime());
                        }

                        break;
                }

                case VOS_MSG_CT_JS_LOID_CHANGE:
                {
                    vosLog_debug("got VOS_MSG_CT_JS_LOID_CHANGE");
                    addInformEventToList(INFORM_EVENT_LOID_CHANGE);
                    sendInform(NULL);
                }
                break;

                case VOS_MSG_TR69_GETRPCMETHODS_DIAG:
                    addInformEventToList(INFORM_EVENT_PERIODIC);
                    sendInform(NULL);
                    sendGETRPC = 1;
                    vosLog_debug("got VOS_MSG_TR69_GETRPCMETHODS_DIAG");
                    break;

                case VOS_MSG_MANAGEABLE_DEVICE_NOTIFICATION_LIMIT_CHANGED:
                    handleNotificationLimit("InternetGatewayDevice.ManagementServer.ManageableDeviceNumberOfEntries",
                    msg->wordData,manageableDeviceNotificationLimitFunc);
                    break;

#ifdef SUPPORT_DEBUG_TOOLS
                case VOS_MSG_MEM_DUMP_STATS:
                    vosMem_dumpMemStats();
                    break;
#endif

#ifdef VOS_MEM_LEAK_TRACING
                case VOS_MSG_MEM_DUMP_TRACEALL:
                    vosMem_dumpTraceAll();
                    break;

                case VOS_MSG_MEM_DUMP_TRACE50:
                    vosMem_dumpTrace50();
                    break;

                case VOS_MSG_MEM_DUMP_TRACECLONES:
                    vosMem_dumpTraceClones();
                    break;
#endif

                case VOS_MSG_CT_ALARM_CHANGED:
                    if (SF_FEATURE_SUPPORT_TR69C_ALARM)
                    {
                        changeAlarm(msg->wordData);
                    }
                    break;

                case VOS_MSG_CT_ALARM_STATE_CHANGED:
                    if (SF_FEATURE_SUPPORT_TR69C_ALARM)
                    {
                        startAlarm();
                    }
                    break;

                case VOS_MSG_CT_MONITOR_STATE_CHANGED:
                    if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
                    {
                        startMonitor();
                    }
                    break;

                case VOS_MSG_CT_MONITOR_CHANGED:
                    if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
                    {
                        changeMonitor(msg->wordData);
                    }
                    break;

                case VOS_MSG_CT_USERINFO_INFORM:
                    if (SF_FEATURE_SUPPORT_CT_USERINFO)
                    {
                        vosLog_notice("Register: received VOS_MSG_CT_USERINFO_INFORM msg");
                        tr69c_loidRegister(msg);
                    }
                    break;

                case VOS_MSG_CT_DOWNLOADDIAG_INFORM:
                    vosLog_notice(" received VOS_MSG_CT_DOWNLOADDIAG_INFORM msg");
                    addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
                    sendInform(NULL);
                    break;

                case VOS_MSG_SEND_CT_ACCOUNT_CHANGE:
                    if (SF_FEATURE_SUPPORT_TR69C_MAINTAIN)
                    {
                        if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                        {
                            if ((enblCTMiddleware == CTMDW_MODE_0) && ctstatus)
                            {
                                ctmdw_sendCTAccountChangeInform();
                            }
                            else
                            {
                                addInformEventToList(INFORM_EVENT_CT_MAINTAIN);
                                sendInform(NULL);
                            }
                        }
                        else
                        {
                            addInformEventToList(INFORM_EVENT_CT_MAINTAIN);
                            sendInform(NULL);
                        }
                    }
                    break;

                case VOS_MSM_SYSTEM_OFFLINE:
                    if (SF_FEATURE_ISP_CU)
                    {
                        //printf("sos##\n");
                        addInformEventToList(INFORM_EVENT_SYSTEM_OFFLINE);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_SEND_CT_INFORM_STATUS:
                    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
                    {
                        addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
                        vosLog_debug("%s, %d, %s\n", __FUNCTION__, __LINE__, "sendInform for INFORM_EVENT_DIAGNOSTICS_COMPLETE event");
                        sendInform(NULL);
                    }
                    break;

                /* Add by tuhanyu, for upnp device management of separate AP, 2011/04/15, start */
                case VOS_MSG_UPNP_EVENT:
                    if (SF_FEATURE_SUPPORT_UPNP_DMCP)
                    {
                        sendInform((void *)eIEValueChanged);
                    }
                    break;

                case VOS_MSG_UPNP_GETALL:
                    if (SF_FEATURE_SUPPORT_UPNP_DMCP)
                    {
                        int errocode = 0;
                        char errorString[BUFLEN_128] = {0};
                        char value[1024*5] = {0};

                        UTIL_SNPRINTF(path, sizeof(path), "InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.%d.", msg->wordData);
                        CMC_phlUpdateUpnpProxyDevice(path, value, 0, &errocode, errorString);
                    }
                break;
                /* Add by tuhanyu, end */

                case VOS_MSG_CT_CARDALARM:
                    if (SF_FEATURE_SUPPORT_CARD)
                    {
                        CMC_TR69C_DEV_REG_CFG_T userinfo;

                        CMC_tr69cGetUserInfo(&userinfo);
                        if ((0 == userinfo.status) && (0 == userinfo.result))
                        {
                            addInformEventToList(INFORM_EVENT_CT_CARDWRITE);
                        }
                        addInformEventToList(INFORM_EVENT_CT_ALARM);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_CT_LONGRESET:
                    addInformEventToList(INFORM_EVENT_CT_LONGRESET);
                    saveTR69StatusItems();
                    break;

                case VOS_MSG_ADMIN_PASSWD_CHANGED:
                    if (SF_FEATURE_ISP_CU)
                    {
                        addInformEventToList(INFORM_ADMIN_PASSWD_CHANGED);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_CLI_TR69_SHOW_SOAP:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processShowLog);
                    break;

                case VOS_MSG_CLI_GET_VALUE:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteGetValue);
                    break;

                case VOS_MSG_CLI_SET_VALUE:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteSetValue);
                    break;

                case VOS_MSG_CLI_ADD_OBJ:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteAddObj);
                    break;

                case VOS_MSG_CLI_DEL_OBJ:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteDelObj);
                    break;

                case VOS_MSG_CLI_GET_NAME:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteGetName);
                    break;

                case VOS_MSG_CLI_GET_ATTRIBUTES:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteGetAttributes);
                    break;

                case VOS_MSG_CLI_SET_ATTRIBUTES:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteSetAttributes);
                    break;

                case VOS_MSG_CLI_DO_REBOOT:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteReboot);
                    break;

                case VOS_MSG_CLI_DO_RESET:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processRemoteReset);
                    break;

                case VOS_MSG_CLI_CLEAR_TR69_SOAP:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processClearLog);
                    break;

                case VOS_MSG_CLI_TR69_ENABLE_SOAP:
                    UTIL_processRemoteCli(g_msgHandle, msg, TR69_processEnableShowLog);
                    break;

                case VOS_MSG_CERT_CHANGE:
                    vosLog_debug("Recevie msg VOS_MSG_CERT_CHANGE");
                    proto_Init();
                    break;

                case VOS_MSG_EMLUATE_COMPLTETE_INFORM:
                    vosLog_debug("Recevie msg VOS_MSG_EMLUATE_COMPLTETE_INFORM");
                    addInformEventToList(INFORM_EVENT_DIAGNOSTICS_COMPLETE);
                    sendInform(NULL);
                    break;

                case VOS_MSG_WATCHDOG_HEARTBEAT:
                    UTIL_sendHeartbeat(g_msgHandle);
                    break;

                case VOS_MSG_REQUEUE:
                    break;

                case VOS_MSG_STB_MAC_REPORT_TO_TR69C:
                    if (SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU || SF_FEATURE_LOCATION_JIANGXI)
                    {
                        vosLog_debug("Receive iptv stb mac msg");
                        UINT32 responseTime = 2 * 1000;
                        char stbMac[BUFLEN_128] = {0};
                        CMC_MCAST_IPTV_STB_MAC_T iptvStbMac;
                        UTIL_STRNCPY(stbMac, (char *)(msg + 1), sizeof(stbMac));
                        if (0 == util_strcmp(stbMac, "null_mac"))
                        {
                            ret = utilTmr_set(tmrHandle, processIptvStbMac, NULL, responseTime, "timer_response");
                            if (ret != VOS_RET_SUCCESS)
                            {
                                vosLog_error("could not set response timer, ret = %d", ret);
                            }
                        }
                        else
                        {
                            memset((void *)&iptvStbMac, 0, sizeof(iptvStbMac));
                            UTIL_STRNCPY(iptvStbMac.STBMAC, stbMac, sizeof(iptvStbMac.STBMAC));

                            ret = CMC_igmpSetIptvStbMac(&iptvStbMac);
                            if (ret != VOS_RET_SUCCESS)
                            {
                                vosLog_error("process ping state changed msg failed,ret=%d",ret);
                            }
                            else
                            {
                                addInformEventToList(INFORM_EVENT_CT_STB_BIND);
                                sendInform(NULL);
                            }
                        }
                    }
                    break;

                case VOS_MSG_WLAN_TOTAL_ASSOCIATION:
                    if (SF_FEATURE_LOCATION_FUJIAN)
                    {
                        vosLog_debug("got wlan associated device num changed msg");
                        g_totalAssociationsEnable = TRUE;
                        addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_PROCESS_ABNORMAL_SYSTEM_REBOOT:
                    if (SF_FEATURE_LOCATION_FUJIAN)
                    {
                        vosLog_debug("got process Abnormal need reboot system msg");

                        g_processAbnormal = TRUE;
                        addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
                        sendInform(NULL);
                    }
                    break;

                case VOS_MSG_SEND_ALARM_INFO:
                    vosLog_debug("got VOS_MSG_CT_DBUS_SEND_ALARM_INFO");
                    vosLog_debug("get dataLenth = %d", msg->dataLength);
                    vosLog_debug("get alarmNumber = %s", (char *)(msg + 1));
                    vosLog_debug("get alarm type = %d", msg->wordData);
                    if (!IS_EMPTY_STRING((char *)(msg + 1)))
                    {
                        UTIL_STRNCPY(alarmNumber, (char *)(msg + 1), sizeof(alarmNumber));
                    }

                    if (1 == msg->wordData)
                    {
                        addInformEventToList(INFORM_EVENT_CLEAR_CT_ALARM);
                        sendInform(NULL);
                    }
                    else
                    {
                        addInformEventToList(INFORM_EVENT_CT_ALARM);
                        sendInform(NULL);
                    }

                    break;

                default:
                    vosLog_error("unrecognized msg 0x%x", msg->type);
                    break;
        }

        VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
    }

    if (VOS_RET_DISCONNECTED == ret)
    {
        if (0 == tr69cTerm)
        {
            VOS_RET_E r2;
            vosLog_error("lost connection to smd, exiting now.");
            unregisterSmdMessageListener();
            tr69cTerm = 1;
            r2 = utilTmr_set(tmrHandle, delayedTermFunc, 0, DELAYED_TERMINAL_ACTION_DELAY, "sig_delayed_proc");
            if (r2 != VOS_RET_SUCCESS)
            {
                vosLog_error("setting delayed signal processing timer failed, ret=%d", r2);
            }
        }
    }
}

static void usage(char *progName)
{
    /* use print because I don't know the state of the log (?) */
    printf("usage: %s [-v num] [-m shmId] [-e num] [-u acsURL] [-i informInterval]\n", progName);
    printf("       v: set verbosity, where num==0 is LOG_ERROR, 1 is LOG_NOTICE, all others is LOG_DEBUG\n");
    printf("       m: shared memory id, -1 if standalone or not using shared mem.\n");
    printf("       u: for url for the ACS, otherwise, URL is obtained from mdm\n");
    printf("       r: connection request URL, otherwise, URL is obtained from mdm\n");
    printf("       o: open connection request server socket, normally this server socket is inheritted from smd\n");
    printf("       i: inform interval, otherwise, informInterval is obtained from mdm\n");
    printf("       b: informEnable, 1 is true, otherwise, informEnable is obtained from mdm\n");
    printf("       f: boundIfName, otherwise, boundIfName is obtained from mdm\n");
    exit(1);
}

int main(int argc, char** argv)
{
    SINT32      c, logLevelNum;
    SINT32      shmId = UNINITIALIZED_SHM_ID;
    VosLogLevel logLevel = DEFAULT_LOG_LEVEL;
    UBOOL8      useConfiguredLogLevel = TRUE;
    char        *forcedAcsUrl = NULL;
    char        *forcedConnReqUrl = NULL;
    UINT32      informInterval = 0;  /* 0 is invalid value */
    SINT32      informEnable = -1;   /* -1 is not set, 0 is false, 1 is true */
    char        *forcedBoundIfName = NULL;
    VOS_RET_E    ret = VOS_RET_SUCCESS;
    int timeout = 0;
    UBOOL8 logging = FALSE;

    /* init log util */
    vosLog_init(EID_TR69C);
    FWK_btInit(0);
   /* parse command line args */
   while ((c = getopt(argc, argv, "v:m:e:u:i:b:r:of:")) != -1)
   {
      switch(c)
      {
         case 'v':
            logLevelNum = atoi(optarg);
            if (logLevelNum == 0)
            {
               logLevel = VOS_LOG_LEVEL_ERR;
            }
            else if (logLevelNum == 1)
            {
               logLevel = VOS_LOG_LEVEL_NOTICE;
            }
            else
            {
               logLevel = VOS_LOG_LEVEL_DEBUG;
            }
            useConfiguredLogLevel = FALSE;
            break;

         case 'm':
            shmId = atoi(optarg);
            break;

         case 'u':
            forcedAcsUrl = optarg;
            break;

         case 'r':
            forcedConnReqUrl = optarg;
            break;

         case 'o':
            openConnReqServerSocket = TRUE;
            break;

         case 'b':
            informEnable = atoi(optarg);
            break;

         case 'i':
            informInterval = (UINT32)atoi(optarg);
            break;

         case 'f':
            forcedBoundIfName = optarg;
            break;
                    
         default:
            usage(argv[0]);
            break;
      }
   }

   /*
    * Detach myself from the terminal so I don't get any control-c/sigint.
    * On the desktop, it is smd's job to catch control-c and exit.
    * When tr69c detects that smd has exited, tr69c will also exit.
    */
   if (setsid() == -1)
   {
      vosLog_error("Could not detach from terminal");
   }
   else
   {
      vosLog_debug("detached from terminal");
   }

    /* set signal masks */
    signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE signals */
    signal(SIGTERM, tr69c_sigTermHandler);
    signal(SIGINT, tr69c_sigTermHandler);

    vosLog_notice("initializing timers");
    if ((ret = utilTmr_init(&tmrHandle)) != VOS_RET_SUCCESS)
    {
        vosLog_error("utilTmr_init failed, ret=%d", ret);
        return -1;
    }

    vosLog_notice("calling vosMsg_init");
    if ((ret = vosMsg_initEasy(&g_msgHandle)) != VOS_RET_SUCCESS)
    {
        vosLog_error("vosMsg_init failed, ret=%d", ret);
        return 0;
    }
   
    if (useConfiguredLogLevel)
    {
        initLoggingFromConfig();
    }

    loggingSOAP = logging;
    /* initialize acs state from mdm */
    memset(&acsState, 0, sizeof(ACSState));

    /* read saved state from persistent scratch pad, including informState */
    retrieveTR69StatusItems();

    /* read any vendor config info from flash if any */
    retrieveClearTR69VendorConfigInfo();

    /* clear limit notification list */
    memset((char*)&limitNotificationList,0,sizeof(limitNotificationList));
   
    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        adjustCTMDWHW();
    }
    
    /* retrieve tr69c configuration including acsUrl, ManageableDeviceLimitNotification if any */
    updateTr69cCfgInfo();

    /* check for any message from smd telling me why I was launched. */
    timeout = BOOTUP_MSG_RECEIVE_TIMEOUT;
    readMessageFromSmd(&timeout);

    /*系统启动之后开始检测cpu、内存使用率和WLAN硬件是否故障*/
    tr69c_scanCpuUsage(NULL);
    tr69c_scanMemUsage(NULL);
    tr69c_judgeWirelessHwFailure(NULL);

    vosLog_debug("acsState.manufacturer   = %s", acsState.manufacturer);
    vosLog_debug("acsState.manufacturerOUI= %s", acsState.manufacturerOUI);
    vosLog_debug("acsState.productClass   = %s", acsState.productClass);
    vosLog_debug("acsState.serialNumber   = %s", acsState.serialNumber);
    vosLog_debug("acsState.acsURL         = %s", acsState.acsURL);
    vosLog_debug("acsState.boundIfName    = %s", acsState.boundIfName);
    vosLog_debug("acsState.acsUser        = %s", acsState.acsUser);
    vosLog_debug("acsState.acsPwd         = %s", acsState.acsPwd);
    vosLog_debug("acsState.connReqURL     = %s", acsState.connReqURL);
    vosLog_debug("acsState.connReqIpAddr  = %s", acsState.connReqIpAddr);
    if (acsState.connReqIpAddrFullPath != NULL)
    {
        vosLog_debug("acsState.connReqIpAddrFullPath= %s", acsState.connReqIpAddrFullPath);
    }
    else
    {
        vosLog_error("acsState.connReqIpAddrFullPath is NULL");
    }
    vosLog_debug("acsState.connReqUser    = %s", acsState.connReqUser);
    vosLog_debug("acsState.connReqPwd     = %s", acsState.connReqPwd);
    vosLog_debug("acsState.informEnable   = %d", acsState.informEnable);
    vosLog_debug("acsState.informInterval = %ld", acsState.informInterval);
    vosLog_debug("informState             = %d", informState);

    /*
    * Register our interest for ACS_CONFIG_CHANGED event with smd.
    */
    registerInterestInEvent(VOS_MSG_ACS_CONFIG_CHANGED, TRUE, NULL, 0);
    registerInterestInEvent(VOS_MSG_TR69_ACTIVE_NOTIFICATION, TRUE, NULL, 0);
    registerInterestInEvent(VOS_MSG_TR69_GETRPCMETHODS_DIAG, TRUE, NULL, 0);
    registerInterestInEvent(VOS_MSG_CT_DOWNLOADDIAG_INFORM, TRUE, NULL, 0);

    if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
    {
        registerInterestInEvent(VOS_MSG_MDW_MODE_CHANGED, TRUE, NULL, 0);
        registerInterestInEvent(VOS_MSG_MDW_ACTIVE_NOTIFICATION, TRUE, NULL, 0);
        registerInterestInEvent(VOS_MSG_MDW_ACTIVE_NOTIFICATION_BAK, TRUE, NULL, 0);
        registerInterestInEvent(VOS_MSG_ALL_ACTIVE_NOTIFICATION, TRUE, NULL, 0);
        registerInterestInEvent(VOS_MSG_ALL_ACTIVE_NOTIFICATION_BAK, TRUE, NULL, 0);
    }

    if (SF_FEATURE_SUPPORT_CT_USERINFO)
    {
        UTIL_registerEvent(g_msgHandle, VOS_MSG_CT_USERINFO_INFORM, NULL, 0);
    }
   
    if (SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU) // SUPPORT_JIANGSU CT_JS_INFORM_NAME_CHANGE
    {
        registerInterestInEvent(VOS_MSG_CT_JS_LOID_CHANGE, TRUE, NULL, 0);
        registerInterestInEvent(VOS_MSG_CT_JS_NAME_CHANGE, TRUE, NULL, 0);
    }

    checkStartupPreReqs();

    if (1)  /*Don't let tr069 exit*/
    {
      /*avoid the situation when dmz enable, itms can not connect to our device*/
      openFireWallForTr69();

      if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
      {
        /*RUN CT middleware, if necessary.*/
        StartChinaTelecomMDW();
      }

      /*Initialize SSL and also send first inform, if necessary.*/
      initTasks();

      /*This is where everything happens.*/
       
      eventLoop();
   }
   else
   {
      /* we don't have enough info to run or or WAN connection is not up yet.
       * Fall through to exit path and let smd wake us up when there is
       * enough info or when the WAN connection is up.
       */
      saveTR69StatusItems();
   }


   /*
    * Cleanup before exiting.
    * There is one other exit point in informer.c: acsDisconnect.
    * Eventually that should be consolidated into here.
    */
   main_cleanup(0);

   return(0); /* not reached, since main_cleanup calls exit */
}  /* End of main() */




/** Cleanup all resources before exiting.  This makes resource checkers happy.
 *
 * This function does not return.  It will cause the program to exit.
 *
 * @param code (IN) exit code.
 */
void main_cleanup(SINT32 code)
{
    vosLog_notice("exiting with code %d", code);

    freeAllListeners();

    acsState_cleanup();
    vosMsg_cleanup(&g_msgHandle);
    utilTmr_cleanup(&tmrHandle);
    vosLog_cleanup();
    exit(code);
}

void acsState_cleanup(void)
{
   VOS_FREE(acsState.acsURL);
   VOS_FREE(acsState.acsUser);
   VOS_FREE(acsState.acsPwd);
   VOS_FREE(acsState.parameterKey);
   VOS_FREE(acsState.newParameterKey);
   VOS_FREE(acsState.rebootCommandKey);
   VOS_FREE(acsState.downloadCommandKey);
   VOS_FREE(acsState.boundIfName);
   VOS_FREE(acsState.connReqURL);
   VOS_FREE(acsState.connReqIpAddr);
   VOS_FREE(acsState.connReqIpAddrFullPath);
   VOS_FREE(acsState.connReqPath);
   VOS_FREE(acsState.connReqUser);//fangzhen
   VOS_FREE(acsState.connReqPwd);
   VOS_FREE(acsState.kickURL);
   VOS_FREE(acsState.provisioningCode);
   VOS_FREE(acsState.dlFaultMsg);
   VOS_FREE(acsState.scheduleInformCommandKey);
   VOS_FREE(acsState.manufacturer);
   VOS_FREE(acsState.manufacturerOUI);
   VOS_FREE(acsState.productClass);
   VOS_FREE(acsState.serialNumber);

   if (SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
   {
       VOS_FREE(acsState.MWSURL);
   }
}

VOS_RET_E closeSrvSocket(void)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    msg.type = VOS_MSG_CLOSE_SERVER_LISTEN;
    msg.src = EID_TR69C;
    msg.dst = EID_SMD;

    msg.flags_request = 1;
    msg.wordData = EID_TR69C;

    ret = vosMsg_sendAndGetReplyWithTimeout(g_msgHandle, &msg, (5*MSECS_IN_SEC));
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("close server socket failed, ret=%d", ret);
    }

    return ret;
}

void adjustCTMDWHW()
{
    struct stat statbuf;
    static UBOOL8 initflag = FALSE;
    char boardId[20];

/*add it by Stone Liu*/
    struct sched_param param = { .sched_priority = 5 /*BRCM_SOFTIRQD_RTPRIO*/ };

    sched_setscheduler(getpid(), SCHED_RR,&param);
/*end*/

    if (!initflag)
    {
         memset(boardId,0,sizeof(boardId));
         HAL_sysGetBoardId(boardId,sizeof(boardId));
         if (!strcmp(boardId,"968380SGW") || !strcmp(boardId,"968380SGWEPON")) //smart netmask
         {    
            if (stat("/usr/local/ct/ctadmin", &statbuf))
            {
                prctl_runCommandInShellWithTimeout("rm -rf /usr/plugin/ctadmin");
                prctl_runCommandInShellWithTimeout("mkdir /usr/plugin/ctadmin");
                prctl_runCommandInShellWithTimeout("chmod 777 /usr/plugin/ctadmin");
                prctl_runCommandInShellWithTimeout("cp /bin/ctadmin /usr/local/ct -f");
                if (stat("/usr/local/ct/ctadmin", &statbuf))
                {
                     printf("ERROR:%s %s() %d:get ctadmin error\n",__FILE__,__FUNCTION__,__LINE__);
                }else
                {
                     initflag = TRUE;
                }
            }

         }else //e8c
         {

             prctl_runCommandInShellWithTimeout("mount -t jffs2 /dev/mtdblock7 /usr/plugin/ctadmin/");
             if (stat("/usr/local/ct/ctadmin", &statbuf))
             {
                 //Jffs2 is empty. Erase all and release ctadmin from firmware.....
                 //Reboot at last.
                 prctl_runCommandInShellWithTimeout("umount -f usr/plugin/ctadmin");
                 prctl_runCommandInShellWithTimeout("flash_eraseall /dev/mtd7");
             
                 prctl_runCommandInShellWithTimeout("mount -t jffs2 /dev/mtdblock7 /usr/plugin/ctadmin/");
                 prctl_runCommandInShellWithTimeout("cp /bin/ctadmin /usr/local/ct -f");
             
                 //util_sendRequestRebootMsg(g_msgHandle);
                 // prctl_runCommandInShellWithTimeout("reboot");
             }
             initflag = TRUE;
         }     
    }

/*add it */
    param.sched_priority = 0;
    sched_setscheduler(0, SCHED_NORMAL, &param);
/*end*/

    enblCTMiddleware = (int)acsState.enblChinaTelcomMDW;
}

void StartChinaTelecomMDW() 
{
    struct stat statbuf;

    if (acsState.enblChinaTelcomMDW != CTMDW_MODE_1)
    {      
        if (!stat("/usr/local/ct/ctadmin", &statbuf))
        {
            enblCTMiddleware = acsState.enblChinaTelcomMDW;
            prctl_runCommandInShellWithTimeout("echo 1 > /var/ct/status");
            prctl_runCommandInShellWithTimeout("chmod ugo+x /usr/local/ct/ctadmin");

            if (CTMDW_MODE_0 == enblCTMiddleware)
            {
                closeSrvSocket();        
                prctl_runCommandInShellWithTimeout("/usr/local/ct/ctadmin s=0");    
                prctl_runCommandInShellWithTimeout("echo 0 > /var/ct/mode"); 
            }
            else
            {
                prctl_runCommandInShellWithTimeout("/usr/local/ct/ctadmin s=2");
                prctl_runCommandInShellWithTimeout("echo 2 > /var/ct/mode"); 
            }
        }
    }
    else
    {
        prctl_runCommandInShellWithTimeout("echo 1 > /var/ct/mode"); 
    }
}


void * GetTR69CMsgHandler(void)
{
    return g_msgHandle;
}


void stopCTMDWTimer(void* handle)
{
    VOS_RET_E  ret =  VOS_RET_SUCCESS;

    ctmdw_sendMWExit();
    ret = utilTmr_set(tmrHandle, ctmdw_stopCTMDWClient, 0, MDWExitTimeout, "stop_ctmdw_proc");
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("setting stop ctmdw processing timer failed, ret=%d", ret);
    }
}

