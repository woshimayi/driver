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

#ifndef BCMWRAPPER_H
#define BCMWRAPPER_H
 
#include "fwk.h"
#include "cmc_api.h"
#include "../inc/tr69cdefs.h"


typedef struct gwStateData
{
   char     rebootCommandKey[33];
   char     downloadCommandKey[33];
   char     newParameterKey[33];
   int      contactedState;
   int      dlFaultStatus;
   int      upgradeDownloadFlag;
   int      retryCount;
   time_t   startDLTime;
   time_t   endDLTime;
   char     dlFaultMsg[129];
   int      informEvCount;
   char     informEvList[64];
   char     padforexpansion[324];             
} GWStateData, *PGWStateData;


typedef struct ParamPathList
{
	char* paramPath;
	char* value;
	struct ParamPathList* next;
} ParamPathList;


/* general system status and configruation retrieval */
/* Returns state of WAN interface to be used by tr69 client */
typedef enum
{
   eWAN_INACTIVE,
   eWAN_ACTIVE
} eWanState;

eWanState getWanState(void);
eWanState getRealWanState(const char *ifName);

void saveTR69StatusItems(void);
void retrieveTR69StatusItems(void);
void wrapperSaveConfigurations(void);
void wrapperReboot(eInformState rebootContactValue);
UBOOL8 wrapperFactoryReset(void);
void wrapperReset(void);
void setITMSUpdateFlag(SINT32 updateflag);
void upgradePopInfo(int itmsupgrage);

VOS_RET_E downloadComplete(DownloadReq *r, char *buf);
VOS_RET_E downloaddiagComplete(int size, char *buf);
VOS_RET_E downloadComplete_noreboot(DownloadReq *r, char *buf);
void setInformState(eInformState state);
VOS_RET_E tr69RetrieveTransferListFromStore(DownloadReqInfo *list, UINT16 *size);
VOS_RET_E tr69SaveTransferList(void);

/** Return the full MDM path to the specified IP addr.
 * Note objects searched are: WANIPConnection, WANPPPConnection, and
 * LANHostConfigManagement.IPInterface
 *
 * @param ipAddr (IN) The IP address to search for.
 * 
 * @return the full MDM path to the ipAddr, or NULL if the ipAddr was not found.
 */

VOS_RET_E setMSrvrURL(const char *value);
VOS_RET_E setMSrvrInformInterval(UINT32 interval);
VOS_RET_E setMSrvrInformEnable(UBOOL8 enable);
UINT32 getMdmParamValueChanges(void);
VOS_RET_E setMSrvrBoundIfName(const char *boundIfName);
void updateConnectionRequestInfo(void);
void updateConnectionAcsInfo(void);
void updateConnectionInfo(void);
VOS_RET_E tr69SaveConfigFileInfo(DownloadReq *r);
void retrieveClearTR69VendorConfigInfo(void);
void  addToParamPathList(char * path,void **list,char * value );
void freeParamPathList(void **aitem);


#endif /*BCMWRAPPER_H*/
