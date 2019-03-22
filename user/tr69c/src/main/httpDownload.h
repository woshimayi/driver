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
 * File Name  : httpDownload.h
 *
 * Description: download prototypes. 
 * $Revision: 1.2 $
 * $Id: httpDownload.h,v 1.2 2005/12/02 22:12:48 dmounday Exp $
 *----------------------------------------------------------------------*/

#include "../SOAPParser/RPCState.h"

void downloadStart(void *handle); 
void downloadStop(char *msg, int status);
void downloadDiagStop(char *msg, int status);
void uploadStop(char *msg, int status);
void updateDownLoadKey( DownloadReq *);
void downloadStop_nosendinform(char *msg, int status);
void uploadStart( void *handle); 
void downloaddiagStart( CMC_TR69C_DOWNLOAD_DIAG_CFG_T *handle);
void uploaddiagStart(CMC_TR69C_UPLOAD_DIAG_CFG_T *handle);
