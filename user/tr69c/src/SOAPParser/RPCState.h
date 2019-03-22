#ifndef RPCSTATE_H
#define RPCSTATE_H
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
 * File Name  : RPCState.h
 *
 * Description: RPC states and data structures 
 * $Revision: 1.23 $
 * $Id: RPCState.h,v 1.23 2006/01/30 23:01:25 dmounday Exp $
 *----------------------------------------------------------------------*/
 
#include "../inc/tr69cdefs.h"
#include "../webproto/protocol.h"
#include "cmc_api.h"
#include "phl.h"
 
/* fault codes */
#define NO_FAULT    0
#define RPC_PENDING 1
#define RPC_FAULTS  9000

/* status enum for acs connection and msg xfer */
typedef enum {
   eOK,
   eConnectError,
   eGetError,
   ePostError,
   eAuthError,
   eDownloadDone,
   eUploadDone,
   eAcsDone
}AcsStatus;

/* rpcRun return status */
typedef enum {
   eRPCRunOK,      /* sent RPC response to ACS */
   eRPCRunEnd,      /* sent NULL http msg to ACS */
   eRPCRunFail      /* RPC run failed no reponse to send */
               /*  this should probably send a fault */
}RunRPCStatus;


/* move informEvList to tr69cdefs.h */
#if 0
typedef struct ParamItem {
   struct ParamItem   *next;
   char   *pname;
   char   *pvalue;
   char   *pOrigValue;
   int      fault;      /* 0 or set fault code */
}ParamItem;
#endif
typedef struct AttributeItem {
   struct AttributeItem *next;
   char   *pname;
   int    notification;
   int    chgNotify;
   int    chgAccess;
   int    subAccess;   /* need to add list here if spec changes or vendor reqmts */
} AttributeItem;

typedef struct ParamNamesReq {
   char    *parameterPath;
   int     nextLevel;
} ParamNamesReq;

typedef struct AddDelObject {
   char    *objectName;    /* For Add the object name is xxx.yyy. */
                            /* for Delete the object anme is xxx.yyy.i. */
                            /* where i is the instance number */
} AddDelObject;

typedef struct RPCAction {
   char   *ID;    /* pointer to ID string */
   char    *informID;  /* ID string sent with last inform */
   eRPCMethods rpcMethod;
   int     arrayItemCnt;   /* cnt of items in parameter list-not used */
   char    *commandKey;    /* */
   char    *parameterKey;  /* for setParameterValue key */
   union {
        ParamItem       *pItem;
        AttributeItem   *aItem;
        ParamNamesReq   paramNamesReq;
        AddDelObject    addDelObjectReq;
        DownloadReq     downloadReq;
        /* more items here later for each rpc method*/
   } ud;
} RPCAction;

/* structures to save notification entries */
typedef struct AttEntry {
   short int   nodeIndex;         /* index in CPE param table */
   short int   attrValue;         /* attribute value (1..2)  0 is not saved*/
   int         instanceId;         /* Id of instance or zero */
} AttEntry;

typedef struct AttSaveBuf {
   short int   sigValue;
   short int   numAttSaved;      /* number of notification attributes saved*/
   AttEntry    attEntry[];
} AttSaveBuf;


RPCAction* newRPCAction(void);
void freeRPCAction(RPCAction *item);

void dumpAcsState(void);
void dumpRpcAction(RPCAction *);
void buildInform(RPCAction *a, InformEvList *);
void updateKeys( RPCAction *a);
RunRPCStatus runRPC(void);
int  checkActiveNotifications(void);
void resetNotification(void);
void saveConfigurations(void);
UBOOL8 rebootCompletion(void);
UBOOL8 factoryResetCompletion(void);
void sendTransferComplete(void);
void sendGetRPCMethods(void);
void initTransferList(void);
int isTransferInProgress(void);
int requestQueued(DownloadReq *r, eRPCMethods method);
void transferListDequeueFree(DownloadReq *q);
void transferListEnqueueCopy(DownloadReq *q, DownloadReq *r);
void updateTransferState(char *commandKey, eTransferState state);
void setDefaultActiveNotification(void);
VOS_RET_E freeGetParamValueBuf(CMC_PHL_GET_PARAM_VALUE_T *buf, SINT32 numEntries);

VOS_RET_E tr69c_getParamValue(const char *fullPath, char **paramValue);

void ctmdw_resetNotification(UINT8 cleanflag);
int ctmdw_getAllNotifications(char **pc);
int ctmdw_checkActiveNotifications(void);
int ctmdw_doGetParameterNamesFromTR69(char *name, char *retbuf);
int ctmdw_doAddObjectFromTR69(char *name, char **value);
int ctmdw_doDeleteObjectFromTR69(char *name);
int getsysRunTime(void);
int getTime(void);

void TR69C_writePathAndUintValue(char *path, UINT32 value, tProtoCtx *pc, int *bufsz, int *paramNum);

void tr69c_GetMemOccupyInfo(char *memInfo, UINT32 len);
void tr69c_GetCpuOccupyInfo(char *cpuInfo, UINT32 len);


#endif
