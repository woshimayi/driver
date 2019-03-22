
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
 * File Name  : RPCState.c
 *
 * Description: RPC routines 
 * $Revision: 1.54 $
 * $Id: RPCState.c,v 1.54 2006/02/03 15:53:05 dmounday Exp $
 *----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <syslog.h>
#include <sys/sysinfo.h>

#include "fwk.h"
#include "phl.h"
#include "cmc_api.h"
#include "../inc/tr69cdefs.h" /* defines for ACS state */
#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include "xml_nano.h"
#include "xml_parser_sm.h"
#include "RPCState.h"
#include "xmlTables.h"
#include "../main/event.h"
#include "../main/informer.h"
#include "../main/httpDownload.h"
#include "../bcmLibIF/bcmWrapper.h"
#include "mdm.h"
#include "emdm.h"
#include "hal_util_sys.h"
#include "../ctMiddleware/ctMiddleware.h"
#include "custom.h"


#define MAX_PADDINGS 20
#ifdef OMIT_INDENT
#define xml_mIndent(A,B,C);
#else
#define xml_mIndent(A,B,C) // XMLmIndent( A, B, 0);
#endif
#define NUMREQINFORMDEVIDS (sizeof(informDevIds)/sizeof(char *))


#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define  STR_VOICE_LINE  ".Line." 
#define  STR_WAN_LANINTERFACE "X_CT-COM_LanInterface"
#define  STR_WAN_LANINTERFACE_CU "X_CU_LanInterface"
#define  STR_CARD_KEYPARAM "X_CT-COM_CardKey"
#define  STR_CARD_USERINFO_NAME  "X_CT-COM_UserInfo.UserName"
#define  STR_CARD_USERINFO_PASS  "X_CT-COM_UserInfo.UserId"
#define  CARD_KEY_LEN       16
#define  CHECK_LOID_STATUSPARAM "InternetGatewayDevice.X_CT-COM_UserInfo.Status"
#define  CHECK_LOID_RESULTPARAM "InternetGatewayDevice.X_CT-COM_UserInfo.Result"

#define DOWNLOAD_DIAG_STATE_PARAM "InternetGatewayDevice.DownloadDiagnostics.DiagnosticsState"
#define DOWNLOAD_DIAG_INTER_PARAM "InternetGatewayDevice.DownloadDiagnostics.Interface"
#define DOWNLOAD_DIAG_URL_PARAM "InternetGatewayDevice.DownloadDiagnostics.DownloadURL"
#define DOWNLOAD_DIAG_DSCP_PARAM "InternetGatewayDevice.DownloadDiagnostics.DSCP"
#define DOWNLOAD_DIAG_ETHPRI_PARAM "InternetGatewayDevice.DownloadDiagnostics.EthernetPriority"

#define UPLOAD_DIAG_STATE_PARAM "InternetGatewayDevice.UploadDiagnostics.DiagnosticsState"
#define UPLOAD_DIAG_INTER_PARAM "InternetGatewayDevice.UploadDiagnostics.Interface"
#define UPLOAD_DIAG_URL_PARAM "InternetGatewayDevice.UploadDiagnostics.UploadURL"
#define UPLOAD_DIAG_DSCP_PARAM "InternetGatewayDevice.UploadDiagnostics.DSCP"
#define UPLOAD_DIAG_ETHPRI_PARAM "InternetGatewayDevice.UploadDiagnostics.EthernetPriority"
#define UPLOAD_DIAG_FILELEN_PARAM "InternetGatewayDevice.UploadDiagnostics.TestFileLength"

#define VOIP_PRO_SWITCH_PARAM "InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.X_CU_ServerType"

typedef struct
{
    char *buf;
    UINT32 size;
    UINT32 offset;
} TR69C_ALLOC_BUF_T;

RPCAction *acsRpcAction;
RPCAction *simRpcAction;
int transferCompletePending;
int sendGETRPC;
static eInformState rebootFlag;       /* a TRX_REBOOT has been returned from a setxxx */
static UBOOL8 saveConfigFlag = FALSE;   /* save config on disconnect */
static int factoryResetFlag;
TransferInfo transferList;

int      wanChangeNotification = 0;
char     g_keyhaveset      = 0;
char     g_statushaveset = 0;
char     g_statusautheachother = 0;
char     g_statusresult_recv = 0;

int download_diag = 0;
int change_download_param = 0;
int upload_diag = 0;
int change_upload_param = 0;
int wan_laninterface = 0;
//int voice_line = 0;
UBOOL8 loid_changed_flag = FALSE;
extern char alarmNumber[BUFLEN_8]; //ALARM or CLEAR ALARM with the alarm number


static const char *informDevIds[] = {
   "InternetGatewayDevice.DeviceInfo.Manufacturer",
   "InternetGatewayDevice.DeviceInfo.ManufacturerOUI",
   "InternetGatewayDevice.DeviceInfo.ProductClass",
   "InternetGatewayDevice.DeviceInfo.SerialNumber",
};

static char *informParameters[] = {
   "InternetGatewayDevice.DeviceSummary",
   "InternetGatewayDevice.DeviceInfo.SpecVersion",
   "InternetGatewayDevice.DeviceInfo.HardwareVersion",
   "InternetGatewayDevice.DeviceInfo.SoftwareVersion",
   "InternetGatewayDevice.DeviceInfo.ProvisioningCode",
   "InternetGatewayDevice.ManagementServer.ConnectionRequestURL",
   "InternetGatewayDevice.ManagementServer.ParameterKey",
   NULL  /* this pathname can change, copied from acsState.connReqIfNameFullPath*/
};

static char *informParametersYunnan[] = {
   "InternetGatewayDevice.DeviceSummary",
   "InternetGatewayDevice.DeviceInfo.SpecVersion",
   "InternetGatewayDevice.DeviceInfo.HardwareVersion",
   "InternetGatewayDevice.DeviceInfo.SoftwareVersion",
   "InternetGatewayDevice.DeviceInfo.ProvisioningCode",
   "InternetGatewayDevice.ManagementServer.ConnectionRequestURL",
   "InternetGatewayDevice.ManagementServer.ParameterKey",
   "InternetGatewayDevice.DeviceInfo.DeviceType",
   "InternetGatewayDevice.DeviceInfo.AccessType",
   NULL  /* this pathname can change, copied from acsState.connReqIfNameFullPath*/
};

static char *informParametersFujian[] = {
    "InternetGatewayDevice.DeviceSummary",
    "InternetGatewayDevice.DeviceInfo.SpecVersion",
    "InternetGatewayDevice.DeviceInfo.HardwareVersion",
    "InternetGatewayDevice.DeviceInfo.SoftwareVersion",
    "InternetGatewayDevice.DeviceInfo.ProvisioningCode",
    "InternetGatewayDevice.ManagementServer.ConnectionRequestURL",
    "InternetGatewayDevice.ManagementServer.ParameterKey",
    "InternetGatewayDevice.DeviceInfo.X_CT-COM_MACAddress",
    NULL  /* this pathname can change, copied from acsState.connReqIfNameFullPath*/
};

static const char *informEventStr[] = {
   "0 BOOTSTRAP",
   "1 BOOT",
   "2 PERIODIC",
   "3 SCHEDULED",
   "4 VALUE CHANGE",
   "5 KICKED",
   "6 CONNECTION REQUEST",
   "7 TRANSFER COMPLETE",
   "8 DIAGNOSTICS COMPLETE",
   "M Reboot",
   "M ScheduleInform",
   "M Download",
   "M Upload",
   "NULL",
   "NULL",     //14
   "NULL",
   "NULL",
   "NULL",
   "NULL",
   "X CT-COM BIND2",
   "X CT-COM NAME CHANGE",
   "NULL",
   "NULL",
   "NULL",
   "NULL",
   "NULL",
   "NULL",
   "X CT-COM BIND1",
};


CTMDW_INFORM_VALUE ctdefaultvalue[] = {
{"InternetGatewayDevice.ManagementServer.CTMgtIPAddress", "NULL"},
{"InternetGatewayDevice.ManagementServer.MgtDNS", "NULL"},
{"InternetGatewayDevice.ManagementServer.InternetPvc", "NULL"},
{"InternetGatewayDevice.ManagementServer.CTUserIPAddress.", "NULL"},
};

static TR69C_ALLOC_BUF_T sg_tr69cAllocBuf;

extern void *g_msgHandle;
extern ACSState acsState;
extern eSessionState sessionState;

extern CT_ALARMORMONITOR_SEND monitorSend[50];
extern int send_monitor_number;

extern CT_ALARMORMONITOR_SEND alarmSend[50];
extern int send_alarm_number;

extern CT_ALARMORMONITOR_SEND cleanalarmSend[50];
extern int send_clean_alarm_number;

static int beginTime = 0;

extern UBOOL8 g_processAbnormal;
extern UBOOL8 g_totalAssociationsEnable;

void handle_download_diag();
void handle_upload_diag();

/** private functions **/
static const char *getRPCMethodName(eRPCMethods);
static void freeParamItems(ParamItem *item);
static void freeAttributeItems(AttributeItem *item);
static void freeDownloadReq(DownloadReq *r);
static void mprintf(tProtoCtx *pc, int *len, const char *fmt, ...);
static void xml_mprintf(tProtoCtx *pc, int *len, char *s);
static void closeBodyEnvelope(tProtoCtx *pc, int *lth);
static void openBody(tProtoCtx *pc, int *lth);
static void openEnvWithHeader(char *idstr, tProtoCtx *pc, int *lth);
static const char *getFaultCode(int fault);
static const char *getFaultStr(int fault);
static int getParamCnt(ParamItem *pi);
static void writeSoapFault(RPCAction *a, int fault);
static void doGetRPCMethods(RPCAction *a);
static void writeGetAttribute(CMC_PHL_GET_PARAM_ATTR_T *pParamAttr, tProtoCtx *pc, int *bufsz);
static void writeGetPName(CMC_PHL_GET_PARAM_NAME_T *pParamInfo, tProtoCtx *pc, int *bufsz);
static void doGetParameterNames(RPCAction *a);
static void doSetParameterAttributes(RPCAction *a);
static void doGetParameterAttributes(RPCAction *a);
static void doSetParameterValues(RPCAction *a);
static void writeGetPValue(char *path, char *type, char *value,  tProtoCtx *pc, int *bufsz);
static void doGetParameterValues(RPCAction *a);
static void doAddObject(RPCAction *a);
static void doDeleteObject(RPCAction *a);
static void doRebootRPC(RPCAction *a);
static void doFactoryResetRPC(RPCAction *a);
static void doDownload(RPCAction *a);
static void doGetQueuedTransfers(RPCAction *a);
void dealcardinfo(RPCAction *a);
void Sendalarmmsgtoitms(char *errorcode);

extern void writeLog(const char *buf, int bufLen);

static void inversionParamItem(ParamItem *head)
{
    ParamItem *p, *t , *q;
    p = NULL;

    t = head->next;
        
    if(t == NULL)
    {
        return;
    }        

    q = t->next;
    if(q == NULL) 
    {
        return;
    }           
    while(q)
    {
        t->next = p;
        p = t;
        t = q;
        q = q->next;
    }
    t->next = p;
    head->next = t;
}

int getsysRunTime(void)
{
    struct sysinfo sysRunInfo;
    
    /*success*/
    if (sysinfo(&sysRunInfo) == 0)
    {
        return sysRunInfo.uptime;
    }
    else
    {
        return 0;
    }
}

int getTime(void)
{
    return getsysRunTime() - beginTime;
}
extern char bridgeUserName[64];


static void tr69c_initInformEventStr()
{
    if (SF_FEATURE_SUPPORT_TR69C_ALARM)
    {
        informEventStr[13] = "X CT-COM ALARM";
        informEventStr[17] = "X CT-COM CLEARALARM";
    }
    
    if (SF_FEATURE_SUPPORT_TR69C_MONITOR)
    { 
        informEventStr[14] = "X CT-COM MONITOR";
    }
    
    if (SF_FEATURE_SUPPORT_TR69C_MAINTAIN)
    {
        informEventStr[16] = "X CT-COM ACCOUNTCHANGE";
    }

    if (SF_FEATURE_SUPPORT_CT_USERINFO)
    {
        informEventStr[15] = "X CT-COM BIND";
    }

    if (SF_FEATURE_SUPPORT_CARD)
    {
        informEventStr[18] = "X CT-COM CARDWRITE";
        informEventStr[22] = "X CT-COM CARDNOTIFY";
    }

    if (SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU || SF_FEATURE_LOCATION_JIANGXI)
    {
        informEventStr[24] = "X CT-COM STBBIND";
    }

    if (SF_FEATURE_LOCATION_XINJIANG || SF_FEATURE_LOCATION_SHANDONG)
    {
        informEventStr[15] = "X CT-COM BIND2";
    }

    informEventStr[26] = "X CT-COM LONGRESET";
}



/*for CU inform Event*/
static void tr69c_initInformEventStrforCU()
{
    if (SF_FEATURE_SUPPORT_TR69C_ALARM)
    {
        informEventStr[13] = "M X_CU_ALARM";
        informEventStr[17] = "M X_CU_CLEARALARM";
    }

    if (SF_FEATURE_SUPPORT_CT_USERINFO)
    {
        informEventStr[15] = "X CU BIND";
    }

    informEventStr[25] = "11 X_CU_OFFLINE";
    
}

RPCAction* newRPCAction(void)
{
    return ((RPCAction *)VOS_MALLOC_FLAGS(sizeof(RPCAction), ALLOC_ZEROIZE));
}


#ifdef DEBUG
void dumpAcsState(void)
{
    fprintf(stderr, "ACS State dump\n");
    fprintf(stderr, "HoldRequests       %d\n", acsState.holdRequests);
    fprintf(stderr, "NoMoreRequest      %d\n", acsState.noMoreRequests);
    fprintf(stderr, "CommandKey(DL)     %s\n", acsState.downloadCommandKey);
    fprintf(stderr, "CommandKey(Reboot  %s\n", acsState.rebootCommandKey);
    fprintf(stderr, "ParameterKey       %s\n", acsState.parameterKey);
    fprintf(stderr, "MaxEnvelopes       %d\n", acsState.maxEnvelopes);
    fprintf(stderr, "RPC Methods supported by ACS:\n");
    fprintf(stderr, "   GetRpcMethods           %s\n", acsState.acsRpcMethods.rpcGetRPCMethods?
            "Yes": "No");
    fprintf(stderr, "   SetParameterValues      %s\n", acsState.acsRpcMethods.rpcSetParameterValues?
            "Yes": "No");
    fprintf(stderr, "   GetParameterValues      %s\n", acsState.acsRpcMethods.rpcGetParameterValues?
            "Yes": "No");
    fprintf(stderr, "   GetParameterNames       %s\n", acsState.acsRpcMethods.rpcGetParameterNames?
            "Yes": "No");
    fprintf(stderr, "   GetParameterAttributes  %s\n", acsState.acsRpcMethods.rpcGetParameterAttributes?
            "Yes": "No");
    fprintf(stderr, "   SetParameterAttributes  %s\n", acsState.acsRpcMethods.rpcSetParameterAttributes?
            "Yes": "No");
    fprintf(stderr, "   Reboot                  %s\n", acsState.acsRpcMethods.rpcReboot?
            "Yes": "No");
    fprintf(stderr, "   FactoryReset            %s\n", acsState.acsRpcMethods.rpcFactoryReset?
            "Yes": "No");
    fprintf(stderr, "   Download                %s\n", acsState.acsRpcMethods.rpcDownload?
            "Yes": "No");
    fprintf(stderr, "   ScheduleInform          %s\n", acsState.acsRpcMethods.rpcScheduleInform?
            "Yes": "No");
    fprintf(stderr, "   Upload                  %s\n", acsState.acsRpcMethods.rpcUpload?
            "Yes": "No");
    fprintf(stderr, "   GetQueuedTransfers      %s\n", acsState.acsRpcMethods.rpcGetQueuedTransfers?
            "Yes": "No");
}

void dumpRpcAction(RPCAction *a)
{
    fprintf(stderr, "RPC description: RPC Method = %s ID=%s\n",
            getRPCMethodName(a->rpcMethod), a->ID);
}
#endif

/* rebootCompletion routine */
/* This is envoked following the ACS response to the rebootresponse msg */
UBOOL8 rebootCompletion(void)
{
    if (rebootFlag >= eACSDownloadReboot && rebootFlag <= eACSRPCReboot)
    {
        wrapperReboot(rebootFlag);
        return TRUE;
    }

    return FALSE;
}

/* factoryResetCompletion routine */
/* This is envoked following the ACS response to the FactoryResetResponse msg */
UBOOL8 factoryResetCompletion(void)
{
    if (factoryResetFlag)
    {
        factoryResetFlag = 0;
        if (wrapperFactoryReset())
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}

/* save Configuration routine */
/* This is envoked following the ACS response to the a set/add/delete RPC msg */
void saveConfigurations(void)
{
    if (saveConfigFlag)
    {
        wrapperSaveConfigurations();
        saveConfigFlag = FALSE;
    }
}

/* Utility routines for data structures */
static const char *getRPCMethodName(eRPCMethods m)
{
    const char    *t;
    switch (m)
    {
        case rpcGetRPCMethods:
            t = "GetRPCMethods";
            break;
        case rpcScheduleInform:
            t = "ScheduleInform";
            break;          
        case rpcSetParameterValues:
            t = "SetParameterValues";
            break;
        case rpcGetParameterValues:
            t = "GetParameterValues";
            break;
        case rpcGetParameterNames:
            t = "GetParameterNames";
            break;
        case rpcGetParameterAttributes:
            t = "GetParameterAttributes";
            break;
        case rpcSetParameterAttributes:
            t = "SetParameterAttributes";
            break;
        case rpcAddObject:
            t = "AddObject";
            break;
        case rpcDeleteObject:
            t = "DeleteObject";
            break;
        case rpcReboot:
            t = "Reboot";
            break;
        case rpcDownload:
            t = "Download";
            break;
        case rpcUpload:
            t = "Upload";
            break;
        case rpcFactoryReset:
            t = "FactoryReset";
            break;
        case rpcInformResponse:
            t = "InformResponse";
            break;
        case rpcGetQueuedTransfers:
            t = "GetQueuedTransfers";
            break;
        default:
            t = "no RPC methods";
            break;
    }
    
    return t;
}

static void freeParamItems(ParamItem *item)
{
    ParamItem *next;
    while (item)
    {
        next = item->next;
        VOS_MEM_FREE_BUF_AND_NULL_PTR(item->pname);     /* free data */
        VOS_MEM_FREE_BUF_AND_NULL_PTR(item->pvalue);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(item->pOrigValue);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(item);           /* free ParamItem */
        item = next;
    }
}

static void freeAttributeItems(AttributeItem *item)
{
   AttributeItem *next;
   while (item)
   {
      next = item->next;
      VOS_MEM_FREE_BUF_AND_NULL_PTR(item->pname);
      VOS_MEM_FREE_BUF_AND_NULL_PTR(item);
      item = next;
   }
}

static void freeDownloadReq(DownloadReq *r)
{
   VOS_MEM_FREE_BUF_AND_NULL_PTR(r->url);
   VOS_MEM_FREE_BUF_AND_NULL_PTR(r->user);
   VOS_MEM_FREE_BUF_AND_NULL_PTR(r->pwd);
   VOS_MEM_FREE_BUF_AND_NULL_PTR(r->fileName);
}
/*
* item is undefined on return
*/
void freeRPCAction(RPCAction *item)
{
   VOS_MEM_FREE_BUF_AND_NULL_PTR(item->ID);
   VOS_MEM_FREE_BUF_AND_NULL_PTR(item->parameterKey);
   VOS_MEM_FREE_BUF_AND_NULL_PTR(item->commandKey);
   VOS_MEM_FREE_BUF_AND_NULL_PTR(item->informID);
   switch (item->rpcMethod)
   {
      case rpcGetParameterNames:
         VOS_MEM_FREE_BUF_AND_NULL_PTR(item->ud.paramNamesReq.parameterPath);
         break;
      case rpcSetParameterValues:
      case rpcGetParameterValues:
      case rpcScheduleInform:
         freeParamItems(item->ud.pItem);
         break;
      case rpcSetParameterAttributes:
      case rpcGetParameterAttributes:
         freeAttributeItems(item->ud.aItem);
         break;
      case rpcAddObject:
      case rpcDeleteObject:
         VOS_MEM_FREE_BUF_AND_NULL_PTR(item->ud.addDelObjectReq.objectName);
         break;
      case rpcDownload:
         freeDownloadReq( &item->ud.downloadReq );
         break;
      case rpcUpload:
         freeDownloadReq( &item->ud.downloadReq );
         break;
      case rpcGetQueuedTransfers:
        /* do we need to free anything? */
        break;
      default:
         break;
   }
   VOS_MEM_FREE_BUF_AND_NULL_PTR(item);
}  /* End of freeRPCAction() */

/*----------------------------------------------------------------------*/
static VOS_RET_E tr69c_initAllocBuf(void)
{
    const UINT32 allocSize = 1024;

    vosLog_debug("Enter");

    VOS_FREE(sg_tr69cAllocBuf.buf);
    memset(&sg_tr69cAllocBuf, 0, sizeof(sg_tr69cAllocBuf));

    sg_tr69cAllocBuf.buf = VOS_MALLOC_FLAGS(allocSize, ALLOC_ZEROIZE);
    if (NULL == sg_tr69cAllocBuf.buf)
    {
        vosLog_error("Alloc buf fail");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    sg_tr69cAllocBuf.size = allocSize;
    sg_tr69cAllocBuf.offset = 0;

    return VOS_RET_SUCCESS;
}

/*----------------------------------------------------------------------*/
static void tr69c_freeAllocBuf(void)
{
    vosLog_debug("Enter");
    VOS_FREE(sg_tr69cAllocBuf.buf);
    memset(&sg_tr69cAllocBuf, 0, sizeof(sg_tr69cAllocBuf));
}

/*----------------------------------------------------------------------*/
static VOS_RET_E tr69c_saveToAllocBuf(const char *data, UINT32 len)
{
    const UINT32 allocSize = 1024;
    SINT32 freeSize = 0;

    if (NULL == data)
    {
        vosLog_debug("Null string");
        return VOS_RET_SUCCESS;
    }

    if (NULL == sg_tr69cAllocBuf.buf)
    {
        vosLog_debug("Do nothing");
        return VOS_RET_SUCCESS;
    }

    freeSize = (sg_tr69cAllocBuf.size - sg_tr69cAllocBuf.offset);

    if (freeSize < len)
    {
        vosLog_debug("Realloc buf");

        sg_tr69cAllocBuf.size += (len + allocSize);
        sg_tr69cAllocBuf.buf = VOS_REALLOC(sg_tr69cAllocBuf.buf, sg_tr69cAllocBuf.size);
        if (NULL == sg_tr69cAllocBuf.buf)
        {
            vosLog_error("Realloc buf fail");
            memset(&sg_tr69cAllocBuf, 0, sizeof(sg_tr69cAllocBuf));
            return VOS_RET_RESOURCE_EXCEEDED;
        }
    }

    memcpy(sg_tr69cAllocBuf.buf + sg_tr69cAllocBuf.offset, data, len);
    sg_tr69cAllocBuf.offset += len;

    return VOS_RET_SUCCESS;
}


/*----------------------------------------------------------------------*
 * memory printf
 */
static void mprintf(tProtoCtx *pc, int *len, const char *fmt, ...)
{
   int      n;
   va_list  ap;
   char     buf[512];
    
   va_start(ap, fmt);
   if (ap == NULL)
   {
      n = util_strlen(fmt);
      if (n > 0)
      {
         *len += n;
         if (pc != NULL)
         {
            proto_SendRaw(pc, fmt, n);
         }
         else
         {
             tr69c_saveToAllocBuf(fmt, n);
         }
      }
   }
   else
   {
      n = vsnprintf(buf, 512, fmt, ap);
      if (n < 0 || n >= 512)
      {
         /* out of memory */ 
         vosLog_error("xml: mprintf: out of memory");
         acsState.fault = VOS_RET_RESOURCE_EXCEEDED;
      }
      else if (n > 0)
      {
         *len += n;
          if (pc != NULL)
          {
            proto_SendRaw(pc, buf, n);
          } 
          else
          {
              tr69c_saveToAllocBuf(buf, n);
          }
      }
   }
   va_end(ap);
}

/*----------------------------------------------------------------------*/
static void xml_mprintf(tProtoCtx *pc, int *len, char *s)
{
   if (s)
   {
      for (; *s; s++)
      {
         switch (*s)
         {
            case '&':
               mprintf(pc, len, "&amp;");
               break;
            case '<':
               mprintf(pc, len, "&lt;");
               break;
            case '>':
               mprintf(pc, len, "&gt;");
               break;
            case '"':
               mprintf(pc, len, "&quot;");
               break;
            case '\'':
               mprintf(pc, len, "&apos;");
            case 9:
            case 10:
            case 13:
               mprintf(pc, len, "&#%d;", *s);
               break;
            default:
               if (isprint(*s))
               {
                  mprintf(pc, len, "%c", *s);
               }
               else
               {
                  mprintf(pc, len, " ");
               }
               break;
         }
      }
   }
}

#if 0
/*----------------------------------------------------------------------*/
static void XMLmIndent(tProtoCtx *pc, int *len, int indent)
{
   int i;

   if (indent < 0)
   {
      return;
   }

   for (i = 0; i < indent; i++)
   {
      mprintf(pc, len, " ");
   }
}
#endif

char *changeDot(char *pp)
{
    char *temp = pp;
    char *pos = strstr(temp, ".");

    while(pos)
    {
        temp = pos;
        pos = strstr(temp+1, ".");
    }
    
    if (temp && (NULL == pos))
    {
        *temp = ' ';
    }
    
    return pp;
}


RPCAction* getAction()
{
    if (simRpcAction != NULL)
    {
        vosLog_debug("simulate");
        return simRpcAction;
    }
    else
    {
        vosLog_debug("itms and cpe");
        return acsRpcAction;
    }    
}


void wriltComToFile(char *format)
{
    char buf[512] = {0};
    char mark[128] = {0};
    
    UTIL_SNPRINTF(mark, sizeof(mark), "\n----------------------------------------------------------------------\n");
    UTIL_SNPRINTF(buf, sizeof(buf), "%s%s", mark, format);
    UTIL_SNPRINTF(buf, sizeof(buf), "%s%s\n", buf, mark);

    if (simRpcAction != NULL)
    {
        writeLog(buf, util_strlen(buf));
    }
}


void commandPrint(int rpcMethod)
{
    char buf[128] = {0};
    RPCAction *rpcAction = getAction();
    ParamItem   *pi   = rpcAction->ud.pItem;
    AttributeItem *ai = rpcAction->ud.aItem;
    char   *pp = rpcAction->ud.addDelObjectReq.objectName;
    char *name = rpcAction->ud.paramNamesReq.parameterPath;
    char *str_tmp = NULL;

    if (simRpcAction)
    {
        vosLog_debug("acsRpcAction->rpcMethod =%d", rpcAction->rpcMethod);
        switch (rpcMethod)
        {
            case rpcGetParameterValues:              
                if (pi)
                {
                    for( ;pi != NULL; pi = pi->next)
                    {
                        str_tmp = VOS_MALLOC_FLAGS(strlen(pi->pname) + 1, ALLOC_ZEROIZE);
                        UTIL_STRNCPY(str_tmp, pi->pname, strlen(pi->pname) + 1);
                        UTIL_SNPRINTF(buf, sizeof(buf), "tr69c getParamval %s", changeDot(str_tmp));
                        VOS_FREE(str_tmp);
                        wriltComToFile(buf);
                    }
                }
                break;

            case rpcSetParameterValues:
               if (pi)
                {
                    for( ;pi != NULL; pi = pi->next)
                    {                      
                        str_tmp = VOS_MALLOC_FLAGS(strlen(pi->pname) + 1, ALLOC_ZEROIZE);
                        UTIL_STRNCPY(str_tmp, pi->pname, strlen(pi->pname) + 1);
                        UTIL_SNPRINTF(buf, sizeof(buf), "tr69c setParamval %s %s", changeDot(str_tmp), pi->pvalue);                        
                        VOS_FREE(str_tmp);
                        wriltComToFile(buf);
                    }
                }
                break;

            case rpcGetParameterNames:
                if (name)
                {
                    str_tmp = VOS_MALLOC_FLAGS(strlen(name) + 1, ALLOC_ZEROIZE);
                    UTIL_STRNCPY(str_tmp, name, strlen(name) + 1);
                    UTIL_SNPRINTF(buf, sizeof(buf), "tr69c getParamNames %s", changeDot(str_tmp));
                    VOS_FREE(str_tmp);
                    wriltComToFile(buf);
                }                
                break;

            case rpcGetParameterAttributes:
                if (ai)
                {
                    for( ;ai != NULL; ai = ai->next)
                    {                     
                        str_tmp = VOS_MALLOC_FLAGS(strlen(ai->pname) + 1, ALLOC_ZEROIZE);
                        UTIL_STRNCPY(str_tmp, ai->pname, strlen(ai->pname) + 1);
                        UTIL_SNPRINTF(buf, sizeof(buf), "tr69c getParamattr %s", changeDot(str_tmp));
                        VOS_FREE(str_tmp);
                        wriltComToFile(buf);
                    }
                }
                break;

            case rpcSetParameterAttributes:
                if (ai)
                {
                    for( ;ai != NULL; ai = ai->next)
                    {
                        str_tmp = VOS_MALLOC_FLAGS(strlen(ai->pname) + 1, ALLOC_ZEROIZE);
                        UTIL_STRNCPY(str_tmp, ai->pname, strlen(ai->pname) + 1);
                        UTIL_SNPRINTF(buf, sizeof(buf), "tr69c setParamattr %s %d", changeDot(str_tmp), ai->chgNotify);                       
                        VOS_FREE(str_tmp);
                        wriltComToFile(buf);
                    }
                }
                break;

            case rpcAddObject:
                if (pp)
                {                    
                    UTIL_SNPRINTF(buf, sizeof(buf), "tr69c addObj %s", pp);                    
                    wriltComToFile(buf);
                }
                break;

            case rpcDeleteObject:
                if (pp)
                {                   
                    str_tmp = VOS_MALLOC_FLAGS(strlen(pp) + 1, ALLOC_ZEROIZE);
                    UTIL_STRNCPY(str_tmp, pp, strlen(pp) + 1);
                    UTIL_SNPRINTF(buf, sizeof(buf), "tr69c delObj %s", changeDot(str_tmp));                   
                    VOS_FREE(str_tmp);
                    wriltComToFile(buf);
                }
                break;

            case rpcReboot:
                UTIL_SNPRINTF(buf, sizeof(buf), "tr69c reboot");            
                wriltComToFile(buf);
                break;

            case rpcFactoryReset:
                UTIL_SNPRINTF(buf, sizeof(buf), "tr69c factoryreset");               
                wriltComToFile(buf);
                break;

            default: 
                vosLog_debug("No RPCmethod");
        }
    }
}

static void closeBodyEnvelope(tProtoCtx *pc, int *lth)
{
   xml_mIndent(pc, lth, 2);
   mprintf(pc, lth, "</%sBody>\n", nsSOAP);
   mprintf(pc, lth, "</%sEnvelope>\n", nsSOAP);
}

/* Add <SOAP:Body>
*/
static void openBody(tProtoCtx *pc, int *lth)
{
   xml_mIndent(pc, lth, 2);
   mprintf(pc, lth, "<%sBody>\n", nsSOAP);
}

/*
* Add <SOAP:Envelope
 *      xmlns:....
 *      <..:Header>
 *      ....
 *      </..:Header>
 * to the buffer
 */
static void openEnvWithHeader(char *idstr, tProtoCtx *pc, int *lth)
{
   NameSpace   *ns;
   mprintf(pc, lth, "<%sEnvelope", nsSOAP);
   /* generate Namespace declarations */
   ns = nameSpaces;
   while (ns->sndPrefix)
   {
      char    pbuf[40];
      char    *e;
#if 1
      mprintf(pc, lth, " ");
#else
      mprintf(pc, lth, "\n");
#endif
      UTIL_STRNCPY(pbuf, ns->sndPrefix, sizeof(pbuf));
      e=strchr(pbuf,':');
      if (e) *e='\0'; /* find : in prefix */
      xml_mIndent(pc, lth, 2);
      mprintf(pc, lth, "xmlns:%s=\"%s\"", pbuf, ns->nsURL);
      ++ns;
   }
   mprintf(pc, lth, ">\n");
   if (idstr)
   {
      xml_mIndent(pc, lth, 2);
      mprintf(pc, lth, "<%sHeader>\n", nsSOAP);
      xml_mIndent(pc, lth, 3);
      mprintf(pc, lth, "<%sID %smustUnderstand=\"1\">%s</%sID>\n",
              nsCWMP, nsSOAP,idstr , nsCWMP);
      xml_mIndent(pc, lth, 2);
      mprintf(pc, lth, "</%sHeader>\n", nsSOAP);
   }
}

static const char *getFaultCode(int fault)
{
   const char *r;

   switch (fault)
   {
      case 9000:
      case 9001:
      case 9002:
      case 9004:
      case 9009:
      case 9010:
      case 9011:
      case 9012:
      case 9013:
         r = "Server";
         break;
      case 9003:
      case 9005:
      case 9006:
      case 9007:
      case 9008:
         r = "Client";
         break;
      default:
         r = "Vendor";
         break;
   }
   return r;
}

static const char *getFaultStr(int fault)
{
   const char *detailFaultStr = NULL;

   switch (fault)
   {
      case 9000:
         detailFaultStr = "Method not supported";
         break;
      case 9001:
         detailFaultStr = "Request denied";
         break;
      case 9002:
         detailFaultStr = "Internal Error";
         break;
      case 9003:
         detailFaultStr = "Invalid arguments";
         break;
      case 9004:
         detailFaultStr = "Resources Exceeded";
         break;
      case 9005:
         detailFaultStr = "Invalid Parameter Name";
         break;
      case 9006:
         detailFaultStr = "Invalid parameter type";
         break;
      case 9007:
         detailFaultStr = "Invalid parameter value";
         break;
      case 9008:
         detailFaultStr = "Attempt to set a non-writeable parameter";
         break;
      case 9009:
         detailFaultStr = "Notification request rejected";
         break;
      case 9010:
         detailFaultStr = "Download failure";
         break;
      case 9011:
         detailFaultStr = "Upload failure";
         break;
      case 9012:
         detailFaultStr = "File transfer server authentication failure";
         break;
      case 9013:
         detailFaultStr = "Unsupported protocol for file transfer";
         break;
      case 9014:
         detailFaultStr = "MaxEnvelopes exceeded";
         break;
      case 9803:
         if (SF_FEATURE_SUPPORT_CARD)
         {
            detailFaultStr = "Authentication Failure";
         }
         break;
      default:
         detailFaultStr = "Vendor defined fault";
         break;
   }
   return detailFaultStr;
}  /* End of getFaultStr() */  

static int getParamCnt(ParamItem *pi)
{
   int   cnt = 0;

   while (pi != NULL)
   {
      cnt++;
      pi = pi->next;
   }

   return cnt;

}  /* End of getParamCnt() */

static void writeSoapFault(RPCAction *a, int fault)
{
   tProtoCtx *pc = NULL;

   vosLog_debug("=====>ENTER");
   
   tr69c_initAllocBuf();

   do
   {
      int   bufsz = 0;

      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sFault>\n", nsSOAP);
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<faultcode>%s</faultcode>\n", getFaultCode(fault));
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<faultstring>CWMP fault</faultstring>\n");
      xml_mIndent(pc, &bufsz, 5);
      mprintf(pc, &bufsz, "<detail>\n");
      xml_mIndent(pc, &bufsz, 6);
      mprintf(pc, &bufsz, "<%sFault>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 7);
      mprintf(pc, &bufsz, "<FaultCode>%d</FaultCode>\n", fault);
      xml_mIndent(pc, &bufsz, 7);
      mprintf(pc, &bufsz, "<FaultString>%s</FaultString>\n", getFaultStr(fault));
      if (a->rpcMethod == rpcSetParameterValues)
      {
         ParamItem   *pi = a->ud.pItem;
         /* walk thru parameters to generate errors */
        
         while (pi != NULL )
         {
            if (pi->fault)
            {
               xml_mIndent(pc, &bufsz, 7);
               mprintf(pc, &bufsz, "<SetParameterValuesFault>\n");
               xml_mIndent(pc, &bufsz, 8);
               mprintf(pc, &bufsz, "<ParameterName>%s</ParameterName>\n",
                           pi->pname);
               xml_mIndent(pc, &bufsz, 8);
               mprintf(pc, &bufsz, "<FaultCode>%d</FaultCode>\n", pi->fault);
               xml_mIndent(pc, &bufsz, 8);
               mprintf(pc, &bufsz, "<FaultString>%s</FaultString>\n",
                           getFaultStr(pi->fault));
               xml_mIndent(pc, &bufsz, 7);
               mprintf(pc, &bufsz, "</SetParameterValuesFault>\n");
            }
            pi = pi->next;
         }
      }
      xml_mIndent(pc, &bufsz, 6);
      mprintf(pc, &bufsz, "</%sFault>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 5);
      mprintf(pc, &bufsz, "</detail>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sFault>\n", nsSOAP);
      closeBodyEnvelope(pc, &bufsz);
    
      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
  
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
	  
   } while (0); 
   
   tr69c_freeAllocBuf();

}  /* End of writeSoapFault() */

static void doScheduleInform(RPCAction *a)
{
   int interval = 0;
   UINT32 var = 0;
   ParamItem   *pi = a->ud.pItem;
   
   //printf("doScheduleInform name = %p;value = %s\n",a->ud.pItem->pname,a->ud.pItem->pvalue);
  
   acsState.fault = 0;  /* init to no fault */

   // if attribute name is empty then return fault message
   //invalid arguments
    if (pi == NULL) 
    {
        acsState.fault = 9001;
    }

   /* first set attributes */
   while (pi!=NULL && !acsState.fault )
   {
      const char  *pp = pi->pname;
      const char  *pv = pi->pvalue;

      if ((NULL == pp)||(NULL == pv))
      {
          acsState.fault = 9003;      
      }

      if (0 == util_strcmp(pp,"DelaySeconds"))
      {
          interval = atoi(pv);
          if (interval > 0)
          {
              var = (UINT32)interval;
              setScheduleInform(var);              
          }
          else
          {
              acsState.fault = 9003;  
          }
      }
      pi = pi->next;
   }

   if (acsState.fault == 0) 
   {

      tProtoCtx *pc = NULL;

      tr69c_initAllocBuf();
      
      do
      {

         int   bufsz = 0;

         /* build good response */
         openEnvWithHeader(a->ID, pc, &bufsz);
         openBody(pc, &bufsz);
         xml_mIndent(pc,&bufsz, 3);
         mprintf(pc,&bufsz,"<%sScheduleInformResponse/>\n", nsCWMP);
         closeBodyEnvelope(pc, &bufsz);

         /* send the HTTP message header*/
         sendToAcs(bufsz, NULL);
         
         /* send the HTTP message body*/
         pc = getAcsConnDesc();
         proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
		 
      } while (0);
      
      tr69c_freeAllocBuf();
      
   } else {
      writeSoapFault(a, acsState.fault);
      #ifdef DEBUG
      fprintf(stderr, "Fault in ScheduleInform %d\n", acsState.fault);
      #endif
   }
}  /* End of doScheduleInform() */

static void doGetRPCMethods(RPCAction *a)
{
   eRPCMethods m;
   tProtoCtx *pc = NULL;

   tr69c_initAllocBuf();
   do
   {
      int   bufsz = 0;
   
      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sGetRPCMethodsResponse>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 4);
      #ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, &bufsz, "<MethodList>\n");
      #else
      mprintf(pc, &bufsz, "<MethodList %sarrayType=\"%sstring[%d]\">\n",
              nsSOAP_ENC, nsXSD, LAST_RPC_METHOD );
      #endif
   
      for (m = rpcGetRPCMethods; m <= LAST_RPC_METHOD; ++m)
      {
         xml_mIndent(pc, &bufsz, 5);
         mprintf(pc, &bufsz, "<string>%s</string>\n", getRPCMethodName(m));
      }
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "</MethodList>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sGetRPCMethodsResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
	  
   } while (0); 
   
   tr69c_freeAllocBuf();

}  /* End of doGetRPCMethods() */


static void writeGetAttribute(CMC_PHL_GET_PARAM_ATTR_T *pParamAttr, tProtoCtx *pc, int *bufsz)
{
   unsigned int   accessList;

   /* now fill in ParameterAttributeStruct in response */
   xml_mIndent(pc, bufsz, 5);
   mprintf(pc, bufsz, "<ParameterAttributeStruct>\n");
   xml_mIndent(pc, bufsz, 6);
   mprintf(pc, bufsz, "<Name>%s</Name>\n", pParamAttr->paramPath);
   xml_mIndent(pc, bufsz, 6);
   mprintf(pc, bufsz, "<Notification>%d</Notification>\n",
            pParamAttr->notification);
   accessList = pParamAttr->accessBitMask;
   if (accessList == 0 || accessList == NDA_ACCESS_TR69C)
   {
      xml_mIndent(pc, bufsz, 6);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, bufsz, "<AccessList>\n");
#else
      mprintf(pc, bufsz, "<AccessList %sarrayType=\"%sstring[0]\">\n",
                 nsSOAP_ENC, nsXSD);
#endif
      xml_mIndent(pc, bufsz, 6);
      mprintf(pc, bufsz, "</AccessList>\n");
   }
   else
   {
      /*
       * If we get into this else block, then accessList definately
       * has a bit set for some entity that is not TR69C.  So just
       * report that as "Subscriber" has write access to the ACS.
       */
      xml_mIndent(pc, bufsz, 6);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, bufsz, "<AccessList>\n");
#else
      mprintf(pc, bufsz, "<AccessList %sarrayType=\"%sstring[1]\">\n",
              nsSOAP_ENC, nsXSD);
#endif
      xml_mIndent(pc, bufsz, 7);
      mprintf(pc, bufsz, "<string>Subscriber</string>\n");
      xml_mIndent(pc, bufsz, 6);
      mprintf(pc, bufsz, "</AccessList>\n");
   }
   xml_mIndent(pc, bufsz, 5);
   mprintf(pc, bufsz, "</ParameterAttributeStruct>\n");
}  /* End of writeGetAttribute() */

static void writeGetPName(CMC_PHL_GET_PARAM_NAME_T *paramInfo, tProtoCtx *pc, int *bufsz)
{
    
   /* convert MDM name path back to TR69 name string */
   xml_mIndent(pc, bufsz, 5);
   mprintf(pc, bufsz, "<ParameterInfoStruct>\n");
   xml_mIndent(pc, bufsz, 6);
   mprintf(pc, bufsz, "<Name>%s</Name>\n", paramInfo->paramPath);
   xml_mIndent(pc, bufsz, 6);
   mprintf(pc, bufsz, "<Writable>%s</Writable>\n", paramInfo->writable?"1":"0");
   xml_mIndent(pc, bufsz, 5);
   mprintf(pc, bufsz, "</ParameterInfoStruct>\n");

}  /* End of writeGetPName() */


static VOS_RET_E tr69c_getParamNameList(const char *fullPath,
                                        UBOOL8 nextLevelOnly,
                                        UBOOL8 *isParamPath,
                                        CMC_PHL_GET_PARAM_NAME_T **paramNameList,
                                        UINT32 *paramNum)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 len = 0;
    UINT32 reqDataLen = 0;
    UINT8 *reqBuf = NULL;
    VosMsgHeader *reqMsg = NULL;
    VosMsgHeader *respMsg = NULL;
    UINT8 *respBuf = NULL;

    vosLog_debug("Enter>, fullPath = %p(%s), nextLevelOnly = %u, isParamPath = %p, paramValue = %p, paramNum = %p",
        fullPath, fullPath ? fullPath : "", nextLevelOnly, isParamPath, paramNameList, paramNum);

    if (NULL == fullPath)
    {
        len = 1;
    }
    else
    {
        len = util_strlen(fullPath) + 1;
    }

    reqDataLen = sizeof(UBOOL8) + len;
    reqMsg = (VosMsgHeader *)VOS_MALLOC_FLAGS(sizeof(VosMsgHeader) + reqDataLen, ALLOC_ZEROIZE);
    if (NULL == reqMsg)
    {
        vosLog_error("VOS_MALLOC_FLAGS failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    reqBuf = (UINT8 *)(reqMsg + 1);

    *((UBOOL8 *)reqBuf) = nextLevelOnly;
    reqBuf += sizeof(UBOOL8);
    
    if (NULL != fullPath)
    {
        UTIL_STRNCPY((char *)reqBuf, fullPath, len);
        reqBuf += len;
    }
  
    reqMsg->type = VOS_MSG_TR69C_GET_NAME_LIST;
    reqMsg->src = EID_TR69C;
    reqMsg->dst = EID_CMC;
    reqMsg->flags_request = 1;
    reqMsg->flags_response = 0;
    reqMsg->dataLength = reqDataLen;
    
    ret = vosMsg_sendAndGetReplyBuf(g_msgHandle, reqMsg, &respMsg);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("vosMsg_sendAndGetReplyBuf failed (ret=%d)", ret);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(reqMsg);
        
        return ret;
    }

    ret = (VOS_RET_E)respMsg->wordData;
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("respMsg return %d)", ret);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(reqMsg);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(respMsg);
        return ret;
    }

    respBuf = (UINT8 *)(respMsg + 1);
    len = respMsg->dataLength;

    if (isParamPath)
    {
        *isParamPath = *((UBOOL8 *)respBuf);
    }
    respBuf += sizeof(UBOOL8);
    len -= sizeof(UBOOL8);

    if (paramNum)
    {
        *paramNum = len / sizeof(CMC_PHL_GET_PARAM_NAME_T);
    }

    if (paramNameList)
    {
        *paramNameList = (CMC_PHL_GET_PARAM_NAME_T *)VOS_MALLOC_FLAGS(len, ALLOC_ZEROIZE);
        if (NULL == *paramNameList)
        {
            vosLog_error("VOS_MALLOC_FLAGS failed");
            ret = VOS_RET_RESOURCE_EXCEEDED;
        }
        else
        {
            memcpy(*paramNameList, respBuf, len);
        }
    }
    
    VOS_MEM_FREE_BUF_AND_NULL_PTR(reqMsg);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(respMsg);

    return ret;
}


/* 
* GetParameterNames requests a single parameter path or single parameter path fragment 
*/
static void doGetParameterNames(RPCAction *a)
{
   int i = 0;
   UINT32 paramNum = 0;
   tProtoCtx *pc = NULL;
   UBOOL8 nextLevelOnly = (UBOOL8)a->ud.paramNamesReq.nextLevel;
   UBOOL8 pathIsEmpty = FALSE;
   UBOOL8 firstParam = TRUE;
   UBOOL8 isparamterPath = FALSE;
   CMC_PHL_GET_PARAM_NAME_T *paramNameList = NULL;

   vosLog_debug("==Enter==");

   acsState.fault = VOS_RET_SUCCESS; /* init to no fault */
   char *pp = a->ud.paramNamesReq.parameterPath;

   tr69c_initAllocBuf();
   
   do
   {
      int bufsz = 0;

      /* create response msg start */
      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sGetParameterNamesResponse>\n", nsCWMP);

      /* copy parameter list */
      xml_mIndent(pc, &bufsz, 4);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, &bufsz, "<ParameterList>\n");
#else
      /* In the first loop paramNum=0.  In the second loop, paramNum will have
       * the actual parameter count.
       */
      if(SF_FEATURE_CUSTOMER_3BB)
      {
          paramNum=3000;
          mprintf(pc, &bufsz,
              "<ParameterList %sarrayType=\"%sParameterInfoStruct[%04d]\">\n",
              nsSOAP_ENC, nsCWMP, paramNum);
      }
      else
      {
          mprintf(pc, &bufsz,
              "<ParameterList %sarrayType=\"%sParameterInfoStruct[%04d]\">\n",
              nsSOAP_ENC, nsCWMP, paramNum);
      }
#endif

      if (pp == NULL || util_strlen(pp) == 0)
      {
         pathIsEmpty = TRUE;
      }

      acsState.fault = tr69c_getParamNameList(pp, nextLevelOnly, &isparamterPath, &paramNameList, &paramNum);
      
      if (acsState.fault == VOS_RET_SUCCESS)
      {
          if (isparamterPath)
          {
              writeGetPName(&paramNameList[0], pc, &bufsz);
          }
          else
          {
              firstParam = FALSE;

              for(i = 0; i < (int)paramNum; i++)
              {
                  /* if nextLevelOnly is true, the first parameter name that matches
                       the given partial path object name should NOT be included
                       in the GetParameterNamesResponse */
                  if (!firstParam || (nextLevelOnly == FALSE))
                  {
                      writeGetPName(&paramNameList[i], pc, &bufsz);
                  }
                  else if (pathIsEmpty && nextLevelOnly)
                  {
                    
                      /* However, for the special case where ACS does a GetParameterNames
                           with a blank name, then return the first name,
                           which is InternetGatewayDevice. */
                      writeGetPName(&paramNameList[i], pc, &bufsz);
                  }

                  if (pathIsEmpty && nextLevelOnly)
                  {
                    
                      /* out of while loop after write out the first object "InternetGatewayDevice." */
                      break; 
                  }
              }
              
          }
      }
      else if (acsState.fault == VOS_RET_NO_MORE_INSTANCES)
      {
         acsState.fault = VOS_RET_SUCCESS;
      }
      else
      {
         vosLog_error("get param name error for %d",acsState.fault);
      }
 
      VOS_MEM_FREE_BUF_AND_NULL_PTR(paramNameList);

      /* if ParameterPath is empty, with NextLevel is true, the response
          should list only "InternetGatewayDevice.". */


      if (acsState.fault != VOS_RET_SUCCESS)
      {
         break;   /* quit */
      }

      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "</ParameterList>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sGetParameterNamesResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);
      
      commandPrint(rpcGetParameterNames);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
   } while(0);
   
   tr69c_freeAllocBuf();

   if (acsState.fault != VOS_RET_SUCCESS)
   {
      /* build fault here */
      writeSoapFault(a, acsState.fault);
      vosLog_error("Fault %d", acsState.fault);
   }
}  /* doGetParameterNames() */

static void doSetParameterAttributes(RPCAction *a)
{
   AttributeItem  *pi = a->ud.aItem;

   acsState.fault = VOS_RET_SUCCESS; /* init to no fault */

   vosLog_debug("==Enter==");
   
   if (pi != NULL)
   {
      UINT32 paramNum = 0;
      CMC_PHL_SET_PARAM_ATTR_T *pSetParamAttrList = NULL;

      /* find out the total number of parameters requested */
      while (pi != NULL)
      {
         paramNum++;
         pi = pi->next;
      }
      pi = a->ud.aItem;

      /* allocate memory for the set parameter value list */
      pSetParamAttrList = VOS_MALLOC_FLAGS(sizeof(CMC_PHL_SET_PARAM_ATTR_T) * TR69C_PARAM_SET_ATTR_ARRAY, ALLOC_ZEROIZE);
      if (pSetParamAttrList == NULL)
      {
         vosLog_error("doSetParameterAttributes: malloc failed\n");
         acsState.fault = VOS_RET_INTERNAL_ERROR;
      }
      else
      {
         CMC_PHL_SET_PARAM_ATTR_T *pSetParamAttr = pSetParamAttrList;

         while (pi != NULL)
         {
            /*acsState.fault = CMC_tr69cFullPathToPathDescriptor(pi->pname,
                                                 &(pSetParamAttr->pathDesc));
            if (acsState.fault != VOS_RET_SUCCESS)
            {
               break;
            }*/
            UTIL_STRNCPY(pSetParamAttr->paramPath, pi->pname, TR69C_PARAM_FULL_PATH_LENGTH);
            pSetParamAttr->notificationChange = (UBOOL8)pi->chgNotify;
            pSetParamAttr->notification = (UINT8)pi->notification;
            pSetParamAttr->accessBitMaskChange = (UBOOL8)pi->chgAccess;
            pSetParamAttr->accessBitMask = NDA_ACCESS_TR69C;

            if (pi->subAccess != 0)
            {
               pSetParamAttr->accessBitMask |=  NDA_ACCESS_SUBSCRIBER;
            }

            pSetParamAttr++;
            pi = pi->next;
         }
   
         if (acsState.fault == VOS_RET_SUCCESS)
         {
            acsState.fault = CMC_phlSetParamAttrList(pSetParamAttrList, paramNum);
         }
         /* free pSetParamAttrList buffer */
         VOS_MEM_FREE_BUF_AND_NULL_PTR(pSetParamAttrList);
      }
   }
   else
   {
      /* no parameter specified - Invalid arguments */
      acsState.fault = VOS_RET_INVALID_ARGUMENTS;
   }
 
   if (acsState.fault == VOS_RET_SUCCESS)
   {
      tProtoCtx *pc = NULL;

      tr69c_initAllocBuf();
	  
      do
      {
         int   bufsz = 0;

         openEnvWithHeader(a->ID, pc, &bufsz);
         openBody(pc, &bufsz);
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz, "<%sSetParameterAttributesResponse/>\n", nsCWMP);
         closeBodyEnvelope(pc, &bufsz);
         
         commandPrint(rpcSetParameterAttributes);
		 
         /* send the HTTP message header*/
         sendToAcs(bufsz, NULL);
         
         /* send the HTTP message body*/
         pc = getAcsConnDesc();
         proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
		 
      } while (0);
      
      tr69c_freeAllocBuf();

      saveConfigFlag = TRUE;
      
   }
   else
   {
      writeSoapFault(a, acsState.fault);  
      vosLog_debug("Fault %d", acsState.fault);
   }
}  /* End of doSetParameterAttributes() */

/* 
* GetParameterAttributes requests a single parameter path or single parameter path fragment
* This RPC uses the paramItem union member. 
*/
static void doGetParameterAttributes(RPCAction *a)
{
   int i = 0;
   UINT32 paramNum = 0;
   tProtoCtx *pc = NULL;
   UBOOL8 nextLevelOnly = FALSE;
   UBOOL8 isParamterPath = FALSE;
   CMC_PHL_GET_PARAM_ATTR_T *paramAttr = NULL;

   vosLog_debug("==Enter==");

   acsState.fault = VOS_RET_SUCCESS; /* init to no fault */
          
   tr69c_initAllocBuf();
   
   do
   {
      int         bufsz = 0;
      ParamItem   *pi   = a->ud.pItem;

      if (pi == NULL)
      {
         /* no parameter specified - Invalid arguments */
         acsState.fault = VOS_RET_INVALID_ARGUMENTS;
         break;   /* quit */
      }

      /* create response msg start */
      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sGetParameterAttributesResponse>\n", nsCWMP);

      /* copy parameter list */
      xml_mIndent(pc, &bufsz, 4);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, &bufsz, "<ParameterList>\n");
#else
      /* In the first loop paramNum=0.  In the second loop, paramNum will have
       * the actual parameter count.
       */
      mprintf(pc, &bufsz,
              "<ParameterList %sarrayType=\"%sParameterAttributeStruct[%04d]\">\n",
              nsSOAP_ENC, nsCWMP, paramNum);
#endif
      paramNum = TR69C_PARAM_GET_ATTR_ARRAY;  /* reset paramNum */

      for ( ; pi != NULL && acsState.fault == VOS_RET_SUCCESS; pi = pi->next)
      {
         char *pp = pi->pname;

         paramAttr = VOS_MALLOC_FLAGS(sizeof(CMC_PHL_GET_PARAM_ATTR_T) * TR69C_PARAM_GET_ATTR_ARRAY, ALLOC_ZEROIZE);
         memset(paramAttr, 0, sizeof(CMC_PHL_GET_PARAM_ATTR_T) * TR69C_PARAM_GET_ATTR_ARRAY);
         CMC_PHL_GET_PARAM_ATTR_T *pa = paramAttr;

         acsState.fault = CMC_phlGetParamAttrList(pp, nextLevelOnly, &isParamterPath, paramAttr, &paramNum);

         if (acsState.fault == VOS_RET_SUCCESS)
         {
            if (isParamterPath)
            {
               writeGetAttribute(pa, pc, &bufsz);
            }
            else
            {
               for(i = 0; i < (int)paramNum; i++)
               {
                  writeGetAttribute(pa, pc, &bufsz);
                  pa ++;
               }
            }
         }
         else if (acsState.fault == VOS_RET_NO_MORE_INSTANCES)
         {
            acsState.fault = VOS_RET_SUCCESS;
         }
         else
         {
            vosLog_error("get param attribute error for %d",acsState.fault);
         }

      VOS_MEM_FREE_BUF_AND_NULL_PTR(paramAttr);
      }  /* for () */

      if (acsState.fault != VOS_RET_SUCCESS)
      {
         break;   /* quit */
      }

      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "</ParameterList>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sGetParameterAttributesResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);
      
      commandPrint(rpcGetParameterAttributes);
	  
      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
	  
   } while (0);
   
   tr69c_freeAllocBuf();
             
   if (acsState.fault != VOS_RET_SUCCESS)
   {
      /* build fault here */
      writeSoapFault(a, acsState.fault);
      vosLog_debug("Fault %d", acsState.fault);
   }
}  /* End of doGetParameterAttributes() */


void Sendalarmmsgtoitms(char *errorcode)
{
#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
    if (SF_FEATURE_SUPPORT_CARD)
    {
        char buf[sizeof(VosMsgHeader)] = {0};
        VosMsgHeader *tr69msg = (VosMsgHeader *)buf;
        
        VOS_RET_E ret = VOS_RET_SUCCESS;
        /*send alarm to itms*/
        tr69msg->type = VOS_MSG_CT_CARDALARM;
        tr69msg->src = EID_TR69C;
        tr69msg->dst = EID_TR69C;
        tr69msg->flags_event = 1;
        tr69msg->dataLength = 0;

        CMC_cardSetTr69cManageErrorcode(errorcode);

        if ((ret = vosMsg_send(g_msgHandle, tr69msg)) != VOS_RET_SUCCESS)
        {
            vosLog_error("Could not send out CMS_MSG_UPNP_GETALL ret=%d", ret);
        }
    }
#endif    
}


typedef enum
{
    CMC_TR69C_CARD_WRITE_SUCCESS = 0,
    CMC_TR69C_CARD_WRITE_FAIL    = 1
} CMC_TR69C_CARD_WRITE_E;


#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
static void Files_writeandread()
{
    UINT32 i = 0;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    printf("func = %s***line = %d\n", __FUNCTION__, __LINE__);
    //first write card
    if ((ret = CMC_cardFilesWriteIntoCard()) == VOS_RET_SUCCESS)
    {
        //write card success
        if (1 == g_statusresult_recv)
        {
            CMC_tr69cSetCardManageStatus(3);
        }
        else
        {
            CMC_tr69cSetCardManageStatus(2);
        }
    }
    else //first write card failed,Repeat three times
    {
        for(i = 0; i < 3; i++)
        {
            if ((ret = CMC_cardFilesWriteIntoCard()) == VOS_RET_SUCCESS)
            {
                //write card success
                if (1 == g_statusresult_recv)
                {
                    CMC_tr69cSetCardManageStatus(3);
                }
                else
                {
                    CMC_tr69cSetCardManageStatus(2);
                }
                break;
            }
        }
        if (ret!= VOS_RET_SUCCESS)
        {
            //Repeat three times,but failed
            CMC_tr69cSetCardManageStatus(0);
        }
    }

    printf("func = %s***line = %d\n", __FUNCTION__, __LINE__);

    /*only recv result value,then send writed even*/
    if (1 == g_statusresult_recv)
    {
        addInformEventToList(INFORM_EVENT_CT_CARDWRITE);
        sendInform(NULL);
        printf("func = %s***line = %d\n", __FUNCTION__, __LINE__);

        if (ret != VOS_RET_SUCCESS)
        {
            Sendalarmmsgtoitms("105002");
            CMC_cardChangeWanToDefault();
            CMC_cardReadFirstInternetAndTr069File();
        }
        else
        {
            CMC_cardSetTerminalType(2);
            UTIL_DO_SYSTEM_ACTION("echo cardterminal > /var/config/terminaltype");
        }
        g_statusresult_recv = 0;
    }
}
#endif


typedef struct
{
    MdmPathDescriptor   pathDesc;   /**< Full path name of a parameter. */   
    char                *pValue;    /**< Pointer to parameter value string. */   
    VOS_RET_E              status;     /**< Paremeter value set status. */
} PhlWriteParamValue_t;


#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
static void doWriteintoCard(RPCAction *a)
{
    CMC_TR69C_DEV_REG_CFG_T userInfo;
    UINT32 check_status = 0;
    UINT32 card_status = 0;
    
    if (rpcSetParameterValues == a->rpcMethod)
    {
        ParamItem *pi = a->ud.pItem;
        if (pi != NULL)
        {
            UINT32 numEntries = 0;
            PhlWriteParamValue_t *pWriteParamValueList = NULL;

            numEntries = getParamCnt(pi);
            pWriteParamValueList = VOS_MALLOC_FLAGS(numEntries * sizeof(PhlWriteParamValue_t), ALLOC_ZEROIZE);
            if (pWriteParamValueList == NULL)
            {
                vosLog_error("doWriteParameterValues: malloc failed\n");
            }
            else
            {
                PhlWriteParamValue_t pWriteParamValue = *(pWriteParamValueList + (numEntries - 1));
                while (pi != NULL)
                {
                    pi->fault = CMC_cardGetObjectNumber(pi->pname, &(pWriteParamValue.pathDesc));

                    if (numEntries > 0)
                    {
                        numEntries--;
                        pWriteParamValue = *(pWriteParamValueList + (numEntries - 1));
                    }
                    pi = pi->next;
                }
            }
            VOS_MEM_FREE_BUF_AND_NULL_PTR(pWriteParamValueList);
        }
    }
    else if ((rpcAddObject == a->rpcMethod) || (rpcDeleteObject == a->rpcMethod))
    {
        char *pp  = a->ud.addDelObjectReq.objectName;
        PhlWriteParamValue_t *pWriteParamValueList1 = NULL;
        pWriteParamValueList1 = VOS_MALLOC_FLAGS(sizeof(PhlWriteParamValue_t), ALLOC_ZEROIZE);
        if (pWriteParamValueList1 == NULL)
        {
            vosLog_error("doWriteParameterValues: malloc failed\n");
        }
        else
        {
            CMC_cardGetObjectNumber(pp, &(pWriteParamValueList1->pathDesc));
        }
        VOS_MEM_FREE_BUF_AND_NULL_PTR(pWriteParamValueList1);
    }
    
    if (CMC_tr69cGetUserInfo(&userInfo) != VOS_RET_SUCCESS)
    {
        vosLog_error("get MDMOID_USER_INFO failed !");
    }

    if ((1 == userInfo.result) || (!((0 == userInfo.status) && (0 == userInfo.result))))
    {
        CMC_cardGetCardStatus(&card_status);
        if (1 == card_status)
        {
            if (1 == userInfo.result)
            {
                CMC_cardBusinessLedOn();
                CMC_cardGetCardkeyAndCheck(&check_status);
            }
            Files_writeandread();
        }
    }
    else if (2 == userInfo.result)
    {
        CMC_cardBusinessLedOff();
    }
}
#endif

VOS_RET_E tr69c_setParamValues(char **paramArray, UINT32 paramNum, VOS_RET_E *status)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 i = 0;
    UINT32 len = 0;
    VosMsgHeader *reqMsg = NULL;
    VosMsgHeader *respMsg = NULL;
    UINT8 *reqBuf = NULL;
    UINT8 *respBuf = NULL;

    vosLog_debug("Enter>");
    
    if (NULL == paramArray)
    {
        vosLog_debug("paramArray is NULL");
        return VOS_RET_INVALID_ARGUMENTS;
    }

    len = sizeof(UINT32);
    for (i = 0; i < paramNum * 2; i++)
    {
        len += (UINT32)util_strlen(paramArray[i]) + 1;
    }

    reqMsg = (VosMsgHeader *)VOS_MALLOC_FLAGS(sizeof(VosMsgHeader) + len, ALLOC_ZEROIZE);
    if (NULL == reqMsg)
    {
        vosLog_error("VOS_MALLOC_FLAGS failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    reqMsg->type = VOS_MSG_TR69C_SET_VALUE;
    reqMsg->src = EID_TR69C;
    reqMsg->dst = EID_CMC;
    reqMsg->flags_request = 1;
    reqMsg->flags_response = 0;
    reqMsg->dataLength = len;
    
    reqBuf = (UINT8 *)(reqMsg + 1);
    *((UINT32 *)reqBuf) = paramNum;
    reqBuf += sizeof(UINT32);
    
    for (i = 0; i < paramNum * 2; i++)
    {
        len = (UINT32)util_strlen(paramArray[i]) + 1;
        UTIL_STRNCPY((char *)reqBuf, paramArray[i], len);
        reqBuf += len;
    }

    /*t = vosMsg_sendAndGetReplyBuf(g_msgHandle, reqMsg, &respMsg);*/
    ret = vosMsg_sendAndGetReplyBufWithTimeout(g_msgHandle, reqMsg, &respMsg,8000);
    if (VOS_RET_SUCCESS != ret)
    {
        if(VOS_RET_TIMED_OUT == ret)
        {
            vosLog_error("send reqMsg VOS_MSG_TR69C_SET_VALUE time out");
        }
        else
        {
            vosLog_error("send reqMsg VOS_MSG_TR69C_SET_VALUE failed");
            VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);
            return ret;
        }
       
    }

    /*if timeout ,send successful to itms default*/
    if(VOS_RET_TIMED_OUT == ret)
    {
        ret = VOS_RET_SUCCESS;
    }
    else
    {
        ret = (VOS_RET_E)respMsg->wordData;
        respBuf = (UINT8 *)(respMsg + 1);
        if (respMsg->dataLength != (sizeof(VOS_RET_E) * paramNum))
        {
            vosLog_error("respMsg->dataLength != sizeof(VOS_RET_E) * paramNum");
        }
        else
        {
            memcpy(status, respBuf, (int)respMsg->dataLength);
        }
    }
    
    VOS_MEM_FREE_BUF_AND_NULL_PTR(reqMsg);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(respMsg);

    return ret;
}

static void doSetParameterValues(RPCAction *a)
{
    int    setParamReboot = rebootFlag;
    ParamItem   *pi            = a->ud.pItem;
    int configFileSavedToFlash = 0;
    int iskeyvalue = 0;
    UINT32 stop_write_flag = 0;
    UBOOL8 ctcstp_flg = FALSE;
    UBOOL8 isSetParamOk = TRUE;
    char board_id[16] = {0};    
    vosLog_debug("==Enter==");

    acsState.fault = VOS_RET_SUCCESS; /* init to no fault */

    g_keyhaveset = 0;
    g_statushaveset = 0;
    g_statusautheachother = 0;
    wan_laninterface = 0;
    ParamPathList *paramList = NULL;

#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
    UINT32 cardtype = 0;
    CMC_cardGetCardType(&cardtype);
    if (SF_FEATURE_SUPPORT_CARD)
    {
        if (1 == cardtype)
        {
            CMC_TR69C_SIM_CARD_CFG_T cardManager;
            CMC_tr69cGetCardManager(&cardManager);
            /*if card is down ,no need to set the param*/
         
            if (cardManager.cardStatus == 0)
            {
                CMC_cardStatusLedOff();
                printf("Card is down,doSetParameterValues failed\n");
                return ;
            }
        }
    }
#endif

   if (pi != NULL)
   {
      UINT32 paramNum = 0;
      VOS_RET_E status[TR69C_PARAM_SET_VALUE_ARRAY] = {VOS_RET_SUCCESS};
      char* errstring= NULL;
      
      /* find out the total number of parameters requested */
      paramNum = (UINT32)getParamCnt(pi);

/* Add by tuhanyu, for upnp device management of separate AP, 2011/04/15, start */
    if (SF_FEATURE_SUPPORT_UPNP_DMCP)
    {

        ParamItem   *pitem   = a->ud.pItem;
        void* list = NULL;
        int len;

         for ( ; pitem != NULL ; pitem = pitem->next)
         {
             char     *pp = pitem->pname;
             char     *pv=pitem->pvalue; 
             
            if (SF_FEATURE_SUPPORT_CT_LOOPDETECT)
            {
                if (util_strstr(pp,"X_CT-COM_LoopbackDetection"))
                {
                    ctcstp_flg = TRUE;
                }
            }

            if (SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU)
            {
                if (util_strstr(pp, STR_CARD_USERINFO_NAME))
                {
                    loid_changed_flag = TRUE;
                }
            }


#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
             if (SF_FEATURE_SUPPORT_CARD)
             {
                if (1 == cardtype)
                {
                    if (util_strstr(pp, STR_CARD_KEYPARAM))
                    {
                        /*for card key must ATH first*/
                        char card_key[CARD_KEY_LEN + 1] = {0};
                        UINT32 ret = 0;
                        CMC_cardInit();

                        UTIL_STRNCPY((char*)card_key, pitem->pvalue, sizeof(card_key));

                        vosLog_debug("card_key=%s\n", card_key);
                        CMC_cardTermCardCheckEachOther(card_key, &ret);
                        /*Ath OK*/
                        if (ret == 0)
                        {
                            g_keyhaveset = 1;
                            CMC_cardSetCheckStatus(1);
                            UTIL_DO_SYSTEM_ACTION("soft_feature set support_voip true");
                        }
                        else
                        {
                            UINT32 i = 0;
                            UINT32 resultcard = 0;
                            for (i = 0; i < 3; ++i)
                            {
                                CMC_cardTermCardCheckEachOther(card_key, &resultcard);
                                
                                if (0 == resultcard)
                                {
                                    g_keyhaveset = 1;
                                    CMC_cardSetCheckStatus(1);
                                    UTIL_DO_SYSTEM_ACTION("soft_feature set support_voip true");
                                    break;
                                }
                            }
                            if (resultcard != 0)
                            {
                                g_keyhaveset = 0;
                                g_statusautheachother = resultcard;
                                CMC_cardSetCheckStatus(0);
                                UTIL_DO_SYSTEM_ACTION("soft_feature set support_voip false");
                            }
                        }
                        iskeyvalue = 1;
                    }
                   
                    if (util_strstr(pp, STR_CARD_USERINFO_NAME) || util_strstr(pp, STR_CARD_USERINFO_PASS))
                    {
                        /* build fault response */
                        writeSoapFault(a, 9802);
                        printf("doSetParameterValues faulte code is %d\n", 9802);
                        vosLog_debug("Fault %d", 9802);
                        return;                
                    }
                   

                    if ((1 == iskeyvalue) && (0 == g_keyhaveset))
                    {
                        break;
                    }

                    if (!util_strcmp(pp, CHECK_LOID_STATUSPARAM))
                    {
                        if ((!util_strcmp("1", pitem->pvalue))
                            || (!util_strcmp("2", pitem->pvalue))
                            || (!util_strcmp("3", pitem->pvalue)))
                        {
                            g_statushaveset = 1;
                            CMC_cardBusinessLedOff();
                        }
                        else if (!util_strcmp("5", pitem->pvalue))
                        {
                            CMC_cardBusinessLedOn();
                        }
                    }

                    if (!util_strcmp(pp, CHECK_LOID_RESULTPARAM))
                    {
                        g_statusresult_recv = 1;
                    }

                    CMC_cardGetStopCardWrittingFlag(&stop_write_flag);
                 }
                 else
                 {
                    if ((!util_strcmp(pp, CHECK_LOID_RESULTPARAM)) && (!util_strcmp("1", pitem->pvalue)))
                    {
                        CMC_cardSetTerminalType(1);
                        UTIL_DO_SYSTEM_ACTION("echo notcardterminal > /var/config/terminaltype");
                    }
                 }
             }
#endif
            if (!util_strcmp(pp, DOWNLOAD_DIAG_STATE_PARAM))
            {
                download_diag = 1;
            }

            if ((!util_strcmp(pp, DOWNLOAD_DIAG_INTER_PARAM))
             || (!util_strcmp(pp, DOWNLOAD_DIAG_URL_PARAM))
             || (!util_strcmp(pp, DOWNLOAD_DIAG_DSCP_PARAM))
             || (!util_strcmp(pp, DOWNLOAD_DIAG_ETHPRI_PARAM)))
            {
                change_download_param = 1;
            }

            if (!util_strcmp(pp, UPLOAD_DIAG_STATE_PARAM))
            {
                upload_diag = 1;
            }

            if ((!util_strcmp(pp, UPLOAD_DIAG_INTER_PARAM))
             || (!util_strcmp(pp, UPLOAD_DIAG_URL_PARAM))
             || (!util_strcmp(pp, UPLOAD_DIAG_DSCP_PARAM))
             || (!util_strcmp(pp, UPLOAD_DIAG_ETHPRI_PARAM))
             || (!util_strcmp(pp, UPLOAD_DIAG_FILELEN_PARAM)))
            {
                change_upload_param = 1;
            }

            if ((util_strstr(pp, STR_WAN_LANINTERFACE) && SF_FEATURE_LOCATION_NINGXIA)
                || (util_strstr(pp, STR_WAN_LANINTERFACE_CU) && SF_FEATURE_LOCATION_SHANDONG)
                || (util_strstr(pp, STR_WAN_LANINTERFACE) && SF_FEATURE_LOCATION_JIANGXI && SF_FEATURE_CUSTOMER_ASB))
            {
                HAL_sysGetBoardId(board_id,16);
                if(!util_strcmp(board_id, "968385PVSFUEPON") || !util_strcmp(board_id, "968385PVSFU"))
                    wan_laninterface = 1;
            }
            #if 0
            if (util_strstr(pp, STR_VOICE_LINE))
            {
                voice_line = 1;
            }
            #endif 
                
            len = util_strlen(pp)- util_strlen("InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.");
            if ((len>0)&&util_strstr(pp,"DeviceList.") && !util_strstr(pp,"ActionList."))
            {
                addToParamPathList(pp,&list,pv);    
            }
        }

        paramList = (ParamPathList *)list;
        while (paramList)
        {
            int errorCode = 0;
            int result = 0;
            result = CMC_phlUpdateUpnpProxyDevice(paramList->paramPath, paramList->value, 1, &errorCode, errstring);
            
            if (result)
            {
                vosLog_error("  error\n");

                isSetParamOk = FALSE;
            }

            paramList = paramList->next;
        }

        paramList = NULL;
        freeParamPathList(&list);

        if (!isSetParamOk)
        {
            acsState.fault = VOS_RET_INTERNAL_ERROR;
            writeSoapFault(a, acsState.fault);
            return;
        }
    }
/* Add by tuhanyu, end */
        if (SF_FEATURE_SUPPORT_CARD && (((1 == iskeyvalue) && (0 == g_keyhaveset)) || stop_write_flag))
        {
            acsState.fault = VOS_RET_FAIL_REBOOT_REQUIRED;
        }
        else
        {
            char **paramArray = NULL;
            UINT32 i = 0;
            /*
                    * The tr69c xml parser (see fParameterNames) links the
                    * parameter values in reverse order as they arrive over the
                    * network.  So when we build the pSetParamValue struct,
                    * build it in reverse order again so that the ODL sees the
                    * same order as the ACS sent out.
                    */

            paramArray = VOS_MALLOC_FLAGS(sizeof(char *) * paramNum * 2, ALLOC_ZEROIZE);
            if (NULL == paramArray)
            {
                vosLog_error("VOS_MALLOC_FLAGS failed");
                return;
            }

            while (pi != NULL)
            {
                /*
                            * 1/21/08 plugfest: even when we detect an error early here,
                            * keep going into cmcPhl_setParameterValues so we can detect
                            * any other errors in the param list.
                            */

                if (NULL == pi->pvalue) /* only valid for tSting params */
                {
                    /* fake up a null string to avoid NULL ptr problem*/
                    pi->pvalue = VOS_STRDUP("");
                }

                if(SF_FEATURE_LOCATION_FUJIAN || SF_FEATURE_LOCATION_NINGXIA || SF_FEATURE_LOCATION_SHANDONG || SF_FEATURE_LOCATION_SHANGHAI)
                {
                    CMC_PHL_GET_PARAM_ATTR_T paramAttr;
                    if(NULL != util_strstr(pi->pname, "InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.") && 
                        VOS_RET_SUCCESS != CMC_phlGetParamAttr(pi->pname, &paramAttr))
                    {
                        paramNum--;                        
                    } 
                    else if ((SF_FEATURE_LOCATION_NINGXIA || SF_FEATURE_LOCATION_SHANDONG) && 
                        NULL != util_strstr(pi->pname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration") && 
                        VOS_RET_SUCCESS != CMC_phlGetParamAttr(pi->pname, &paramAttr))
                    {
                        paramNum--;
                    }
                    else
                    {
                        paramArray[i++] = pi->pname;
                        paramArray[i++] = pi->pvalue;
                    }        
                }
                else if ((SF_FEATURE_LOCATION_JIANGXI) && (SF_FEATURE_CUSTOMER_ASB))
                {
                    CMC_PHL_GET_PARAM_ATTR_T paramAttr;

                    HAL_sysGetBoardId(board_id,16);
                    if(!util_strcmp(board_id, "968385PVSFUEPON"))
                    {
                        if(NULL != util_strstr(pi->pname, "InternetGatewayDevice.Services.VoiceService.1.VoiceProfile.1.Line.2.")
                           && VOS_RET_SUCCESS != CMC_phlGetParamAttr(pi->pname, &paramAttr))
                        {
                            paramNum--;                        
                        } 
                        else if (NULL != util_strstr(pi->pname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration")
                                 && VOS_RET_SUCCESS != CMC_phlGetParamAttr(pi->pname, &paramAttr))
                        {
                            paramNum--;
                        }
                        else
                        {
                            paramArray[i++] = pi->pname;
                            paramArray[i++] = pi->pvalue;
                        }
                    }
                    else
                    {
                        paramArray[i++] = pi->pname;
                        paramArray[i++] = pi->pvalue;
                    }
                }
                else
                {
                    paramArray[i++] = pi->pname;
                    paramArray[i++] = pi->pvalue;
                }


                if (util_strcmp(pi->pname, "InternetGatewayDevice.DeviceConfig.ConfigFile") == 0)
                {
                    /* This parameter takes ACS config file and saved to flash; 
                                    do not overwrite the config file at the end of set RPC */
                    configFileSavedToFlash = 1;
                }
               
                pi = pi->next;
            }


            acsState.fault = tr69c_setParamValues(paramArray, paramNum, status);
            vosLog_error("acsState.fault = %d", acsState.fault);
            if (acsState.fault == VOS_RET_SUCCESS_REBOOT_REQUIRED)
            {
                if (!SF_FEATURE_LOCATION_SHANGHAI && !SF_FEATURE_SUPPORT_PLUGIN)
                {
                    setParamReboot = 1;
                }
                acsState.fault = VOS_RET_SUCCESS;
            }
            else if (wan_laninterface && acsState.fault != VOS_RET_SUCCESS)
            {
                acsState.fault = VOS_RET_SUCCESS;
            }
            #if 0
            else if (voice_line && acsState.fault != VOS_RET_SUCCESS)
            {
                acsState.fault = VOS_RET_SUCCESS;
            }
            #endif 
            else if (acsState.fault == VOS_RET_INVALID_ARGUMENTS)
            {
                int id = (int)paramNum -1;
                pi = a->ud.pItem;

                while (pi != NULL)
                {
                    pi->fault = status[id];
                    id --;
                    pi = pi->next;
                }
            }
            else if (VOS_RET_WAN_CONNTYPE_NOT_IPROUTE == acsState.fault) //si chuan
            {
                acsState.fault = VOS_RET_SUCCESS_REBOOT_REQUIRED;
            }

            vosLog_error("acsState.fault = %d", acsState.fault);

            VOS_MEM_FREE_BUF_AND_NULL_PTR(paramArray);

            if (SF_FEATURE_SUPPORT_CT_LOOPDETECT)
            {
                if (ctcstp_flg == TRUE)
                {
                    /*send msg to ctcstp to config param if there ehave config param for loopbackdetection*/
                    VosMsgHeader msg = EMPTY_MSG_HEADER;
                    msg.type = VOS_MSG_CT_CONFIGD_ETECTION_PARAM;
                    msg.src =  EID_TR69C;
                    msg.dst = EID_CTCSTP;
                    msg.flags_event = 1;
                    if (vosMsg_send(g_msgHandle, &msg) != VOS_RET_SUCCESS)
                    {
                        vosLog_error("could not send out CMS_MSG_CT_CONFIGD_ETECTION_PARAM event msg");
                    }
                    else
                    {
                        vosLog_debug("Send out CMS_MSG_CT_CONFIGD_ETECTION_PARAM event msg.");
                    } 
                }
            }
        }  

        VOS_MEM_FREE_BUF_AND_NULL_PTR(errstring);
   }
   else
   {
      /* no parameter specified - Invalid arguments */
      acsState.fault = VOS_RET_INVALID_ARGUMENTS;
   }

   if (acsState.fault == VOS_RET_SUCCESS)
   {
      /* build good response */
      tProtoCtx *pc = NULL;

      if (!configFileSavedToFlash)
      {
         saveConfigFlag = TRUE;
      }

      /* 网关不能够自动重启，需要平台下发重启命令才能重启*/
      #if 0
      if (setParamReboot)
      {
         rebootFlag = eACSSetValueReboot;
      }
      #endif

      tr69c_initAllocBuf();
      
      do
      {
         int   bufsz = 0;

         openEnvWithHeader(a->ID, pc, &bufsz);
         openBody(pc, &bufsz);
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz, "<%sSetParameterValuesResponse>\n", nsCWMP);
         xml_mIndent(pc, &bufsz, 4);
         if (SF_FEATURE_SUPPORT_CARD)
         {
            #ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
            if (1 == cardtype)
                mprintf(pc, &bufsz, "<Status>%s</Status>\n", setParamReboot?"3":"2");
            else
                mprintf(pc, &bufsz, "<Status>%s</Status>\n", setParamReboot?"1":"0");
            #endif
         }
         else
         {
            mprintf(pc, &bufsz, "<Status>%s</Status>\n", setParamReboot?"1":"0");
         }
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz,"</%sSetParameterValuesResponse>\n", nsCWMP);
         closeBodyEnvelope(pc, &bufsz);
         
         commandPrint(rpcSetParameterValues);
		 
         /* send the HTTP message header*/
         sendToAcs(bufsz, NULL);
         
         /* send the HTTP message body*/
         pc = getAcsConnDesc();
         proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
		 
      } while (0);
	  

      tr69c_freeAllocBuf();
   }
   else
   {
      /* build fault response */
      writeSoapFault(a, acsState.fault);
      vosLog_error("Fault %d", acsState.fault);
   }
}  /* End of doSetParameterValues() */


/*
* write SOAP value with type at buffer location **bp. Limit to size *bufsz.
* increments paramNum for each value written to buffer.
*/
static void writeGetPValue(char *path, char *type, char *value,  tProtoCtx *pc, int *bufsz)
{
    /* now fill in ParameterValueStruct in response */
    xml_mIndent(pc, bufsz, 6);
    
    mprintf(pc, bufsz, "<ParameterValueStruct>\n");
    xml_mIndent(pc, bufsz, 7);
    mprintf(pc, bufsz, "<Name>%s</Name>\n", path);
    xml_mIndent(pc, bufsz, 7);

    mprintf(pc, bufsz, "<Value %stype=\"%s%s\">",
               nsXSI, nsXSD, type);
    
#ifdef SUPPRESS_EMPTY_PARAM
    if (empty(value))
    { 
        xml_mprintf(pc, bufsz, "empty");
    }
    else
    {
        xml_mprintf(pc, bufsz, value);
    }
#else
    xml_mprintf(pc, bufsz, value);
#endif

    mprintf(pc, bufsz, "</Value>\n");
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz, "</ParameterValueStruct>\n");
}  /* End of writeGetPValue() */


static void writeGetParamValue(char *pv, tProtoCtx *pc, int *bufsz, UINT32 paramNum)
{
    char *name = NULL;
    char *value = NULL;
    char *type = NULL;
    UINT32 len = 0;
    UINT32 paramIdex = 0;

    for (paramIdex = 1; paramIdex <= paramNum && (pv != NULL); paramIdex++)
    {
        len = util_strlen(pv) + 1;
        name = (char *)VOS_MALLOC_FLAGS(len, ALLOC_ZEROIZE);
        UTIL_STRNCPY(name, pv, len);
        pv +=  len;

        len = util_strlen(pv) + 1;
        value = (char *)VOS_MALLOC_FLAGS(len, ALLOC_ZEROIZE);
        UTIL_STRNCPY(value, pv, len);
        pv += len;

        len = util_strlen(pv) + 1;
        type = (char *)VOS_MALLOC_FLAGS(len, ALLOC_ZEROIZE);
        UTIL_STRNCPY(type, pv, len);
        pv += len;
        
        writeGetPValue(name, type, value, pc, bufsz);

        VOS_MEM_FREE_BUF_AND_NULL_PTR(name);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(type);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(value);
    }
}


VOS_RET_E freeGetParamValueBuf(CMC_PHL_GET_PARAM_VALUE_T *buf,
                                 SINT32             paramNum)
{
    SINT32 i;
    CMC_PHL_GET_PARAM_VALUE_T *ptr = buf;

    for (i = 0; (i < paramNum) && (ptr != NULL); i++, ptr++)
    {
        VOS_FREE(ptr->value);
    }
    VOS_FREE(buf);

    return VOS_RET_SUCCESS;
}


VOS_RET_E tr69c_getParamValue(const char *fullPath, char **paramValue)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 pathLen = (NULL == fullPath) ? 0 : (util_strlen(fullPath) + 1);
    UINT32 reqBufSize = sizeof(VosMsgHeader) + pathLen;
    UINT8 *reqBuf = (UINT8 *)VOS_MALLOC_FLAGS(reqBufSize, ALLOC_ZEROIZE);
    VosMsgHeader *reqMsg = (VosMsgHeader *)reqBuf;
    VosMsgHeader *respMsg = NULL;
    char *respBuf = NULL;

    vosLog_debug("Enter>, fullPath = %p(%s)", fullPath, fullPath ? fullPath : "");
    
    if (NULL == reqMsg)
    {
        vosLog_error("VOS_MALLOC_FLAGS failed");
        VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    if (NULL != fullPath)
    {
        UTIL_STRNCPY((char *)&reqBuf[sizeof(VosMsgHeader)], fullPath, pathLen);
    }
  
    reqMsg->type = VOS_MSG_TR69C_GET_VALUE;
    reqMsg->src = EID_TR69C;
    reqMsg->dst = EID_CMC;
    reqMsg->flags_request = 1;
    reqMsg->flags_response = 0;
    reqMsg->dataLength = reqBufSize - sizeof(VosMsgHeader);
    
    ret = vosMsg_sendAndGetReplyBuf(g_msgHandle, reqMsg, &respMsg);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("sendAndGetReply failed (ret=%d)", ret);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);
        return ret;
    }

    ret = (VOS_RET_E)respMsg->wordData;
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("respMsg return %d", ret);
    }
    else
    {
        respBuf = (char *)(respMsg + 1);
        
        if (paramValue)
        {
            *paramValue = (char *)VOS_STRDUP(respBuf);
        }
    }

    VOS_MEM_FREE_BUF_AND_NULL_PTR(respMsg);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);

    return ret;
}


VOS_RET_E tr69c_getParamValues(const char *fullPath,
                                   UBOOL8 *isParamPath,
                                   UINT8 **paramValue,
                                   UINT32 *paramNum)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 pathLen = (NULL == fullPath) ? 0 : (util_strlen(fullPath) + 1);
    UINT32 reqBufSize = sizeof(VosMsgHeader) + pathLen;
    UINT8 *reqBuf = (UINT8 *)VOS_MALLOC_FLAGS(reqBufSize, ALLOC_ZEROIZE);
    VosMsgHeader *reqMsg = (VosMsgHeader *)reqBuf;
    VosMsgHeader *respMsg = NULL;
    UINT8 *respBuf = NULL;
    UINT32 valueSize = 0;
    UINT32 offLen = 0;

    vosLog_debug("Enter>, fullPath = %p(%s), isParamPath = %p, paramValue = %p, paramNum = %p",
        fullPath, fullPath ? fullPath : "", isParamPath, paramValue, paramNum);

    if (NULL == reqBuf)
    {
        vosLog_error("VOS_MALLOC_FLAGS failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }
    
    if (NULL != fullPath)
    {
        UTIL_STRNCPY((char *)&reqBuf[sizeof(VosMsgHeader)], fullPath, pathLen);
    }
  
    reqMsg->type = VOS_MSG_TR69C_GET_VALUE_LIST;
    reqMsg->src = EID_TR69C;
    reqMsg->dst = EID_CMC;
    reqMsg->flags_request = 1;
    reqMsg->flags_response = 0;
    reqMsg->dataLength = pathLen;
    
    ret = vosMsg_sendAndGetReplyBuf(g_msgHandle, reqMsg, &respMsg);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("vosMsg_sendAndGetReplyBuf failed (ret=%d)", ret);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);
        
        return ret;
    }

    ret = (VOS_RET_E)respMsg->wordData;
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("respMsg return %d)", ret);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(respMsg);
        return ret;
    }

    respBuf = (UINT8 *)(respMsg + 1);

    if (paramNum)
    {
        *paramNum = *((UINT32 *)respBuf);
    }
    offLen += sizeof(UINT32);

    if (isParamPath)
    {
        *isParamPath = *((UBOOL8 *)&respBuf[offLen]);
    }
    offLen += sizeof(UBOOL8);

    valueSize = respMsg->dataLength - offLen;

    if (paramValue)
    {
        *paramValue = (UINT8 *)VOS_MALLOC_FLAGS(valueSize, ALLOC_ZEROIZE);
        if (NULL == *paramValue)
        {
            vosLog_error("VOS_MALLOC_FLAGS failed");
            ret = VOS_RET_RESOURCE_EXCEEDED;
        }
        else
        {
            memcpy(*paramValue, &respBuf[offLen], valueSize);
        }
    }
    
    VOS_MEM_FREE_BUF_AND_NULL_PTR(reqBuf);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(respMsg);

    return ret;
}

static void doGetParameterValues(RPCAction *a)
{
    UINT32 paramNum = 0;
    tProtoCtx *pc = NULL;
    UBOOL8 isParamPath = FALSE;

    vosLog_debug("==Enter==");

    acsState.fault = VOS_RET_SUCCESS;   /* init to no fault */
          
   /* Loop through the code twice.  The first loop is to calculate
    * the size of the entire soap envelope so that the HTTP header
    * can be written to the TCP socket first.  The second loop is
    * to write the content of the envelope to the TCP socket as it
    * is generated.
    */
    
    ParamItem   *pitem   = a->ud.pItem;
    void* list = NULL;
    int result =-1;
    char* errstring= NULL;
    char     *ppn;
    int len;
    InstanceIdStack iidStack_dl = EMPTY_INSTANCE_ID_STACK;
    ParamPathList *paramList = NULL;
    ParamItem phead;
     if (SF_FEATURE_SUPPORT_UPNP_DMCP)
    {
#ifdef __DEBUG_UPNP_TR69_SIMULATE
        if (pitem)
        {    
            ppn = pitem->pname;
            len = util_strlen(ppn)- util_strlen("InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.");     
            if ((len>=0)&&util_strstr(ppn,"DeviceList.")&&!util_strstr(ppn, "ActionList"))
            {    
                char* p;    
                char tmp[256];
                UTIL_STRNCPY(tmp,ppn,sizeof(tmp)); 
    
                p= util_strstr(tmp,"DeviceList.")+util_strlen("DeviceList.");
                while (*(p))
                {
                    if (isdigit(*p))
                    {
                        p++;
                    }
                    else
                    {
                        //*p = '\0';  //exlude .
                        break;
                    }
                }
                
                p++;
                
                UTIL_STRNCPY(p ,"Monitoring.", BUFLEN_32); 
                addToParamPathList(tmp,&list,NULL);    
                    
                UTIL_STRNCPY(p ,"ServiceObject.", BUFLEN_32); 
                addToParamPathList(tmp,&list,NULL);    
                
                UTIL_STRNCPY(p ,"DeviceInfo.", BUFLEN_32); 
                addToParamPathList(tmp,&list,NULL);            
            }
        }
#else
        for ( ; pitem != NULL ; pitem = pitem->next)
        {
           ppn = pitem->pname;
           len = util_strlen(ppn)- util_strlen("InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.");     
           if ((len>0)&&util_strstr(ppn,"DeviceList")&&!util_strstr(ppn, "ActionList"))
           {
               addToParamPathList(ppn,&list,NULL);    
           } 
           else if (0== util_strcmp(ppn,"InternetGatewayDevice.")||
                   (0== util_strcmp(ppn,"InternetGatewayDevice.X_CT-COM_ProxyDevice."))||
                   (0== util_strcmp(ppn,"InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.")))
           {
               char aName[256];
               
               INIT_INSTANCE_ID_STACK(&iidStack_dl);
               
               while (VOS_RET_SUCCESS == CMC_tr69cGetDeviceList(&iidStack_dl))
               {
                   UTIL_SNPRINTF(aName, sizeof(aName),"InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.%d.",
                   iidStack_dl.instance[iidStack_dl.currentDepth-1]);
                   addToParamPathList(aName,&list,NULL);   
               }
               
           }
       }
#endif

       paramList = (ParamPathList *)list;
       while (paramList)
       {
           int errorCode = 0;
           
           result = CMC_phlUpdateUpnpProxyDevice(paramList->paramPath, paramList->value, 0, &errorCode, errstring);
           
           if (result)
           {
               vosLog_error("  error\n");    
           }
           paramList = paramList->next;
       }
       freeParamPathList(&list);
   }

   tr69c_initAllocBuf();

   do
   {
      int         bufsz = 0;
      ParamItem   *pi   = a->ud.pItem;

      if (NULL == pi)
      {
         /* no parameter specified - Invalid arguments */
         acsState.fault = VOS_RET_INVALID_ARGUMENTS;
         break;   /* quit */
      }
      
      if(SF_FEATURE_LOCATION_GUANGDONG)
      {     
          phead.next = a->ud.pItem;          
          inversionParamItem(&phead);
          pi = phead.next;
      }
      
      /* create response msg start */
      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz,"<%sGetParameterValuesResponse>\n", nsCWMP);

      /* copy parameter list */
      xml_mIndent(pc, &bufsz, 4);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, &bufsz, "<ParameterList>\n");
#else
      /* In the first loop paramNum=0.  In the second loop, paramNum will have
       * the actual parameter count.
       */
      mprintf(pc, &bufsz,
              "<ParameterList %sarrayType=\"%sParameterValueStruct[%04d]\">\n",
              nsSOAP_ENC, nsCWMP, paramNum);
#endif
      paramNum = TR69C_PARAM_GET_VALUE_ARRAY;  /* reset paramNum */

      for ( ; pi != NULL && acsState.fault == VOS_RET_SUCCESS; pi = pi->next)
      {
         char *pp = pi->pname;
         UINT8 *paramValue = NULL;

         acsState.fault = tr69c_getParamValues(pp, &isParamPath, &paramValue, &paramNum);
         if (acsState.fault == VOS_RET_SUCCESS)
         {
            writeGetParamValue((char *)paramValue, pc, &bufsz, paramNum);
         }
         else if (acsState.fault == VOS_RET_NO_MORE_INSTANCES)
         {
            acsState.fault = VOS_RET_SUCCESS;
         }
         else
         {
            vosLog_error("CMC_phlGetNextParamValue error %d", acsState.fault);
         }

         VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
      }  /* for () */

      
      if(SF_FEATURE_LOCATION_GUANGDONG)
      {     
          inversionParamItem(&phead);
          pi = phead.next;
      }

      if (acsState.fault != VOS_RET_SUCCESS)
      {
         break;   /* quit */
      }

      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "</ParameterList>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sGetParameterValuesResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);    

      xml_mIndent(pc, &bufsz, MAX_PADDINGS);
      commandPrint(rpcGetParameterValues);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
	  
    }  while (0);
	
    tr69c_freeAllocBuf();

    if (acsState.fault != VOS_RET_SUCCESS)
    {
        writeSoapFault(a, acsState.fault);
        vosLog_debug("Fault %d", acsState.fault);
    }

    /* Add by tuhanyu, for upnp device management of separate AP, 2011/04/15, start */
    if (SF_FEATURE_SUPPORT_UPNP_DMCP)
    {
       VOS_MEM_FREE_BUF_AND_NULL_PTR(errstring);
    }
}



/* AddObject*/
static void doAddObject(RPCAction *a)
{
   char              *pp = a->ud.addDelObjectReq.objectName;
   int               len;
   UINT32 instanceNum = 0;

   vosLog_debug("==Enter==");

   acsState.fault = VOS_RET_SUCCESS; /* init to no fault */
   
   /* The path name must end with a "." (dot) after the last node
    * in the hierarchical name of the object.
    */
   acsState.fault = VOS_RET_INVALID_PARAM_NAME;
   if ((pp != NULL) &&
       ((len = util_strlen(pp)) > 0) && (len <= 256) &&
       (pp[len-1] == '.'))
   {
      char  *pLastToken;

      pp[len-1] = 0;
      if (((pLastToken = strrchr(pp, (int)'.')) != NULL) &&
          (isalpha(*(++pLastToken))))
      {
         acsState.fault = VOS_RET_SUCCESS;
      }
      pp[len-1] = '.';
   }

   if (acsState.fault == VOS_RET_SUCCESS)
   {
       acsState.fault = CMC_phlAddInstance(pp, &instanceNum);
       if (acsState.fault == VOS_RET_SUCCESS_REBOOT_REQUIRED)
       {
          if (!SF_FEATURE_SUPPORT_PLUGIN)
          {
              rebootFlag     = eACSAddObjectReboot;//no reboot
          }
          acsState.fault = VOS_RET_SUCCESS;
       }
   }

   if (acsState.fault == VOS_RET_SUCCESS)
   {
      /* build AddObject response */
      tProtoCtx *pc = NULL;

      tr69c_initAllocBuf();
      
      do
      {
         int   bufsz = 0;

         openEnvWithHeader(a->ID, pc, &bufsz);
         openBody(pc, &bufsz);
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz, "<%sAddObjectResponse>\n", nsCWMP);
         xml_mIndent(pc, &bufsz, 4);
         mprintf(pc, &bufsz, "<InstanceNumber>%d</InstanceNumber>\n", instanceNum);
         xml_mIndent(pc, &bufsz, 4);
         mprintf(pc, &bufsz, "<Status>%s</Status>\n", rebootFlag? "1":"0");
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz, "</%sAddObjectResponse>\n", nsCWMP);
         closeBodyEnvelope(pc, &bufsz);
         
         commandPrint(rpcAddObject);
		 
         /* send the HTTP message header*/
         sendToAcs(bufsz, NULL);
         
         /* send the HTTP message body*/
         pc = getAcsConnDesc();
         proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);

      } while (0);
	  
      tr69c_freeAllocBuf();

      saveConfigFlag = TRUE;
   }
   else
   {
      /* build fault here */
      writeSoapFault(a, acsState.fault);
      vosLog_debug("Fault %d", acsState.fault);
   }
}  /* End of doAddObject() */

static void doDeleteObject(RPCAction *a)
{
   char              *pp = a->ud.addDelObjectReq.objectName;
   int               len;

   vosLog_debug("==Enter==");

   acsState.fault = VOS_RET_SUCCESS; /* init to no fault */

   /* The path name must end with a "." (dot) after the instance
    * number of the object.
    */
   if ((pp != NULL) &&
       ((len = util_strlen(pp)) > 1) && (len <= 256) &&
       (pp[len-1] == '.') && (isdigit(pp[len-2])))
   {
      acsState.fault = VOS_RET_SUCCESS;
   }
   else
   {
      acsState.fault = VOS_RET_INVALID_PARAM_NAME;
   }

   if (acsState.fault == VOS_RET_SUCCESS)
   {
      acsState.fault = CMC_phlDelInstance(pp);
      if (acsState.fault == VOS_RET_SUCCESS_REBOOT_REQUIRED)
      {
          if (!SF_FEATURE_SUPPORT_PLUGIN)
          {
              rebootFlag     = eACSDelObjectReboot;
          }
          acsState.fault = VOS_RET_SUCCESS;
      }
   }

   if (acsState.fault == VOS_RET_SUCCESS)
   {
      /* build DeleteObject response */
      tProtoCtx *pc = NULL;

      tr69c_initAllocBuf();
	  
      do
      {
         int   bufsz = 0;

         openEnvWithHeader(a->ID, pc, &bufsz);
         openBody(pc, &bufsz);
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz, "<%sDeleteObjectResponse>\n", nsCWMP);
         xml_mIndent(pc, &bufsz, 4);
         mprintf(pc, &bufsz, "<Status>%s</Status>\n", rebootFlag? "1":"0");
         xml_mIndent(pc, &bufsz, 3);
         mprintf(pc, &bufsz, "</%sDeleteObjectResponse>\n", nsCWMP);
         closeBodyEnvelope(pc, &bufsz);
         
         commandPrint(rpcDeleteObject);
		 
         /* send the HTTP message header*/
         sendToAcs(bufsz, NULL);
         
         /* send the HTTP message body*/
         pc = getAcsConnDesc();
         proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);

      } while (0);
	  
      tr69c_freeAllocBuf();

      saveConfigFlag = TRUE;
   }
   else
   {
      /* build fault here */
      writeSoapFault(a, acsState.fault);
      vosLog_debug("Fault %d", acsState.fault);
   }
}  /* End of doDeleteObject() */

static void doRebootRPC(RPCAction *a)
{
    tProtoCtx *pc = NULL;

   tr69c_initAllocBuf();
   
   do
   {
      int   bufsz = 0;

      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sRebootResponse/>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);
      
      commandPrint(rpcReboot);
	  
      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);

   } while (0);
   
   tr69c_freeAllocBuf();

    /* 平台下发重启，响应平台后，等待5s再重启，
       确保平台下发完工单后立刻下发重启，能弹出"注册成功，下发业务成功"页面 */
    sleep(5);

    if (simRpcAction != NULL)
    {
        rebootFlag = eACSRPCReboot;
        rebootCompletion();
    }
    else
    {
        rebootFlag = eACSRPCReboot;
    }

}  /* End of doRebootRPC() */


static void doFactoryResetRPC(RPCAction *a)
{
   tProtoCtx *pc = NULL;

   tr69c_initAllocBuf();
   
   do
   {
      int   bufsz = 0;

      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sFactoryResetResponse/>\n", nsCWMP);
      closeBodyEnvelope(pc,&bufsz);
      
      commandPrint(rpcFactoryReset);
	  
      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);

   } while (0);
   
   tr69c_freeAllocBuf();

    if (simRpcAction != NULL)
    {
        factoryResetFlag = 1;
        factoryResetCompletion();
    }
    else
    {
        factoryResetFlag = 1;
        /* remote reset need inform INFORM_EVENT_BOOTSTRAP*/
        addInformEventToList(INFORM_EVENT_BOOTSTRAP);
    }

}  /* End of doFactoryResetRPC() */


static VOS_RET_E tr69c_wanServModeMask2String(UINT32 servModeMask, char *servMode, UINT32 len)
{
    UBOOL8  firstFlag = TRUE;
    
    vosLog_debug("Enter>, servModeMask = %u, servMode = %p, len = %u", servModeMask, servMode, len);

    if(NULL == servMode)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    
    servMode[0] = '\0';
    
    if (servModeMask & SERV_MODE_TR069)
    {         
        if (firstFlag)
        {
            firstFlag = FALSE;
            UTIL_STRNCPY(servMode, "TR069", len);
        }
        else
        {
            UTIL_STRNCAT(servMode, ",", len);
            UTIL_STRNCAT(servMode, "TR069", len);
        }
    }

    if (servModeMask & SERV_MODE_VOIP)
    {
        if (firstFlag)
        {
            firstFlag = FALSE;
            UTIL_STRNCPY(servMode, "VOIP", len);
        }
        else
        {
            UTIL_STRNCAT(servMode, ",", len);
            UTIL_STRNCAT(servMode, "VOIP", len);
        }
    }
    
    if (servModeMask & SERV_MODE_INTERNET)
    {
        if (firstFlag)
        {
            firstFlag = FALSE;
            UTIL_STRNCPY(servMode, "INTERNET", len);
        }
        else
        {
            UTIL_STRNCAT(servMode, ",", len);
            UTIL_STRNCAT(servMode, "INTERNET", len);
        }
    }

    if (servModeMask & SERV_MODE_OTHER)
    {
        if (firstFlag)
        {
            firstFlag = FALSE;
            UTIL_STRNCPY(servMode, "OTHER", len);          
        }
        else
        {
            UTIL_STRNCAT(servMode, ",", len);
            UTIL_STRNCAT(servMode, "OTHER", len);           
        }
    }

    if (servModeMask & SERV_MODE_IPTV)
    {
        if (firstFlag)
        {
            firstFlag = FALSE;
            UTIL_STRNCPY(servMode, "IPTV", len);          
        }
        else
        {
            UTIL_STRNCAT(servMode, ",", len);
            UTIL_STRNCAT(servMode, "IPTV", len);           
        }
    }

    vosLog_debug("servMode = %s", servMode);
    return VOS_RET_SUCCESS;
}


void tr69c_GetMemOccupyInfo(char *memInfo, UINT32 len)
{
    FILE *fp = NULL;
    char cmd[BUFLEN_48] = {0};
    char buf[BUFLEN_80] = {0};
    
    UINT32 totalmm = 0;
    UINT32 totalActive = 0;
    char tempMemInfo[BUFLEN_128] = {0};

    UTIL_SNPRINTF(cmd, sizeof(cmd), "%s", "cat /proc/meminfo > /var/meminfo");
    UTIL_DO_SYSTEM_ACTION(cmd);

    fp = fopen("/var/meminfo", "r");

    if (fp != NULL)
    {
        while (fgets(buf, 128, fp) != NULL)
        {
            if (strstr(buf, "MemTotal:") != NULL)
            {
                sscanf(buf, "%*s%d%*s", &totalmm);
            }
            if (strstr(buf, "Active:") != NULL)
            {
                sscanf(buf, "%*s%d%*s", &totalActive);
            }
        }
    }
    else
    {
        vosLog_error("open /var/meminfo failed");
    }

    fclose(fp);

    //vosLog_debug("MemTotal = %d,MemFree = %d,Cached = %d", totalmm, totalfree, totalcache);
    UTIL_SNPRINTF(tempMemInfo, sizeof(tempMemInfo), "%d", (totalActive*100/totalmm));
    vosLog_debug("tempMemInfo = %s", tempMemInfo);

    UTIL_STRNCPY(memInfo, tempMemInfo, len);

    UTIL_DO_SYSTEM_ACTION("rm -rf /var/meminfo");
}


void tr69c_GetCpuOccupyInfo(char *cpuInfo, UINT32 len)
{
    char name_1[20] = {0};
    UINT32 user_1 = 0;
    UINT32 nice_1 = 0;
    UINT32 system_1 = 0;
    UINT32 idle_1 = 0;
    UINT32 iowait_1 = 0;
    UINT32 irq_1 = 0;
    UINT32 softirq_1 = 0;
    char name_2[20] = {0};
    UINT32 user_2 = 0;
    UINT32 nice_2 = 0;
    UINT32 system_2 = 0;
    UINT32 idle_2 = 0;
    UINT32 iowait_2 = 0;
    UINT32 irq_2 = 0;
    UINT32 softirq_2 = 0;
    FILE *fd = NULL;
    char buff[256] = {0};
    UINT32 cpu_use = 0;
    UINT32 cpuSum1 = 0;
    UINT32 cpuSum2 = 0;

    fd = fopen("/proc/stat", "r");
    if (fd != NULL)
    {
        fgets(buff, sizeof(buff), fd);
        sscanf(buff, "%s %u %u %u %u %u %u %u",
               name_1, &user_1, &nice_1, &system_1, &idle_1, &iowait_1, &irq_1, &softirq_1);
        fclose(fd);
    }

    memset(buff, 0, sizeof(buff));
    sleep(1);

    fd = fopen("/proc/stat", "r");
    if (fd != NULL)
    {
        fgets(buff, sizeof(buff), fd);
        sscanf(buff, "%s %u %u %u %u %u %u %u",
               name_2, &user_2, &nice_2, &system_2, &idle_2, &iowait_2, &irq_2, &softirq_2);
        fclose(fd);
    }

    cpuSum1 = user_1 + nice_1 + system_1 + idle_1 + iowait_1 + irq_1 + softirq_1;
    cpuSum2 = user_2 + nice_2 + system_2 + idle_2 + iowait_2 + irq_2 + softirq_2;

    if (cpuSum1 != cpuSum2)
    {
        cpu_use = (user_2 + system_2 + nice_2 - user_1 - system_1 - nice_1)*100/(cpuSum2 - cpuSum1);
    }
    if (0 == cpu_use)
    {
        cpu_use = 1;
    }
    else if (cpu_use > 100)
    {
        cpu_use = 100;
    }

    vosLog_debug("cpu: %u\n", cpu_use);
    UTIL_STRNCPY(cpuInfo, itoa(cpu_use), len);
}


void TR69C_writePathAndUintValue(char *path, UINT32 value, tProtoCtx *pc, int *bufsz, int *paramNum)
{
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz, "<ParameterValueStruct>\n");
    xml_mIndent(pc, bufsz, 7);
    mprintf(pc, bufsz, "<Name>%s</Name>\n", path);
    xml_mIndent(pc, bufsz, 7);
    
    mprintf(pc, bufsz, "<Value %stype=\"%sunsignedInt\">", nsXSI, nsXSD);
    mprintf(pc, bufsz, "%d", value);
    mprintf(pc, bufsz, "</Value>\n");
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz,"</ParameterValueStruct>\n");   

    (*paramNum)++;
}


static void tr69c_writePathAndStringValue(char *path, char *value, tProtoCtx *pc, int *bufsz, int *paramNum)
{
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz, "<ParameterValueStruct>\n");
    xml_mIndent(pc, bufsz, 7);
    mprintf(pc, bufsz, "<Name>%s</Name>\n", path);
    xml_mIndent(pc, bufsz, 7);
    
    mprintf(pc, bufsz, "<Value %stype=\"%sstring\">", nsXSI, nsXSD);
    mprintf(pc, bufsz, "%s", value);
    mprintf(pc, bufsz, "</Value>\n");
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz,"</ParameterValueStruct>\n");   

    (*paramNum)++;
}


static VOS_RET_E tr69c_buildDeviceInfo(int i, tProtoCtx *pc, int *bufsz)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char *pDevName = NULL;
    
    pDevName = strrchr(informDevIds[i], '.');
    pDevName++;

    xml_mIndent(pc, bufsz, 5);

    if (!util_strcmp(pDevName, "Manufacturer"))
    {
        mprintf(pc, bufsz, "<%s>%s</%s>\n",
        pDevName, acsState.manufacturer, pDevName);
    }
    else if (!util_strcmp(pDevName, "ManufacturerOUI"))
    {
        mprintf(pc, bufsz, "<OUI>%s</OUI>\n", acsState.manufacturerOUI);
    }
    else if (!util_strcmp(pDevName, "ProductClass"))
    {
        mprintf(pc, bufsz, "<%s>%s</%s>\n",
        pDevName, acsState.productClass, pDevName);
    }
    else if (!util_strcmp(pDevName, "SerialNumber"))
    {
        mprintf(pc, bufsz, "<%s>%s</%s>\n",
        pDevName, acsState.serialNumber, pDevName);
    }
    else if (SF_FEATURE_LOCATION_YUNNAN && !util_strcmp(pDevName, "DeviceType"))
    {
        mprintf(pc, bufsz, "<%s>%s</%s>\n",
        pDevName, acsState.deviceType, pDevName);
    }
    else if (SF_FEATURE_LOCATION_YUNNAN && !util_strcmp(pDevName, "AccessType"))
    {
        mprintf(pc, bufsz, "<%s>%s</%s>\n",
        pDevName, acsState.accessType, pDevName);
    }
    
    return ret;
}


static int tr69c_buildBootInfo(int bootstrap, int x_ct_com_xbind, int valuechange, int x_ct_longreset)
{
    int paralistSupportCardmon = 0;
    static UBOOL8 del_flag = FALSE;
    
    if (1 == bootstrap)
    {
        /*we got BOOTSTRAP event, so discard all event*/
        clearInformEventListNoBackup();

        addInformEventToList(INFORM_EVENT_BOOTSTRAP);
        
        if(SF_FEATURE_LOCATION_FUJIAN)
        {
            addInformEventToList(INFORM_EVENT_BOOT);        
        }
        
        if (SF_FEATURE_SUPPORT_CT_USERINFO 
         && !SF_FEATURE_UPLINK_TYPE_EOC 
         && !SF_FEATURE_LOCATION_JIANGSU 
         && !SF_FEATURE_LOCATION_SUZHOU
         && !SF_FEATURE_LOCATION_SICHUAN 
         && !SF_FEATURE_LOCATION_GUANGXI
         && !SF_FEATURE_LOCATION_SHANGHAI
         && !SF_FEATURE_LOCATION_FUJIAN
         && !SF_FEATURE_SUPPORT_CARD
         && !SF_FEATURE_LOCATION_GUANGDONG)
        {
            addInformEventToList(INFORM_EVENT_CT_USERINFO);
        }

        if (SF_FEATURE_SUPPORT_CARD)
        {
            paralistSupportCardmon = 1;
            UINT32 cardtype = 0;
#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
            CMC_cardGetCardType(&cardtype);
#endif
            if (1 == cardtype)
            {
                //no need to report INFORM_EVENT_CT_LONGRESET
                FILE *fp = NULL;
                char file_buf[64] = {0};

                if (NULL == (fp = fopen("/var/config/remote_resetstatus", "r")))
                {
                    vosLog_debug("can not open file /var/config/remote_resetstatus");
                    
                    if (SF_FEATURE_SUPPORT_CT_USERINFO)
                    {
                        addInformEventToList(INFORM_EVENT_CT_USERINFO);
                    }
                }
                else
                {
                    del_flag = TRUE;
                    fgets(file_buf, sizeof(file_buf), fp);
                    
                    if (NULL != util_strstr(file_buf, "remote reset finished !"))
                    {
                        vosLog_debug("remote reset no need to send X CT-COM BIND event !");
                    }
                    else
                    {
                        if (SF_FEATURE_SUPPORT_CT_USERINFO)
                        {
                            addInformEventToList(INFORM_EVENT_CT_USERINFO);
                        }
                    }
                    
                    fclose(fp);
                }
            }
            else
            {
                /*X CT-COM BIND must inform*/
                if (SF_FEATURE_SUPPORT_CT_USERINFO)
                {
                    addInformEventToList(INFORM_EVENT_CT_USERINFO);
                }

                if (1 == x_ct_longreset)
                {
                    addInformEventToList(INFORM_EVENT_CT_LONGRESET);
                }
            }
        }
        else
        {
            /*X CT-COM BIND must inform*/
            if ( (1 == x_ct_com_xbind) && SF_FEATURE_SUPPORT_CT_USERINFO)
            {
                addInformEventToList(INFORM_EVENT_CT_USERINFO);
            }

            if (1 == x_ct_longreset)
            {
                addInformEventToList(INFORM_EVENT_CT_LONGRESET);
            }
        }

        /*VALUE_CHANGE event is discarded, so we need to clear all parameters that value changed.*/
        if (1 == valuechange)
        {
            FILE *managementserver_url = fopen("/var/ms_url", "r");
            int msurlchanged = 0;
            if (managementserver_url != NULL)
            {
                fread((void *)&msurlchanged, sizeof(msurlchanged), 1, managementserver_url);
                fclose(managementserver_url);
            }

            if (msurlchanged != 1)
            {
                CMC_phlClearAllParamValueChanges();
            }
        }         
    }

    if (SF_FEATURE_SUPPORT_CARD)
    {
        if (0 == bootstrap && TRUE == del_flag)
        {
            del_flag = FALSE;
            UTIL_DO_SYSTEM_ACTION("rm -f /var/config/remote_resetstatus");
        }
    }

    return paralistSupportCardmon;
}


static int tr69c_buildParamValueChanges(int numParamValueChanges, int fault, tProtoCtx *pc, int *bufsz, int *paramNum)
{
    int ret = fault;
    int i = 0;
    UINT8 *paramValue = NULL;

    if (numParamValueChanges)
    {
        FILE    *fs;
        char line[1024] = {'\0'};
        char *site = NULL;
        char fullPath[128] = {'\0'};
        UBOOL8 change = FALSE;

        fs = fopen(VAR_CONFIG_SETATTR_FILE, "r");
        if (NULL == fs)
        {
            vosLog_debug( "open file setattr faild" );
        }
        else
        {
            fgets(line, sizeof(line), fs);
            site = line;
            i = 0;

            if (*site != '\0')
            {
                while (*site != '\0')
                {
                   memset(fullPath, 0, sizeof(fullPath));
                   
                   while(*site != '\0')
                   {
                      fullPath[i] = *site;
                      site ++;
                      i ++;
                      
                      if (*site == '&')
                      {
                         site ++;
                         i = 0;
                         break;
                      }
                   }

                vosLog_debug("value change fullPath:%s",fullPath);
               
                if (fullPath[0] == 'I')
                {
                    if (ret == VOS_RET_SUCCESS)
                    {
                         CMC_phlIsParamValueChanged(fullPath, &change);
                         if (change)
                         {
                            ret = tr69c_getParamValues(fullPath, NULL, &paramValue, NULL);
                            if (ret == VOS_RET_SUCCESS)
                            {
                               writeGetParamValue((char *)paramValue, pc, bufsz, 1);
                               (*paramNum)++;
                            }

                            memset(fullPath, 0, sizeof(fullPath));
                            VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
                        }
                    }
                    else
                    {
                        vosLog_error("CMC_tr69cFullPathToPathDescriptor error %d", ret);
                    }
                }
               
                if (*site == '\0')/*traversed all the nodes*/
                {
                    break;
                }
            }
        }

        fclose(fs);
        }
    }

    return ret;
}


#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
static void tr69c_buildAlarmNumberInfo(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_TR69C_ALARM_CFG_T alarm_info;

    CMC_tr69cGetAlarm(&alarm_info);

    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz,"<ParameterValueStruct>\n");
    xml_mIndent(pc, bufsz, 7);
    mprintf(pc, bufsz,"<Name>InternetGatewayDevice.DeviceInfo.X_CT-COM_Alarm.AlarmNumber</Name>\n");
    xml_mIndent(pc, bufsz, 7);

    mprintf(pc, bufsz,"<Value %stype=\"%sstring\">", nsXSI, nsXSD);
    mprintf(pc, bufsz,"%s", alarm_info.alarmNumber);
    mprintf(pc, bufsz,"</Value>\n");
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz,"</ParameterValueStruct>\n");

    (*paramNum)++;
}
#endif


static void tr69c_buildAlarmInfo(int paralistMonitorFlag, 
                                 int paralistAlarmFlag,
                                 int paralistCleanAlarmFlag, 
                                 tProtoCtx *pc, int *bufsz, int *paramNum)
{
    int i = 0;

    vosLog_debug("Enter>, paralistMonitorFlag = %d, paralistAlarmFlag = %d, paralistCleanAlarmFlag = %d", 
                          paralistMonitorFlag, paralistAlarmFlag, paralistCleanAlarmFlag);
    
    if (1 == paralistMonitorFlag)
    {
        for(i = 0; i< send_monitor_number; i++)
        {
            tr69c_writePathAndStringValue(monitorSend[i].paralist, monitorSend[i].value, pc, bufsz, paramNum);
        }
    }
    
    if (1 == paralistAlarmFlag)
    {
        #ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
        if (SF_FEATURE_SUPPORT_CARD)
        {
            tr69c_buildAlarmNumberInfo(pc, bufsz, paramNum);
        }
        #endif

        for(i = 0; i< send_alarm_number; i++)
        {
            tr69c_writePathAndStringValue(alarmSend[i].paralist, alarmSend[i].value, pc, bufsz, paramNum);
        }
    }

    if (1 == paralistCleanAlarmFlag)
    {
        for(i = 0; i< send_clean_alarm_number; i++)
        {
            tr69c_writePathAndStringValue(cleanalarmSend[i].paralist, cleanalarmSend[i].value, pc, bufsz, paramNum);
        }
    }      
}


static void tr69c_buildNameChangeInfo(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_WAN_CONN_CFG_T wanCfg;
    CMC_LAN_ETH_IF_LINK_STATUS_T  ethIntfInfo;
    InstanceIdStack  iidStack = EMPTY_INSTANCE_ID_STACK;
    char paramPath[BUFLEN_128] = {0};
    char servMode[BUFLEN_128] = {0};
   // UINT32 vlan = 0;
    char accessType[BUFLEN_32] = {0};
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char wlanSsid[UTIL_WLAN_SSID_MAX_LEN] = {0};
    
    vosLog_debug("Enter>, pc = %p, bufsz = %p, paramNum = %p ", pc, bufsz, paramNum);
        
    memset(&wanCfg, 0, sizeof(wanCfg));
    memset(&ethIntfInfo, 0, sizeof(ethIntfInfo));

    while(VOS_RET_SUCCESS == CMC_wanGetNextIpConn(&wanCfg, &iidStack))
    {
        ret = CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN, &iidStack, paramPath, sizeof(paramPath), "X_CT-COM_ServiceList");
        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_phlGetPathByDesc failed, ret = %d", ret);
            return;
        }
        
        tr69c_wanServModeMask2String(wanCfg.servModeMask, servMode, sizeof(servMode));
        tr69c_writePathAndStringValue(paramPath, servMode, pc, bufsz, paramNum);

        ret = CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN, &iidStack, paramPath, sizeof(paramPath), "X_CT-COM_LanInterface");
        if (!IS_EMPTY_STRING(wanCfg.lanInterface))
        {
            tr69c_writePathAndStringValue(paramPath, wanCfg.lanInterface, pc, bufsz, paramNum);
        }

        memset(&wanCfg, 0, sizeof(wanCfg));
    }
    
    INIT_INSTANCE_ID_STACK(&iidStack);
    while(VOS_RET_SUCCESS == CMC_wanGetNextPppConn(&wanCfg, &iidStack))
    {
        ret = CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "X_CT-COM_ServiceList");
        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_phlGetPathByDesc failed, ret = %d", ret);
            return;
        }
        
        tr69c_wanServModeMask2String(wanCfg.servModeMask, servMode, sizeof(servMode));
        tr69c_writePathAndStringValue(paramPath, servMode, pc, bufsz, paramNum);

        ret = CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "X_CT-COM_LanInterface");
        if (!IS_EMPTY_STRING(wanCfg.lanInterface))
        {
            tr69c_writePathAndStringValue(paramPath, wanCfg.lanInterface, pc, bufsz, paramNum);
        }
        
        memset(&wanCfg, 0, sizeof(wanCfg));
    }
    
    INIT_INSTANCE_ID_STACK(&iidStack);
    while (VOS_RET_SUCCESS == CMC_lanGetNextIfLinkStatus(&ethIntfInfo, &iidStack))
    {        
        ret = CMC_phlGetPathByDesc(EMDMOID_LAN_ETH_INTF, &iidStack, paramPath, sizeof(paramPath), "MACAddress");
        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_phlGetPathByDesc failed, ret = %d", ret);
            return;        
        }
        
        tr69c_writePathAndStringValue(paramPath, ethIntfInfo.macAddr, pc, bufsz, paramNum);
        memset(&ethIntfInfo, 0, sizeof(ethIntfInfo));
    }
    
    INIT_INSTANCE_ID_STACK(&iidStack);
    while (VOS_RET_SUCCESS == CMC_wlanGetNextSSID(wlanSsid, sizeof(wlanSsid), &iidStack))
    {
        ret = CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN, &iidStack, paramPath, sizeof(paramPath), "SSID");
        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_phlGetPathByDesc failed, ret = %d", ret);
            return;
        }
        
        tr69c_writePathAndStringValue(paramPath, wlanSsid, pc, bufsz, paramNum);
    }
    
    TR69C_buildVlanCustom(pc, bufsz, paramNum);

    INIT_INSTANCE_ID_STACK(&iidStack);
    while(VOS_RET_SUCCESS == CMC_wanGetAccessType(accessType, &iidStack, sizeof(accessType)))
    {
        ret = CMC_phlGetPathByDesc(EMDMOID_WAN_COMMON_INTF_CFG, &iidStack, paramPath, sizeof(paramPath), "WANAccessType");
        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_phlGetPathByDesc failed, ret = %d", ret);
            return;
        }
        
        tr69c_writePathAndStringValue(paramPath, accessType, pc, bufsz, paramNum);
    }
}


static void tr69c_buildMacInfo(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_WAN_CONN_CFG_T wanCfg;
    InstanceIdStack  iidStack = EMPTY_INSTANCE_ID_STACK;
    char paramPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};
    int found = 0;

    memset(&wanCfg, 0, sizeof(wanCfg));
    while(!found && (VOS_RET_SUCCESS == CMC_wanGetNextIpConn(&wanCfg, &iidStack)))
    {
        if (wanCfg.servModeMask & SERV_MODE_TR069)
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN, &iidStack, paramPath, sizeof(paramPath), "MACAddress");

            tr69c_writePathAndStringValue(paramPath, wanCfg.macAddr, pc, bufsz, paramNum);
            found = 1;
            break;
        }

        memset(&wanCfg, 0, sizeof(wanCfg));
    }

    INIT_INSTANCE_ID_STACK(&iidStack);
    while(!found && (VOS_RET_SUCCESS == CMC_wanGetNextPppConn(&wanCfg, &iidStack)))
    {
        if (wanCfg.servModeMask & SERV_MODE_TR069)
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "MACAddress");

            tr69c_writePathAndStringValue(paramPath, wanCfg.macAddr, pc, bufsz, paramNum);
            found = 1;
            break;
        }

        memset(&wanCfg, 0, sizeof(wanCfg));
    }
}


static void tr69c_buildCpuAndMemInfo(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_TR69C_CPU_OCCUPY_INFO_T cpuAndMemInfo;
    char paramPath[BUFLEN_128] = {0};

    tr69c_GetCpuOccupyInfo(cpuAndMemInfo.cpuOccupy, sizeof(cpuAndMemInfo.cpuOccupy));
    tr69c_GetMemOccupyInfo(cpuAndMemInfo.memOccupy, sizeof(cpuAndMemInfo.memOccupy));
    CMC_tr69cSetCpuMemOccupyInfo(cpuAndMemInfo);

    UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CT-COM_UserInfo.UserCpu", sizeof(paramPath));
    tr69c_writePathAndStringValue(paramPath, cpuAndMemInfo.cpuOccupy, pc, bufsz, paramNum);

    UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CT-COM_UserInfo.UserMemory", sizeof(paramPath));
    tr69c_writePathAndStringValue(paramPath, cpuAndMemInfo.memOccupy, pc, bufsz, paramNum);
}


void tr69c_buildUserInfo(int paralistUserInfoFlag, 
                         int paralistSupportCardmon,
                         int paralistLoidChange,
                         tProtoCtx *pc, int *bufsz,
                         int *paramNum)
{
    CMC_TR69C_DEV_REG_CFG_T userInfo;
    VOS_RET_E userRet = VOS_RET_SUCCESS;
    char paramValue[BUFLEN_64] = {0};
    char paramPath[BUFLEN_128] = {0};
    UINT8 *value = NULL;
    UINT32 num = 0;
    
    vosLog_debug("Enter>, paralistUserInfoFlag = %d, paralistLoidChange = %d", paralistUserInfoFlag, paralistLoidChange);

    memset(&userInfo, 0, sizeof(userInfo));
    userRet = CMC_tr69cGetUserInfo(&userInfo);
    if (VOS_RET_SUCCESS != userRet)
    {
        vosLog_error("CMC_tr69cGetUserInfo failed, userRet = %d", userRet);
    }

    if (SF_FEATURE_ISP_CU)
    {
        UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CU_UserInfo.UserName", sizeof(paramPath));
    }
    else
    {
        if (!SF_FEATURE_LOCATION_GUANGDONG)
        {
            UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CT-COM_UserInfo.UserName", sizeof(paramPath));
        }
        else
        {
            UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CT-COM_UserInfo.UserId", sizeof(paramPath));
        }
    }
    
    if (!IS_EMPTY_STRING(userInfo.username))
    {
        UTIL_STRNCPY(paramValue, userInfo.username, sizeof(paramValue));
    }
    
    if (SF_FEATURE_LOCATION_GUANGDONG && SF_FEATURE_SUPPORT_CT)
    {
        if (!IS_EMPTY_STRING(userInfo.password))
        {
            UTIL_STRNCPY(paramValue, userInfo.password, sizeof(paramValue));
        }
    }

    if ((1 == paralistUserInfoFlag) 
      || SF_OR_IF(SF_FEATURE_SUPPORT_CARD)
        (1 == paralistSupportCardmon)
        SF_OR_ENDIF) 
    {
        tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);

        if (SF_FEATURE_ISP_CU)
        {
            UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CU_UserInfo.UserId", sizeof(paramPath));
        }
        else
        {
            if (!SF_FEATURE_LOCATION_GUANGDONG)
            {
                UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CT-COM_UserInfo.UserId", sizeof(paramPath));
            }
            else
            {
                UTIL_STRNCPY(paramPath, "InternetGatewayDevice.X_CT-COM_UserInfo.UserName", sizeof(paramPath));
            }
        }
        
        memset(paramValue, 0, sizeof(paramValue));
        if (!(SF_FEATURE_LOCATION_GUANGDONG && SF_FEATURE_SUPPORT_CT))
        {
            if (!IS_EMPTY_STRING(userInfo.password))
            {
                UTIL_STRNCPY(paramValue, userInfo.password, sizeof(paramValue));
            }
        }
        
        tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
    }
    else if (1 == paralistLoidChange)
    {
        if (!IS_EMPTY_STRING(userInfo.username))
        {
            tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
        }
    }
    else
    {
        tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
    }

    if (SF_FEATURE_LOCATION_FUJIAN)
    {
        if ((CMC_TR69C_DEV_REG_SUCCESS == userInfo.status) 
         || (CMC_TR69C_DEV_REG_SEND_BUSINESS_SUCCESS == userInfo.result))
        {
            UTIL_STRNCPY(paramPath, "InternetGatewayDevice.DeviceInfo.X_CT-COM_RegStatistics.", sizeof(paramPath));
            tr69c_getParamValues(paramPath, NULL, &value, &num);
            writeGetParamValue((char *)value, pc, bufsz, num); 
            *paramNum += num;
        }
    }
}


void tr69c_buildPppAccount(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_WAN_CONN_CFG_T wanCfg;
    InstanceIdStack  iidStack = EMPTY_INSTANCE_ID_STACK;
    char paramPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};

    memset(&wanCfg, 0, sizeof(wanCfg));
    
    while(VOS_RET_SUCCESS == CMC_wanGetNextPppConn(&wanCfg, &iidStack))
    {
        if (SF_AND_IF(SF_FEATURE_SUPPORT_RTP_ROX)
            ((wanCfg.servModeMask & SERV_MODE_INTERNET) &&(VOS_WAN_TYPE_BRIDGE != wanCfg.connType))
            || (1 == wanCfg.enablePppProxy)
            SF_AND_ELSE(SF_FEATURE_SUPPORT_RTP_ROX)
            (wanCfg.servModeMask & SERV_MODE_INTERNET) 
            && (wanCfg.connType != VOS_WAN_TYPE_BRIDGE)
            SF_AND_ENDIF)
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "Username");

            vosLog_debug("paramPath = %s", paramPath);
            tr69c_writePathAndStringValue(paramPath, wanCfg.pppUsername, pc, bufsz, paramNum);
        }
    }
}


void tr69c_buildCardInfo(int paralistCardStatusFlag,
                         int paralistSupportCardmon, 
                         tProtoCtx *pc, int *bufsz)
{
    CMC_TR69C_SIM_CARD_CFG_T pCardManager;
    VOS_RET_E ret= VOS_RET_SUCCESS;

    memset (&pCardManager, 0, sizeof(pCardManager));

    ret = CMC_tr69cGetCardManager(&pCardManager);
    if (1 == paralistCardStatusFlag)
    {
        printf("func=%s***line=%d\n",__FUNCTION__,__LINE__);
        if (ret == VOS_RET_SUCCESS)
        {
            printf("func=%s***line=%d\n",__FUNCTION__,__LINE__);
            xml_mIndent(pc, bufsz, 6);
            mprintf(pc, bufsz,"<ParameterValueStruct>\n");
            xml_mIndent(pc, bufsz, 7);
            mprintf(pc, bufsz,"<Name>InternetGatewayDevice.DeviceInfo.X_CT-COM_Cardmanage.status</Name>\n");
            xml_mIndent(pc, bufsz, 7);

            mprintf(pc, bufsz,"<Value %stype=\"%sstring\">", nsXSI, nsXSD);
            mprintf(pc, bufsz,"%d", pCardManager.status);
            mprintf(pc, bufsz,"</Value>\n");
            xml_mIndent(pc, bufsz, 6);
            mprintf(pc, bufsz,"</ParameterValueStruct>\n");
        }
        
        printf("func=%s***line=%d\n",__FUNCTION__,__LINE__);
    }

    if (1 == paralistSupportCardmon)
    {
        if (VOS_RET_SUCCESS == ret)
        {
            xml_mIndent(pc, bufsz, 6);
            mprintf(pc, bufsz,"<ParameterValueStruct>\n");
            xml_mIndent(pc, bufsz, 7);
            mprintf(pc, bufsz,"<Name>InternetGatewayDevice.DeviceInfo.X_CT-COM_Cardmanage.CardNo</Name>\n");
            xml_mIndent(pc, bufsz, 7);

            mprintf(pc, bufsz,"<Value %stype=\"%sstring\">",nsXSI, nsXSD);
            mprintf(pc, bufsz,"%s", pCardManager.cardNum);
            mprintf(pc, bufsz,"</Value>\n");
            xml_mIndent(pc, bufsz, 6);
            mprintf(pc, bufsz,"</ParameterValueStruct>\n");
        }
    }
}


static void tr69c_buildWanAndWlanStats(tProtoCtx *pc, int *bufsz, int *paramNum, int informWanAndWlanStat)
{
    CMC_WAN_CON_STATUS_T wanStat; 
    CMC_WAN_CONN_CFG_T wanCfg;
    char paramPath[BUFLEN_128] = {0};
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
 
    memset(&wanStat, 0, sizeof(wanStat));
    
    if (informWanAndWlanStat)
    {
        TR69C_buildWlanStatsCustom(pc, bufsz, paramNum);
    
        while(VOS_RET_SUCCESS == CMC_wanGetNextIpStatus(&wanStat, &iidStack))
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetBytesSent");
            TR69C_writePathAndUintValue(paramPath, wanStat.txByte, pc, bufsz, paramNum);
            
            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetBytesReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxByte, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetPacketsSent");
            TR69C_writePathAndUintValue(paramPath, wanStat.txPacket, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetPacketsReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxPacket, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetErrorsReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxErr, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetDiscardPacketsReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxDrop, pc, bufsz, paramNum);

            memset(&wanStat, 0, sizeof(wanStat));
        }
    
        INIT_INSTANCE_ID_STACK(&iidStack);
        while(VOS_RET_SUCCESS == CMC_wanGetNextPppStatus(&wanStat, &iidStack))
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetBytesSent");
            TR69C_writePathAndUintValue(paramPath, wanStat.txByte, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetBytesReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxByte, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetPacketsSent");
            TR69C_writePathAndUintValue(paramPath, wanStat.txPacket, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetPacketsReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxPacket, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetErrorsReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxErr, pc, bufsz, paramNum);

            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN_STATS, &iidStack, paramPath, sizeof(paramPath), "ethernetDiscardPacketsReceived");
            TR69C_writePathAndUintValue(paramPath, wanStat.rxDrop, pc, bufsz, paramNum);

            memset(&wanStat, 0, sizeof(wanStat));
        }
        
        INIT_INSTANCE_ID_STACK(&iidStack);
        while(VOS_RET_SUCCESS == CMC_wanGetNextPppConn(&wanCfg, &iidStack))
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "uptime");
            TR69C_writePathAndUintValue(paramPath, wanCfg.upTime, pc, bufsz, paramNum);
            
            memset(&wanCfg, 0, sizeof(wanCfg));
        }
        
        
    }
}
#if 0
static void tr69c_buildFuJianInform(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_WAN_CONN_CFG_T wanCfg;
    InstanceIdStack  iidStack = EMPTY_INSTANCE_ID_STACK;
    char paramPath[BUFLEN_128] = {0};
    char *paramValue = NULL;
        
    vosLog_debug("Enter>, pc = %p, bufsz = %p, paramNum = %p", pc, bufsz, paramNum);
    
    memset(&wanCfg, 0, sizeof(wanCfg));
    
    UTIL_STRNCPY(paramPath, "InternetGatewayDevice.DeviceInfo.X_CT-COM_MacAddress", sizeof(paramPath));
    tr69c_getParamValue(paramPath, &paramValue);
    tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);

    UTIL_STRNCPY(paramPath, "InternetGatewayDevice.DeviceInfo.X_CT-COM_VoipWanStatus.Status", sizeof(paramPath));
    tr69c_getParamValue(paramPath, &paramValue);
    tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
    
    while(VOS_RET_SUCCESS == CMC_wanGetNextIpConn(&wanCfg, &iidStack))
    {
        if (wanCfg.servModeMask & SERV_MODE_INTERNET)
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN, &iidStack, paramPath, sizeof(paramPath), "connectionStatus");
            tr69c_writePathAndStringValue(paramPath, wanCfg.connectStatus, pc, bufsz, paramNum);
        }

        if (!(wanCfg.servModeMask & SERV_MODE_TR069))        
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN, &iidStack, paramPath, sizeof(paramPath), "ExternalIPAddress");
            tr69c_writePathAndStringValue(paramPath, wanCfg.ipAddr, pc, bufsz, paramNum);
        }
        
        CMC_phlGetPathByDesc(EMDMOID_WAN_IP_CONN, &iidStack, paramPath, sizeof(paramPath), "connectionType");
        tr69c_getParamValue(paramPath, &paramValue);
        tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
        
        memset(&wanCfg, 0, sizeof(wanCfg));
    }
    
    memset(&iidStack, 0, sizeof(iidStack));
    while(VOS_RET_SUCCESS == CMC_wanGetNextPppConn(&wanCfg, &iidStack))
    {
        if (wanCfg.servModeMask & SERV_MODE_INTERNET)
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "connectionStatus");
            tr69c_writePathAndStringValue(paramPath, wanCfg.connectStatus, pc, bufsz, paramNum);
        }

        if (!(wanCfg.servModeMask & SERV_MODE_TR069))        
        {
            CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "ExternalIPAddress");
            tr69c_writePathAndStringValue(paramPath, wanCfg.ipAddr, pc, bufsz, paramNum);
        }
        
        CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "connectionType");
        tr69c_getParamValue(paramPath, &paramValue);
        tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);

        CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "lastConnectionError");
        tr69c_getParamValue(paramPath, &paramValue);
        tr69c_writePathAndStringValue(paramPath, paramValue, pc, bufsz, paramNum);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
        
        CMC_phlGetPathByDesc(EMDMOID_WAN_PPP_CONN, &iidStack, paramPath, sizeof(paramPath), "uptime");
        TR69C_writePathAndUintValue(paramPath, wanCfg.upTime, pc, bufsz, paramNum);
            
        memset(&wanCfg, 0, sizeof(wanCfg));
    }    
}
#endif

void tr69c_buildIptvStbMacInfo(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_MCAST_IPTV_STB_MAC_T iptvStbMac;

    tr69c_writePathAndStringValue("InternetGatewayDevice.DeviceInfo.SerialNumber", acsState.serialNumber, pc, bufsz, paramNum);

    memset(&iptvStbMac, 0, sizeof(iptvStbMac));
    if (VOS_RET_SUCCESS == CMC_igmpGetIptvStbMac(&iptvStbMac))
    {
        tr69c_writePathAndStringValue("InternetGatewayDevice.Services.X_CT-COM_IPTV.STBMAC", iptvStbMac.STBMAC, pc, bufsz, paramNum);
    }
}

void tr69c_buildWlanTotalAssociations(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    CMC_WLAN_INFO_T wlanCfg;
    InstanceIdStack  iidStack = EMPTY_INSTANCE_ID_STACK;
    char paramPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};

    memset(&wlanCfg, 0, sizeof(wlanCfg));
    
    while(VOS_RET_SUCCESS == CMC_lanGetNextWlanInfo(&wlanCfg, &iidStack))
    {
        //if (wlanCfg.enable && wlanCfg.showToACS)
        if (wlanCfg.enable)
        {
            CMC_phlGetPathByDesc(EMDMOID_LAN_WLAN, &iidStack, paramPath, sizeof(paramPath), "TotalAssociations");
            TR69C_writePathAndUintValue(paramPath, wlanCfg.totalAssociations, pc, bufsz, paramNum);
        }
    }
}


void tr69c_buildLanEthAvailableIfnameInfo(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    UBOOL8 findFile = FALSE;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    CMC_LAN_ETH_IF_LINK_STATUS_T ethIntfInfo;
    char paramPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};
    FILE *fp = NULL;
    char ifName[BUFLEN_8] = {0};

    if (!access("/tmp/lanEthDisavailable", F_OK))
    {
        findFile = TRUE;
        fp = fopen("/tmp/lanEthDisavailable", "r");
    }
    else
    {
        if (!access("/tmp/lanEthAvailable", F_OK))
        {
            findFile = TRUE;
            fp = fopen("/tmp/lanEthAvailable", "r");
        }
    }

    if (NULL != fp)
    {
        fgets(ifName, sizeof(ifName), fp);
        ifName[4] = '\0';
    }

    memset(&ethIntfInfo, 0, sizeof(ethIntfInfo));
    if (findFile && !IS_EMPTY_STRING(ifName))
    {
        while(VOS_RET_SUCCESS == CMC_lanGetNextIfLinkStatus(&ethIntfInfo, &iidStack))
        {
            if (!util_strcmp(ifName, ethIntfInfo.ifName))
            {
                CMC_phlGetPathByDesc(EMDMOID_LAN_ETH_INTF, &iidStack, paramPath, sizeof(paramPath), "X_BROADCOM_COM_IfName");
                tr69c_writePathAndStringValue(paramPath, ethIntfInfo.ifName, pc, bufsz, paramNum);
                break;
            }
        }
    }

    fclose(fp);
}


static void tr69c_buildAlarmNumberStruct(tProtoCtx *pc, int *bufsz, int *paramNum)
{
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz, "<ParameterValueStruct>\n");
    xml_mIndent(pc, bufsz, 7);
    mprintf(pc, bufsz, "<Name>%s</Name>\n", "InternetGatewayDevice.DeviceInfo.X_CT-COM_Alarm.AlarmNumber");
    xml_mIndent(pc, bufsz, 7);

    mprintf(pc, bufsz, "<Value %stype=\"%sstring\">", nsXSI, nsXSD);
    mprintf(pc, bufsz, "%s", alarmNumber);
    mprintf(pc, bufsz, "</Value>\n");
    xml_mIndent(pc, bufsz, 6);
    mprintf(pc, bufsz,"</ParameterValueStruct>\n");

    (*paramNum)++;

    return;
}


/** Build an Inform msg*/
void buildInform(RPCAction *a, InformEvList *infEvent)
{
    int paramNum = 0;
    tProtoCtx *pc = NULL;
    char dateTimeBuf[BUFLEN_64] = {0};
    int numParamValueChanges = 0;
    int passiveNotification = 1;
    int bootstrap = 0, boot = 0, valuechange = 0;
    int x_ct_com_xbind = 0;
    int informParamCount = 0;
    char **informParam = NULL;

    vosLog_debug("Enter>, connReqIpAddrFullPath = %s", acsState.connReqIpAddrFullPath);

    tr69c_initInformEventStr();

    if(SF_FEATURE_ISP_CU)
    {
        tr69c_initInformEventStrforCU();
    }
   
    acsState.fault = VOS_RET_SUCCESS; /* init to no fault */
    a->informID    = VOS_STRDUP(itoa(rand()));

    tr69c_initAllocBuf();
	
    do
    {
        int  i = 0;
        int  bufsz = 0;
        UINT8 *paramValue = NULL;
        UINT32 changeNum = 0;
        int paralistMonitorFlag = 0;
        int paralistAlarmFlag = 0;
        int paralistCleanAlarmFlag = 0;
        int paralistUserInfoFlag = 0;
        int paralistCardStatusFlag = 0;
        int paralistReadCardFlag = 0;
        int paralistSupportCardmon = 0;
        int x_cu_admin_passwd = 0;
        int paralistLoidChange = 0;
        int paralistMacInfoFlag = 0;
        int needInformNameChange = 0;
        int informStbMacFlag = 0;
        int informWlanStat = 0;
        int x_ct_longreset = 0;
        int firstBridgeUsernameFlag = 0;
        
        openEnvWithHeader(a->informID, pc, &bufsz);
        openBody(pc, &bufsz);
        xml_mIndent(pc, &bufsz, 3);
        mprintf(pc, &bufsz, "<%sInform>\n", nsCWMP);
        xml_mIndent(pc, &bufsz, 4);

        /* build DeviceId List */
        mprintf(pc, &bufsz, "<DeviceId>\n");
        for (i = 0; i < (SINT32) NUMREQINFORMDEVIDS && acsState.fault == VOS_RET_SUCCESS; i++)
        {
            acsState.fault = tr69c_buildDeviceInfo(i, pc, &bufsz);
        }  

        if (acsState.fault != VOS_RET_SUCCESS)
        {
            vosLog_error("acsState.fault = %d, quit", acsState.fault);
            break;
        }
        
        xml_mIndent(pc, &bufsz, 4);
        mprintf(pc, &bufsz, "</DeviceId>\n");
        xml_mIndent(pc, &bufsz, 4);
        /*build DeviceId List End*/
    
        /*If we get an BOOTSTRAP event, we need to discard all other event, except BOOT*/
        for (i = 0; i < infEvent->informEvCnt; i++)
        {         
            if (infEvent->informEvList[i] == INFORM_EVENT_BOOTSTRAP)
            {
                bootstrap = 1;
            }
            else if (infEvent->informEvList[i] == INFORM_EVENT_BOOT)
            {
                boot = 1;
            }
            else if (infEvent->informEvList[i] == INFORM_EVENT_VALUE_CHANGE)
            {
                valuechange = 1;
            }
            else if (infEvent->informEvList[i] == INFORM_EVENT_CT_USERINFO && SF_FEATURE_SUPPORT_CT_USERINFO)
            {
                x_ct_com_xbind = 1;
            }
            else if (SF_FEATURE_ISP_CU)
            {
                if (infEvent->informEvList[i] == INFORM_ADMIN_PASSWD_CHANGED)
                {
                    x_cu_admin_passwd = 1;
                }
            }

            if (infEvent->informEvList[i] == INFORM_EVENT_CT_LONGRESET)
            {
                x_ct_longreset = 1;
            }
        }

        /*hanlde boot event*/
        paralistSupportCardmon = tr69c_buildBootInfo(bootstrap, x_ct_com_xbind, valuechange, x_ct_longreset);

        if (SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU || SF_FEATURE_LOCATION_NEIMENGGU)
        {
            if (boot || bootstrap)
            {
                addInformEventToList(INFORM_EVENT_LOID_CHANGE);
                paralistMacInfoFlag = 1;
                needInformNameChange = 1;
            }
        }
        
        CMC_phlGetNumOfParamValueChanged(&changeNum);
        
        vosLog_debug("numParamValueChanges = %d", numParamValueChanges);
        numParamValueChanges = (int)changeNum;
        if (numParamValueChanges > 0)
        {
            /* if there is no value_change event in the event list, add it.  It's passive notification */
            for (i = 0; i < infEvent->informEvCnt; i++)
            {         
                if (infEvent->informEvList[i] == INFORM_EVENT_VALUE_CHANGE)
                {
                    /* nothing needs to be done */
                    passiveNotification = 0;
                    break;
                }
            }
        }

    /* build Event List */
#ifdef SUPPRESS_SOAP_ARRAYTYPE
        mprintf(pc, &bufsz, "<Event>\n");
#else
        mprintf(pc, &bufsz, "<Event %sarrayType=\"%sEventStruct[%d]\">\n",
        nsSOAP_ENC, nsCWMP, infEvent->informEvCnt);
#endif

        for (i = 0; i < infEvent->informEvCnt; i++)
        {
            char  *ck = NULL;

            xml_mIndent(pc, &bufsz, 5);
            mprintf(pc, &bufsz, "<EventStruct>\n");
            xml_mIndent(pc, &bufsz, 6);

            if (SF_FEATURE_ISP_CU && (infEvent->informEvList[i] == INFORM_ADMIN_PASSWD_CHANGED))
            {
                mprintf(pc, &bufsz, "<EventCode>X_CU_ADMINPASSWORDCHANGE</EventCode>\n");
            }
            else
            {
                mprintf(pc, &bufsz, "<EventCode>%s</EventCode>\n",
                informEventStr[infEvent->informEvList[i]]);
            }

            xml_mIndent(pc, &bufsz, 6);

            if (infEvent->informEvList[i]==INFORM_EVENT_REBOOT_METHOD)
            {
                ck = acsState.rebootCommandKey;
            }
            else if (infEvent->informEvList[i]==INFORM_EVENT_DOWNLOAD_METHOD)
            {
                ck = acsState.downloadCommandKey;
            }
            else if (infEvent->informEvList[i]== INFORM_EVENT_UPLOAD_METHOD)
            { 
                ck = acsState.downloadCommandKey;
            }
            else if (infEvent->informEvList[i]==INFORM_EVENT_SCHEDULE_METHOD)
            { 
                ck = acsState.scheduleInformCommandKey;
            }
            else if (INFORM_EVENT_CT_STB_BIND == infEvent->informEvList[i])
            {
                informStbMacFlag = 1;
            }
            else if (INFORM_EVENT_LOID_CHANGE == infEvent->informEvList[i])
            {
                paralistLoidChange = 1;
                beginTime = getsysRunTime();
                vosLog_debug("device set loid and start timer! beginTime = %d", beginTime);
            }
            else if (INFORM_EVENT_PERIODIC == infEvent->informEvList[i])
            {
                paralistMacInfoFlag = 1;
                informWlanStat = 1;
            }
            else if (INFORM_EVENT_NAME_CHANGE == infEvent->informEvList[i])
            {
                needInformNameChange = 1;
            }
            else if (SF_FEATURE_SUPPORT_TR69C_MONITOR && (INFORM_EVENT_CT_MONITOR == infEvent->informEvList[i]))
            { 
                paralistMonitorFlag = 1;
            }
            
            if (SF_FEATURE_SUPPORT_TR69C_ALARM)
            {
                if (INFORM_EVENT_CT_ALARM == infEvent->informEvList[i])
                {   
                    paralistAlarmFlag = 1;
                }
                else if (INFORM_EVENT_CLEAR_CT_ALARM == infEvent->informEvList[i])
                {    
                    paralistCleanAlarmFlag = 1;
                }
            }

        if (SF_FEATURE_SUPPORT_PPPOE_SNOOPING && (INFORM_BRIDGE_USERNAME == infEvent->informEvList[i]))
        {
            firstBridgeUsernameFlag = 1;
            paramNum ++;
        }
            if (SF_FEATURE_SUPPORT_CT_USERINFO)
            {
                 /* register infomation, add userId and Password*/
                 if (INFORM_EVENT_CT_USERINFO == infEvent->informEvList[i])
                 {
                    paralistUserInfoFlag = 1;
                    if (SF_FEATURE_SUPPORT_CARD)
                    {
                        UINT32 cardtype = 0;
                        #ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
                        CMC_cardGetCardType(&cardtype);
                        #endif
                        if (1 == cardtype)
                        {
                            paralistSupportCardmon = 1;
                            paramNum += 3;
                        }
                        else
                        {
                            paramNum += 2;
                        }
                    }
                 }
            }
        
            if (SF_FEATURE_SUPPORT_CARD)
            {
                UINT32 cardtype = 0;
                #ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
                CMC_cardGetCardType(&cardtype);
                #endif

                if (INFORM_EVENT_CT_CARDWRITE == infEvent->informEvList[i])
                {
                   paralistCardStatusFlag = 1;
                   paramNum ++;
                }
                
                 if (INFORM_EVENT_CT_CARDNOTIFY == infEvent->informEvList[i])
                 {
                    paralistReadCardFlag = 1;
                    paramNum++;
                 }

                 if (1 == cardtype)
                 {
                    if (INFORM_EVENT_BOOT == infEvent->informEvList[i])
                    {
                       paralistSupportCardmon = 1;
                       paramNum++;
                    }
                 }
            }

#ifdef SUPPRESS_EMPTY_PARAM
             if (empty(ck))
             {
                mprintf(pc, &bufsz, "<CommandKey>empty</CommandKey>\n");
             }
             else
             {
                mprintf(pc, &bufsz, "<CommandKey>%s</CommandKey>\n", ck);
             }
#else
             mprintf(pc, &bufsz, "<CommandKey>%s</CommandKey>\n", ck? ck: "");
#endif
             xml_mIndent(pc, &bufsz, 5);
             mprintf(pc, &bufsz,"</EventStruct>\n");

        }  /* for (i = 0; i < infEvent->informEvCnt; i++) */

        /* insert value change event if this passive notification */
        if (numParamValueChanges && passiveNotification)
        {
            xml_mIndent(pc, &bufsz, 5);
            mprintf(pc, &bufsz, "<EventStruct>\n");
            xml_mIndent(pc, &bufsz, 6);
            mprintf(pc, &bufsz, "<EventCode>%s</EventCode>\n",
            informEventStr[INFORM_EVENT_VALUE_CHANGE]);
            xml_mIndent(pc, &bufsz, 6);
#ifdef SUPPRESS_EMPTY_PARAM
            mprintf(pc, &bufsz, "<CommandKey>empty</CommandKey>\n");
#else
            mprintf(pc, &bufsz, "<CommandKey>%s</CommandKey>\n", "");
#endif
            xml_mIndent(pc, &bufsz, 5);
            mprintf(pc, &bufsz,"</EventStruct>\n");
        }

        xml_mIndent(pc, &bufsz, 4);
        mprintf(pc, &bufsz, "</Event>\n");
        xml_mIndent(pc, &bufsz, 4);
        /* build Event List End*/

        mprintf(pc, &bufsz, "<MaxEnvelopes>1</MaxEnvelopes>\n");
        xml_mIndent(pc, &bufsz, 4);
        utilTms_getXSIDateTime(0, dateTimeBuf, sizeof(dateTimeBuf));
        mprintf(pc, &bufsz, "<CurrentTime>%s</CurrentTime>\n", dateTimeBuf);
        xml_mIndent(pc, &bufsz, 4);
        mprintf(pc, &bufsz, "<RetryCount>%d</RetryCount>\n", acsState.retryCount);
        xml_mIndent(pc, &bufsz, 4);

      /* build Parameter List */
#ifdef SUPPRESS_SOAP_ARRAYTYPE
        mprintf(pc, &bufsz, "<ParameterList>\n");
#else
        /* In the first loop paramNum=0.  In the second loop, paramNum will have
        * the actual parameter count.
        */
        mprintf(pc, &bufsz,
                  "<ParameterList %sarrayType=\"%sParameterValueStruct[%04d]\">\n",
                  nsSOAP_ENC, nsCWMP, paramNum);
#endif
        paramNum = 0;  /* reset paramNum */

        if (SF_FEATURE_LOCATION_YUNNAN || SF_FEATURE_LOCATION_GUANGDONG)
        {
            informParamCount = sizeof(informParametersYunnan) / sizeof(informParametersYunnan[0]);
            informParam = &informParametersYunnan[0];
        }
        else if (SF_FEATURE_LOCATION_FUJIAN)
        {
            informParamCount = sizeof(informParametersFujian) / sizeof(informParametersFujian[0]);
            informParam = &informParametersFujian[0];
        }
        else
        {
            informParamCount = sizeof(informParameters) / sizeof(informParameters[0]);
            informParam = &informParameters[0];
        }

        /* the External IP address parameter name was set in updateTr69cCfg */
        informParam[informParamCount-1] = VOS_STRDUP(acsState.connReqIpAddrFullPath);
        vosLog_debug("----informParam[i]:%s--acsState.connReqIpAddrFullPath:%s----",informParam[i],acsState.connReqIpAddrFullPath);

        for (i = 0; i < (SINT32) informParamCount && acsState.fault == VOS_RET_SUCCESS && informParam[i]; i++)
        {
            if (acsState.fault == VOS_RET_SUCCESS)
            {
                acsState.fault = tr69c_getParamValues(informParam[i], NULL, &paramValue, NULL);
                if (acsState.fault == VOS_RET_SUCCESS)
                {
                    writeGetParamValue((char *)paramValue, pc, &bufsz, 1);
                    paramNum++;
                }

                VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
            }
            else
            {
                vosLog_debug("fault on informParameters[%d]=%s", i, informParam[i]);
            }

        }  /* for (i = 0; i < informParamCount; i++) */
        
        /* free the external IP address parameter name */
        VOS_MEM_FREE_BUF_AND_NULL_PTR(informParam[informParamCount-1]);

        /*register infomation, add userID and Password*/
        tr69c_buildUserInfo(paralistUserInfoFlag, paralistSupportCardmon, paralistLoidChange, pc, &bufsz, &paramNum);

        if ((SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU) && paralistMacInfoFlag)
        {
            tr69c_buildMacInfo(pc, &bufsz, &paramNum);
        }

        if ((SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU) && needInformNameChange)
        {
            tr69c_buildNameChangeInfo(pc, &bufsz, &paramNum);
        }
        
#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
        if (SF_FEATURE_SUPPORT_CARD)
        {
            tr69c_buildCardInfo(paralistCardStatusFlag, 0, pc, &bufsz);
        }
#endif

        /*inform ppp account*/
        if (!SF_FEATURE_LOCATION_YUNNAN)
        {
            tr69c_buildPppAccount(pc, &bufsz, &paramNum);
        }

        if ((SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU || SF_FEATURE_LOCATION_JIANGXI) && informStbMacFlag)
        {
            tr69c_buildIptvStbMacInfo(pc, &bufsz, &paramNum);
        }

        if (SF_FEATURE_LOCATION_FUJIAN)
        {
            //tr69c_buildFuJianInform(pc, &bufsz, &paramNum);
            tr69c_buildWanAndWlanStats(pc, &bufsz, &paramNum, informWlanStat);
            tr69c_buildMacInfo(pc, &bufsz, &paramNum);

            if ((1 == valuechange) && (TRUE == g_processAbnormal))
            {
                tr69c_buildCpuAndMemInfo(pc, &bufsz, &paramNum);
            }


            if (g_totalAssociationsEnable)
            {
                tr69c_buildWlanTotalAssociations(pc, &bufsz, &paramNum);
            }

        }
        
        if (acsState.fault != VOS_RET_SUCCESS)
        {
            vosLog_error("failed to build inform, acsState.fault=%d", acsState.fault);
            break;   /* quit */
        }

        /*traverse all the nodes which has been concerned and parameters has changed*/
        acsState.fault =  tr69c_buildParamValueChanges(numParamValueChanges, 
                          acsState.fault, 
                          pc, &bufsz, 
                          &paramNum);

        if (acsState.fault != VOS_RET_SUCCESS)
        {
            vosLog_error("acsState.fault = %d, quit", acsState.fault);
            break;   /* quit */
        }

        /*handle alarm info*/
        tr69c_buildAlarmInfo(paralistMonitorFlag, 
                            paralistAlarmFlag,
                            paralistCleanAlarmFlag, 
                            pc, &bufsz, &paramNum);

        if (SF_FEATURE_SUPPORT_PLUGIN && (paralistAlarmFlag || paralistCleanAlarmFlag))
        {
            if (SF_FEATURE_LOCATION_SHANGHAI)
            {
                if (boot)
                {
                    UTIL_STRNCPY(alarmNumber, "104001", sizeof(alarmNumber));
                }
            }
            tr69c_buildAlarmNumberStruct(pc, &bufsz, &paramNum);
            if (!util_strcmp(alarmNumber, "104006"))
            {
                tr69c_buildLanEthAvailableIfnameInfo(pc, &bufsz, &paramNum);
            }
        }

#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
        if (SF_FEATURE_SUPPORT_CARD)
        {
            tr69c_buildCardInfo(0, paralistSupportCardmon, pc, &bufsz);
        }
#endif
    if (SF_FEATURE_SUPPORT_PPPOE_SNOOPING)
    {
        if (firstBridgeUsernameFlag == 1 && bridgeUserName[0] != 0 )
        {
            xml_mIndent(pc,&bufsz, 6);
            mprintf(pc,&bufsz,"<ParameterValueStruct>\n");
            xml_mIndent(pc,&bufsz, 7);
            mprintf(pc,&bufsz,"<Name>InternetGatewayDevice.X_CT-COM_UserInfo.UserName</Name>\n");
            xml_mIndent(pc,&bufsz, 7);

            mprintf(pc,&bufsz,"<Value %stype=\"%sstring\">",nsXSI, nsXSD);
            mprintf(pc,&bufsz,"%s", bridgeUserName);
            mprintf(pc,&bufsz,"</Value>\n");
            xml_mIndent(pc,&bufsz, 6);
            mprintf(pc,&bufsz,"</ParameterValueStruct>\n");
        }
    }
        /* notify changes are released after ACS response */

        xml_mIndent(pc, &bufsz, 4);
        mprintf(pc, &bufsz, "</ParameterList>\n");
        xml_mIndent(pc, &bufsz, 3);
        mprintf(pc, &bufsz, "</%sInform>\n", nsCWMP);
        closeBodyEnvelope(pc, &bufsz);

        xml_mIndent(pc, &bufsz, MAX_PADDINGS);

        /* send the HTTP message header*/
        sendToAcs(bufsz, NULL);

        /* send the HTTP message body*/
        pc = getAcsConnDesc();
        proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
        
    } while (0);

    tr69c_freeAllocBuf();
   
    if (acsState.fault == VOS_RET_SUCCESS)
    {
        transferCompletePending = ((informState == eACSDownloadReboot) ||
                                   (informState == eACSUpload));
        /* increment retry count is moved to runRPC() function */
    }

    if (SF_FEATURE_LOCATION_FUJIAN)
    {
        g_totalAssociationsEnable = FALSE;
    }
}  /* End of buildInform() */


void sendGetRPCMethods(void)
{
   tProtoCtx *pc = NULL;

   vosLog_debug("sendGetRPCMethods");

   tr69c_initAllocBuf();
   
   do
   {
      int   bufsz = 0;

      openEnvWithHeader(NULL, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sGetRPCMethods>\n", nsCWMP);
      xml_mIndent(pc, &bufsz,3);
      mprintf(pc, &bufsz, "</%sGetRPCMethods>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
	  
   } while (0);
   
   tr69c_freeAllocBuf();
      
}  /* End of sendGetRPCMethods() */

void sendTransferComplete(void)
{
   char        *ck;
   tProtoCtx *pc = NULL;
   char        dateTimeBuf[BUFLEN_64];

   vosLog_debug("sendTransferComplete");

   tr69c_initAllocBuf();
   
   do
   {
      int   bufsz = 0;

      openEnvWithHeader("12345678", pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sTransferComplete>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 4);
      ck=acsState.downloadCommandKey;
      #ifdef SUPPRESS_EMPTY_PARAM
      if (ck && util_strlen(ck)>0)
      {
         mprintf(pc, &bufsz, "<CommandKey>%s</CommandKey>\n", ck);
      }
      else
      {
         mprintf(pc, &bufsz, "<CommandKey>empty</CommandKey>\n");
      }
      #else
      mprintf(pc, &bufsz, "<CommandKey>%s</CommandKey>\n", ck? ck: "");
      #endif
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<FaultStruct>\n");
      xml_mIndent(pc, &bufsz, 5);
      mprintf(pc, &bufsz, "<FaultCode>%d</FaultCode>\n",acsState.dlFaultStatus);
      xml_mIndent(pc, &bufsz, 5);
      mprintf(pc, &bufsz, "<FaultString>%s</FaultString>\n", acsState.dlFaultMsg?
      #ifdef SUPPRESS_EMPTY_PARAM
                  acsState.dlFaultMsg: "empty");
      #else
                  acsState.dlFaultMsg: "");
      #endif
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "</FaultStruct>\n");
      xml_mIndent(pc, &bufsz, 4);
      utilTms_getXSIDateTime((UINT32)acsState.startDLTime, dateTimeBuf, sizeof(dateTimeBuf));
      mprintf(pc, &bufsz, "<StartTime>%s</StartTime>\n", dateTimeBuf);
      xml_mIndent(pc, &bufsz, 4);
      utilTms_getXSIDateTime((UINT32)acsState.endDLTime, dateTimeBuf, sizeof(dateTimeBuf));
      mprintf(pc, &bufsz, "<CompleteTime>%s</CompleteTime>\n", dateTimeBuf);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sTransferComplete>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
	  
   } while (0);
   
   tr69c_freeAllocBuf();

}  /* End of sendTransferComplete() */

static void doDownload(RPCAction *a)
{
   tProtoCtx *pc = NULL;
   DownloadReq *r = &a->ud.downloadReq;
   
   vosLog_debug("doDownload");

   if (a == acsRpcAction)
   {
      acsRpcAction = NULL;  /* if *a is copy of acsRpcAction. set to NULL */
   }

   vosLog_debug("preDownloadSetup: URL=%s", r->url);
   vosLog_debug("User/pw: %s:%s", r->user, r->pwd);
   vosLog_debug("Required memory buffer size will be %d", r->fileSize);

   tr69c_initAllocBuf();
   
   do
   {
      int   bufsz = 0;

      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sDownloadResponse>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<Status>1</Status>\n");
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<StartTime>0001-01-01T00:00:00Z</StartTime>\n");
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<CompleteTime>0001-01-01T00:00:00Z</CompleteTime>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sDownloadResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);

   } while (0);
   
   tr69c_freeAllocBuf();

   /* queue this event up */
   if (requestQueued(r,a->rpcMethod) == FALSE)
   {
      /* cannot queue this request, resource exceeded, mark it so when it's time to
         process it, an error is sent instead */
      r->state = eTransferRejected;
   }

   if (eFirmwareUpgrade == r->efileType && SF_FEATURE_SUPPORT_PLUGIN)
   {
      acsState.upgradeDownloadFlag = 1;
   }

   utilTmr_set(tmrHandle, downloadStart, (void *)a, (UINT32)(1+r->delaySec)*1000, "download");
}  /* End of doDownload() */

static void doUpload(RPCAction *a)
{
   tProtoCtx *pc = NULL;
   DownloadReq *r = &a->ud.downloadReq;
   
   if (a == acsRpcAction)
      acsRpcAction = NULL;  /* if *a is copy of acsRpcAction. set to NULL */

   vosLog_debug("UploadSetup: URL=%s", r->url);
   vosLog_debug("User/pw: %s:%s", r->user, r->pwd);
   vosLog_debug("Required memory buffer size will be %d", r->fileSize);

   tr69c_initAllocBuf();
   
   do
   {
      int   bufsz = 0;

      /* build good response */
      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sUploadResponse>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<Status>1</Status>\n");
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<StartTime>0001-01-01T00:00:00Z</StartTime>\n");
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "<CompleteTime>0001-01-01T00:00:00Z</CompleteTime>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sUploadResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);
	  
      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);

   } while (0);
   
   tr69c_freeAllocBuf();
   
   /* queue this event up */
   if (requestQueued(r,a->rpcMethod) == FALSE)
   {
      /* cannot queue this request, resource exceeded, mark it so when it's time to
         process it, an error is sent instead */
      r->state = eTransferRejected;
   }
   utilTmr_set(tmrHandle, uploadStart, (void *)a, (UINT32)(1+r->delaySec)*1000, "upload");
}  /* End of doUpload() */

/*
 * Update the ACSState parameterKey or commandKeys if they are in
 * the RPC.
 */
void updateKeys(RPCAction *pRpcAction)
{
   if (pRpcAction->parameterKey)
   {
      if (rebootFlag == NOREBOOT)
      {
         /* not rebooting also copy to current parameter key */
         VOS_MEM_FREE_BUF_AND_NULL_PTR(acsState.parameterKey);
         acsState.parameterKey = VOS_STRDUP(pRpcAction->parameterKey);
      }
      if (acsState.newParameterKey)
      {
         VOS_MEM_FREE_BUF_AND_NULL_PTR(acsState.newParameterKey);
      }
      acsState.newParameterKey = pRpcAction->parameterKey;
      CMC_tr69cSetMgmtServerParameterKey(pRpcAction->parameterKey);
      pRpcAction->parameterKey = NULL;
   }
   if (pRpcAction->rpcMethod==rpcReboot && pRpcAction->commandKey)
   {
      if (acsState.rebootCommandKey)
      {
         VOS_MEM_FREE_BUF_AND_NULL_PTR(acsState.rebootCommandKey);
      }
      acsState.rebootCommandKey = pRpcAction->commandKey;
      pRpcAction->commandKey = NULL;
   }
   if (pRpcAction->rpcMethod==rpcScheduleInform && pRpcAction->commandKey)
   {
      if (acsState.scheduleInformCommandKey)
      {
         VOS_MEM_FREE_BUF_AND_NULL_PTR(acsState.scheduleInformCommandKey);
      }
      acsState.scheduleInformCommandKey = pRpcAction->commandKey;
      pRpcAction->commandKey = NULL;
   }
}  /* End of updateKeys() */


static void tr69c_initCtcLoidAuthenticationStatus(void)
{
    FILE    *fp = NULL;
    fp = fopen("/tmp/CtcLoidAuthStatus", "w+");

    if (NULL != fp)
    {
        fprintf(fp, "%d", 0);
        fclose(fp);
    }
} 


static VOS_RET_E tr69c_sendMsgToRegister(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char msg_buf[sizeof(VosMsgHeader) + BUFLEN_24 + BUFLEN_12] = {0};
    REGISTER_INFO_T regInfo;
    VosMsgHeader *msg_hal = (VosMsgHeader *)msg_buf;
    char *regLoidPwd = NULL;

    memset(&regInfo, 0, sizeof(regInfo));
    regLoidPwd = (char *)(msg_hal + 1);
    ret = CMC_registerGetInfo(&regInfo);
    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(regLoidPwd, regInfo.userName, BUFLEN_24);
        memcpy(regLoidPwd + BUFLEN_24, regInfo.userPassword, BUFLEN_12);

        ret = HAL_sysSetLoidPwd(regInfo.userName, regInfo.userPassword);
        if (ret != VOS_RET_SUCCESS)
        {
            vosLog_error("set HAL_sysSetLoidPwd fail, ret = %d", ret);
        }
    }
    msg_hal->type = VOS_MSG_CT_CHANGE_LOID_PASSWD;
    msg_hal->src  = EID_TR69C;

    if (SF_FEATURE_UPLINK_TYPE_EPON)
    {
        msg_hal->dst  = EID_EPON_APP;
        vosLog_debug("Register: this is EPON!");
    }
    else
    {
        msg_hal->dst  = EID_GPONHAL;
        vosLog_debug("Register: this is GPON!");
    }

    msg_hal->dataLength = BUFLEN_24 + BUFLEN_12;
    msg_hal->flags_request = 1;

    vosLog_debug("send msg VOS_MSG_CT_CHANGE_LOID_PASSWD");
    ret = vosMsg_send(g_msgHandle, msg_hal);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("could not send out VOS_MSG_CT_CHANGE_LOID_PASSWD event msg, ret = %d\n", ret);
    }
    else
    {
        vosLog_notice("send msg VOS_MSG_CT_CHANGE_LOID_PASSWD success");
    }

    tr69c_initCtcLoidAuthenticationStatus();

    regInfo.status = 99;
    regInfo.result = 99;
    ret = CMC_registerSetInfo(&regInfo);
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("CMC_registerSetInfo fail, ret = %d", ret);
    }

    addInformEventToList(INFORM_EVENT_LOID_CHANGE);
    sendInform(NULL);

    return ret;
}


RunRPCStatus runRPC(void)
{
   RunRPCStatus rpcStatus = eRPCRunOK;
   VOS_RET_E ret = VOS_RET_SUCCESS;
   UBOOL8 writecard_flag = FALSE;
   RPCAction *rpcAction = getAction();

   if (rpcAction != NULL)
   {
      vosLog_debug("=====>ENTER: rcpMethod=%d", rpcAction->rpcMethod);
   }
   else
   {
      vosLog_debug("=====>ENTER: NULL acsRpcAction pointer!");
   }

   acsState.fault = 0;
  
   if (!isAcsConnected())
   {
      vosLog_debug("Not connected to ACS");
      rpcStatus = eRPCRunFail;
   }
   
   rpcStatus = eRPCRunOK;

   if ((rpcStatus == eRPCRunOK) && (rpcAction != NULL))
   {
      /*
       * Acquire lock before doing any RPC method.
       */
      if (ret != VOS_RET_SUCCESS)
      {
         rpcStatus = eRPCRunFail;
         acsState.fault = VOS_RET_INTERNAL_ERROR;
         writeSoapFault(rpcAction, acsState.fault);
      }
      else
      {
         /* after Inform is sent, no other RPCs should be sent before CPE receives InformResponse
          * if any RPC is sent before InformResponse (before sessionState is set to eSessionDeliveryConfirm)
          * then CPE should cancel current session and retry another session
          */
         if (rpcAction->rpcMethod != rpcInformResponse 
          && sessionState != eSessionDeliveryConfirm 
          && (NULL == simRpcAction))
         {
            vosLog_error("state machine wrong state, fail conn");
            rpcStatus = eRPCRunFail;
            acsState.retryCount++;
            retrySessionConnection();
            saveTR69StatusItems();   // save retryCount to scratchpad
         }
         else
         {
            switch (rpcAction->rpcMethod)
            {
            case rpcGetRPCMethods:
               doGetRPCMethods(rpcAction);
               break;
            case rpcSetParameterValues:
               doSetParameterValues(rpcAction);
               if (SF_FEATURE_SUPPORT_CARD)
               {
                   writecard_flag = TRUE;
               }
               break;
            case rpcGetParameterValues:
               doGetParameterValues(rpcAction);
               break;
            case rpcGetParameterNames:
               doGetParameterNames(rpcAction);
               break;
            case rpcGetParameterAttributes:
               doGetParameterAttributes(rpcAction);
               break;
            case rpcSetParameterAttributes:
               doSetParameterAttributes(rpcAction);
               break;
            case rpcAddObject:
               doAddObject(rpcAction);
               if (SF_FEATURE_SUPPORT_CARD)
               {
                   writecard_flag = TRUE;
               }
               break;
            case rpcDeleteObject:
               doDeleteObject(rpcAction);
               if (SF_FEATURE_SUPPORT_CARD)
               {
                   writecard_flag = TRUE;
               }
               break;
            case rpcReboot:
               doRebootRPC(rpcAction);
               break;
            case rpcFactoryReset:
               doFactoryResetRPC(rpcAction);
               break;
            case rpcDownload:
               doDownload(rpcAction);
               break;
            case rpcUpload:
               doUpload(rpcAction);
               break;
            case rpcScheduleInform:
               doScheduleInform(rpcAction);
               break;            
            case rpcGetQueuedTransfers:
               doGetQueuedTransfers(rpcAction);
               break;
            case rpcInformResponse:
               // event delivery is confirmed by receiving InformResponse            
               sessionState = eSessionDeliveryConfirm;
               acsState.retryCount = 0;
               /* move inform success to here */
               CMC_tr69cSetRemoteInform(CMC_TR69C_DIAG_INFORM_SUCCESS, CMC_TR69C_REMOTE_INFORM_STATUS);    //inform success

               /* we have received an informResponse, clear inform event list */
               clearInformEventList();

               /* we have Inform , we need clear upgradeDownloadFlag */
               if (SF_FEATURE_SUPPORT_PLUGIN)
               {
                  acsState.upgradeDownloadFlag = 0;
                  if (!util_strcmp(alarmNumber, "104006"))
                  {
                     UTIL_DO_SYSTEM_ACTION("rm /tmp/lanEthDisavailable -rf; rm /tmp/lanEthAvailable -rf");
                  }
               }

               {
                  FILE *managementserver_url = fopen("/var/ms_url", "w");
                  if (managementserver_url != NULL)
                  {
                     int urlchanged = 0;
                     fwrite((void *)&urlchanged, sizeof(urlchanged), 1, managementserver_url);
                     fclose(managementserver_url);
                  }
               }
               
               CMC_tr69cSetLastConnectedURL(&saveConfigFlag);

               if (acsState.holdRequests == 0)
               {
                  // only send pending request when holdrequests is false
                  if (transferCompletePending == 1)
                  {
                     /* make any callbacks that were setup when RPC started */
                     sendTransferComplete();
                     transferCompletePending = 0;
                     // setACSContactedState to eACSInformed for clearing 
                     // previous state which is eACSDownloadReboot or eACSUpload
                     setInformState(eACSInformed);
                  }
                  else if (sendGETRPC)
                  {
                     sendGetRPCMethods();
                     sendGETRPC = 0;
                  }
                  else
                  {
                     /* send empty message to indcate no more requests */
                     sendNullHttp(TRUE);
                     rpcStatus=eRPCRunEnd;
                  }
               }
               else
               { 
                  // only send NULL msg when holdrequests is true
                  sendNullHttp(TRUE);
                  rpcStatus = eRPCRunEnd;
               }
               resetNotification();  /* update notifications following informResponse*/
               setInformState(eACSInformed);
               break;
            case rpcTransferCompleteResponse:
               sendNullHttp(TRUE);
               rpcStatus = eRPCRunEnd;
               setInformState(eACSInformed);
               break;
            case rpcGetRPCMethodsResponse:
               sendNullHttp(TRUE);
               rpcStatus = eRPCRunEnd;
               break;
            case rpcFault:
            default:
               rpcStatus = eRPCRunFail;
               break;
            }  /* end of switch(acsRpcAction->rpcMethod) */
          }
       }

#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
      if (SF_FEATURE_SUPPORT_CARD)
      {
          //支持写卡
          UINT32 cardtype = 0;
          CMC_cardGetCardType(&cardtype);
          if ((TRUE == writecard_flag) && (1 == cardtype))
          {
             writecard_flag = FALSE;
             switch (rpcAction->rpcMethod)
             {
                case rpcAddObject:
                case rpcSetParameterValues:
                case rpcDeleteObject:
                   dealcardinfo(rpcAction);
                   break;
                default:
                   break;
             }
          }
      }
#endif

        if (SF_FEATURE_LOCATION_JIANGSU || SF_FEATURE_LOCATION_SUZHOU)
        {
            if (TRUE == loid_changed_flag)
            {
                loid_changed_flag = FALSE;
                sleep(2);
                tr69c_sendMsgToRegister();
            }
        }

        if (1 == download_diag)
        {
            if (!SF_FEATURE_LOCATION_SICHUAN)
            {
                handle_download_diag();
            }
            download_diag = 0;
            change_download_param = 0;
        }
        else if (1 == change_download_param)
        {
            change_download_param = 0;
            if (isTransferInProgress())
            {
                downloadDiagStop("is Transfer In Progress", 9013);
            }
            CMC_tr69cSetDownloadDiagState(NONE);
        }

        if (1 == upload_diag)
        {
            handle_upload_diag();
            upload_diag = 0;
            change_upload_param = 0;
        }
        else if (1 == change_upload_param)
        {
            change_upload_param = 0;
            if (isTransferInProgress())
            {
                uploadStop("is Transfer In Progress", 9013);
            }
            CMC_tr69cSetUploadDiagState(NONE);
        }

      /* if no faults then update ACS state with parameter key or command key. */
      if (rpcStatus != eRPCRunFail && acsState.fault == NO_FAULT && acsRpcAction != NULL)
      {
         /* in the case of download this must wait until the download completes*/
         updateKeys(acsRpcAction);
      }

      /* 
       * Possibly write config here?
       * Release lock after updateKeys() since updateKeys() needs lock to read/write parameterKey.
       */
   } /* end of if ((rpcStatus == eRPCRunOK) && (acsRpcAction != NULL)) */



   vosLog_debug("=====>EXIT, rpcStatus=%d", rpcStatus);

#ifdef verbose_mem_stats
   {
      /*
       * This bit of code can be used to detect whether tr69c memory use
       * is increasing after each RPC.  Increasing memory usage most likely
       * means memory leak.
       */
      VosMemStats stats;

      vosMem_getStats(&stats);

      printf("\n\n=========== tr69c (pid=%d) mem stats after RPC =================\n", getpid());
      printf("shmBytesAllocd=%u (total=%u) shmNumAllocs=%u shmNumFrees=%u delta=%u\n",
             stats.shmBytesAllocd, stats.shmTotalBytes,
             stats.shmNumAllocs, stats.shmNumFrees, stats.shmNumAllocs - stats.shmNumFrees);
      printf("bytesAllocd=%u numAllocs=%u numFrees=%u delta=%u\n",
             stats.bytesAllocd, stats.numAllocs, stats.numFrees,
             stats.numAllocs-stats.numFrees);
      printf("============================================\n\n");
   }
#endif

   return rpcStatus;
}  /* End of runRPC() */


/*begin and by hehulin*/
void dealcardinfo(RPCAction *a)
{
#ifdef DMP_X_CT_COM_SUPPORTCARDMON_1
    if (SF_FEATURE_SUPPORT_CARD)
    {
        char *pp = NULL;
        UINT32 ret = 0;
        VOS_RET_E resulte = 0;
        UINT32 resiult = 0;

        printf("dealcardinfo\n");

        if (NULL == a)
        {
            return;
        }

        if (rpcSetParameterValues == a->rpcMethod)
        {
            pp  = a->ud.pItem->pname;
        }
        else if ((rpcAddObject == a->rpcMethod) || (rpcDeleteObject == a->rpcMethod))
        {
            pp  = a->ud.addDelObjectReq.objectName;
        }

        if (util_strstr(pp, STR_CARD_KEYPARAM) && g_keyhaveset == 1)
        {
            CMC_cardStatusLedOn();
            /*Auth sucessful read data from card*/
            resulte = CMC_cardFilesReadFromCard();
            if (VOS_RET_SUCCESS == resulte)
            {
                addInformEventToList(INFORM_EVENT_CT_CARDNOTIFY);
                sendInform(NULL);
            }
            else if (VOS_RET_CARD_STATUS_FAILED == resulte)
            {
                Sendalarmmsgtoitms("105003");
                return;
            }

            /*write MAC to card*/
            CMC_cardFilesWriteMACToCard();

            /*read card result from card*/
            CMC_cardFilesReadSaveResultFlag(&resiult);
        }
        else if ((util_strstr(pp, STR_CARD_KEYPARAM) && g_keyhaveset == 0)
            || (util_strstr(pp, CHECK_LOID_STATUSPARAM) && g_statushaveset == 1))
        {
            /*send alarm to tr069 for auth failed*/
            if (g_statusautheachother == 1)
            {
                Sendalarmmsgtoitms("105004");
            }
            else if (g_statusautheachother == 2)
            {
                Sendalarmmsgtoitms("105005");
            }
            /*show light*/
            if (util_strstr(pp, STR_CARD_KEYPARAM) && g_keyhaveset == 0)
            {
                CMC_cardCheckFailedFlicker();
                CMC_cardBusinessLedOff();
            }
         
            /*remember deal with web page*/    

            /*stop all business*/
            char buf[sizeof(VosMsgHeader)] = {0};
            VosMsgHeader *tr69msg = (VosMsgHeader *)buf;
            tr69msg->type = VOS_MSG_CT_RESETMDM;
            tr69msg->src = EID_TR69C;
            tr69msg->dst = EID_SSK;

            if ((ret = vosMsg_send(g_msgHandle, tr69msg)) != VOS_RET_SUCCESS)
            {
                vosLog_error("Could not send out CMS_MSG_UPNP_GETALL ret=%d", ret);
            }
            
        }
        else
        {
            if (util_strstr(pp, CHECK_LOID_STATUSPARAM))
            {
                return;       
            }
            
            doWriteintoCard(a);
        }
    }
#endif    
}
/*end and by hehulin*/

void handle_download_diag()
{
    CMC_TR69C_DOWNLOAD_DIAG_CFG_T download_diag_tmp;
    memset((void *)&download_diag_tmp, 0, sizeof(download_diag_tmp));

    CMC_tr69cGetDownloadDiagParam(&download_diag_tmp);

    if (0 == util_strcmp(download_diag_tmp.diagnosticsstate, "Requested"))
    {
        if (isTransferInProgress())
        {
            downloadDiagStop("is Transfer In Progress", 9013);
        }
        downloaddiagStart(&download_diag_tmp);
    }
}

void handle_upload_diag()
{
    CMC_TR69C_UPLOAD_DIAG_CFG_T upload_diag_tmp;
    memset((void *)&upload_diag_tmp, 0, sizeof(upload_diag_tmp));

    CMC_tr69cGetUploadDiagParam(&upload_diag_tmp);

    if (util_strcmp(upload_diag_tmp.diagnosticsstate, "Requested"))
    {
        if (isTransferInProgress())
        {
            uploadStop("is Transfer In Progress", 9013);
        }
        uploaddiagStart(&upload_diag_tmp);
    }
}

void resetNotification(void)
{
   vosLog_debug("============>ENTER");
   /* we must be inside an RPC method function because we have the lock at this point. */
   CMC_phlClearAllParamValueChanges();
}

int checkActiveNotifications(void)
{
   int active = 0;
   return active;
}

void initTransferList(void)
{
   UINT16 size, i, queueEntryCount;
   DownloadReq *q;
   RPCAction *a;
   DownloadReqInfo savedList[TRANSFER_QUEUE_SIZE], *saved;

   memset((void*)&transferList,0,sizeof(TransferInfo));
   size = i = queueEntryCount = 0;

   /* if retrieve fails because nothing was read or error occured, clear list */
   if (tr69RetrieveTransferListFromStore(savedList, &size) == VOS_RET_SUCCESS && size > 0)
   {
      /* remove the tr69c_transfer list from scratch pad by setting it again with len=0 */
      if (VOS_RET_SUCCESS != HAL_sysSetTr69cData("tr69c_transfer", NULL, 0))
      {
         vosLog_error("Unable to save tr69c_transfer in scratch PAD");
      }

      queueEntryCount = size / sizeof(DownloadReqInfo);
      for (i = 0; i < queueEntryCount; i++)
      {
         saved = &savedList[i];
         q = &transferList.queue[i].request;
         
         q->efileType = saved->efileType;
         q->commandKey = (saved->commandKey != NULL) ? VOS_STRDUP(saved->commandKey) : NULL;
         q->url = (saved->url != NULL) ? VOS_STRDUP(saved->url) : VOS_STRDUP("");
         q->user = (saved->user != NULL) ? VOS_STRDUP(saved->user) : VOS_STRDUP("");
         q->pwd = (saved->pwd != NULL) ? VOS_STRDUP(saved->pwd) : VOS_STRDUP("");
         q->fileSize = saved->fileSize;
         q->fileName = (saved->fileName != NULL) ? VOS_STRDUP(saved->fileName) : VOS_STRDUP("");
         q->delaySec = saved->delaySec;
         q->state = saved->state;
         transferList.queue[i].rpcMethod = saved->rpcMethod;
         if (q->state == eTransferNotYetStarted)
         {
            a = newRPCAction(); /* need to be freed somewhere? */
            memcpy(&a->ud.downloadReq,q,sizeof(DownloadReq));
            a->rpcMethod = transferList.queue[i].rpcMethod;
            if (transferList.queue[i].rpcMethod == rpcDownload)
            {
               utilTmr_set(tmrHandle, downloadStart, (void *)a, (UINT32)(1+q->delaySec)*1000, "download");
            }
            else
            {
               utilTmr_set(tmrHandle, uploadStart, (void *)a, (UINT32)(1+q->delaySec)*1000, "upload");
            }
         } /* eTransferNotYetStarted */         
      } /* for */
   } /* read from scratch pad */
   else
   {
#ifdef DEBUG
      printf("initTransferList(): no list read from persistent storage.\n");
#endif
   }
} /* initTransferList */

/* return true if a transfer transaction is in progess; 0 if not.*/
int isTransferInProgress(void)
{
   int i;
   DownloadReq *q;
   
   for (i = 0; i < TRANSFER_QUEUE_SIZE; i++)
   {
      q = &transferList.queue[i].request;
      if (q->state == eTransferInProgress)
      {
         return 1;
      }
   }
   return (0);
}

void transferListEnqueueCopy(DownloadReq *q, DownloadReq *r)
{
   if ((q != NULL) && (r != NULL))
   {
      q->efileType = r->efileType;
      q->commandKey = (r->commandKey != NULL) ? VOS_STRDUP(r->commandKey) : NULL;
      q->url = (r->url != NULL) ? VOS_STRDUP(r->url) : VOS_STRDUP("");
      q->user = (r->user != NULL) ? VOS_STRDUP(r->user) : VOS_STRDUP("");
      q->pwd = (r->pwd != NULL) ? VOS_STRDUP(r->pwd) : VOS_STRDUP("");
      q->fileSize = r->fileSize;
      q->fileName = (r->fileName != NULL) ? VOS_STRDUP(r->fileName) : VOS_STRDUP("");
      q->delaySec = r->delaySec;
      q->state = eTransferNotYetStarted;
   }
}

void transferListDequeueFree(DownloadReq *q)
{
   if (q->commandKey)
      VOS_FREE(q->commandKey);
   if (q->url)
      VOS_FREE(q->url);
   if (q->user)
      VOS_FREE(q->user);
   if (q->pwd)
      VOS_FREE(q->pwd);
   if (q->fileName)
      VOS_FREE(q->fileName);
   memset((void*)q,0,sizeof(DownloadReq));
}

/* This function queue the request if there is room and return 1, otherwise, return 0 */
int requestQueued(DownloadReq *r, eRPCMethods method)
{
   int i, j;
   DownloadReq *q;
   
   for (i = 0; i < TRANSFER_QUEUE_SIZE; i++)
   {
      q = &transferList.queue[i].request;
      if (q->state == eTransferNotInitialized)
      {
         transferListEnqueueCopy(q,r);
         transferList.queue[i].rpcMethod = method;
         /* request is queued */

         return (1); 
      }
   } /* for */
   /* queue is full; purge the completed one for this new request */
   /* the goal is to leave the recently completed ones for ACS to queried, overwrite the oldest
      completed entries when when purged */
   for (i = transferList.mostRecentCompleteIndex+1, j=0; j < TRANSFER_QUEUE_SIZE; i++, j++)
   {
      q = &transferList.queue[i%TRANSFER_QUEUE_SIZE].request;
      if (q->state == eTransferCompleted)
      {
         transferListDequeueFree(q);
         transferListEnqueueCopy(q,r);
         transferList.queue[i%TRANSFER_QUEUE_SIZE].rpcMethod = method;

         return (1); 
      }
   } /* for */
   /* queue is full; we have to reject this request; there is no room to queue it */
   /* return 9004, resource exceeded: how?
      acsState.fault is a global flag and if a transfer is in progress, this doesn't work */
   
   return (0);
}

void updateTransferState(char *commandKey, eTransferState state)
{
   int i;
   DownloadReq *q;

   for (i = 0; i < TRANSFER_QUEUE_SIZE; i++)
   {
      q = &transferList.queue[i].request;
      if (q->state != eTransferNotInitialized)
      {
         if ((commandKey == NULL && q->commandKey == NULL) ||
             (commandKey != NULL && q->commandKey != NULL && 
              util_strcmp(commandKey,q->commandKey) == 0))
         {
            q->state = state;
            if (state == eTransferCompleted)
            {
               transferList.mostRecentCompleteIndex = i;
            }
            break;
         }
      }
   } /* for */
}

static void doGetQueuedTransfers(RPCAction *a)
{
   int i;
   tProtoCtx *pc = NULL;
   DownloadReq *q;
   TransferInfo list;
   int qEntryCount = 0;

   memcpy(&list,&transferList,sizeof(TransferInfo));

   for (i = 0; i < TRANSFER_QUEUE_SIZE; i++) {
      q = &list.queue[i].request;
      if (q->state != eTransferNotInitialized)
      {
         qEntryCount++;
      }
   } /* for */

   tr69c_initAllocBuf();
   
   do 
   {

      int   bufsz = 0;
   
      openEnvWithHeader(a->ID, pc, &bufsz);
      openBody(pc, &bufsz);
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "<%sGetQueuedTransfersResponse>\n", nsCWMP);
      xml_mIndent(pc, &bufsz, 4);
      #ifdef SUPPRESS_SOAP_ARRAYTYPE
      mprintf(pc, &bufsz, "<TransferList>\n");
      #else
      mprintf(pc, &bufsz, "<TransferList %sarrayType=\"%sQueuedTransferStruct[%d]\">\n",
              nsSOAP_ENC, nsCWMP, qEntryCount );
      #endif

      
      for (i = 0; i < qEntryCount; i++) {
         q = &list.queue[i].request;
         xml_mIndent(pc, &bufsz, 5);
         mprintf(pc, &bufsz, "<QueuedTransferStruct>\n");
         if (q->state != eTransferNotInitialized)
         {
            xml_mIndent(pc, &bufsz, 6);
            if (q->commandKey != NULL)
               mprintf(pc, &bufsz, "<CommandKey>%s</CommandKey>\n", q->commandKey);
            else
               mprintf(pc, &bufsz, "<CommandKey></CommandKey>\n");
            xml_mIndent(pc, &bufsz, 6);
            mprintf(pc, &bufsz, "<State>%d</State>\n", q->state);
         }
         else
         {
            xml_mIndent(pc, &bufsz, 6);
            mprintf(pc, &bufsz, "<CommandKey></CommandKey>\n");
            xml_mIndent(pc, &bufsz, 6);
            mprintf(pc, &bufsz, "<State></State>\n");
         }
         xml_mIndent(pc, &bufsz, 5);
         mprintf(pc, &bufsz, "</QueuedTransferStruct>\n");
      } /* for all queued entries */
      xml_mIndent(pc, &bufsz, 4);
      mprintf(pc, &bufsz, "</TransferList>\n");
      xml_mIndent(pc, &bufsz, 3);
      mprintf(pc, &bufsz, "</%sGetQueuedTransfersResponse>\n", nsCWMP);
      closeBodyEnvelope(pc, &bufsz);

      /* send the HTTP message header*/
      sendToAcs(bufsz, NULL);
      
      /* send the HTTP message body*/
      pc = getAcsConnDesc();
      proto_SendRaw(pc, sg_tr69cAllocBuf.buf, bufsz);
      
   } while (0);
   
   tr69c_freeAllocBuf();

}  /* End of doGetQueuedTransfers() */

/* this routine just set some parameters to have
 * default active notification as defined in section 2.4
 * of TR98 specfication.
 */

void setDefaultActiveNotification(void)
{
   CMC_tr69cSetDefaultActiveNotification(&saveConfigFlag);
   saveConfigurations();
}


void ctmdw_resetNotification(UINT8 cleanflag)
{
   vosLog_debug("============>ENTER");
   /* we must be inside an RPC method function because we have the lock at this point. */
   CMC_phlMdwClearAllParamValueChanges(cleanflag);
}


int ctmdw_getAllMDWNotifications(char *pc)
{
    int i;
    //int a;
    //UINT32 paramNum = 0;
    
    char  *fullpath = NULL;
    int ret = VOS_RET_SUCCESS;
    CMC_PHL_GET_PARAM_ATTR_T paramAttr;
    char tmpbuf[BUFLEN_512];
    char tmpctname[BUFLEN_512];        
    char skip = 0;
    UBOOL8 change = FALSE;
    UBOOL8 isExist = FALSE;
    char nextPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};
    int isfirst = 1;

    memset(&paramAttr, 0, sizeof(paramAttr));

    for (i = 0 ; i < ARRAY_SIZE(ctdefaultvalue); i++) 
    {
        char*  paramValue;

        ret = tr69c_getParamValue(ctdefaultvalue[i].paraName, &paramValue);
        if (ret == VOS_RET_SUCCESS)
        {
            if (paramValue[0] != '\0' && ((util_strcmp(paramValue, ctdefaultvalue[i].pvalue) != 0)
            || (wanChangeNotification && ( !util_strcmp(ctdefaultvalue[i].paraName, "InternetGatewayDevice.ManagementServer.InternetPvc")))))
            {
                UTIL_STRNCPY(ctdefaultvalue[i].pvalue, paramValue, BUFLEN_256);
                
                if (pc)
                {
                    memset(tmpbuf, 0, 512);
                    memset(tmpctname, 0, 512);
                    if (0 != mappingTR69NameToCTName(ctdefaultvalue[i].paraName, tmpctname, util_strlen(ctdefaultvalue[i].paraName)))
                    {
                        UTIL_STRNCPY(tmpctname, fullpath, BUFLEN_512);
                    }

                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "&%s=%s", tmpctname, paramValue);
                    if (isfirst)
                    {
                        UTIL_STRNCPY(pc, tmpbuf, CTMDW_PACKET_MAXLEN);
                        isfirst = 0;
                    }
                    else
                    {
                        UTIL_STRNCAT(pc, tmpbuf, CTMDW_PACKET_MAXLEN);
                    }

                    CTMDW_DEBUG("ctmdw: ctmdw_getAllMDWNotifications found %s \n", tmpbuf);
                }    
            }
            
            VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
        }                    
        else
        {                
            CTMDW_DEBUG("ctmdw: ctmdw_getAllMDWNotifications can't find ctdefaultvalue from the TR069 Object Tree!!!\n");
        }
    }

    ret = CMC_phlIsPathExist("InternetGatewayDevice.", &isExist);
    if (isExist)
    {

        while (ret == VOS_RET_SUCCESS)
        {
            ret = CMC_phlGetNextPath(TRUE, FALSE, "InternetGatewayDevice.", nextPath, sizeof(nextPath));

            if (ret == VOS_RET_SUCCESS)
            {
                for (i = 0; i < ARRAY_SIZE(ctdefaultvalue); i++)
                {
                    if (util_strcmp(ctdefaultvalue[i].paraName, "InternetGatewayDevice") == 0);
                    skip = 1;
                }

                if (skip == 0)
                {
                    memset(tmpbuf, 0, BUFLEN_512);
                    memset(tmpctname, 0, BUFLEN_512);
    
                    CMC_phlGetParamAttr(nextPath, &paramAttr);
    
                    if (paramAttr.notification >= CTMDW_NOTIFICATION_READABLE)
                    {
                        if (CMC_phlIsParamValueChanged(nextPath, &change))
                        {                        
                            char *paramValue = NULL;
                            if (tr69c_getParamValue(nextPath, &paramValue) == VOS_RET_SUCCESS)
                            {
                                if ((paramValue[0] != '\0') && pc)
                                {
                                    if(0 != mappingTR69NameToCTName(paramAttr.paramPath, tmpctname, sizeof(paramAttr.paramPath)))
                                    {
                                        UTIL_STRNCPY(tmpctname, paramAttr.paramPath, BUFLEN_512);
                                    }
    
                                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "&%s=%s", tmpctname, paramValue);
                                    UTIL_STRNCAT(pc, tmpbuf, CTMDW_PACKET_MAXLEN);
                                }                              
                            }
                        }
                    }
                }
                else
                {
                    skip = 0;
                    vosLog_debug("CMC_phlGetParamAttrList skip default param");
                }
            }
            else if (ret == VOS_RET_NO_MORE_INSTANCES)
            {
                ret = VOS_RET_SUCCESS;
                break;
            }
            else
            {
                vosLog_error("CMC_phlGetNextPath error ret = %d", ret);
            }
        }
#if 0
    ret = CMC_phlGetParamAttrList("InternetGatewayDevice",TRUE, FALSE, paramAttr, &paramNum);
    if (ret == VOS_RET_SUCCESS)
    {
        for (a = 0; a < paramNum; a++)
        {
            for (i=0 ; i < ARRAY_SIZE(ctdefaultvalue); i++ ) 
            {
                if (util_strcmp(ctdefaultvalue[i].paraName, "InternetGatewayDevice") == 0);
                skip = 1;
            }

            if (skip == 0)
            {
                memset(tmpbuf, 0, BUFLEN_512);
                memset(tmpctname, 0, BUFLEN_512);

                if (paramAttr->notification >= CTMDW_NOTIFICATION_READABLE)
                {
                    CMC_phlIsParamValueChanged(paramAttr->paramPath, &change);
                    if (TRUE == change)
                    {
                        ret = tr69c_getParamValue(paramAttr->paramPath, &paramValue);
                        if (ret == VOS_RET_SUCCESS)
                        {
                            if ((paramValue != NULL) && pc)
                            {
                                if (0 != mappingTR69NameToCTName(paramAttr->paramPath, tmpctname, sizeof(paramAttr->paramPath)))
                                {
                                    UTIL_STRNCPY(tmpctname, paramAttr->paramPath, BUFLEN_512);
                                }

                                UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "&%s=%s", tmpctname, paramValue);
                                UTIL_STRNCAT(*pc, tmpbuf, CTMDW_PACKET_MAXLEN - util_strlen(*pc));
                            } 
                            
                            VOS_MEM_FREE_BUF_AND_NULL_PTR(paramValue);
                        }
                    }
                }
            }
            else
            {
                skip = 0;
                vosLog_debug("CMC_phlGetParamAttrList skip default param");
            }

            paramAttr ++;
        }
#endif
    }
    else
    {
        vosLog_error("CMC_phlGetParamAttrList run failed!");
    }

    return ret;
}

int ctmdw_getAllITMSNotifications(char *pc)
{
    char   *paramValue = NULL;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char nextPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};
    UBOOL8 change = FALSE;
    UBOOL8 isExist;
    UBOOL8 isFirst = TRUE;
    int   cnt = 0;

    ret = CMC_phlIsPathExist("InternetGatewayDevice.", &isExist);

    if (isExist)
    {
        while (ret == VOS_RET_SUCCESS)
        {
            ret = CMC_phlGetNextPath(TRUE, FALSE, "InternetGatewayDevice.", nextPath, sizeof(nextPath));
            if (ret == VOS_RET_SUCCESS)
            {
                CMC_PHL_GET_PARAM_ATTR_T paramAttr;
                char tmpbuf[512];
                char tmpctname[512];
                VOS_RET_E ret1 = VOS_RET_SUCCESS;
                memset(tmpbuf, 0, 512);
                memset(tmpctname, 0, 512);
                memset(&paramAttr, 0, sizeof(CMC_PHL_GET_PARAM_ATTR_T));

                ret1 = CMC_phlGetParamAttr(nextPath, &paramAttr);
                if (ret1 == VOS_RET_SUCCESS && (paramAttr.notification & (PASSIVE_NOTIFICATION|ACTIVE_NOTIFICATION)) > NOTIFICATION_OFF)
                {
                    CMC_phlIsParamValueChanged(paramAttr.paramPath, &change);
                    if (TRUE == change)
                    {
                        ret = tr69c_getParamValue(paramAttr.paramPath, &paramValue);
                        if (ret == VOS_RET_SUCCESS)
                        {
                            if(0 == util_strcmp(paramAttr.paramPath, "InternetGatewayDevice.ManagementServer.ConnectionRequestURL"))
                            {
                                continue;
                            }
                            if ((paramValue!= NULL) && pc)
                            {
                                if (0 != mappingTR69NameToCTName(paramAttr.paramPath, tmpctname, sizeof(paramAttr.paramPath)))
                                {
                                    UTIL_STRNCPY(tmpctname, paramAttr.paramPath, BUFLEN_512);
                                }
                                if (isFirst)
                                {
                                    UTIL_SNPRINTF(pc, CTMDW_PACKET_MAXLEN, "&%s=%s", tmpctname, paramValue); 
                                    isFirst = FALSE;
                                }
                                else
                                {
                                    UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "&%s=%s", tmpctname, paramValue); 
                                    UTIL_STRNCAT(pc,tmpbuf, CTMDW_PACKET_MAXLEN);
                                }
                                
                                cnt ++;
                            } 

                        }
                    }
                }
            }
            else if (ret == VOS_RET_NO_MORE_INSTANCES)
            {
                ret = VOS_RET_SUCCESS;
                break;
            }
            else
            {
                vosLog_error("CMC_phlGetNextPath error, ret = %d", ret);
            }
        }
    }

    return ret;
}

static void ctmdw_writeGetPName(CMC_PHL_GET_PARAM_NAME_T *paramName, char **pc)
{
   int   writeable    = 0;
   int   makeFragName = 0;
   char *p = NULL;
   char *param = NULL; 
   char tmpbuf[512];
   char tmpctname[512];
   
   memset(tmpbuf, 0, 512);
   memset(tmpctname, 0, 512);

  if (paramName->paramPath[util_strlen(paramName->paramPath)-1] == '.')
  {
      paramName->paramPath[util_strlen(paramName->paramPath)-1] = '\0';
      makeFragName = 1;
  }
  
  param = p = paramName->paramPath;

  while((p = util_strstr(p, ".")) != NULL)
  {
      p++;
      param = p;
  }

   if (paramName->writable == 1)
   {
    writeable = 1;
   }
   
   UTIL_SNPRINTF(tmpbuf, sizeof(tmpbuf), "&%s%s=%s", param, (makeFragName)?".":"", writeable?"1":"0");
   if (pc)
   {
      UTIL_STRNCAT(*pc, tmpbuf, CTMDW_PACKET_MAXLEN+CTMDW_PACKET_MAXLEN+CTMDW_PACKET_MAXLEN);
   }
}  /* End of ctmdw_writeGetPName() */


/* 
* ctmdw_doGetParameterNames requests a single parameter path or single parameter path fragment 
*/
int ctmdw_doGetParameterNamesFromTR69(char *name, char *retbuf)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VOS_RET_E ret1 = VOS_RET_SUCCESS;
    const char *pp = name;
    UBOOL8 nextLevelOnly = TRUE;
    UBOOL8 pathIsEmpty = FALSE;
//    UBOOL8 isParamPath = FALSE;
    UBOOL8 isPathExists = FALSE;
    CMC_PHL_GET_PARAM_NAME_T paramName; 
//    UINT32 paramNum = 0;
    char nextPath[TR69C_PARAM_FULL_PATH_LENGTH] = {0};
    memset(&paramName, 0, sizeof(CMC_PHL_GET_PARAM_NAME_T));

    if (pp == NULL || util_strlen(pp) == 0)
    {
        ret = CMC_phlIsPathExist("InternetGatewayDevice.", &isPathExists);
        pathIsEmpty = TRUE;
    }
    else
    {
        ret = CMC_phlIsPathExist(pp, &isPathExists);
    }

    if ( ret != VOS_RET_SUCCESS)
    {
        vosLog_error("invalid param: %s!", pp);
        return -1;
    }

    ret = CMC_phlGetParamName(pp, &paramName);

    if (paramName.paramPath[util_strlen(paramName.paramPath) - 1] != '.')
    {
            ctmdw_writeGetPName(&paramName, (char **)&retbuf);
            ret = 1;
    }
    else
    {
        /* this is an object path */
        /* traverse the sub-tree below the object node */
        UBOOL8 firstParam = TRUE;

        while (ret1 == VOS_RET_SUCCESS)
        {
            ret1 = CMC_phlGetNextPath(FALSE, nextLevelOnly, pp, nextPath, sizeof(nextPath));

            if (ret1 == VOS_RET_SUCCESS)
            {
                ret1 = CMC_phlGetParamName(nextPath, &paramName);
                if (ret1 == VOS_RET_SUCCESS)
                {
                    if (!firstParam || !nextLevelOnly)
                    {
                        ctmdw_writeGetPName(&paramName, (char **)&retbuf);
                    }
                    else if (pathIsEmpty)
                    {
                        ctmdw_writeGetPName(&paramName, (char **)&retbuf);   
                    }
                    firstParam=FALSE;
                    ret = 1;
                }

                if (pathIsEmpty && nextLevelOnly)
                {
                    break;
                }
            }
            else if (ret1 == VOS_RET_NO_MORE_INSTANCES)
            {
                break;
            }
            else
            {
                vosLog_error("CMC_phlGetNextPath error! ret = %d", ret);
                return ret;
            }
        }
#if 0
        for (; i < paramNum; i++)
        {
            if (ret == VOS_RET_SUCCESS)
            {
                /* if nextLevelOnly is true, the first parameter name that matches
                the given partial path object name should NOT be included
                in the GetParameterNamesResponse */
                if (!firstParam || nextLevelOnly == FALSE)
                {
                    ctmdw_writeGetPName(paramName, (char **)&retbuf);                     
                }
                else if (pathIsEmpty && nextLevelOnly)
                {
                    /* However, for the special case where ACS does a GetParameterNames
                    with a blank name, then return the first name,
                    which is InternetGatewayDevice. */
                    ctmdw_writeGetPName(paramName, (char **)&retbuf);                     
                }

                ret = 1;
            }
            /* if ParameterPath is empty, with NextLevel is true, the response
            * should list only "InternetGatewayDevice.".
            */
            if (pathIsEmpty && nextLevelOnly)
            {
                /* out of while loop after write out the first object "InternetGatewayDevice."*/
                break; 
            }
        }
#endif
    }
    return ret;
}

int ctmdw_doAddObjectFromTR69(char *name, char **value)
{
    char *pp = name;
    int ret = -1;
    int len = 0;
    VOS_RET_E fault = VOS_RET_SUCCESS;;
    UINT32 instanceNum = 0;

    /* The path name must end with a "." (dot) after the last node
    * in the hierarchical name of the object.
    */
    if ((pp != NULL) && ((len = util_strlen(pp)) > 0) && (len <= 256) && (pp[len-1] == '.'))
    {
        char  *pLastToken;

        pp[len-1] = 0;
        if (((pLastToken = strrchr(pp, (int)'.')) != NULL) && (isalpha(*(++pLastToken))))
        {
            fault = VOS_RET_SUCCESS;
        }
        pp[len-1] = '.';
    }
    else
    {
        fault = VOS_RET_INVALID_PARAM_NAME;
    }

    if (fault == VOS_RET_SUCCESS)
    {
        fault = CMC_phlAddInstance(pp, &instanceNum);
        if (fault == VOS_RET_SUCCESS_REBOOT_REQUIRED)
        {
            ret = 2;
        }
        else if (fault == VOS_RET_SUCCESS)
        {
            ret = 1;
        }
        else if (fault == VOS_RET_INVALID_PARAM_NAME)
        {
            ret = -1;
        }

        if (ret != -1)
        {
            *value = VOS_STRDUP(itoa(instanceNum));
        }
    }
    
    return ret;
}

int ctmdw_doDeleteObjectFromTR69(char *name)
{
    char *pp = name;
    int  ret = -1;
    int len = 0;
    VOS_RET_E fault = VOS_RET_INVALID_PARAM_NAME;

    if ((pp != NULL) &&
    ((len = util_strlen(pp)) > 1) && (len <= 256) &&
    (pp[len-1] == '.') && (isdigit(pp[len-2])))
    {
        fault = VOS_RET_SUCCESS;
    }
    else
    {
        fault = VOS_RET_INVALID_PARAM_NAME;
    }

    if (fault == VOS_RET_SUCCESS)
    {
        fault = CMC_phlDelInstance(pp);
        if (fault == VOS_RET_SUCCESS_REBOOT_REQUIRED)
        {
            ret = 2;
        }
        else if (fault == VOS_RET_SUCCESS)
        {
            ret = 1;
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
}  /* End of doDeleteObject() */


