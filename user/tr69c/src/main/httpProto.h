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
 *----------------------------------------------------------------------*/

#ifndef HTTPPROTO_H
#define HTTPPROTO_H

#include "../SOAPParser/RPCState.h"
#include "../webproto/wget.h"


typedef enum
{
    sIdle,
    sAuthenticating,
    sAuthenticated,
    sAuthFailed,
    sShutdown
} AuthState, eACSConnState;

typedef enum
{
    eClosed,    /* connection is closed */
    eClose,     /* set Connection: close on next send */
    eStart,     /* connection is connecting */
    eConnected, /* connection has completed */
} HttpState;

typedef void (*CallBack)(AcsStatus);

typedef struct HttpTask
{
    /* current posted msg dope */
    HttpState      eHttpState;
    AuthState      eAuthState;
    tWgetInternal  *wio;
    char           *postMsg;
    int            postLth;
    int            content_len;
    AcsStatus      xfrStatus;
    CallBack       callback;
    char           *authHdr;
} HttpTask;

#endif /*HTTPPROTO_H*/
