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

#ifndef TR69C_DEFS_H
#define TR69C_DEFS_H

#include "fwk.h"
#include "util_tmr.h"
#include "util_tms.h"

typedef enum
{
    TRX_OK = 0,
    TRX_REBOOT,
    TRX_ERR, /* for internal error */
    TRX_INVALID_PARAMETER_VALUE
} TRX_STATUS;

typedef enum
{
    NOTIFICATION_OFF = 0,
    PASSIVE_NOTIFICATION = 1,
    ACTIVE_NOTIFICATION = 2,
    //ITMS_NOTIFICATION_READABLE = 1,
    //ITMS_REBOOT_NOTIFICATION_READABLE = 2,
    ITMS_NOTIFICATION_OFF_WRITABLE = 4,
    ITMS_NOTIFICATION_WRITABLE = 5,
    ITMS_REBOOT_NOTIFICATION_WRITABLE = 6,
    CTMDW_NOTIFICATION_READABLE = 8,
    CTMDW_ITMS_NOTIFICATION_READABLE = 9,
    CTMDW_REBOOT_ITMS_NOTIFICATION_READABLE = 10,
    CTMDW_NOTIFICATION_WRITABLE = 12,
    CTMDW_ITMS_NOTIFICATION_WRITABLE = 13,
    CTMDW_REBOOT_ITMS_NOTIFICATIONW_WRITABLE = 14
} eNotification;

/* inform event enum -- kind of inform msg */
typedef enum
{
    eIEBootStrap,
    eIEBoot,
    eIEPeriodix,
    eIEScheduled,
    eIEValueChanged,
    eIEKicked,
    eIEConnectionRequest,
    eIETransferComplete,
    eIEDiagnostics,
    eIEMethodResult,
    eIEXVendor
} eInformEvent;

typedef enum
{
    eACSNeverContacted = 0,
    eACSContacted,
    eACSInformed,
    eACSUpload,
    eACSDownloadReboot,
    eACSSetValueReboot,
    eACSAddObjectReboot,
    eACSDelObjectReboot,
    eACSRPCReboot
} eInformState;

#define NOREBOOT eACSNeverContacted

extern eInformState  informState;


/* TR-069 session enum
 *  eSessionStart - sending 1st Inform
 *  eSessionAuthentication - sending 2nd Inform
 *  eSessionDeliveryConfirm - receiving InformResponse
 *  eSessionEnd - receiving 204 No Content or 200 OK
 */
typedef enum
{
    eSessionUnknown,
    eSessionStart,
    eSessionAuthenticating,
    eSessionDeliveryConfirm,
    eSessionEnd
} eSessionState;

/*
* Define CPEVARNAMEINSTANCE in standardparams.c to create an
 * instance of all the CPE parameter strings.
 * undef VARINSTANCE to create a extern to the string pointer
 * If CPEVARNAMEINSTANCE is defined
 * SVAR(X) creates a char string constant of X and labels it with
 * the variable name X.
 * SSVAR(X,Y) creates a char string constant of Y and labels it with
 * the variable name X. This is used for strings that can't be C labels.
 *
 * If CPEVARNAMEINSTANCE is NOT defined SVAR generates
 * a extern of the form   extern const char X[];
*/
#ifdef CPEVARNAMEINSTANCE
    /*#define mkstr(S) # S  */
    #define SVAR(X) const char X[]=#X
    #define SSVAR(X,Y) const char X[]=#Y
#else
    #define SVAR(X) extern char X[]
    #define SSVAR(X,Y) extern char X[]
#endif

SSVAR(InternetGatewayDevice, InternetGatewayDevice.);
SSVAR(ManagementServer, ManagementServer.);
SSVAR(WANDevice, WANDevice.);
SSVAR(WANConnectionDevice, WANConnectionDevice.);
SVAR(URL);
SVAR(ConnectionRequestURL);
SVAR(Username);
SVAR(Password);
SVAR(PeriodicInformEnable);
SVAR(PeriodicInformInterval);
SVAR(ConnectionRequestUsername);
SVAR(ConnectionRequestPassword);

typedef enum
{
    rpcUnknown = 0,
    rpcGetRPCMethods,
    rpcSetParameterValues,
    rpcGetParameterValues,
    rpcGetParameterNames,
    rpcGetParameterAttributes,
    rpcSetParameterAttributes,
    rpcAddObject,
    rpcDeleteObject,
    rpcReboot,
    rpcDownload,
    rpcUpload,
    rpcGetQueuedTransfers,
    rpcScheduleInform,
    rpcFactoryReset,            /******** last rpc method ******/
    rpcInformResponse,          /* responses start here */
    rpcTransferCompleteResponse,
    rpcGetRPCMethodsResponse,
    rpcFault               /* soapenv:Fault response from ACS */
} eRPCMethods;
#define LAST_RPC_METHOD    rpcFactoryReset     /* see above enumeration */


/* must match eRPCMethods (enumeration of methods, see above) */
typedef struct RpcMethods
{
    unsigned   rpcGetRPCMethods: 1;
    unsigned   rpcSetParameterValues: 1;
    unsigned   rpcGetParameterValues: 1;
    unsigned   rpcGetParameterNames: 1;
    unsigned   rpcGetParameterAttributes: 1;
    unsigned   rpcSetParameterAttributes: 1;
    unsigned   rpcReboot: 1;
    unsigned   rpcDownload: 1;
    unsigned   rpcUpload: 1;
    unsigned   rpcGetQueuedTransfers: 1;
    unsigned   rpcFactoryReset: 1;
    unsigned   rpcScheduleInform: 1;
} RpcMethods;

typedef struct ACSState
{
    char        *acsURL;        /* URL of ACS */
    char        *acsUser;
    char        *acsPwd;
    time_t      informTime;     /* next ACS inform Time */
    time_t      informInterval; /* inform interval */
    int         informEnable;   /* True if inform to be performed*/
    int         randomInformEnable;   /* True if random inform to be performed*/
    int         maxEnvelopes;   /* Number of max env returned in inform response*/
    int         holdRequests;   /* hold request to ACS if true */
    int         noMoreRequests; /* don't send any more Req to ACS */
    RpcMethods  acsRpcMethods;  /* methods from GetRPCMethods response*/
    char        *parameterKey;  /* update key for ACS - may be NULL */
    char        *newParameterKey;  /* the pending key */
    char        *rebootCommandKey; /* key for reboot command key */
    char        *downloadCommandKey;    /* key for download cmd*/
    int         noneConnReqAuth;  /* no connection request authentication*/
    char        *boundIfName;     /* name of interface which tr69c works on, see description in data model */
    char        *connReqURL;
    char        *connReqIpAddr; /* IP address part of connReqURL -- part of the inform msg */
    char
    *connReqIpAddrFullPath; /* full path to the parameter that holds the connReqIpAddr, part of the inform msg */
    char        *connReqPath;   /* path part of connReqURL -- used by listener.c */
    char        *connReqUser;
    char        *connReqPwd;
    char        *kickURL;
    char        upgradesManaged;
    char        *provisioningCode;
    int         retryCount;     /* reset on each ACS response*/
    int         fault;          /* last operation fault code */
    int
    upgradeDownloadFlag;   /* 1:get download rpc and fileType is upgrade Image, 0:upgrade success and get InformResponce*/
    int         dlFaultStatus;  /* download fault status */
    char        *dlFaultMsg;    /* download fault message */
    time_t      startDLTime;    /* start download time */
    time_t      endDLTime;      /* complete download time*/
    char        *scheduleInformCommandKey;    /* key for scheduleInform*/
    char        *manufacturer;      /**< from deviceInfo */
    char        *manufacturerOUI;   /**< from deviceInfo */
    char        *productClass;      /**< from deviceInfo */
    char        *serialNumber;      /**< from deviceInfo */
    unsigned        enblChinaTelcomMDW;
    char        *MWSURL;
#if 1    /* zhangzhenhui */
    char        *deviceType;    /* < from deviceInfo*/
    char        *accessType;    /* < from deviceInfo*/
#endif    /* zhangzhenhui */
} ACSState;


typedef enum
{
    eFirmwareUpgrade = 1,
    eWebContent     = 2,
    eVendorConfig   = 3,
    eVendorLog      = 4
} eFileType;

typedef enum
{
    eTransferNotInitialized = 0, /* used internally */
    eTransferNotYetStarted = 1,
    eTransferInProgress    = 2,
    eTransferCompleted     = 3,
    eTransferRejected      = 4   /* used internally */
} eTransferState;
#define TRANSFER_QUEUE_SIZE 16

typedef struct DownloadReq
{
    eFileType efileType;
    char   *commandKey;
    char    *url;
    char    *user;
    char    *pwd;
    int     fileSize;
    char    *fileName;   /* ignore in this implementation- everything is in memory */
    int     delaySec;
    eTransferState state;
} DownloadReq;

typedef struct DownloadReqInfo
{
    eFileType   efileType;
    char        commandKey[33];
    char        url[256];
    char        user[256];
    char        pwd[256];
    int         fileSize;
    char        fileName[256];  /* ignore in this implementation- everything is in memory */
    int         delaySec;
    eTransferState state;
    eRPCMethods rpcMethod;
} DownloadReqInfo;

typedef struct TransferQInfo
{
    DownloadReq request;
    eRPCMethods rpcMethod;
} TransferQInfo;

typedef struct TransferInfo
{
    int mostRecentCompleteIndex;
    TransferQInfo queue[TRANSFER_QUEUE_SIZE];
} TransferInfo;

typedef struct LimitNotificationInfo
{
    char *parameterFullPathName; /* for example, IGD.ManagementServer.ManageableDeviceNumberOfEntries */
    int notificationPending; /* 1 = pending; 0 = nothing to send */
    int limitValue;          /* how often do we send active notification, in ms rather than Second */
    UtilEventHandler func;    /* notification timer function */
    UtilTimestamp lastSent;   /* the time stamp at which the last notification was sent */
    struct LimitNotificationInfo *next;
} LimitNotificationInfo;

typedef struct LimitNotificationQInfo
{
    int count;
    LimitNotificationInfo *limitEntry;
} LimitNotificationQInfo;

#define VENDOR_CFG_INFO_TOKEN     "vendorCfg"
typedef struct DownloadVendorConfigInfo
{
    char        name[BUFLEN_64];
    char        version[BUFLEN_16];
    char        date[BUFLEN_64];
    char        description[BUFLEN_256];
} DownloadVendorConfigInfo;

#define MAXINFORMEVENTS 27   /* should be match with number of eInformEvent */
typedef struct InformEvList
{
    eInformEvent     informEvList[MAXINFORMEVENTS];
    int              informEvCnt;   /* number of events in list */
    eRPCMethods      mMethod;      /* set if M <method> event required */
} InformEvList;

/* These definitions correspond to informEventStr array which
 * we use to map these integers value (saved in informEventList)
 * to string returned in inform message
*/
#define INFORM_EVENT_BOOTSTRAP                0
#define INFORM_EVENT_BOOT                     1
#define INFORM_EVENT_PERIODIC                 2
#define INFORM_EVENT_SCHEDULED                3
#define INFORM_EVENT_VALUE_CHANGE             4
#define INFORM_EVENT_KICKED                   5
#define INFORM_EVENT_CONNECTION_REQUEST       6
#define INFORM_EVENT_TRANSER_COMPLETE         7
#define INFORM_EVENT_DIAGNOSTICS_COMPLETE     8
#define INFORM_EVENT_REBOOT_METHOD            9
#define INFORM_EVENT_SCHEDULE_METHOD          10
#define INFORM_EVENT_DOWNLOAD_METHOD          11
#define INFORM_EVENT_UPLOAD_METHOD            12

#define INFORM_EVENT_CT_ALARM            13
#define INFORM_EVENT_CLEAR_CT_ALARM            17

#define INFORM_EVENT_CT_MONITOR            14

#define INFORM_EVENT_CT_USERINFO           15

#define INFORM_EVENT_CT_MAINTAIN           16


#define INFORM_EVENT_CT_CARDWRITE           18

#define INFORM_EVENT_LOID_CHANGE           19

#define INFORM_EVENT_NAME_CHANGE           20

#define INFORM_EVENT_CT_CARDNOTIFY         22
#define INFORM_ADMIN_PASSWD_CHANGED        23

#define INFORM_EVENT_CT_STB_BIND        24

#define INFORM_EVENT_SYSTEM_OFFLINE 25

#define INFORM_EVENT_CT_LONGRESET          26

#define INFORM_BRIDGE_USERNAME        27

#ifdef USE_DMALLOC
    #include "dmalloc.h"
#endif // USE_DMALLOC

#endif   // TR69C_DEFS_H
