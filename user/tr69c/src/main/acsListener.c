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
 * File Name  : acsListener.c
 *
 * Description: ACS listener: Listens for connections from ACS
 * $Revision: 1.11 $
 * $Id: acsListener.c,v 1.11 2006/01/31 23:11:06 dmounday Exp $
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
//#define _GNU_SOURCE
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
//#include <stdarg.h>
#include <errno.h>
//#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>

#include "cmc_api.h"
#include "../inc/tr69cdefs.h"       /* defines for ACS state */
#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include "../SOAPParser/RPCState.h"
#include "../SOAPParser/xmlTables.h"
#include "../webproto/protocol.h"
#include "../main/httpProto.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../bcmLibIF/bcmWrapper.h"
#include "event.h"
#include "informer.h"

#include "../ctMiddleware/ctMiddleware.h"

#include "locks.h"


typedef struct ACSConnection
{
    eACSConnState cState;  /* authentication state */
    tProtoCtx   *cpc;      /* so we can use wget proto functions */
    int         lfd;       /* listener socket */
    tHttpHdrs   hdrs;
    int         firstFd;
    struct ACSConnection *prev;
    struct ACSConnection *next;
} ACSConnection, *PACSConnection;


extern ACSState acsState;
extern UBOOL8 openConnReqServerSocket;
extern HttpTask  httpDownload;
extern HttpTask  httpUpload;
extern HttpTask  httpTask;      /* http io desc */

int connReqSessionNum = 0;

static ACSConnection connection;
static SessionAuth   acsConnSess;

static void startACScallback(void *handle);
void handleConnectionRequest(void);


static void free_http_headers(tHttpHdrs *hdrs)
{
    CookieHdr   *cp, *last;

    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->content_type);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->protocol);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->wwwAuthenticate);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->Authorization);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->TransferEncoding);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->Connection);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->method);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->path);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->host);
    cp = hdrs->setCookies;
    while (cp)
    {
        last = cp->next;
        VOS_MEM_FREE_BUF_AND_NULL_PTR(cp->name);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(cp->value);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(cp);
        cp = last;
    }

    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->message);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->locationHdr);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->filename);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(hdrs->arg);
    /* do not free(hdrs) since it's needed for Connection Request*/
    memset(hdrs, 0, sizeof(tHttpHdrs));
}


static int testChallenge(ACSConnection *cd)
{
    return (parseAuthorizationHdr(cd->hdrs.Authorization, &acsConnSess,
                                  acsState.connReqUser, acsState.connReqPwd));
}


/*1:busy, 0:not busy*/
int isTr069Busy()
{
    int ret  = 0;

    if ((httpDownload.eHttpState == eClosed)
            && (httpUpload.eHttpState == eClosed)
            && (httpTask.eHttpState == eClosed))
        ret = 0;
    else
        ret = 1;

    return ret;
}


void statusWriteComplete(void *handle)
{
    ACSConnection *cd = (ACSConnection *)handle;
    ACSConnection *prev, *next;
    UINT32 delayMs = 2000;  /* may want smarter time value here ????*/
    VOS_RET_E ret = VOS_RET_SUCCESS;

    free_http_headers(&cd->hdrs);
    proto_FreeCtx(cd->cpc);

    /*
    * The orig code was:
    * memset(cd, 0, sizeof(struct ACSConnection));
    * but that wipes out cd->lfd, which we still want to keep open.
    * When we call startACScallback from the 2 sec. timer, we will
    * start listening to that fd again.
    */
    cd->cState = sIdle;
    cd->cpc = NULL;
    memset(&(cd->hdrs), 0, sizeof(tHttpHdrs));

    /*added by polo*/
    connReqSessionNum = 0;

    if (cd->prev)
    {
        prev = cd->prev;
        next = cd->next;
        prev->next = next;
        if (next)
            next->prev = prev;
    }
    else
    {
        /*that's impossible run here since connection is static memory allocated for cd*/
    }

    memset(cd, 0, sizeof(struct ACSConnection));
    VOS_FREE(cd);

    /*
    * mwang: Why do we wait a whole 2 seconds before we accept another connection?
    */
    ret = utilTmr_set(tmrHandle, startACScallback, NULL, delayMs, "wrt_comp_startACScallback");
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_debug("could not set startACSCallback timer, ret=%d", ret);
    }

    return;
}

void statusWrite503Complete(void *handle)
{
    ACSConnection *prev, *next;

    ACSConnection *cd = (ACSConnection *)handle;
    free_http_headers(&cd->hdrs);
    proto_FreeCtx(cd->cpc);

    /*added by polo*/
    if (cd->prev)
    {
        prev = cd->prev;
        next = cd->next;
        prev->next = next;
        if (next)
            next->prev = prev;
    }
    else
    {
        vosLog_error("This is a impossible thing !");
        /*that's impossible run here since connection is static memory allocated for cd*/
    }

    memset(cd, 0, sizeof(struct ACSConnection));
    VOS_FREE(cd);
}

/*added by polo*/
static int send503(ACSConnection *cd)
{
    char    response[300];
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 503 Service Unavailable\r\n");
    i += snprintf(response + i, sizeof(response) - i, "Content-Length: 0\r\n\r\n");
    if (proto_Writen(cd->cpc, response, i) < i)
        return 0;

    return 1;
}

static int sendOK(ACSConnection *cd)
{
    char    response[300];
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n");
    i += snprintf(response + i, sizeof(response) - i, "Content-Length: 0\r\n\r\n");

    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
    {
        CMC_tr69cSetRemoteInform(CMC_TR69C_CONN_BY_ACS_SUCCESS, CMC_TR69C_ACS_CONNECTED_STATUS); //lixuefei 20090722
    }

    if (proto_Writen(cd->cpc, response, i) < i)
    {
        return 0;
    }

    return 1;
}

static int sendAuthFailed(ACSConnection *cd)
{
    char    response[300];
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 401 Unauthorized\r\n");
    i += snprintf(response + i, sizeof(response) - i, "Content-Length: 0\r\n\r\n");

    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
    {
        CMC_tr69cSetRemoteInform(CMC_TR69C_CONN_BY_ACS_FAIL, CMC_TR69C_ACS_CONNECTED_STATUS); //lixuefei 20090722
    }

    if (proto_Writen(cd->cpc, response, i) < i)
    {
        return 0;
    }

    return 1;
}

static int sendChallenge(ACSConnection *cd)
{
    char    response[300];
    char    *h;
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 401 Unauthorized\r\n");
    i += snprintf(response + i, sizeof(response) - i, "Content-Length: 0\r\n");
    h = generateWWWAuthenticateHdr(&acsConnSess, ACSREALM, ACSDOMAIN, cd->hdrs.method);
    i += snprintf(response + i, sizeof(response) - i, "%s\r\n\r\n", h);
    VOS_MEM_FREE_BUF_AND_NULL_PTR(h);

    if (proto_Writen(cd->cpc, response, i) < i)
    {
        return 0;
    }

    return 1;
}

/**
 * A connected ACS is sending us data,
 * Our action is to generate a digest authentication challange
 * with a 401 Unauthorized status code and
 * wait for the response to the challange. Then  send a
 * 200 OK or a 401 Unauthorized. */
static void acsReadData(void *handle)
{
    ACSConnection *cd = (ACSConnection *)handle;

    vosLog_debug("acsReadData");

    if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
    {
        CMC_tr69cSetRemoteInform(CMC_TR69C_CONN_BY_ACS_INTERRUPT, CMC_TR69C_ACS_CONNECTED_STATUS);
    }

    /* Free resources allocated earlier */
    free_http_headers(&cd->hdrs);

    /* Do not need to call updateConnectionRequestInfo() anymore since this information
      * is updated automatically by using event mechanism so that
      *when Management Sever or Tr69cConfig object is changed, event is sent to tr69c.
      *updateConnectionRequestInfo();
      */
    if (0 == proto_ParseRequest(cd->cpc, &cd->hdrs))
    {
        proto_ParseHdrs(cd->cpc, &cd->hdrs);
        if (!util_strcasecmp("http/1.1", cd->hdrs.protocol))
        {
            /* protocol is correct */
            if (!util_strcmp(acsState.connReqPath, cd->hdrs.path))//fangzhen
            {
                if (cd->firstFd != cd->cpc->fd)
                {
                    if (isTr069Busy())
                    {
                        vosLog_debug("Tr069 is busy send 503");
                        send503(cd);
                        statusWrite503Complete(cd);
                        return;
                    }
                }

                /* path is correct proceed with authorization */
                if (acsState.connReqUser == NULL || acsState.connReqUser[0] == '\0')
                {
                    cd->cState = sAuthenticated;
                }

                vosLog_debug("cd->cState = %d", cd->cState);
                if (sIdle == cd->cState)
                {
                    if (acsState.noneConnReqAuth)
                    {
                        if (cd->firstFd != cd->cpc->fd && isTr069Busy())
                        {
                            vosLog_debug("firstfd and cpc->fd is different");
                            send503(cd);
                            statusWrite503Complete(cd);
                            return;
                        }
                        else
                        {
                            vosLog_debug("cd->cState = %s", (cd->cState == sAuthenticated) ? "sAuthenticated" : "");
                            cd->cState = sAuthenticated;
                            sendOK(cd);
                            handleConnectionRequest();
                            cd->cState = sShutdown;
                            setListenerType(cd->cpc->fd, statusWriteComplete, cd, iListener_Write);
                            return;
                        }
                    }
                    else
                    {
                        if (cd->firstFd != cd->cpc->fd && isTr069Busy())
                        {
                            vosLog_debug("firstfd and cpc->fd is different");
                            send503(cd);
                            statusWrite503Complete(cd);
                            return;
                        }
                        else
                        {
                            if (SF_FEATURE_CUSTOMER_3BB)
                            {
                                if (testChallenge(cd))
                                {
                                    sendOK(cd);
                                    /* avoid race condition between periodic inform and connection request inform */
                                    resetPeriodicInform(acsState.informInterval);
                                    handleConnectionRequest();
                                    cd->cState = sShutdown;
                                }
                                else
                                {
                                    vosLog_debug("Verify the authentication information");
                                    sendChallenge(cd);
                                    cd->cState = sShutdown;
                                }
                                setListenerType(cd->cpc->fd, statusWriteComplete, cd, iListener_Write);
                                return;
                            }
                            else
                            {
                                /* send 401 with digest challange */
                                sendChallenge(cd);
                                cd->cState = sAuthenticating;
                                setListener(cd->cpc->fd, acsReadData, (void *)cd);
                                return;
                            }
                        }
                    }
                }
                else if (sAuthenticating == cd->cState)
                {
                    if (testChallenge(cd))
                    {
                        vosLog_debug("Verify OK");
                        sendOK(cd);
                        /* avoid race condition between periodic inform and connection request inform */
                        resetPeriodicInform(acsState.informInterval);
                        handleConnectionRequest();
                        cd->cState = sShutdown;
                    }
                    else
                    {
                        vosLog_debug("ConnectRequest authentication error 401");
                        sendAuthFailed(cd);
                        cd->cState = sShutdown;
                    }

                    setListenerType(cd->cpc->fd, statusWriteComplete, cd, iListener_Write);
                    return;
                }
                else if (sAuthenticated == cd->cState)
                {
                    vosLog_debug("send 200 OK");
                    sendOK(cd);
                    /* avoid race condition between periodic inform and connection request inform */
                    resetPeriodicInform(acsState.informInterval);
                    handleConnectionRequest();
                    cd->cState = sShutdown;
                    setListenerType(cd->cpc->fd, statusWriteComplete, cd, iListener_Write);
                    return;
                }
                else
                {
                    vosLog_error("cd->cState = %u", cd->cState);
                }
            }
            else
            {
                vosLog_error("acsState.connReqPath = %s",
                             acsState.connReqPath ? acsState.connReqPath : "");
            }
        }
        else
        {
            vosLog_error("cd->hdrs.protocol = %s", cd->hdrs.protocol ? cd->hdrs.protocol : "");
        }
    }
    else
    {
        vosLog_debug("acsListener Error reading response");
    }

    cd->cState = sShutdown;
    vosLog_debug("cd->cState = %s", sShutdown == cd->cState ? "sShutdown" : "");
    setListenerType(cd->cpc->fd, statusWriteComplete, cd, iListener_Write);
}


/** ACS is trying to connect to us.
 *
 * Accept the connection, add the new fd to the listeners list.
 */
static void acsConnect(void *handle)
{
    struct sockaddr_in addr;
    ACSConnection      *cd = (ACSConnection *) handle;
    ACSConnection *ncd = NULL;
    ACSConnection *prev, *next;
    prev = next = NULL;
    socklen_t  sz  = sizeof(struct sockaddr_in);
    int flags = 1;
    int fd;
    int res;
    const UINT32 delayMs = 5000; /* reenable listen in 5 sec */
    VOS_RET_E ret;

    vosLog_debug("cd->lfd = %d", cd->lfd);

#if 0
    stopListener(cd->lfd);
#endif

    memset(&addr, 0, sz);
    if ((fd = accept(cd->lfd, (struct sockaddr *)&addr, &sz)) < 0)
    {
        vosLog_error("acsListen accept failed errno=%d (%s) fd=%d", errno, strerror(errno), cd->lfd);
        ret = utilTmr_set(tmrHandle, startACScallback, NULL, delayMs, "conn_err_startACScallback");
        if (ret != VOS_RET_SUCCESS)
        {
            vosLog_error("could not set startACSCallback timer, ret=%d", ret);
        }

        return;
    }
    else
    {
        vosLog_debug("accepted conn from %s on fd=%d", inet_ntoa(addr.sin_addr), fd);
    }

    ncd = VOS_MALLOC(sizeof(ACSConnection));
    if (NULL == ncd)
        return;

    memset((void *)ncd, 0, sizeof(ACSConnection));

    ncd->cState = cd->cState;
    ncd->lfd = cd->lfd;
    ncd->firstFd = cd->firstFd;

    if (connReqSessionNum <= 0)
    {
        cd->firstFd = fd;
        ncd->firstFd = cd->firstFd;
        connReqSessionNum++;
    }

    prev = cd;
    next = cd;

    while (next)
    {
        prev = next;
        next = next->next;
    }

    prev->next = ncd;
    ncd->prev = prev;
    ncd->next = NULL;

    /*
     * Orig code:
     * close(cd->lfd);
     * cd->lfd = 0;
     *
     * In CMS, tr69c will not close the listener socket fd after it accepts a
     * connection.  Instead, stopListener call above will effectively cause tr69c
     * to stop servicing any new connection requests.
     * After this transaction is done, statusWriteComplete() will schedule a
     * delayed timer call, which will call startACSCallback, which will call
     * setListener() on the listener socket fd again.  We don't close the
     * listener socket in tr69c because smd also has a reference to the socket.
     */

    ncd->cpc = proto_NewCtx(fd);
    if ((res = setsockopt(ncd->cpc->fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags))) < 0)
        vosLog_error("proxy_connection() setsockopt error %d %d %s", cd->cpc->fd, errno, strerror(errno));

    setListener(ncd->cpc->fd, acsReadData, ncd);
}

/*
* return -1: for error
*       != -1 is socket
*/
static int initSocket(unsigned int ip, int port)
{
    struct sockaddr_in addr;
    int port_sock = 0;
    int res, i = 1;

    port_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (port_sock < 0)
    {
        vosLog_error("web: init_listen_socket(port=%d), socket failed", port);
        return -1;
    }

    res = setsockopt(port_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &i, sizeof(i));
    if (res < 0)
    {
        vosLog_error("web: %s", "Socket error listening to ACS");
        close(port_sock);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family       = AF_INET;
    addr.sin_addr.s_addr  = htonl(ip);
    addr.sin_port         = htons(port);

    res = bind(port_sock, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0)
    {
        vosLog_error("acsListener bind failed errno=%d.%s", errno, strerror(errno));
        close(port_sock);
        return -1;
    }

    res = listen(port_sock, 1);
    if (res < 0)
    {
        vosLog_error("acsListener listent failed errno=%d.%s", errno, strerror(errno));
        close(port_sock);
        return -1;
    }

    return port_sock;
}

static void startACScallback(void *handle __attribute__((unused)))
{
    ACSConnection *cd = &connection;

    connReqSessionNum = 0;

    vosLog_debug("openConnReqServerSocket=%d cd->lfd=%d",
                 openConnReqServerSocket, cd->lfd);

    if (openConnReqServerSocket &&
            SF_AND_IF(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
            (enblCTMiddleware != CTMDW_MODE_0)
            SF_AND_ENDIF)
    {
        /* used during unittests, tr69c started without smd, needs to open its
         * own connection request server socket. */
        if (UTIL_INVALID_FD == cd->lfd)
        {

            if (SF_FEATURE_CUSTOMER_3BB)
            {
                CMC_TR69C_ADVANCE_CFG_T acsCfg;
                VOS_RET_E ret = VOS_RET_SUCCESS;
                if ((ret = CMC_tr69cGetManagementServer(&acsCfg)) != VOS_RET_SUCCESS)
                {
                    vosLog_error("get of MDMOID_MANAGEMENT_SERVER failed, ret = %d", ret);
                }

                if (UTIL_INVALID_FD == (cd->lfd = initSocket(INADDR_ANY, acsCfg.connectionRequestPort)))
                {
                    vosLog_error("could not open server socket");
                }
                else
                {
                    vosLog_debug("ACS connection req server socket opened at %d", cd->lfd);
                }

            }
            else
            {
                if (UTIL_INVALID_FD == (cd->lfd = initSocket(INADDR_ANY, TR69C_CONN_REQ_PORT)))
                {
                    /* the original code had this:
                     * utilTmr_set(&timers, startACScallback, NULL, 5000 );  retry init_socket in 5 sec
                     * But why should initSocket fail anyways?
                     * Take it out of this code path because this is for unittests only.
                     * If there really is a problem, we should root cause it (or at least
                     * apply this hack to smd, which is responsible for opening the
                     * server socket.)
                     */
                    vosLog_error("could not open server socket");
                }
                else
                {
                    vosLog_debug("ACS connection req server socket opened at %d", cd->lfd);
                }
            }
        }
    }
    else
    {
        /* typically, I will inherit my server socket from smd */
        cd->lfd = VOS_DYNAMIC_LAUNCH_SERVER_FD;
        vosLog_debug("ACS connection req server socket inherited at %d", cd->lfd);
    }

    setListener(cd->lfd, acsConnect, cd);
}  /* End of startACScallback() */


/*
* Listen for connections from the ACS
*/
void startACSListener(void)
{
    vosLog_debug("=====>ENTER");

    memset(&connection, 0, sizeof(struct ACSConnection));
    connection.lfd = UTIL_INVALID_FD;

    startACScallback(NULL);
}

void handleConnectionRequest(void)
{
    vosLog_debug("Enter>");

    /* avoid race condition between periodic inform and connection request inform */
    resetPeriodicInform(acsState.informInterval);

    addInformEventToList(INFORM_EVENT_CONNECTION_REQUEST);

    /* connection request should be handled right away.
    * When test with Cisco ACS simulator which is not always in the mode
    * to accept inform from CPE.  CPE is sitting in session retry algorithm
    * when the retry interval is getting bigger and bigger.
    * utilTmr_set doesn't allow same routine being added.
    * So, we should cancel the current retry and execute this one right away.
    */
    utilTmr_cancel(tmrHandle, sendInform, NULL);

    utilTmr_set(tmrHandle, sendInform, NULL, 0, "acs_conn_sendInform");
}

