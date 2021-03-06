
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
 * File Name  :
 *
 * Description:
 * $Revision: 1.20 $
 * $Id: protocol.c,v 1.20 2006/01/31 23:19:54 dmounday Exp $
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fwk.h"
#include "../inc/appdefs.h"
#include "../inc/utils.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../main/event.h"
#include "../inc/tr69cdefs.h"

#include "protocol.h"
#include "www.h"

#ifdef USE_DMALLOC
    #include <dmalloc.h>
#endif

static UBOOL8 firstEagain = FALSE; /* true means had a EAGAIN for errno */
static UtilTimestamp socketErrorTimeStamp = {0, 0};
#define SOCKET_ERROR_TIMEOUT (3 * MSECS_IN_SEC)


#define SERVER_NAME "milli_httpd"
#define PROTOCOL "HTTP/1.1"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

//#define DEBUG 1  /* general debug log */
//#define WRITETRACE  /* prints everything written to socket */
/*#define READTRACE*/  /* prints everything read to socket */
//#define DEBUGSSL  /* log ssl io and connections */
/*Define DBGSSLC(X) to debug SSL connection and SSL_shutdown*/
//#define DBGSSLC(X) fprintf X
#define DBGSSLC(X)
//#define DEBUGHDRS /* log html headers */

#ifdef DEBUGSSL
#define mkstr(S) # S
#define setListener(A,B,C) {fprintf(stderr,mkstr(%s setListener B fd=%d\n), getticks(), A);\
        setListener( A,B,C);}

#define setListenerType(A,B,C,E) {fprintf(stderr,mkstr(%s setListenerType B-E fd=%d\n), getticks(), A);\
        setListenerType( A,B,C,E);}

#define stopListener(A) {fprintf(stderr,"%s stopListener fd=%d\n", getticks(), A);\
        stopListener( A );}
#endif
#ifdef DEBUGSSL
char timestr[40];
char *getticks()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    sprintf(timestr, "%04ld.%06ld", now.tv_sec % 1000, now.tv_usec);
    return timestr;
}

#endif

#define BUF_SIZE_MAX 4096

/*======================================================================*
 * Util
 *======================================================================*/
typedef struct
{
    tProtoCtx *pc;
    tSSLIO    iofunc;
    char *ptr;
    int nbytes;
    tProtoHandler cb;
    void *userdata;
} SSL_io_ctx;


/*
  * display SOAP messages on serial console.
  * This flag is initialize, enabled or disabled in main.c,
  * and perform action in protocol.c
  */

extern ACSState acsState;

static SSL_CTX *ssl_ctx = NULL;

extern void writeLog(const char *buf, int bufLen);


#ifdef DEBUGSSL
static void showSocketStatus(unsigned fd)
{

    struct pollfd fdl[1];
    int e;

    fdl[0].fd = fd;
    fdl[0].events = 0xff;
    if ((e = poll(fdl, 1, 0)) < 0)
        fprintf(stderr, "*poll() error\n");
    else
        fprintf(stderr, "poll=%0x\n", fdl[0].revents);
}
#endif


/*
* verify server certificate    see SSL_CTS_set_verify()
* Returns: 0 - verification failed: stop verifying in failed state.
*          1 - continue  verifying.
*/
static int verify_callback(int ok, X509_STORE_CTX *store __attribute__((unused)))
{
    char subject[BUF_SIZE_MAX], issuer[BUF_SIZE_MAX];
    char *pCN = NULL, *pEnd = NULL;
    X509 *cert;
    int  err, depth;

    if (SF_FEATURE_SUPPORT_TR69C_ORIGINAL_OPENSSL)
    {
        cert = X509_STORE_CTX_get_current_cert(store);
        err = X509_STORE_CTX_get_error(store);
        depth = X509_STORE_CTX_get_error_depth(store);

        /*
        * Retrieve the pointer to the SSL of the connection currently treated
        * and the application specific data stored into the SSL object.
        */
        X509_NAME_oneline(X509_get_subject_name(cert), subject, BUF_SIZE_MAX);
        X509_NAME_oneline(X509_get_issuer_name(store->current_cert), issuer, BUF_SIZE_MAX);

        if (0 == ok)
        {
            vosLog_error("error_num = %d, err_msg = %s, depth = %d,\nsubject = %s,\nissuer = %s\n",
                         err, X509_verify_cert_error_string(err), depth, subject, issuer);
        }
        else
        {
            vosLog_debug("error_num = %d, err_msg = %s, depth = %d,\nsubject = %s,\nissuer = %s\n",
                         err, X509_verify_cert_error_string(err), depth, subject, issuer);
        }

        if (subject != NULL)
        {
            if ((pCN = strstr(subject, "CN=")) != NULL)
            {
                pCN += 3;   /* pass "CN=" to point to value of CN*/
                if ((pEnd = strchr(pCN, '/')) != NULL)
                {
                    *pEnd = '\0';
                    if (strstr(acsState.acsURL, pCN) != NULL)
                    {
                        vosLog_debug("return X509_V_OK, CN = %s, URL = %s\n", pCN, acsState.acsURL);
                    }
                    else
                    {
                        ok = 0;
                        err = X509_V_ERR_APPLICATION_VERIFICATION;
                        X509_STORE_CTX_set_error(store, err);
                        vosLog_error("return X509_V_ERR_APPLICATION_VERIFICATION, CN is not in URL, CN = %s, URL = %s\n", pCN, acsState.acsURL);
                    }
                }
            }
        }
    }

    return ok;
}


/*======================================================================*
 * Init
 *======================================================================*/
void proto_Init()
{
    vosLog_debug("=====>ENTER");

    if (SF_FEATURE_SUPPORT_TR69C_SSL)
    {
#ifdef USE_DMALLOC
        CRYPTO_malloc_debug_init();
        CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
        CRYPTO_set_mem_debug_options(V_CRYPTO_MDEBUG_ALL);
#endif
        SSL_load_error_strings();
        SSL_library_init();
        ssl_ctx = SSL_CTX_new(SSLv3_client_method());
        if (NULL == ssl_ctx)
        {
            vosLog_error("Could not create SSL context");
            exit(1);
        }

        if (SF_FEATURE_SUPPORT_TR69C_ORIGINAL_OPENSSL)
        {
            /* bcm ssl setup cipher list by default, so skip the error log */
            if (!SSL_CTX_set_cipher_list(ssl_ctx, ACS_CIPHERS))
            {
                vosLog_error("Could not set cipher list for SSL");
            }
        }

#ifdef USE_CERTIFICATES
        {
            struct stat filestat;
            /* PT: add to support client certificate*/
            if (SF_FEATURE_SUPPORT_TR69C_ORIGINAL_OPENSSL)
            {
                if (0 == lstat(CLIENT_CERT_FILE, &filestat))
                {
                    if (0 == lstat(CLIENT_PRIVATE_KEY_FILE, &filestat))
                    {
                        if (SSL_CTX_use_certificate_file(ssl_ctx, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
                        {
                            vosLog_error("Error loading the client certificate");
                        }

                        if (SSL_CTX_use_PrivateKey_file(ssl_ctx, CLIENT_PRIVATE_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
                        {
                            vosLog_error("Error loading the client private key");
                        }

                        if (! SSL_CTX_check_private_key(ssl_ctx))
                        {
                            vosLog_error("Private key does not match the client certificate public key");
                        }
                    }
                    else
                    {
                        vosLog_error("No private key found");
                    }
                }
                else
                {
                    vosLog_notice("No client certificate found");
                }
            }

            if (0 == lstat(ROOT_CERT_FILE, &filestat))
            {
                int retval = SSL_CTX_load_verify_locations(ssl_ctx, ROOT_CERT_FILE, CERT_PATH);
                /* for both ssl, retval == 1 is load verified */
                if (retval != 1)
                {
                    vosLog_error("Could not load verify locations");
                }

                /* if fail to load certificate, set the certificate verify anyway */
                SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, verify_callback);
            }
            else
            {
                /* for bcm ssl, if no certificate in the system, just skip the certificate check */
                vosLog_notice("No server certificate found. Skip checking on certificate.");
                SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, 0);
            }
        }

#endif // USE_CERTIFICATES
        if (SF_FEATURE_SUPPORT_TR69C_ORIGINAL_OPENSSL)
        {
            SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);
            ERR_print_errors_fp(stderr);
            SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_OFF);
        }
    }
}  /* End of proto_Init() */

/*======================================================================*
 * Ctx
 *======================================================================*/
/*----------------------------------------------------------------------*/
tProtoCtx *proto_NewCtx(int fd)
{
    tProtoCtx *pc;

    pc = (tProtoCtx *)VOS_MALLOC_FLAGS(sizeof(tProtoCtx), ALLOC_ZEROIZE);
    pc->type = iNormal;
    pc->fd   = fd;

    return pc;

}  /* End of proto_NewCtx() */


/*----------------------------------------------------------------------*/
static void server_wait_for_ssl(void *handle)
{
    tProtoCtx *pc = handle;

#ifdef DEBUGSSL
    vosLog_debug("%s server_wait_for_ssl() SSL connect fd=%d ", getticks(), pc->fd);
    showSocketStatus(pc->fd);
#endif
    /*stopListener(pc->fd);*/
    if ((pc->sslConn = SSL_connect(pc->ssl)) <= 0)
    {
        int sslres;
        sslres = SSL_get_error(pc->ssl, pc->sslConn);
        if (sslres == SSL_ERROR_WANT_READ)
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL connection wants to read fd=%d", getticks(), pc->fd);
#endif
            setListener(pc->fd, server_wait_for_ssl, pc);
        }
        else if (sslres == SSL_ERROR_WANT_WRITE)
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL connection wants to write fd=%d", getticks(), pc->fd);
#endif
            setListenerType(pc->fd, server_wait_for_ssl, pc, iListener_Write);
        }
        else
        {
            char errstr[256];
            ERR_error_string_n(sslres, errstr, 256);
            vosLog_debug("Error connecting to server(Possible certificate error): (res=%d,sslres=%d) %s",
                         pc->sslConn, sslres, errstr);
            (*(pc->cb))(pc->data, PROTO_ERROR_SSL);
            /* Note: pc may have been freed by the callback */
            return;
        }
    }
    else
    {
#ifdef DEBUGSSL
        vosLog_debug("%s SSL server_wait_for_ssl callback fd=%d", getticks(), pc->fd);
#endif
        (*(pc->cb))(pc->data, PROTO_OK);
    }
}  /* End of server_wait_for_ssl() */

/*----------------------------------------------------------------------*
 * callback errorcodes
 *  PROTO_OK          all ok
 *  PROTO_ERROR       generic error
 *  PROTO_ERROR_SSL   ssl error
 */
void proto_SetSslCtx(tProtoCtx *pc, tProtoHandler cb, void *data)
{
    vosLog_debug("=====>ENTER");
    pc->type = iSsl;

    if (pc->ssl == NULL)
    {
        pc->ssl = SSL_new(ssl_ctx);
    }

    if (pc->ssl != NULL)
    {
        DBGSSLC((stderr, "%s proto_SetSslCtx() ssl_ctx=%p ssl=%p fd=%d\n",
                 getticks(), ssl_ctx, pc->ssl, pc->fd));
        if (SSL_set_fd(pc->ssl, pc->fd) > 0)
        {
            pc->cb   = cb;
            pc->data = data;
            /* TBD: add a timeout */
            server_wait_for_ssl(pc);
        }
        else
        {
            vosLog_error("SSL_set_fd failed");
        }
    }
    else
    {
        vosLog_error("SSL_new failed");
    }
}  /* End of proto_SetSslCtx() */

static void postShutdownCleanUp(void *handle)
{
    tProtoCtx   *pc = (tProtoCtx *)handle;

    DBGSSLC((stderr, "%s  postShutdownCleanUp() ssl=%p fd=%d\n", getticks(), pc->ssl, pc->fd));
    utilTmr_cancel(tmrHandle, postShutdownCleanUp, (void *)pc);
    stopListener(pc->fd);
    close(pc->fd);
    if (pc->ssl)
    {
        SSL_free(pc->ssl);
    }
    pc->sslConn = 0;
    VOS_FREE(pc);
    return;

}  /* End of postShutdownCleanUp() */

static void wait_for_sslShutdown(void *handle)
{
    tProtoCtx   *pc = (tProtoCtx *)handle;
    int         r;

    DBGSSLC((stderr, "%s  wait_for_sslShutdown()fd=%d\n", getticks(), pc->fd));
    r = SSL_shutdown(pc->ssl);
    DBGSSLC((stderr, "%s  SSL_shutdown= %d ssl=%p fd=%d\n",
             getticks(), r, pc->ssl, pc->fd));
    if (r == 0)
    {
        /* started shutdown -- now call again */
        r = SSL_shutdown(pc->ssl);
        DBGSSLC((stderr, "%s  2nd SSL_shutdown= %d ssl=%p fd=%d\n",
                 getticks(), r, pc->ssl, pc->fd));
    }
    if (r == 1)
    {
        postShutdownCleanUp(pc);
    }
    else if (r == -1)
    {
        int sslres;

        sslres = SSL_get_error(pc->ssl, r);
        if (sslres == SSL_ERROR_WANT_READ)
        {
            DBGSSLC((stderr, "%s SSL_shutdown wants to read fd=%d\n", getticks(), pc->fd));
            setListener(pc->fd, wait_for_sslShutdown, pc);
        }
        else if (sslres == SSL_ERROR_WANT_WRITE)
        {
            DBGSSLC((stderr, "%s SSL_shutdown wants to write fd=%d\n", getticks(), pc->fd));
            setListenerType(pc->fd, wait_for_sslShutdown, pc, iListener_Write);
        }
        else
        {
            char errstr[256];
            ERR_error_string_n(sslres, errstr, 256);
            vosLog_error("SSL_shutdown server: (r=%d,sslres=%d) %s", r, sslres, errstr);
            postShutdownCleanUp(pc);
            return;
        }
    }
    else
    {
        DBGSSLC((stderr, "%s  SSL_shutdown state error ssl=%p fd=%d\n",
                 getticks(), pc->ssl, pc->fd));
        postShutdownCleanUp(pc);
    }
    return;

}  /* End of wait_for_sslShutdown() */


/*----------------------------------------------------------------------*/
/* For iNormal protoCtx stopListener and close the fd               */
/* For iSSL start shutdown                                    */

void proto_FreeCtx(tProtoCtx *pc)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    switch (pc->type)
    {
        case iNormal:
            stopListener(pc->fd);
            close(pc->fd);
            VOS_FREE(pc);
            break;

        case iSsl:
            if (SF_FEATURE_SUPPORT_TR69C_SSL)
            {
                stopListener(pc->fd);
                DBGSSLC((stderr, "%s   proto_FreeCtx()ssl=%p fd=%d\n",
                         getticks(), pc->ssl, pc->fd));
                ret = utilTmr_set(tmrHandle, postShutdownCleanUp, (void *)pc, 3000, "postShutdownCleanUp");
                if (ret != VOS_RET_SUCCESS)
                {
                    vosLog_error("could not set postShutdownCleanUp timer, ret=%d", ret);
                }

                if (pc->ssl)
                {
                    /* not completed */
                    wait_for_sslShutdown(pc);
                }
                else
                {
                    postShutdownCleanUp(pc);
                }
            }
            break;

        default:
            vosLog_error("Impossible error; proto_FreeCtx() illegal ProtoCtx type (%d)",
                         pc->type);
            VOS_FREE(pc);
            break;
    }
}  /* End of proto_FreeCtx() */


/*
* The following SSL io routines ensure that the parameters are saved
 * and restored in the subsequent call to SSL_read/write whenever the
 * functions return a -1 indicating non-blocking inprogress io.
 */
static void SSL_io_handler(void *handle)
{
    SSL_io_ctx *rc = handle;
    int nresult;
    int sslres;
    if (rc->iofunc == sslRead)
    {
        nresult = SSL_read(rc->pc->ssl, (void *)rc->ptr, rc->nbytes);
#ifdef DEBUGSSL
        vosLog_debug("%s SSL_io_handler read ssl=%x socket=%d nresult=%d", getticks(), rc->pc->ssl, rc->pc->fd, nresult);
#endif
    }
    else
    {
        nresult = SSL_write(rc->pc->ssl, (void *)rc->ptr, rc->nbytes);
#ifdef DEBUGSSL
        vosLog_debug("%s SSL_io_handler write ssl=%x socket=%d nresult=%d", getticks(), rc->pc->ssl, rc->pc->fd, nresult);
#endif
    }
    if (nresult < 0)
    {
        sslres = SSL_get_error(rc->pc->ssl, nresult);
        if (sslres == SSL_ERROR_WANT_READ)
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL connection listen to read fd=%d", getticks(), rc->pc->fd);
#endif
            setListener(rc->pc->fd, SSL_io_handler, rc);
            return;
        }
        else if (sslres == SSL_ERROR_WANT_WRITE)
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL connection listen to write fd=%d", getticks(), rc->pc->fd);
#endif
            setListenerType(rc->pc->fd, SSL_io_handler, rc, iListener_Write);
            return;
        }
        vosLog_debug("SSL_io_handler %s error fd=%d errcode=%d",
                     rc->iofunc == sslRead ? "read" : "write", rc->pc->fd, sslres);
        return;
    }
    /* If we get here, we're done */

    stopListener(rc->pc->fd);
    (*(rc->cb))((void *)rc->userdata, nresult);
    VOS_FREE(rc);
}

/*----------------------------------------------------------------------*/
int proto_SSL_IO(tSSLIO func, tProtoCtx *pc, char *ptr, int nbytes, tProtoHandler cb, void *data)
{

    SSL_io_ctx *rc;
    int nresult = 0;

    if (func == sslRead)
    {
        nresult = SSL_read(pc->ssl, ptr, nbytes);
#ifdef DEBUGSSL
        vosLog_debug("%s proto_SSL_io read fd=%d nresult=%d", getticks(), pc->fd, nresult);
#endif
    }
    else if (func == sslWrite)
    {
        nresult = SSL_write(pc->ssl, ptr, nbytes);
#ifdef DEBUGSSL
        vosLog_debug("%s proto_SSL_io write fd=%d nresult=%d", getticks(), pc->fd, nresult);
#endif
    }
    if (nresult < 0)
    {
        int sslres = SSL_get_error(pc->ssl, nresult);
        rc = (SSL_io_ctx *)VOS_MALLOC_FLAGS(sizeof(SSL_io_ctx), ALLOC_ZEROIZE);
        if (!rc)
            return -2;
        rc->iofunc = func;
        rc->pc = pc;
        rc->ptr = ptr;
        rc->nbytes = nbytes;
        rc->cb = cb;
        rc->userdata = data;
        if (sslres == SSL_ERROR_WANT_READ)
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL_IO connection listen to read fd=%d", getticks(), rc->pc->fd);
#endif
            setListenerType(rc->pc->fd, SSL_io_handler, rc, iListener_ReadWrite);
        }
        else if (sslres == SSL_ERROR_WANT_WRITE)
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL_IO connection listen to write fd=%d", getticks(), rc->pc->fd);
#endif
            setListenerType(rc->pc->fd, SSL_io_handler, rc, iListener_Write);
        }
        else
        {
#ifdef DEBUGSSL
            vosLog_debug("%s SSL_IO fd=%d error=%d", getticks(), rc->pc->fd, sslres);
#endif
            VOS_FREE(rc);
        }
    }
    return nresult;
}


/*----------------------------------------------------------------------*/
/* blocking read */
int proto_ReadWait(tProtoCtx *pc, char *ptr, int nbytes)
{
    int nread = 0;
    int flags, bflags;

    /* turn on synchroneous I/O, this call will block. */
    {
        flags = (long) fcntl(pc->fd, F_GETFL);
        bflags = flags & ~O_NONBLOCK; /* clear non-block flag, i.e. block */
        fcntl(pc->fd, F_SETFL, bflags);
    }

    errno = 0;
    switch (pc->type)
    {
        case iNormal:
            nread = read(pc->fd, ptr, nbytes);
            break;
        case iSsl:
            if (SF_FEATURE_SUPPORT_TR69C_SSL)
            {
#ifdef DEBUGSSL
                vosLog_debug("%s read_SSL(%d, lth=%d)", getticks(), pc->fd, nbytes);
                vosLog_debug(" result=%d", nread = SSL_read(pc->ssl, (void *) ptr, nbytes));
#else
                nread = SSL_read(pc->ssl, (void *) ptr, nbytes);
#endif
            }

            break;

        default:
            vosLog_error("Impossible error; readn() illegal ProtoCtx type (%d)", pc->type);
            break;
    }
    if (nread > nbytes)
    {
        vosLog_error("proto_READ of %d returned %d", nbytes, nread);
    }

    fcntl(pc->fd, F_SETFL, flags); /* remove blocking flags */

    return nread;
}

int read_timeout(int socket_, int timeOutSec_)
{
    fd_set readSet;

    FD_ZERO(&readSet);
    FD_SET(socket_, &readSet);
    if (timeOutSec_ == 0)
    {
        // zero means BLOCKING operation (will wait indefinitely)
        return (select(socket_ + 1, &readSet, NULL, NULL, NULL));
    }
    // otherwise, wait up to the specified time period
    struct timeval tv;

    tv.tv_sec = timeOutSec_;
    tv.tv_usec = 0;

    return (select(socket_ + 1, &readSet, NULL, NULL, &tv));

    // returns 0 if the time limit expired.
    // returns -1 on error, otherwise there is data on the port ready to read
}

/*----------------------------------------------------------------------*/
int proto_Readn(tProtoCtx *pc, char *ptr, int nbytes)
{
    int nleft, nread = 0;
    int   errnoval;

    nleft = nbytes;
    while (nleft > 0)
    {
        errno = 0;
        switch (pc->type)
        {
            case iNormal:
                if (read_timeout(pc->fd, 60) <= 0)
                {
                    vosLog_error("read packet timeout");
                    return -99; //timeout!!!
                }

                nread = read(pc->fd, ptr, nleft);
                break;
            case iSsl:
                if (SF_FEATURE_SUPPORT_TR69C_SSL)
                {
#ifdef DEBUGSSL
                    vosLog_debug("%s SSL_read(%d, lth=%d)", getticks(), pc->fd, nleft);
                    nread = SSL_read(pc->ssl, (void *) ptr, nleft);
                    vosLog_debug(" result=%d", nread);
#else
                    nread = SSL_read(pc->ssl, (void *) ptr, nleft);
#endif
                }
                break;
            default:
                vosLog_error("Impossible error; readn() illegal ProtoCtx type (%d)", pc->type);
                break;
        }

        if (nread < 0)                              /* This function will read until the byte cnt*/
        {
            errnoval = errno;                       /* is reached or the return is <0. In the case*/
            if (errnoval == EAGAIN)                 /* of non-blocking reads this may happen after*/
                return nbytes - nleft;              /* some bytes have been retrieved. The EAGAIN*/
            else                                    /* status indicates that more are coming */
                /* Other possibilites are ECONNRESET indicating*/
                /* that the tcp connection is broken */
                fprintf(stderr, "!!!!!!!! read(fd=%d) error=%d\n",
                        pc->fd, errnoval);
            return nread; /* error, return < 0 */

        }
        else if (nread == 0)
        {
            break; /* EOF */
        }

        nleft -= nread;
        ptr += nread;
    }

    return nbytes - nleft; /* return >= 0 */
}
/*
 * Return number of bytes written or -1.
 * If -1 check for errno for EAGAIN and recall.
 *----------------------------------------------------------------------*/
int proto_Writen(tProtoCtx *pc, const char *ptr, int nbytes)
{
    int  nwritten = 0;

    errno = 0;
    switch (pc->type)
    {
        case iNormal:
#ifdef DUMPSOAPOUT
            fprintf(stderr, "%s", ptr);
#endif
            nwritten = write(pc->fd, ptr, nbytes);
            break;
        case iSsl:
            if (SF_FEATURE_SUPPORT_TR69C_SSL)
            {
#ifdef DEBUGSSL
                vosLog_debug("%s SSL_write(%d, lth=%d)", getticks(), pc->fd, nbytes);
                nwritten = SSL_write(pc->ssl, ptr, nbytes);
                vosLog_debug("result=%d", nwritten);
#else
                nwritten = SSL_write(pc->ssl, ptr, nbytes);
#endif
            }
            break;
        default:
            vosLog_error("Impossible error; writen() illegal ProtoCtx type (%d)", pc->type);
            break;
    }

    writeLog(ptr, nbytes);

    if (nwritten <= 0)
    {
        if (errno != EAGAIN)
            /*            fprintf(stderr,"proto_Writen() status = %d Error%s(%d)\n",nwritten,strerror(errno),errno);  */
            return nwritten;
        /*
        else
              fprintf(stderr,"proto_Writen() status = %d Error%s(%d)\n",nwritten,strerror(errno),errno);
          */
    }
    /*
     if (nwritten != nbytes) {
         fprintf(stderr,"proto_Writen() short write rlth=%d actual=%d\n", nbytes, nwritten);
     }
    */
    return nwritten;

}  /* End of proto_Writen() */

/*----------------------------------------------------------------------*
 * Read a line from a descriptor. Read the line one byte at a time,
 * looking for the newline. We store the newline in the buffer,
 * then follow it with a \0 (the same as fgets).
 * We return the number of characters up to, but not including,
 * the \0 (the same as strlen).
 */

int proto_Readline(tProtoCtx *pc, char *buf, int maxlen)
{
    int n, rc;
    char   *ptr = buf;
    char c;
    int flags, bflags;

    /* turn on synchroneous I/O, this call will block. */
    {
        flags = (long) fcntl(pc->fd, F_GETFL);
        bflags = flags & ~O_NONBLOCK; /* clear non-block flag, i.e. block */
        fcntl(pc->fd, F_SETFL, bflags);
    }

    for (n = 1; n < maxlen; n++)
    {
        rc = proto_Readn(pc, &c, 1);
        if (rc == 1)
        {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            if (n == 1)
            {
                fcntl(pc->fd, F_SETFL, flags); /* TBD: fix part2, remove blocking flags */
                return 0; /* EOF, no data read */
            }
            else
                break;    /* EOF, some data was read */
        }
        else
        {
            vosLog_debug("ERROR: proto_Readline fd=%d (%d)", pc->fd, errno);
            fcntl(pc->fd, F_SETFL, flags); /* remove blocking flags */
            return -1; /* ERROR */
        }
    }

    *ptr = '\0';

    writeLog(buf, util_strlen(buf));

    fcntl(pc->fd, F_SETFL, flags); /* remove blocking flags */
    return n;
}


/*----------------------------------------------------------------------*/
void proto_Printline(tProtoCtx *pc, const char *fmt, ...)
{
    char *p;
    va_list ap;
    int n;
    int size;

    size = 1024;
    if ((p = VOS_MALLOC_FLAGS((UINT32)size, 0)) == NULL)
    {
        vosLog_error("failed to malloc(%d)", size);
        return;
    }

    while (1)
    {
        /* try to print in the allocated space */
        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        if (n < 0)
        {
            vosLog_notice("fdprintf() vsnprintf failed *%d): %s (%d) fmt=\"%s\"", n, strerror(errno), errno, fmt);
            return;
        }
        else if (n >= 0 && n < size)
        {
            /* print succeeded, let's write it on outstream */
            proto_Writen(pc, p, n);
            VOS_FREE(p);
            return;
        }
        else
        {
            vosLog_debug("vsnprintf, only wrote %d bytes, retrying: fmt=\"%s\" strlen(fmt)=%d size=%d",
                         n, fmt, strlen(fmt), size);
            size *= 2;
            if ((p = VOS_REALLOC(p, (UINT32)size)) == NULL)
            {
                vosLog_error("failed to realloc(%d)", size);
                return;
            }
        }
    }/*end while(1)*/
}

/*======================================================================*
 * Data
 *======================================================================*/
tHttpHdrs *proto_NewHttpHdrs()
{
    return ((tHttpHdrs *)VOS_MALLOC_FLAGS(sizeof(tHttpHdrs), ALLOC_ZEROIZE));

}  /* End of proto_NewHttpHdrs() */

/*----------------------------------------------------------------------*/
void proto_FreeHttpHdrs(tHttpHdrs *p)
{
    CookieHdr   *cp, *last;
    VOS_FREE(p->content_type);
    VOS_FREE(p->protocol);
    VOS_FREE(p->wwwAuthenticate);
    VOS_FREE(p->Authorization);
    VOS_FREE(p->TransferEncoding);
    VOS_FREE(p->Connection);
    VOS_FREE(p->method);
    VOS_FREE(p->path);
    VOS_FREE(p->host);
    cp = p->setCookies;
    while (cp)
    {
        last = cp->next;
        VOS_FREE(cp->name);
        VOS_FREE(cp->value);
        VOS_FREE(cp);
        cp = last;
    }
    VOS_FREE(p->message);
    VOS_FREE(p->locationHdr);
    VOS_FREE(p->filename);
    VOS_FREE(p->arg);
    VOS_FREE(p);

}  /* End of proto_FreeHttpHdrs() */

/*======================================================================*
 * Sending
 *======================================================================*/
/*----------------------------------------------------------------------*/
void proto_SendRequest(tProtoCtx *pc, const char *method, const char *url)
{
    char buf[BUF_SIZE_MAX] = {0};
    int len;

    len = UTIL_SNPRINTF(buf, BUF_SIZE_MAX, "%s %s HTTP/1.1\r\n", method, url);
    if (len != proto_Writen(pc, buf, len))
    {
        /* error in sending */
        ;
    }
    vosLog_debug("proto_SendRequest(%s)", buf);

}  /* End of proto_SendRequest() */

/*----------------------------------------------------------------------*/
void proto_SendCookie(tProtoCtx *pc, CookieHdr *c)
{
    char buf[BUF_SIZE_MAX];
    int len;

    len = UTIL_SNPRINTF(buf, BUF_SIZE_MAX, "Cookie: %s=%s\r\n", c->name, c->value);
    if (len != proto_Writen(pc, buf, len))
    {
        /* error in sending */
        ;
    }
    /*
    #ifdef DEBUGHDRS
       vosLog_debug("proto_SendCookie -> %s", buf);
    #endif
    */
}  /* End of proto_SendCookie() */

/*----------------------------------------------------------------------*/
void proto_SendHeader(tProtoCtx *pc,  const char *header, const char *value)
{
    char buf[BUF_SIZE_MAX];
    int len;

    if (header == NULL || value == NULL)
    {
        return;
    }

    len = UTIL_SNPRINTF(buf, BUF_SIZE_MAX, "%s: %s\r\n", header, value);
    if (len != proto_Writen(pc, buf, len))
    {
        /* error in sending */
        ;
    }
    /*
    #ifdef DEBUGHDRS
       vosLog_debug("proto_SendHeader -> %s", buf);
    #endif
    */
}  /* End of proto_SendHeader() */


/*----------------------------------------------------------------------*/
void proto_SendRaw(tProtoCtx *pc, const char *arg, int len)
{
    int   wlth;
    int   totWlth = 0;
    int   fault = VOS_RET_SUCCESS;
#ifdef DEBUGHDRS
    if (*arg != '<' && *arg != '\r')
    {
        /* debuggging*/
        vosLog_debug("!!%10.10s!!", arg);
    }
#endif

    firstEagain = FALSE;

    while ((totWlth < len) && (fault == VOS_RET_SUCCESS))
    {
        if ((wlth = proto_Writen(pc, arg + totWlth, len - totWlth)) >= 0)
        {
            /* some or all data sent*/
            totWlth += wlth;
            firstEagain = FALSE;
            continue;
        }
        else
        {
            if (errno == EAGAIN)
            {
                if (firstEagain == FALSE)
                {
                    firstEagain = TRUE;
                    utilTms_get(&socketErrorTimeStamp);
                    continue;
                }
                else
                {
                    UtilTimestamp nowTimestamp;
                    UINT32 deltaMs;

                    utilTms_get(&nowTimestamp);
                    deltaMs = utilTms_deltaInMilliSeconds(&nowTimestamp, &socketErrorTimeStamp);

                    if (deltaMs < SOCKET_ERROR_TIMEOUT)
                    {
                        /* can't send is all keep trying -- busy wait on writes while still in acs retry interval */
                        continue;
                    }
                    else
                    {
                        /* data send error and reset firstEagain to FALSE
                         * and set acsState.fault to VOS_RET_INTERNAL_ERROR
                         */
                        fault = VOS_RET_INTERNAL_ERROR;
                        acsState.fault = fault;
                        vosLog_error("Spent %d milliseconds on write retry and give up.", deltaMs);
                    }
                }
            }
            else /* must be a socket error, just set acsState.fault to VOS_RET_INTERNAL_ERROR */
            {
                fault = VOS_RET_INTERNAL_ERROR;
                acsState.fault = fault;
                vosLog_error("Serious socket error.  errno=%d", errno);
            }
        }
    }

}  /* End of proto_SendRaw() */

/*----------------------------------------------------------------------*/
void proto_SendEndHeaders(tProtoCtx *pc)
{
#ifdef DEBUGHDRS
    vosLog_debug("proto_SendEndHeaders()");
#endif
    proto_SendRaw(pc, "\r\n", 2);
}

/*----------------------------------------------------------------------*/
void proto_SendHeaders(tProtoCtx *pc, int status, const char *title, const char *extra_header, const char *content_type)
{
    time_t now;
    char timebuf[100];

    proto_Printline(pc, "%s %d %s\r\n", PROTOCOL, status, title);
    now = time((time_t *) 0);
    (void) strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    proto_Printline(pc, "Date: %s\r\n", timebuf);
    proto_Printline(pc, "MIME-Version: 1.0\r\n");
    proto_Printline(pc, "Server: %s\r\n", SERVER_NAME);
    proto_Printline(pc, "Connection: Close\r\n");
    if (extra_header != NULL)
        proto_Printline(pc, "%s\r\n", extra_header);
    if (content_type != NULL)
        proto_Printline(pc, "Content-Type: %s\r\n", content_type);
    proto_Printline(pc, "\r\n");
}

/*----------------------------------------------------------------------*/
void proto_SendRedirect(tProtoCtx *pc, const char *host, const char *location)
{
    char header[BUF_SIZE_MAX];
    char slash[2];

    if (location[0] == '/')
        UTIL_STRNCPY(slash, "", sizeof(slash));
    else
        UTIL_STRNCPY(slash, "/", sizeof(slash));
    vosLog_debug("web: proto_SEndRedirect: host=%s location=%s", host, location);
    (void) UTIL_SNPRINTF(header, sizeof(header), "Location: http://%s%s%s", host, slash, location);
    proto_SendHeaders(pc, 307, "Redirect", header, "text/html");
    vosLog_debug("web: proto_SEndRedirect: %s", header);
}


/*----------------------------------------------------------------------*/
void proto_SendRedirectProtoHost(tProtoCtx *pc, const char *protohost, const char *location)
{
    char header[BUF_SIZE_MAX];

    (void) UTIL_SNPRINTF(header, sizeof(header), "Location: %s%s", protohost, location);
    proto_SendHeaders(pc, 307, "Redirect", header, "text/html");
    vosLog_debug("web: proto_SendRedirectProtoHost: %s", header);
}

/*----------------------------------------------------------------------*/
void proto_SendRedirectViaRefresh(tProtoCtx *pc, const char *host, const char *location)
{
    char slash[2];

    if (location[0] == '/')
        UTIL_STRNCPY(slash, "", sizeof(slash));
    else
        UTIL_STRNCPY(slash, "/", sizeof(slash));

    proto_SendHeaders(pc, 200, "Ok", NULL, "text/html");
    proto_Printline(pc, "<HTML><HEAD><TITLE>Redirecting to requested site...</TITLE>\n");
    proto_Printline(pc, "<meta http-equiv=\"refresh\" content=\"0;URL=http://%s%s%s\"></HEAD>\n",
                    host, slash, location);
}

/*----------------------------------------------------------------------*/
void proto_SendError(tProtoCtx *pc, int status, const char *title, const char *extra_header, const char *text)
{
    proto_SendHeaders(pc, status, title, extra_header, "text/html");
    proto_Printline(pc, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n", status, title);
    proto_Printline(pc, "<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title);
    proto_Printline(pc, "%s\n", text);
    proto_Printline(pc, "</BODY></HTML>\n");
}

/*======================================================================*
 * Receiving
 *======================================================================*/
/*----------------------------------------------------------------------*
 * return
 *   0 if ok
 *  -1 on failure
 */
int proto_ParseRequest(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char buf[BUF_SIZE_MAX];
    char method[BUF_SIZE_MAX];
    char path[BUF_SIZE_MAX];
    char protocol[BUF_SIZE_MAX];

    /* Parse the first line of the request. */
    if (proto_Readline(pc, buf, BUF_SIZE_MAX) <= 0)
    {
        vosLog_debug("error =%d proto_ParseRequest() proto_Readline() rtns empty",
                     errno);
        return -1;
    }
    if (sscanf(buf, "%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3)
    {
        vosLog_debug("sscanf error on >>%s<<", buf);
        return -1;
    }

    www_StripTail(method);
    www_StripTail(path);
    www_StripTail(protocol);
    VOS_FREE(hdrs->method);
    hdrs->method = VOS_STRDUP(method);
    VOS_FREE(hdrs->path);
    hdrs->path = VOS_STRDUP(path);
    VOS_FREE(hdrs->protocol);
    hdrs->protocol = VOS_STRDUP(protocol);

#ifdef DEBUGHDRS
    vosLog_debug("proto_ParseRequest method=\"%s\" path=\"%s\" protocol=\"%s\"",
                 hdrs->method, hdrs->path, hdrs->protocol);
#endif
    return 0; /* OK */
}

/*----------------------------------------------------------------------*
 * return
 *   0 if ok
 *  -1 on failure
 */
int proto_ParseResponse(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char buf[BUF_SIZE_MAX];
    char protocol[BUF_SIZE_MAX];
    char status[BUF_SIZE_MAX];
    char message[BUF_SIZE_MAX];

#ifdef DEBUGSSL
    vosLog_debug("%s proto_ParseResponse()", getticks());
#endif
    /* Parse the first line of the request. */
    if (proto_Readline(pc, buf, BUF_SIZE_MAX) <= 0)
    {
        return -1;
    }

    if (sscanf(buf, "%[^ ] %[^ ] %[^\r]", protocol, status, message) != 3)
    {
        vosLog_debug("sscanf error on >>%s<<", buf);
        return -1;
    }

    www_StripTail(protocol);
    www_StripTail(status);
    www_StripTail(message);
    VOS_FREE(hdrs->protocol);
    hdrs->protocol = VOS_STRDUP(protocol);
    hdrs->status_code = atoi(status); /* TBD: add sanity check */
    VOS_FREE(hdrs->message);
    hdrs->message = VOS_STRDUP(message);
    vosLog_debug("proto_ParseResponse(protocol=\"%s\", status=%d message=\"%s\")",
                 hdrs->protocol, hdrs->status_code, hdrs->message);

    return 0; /* OK */
}

static char HostStr[] = "Host:";
static char ConnectionStr[] = "Connection:";
static char SetCookieStr[] = "Set-Cookie:";
static char SetCookieStr2[] = "Set-Cookie2:";
static char ContentLthStr[] = "Content-Length:";
static char ContentTypeStr[] = "Content-Type:";
static char WWWAuthenticateStr[] = "WWW-Authenticate:";
static char AuthorizationStr[] = "Authorization:";
static char TransferEncoding[] = "Transfer-Encoding:";
static char LocationStr[] = "Location:";

static void addCookieHdr(CookieHdr **p, char *c)
{
    CookieHdr   *newCookie = (CookieHdr *)VOS_MALLOC_FLAGS(sizeof(CookieHdr), 0);
    char   *cp;

    if ((cp = strchr(c, '=')) != NULL)
    {
        newCookie->next = *p;
        newCookie->name = (char *)VOS_STRNDUP(c, (UINT32)(cp - c));
        newCookie->value = VOS_STRDUP(cp + 1);
        *p = newCookie;
    }
    else
    {
        VOS_FREE(newCookie);
    }
}
/*----------------------------------------------------------------------*
 * hdrs->type needs to be initiated
 * Only read headers according to type
 * Reads all headers until an empty '\r\n" is found.
 */
void proto_ParseHdrs(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char buf[BUF_SIZE_MAX];
    char *cp;
    int n;

#ifdef DEBUGHDRS
    vosLog_debug("proto_ParseHdrs() pc=%p pc->type=%d", pc, pc->type);
#endif

    /* Parse the rest of the request headers. */
    while ((n = proto_Readline(pc, buf, BUF_SIZE_MAX)) > 0)
    {
        www_StripTail(buf);
#ifdef DEBUGHDRS
        vosLog_debug("  read \"%s\"", buf);
#endif
        if (strcmp(buf, "") == 0)
        {
            break;
        }
        else if (strncasecmp(buf, HostStr, sizeof(HostStr) - 1) == 0)
        {
            cp = &buf[sizeof(HostStr) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->host);
            hdrs->host = VOS_STRDUP(cp);
        }
        else if (strncasecmp(buf, ContentLthStr, sizeof(ContentLthStr) - 1) == 0)
        {
            cp = &buf[sizeof(ContentLthStr) - 1];
            cp += strspn(cp, " \t");
            hdrs->content_length = atoi(cp);
        }
        else if (strncasecmp(buf, ContentTypeStr, sizeof(ContentTypeStr) - 1) == 0)
        {
            cp = &buf[sizeof(ContentTypeStr) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->content_type);
            hdrs->content_type = VOS_STRDUP(cp);
        }
        else if (strncasecmp(buf, ConnectionStr, sizeof(ConnectionStr) - 1) == 0)
        {
            cp = &buf[sizeof(ConnectionStr) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->Connection);
            hdrs->Connection = VOS_STRDUP(cp);
        }
        else if (strncasecmp(buf, WWWAuthenticateStr, sizeof(WWWAuthenticateStr) - 1) == 0)
        {
            cp = &buf[sizeof(WWWAuthenticateStr) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->wwwAuthenticate);
            hdrs->wwwAuthenticate = VOS_STRDUP(cp);
        }
        else if (strncasecmp(buf, AuthorizationStr, sizeof(AuthorizationStr) - 1) == 0)
        {
            cp = &buf[sizeof(AuthorizationStr) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->Authorization);
            hdrs->Authorization = VOS_STRDUP(cp);
        }
        else if (strncasecmp(buf, TransferEncoding, sizeof(TransferEncoding) - 1) == 0)
        {
            cp = &buf[sizeof(TransferEncoding) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->TransferEncoding);
            hdrs->TransferEncoding = VOS_STRDUP(cp);
        }
        else if (strncasecmp(buf, LocationStr, sizeof(LocationStr) - 1) == 0)
        {
            cp = &buf[sizeof(LocationStr) - 1];
            cp += strspn(cp, " \t");
            VOS_FREE(hdrs->locationHdr);
            hdrs->locationHdr = VOS_STRDUP(cp);
        }
        else if ((strncasecmp(buf, SetCookieStr, sizeof(SetCookieStr) - 1) == 0)
                 || (strncasecmp(buf, SetCookieStr2, sizeof(SetCookieStr2) - 1) == 0))
        {
            char *c;
            cp = &buf[sizeof(SetCookieStr) - 1];
            cp += strspn(cp, " \t:");   /* colon is added to skip : in SetCookieStr2 str*/
            /* don't need anything after ";" if it exists */
            if ((c = strstr(cp, ";")) != NULL)
            {
                *c = '\0';
            }
            addCookieHdr(&hdrs->setCookies, cp);
        }


    }

#ifdef DEBUGHDRS
    vosLog_debug("proto_ParseHdrs done.");
#endif
}

/*----------------------------------------------------------------------*/
void proto_ParsePost(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char *data;
    size_t n ;
    int len;

    vosLog_debug("proto_ParsePost() to read %d bytes", hdrs->content_length);
    len = hdrs->content_length;
    data = (char *)VOS_MALLOC_FLAGS((UINT32)len + 1, 0); /* make room for terminating '\0' */
    n = proto_Readn(pc, data, len);
    if (n > 0)
        data[n] = '\0';
    else
        data[0] = '\n';
    vosLog_debug("proto_ParsePost() read %d bytes \"%s\"", n, data);
    VOS_FREE(hdrs->arg);
    hdrs->arg = data;

    proto_Skip(pc);
}

/*----------------------------------------------------------------------*
 * discard all there is to read on the in buffer
 * This is used since some stupid browsers (e.g. IE) sends more data
 * than specified in the content-lenght header
 * Returns result of last read():
 *     0 - eof
 *     -1 - connection error.
 *      1 - no data, possibly more.
 */
int proto_Skip(tProtoCtx *pc)
{
    char c;
    int nread = 0;
    int ret = 0;
    long flags, nbflags;

    vosLog_debug("proto_Skip() read all from fd and ignore");

    flags = (long) fcntl(pc->fd, F_GETFL);
    nbflags = flags | O_NONBLOCK;
    fcntl(pc->fd, F_SETFL, nbflags);

    do
    {
        switch (pc->type)
        {
            case iNormal:
                nread = read(pc->fd, &c, 1);
                break;
            case iSsl:
                if (SF_FEATURE_SUPPORT_TR69C_SSL)
                {
                    nread = SSL_read(pc->ssl, &c, 1);
                }
                break;
            default:
                vosLog_error("Impossible error; illegal ProtoCtx type (%d)", pc->type);
                break;
        }
        if (nread < 0)
        {
            ret = errno == EAGAIN ? 1 : -1;
            break;
        }
    }
    while (nread > 0);
    fcntl(pc->fd, F_SETFL, flags);

    vosLog_debug("proto_Skip() done.ret=%d", ret);
    return ret;
}
