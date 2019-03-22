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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <ctype.h>
#include <syslog.h>

#include "fwk.h"
#include "../inc/appdefs.h"
#include "../inc/utils.h"
#include <openssl/ssl.h>
#include "../main/event.h"
#include "../main/informer.h"

#include "protocol.h"
#include "www.h"
#include "wget.h"
#include "../main/httpProto.h"
#include "../bcmLibIF/bcmWrapper.h"


#define BUF_SIZE 1024
//#define DEBUG

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
#ifdef  DEBUG
    #define mkstr(S) # S
    #define setListener(A,B,C) {fprintf(stderr,mkstr(%s setListener B fd=%d\n), getticks(), A);\
setListener( A,B,C);}

    #define setListenerType(A,B,C,E) {fprintf(stderr,mkstr(%s setListenerType B-E fd=%d\n), getticks(), A);\
setListenerType( A,B,C,E);}

    #define stopListener(A) {fprintf(stderr,"%s stopListener fd=%d\n", getticks(), A);\
stopListener( A );}

static char timestr[40];
static char *getticks()
{
    struct timeval now;
    gettimeofday( &now,NULL);
    sprintf(timestr,"%04ld.%06ld", now.tv_sec%1000, now.tv_usec);
    return timestr;
}
#endif


typedef enum {
    REPLACE,
    ADDTOLIST
} eHdrOp;


/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/*----------------------------------------------------------------------*
 * forwards
 *----------------------------------------------------------------------*/
static char noHostConnectMsg[] = "Could not establish connection to host %s(%s):%d";
static char noHostResolve[] = "Could not resolve host %s";
static char lastErrorMsg[255];
int g_dns_resolve_ret = 1; 

extern RPCAction *simRpcAction;

static void timer_connect(void *p);
static void do_connect(void *p);
static void do_connect_ssl(void *p, int errorcode);
static void timer_response(void *p);
static void do_response(void *p);
static void do_send_request(void *p, int errorcode);


static void freeHdrs( XtraPostHdr **p )
{
    XtraPostHdr *next = *p;
        while( next ){
            XtraPostHdr *temp;
            temp = next->next;
            VOS_FREE(next->hdr);
            VOS_FREE(next->value);
            VOS_FREE(next);
            next = temp;
        }
}

static void freeCookies( CookieHdr **p )
{
    CookieHdr *next = *p;
    while( next ){
        CookieHdr *temp;
        temp = next->next;
        VOS_FREE(next->name);
        VOS_FREE(next->value);
        VOS_FREE(next);
        next = temp;
    }
}


/*----------------------------------------------------------------------*/
void freeData(tWgetInternal *wg)
{
    if (wg != NULL) 
    {
        if (wg->pc != NULL) 
        {
            proto_FreeCtx(wg->pc);
            wg->pc = NULL;
        }

        VOS_FREE(wg->proto);
        VOS_FREE(wg->host);
        VOS_FREE(wg->uri);

        if (wg->hdrs != NULL) 
        {
            proto_FreeHttpHdrs(wg->hdrs);
        }
        
        freeCookies( &wg->cookieHdrs );
        freeHdrs( &wg->xtraPostHdrs );
        // don't free content_type and postdata since they're only pointers, there are no strdup for them.
        VOS_FREE(wg);
    }
}


static int addCookieHdr( CookieHdr **hdrQp, char *cookieName, char *value, eHdrOp replaceDups)
{
    CookieHdr *xh;
    CookieHdr **p = hdrQp;
    xh= (CookieHdr *)VOS_MALLOC_FLAGS(sizeof(struct CookieHdr), ALLOC_ZEROIZE);
    if (xh) {
        xh->name = VOS_STRDUP(cookieName);
        xh->value = VOS_STRDUP(value);
        if (replaceDups == REPLACE) {
            while (*p) {
                CookieHdr *xp = *p;
                if ( strcasecmp(xp->name, xh->name)==0) {
                    /* replace header */
                    xh->next = xp->next;
                    VOS_FREE(xp->name);
                    VOS_FREE(xp->value);
                    VOS_FREE(xp);
                    *p = xh;
                    return 1;
                }
                p = &xp->next;
            }
        }
        /* just stick it at beginning of list */
        xh->next = *hdrQp;
        *hdrQp = xh;
        return 1;
    }
    return 0;
}


static int addPostHdr( XtraPostHdr **hdrQp, char *xhdrname, char *value, eHdrOp replaceDups)
{
    XtraPostHdr *xh;
    XtraPostHdr **p = hdrQp;

    xh= (XtraPostHdr *)VOS_MALLOC_FLAGS(sizeof(struct XtraPostHdr), ALLOC_ZEROIZE);
    if (xh) 
    {
        xh->hdr = VOS_STRDUP(xhdrname);
        xh->value = VOS_STRDUP(value);
        if (replaceDups == REPLACE) 
        {
            while (*p) 
            {
                XtraPostHdr *xp = *p;
                if (strcmp(xp->hdr, xh->hdr) == 0) 
                {
                    /* replace header */
                    xh->next = xp->next;
                    VOS_FREE(xp->hdr);
                    VOS_FREE(xp->value);
                    VOS_FREE(xp);
                    *p = xh;
                    return 1;
                }
                
                p = &xp->next;
            }
        }
        
        /* just stick it at beginning of list */
        xh->next = *hdrQp;
        *hdrQp = xh;
        return 1;
    }
    
    return 0;
}

/*----------------------------------------------------------------------*/
static void report_status(tWgetInternal *data, tWgetStatus status, 
                          const char *msg)
{
    tWget wg;

    vosLog_debug("Enter>, data = %p", data);
    
    /* internal error, call callback */
    wg.status = status;
    wg.pc = data->pc;
    wg.hdrs = data->hdrs;
    wg.msg = msg;
    wg.handle = data->handle;
    data->cbActive = 1;
    (*data->cb)(&wg);
    data->cbActive = 0;
    
    if (data->keepConnection == eCloseConnection && data->status != -9)
    {
        freeData(data);
    }
}

/*----------------------------------------------------------------------*
 * returns
 *   0 if ok
 *  -1 if WAN interface is not active
 */
static int send_get_request(tWgetInternal *p,
                            const char *host,
                            int port __attribute__((unused)),
                            const char *uri)
{
    tProtoCtx   *pc = p->pc;
    XtraPostHdr *next;

    vosLog_debug("Enter>");

    if (getWanState() == eWAN_INACTIVE)
    {
        return -1;
    }

    proto_SendRequest(pc, "GET", uri);
    proto_SendHeader(pc,  "Host", host);
    if (SF_FEATURE_LOCATION_SHANGHAI)
    {
        proto_SendHeader(pc,  "User-Agent", "ASB;RG2010-CA;RG201O-CA_V1A2;E201OCAA2V10S;E8C-E042");
    }
    else
    {
        proto_SendHeader(pc,  "User-Agent", USER_AGENT_NAME);
    }
    
    if (p->keepConnection==eCloseConnection)
    {
        proto_SendHeader(pc,  "Connection", "close");
    }
    else
    {
        proto_SendHeader(pc,  "Connection", "keep-alive");
    }
   
    next = p->xtraPostHdrs;
    while (next)
    {
        proto_SendHeader(pc,next->hdr, next->value);
        next = next->next;
    }

    proto_SendEndHeaders(pc);
    return 0;

}  /* End of send_get_request() */


static int send_request(tWgetInternal *p, const char *host, int port, const char *data,
                        int datalen, const char *content_type, int content_len)
{
    tProtoCtx   *pc = p->pc;
    char        buf[BUF_SIZE] = {0};
    XtraPostHdr *next;
    CookieHdr   *cookie;

    vosLog_debug("Enter>");
    
    UTIL_SNPRINTF(buf, sizeof(buf), "%s:%d", host, port);
    proto_SendHeader(pc,  "Host", buf);
    if (SF_FEATURE_LOCATION_SHANGHAI)
    {
        proto_SendHeader(pc,  "User-Agent", "ASB;RG2010-CA;RG201O-CA_V1A2;E201OCAA2V10S;E8C-E042");
    }
    else
    {
        proto_SendHeader(pc,  "User-Agent", USER_AGENT_NAME);
    }

    
    if (p->keepConnection == eCloseConnection)
    {
        proto_SendHeader(pc,  "Connection", "close");
    }
    else
    {
        proto_SendHeader(pc,  "Connection", "keep-alive");
    }
   
    next = p->xtraPostHdrs;
    while (next)
    {
        proto_SendHeader(pc,next->hdr, next->value);
        next = next->next;
    }
   
    cookie = p->cookieHdrs;
    while(cookie)
    {
        proto_SendCookie(pc, cookie);
        cookie = cookie->next;
    }
   
    proto_SendHeader(pc,  "Content-Type", content_type);
    UTIL_SNPRINTF(buf, sizeof(buf), "%d", content_len);
    proto_SendHeader(pc,  "Content-Length", buf);

    proto_SendEndHeaders(pc);
    if (data && datalen)
    {
        proto_SendRaw(pc, data, datalen);
    }

    return 0;
}  /* End of send_request() */


/*----------------------------------------------------------------------*
 * returns
 *   0 if ok
 *  -1 if WAN interface is not active
 *  arg_keys is a NULL terminated array of (char *)
 *  arg_values is a NULL terminated array of (char *) of same length as arg_keys
 */
static int send_post_request(tWgetInternal *p, const char *host, int port, const char *uri,
                             const char *data, int datalen, const char *content_type,
                             int content_len)
{
    tProtoCtx *pc = p->pc;

    if (getWanState() == eWAN_INACTIVE)
    {
        return -1;
    }

    proto_SendRequest(pc, "POST", uri);

    return send_request(p, host, port, data, datalen, content_type, content_len);
}


/*----------------------------------------------------------------------*
 * returns
 *   0 if ok
 *  -1 if WAN interface is not active
 *  arg_keys is a NULL terminated array of (char *)
 *  arg_values is a NULL terminated array of (char *) of same length as arg_keys
 */
static int send_put_request(tWgetInternal *p, const char *host, int port, const char *uri,
                            const char *data, int datalen, const char *content_type,
                            int content_len)
{
    tProtoCtx *pc = p->pc;

    if (getWanState()== eWAN_INACTIVE)
    {
        return -1;
    }

    proto_SendRequest(pc, "PUT", uri);

    return send_request(p, host, port, data, datalen, content_type, content_len);
}

/*----------------------------------------------------------------------
 * connect timeout
 */
static void timer_connect(void *p)
{
    tWgetInternal *data = (tWgetInternal *) p;
    char          buf[256] = {0};

    stopListener(data->pc->fd);

    UTIL_SNPRINTF(buf, sizeof(buf), "Connection timed out to host %s:%d", data->host, data->port);
    report_status(data, iWgetStatus_ConnectionError, buf);
}


/*----------------------------------------------------------------------*/
static void timer_response(void *p)
{
    tWgetInternal *data = (tWgetInternal *) p;
    char  buf[512] = {0};

    stopListener(data->pc->fd);
    
    UTIL_SNPRINTF(buf, sizeof(buf), "Host (%s:%d) is not responding, timeout", data->host, data->port);
    report_status(data, iWgetStatus_ConnectionError, buf);
}


/*----------------------------------------------------------------------*/
static void do_connect(void *p)
{
    tWgetInternal *data = (tWgetInternal *) p;
    int err;
    u_int n;

    vosLog_debug("Enter>");

    utilTmr_cancel(tmrHandle, timer_connect, data);
    stopListener(data->pc->fd);

    /* check fd status */
    n = sizeof(int);
    if (getsockopt(data->pc->fd, SOL_SOCKET, SO_ERROR, &err, &n) < 0)
    {
        report_status(data, iWgetStatus_InternalError,
                      "internal error: do_connect(): getsockopt failed");
        return;
    }

    vosLog_debug("err = %d", err);
    if (err != 0)//fangzhen
    {
        /* connection not established */
        char buf[256];

        UTIL_SNPRINTF(buf, sizeof(buf), "Connection to host %s(%s):%d failed %d (%s)", 
                      data->host, writeIp(data->host_addr), data->port, 
                      err, strerror(err));
        report_status(data, iWgetStatus_ConnectionError, buf);
        return;
    }

    if (SF_FEATURE_SUPPORT_TR69C_SSL)
    {
        /* init ssl if proto is https */
        if ((util_strcmp(data->proto, "https") == 0) && (data->pc->sslConn <= 0))
        {
            proto_SetSslCtx(data->pc, do_connect_ssl, data);
            return;
        }
    }

    /* return at this point if function is connect only */
    if (data->request == eConnect)
    {
        report_status(data, iWgetStatus_Ok, NULL);
        return;
    }
}  /* End of do_connect() */


/*----------------------------------------------------------------------*/
static void do_connect_ssl(void *p, int errorcode)
{
    tWgetInternal *data = (tWgetInternal *) p;

    if (errorcode < 0)
    {
        report_status(data, iWgetStatus_ConnectionError, 
              "Failed to establish SSL connection");
    }
    else
    {
        do_connect(p);
    }
} 


/*----------------------------------------------------------------------*/
static void do_send_request(void *p, int errorcode)
{
    tWgetInternal  *data = (tWgetInternal *) p;
    HttpTask  *ht   = (HttpTask *)(data->handle);
    UINT32 responseTime = ACSRESPONSETIME;
    int  res;
    VOS_RET_E  ret = VOS_RET_SUCCESS;

    vosLog_debug("Enter>, keepConn = %d status = %d", data->keepConnection, data->status);
    if (errorcode < 0)
    {
        report_status(data, iWgetStatus_ConnectionError, 
                      "Failed to establish SSL connection");
        return;
    }

    vosLog_debug("data->request type = %d", data->request);
    /* send request */
    switch (data->request)
    {
        case eGetData:
            res = send_get_request(p, data->host, data->port, data->uri);
            break;
        case ePostData:
            res = send_post_request(p, data->host, data->port, data->uri, data->postdata, 
                                    data->datalen, data->content_type, ht->content_len);
            break;
        case ePutData:
            res = send_put_request(p, data->host, data->port, data->uri, data->postdata, 
                                   data->datalen, data->content_type, ht->content_len);
            break;
        default:
            res = -1;
            break;
    }

    if (res < 0)
    {
        report_status(data, iWgetStatus_ConnectionError, 
                    "Failed to send request on connection");
        return;
    }

    if (simRpcAction != NULL)
    {
        return;
    }

    /* wait for response */
    setListener(data->pc->fd, do_response, data);
    if (ePutData == data->request || eGetData == data->request)
    {
        responseTime = 60 * 1000*5;
    }

    if(SF_FEATURE_LOCATION_GUANGDONG)
    {
        responseTime = 6 * ACSRESPONSETIME;
    }
    ret = utilTmr_set(tmrHandle, timer_response, data, responseTime, "timer_response"); /* response timeout is 60 sec */
    
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("could not set ACS response timer, ret=%d", ret);
    }

}  /* End of do_send_request() */


/*----------------------------------------------------------------------*/
static void do_response(void *p)
{
    CookieHdr    *cp;
    tWgetInternal *data = (tWgetInternal *) p;

    vosLog_debug("Enter>, data->pc = %p", data->pc);
    utilTmr_cancel(tmrHandle, timer_response, data);

    if (data->pc == NULL)
    {
        vosLog_error("wget, Internal Error");
        
        report_status(data, iWgetStatus_InternalError, 
                      "internal error: no filedescriptor");
        return;
    }
    
    if (data->pc->fd <= 0)
    {
        report_status(data, iWgetStatus_InternalError, 
               "internal error: no filedescriptor");
        return;
    }
    
    if (data->hdrs)
    {
        proto_FreeHttpHdrs(data->hdrs);
    }
    
    data->hdrs = proto_NewHttpHdrs();
    if (data->hdrs == NULL)
    {
        /* memory exhausted?!? */
        vosLog_error("wget, Memory exhausted");
    }

    if (proto_ParseResponse(data->pc, data->hdrs) < 0)
    {
        report_status(data, iWgetStatus_Error, 
                      "error: illegal http response or read failure");
        return;
    }

    proto_ParseHdrs(data->pc, data->hdrs);
    cp = data->hdrs->setCookies;
    while (cp)
    { 
        /* save new cookies if present*/
        addCookieHdr( &data->cookieHdrs,cp->name, cp->value, REPLACE );
        cp = cp->next;
    }

    if (data->hdrs->status_code >= 100 && data->hdrs->status_code < 600)
    {
        report_status(data, iWgetStatus_Ok, NULL);
    }
    else
    {
        char buf[1024];
        UTIL_SNPRINTF(buf, sizeof(buf), "Host %s returned error \"%s\"(%d)", 
                      data->host, data->hdrs->message, data->hdrs->status_code);
        report_status(data, iWgetStatus_HttpError, buf);
    }
}   /*End of do_response*/


/*
* Connect to the specified url
* Returns: NULL  failed allocate memory or immediate connection error.
 *               Call wget_LastErrorMsg() to retrieve last error msg. 
 *         pointer to Connection descriptor tWgetInternal. 
*/
tWgetInternal *wget_DiagConnect(const char *url, UtilEventHandler callback, void *handle)
{
    tWgetInternal  *wg = NULL;
    char proto[10] = {0};
    char host[1024] = {0};
    char uri[1024] = {0};
    int  port = 0;
    int  state = 0;

    vosLog_debug("Enter>, (\"%s\", ...)", url);

    if ((wg = (tWgetInternal*)VOS_MALLOC_FLAGS(sizeof(tWgetInternal), ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("VOS_MALLOC_FLAGS failed");
        return NULL;
    }
   
    lastErrorMsg[0] = '\0';

    wg->request        = eConnect;
    wg->keepConnection = eKeepConnectionOpen;
    CMC_wanGetTr69cWanConnState(&state);
    
    if (state != 1)
    {
        vosLog_debug("tr69 wan is not up");
        return NULL;
    }

    if (util_strlen(url) == 0)
    {
        vosLog_debug("acs URL is NULL");
        wg->status = -6;
        return wg;
    }

    if (www_ParseUrl(url, proto, host, &port, uri) < 0)
    {
        vosLog_debug("www_ParseUrl failed");
        wg->status = -5;
        return wg;
    }

    if (port == 0)
    {
        if (util_strcmp(proto, "http") == 0)
        {
            port = 80;
        }
        else if (util_strcmp(proto, "https") == 0)
        {
            port = 443;
        }
        else
        {
            vosLog_error("unsupported protocol in url \"%s\"", proto);
            port = 80; /* guess http and port 80 */
        }
    }

    wg->pc         = NULL;
    wg->cb         = callback;
    wg->handle     = handle;
    wg->proto      = VOS_STRDUP(proto);
    wg->host       = VOS_STRDUP(host);
    wg->host_addr  = 0;
    wg->port       = port;
    
    if (util_strlen(uri))
    {
        wg->uri = VOS_STRDUP(uri);
    }
    else
    {
        wg->uri = VOS_STRDUP("/");
    }

    /*
    * dns_lookup always returns 1, so this "if" will always evaluate to TRUE.
    * There was some code here that attempted to do a non-blocking DNS
    * lookup if dns_lookup returned 0, but that code did not appear complete and
    * we never executed it.  So I just deleted the dead code.  --mwang 2/1/07
    */
    if (dns_diaglookup(wg->host, &(wg->host_addr)))
    {
        /* immediate return requires special handling. */
        int res;
        int fd;

        vosLog_debug("wg->host_addr = %u ", wg->host_addr);

        if (wg->host_addr == 0)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                g_dns_resolve_ret = 1;
            }

            UTIL_SNPRINTF(lastErrorMsg, sizeof(lastErrorMsg), noHostResolve, wg->host);
            vosLog_debug("%s", lastErrorMsg);
            freeData(wg);
            wg = NULL;
        }
        else if ((res = utils_diagEstablishConnection(wg->host_addr, wg->port, &fd)) < 0)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                g_dns_resolve_ret = 2; 
            }

            if (res == -1)
            {
                UTIL_STRNCPY(lastErrorMsg, "Socket creation error", sizeof(lastErrorMsg));
            }
            else
            { 
                UTIL_SNPRINTF(lastErrorMsg, sizeof(lastErrorMsg), noHostConnectMsg, 
                 wg->host, writeIp(wg->host_addr), wg->port);
            }

            vosLog_debug("%s", lastErrorMsg);
            freeData(wg);
            wg = NULL;
        }
        else
        { 
            /* connection complete- start it */
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                g_dns_resolve_ret = 0;
            }

            vosLog_debug("connection complete- start it");

            utilTmr_set(tmrHandle, timer_connect, (void *)wg, 30 * 1000, "connect_timer");

            wg->pc = proto_NewCtx(fd);
            setListenerType(fd, do_connect, wg, iListener_Write);
        }
    }

    return wg;
}
//liuhm add 20140925 end


/*
* Connect to the specified url
* Returns: NULL  failed allocate memory or immediate connection error.
 *               Call wget_LastErrorMsg() to retrieve last error msg. 
 *         pointer to Connection descriptor tWgetInternal. 
*/
tWgetInternal *wget_Connect(const char *url, UtilEventHandler callback, void *handle)
{
    tWgetInternal  *wg = NULL;
    char proto[10] = {0};
    char host[1024] = {0};
    char uri[1024] = {0};
    int  port = 0;
    int  state = 0;

    vosLog_debug("Enter>, (\"%s\", ...)", url);

    if ((wg = (tWgetInternal*)VOS_MALLOC_FLAGS(sizeof(tWgetInternal), ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("VOS_MALLOC_FLAGS failed");
        return NULL;
    }
   
    lastErrorMsg[0] = '\0';

    wg->request        = eConnect;
    wg->keepConnection = eKeepConnectionOpen;
    CMC_wanGetTr69cWanConnState(&state);
    
    if (state != 1)
    {
        vosLog_debug("tr69 wan is not up");
        return NULL;
    }

    if (util_strlen(url) == 0)
    {
        vosLog_debug("acs URL is NULL");
        wg->status = -6;
        return wg;
    }

    if (www_ParseUrl(url, proto, host, &port, uri) < 0)
    {
        vosLog_debug("www_ParseUrl failed");
        wg->status = -5;
        return wg;
    }

    if (0 == port)
    {
        if (util_strcmp(proto, "http") == 0)
        {
            port = 80;
        }
        else if (util_strcmp(proto, "https") == 0)
        {
            port = 443;
        }
        else
        {
            vosLog_error("unsupported protocol in url \"%s\"", proto);
            port = 80; /* guess http and port 80 */
        }
    }

    wg->pc         = NULL;
    wg->cb         = callback;
    wg->handle     = handle;
    wg->proto      = VOS_STRDUP(proto);
    wg->host       = VOS_STRDUP(host);
    wg->host_addr  = 0;
    wg->port       = port;
    
    if (util_strlen(uri))
    {
        wg->uri = VOS_STRDUP(uri);
    }
    else
    {
        wg->uri = VOS_STRDUP("/");
    }

    /*
    * dns_lookup always returns 1, so this "if" will always evaluate to TRUE.
    * There was some code here that attempted to do a non-blocking DNS
    * lookup if dns_lookup returned 0, but that code did not appear complete and
    * we never executed it.  So I just deleted the dead code.  --mwang 2/1/07
    */
    if (dns_lookup(wg->host, &(wg->host_addr)))
    {
        /* immediate return requires special handling. */
        int res;
        int fd;

        vosLog_debug("wg->host_addr = %u ", wg->host_addr);

        if (wg->host_addr == 0)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                g_dns_resolve_ret = 1;
            }

            UTIL_SNPRINTF(lastErrorMsg, sizeof(lastErrorMsg), noHostResolve, wg->host);
            vosLog_debug("%s", lastErrorMsg);
            freeData(wg);
            wg = NULL;
        }
        else if ((res = www_EstablishConnection(wg->host_addr, wg->port, &fd)) < 0)
        {
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                g_dns_resolve_ret = 2; 
            }

            if (res == -1)
            {
                UTIL_STRNCPY(lastErrorMsg, "Socket creation error", sizeof(lastErrorMsg));
            }
            else
            { 
                UTIL_SNPRINTF(lastErrorMsg, sizeof(lastErrorMsg), noHostConnectMsg, 
                 wg->host, writeIp(wg->host_addr), wg->port);
            }

            vosLog_debug("%s", lastErrorMsg);
            freeData(wg);
            wg = NULL;
        }
        else
        { 
            /* connection complete- start it */
            if (SF_FEATURE_SUPPORT_TR69C_REMOTESTATUS)
            {
                g_dns_resolve_ret = 0;
            }

            vosLog_debug("connection complete- start it");

            utilTmr_set(tmrHandle, timer_connect, (void *)wg, 30 * 1000, "connect_timer");

            wg->pc = proto_NewCtx(fd);
            setListenerType(fd, do_connect, wg, iListener_Write);
        }
    }

    return wg;
}


int wget_GetData(tWgetInternal *wg, UtilEventHandler callback, void *handle)
{
    vosLog_debug("Enter>");

    wg->request = eGetData;
    wg->handle = handle;
    wg->cb = callback;
    if (wg->hdrs)
    {
        wg->hdrs->status_code    = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }

    do_send_request(wg, PROTO_OK);
    return 0;
} 


int wget_PostData(tWgetInternal *wg,char *postdata, int datalen, char *content_type,
                  UtilEventHandler callback, void *handle)
{
    vosLog_debug("Enter>");

    wg->request = ePostData;
    wg->content_type = content_type;
    wg->postdata = postdata;
    wg->datalen = datalen;
    wg->handle = handle;
    wg->cb = callback;
    
    if (wg->hdrs)
    {
        wg->hdrs->status_code    = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }

    do_send_request(wg, PROTO_OK);
    return 0;
}  


int wget_PostDataClose(tWgetInternal *wg, char *postdata, int datalen, char *content_type,
                       UtilEventHandler callback, void *handle)
{
    vosLog_debug("Enter>");

    wg->request      = ePostData;
    wg->content_type = content_type;
    wg->postdata     = postdata;
    wg->datalen      = datalen;
    wg->handle       = handle;
    wg->cb = callback;
    if (wg->hdrs)
    {
        wg->hdrs->status_code    = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }
    wg->keepConnection = eCloseConnection;

    do_send_request(wg, PROTO_OK);
    return 0;

} 


int wget_PutData(tWgetInternal *wg, char *putdata, int datalen, char *content_type,
                 UtilEventHandler callback, void *handle)
{
    vosLog_debug("Enter>");

    wg->request      = ePutData;
    wg->content_type = content_type;
    wg->postdata     = putdata;
    wg->datalen      = datalen;
    wg->handle       = handle;
    wg->cb           = callback;
    if (wg->hdrs)
    {
        wg->hdrs->status_code    = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }

    do_send_request(wg, PROTO_OK);
    return 0;
}  


/*
* Disconnect maybe called from within a callback called
 * by report_status. Don't freeData() if cbActive is set.
 * Setting cCloseConnection will cause report_status
 * to free up the data on return by the callback.
* 
*/

int wget_Disconnect(tWgetInternal *wio)
{
    if (wio != NULL)
    { 
        utilTmr_cancel(tmrHandle, timer_response, wio); /* may be running */
        wio->request = eDisconnect;
        wio->keepConnection = eCloseConnection;
        if (!wio->cbActive)
        {
            freeData(wio);
        }
    }

    return 0;
}


int wget_AddPostHdr( tWgetInternal *wio, char *xhdrname, char *value)
{
   XtraPostHdr   **p = &wio->xtraPostHdrs;

   vosLog_debug("Enter>");
   
   return addPostHdr(p, xhdrname,value, REPLACE);
}


void wget_ClearPostHdrs( tWgetInternal *wio)
{
    XtraPostHdr *xh = wio->xtraPostHdrs;

    vosLog_debug("Enter>, wio = %p", wio);

    while (xh)
    {
        XtraPostHdr *nxt;
        VOS_FREE( xh->hdr);
        VOS_FREE(xh->value);
        nxt = xh->next;
        VOS_FREE(xh);
        xh= nxt;
    }
    
    wio->xtraPostHdrs = NULL;
}


const char *wget_LastErrorMsg(void)
{
    return lastErrorMsg;
}

