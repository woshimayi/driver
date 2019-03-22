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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>

#include "../inc/tr69cdefs.h"
#include "../inc/types.h"
#include "util_defs.h"
#include "www.h"
#include "vos_log.h"
#include "fwk.h"
 
/* #define DEBUG 1   */


extern ACSState acsState;

/*----------------------------------------------------------------------*
 * Converts hexadecimal to decimal (character):
 */
static char hexToDec(char *what)
{
   char digit;

   digit = (char)(what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
   digit *= 16;
   digit += (char)(what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));

   return (digit);
}

/*----------------------------------------------------------------------*
 * Unescapes "%"-escaped characters in a query:
 */
static void unescapeUrl(char *url)
{
   int x, y, len;

   len = util_strlen(url);
   for (x = 0, y = 0; url[y]; x++, y++) {
      if ((url[x] = url[y]) == '%' && y < (len - 2)) {
          url[x] = hexToDec(&url[y+1]);
          y += 2;
      }
   }
   url[x] = '\0';
}

/*----------------------------------------------------------------------*/
void www_UrlDecode(char *s)
{
   char *pstr = s;

   /* convert plus (+) to space (' ') */
   for (pstr = s; pstr != NULL && *pstr != '\0'; pstr++) {
       if (*pstr == '+')
         *pstr = ' ';
   }
   unescapeUrl(s);
}

/*----------------------------------------------------------------------*/
void www_UrlEncode(const char *s, char *t)
{
  while (*s) {
    if (*s == ' ') {
      *t++ = '+';
    } else if (isalnum(*s)) {
      *t++ = *s;
    } else {
      /* hex it */
      *t++ = '%';
      UTIL_SNPRINTF(t,32, "%2x", *s);
      t += 2;
    }
    s++;
  }
  *t = '\0';
}

/*----------------------------------------------------------------------*
 * parse url on form:
 *  "<proto>://<host>[:<port>][<uri>]"
 *  returns
 *    0 if parse ok
 *   -1 if parse failed
 *  port sets to 0 if no port is specified in URL
 *  uri is set to "" if no URI is specified
 */
int www_ParseUrl(const char *url, char *proto, char *host, int *port, char *uri)
{
  int n;
  char *p;

  *port = 0;
  UTIL_STRNCPY(uri, "",8);

  /* proto */
  p = (char *) url;
  if ((p = strchr(url, ':')) == NULL) {
    return -1;
  }
  n = p - url;
  strncpy(proto, url, n);
  proto[n] = '\0';

  /* skip "://" */
  if (*p++ != ':') return -1;
  if (*p++ != '/') return -1;
  if (*p++ != '/') return -1;

  /* host */
  {
    char *hp = host;
    
    while (*p && *p != ':' && *p != '/') {
      *hp++ = *p++;
    }
    *hp = '\0';
  }
  if (strlen(host) == 0)
    return -1;

  /* end */
  if (*p == '\0') {
    *port = 0;
    UTIL_STRNCPY(uri, "",8);
    return 0;
  }

  /* port */
  if (*p == ':') {
    char buf[10];
    char *pp = buf;

    p++;
    while (isdigit(*p)) {
      *pp++ = *p++;
    }
    *pp = '\0';
    if (strlen(buf) == 0)
      return -1;
    *port = atoi(buf);
  }
  
  /* uri */
  if (*p == '/') {
    char *up = uri;
    while ((*up++ = *p++));
  }

  return 0;
}

/*----------------------------------------------------------------------*
 * returns
 *  0   if ok  (fd contains descriptor for connection)
 *  -1  if socket couldn't be created
 *  -2  if connection function could not be started.
 *
 * sock_fd will hold the socket.
 * The caller of www_Establishconnection must wait until write is possible
 * i.e. setListenerType(sockfd, ..., iListener_Write)
 * this to avoid blocking.
 */
int www_EstablishConnection(tIpAddr host_addr, int port, int *sock_fd)
{
    int fd;
    struct sockaddr_in sa;
    long flags;
    int res;
    struct ifreq ifr;
  
    memset(&sa, 0, sizeof(sa));
    sa.sin_family       = AF_INET;
    sa.sin_addr.s_addr  = htonl(host_addr);
    sa.sin_port         = htons(port);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        return -1;
    }

    UTIL_STRNCPY(ifr.ifr_name, acsState.boundIfName, sizeof(ifr.ifr_name));

    if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0)
    {
        vosLog_error("bind socket to device error!");
    }

    /* set non-blocking */
    flags = (long) fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    res = fcntl(fd, F_SETFL, flags);

    errno = 0;
    if (connect(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) 
    {
        if (errno != EINPROGRESS) 
        {
            /* connect failed */
            close(fd);
            return -2;
        }
    }

    /* save the connection interface name for later deciding if
    * it is a WAN or LAN interface in the uploading process
    */
#if 0//zanshi
    if (cmsImg_saveIfNameFromSocket(fd, connIfName) != VOS_RET_SUCCESS)
    {
        vosLog_error("Fail to get ifName from socket");
    }         
#endif
    *sock_fd = fd;
    return 0;
}

/*----------------------------------------------------------------------
 * removes any trailing whitespaces, \r and \n
 * it destroys its argument...
 */
void www_StripTail(char *s)
{
  if (*s != '\0') {
    while(*s) s++;
    s--;
    while(*s == '\r' || *s == '\n' || *s == ' ' || *s == '\t') {
      *s = '\0';
      s--;
    }
  }
}


/*======================================================================*
 * module test code
 *======================================================================*/

#ifdef TEST
int main(int argc, char **argv)
{
    int res;
    char proto[256];
    char host[256];
    int port;
    char uri[256];

    strcpy(proto, "");

    res = www_ParseUrl(argv[1], proto, host, &port, uri);

    printf("result=%d\n", res);
    printf("proto= \"%s\"\n", proto);
    printf("host=  \"%s\"\n", host);
    printf("port=  %d\n", port);
    printf("uri=   \"%s\"\n", uri);
    return 0;
}
#endif
