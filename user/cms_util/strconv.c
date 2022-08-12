/***********************************************************************
 *
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for strcasestr */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>     /* for isDigit, really should be in oal_strconv.c */
#include <sys/stat.h>  /* this really should be in oal_strconv.c */
#include <arpa/inet.h> /* for inet_aton */
#include <sys/time.h> /* for inet_aton */

#include "cms_util.h"
#include "oal.h"
#include "uuid.h"

#include <net/if.h> //Added by xuyong,2015-10-16
#include <sys/ioctl.h> //Added by xuyong,2015-10-16
#include <errno.h> //Added by xuyong,2015-10-16
#include "md5.h"
#include "bcm_fsutils.h"


UBOOL8 cmsUtl_isValidVpiVci(SINT32 vpi, SINT32 vci)
{
   if (vpi >= VPI_MIN && vpi <= VPI_MAX && vci >= VCI_MIN && vci <= VCI_MAX)
   {
      return TRUE;
   }
   
   cmsLog_error("invalid vpi/vci %d/%d", vpi, vci);
   return FALSE;
}

CmsRet cmsUtl_atmVpiVciStrToNum(const char *vpiVciStr, SINT32 *vpi, SINT32 *vci)
{
   char *pSlash;
   char vpiStr[BUFLEN_256];
   char vciStr[BUFLEN_256];
   char *prefix;
   
   *vpi = *vci = -1;   
   if (vpiVciStr == NULL)
   {
      cmsLog_error("vpiVciStr is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }      

   strncpy(vpiStr, vpiVciStr, sizeof(vpiStr));

   if (strstr(vpiStr, DSL_LINK_DESTINATION_PREFIX_SVC))
   {
      cmsLog_error("DesitinationAddress string %s with %s is not supported yet.", vpiStr, DSL_LINK_DESTINATION_PREFIX_SVC);
      return CMSRET_INVALID_PARAM_VALUE;
   }

   if ((prefix = strstr(vpiStr, DSL_LINK_DESTINATION_PREFIX_PVC)) == NULL)
   {
      cmsLog_error("Invalid DesitinationAddress string %s", vpiStr);
      return CMSRET_INVALID_PARAM_VALUE;
   }
 
   /* skip the prefix */
#if 0
   prefix += sizeof(DSL_LINK_DESTINATION_PREFIX_PVC);
#endif
   prefix += strlen(DSL_LINK_DESTINATION_PREFIX_PVC);
   /* skip the blank if there is one */
   if (*prefix == ' ')
   {
      prefix += 1;
   }

   pSlash = (char *) strchr(prefix, '/');
   if (pSlash == NULL)
   {
      cmsLog_error("vpiVciStr %s is invalid", vpiVciStr);
      return CMSRET_INVALID_ARGUMENTS;
   }
   strncpy(vciStr, (pSlash + 1), sizeof(vciStr));
   *pSlash = '\0';       
   *vpi = atoi(prefix);
   *vci = atoi(vciStr);
   if (cmsUtl_isValidVpiVci(*vpi, *vci) == FALSE)
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }     

   return CMSRET_SUCCESS;
   
}


CmsRet cmsUtl_atmVpiVciNumToStr(const SINT32 vpi, const SINT32 vci, char *vpiVciStr)
{
   if (vpiVciStr == NULL)
   {
      cmsLog_error("vpiVciStr is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }         
   if (cmsUtl_isValidVpiVci(vpi, vci) == FALSE)
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }     

   sprintf(vpiVciStr, "%s %d/%d", DSL_LINK_DESTINATION_PREFIX_PVC, vpi, vci);

   return CMSRET_SUCCESS;
}


static int strnlen_safe(const char *str, int max) {
  // note: strnlen is not supported on all compilers -- create our own version
  const char * end = memchr(str, '\0', max+1);
  if (end && *end == '\0') {
       return end-str;
  }
  return -1;
}


CmsRet cmsUtl_macStrToNum(const char *macStr, UINT8 *macNum) 
{
   SINT32 i;
   UINT32 macStrLen;
   
   if (macNum == NULL || macStr == NULL) 
   {
      cmsLog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return CMSRET_INVALID_ARGUMENTS;
   }    
   macStrLen = strnlen_safe(macStr, 19);
   
   i=sscanf(macStr, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
          &(macNum[0]), &(macNum[1]), &(macNum[2]), &(macNum[3]), &(macNum[4]), &(macNum[5]));

   if (i != 6 && macStrLen == 14) {
     i=sscanf(macStr, "%2hhx%2hhx:%2hhx%2hhx:%2hhx%2hhx", 
              &(macNum[0]), &(macNum[1]), &(macNum[2]), &(macNum[3]), &(macNum[4]), &(macNum[5]));
   }

   if (i != 6 && macStrLen == 12) {
     i=sscanf(macStr, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", 
              &(macNum[0]), &(macNum[1]), &(macNum[2]), &(macNum[3]), &(macNum[4]), &(macNum[5]));
   }

   if (i != 6) {
      return CMSRET_INVALID_PARAM_VALUE;
   }
   
   return CMSRET_SUCCESS;
   
}

CmsRet cmsUtl_macNumToStr(const UINT8 *macNum, char *macStr) 
{
   if (macNum == NULL || macStr == NULL) 
   {
      cmsLog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return CMSRET_INVALID_ARGUMENTS;
   }  

   sprintf(macStr, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macNum[0], (UINT8) macNum[1], (UINT8) macNum[2],
           (UINT8) macNum[3], (UINT8) macNum[4], (UINT8) macNum[5]);

   return CMSRET_SUCCESS;
}

CmsRet cmsUtl_macToNumStr(const char *macStr, char *macNumStr)
{
	UINT8 macNum[MAC_ADDR_LEN] = {0};
	if (macStr == NULL || macNumStr == NULL)
	{
		cmsLog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
		return CMSRET_INVALID_ARGUMENTS;
	}

	if (cmsUtl_macStrToNum(macStr, macNum) == CMSRET_SUCCESS)
		sprintf(macNumStr, "%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X",
			(UINT8) macNum[0], (UINT8) macNum[1], (UINT8) macNum[2],
			(UINT8) macNum[3], (UINT8) macNum[4], (UINT8) macNum[5]);
	else 
		return CMSRET_INVALID_ARGUMENTS;

	return CMSRET_SUCCESS;
}

CmsRet cmsUtl_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val)
{
   return(oal_strtol(str, endptr, base, val));
}


CmsRet cmsUtl_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val)
{
   return(oal_strtoul(str, endptr, base, val));
}


CmsRet cmsUtl_strtol64(const char *str, char **endptr, SINT32 base, SINT64 *val)
{
   return(oal_strtol64(str, endptr, base, val));
}


CmsRet cmsUtl_strtoul64(const char *str, char **endptr, SINT32 base, UINT64 *val)
{
   return(oal_strtoul64(str, endptr, base, val));
}


void cmsUtl_strToLower(char *string)
{
   char *ptr = string;
   for (ptr = string; *ptr; ptr++)
   {
       *ptr = tolower(*ptr);
   }
}

void cmsUtl_strToUpper(char *string)
{
   char *ptr = string;
   for (ptr = string; *ptr; ptr++)
   {
       *ptr = toupper(*ptr);
   }
}

CmsRet cmsUtl_parseUrl(const char *url, UrlProto *proto, char **addr, UINT16 *port, char **path)
{
   int n = 0;
   char *p = NULL;
   char protocol[BUFLEN_16];
   char host[BUFLEN_1024];
   char uri[BUFLEN_1024];

   if (url == NULL)
   {
      cmsLog_debug("url is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }

  *port = 0;
   protocol[0] = host[0]  = uri[0] = '\0';

   /* proto */
   p = (char *) url;
   if ((p = strchr(url, ':')) == NULL) 
   {
      return CMSRET_INVALID_ARGUMENTS;
   }
   n = p - url;
   strncpy(protocol, url, n);
   protocol[n] = '\0';

   if (!strcmp(protocol, "http"))
   {
      *proto = URL_PROTO_HTTP;
   }
   else if (!strcmp(protocol, "https"))
   {
      *proto = URL_PROTO_HTTPS;
   }
   else if (!strcmp(protocol, "ftp"))
   {
      *proto = URL_PROTO_FTP;
   }
   else if (!strcmp(protocol, "tftp"))
   {
      *proto = URL_PROTO_TFTP;
   }
   else
   {
      cmsLog_error("unrecognized proto in URL %s", url);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* skip "://" */
   if (*p++ != ':') return CMSRET_INVALID_ARGUMENTS;
   if (*p++ != '/') return CMSRET_INVALID_ARGUMENTS;
   if (*p++ != '/') return CMSRET_INVALID_ARGUMENTS;

   /* host */
   {
      char *pHost = host;
      char endChar1 = ':';  // by default, host field ends if a colon is seen
      char endChar2 = '/';  // by default, host field ends if a / is seen

#ifdef SUPPORT_IPV6
      if (*p && *p == '[')
      {
         /*
          * Square brackets are used to surround IPv6 addresses in : notation.
          * So if a [ is detected, then keep scanning until the end bracket
          * is seen.
          */
         endChar1 = ']';
         endChar2 = ']';
         p++;  // advance past the [
      }
#endif

      while (*p && *p != endChar1 && *p != endChar2)
      {
         *pHost++ = *p++;
      }
      *pHost = '\0';

#ifdef SUPPORT_IPV6
      if (endChar1 == ']')
      {
         // if endChar is ], then it must be found
         if (*p != endChar1)
         {
            return CMSRET_INVALID_ARGUMENTS;
         }
         else
         {
            p++;  // advance past the ]
         }
      }
#endif
   }
   if (strlen(host) != 0)
   {
      *addr = cmsMem_strdup(host);
   }
   else
   {
      cmsLog_error("unrecognized host in URL %s", url);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* end */
   if (*p == '\0') 
   {
      *path = cmsMem_strdup("/");
       return CMSRET_SUCCESS;
   }

   /* port */
   if (*p == ':') 
   {
      char buf[BUFLEN_16];
      char *pBuf = buf;

      p++;
      while (isdigit(*p)) 
      {
         *pBuf++ = *p++;
      }
      *pBuf = '\0';
      if (strlen(buf) == 0)
      {
         CMSMEM_FREE_BUF_AND_NULL_PTR(*addr);
         cmsLog_error("unrecognized port in URL %s", url);
         return CMSRET_INVALID_ARGUMENTS;
      }
      *port = atoi(buf);
   }
  
   /* path */
   if (*p == '/') 
   {
      char *pUri = uri;

      while ((*pUri++ = *p++));
      *path = cmsMem_strdup(uri);  
   }
   else
   {
      *path = cmsMem_strdup("/");
   }

   return CMSRET_SUCCESS;
}

CmsRet cmsUtl_getBaseDir(char *pathBuf, UINT32 pathBufLen)
{
   return (bcmUtl_getBaseDir(pathBuf, pathBufLen));
}

CmsRet cmsUtl_getRunTimeRootDir(char *pathBuf, UINT32 pathBufLen)
{
   UINT32 rc;

#if defined(DESKTOP_LINUX) && !defined(BUILD_DESKTOP_BEEP)
   char baseDir[CMS_MAX_FULLPATH_LENGTH]={0};
   char tmpPath[CMS_MAX_FULLPATH_LENGTH]={0};
   char profileBuf[BUFLEN_64];
   UINT32 bufsize = sizeof(profileBuf);
   CmsRet ret;

   ret = cmsUtl_getBaseDir(baseDir, sizeof(baseDir));
   if (ret != CMSRET_SUCCESS)
   {
      return ret;
   }

   rc = snprintf(tmpPath, sizeof(tmpPath), "%s/.last_profile", baseDir);
   if (rc >= sizeof(tmpPath))
   {
      cmsLog_error("CMS_MAX_FULLPATH_LENGTH (%d) exceeded", CMS_MAX_FULLPATH_LENGTH);
      return CMSRET_RESOURCE_EXCEEDED;
   }

   ret = cmsFil_copyToBuffer(tmpPath, (UINT8 *) profileBuf, &bufsize);
   if (ret != CMSRET_SUCCESS)
   {
      cmsLog_error("Error %d reading .last_profile file at %s (bufsize=%d)",
                   ret, tmpPath, bufsize);
      return ret;
   }

   /* truncate any newlines at the end of the buffer */
   {
      UINT32 i;

      for (i=0; i < bufsize; i++)
      {
         if (profileBuf[i] == '\n' || profileBuf[i] == '\r' || profileBuf[i] == '\t')
         {
            profileBuf[i] = '\0';
         }
      }
   }


   rc = snprintf(pathBuf, pathBufLen,
                 "%s/targets/%s/fs.install", baseDir, profileBuf);
   cmsLog_debug("returning %s", pathBuf);

#else

   rc = snprintf(pathBuf, pathBufLen, "/");

#endif /* DESKTOP_LINUX */

   if (rc >= pathBufLen)
   {
      cmsLog_error("pathBufLen %d is too small for buf %s", pathBufLen, pathBuf);
      return CMSRET_RESOURCE_EXCEEDED;
   }

   return CMSRET_SUCCESS;
}


CmsRet cmsUtl_getRunTimePath(const char *target, char *pathBuf, UINT32 pathBufLen)
{
   CmsRet ret;

   ret = cmsUtl_getRunTimeRootDir(pathBuf, pathBufLen);
   if (ret != CMSRET_SUCCESS)
   {
      return ret;
   }

   /*
    * If the runtime root dir is just "/", and the target path already
    * begins with a "/", then just use target path to avoid beginning
    * the path with "//".
    */
   if (pathBufLen > 1 && pathBuf[0] == '/' && pathBuf[1] == '\0')
   {
      SINT32 rc;
      rc = snprintf(pathBuf, pathBufLen, "%s", target);
      if (rc >= (SINT32) pathBufLen)
      {
         cmsLog_error("pathBufLen %d is too small for target %s",
                      pathBufLen, pathBuf);
         ret = CMSRET_RESOURCE_EXCEEDED;
      }
   }
   else
   {
      ret = cmsUtl_strncat(pathBuf, pathBufLen, target);
   }

   return ret;
}

CmsRet cmsUtl_parseDNS(const char *inDsnServers, char *outDnsPrimary, char *outDnsSecondary, UBOOL8 isIPv4)
{
   CmsRet ret = CMSRET_SUCCESS;
   char *tmpBuf;
   char *separator;
   char *separator1;
   UINT32 len;

   if (inDsnServers == NULL)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }      
   

   cmsLog_debug("entered: DDNSservers=>%s<=, isIPv4<%d>", inDsnServers, isIPv4);

   if ( isIPv4 )
   {
      if (outDnsPrimary)
      {
         strcpy(outDnsPrimary, "0.0.0.0");
      }
   
      if (outDnsSecondary)
      {
         strcpy(outDnsSecondary, "0.0.0.0");
      }
   }

   len = strlen(inDsnServers);

   if ((tmpBuf = cmsMem_alloc(len+1, 0)) == NULL)
   {
      cmsLog_error("alloc of %d bytes failed", len);
      ret = CMSRET_INTERNAL_ERROR;
   }
   else
   {
      SINT32 af = isIPv4?AF_INET:AF_INET6;

      sprintf(tmpBuf, "%s", inDsnServers);
      separator = strstr(tmpBuf, ",");
      if (separator != NULL)
      {
         /* break the string into two strings */
         *separator = 0;
         separator++;
         while ((isspace(*separator)) && (*separator != 0))
         {
            /* skip white space after comma */
            separator++;
         }
         /* There might be 3rd DNS server, truncate it. */
         separator1 = strstr(separator, ",");
         if (separator1 != NULL)
          *separator1 = 0;

         if (outDnsSecondary != NULL)
         {
            if ( cmsUtl_isValidIpAddress(af, separator))
            {
               strcpy(outDnsSecondary, separator);
            }
            cmsLog_debug("dnsSecondary=%s", outDnsSecondary);
         }
      }

      if (outDnsPrimary != NULL)
      {
         if (cmsUtl_isValidIpAddress(af, tmpBuf))
         {
            strcpy(outDnsPrimary, tmpBuf);
         }
         cmsLog_debug("dnsPrimary=%s", outDnsPrimary);
      }

      cmsMem_free(tmpBuf);
   }

   return ret;
   
}


CmsRet cmsUtl_concatDNS(const char *dns1, const char *dns2, char *buf, UINT32 bufLen)
{
   CmsRet ret = CMSRET_SUCCESS;

   if (buf == NULL)
   {
      cmsLog_error("serverList is NULL!");
      return CMSRET_INVALID_ARGUMENTS;
   }
   buf[0] = '\0';

   if (!IS_EMPTY_STRING(dns1))
   {
      if (cmsUtl_strlen(dns1) >= (SINT32) bufLen)
      {
         cmsLog_error("dns1 %s too long for bufLen %d", dns1, bufLen);
         ret = CMSRET_RESOURCE_EXCEEDED;
      }
      else
      {
         strcat(buf, dns1);
      }
   }

   /*
    * If we we used dns1, then dns2 must be non-zero.
    * If dns1 was empty and thus skipped, dns2 only has to be non-empty.
    * This guarantees at most 1 zero addr in the buffer.
    */
   if (((buf[0] != '\0') && !cmsUtl_isZeroIpvxAddress(CMS_AF_SELECT_IPVX, dns2)) ||
       ((buf[0] == '\0') && !IS_EMPTY_STRING(dns2)))
   {
      if (cmsUtl_strlen(buf) + cmsUtl_strlen(dns2) + 1 >= (SINT32) bufLen)
      {
         cmsLog_error("dns2 %s cannot fit into bufLen %d", dns2, bufLen);
         ret = CMSRET_RESOURCE_EXCEEDED;
      }
      else
      {
         if (buf[0] != '\0')
         {
            strcat(buf, ",");
         }
         strcat(buf, dns2);
      }
   }

   return ret;
}


SINT32 cmsUtl_syslogModeToNum(const char *modeStr)
{
   SINT32 mode=1;

   /*
    * These values are hard coded in httpd/html/logconfig.html.
    * Any changes to these values must also be reflected in that file.
    */
   if (!strcmp(modeStr, MDMVS_LOCAL_BUFFER))
   {
      mode = 1;
   }
   else if (!strcmp(modeStr, MDMVS_REMOTE))
   {
      mode = 2;
   }
   else if (!strcmp(modeStr, MDMVS_LOCAL_BUFFER_AND_REMOTE))
   {
      mode = 3;
   }
   else 
   {
      cmsLog_error("unsupported mode string %s, default to mode=%d", modeStr, mode);
   }

   /*
    * The data model also specifies LOCAL_FILE and LOCAL_FILE_AND_REMOTE,
    * but its not clear if syslogd actually supports local file mode.
    */

   return mode;
}


char * cmsUtl_numToSyslogModeString(SINT32 mode)
{
   char *modeString = MDMVS_LOCAL_BUFFER;

   /*
    * These values are hard coded in httpd/html/logconfig.html.
    * Any changes to these values must also be reflected in that file.
    */
   switch(mode)
   {
   case 1:
      modeString = MDMVS_LOCAL_BUFFER;
      break;

   case 2:
      modeString = MDMVS_REMOTE;
      break;

   case 3:
      modeString = MDMVS_LOCAL_BUFFER_AND_REMOTE;
      break;

   default:
      cmsLog_error("unsupported mode %d, default to %s", mode, modeString);
      break;
   }

   /*
    * The data model also specifies LOCAL_FILE and LOCAL_FILE_AND_REMOTE,
    * but its not clear if syslogd actually supports local file mode.
    */

   return modeString;
}


UBOOL8 cmsUtl_isValidSyslogMode(const char * modeStr)
{
   UINT32 mode;

   if (cmsUtl_strtoul(modeStr, NULL, 10, &mode) != CMSRET_SUCCESS) 
   {
      return FALSE;
   }

   return ((mode >= 1) && (mode <= 3));
}


SINT32 cmsUtl_syslogLevelToNum(const char *levelStr)
{
   SINT32 level=3; /* default all levels to error */

   /*
    * These values are from /usr/include/sys/syslog.h.
    */
   if (!strcmp(levelStr, MDMVS_EMERGENCY))
   {
      level = 0;
   }
   else if (!strcmp(levelStr, MDMVS_ALERT))
   {
      level = 1;
   }
   else if (!strcmp(levelStr, MDMVS_CRITICAL))
   {
      level = 2;
   }
   else if (!strcmp(levelStr, MDMVS_ERROR))
   {
      level = 3;
   }
   else if (!strcmp(levelStr, MDMVS_WARNING))
   {
      level = 4;
   }
   else if (!strcmp(levelStr, MDMVS_NOTICE))
   {
      level = 5;
   }
   else if (!strcmp(levelStr, MDMVS_INFORMATIONAL))
   {
      level = 6;
   }
   else if (!strcmp(levelStr, MDMVS_DEBUG))
   {
      level = 7;
   }
   else 
   {
      cmsLog_error("unsupported level string %s, default to level=%d", levelStr, level);
   }

   return level;
}


char * cmsUtl_numToSyslogLevelString(SINT32 level)
{
   char *levelString = MDMVS_ERROR;

   /*
    * These values come from /usr/include/sys/syslog.h.
    */
   switch(level)
   {
   case 0:
      levelString = MDMVS_EMERGENCY;
      break;

   case 1:
      levelString = MDMVS_ALERT;
      break;

   case 2:
      levelString = MDMVS_CRITICAL;
      break;

   case 3:
      levelString = MDMVS_ERROR;
      break;

   case 4:
      levelString = MDMVS_WARNING;
      break;

   case 5:
      levelString = MDMVS_NOTICE;
      break;

   case 6:
      levelString = MDMVS_INFORMATIONAL;
      break;

   case 7:
      levelString = MDMVS_DEBUG;
      break;

   default:
      cmsLog_error("unsupported level %d, default to %s", level, levelString);
      break;
   }

   return levelString;
}


UBOOL8 cmsUtl_isValidSyslogLevel(const char *levelStr)
{
   UINT32 level;

   if (cmsUtl_strtoul(levelStr, NULL, 10, &level) != CMSRET_SUCCESS) 
   {
      return FALSE;
   }

   return (level <= 7);
}


UBOOL8 cmsUtl_isValidSyslogLevelString(const char *levelStr)
{
   if ((!strcmp(levelStr, MDMVS_EMERGENCY)) ||
       (!strcmp(levelStr, MDMVS_ALERT)) ||
       (!strcmp(levelStr, MDMVS_CRITICAL)) ||
       (!strcmp(levelStr, MDMVS_ERROR)) ||
       (!strcmp(levelStr, MDMVS_WARNING)) ||
       (!strcmp(levelStr, MDMVS_NOTICE)) ||
       (!strcmp(levelStr, MDMVS_INFORMATIONAL)) ||
       (!strcmp(levelStr, MDMVS_DEBUG)))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


UBOOL8 cmsUtl_isprocessPid(const char *process)
{
	char line[128] = {0};
	FILE *fpin = NULL;
	int flag = FALSE;
	char cmd[128] = {0};

	if (NULL == cmd)
	{
		return -1;
	}
	snprintf(cmd, sizeof(cmd), "ps | pgrep %s", process);
	if ((fpin = popen(cmd, "r")) == NULL)
	{
		perror("popen error");
		return -1;
	}

	if (fgets(line, 128, fpin) == NULL) /* read from pipe */
	{
		//return -1;
	}
	else
	{
		if (atoi(line))
		{
			flag = TRUE;
		}
	}

	if (pclose(fpin) == -1)
	{
		perror("pclose error");
	}

	return flag;

}


SINT32 cmsUtl_pppAuthToNum(const char *authStr)
{
   SINT32 authNum = PPP_AUTH_METHOD_AUTO;  /* default is auto  */

   if (!strcmp(authStr, MDMVS_AUTO_AUTH))
   {
      authNum = PPP_AUTH_METHOD_AUTO;
   }
   else if (!strcmp(authStr, MDMVS_PAP))
   {
      authNum = PPP_AUTH_METHOD_PAP;
   }
   else if (!strcmp(authStr, MDMVS_CHAP))
   {
       authNum = PPP_AUTH_METHOD_CHAP;
   }
   else if (!strcmp(authStr, MDMVS_MS_CHAP))
   {
         authNum = PPP_AUTH_METHOD_MSCHAP;
   }
   else 
   {
      cmsLog_error("unsupported auth string %s, default to auto=%d", authStr, authNum);
   }

   return authNum;
   
}


char * cmsUtl_numToPppAuthString(SINT32 authNum)
{
   char *authStr = MDMVS_AUTO_AUTH;   /* default to auto */

   switch(authNum)
   {
   case PPP_AUTH_METHOD_AUTO:
      authStr = MDMVS_AUTO_AUTH;
      break;

   case PPP_AUTH_METHOD_PAP:
      authStr = MDMVS_PAP;
      break;

   case PPP_AUTH_METHOD_CHAP:
      authStr = MDMVS_CHAP;
      break;

   case PPP_AUTH_METHOD_MSCHAP:
      authStr = MDMVS_MS_CHAP; 
      break;

   default:
      cmsLog_error("unsupported authNum %d, default to %s", authNum, authStr);
      break;
   }

   return authStr;
   
}


CmsLogLevel cmsUtl_logLevelStringToEnum(const char *logLevel)
{
   if (!strcmp(logLevel, MDMVS_ERROR))
   {
      return LOG_LEVEL_ERR;
   }
   else if (!strcmp(logLevel, MDMVS_NOTICE))
   {
      return LOG_LEVEL_NOTICE;
   }
   else if (!strcmp(logLevel, MDMVS_DEBUG))
   {
      return LOG_LEVEL_DEBUG;
   }
   else
   {
      cmsLog_error("unimplemented log level %s", logLevel);
      return DEFAULT_LOG_LEVEL;
   }
}


CmsLogDestination cmsUtl_logDestinationStringToEnum(const char *logDest)
{
   if (!strcmp(logDest, MDMVS_STANDARD_ERROR))
   {
      return LOG_DEST_STDERR;
   }
   else if (!strcmp(logDest, MDMVS_SYSLOG))
   {
      return LOG_DEST_SYSLOG;
   }
   else if (!strcmp(logDest, MDMVS_TELNET))
   {
      return LOG_DEST_TELNET;
   }
   else
   {
      cmsLog_error("unimplemented log dest %s", logDest);
      return DEFAULT_LOG_DESTINATION;
   }
}


UBOOL8 cmsUtl_isValidIpAddress(SINT32 af, const char* address)
{
   if ( IS_EMPTY_STRING(address) ) return FALSE;
#ifdef SUPPORT_IPV6
   if (af == AF_INET6)
   {
      struct in6_addr in6Addr;
      UINT32 plen;
      char   addr[CMS_IPADDR_LENGTH];

      if (cmsUtl_parsePrefixAddress(address, addr, &plen) != CMSRET_SUCCESS)
      {
         cmsLog_debug("Invalid ipv6 address=%s", address);
         return FALSE;
      }

      if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
      {
         cmsLog_debug("Invalid ipv6 address=%s", address);
         return FALSE;
      }

      return TRUE;
   }
   else
#endif
   {
      if (af == AF_INET)
      {
         return cmsUtl_isValidIpv4Address(address);
      }
      else
      {
         return FALSE;
      }
   }
}  /* End of cmsUtl_isValidIpAddress() */

UBOOL8 cmsUtl_isValidIpv4Address(const char* input)
{
   UBOOL8 ret = TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_16];
   UINT32 i = 0, num;

   if (input == NULL || strlen(input) < 7 || strlen(input) > 15)
   {
      return FALSE;
   }

   // check the input is end with digit
   if (!isdigit(input[strlen(input)-1]))
      return FALSE;

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* IP address has the following format
      xxx.xxx.xxx.xxx where x is decimal number */
   pToken = strtok_r(buf, ".", &pLast);
   if ((cmsUtl_strtoul(pToken, NULL, 10, &num) != CMSRET_SUCCESS) ||
       (num > 255))
   {
      ret = FALSE;
   }
   else
   {
      while (++i)
      {
         pToken = strtok_r(NULL, ".", &pLast);
         if (pToken == NULL)
            break;

         if ((cmsUtl_strtoul(pToken, NULL, 10, &num) != CMSRET_SUCCESS) ||
             (num > 255))
         {
            ret = FALSE;
            break;
         }
      }

      // make sure only 4 numbers separate by '.'
      if (i != 4) 
         ret = FALSE;
   }

   return ret;
}

UBOOL8 cmsUtl_isZeroIpvxAddress(UINT32 ipvx, const char *addr)
{
   if (IS_EMPTY_STRING(addr))
   {
      return TRUE;
   }

   /*
    * Technically, the ::/0 is not an all zero address, but it is used by our
    * routing code to specify the default route.  See Wikipedia IPv6_address
    */
   if (((ipvx & CMS_AF_SELECT_IPV4) && !strcmp(addr, "0.0.0.0")) ||
       ((ipvx & CMS_AF_SELECT_IPV6) &&
           (!strcmp(addr, "0:0:0:0:0:0:0:0") ||
            !strcmp(addr, "::") ||
            !strcmp(addr, "::/128") ||
            !strcmp(addr, "::/0")))  )
   {
      return TRUE;
   }

   return FALSE;
}


UBOOL8 cmsUtl_isValidMacAddress(const char* input)
{
   UBOOL8 ret =  TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_32];
   UINT32 i, num;

   if (input == NULL || strlen(input) != MAC_STR_LEN)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   if ((strlen(pToken) != 2) ||
       (cmsUtl_strtoul(pToken, NULL, 16, &num) != CMSRET_SUCCESS))
   {
      ret = FALSE;
   }
   else
   {
      for ( i = 0; i < 5; i++ )
      {
         pToken = strtok_r(NULL, ":", &pLast);
         if ((strlen(pToken) != 2) ||
             (cmsUtl_strtoul(pToken, NULL, 16, &num) != CMSRET_SUCCESS))
         {
            ret = FALSE;
            break;
         }
      }
   }

   return ret;
}


UBOOL8 cmsUtl_isValidPortNumber(const char * portNumberStr)
{
   UINT32 portNum;

   if (cmsUtl_strtoul(portNumberStr, NULL, 10, &portNum) != CMSRET_SUCCESS) 
   {
      return FALSE;
   }

   return (portNum < (64 * 1024));
}

int cmsUtl_ipv6compare(char * min, char * max)
{
	struct in6_addr ip1;
	struct in6_addr ip2;
	int result = 0;

	inet_pton(AF_INET6, min, &ip1);
	inet_pton(AF_INET6, max, &ip2);

	result = memcmp(&ip1, &ip2, sizeof(struct in6_addr));

	return result;
}

int cmsUtl_ipv4compare(char * min, char * max)
{
	struct in_addr ip1;
	struct in_addr ip2;
	int result = 0;

	inet_pton(AF_INET, min, &ip1);
	inet_pton(AF_INET, max, &ip2);

	result = memcmp(&ip1, &ip2, sizeof(struct in_addr));

	return result;
}


SINT32 cmsUtl_strcmp(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcmp(str1, str2);
}


SINT32 cmsUtl_strcasecmp(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcasecmp(str1, str2);
}


SINT32 cmsUtl_strncmp(const char *s1, const char *s2, SINT32 n) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strncmp(str1, str2, n);
}


SINT32 cmsUtl_strncasecmp(const char *s1, const char *s2, SINT32 n) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strncasecmp(str1, str2, n);
}


char *cmsUtl_strstr(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *)s1;
   char *str2 = (char *)s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strstr(str1, str2);
}

char *cmsUtl_strchr(const char *s1, const char ch)
{
	if (s1 == NULL)
	{
		return NULL;
	}

	return strchr(s1, ch);
}

char* util_strupr(const char * s1)
{
	if (s1 == NULL)
	{
		return NULL;
	}

	int len = strlen(s1)+1;
	char tmp[len];
	char str[len];
	strncpy(str, s1, sizeof(str));
	int i = 0;

	while('\0' != str[i])
	{
		tmp[i] = toupper(str[i]);
		i++;
	}
	tmp[i] = '\0';

	return cmsMem_strdup(tmp);
}

char* util_strlwr(const char * s1)
{
	if (s1 == NULL)
	{
		return NULL;
	}

	int len = strlen(s1);
	char tmp[len + 1];
	char str[len + 1];
	strncpy(str, s1, sizeof(str));
	int i = 0;

	while('\0' != str[i])
	{
		tmp[i] = tolower(str[i]);
		i++;
	}
	tmp[i] = '\0';

	return cmsMem_strdup(tmp);
}

char *cmsUtl_strcasestr(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *)s1;
   char *str2 = (char *)s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcasestr(str1, str2);
}


char *cmsUtl_strcpy(char *dest, const char *src)
{

   /* if the dest ptr is NULL, we cannot copy at all.  Return now */
   if (dest == NULL)
   {
      cmsLog_error("dest is NULL!");
      return NULL;
   }

   /* if src ptr is NULL, copy an empty string to dest */
   if (src == NULL)
   {
      return strcpy(dest, "");
   }
   else
   {
      /* both dest and src are valid, do real strcpy */
      return strcpy(dest, src);
   }
}


char *cmsUtl_strcat(char *dest, const char *src)
{

   /* if the dest ptr is NULL, we cannot copy at all.  Return now */
   if (dest == NULL)
   {
      cmsLog_error("dest is NULL!");
      return NULL;
   }

   /* if src ptr is NULL, dest is unchanged, so just return dest */
   if (src == NULL)
   {
      return dest;
   }
   else
   {
      /* both dest and src are valid, do real strcat */
      return strcat(dest, src);
   }
}


char *cmsUtl_strncpy(char *dest, const char *src, SINT32 dlen)
{

   /* if the dest ptr is NULL, we cannot copy at all.  Return now */
   if (dest == NULL)
   {
      cmsLog_error("dest is NULL!");
      return NULL;
   }

   /* if src ptr is NULL, copy an empty string to dest */
   if (src == NULL)
   {
      return strcpy(dest, "");
      return dest;
   }

   /* do a modified strncpy by making sure dest is NULL terminated */
   if( strlen(src)+1 > (UINT32) dlen )
   {
      cmsLog_notice("truncating:src string length > dest buffer");
      strncpy(dest,src,dlen-1);
      dest[dlen-1] ='\0';
   }
   else
   {
      strcpy(dest,src);
   }
   return dest;
} 


CmsRet cmsUtl_strncat(char *prefix, UINT32 prefixLen, const char *suffix)
{
   CmsRet ret=CMSRET_SUCCESS;
   UINT32 copyLen;

   if((prefix == NULL) || (suffix == NULL))
   {
      cmsLog_error("null pointer reference src=%p dest =%p", prefix, suffix);
      return CMSRET_INVALID_ARGUMENTS;
   }

   if(strlen(prefix) + strlen(suffix) + 1 > prefixLen )
   {
      cmsLog_error("supplied prefix buffer (len=%d) is too small for %s + %s",
            prefixLen, prefix, suffix);
      ret = CMSRET_RESOURCE_EXCEEDED;  /* set error, but copy what we can */
   }

   copyLen = prefixLen - strlen(prefix) - 1;
   strncat(prefix, suffix, copyLen);

   return ret;
}


SINT32 cmsUtl_strlen(const char *src)
{
   char emptyStr[1] = {0};
   char *str = (char *)src;
   
   if(src == NULL)
   {
      str = emptyStr;
   }	

   return strlen(str);
} 

char *cmsUtl_findInList(const char *haystack, const char *needle)
{
   const char *ptr = haystack;
   int needle_len = 0;
   int haystack_len = 0;
   int len = 0;
   int len1 = 0;
   int len2 = 0;
   
   if (!haystack || !needle || !*haystack || !*needle)
      return NULL;
   
   needle_len = strlen(needle);
   haystack_len = strlen(haystack);
   
   while (*ptr != 0 && ptr < &haystack[haystack_len])
   {
      /* consume leading spaces or comma */
	  ptr += strspn(ptr, ",");
      ptr += strspn(ptr, " ");
      
      /* what's the length of the next word */
      len1 = strcspn(ptr, " ");
	  len2 = strcspn(ptr, ",");
	  len = (len1 < len2)? len1:len2;
      if ((needle_len == len) && (!strncmp(needle, ptr, len)))
         return (char*) ptr;
      
      ptr += len;
   }
   return NULL;
}

UBOOL8 cmsUtl_isSubOptionPresent(const char *fullOptionString, const char *subOption)
{
   const char *startChar, *currChar;
   UINT32 len=0;
   UBOOL8 found=FALSE;

   cmsLog_debug("look for subOption %s in fullOptionString=%s", subOption, fullOptionString);

   if (fullOptionString == NULL || subOption == NULL)
   {
      return FALSE;
   }

   startChar = fullOptionString;
   currChar = startChar;

   while (!found && *currChar != '\0')
   {
      /* get to the end of the current subOption */
      while (*currChar != ' ' && *currChar != ',' && *currChar != '\0')
      {
         currChar++;
         len++;
      }

      /* compare the current subOption with the subOption that was specified */
      if ((len == strlen(subOption)) &&
          (0 == strncmp(subOption, startChar, len)))
      {
         found = TRUE;
      }

      /* advance to the start of the next subOption */
      if (*currChar != '\0')
      {
         while (*currChar == ' ' || *currChar == ',')
         {
            currChar++;
         }

         len = 0;
         startChar = currChar;
      }
   }

   cmsLog_debug("found=%d", found);
   return found;
}


void cmsUtl_getWanProtocolName(UINT8 protocol, char *name) 
{
    if ( name == NULL ) 
      return;

    name[0] = '\0';
       
    switch ( protocol ) 
    {
        case CMS_WAN_TYPE_PPPOE:
            strcpy(name, "PPPoE");
            break;
        case CMS_WAN_TYPE_PPPOA:
            strcpy(name, "PPPoA");
            break;
        case CMS_WAN_TYPE_DYNAMIC_IPOE:
        case CMS_WAN_TYPE_STATIC_IPOE:
            strcpy(name, "IPoE");
            break;
        case CMS_WAN_TYPE_IPOA:
            strcpy(name, "IPoA");
            break;
        case CMS_WAN_TYPE_BRIDGE:
            strcpy(name, "Bridge");
            break;
#if SUPPORT_ETHWAN
        case CMS_WAN_TYPE_DYNAMIC_ETHERNET_IP:
            strcpy(name, "IPoW");
            break;
#endif
        default:
            strcpy(name, "Not Applicable");
            break;
    }
}

char *cmsUtl_getAggregateStringFromDhcpVendorIds(const char *vendorIds)
{
   char *aggregateString;
   const char *vendorId;
   UINT32 i, count=0;

   if (vendorIds == NULL)
   {
      return NULL;
   }

   aggregateString = cmsMem_alloc(MAX_PORTMAPPING_DHCP_VENDOR_IDS * (DHCP_VENDOR_ID_LEN + 1), ALLOC_ZEROIZE);
   if (aggregateString == NULL)
   {
      cmsLog_error("allocation of aggregate string failed");
      return NULL;
   }

   for (i=0; i < MAX_PORTMAPPING_DHCP_VENDOR_IDS; i++)
   {
      vendorId = &(vendorIds[i * (DHCP_VENDOR_ID_LEN + 1)]);
      if (*vendorId != '\0')
      {
         if (count > 0)
         {
            strcat(aggregateString, ",");
         }
         /* strncat writes at most DHCP_VENDOR_ID_LEN+1 bytes, which includes the trailing NULL */
         strncat(aggregateString, vendorId, DHCP_VENDOR_ID_LEN);
        
         count++;
      }
   }

   return aggregateString;
}


char *cmsUtl_getDhcpVendorIdsFromAggregateString(const char *aggregateString)
{
   char *vendorIds, *vendorId, *ptr, *savePtr=NULL;
   char *copy;
   UINT32 count=0;

   if (aggregateString == NULL)
   {
      return NULL;
   }

   vendorIds = cmsMem_alloc(MAX_PORTMAPPING_DHCP_VENDOR_IDS * (DHCP_VENDOR_ID_LEN + 1), ALLOC_ZEROIZE);
   if (vendorIds == NULL)
   {
      cmsLog_error("allocation of vendorIds buffer failed");
      return NULL;
   }

   copy = cmsMem_strdup(aggregateString);
   ptr = strtok_r(copy, ",", &savePtr);
   while ((ptr != NULL) && (count < MAX_PORTMAPPING_DHCP_VENDOR_IDS))
   {
      vendorId = &(vendorIds[count * (DHCP_VENDOR_ID_LEN + 1)]);
      /*
       * copy at most DHCP_VENDOR_ID_LEN bytes.  Since each chunk in the linear
       * buffer is DHCP_VENDOR_ID_LEN+1 bytes long and initialized to 0,
       * we are guaranteed that each vendor id is null terminated.
       */
      strncpy(vendorId, ptr, DHCP_VENDOR_ID_LEN);
      count++;

      ptr = strtok_r(NULL, ",", &savePtr);
   }

   cmsMem_free(copy);
   
   return vendorIds;
}


ConnectionModeType cmsUtl_connectionModeStrToNum(const char *connModeStr)
{
   ConnectionModeType connMode = CMS_CONNECTION_MODE_DEFAULT;
   if (connModeStr == NULL)
   {
      cmsLog_error("connModeStr is NULL");
      return connMode;
   }

   if (cmsUtl_strcmp(connModeStr, MDMVS_VLANMUXMODE) == 0)
   {
      connMode = CMS_CONNECTION_MODE_VLANMUX;
   }
   return connMode;

}


#ifdef SUPPORT_IPV6
CmsRet cmsUtl_standardizeIp6Addr(const char *address, char *stdAddr)
{
   struct in6_addr in6Addr;
   UINT32 plen;
   char   addr[BUFLEN_40];

   if (address == NULL || stdAddr == NULL)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }

   if (cmsUtl_parsePrefixAddress(address, addr, &plen) != CMSRET_SUCCESS)
   {
      cmsLog_error("Invalid ipv6 address=%s", address);
      return CMSRET_INVALID_ARGUMENTS;
   }

   if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
   {
      cmsLog_error("Invalid ipv6 address=%s", address);
      return CMSRET_INVALID_ARGUMENTS;
   }

   inet_ntop(AF_INET6, &in6Addr, stdAddr, BUFLEN_40);

   if (strchr(address, '/') != NULL)
   {
      char prefix[BUFLEN_8];

      sprintf(prefix, "/%d", plen);
      strcat(stdAddr, prefix);
   }

   return CMSRET_SUCCESS;

}  /* End of cmsUtl_standardizeIp6Addr() */

UBOOL8 cmsUtl_isGUAorULA(const char *address)
{
   struct in6_addr in6Addr;
   UINT32 plen;
   char   addr[BUFLEN_40];

   if (cmsUtl_parsePrefixAddress(address, addr, &plen) != CMSRET_SUCCESS)
   {
      cmsLog_error("Invalid ipv6 address=%s", address);
      return FALSE;
   }

   if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
   {
      cmsLog_error("Invalid ipv6 address=%s", address);
      return FALSE;
   }

   /* currently IANA assigned global unicast address prefix is 001..... */
   if ( ((in6Addr.s6_addr[0] & 0xe0) == 0x20) || 
        ((in6Addr.s6_addr[0] & 0xfe) == 0xfc) )
   {
      return TRUE;
   }

   return FALSE;


}  /* End of cmsUtl_isGUAorULA() */


UBOOL8 cmsUtl_isUlaPrefix(const char *prefix_str)
{
   struct in6_addr in6Addr;
   UINT32 plen;
   char   prefix[BUFLEN_40];

   if (prefix_str == NULL)
   {
      cmsLog_debug("prefix is null");
      return FALSE;
   }

   if (cmsUtl_parsePrefixAddress(prefix_str, prefix, &plen) != CMSRET_SUCCESS)
   {
      cmsLog_error("Invalid ipv6 prefix_str=%s", prefix_str);
      return FALSE;
   }
   if (inet_pton(AF_INET6, prefix, &in6Addr) <= 0)
   {
      cmsLog_error("Invalid ipv6 prefix=%s", prefix);
      return FALSE;
   }

   if ((in6Addr.s6_addr[0] & 0xfe) == 0xfc) 
   {
      return TRUE;
   }

   return FALSE;
}  /* End of cmsUtl_isUlaPrefix() */


CmsRet cmsUtl_replaceEui64(const char *address1, char *address2)
{
   struct in6_addr   in6Addr1, in6Addr2;

   if (inet_pton(AF_INET6, address1, &in6Addr1) <= 0)
   {
      cmsLog_error("Invalid address=%s", address1);
      return CMSRET_INVALID_ARGUMENTS;
   }
   if (inet_pton(AF_INET6, address2, &in6Addr2) <= 0)
   {
      cmsLog_error("Invalid address=%s", address2);
      return CMSRET_INVALID_ARGUMENTS;
   }

   in6Addr2.s6_addr32[2] = in6Addr1.s6_addr32[2];
   in6Addr2.s6_addr32[3] = in6Addr1.s6_addr32[3];

   if (inet_ntop(AF_INET6, &in6Addr2, address2, BUFLEN_40) == NULL)
   {
      cmsLog_error("inet_ntop returns NULL");
      return CMSRET_INTERNAL_ERROR;
   }

   return CMSRET_SUCCESS;
      
}  /* End of cmsUtl_replaceEui64() */


#endif

CmsRet cmsUtl_getAddrPrefix(const char *address, UINT32 plen, char *prefix)
{
   struct in6_addr   in6Addr;
   UINT16 i, k, mask;

   if (plen > 128)
   {
      cmsLog_error("Invalid plen=%d", plen);
      return CMSRET_INVALID_ARGUMENTS;
   }
   else if (plen == 128)
   {

      cmsUtl_strncpy(prefix, address, INET6_ADDRSTRLEN);
      return CMSRET_SUCCESS; 
   }

   if (inet_pton(AF_INET6, address, &in6Addr) <= 0)
   {
      cmsLog_error("Invalid address=%s", address);
      return CMSRET_INVALID_ARGUMENTS;
   }

   k = plen / 16;
   mask = 0;
   if (plen % 16)
   {
      mask = htons(~(UINT16)(((1 << (16 - (plen % 16))) - 1) & 0xFFFF));
   }

   in6Addr.s6_addr16[k] &= mask;
   
   for (i = k+1; i < 8; i++)
   {
      in6Addr.s6_addr16[i] = 0;
   } 
   
   if (inet_ntop(AF_INET6, &in6Addr, prefix, INET6_ADDRSTRLEN) == NULL)
   {
      cmsLog_error("inet_ntop returns NULL");
      return CMSRET_INTERNAL_ERROR;
   }

   return CMSRET_SUCCESS; 
   
}  /* End of cmsUtl_getAddrPrefix() */


CmsRet cmsUtl_parsePrefixAddress(const char *prefixAddr, char *address, UINT32 *plen)
{
   CmsRet ret = CMSRET_SUCCESS;
   char *tmpBuf;
   char *separator;
   UINT32 len;

   if (prefixAddr == NULL || address == NULL || plen == NULL)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }      
   
   cmsLog_debug("prefixAddr=%s", prefixAddr);

   *address = '\0';
   *plen    = 128;

   len = strlen(prefixAddr);

   if ((tmpBuf = cmsMem_alloc(len+1, 0)) == NULL)
   {
      cmsLog_error("alloc of %d bytes failed", len);
      ret = CMSRET_INTERNAL_ERROR;
   }
   else
   {
      sprintf(tmpBuf, "%s", prefixAddr);
      separator = strchr(tmpBuf, '/');
      if (separator != NULL)
      {
         /* break the string into two strings */
         *separator = 0;
         separator++;
         while ((isspace(*separator)) && (*separator != 0))
         {
            /* skip white space after comma */
            separator++;
         }

         *plen = atoi(separator);
         cmsLog_debug("plen=%d", *plen);
      }

      cmsLog_debug("address=%s", tmpBuf);
      if (strlen(tmpBuf) < BUFLEN_40 && *plen <= 128)
      {
         strcpy(address, tmpBuf);
      }
      else
      {
         ret = CMSRET_INVALID_ARGUMENTS;
      }
      cmsMem_free(tmpBuf);
   }

   return ret;
   
}  /* End of cmsUtl_parsePrefixAddress() */


void cmsUtl_truncatePrefixFromIpv6AddrStr(char *addrStr)
{
   char *pp;

   pp = strchr(addrStr, '/');
   if (pp) {
      *pp = '\0';
   }
}


UBOOL8 cmsUtl_ipStrToOctets(const char *input, char *output)
{
   UBOOL8 ret = TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_16];
   UINT32 i, num;

   if (input == NULL || strlen(input) < 7 || strlen(input) > 15)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* IP address has the following format
      xxx.xxx.xxx.xxx where x is decimal number */
   pToken = strtok_r(buf, ".", &pLast);
   if ((cmsUtl_strtoul(pToken, NULL, 10, &num) != CMSRET_SUCCESS) ||
       (num > 255))
   {
      ret = FALSE;
   }
   else
   {
      output[0] = num;

      for ( i = 0; i < 3; i++ )
      {
         pToken = strtok_r(NULL, ".", &pLast);

         if ((cmsUtl_strtoul(pToken, NULL, 10, &num) != CMSRET_SUCCESS) ||
             (num > 255))
         {
            ret = FALSE;
            break;
         }
         else
         {
            output[i+1] = num;
         }
      }
   }
   return ret;
}

/* rfc4122 version 4 from Random numbers */
void cmsUtl_generateUuidStrFromRandom(char *str, int len)
{
   oal_getRandomUuid(str,len);

#if 0
   /* if UUID is not generated by kernel, then use the following logic to generate one */
   /* the more random the bytes are the better */
   UUID uuid;
   unsigned char randomBytes[UUID_LEN];

   oal_getRandomBytes(randomBytes,UUID_LEN);
   memcpy(&uuid, randomBytes, sizeof(UUID));
   /* bit 6 and 7 of clocks_sq_hi_and_reserve 0 and 1 */
   uuid.clock_seq_hi_and_reserved &= 0x3F;
   uuid.clock_seq_hi_and_reserved |= 0x80;
   /* bit 12-15 of time_hi and version to version 4.*/
   uuid.time_hi_and_version &= 0x0FFF;
   uuid.time_hi_and_version |= (4 << 12);

   snprintf(str, len, "%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x", uuid.time_low, uuid.time_mid,
            uuid.time_hi_and_version, uuid.clock_seq_hi_and_reserved,uuid.clock_seq_low,
            (UINT8)uuid.node[0],(UINT8)uuid.node[1],(UINT8)uuid.node[2],
            (UINT8)uuid.node[3],(UINT8)uuid.node[4],(UINT8)uuid.node[5]);
#endif /* alternate way */

}

/* rfc4122 version 3: uuid is created
   with md5 using a name from NameSpace.
   Implementation is based on rfc4122
   "Appendix A - Sample Implementation"
 */
void cmsUtl_generateUuidStrFromName(const char *name,
                                    UINT32 nameLen,
                                    char *strUuid,
                                    UINT32 strUuidLen)
{
   unsigned char hash[UUID_LEN];
   MD5Context ctx;
   UUID uuid, net_nsid;
   /* 6ba7b810-9dad-11d1-80b4-00c04fd430c8 */
   UUID NameSpace_DNS =
   {
      0x6ba7b810,
      0x9dad,
      0x11d1,
      0x80, 0xb4,
      {0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8}
   };

   /* put name space ID in network byte order so it hashes the same
      no matter what endian machine we're on */
   net_nsid = NameSpace_DNS;
   net_nsid.time_low = htonl(net_nsid.time_low);
   net_nsid.time_mid = htons(net_nsid.time_mid);
   net_nsid.time_hi_and_version = htons(net_nsid.time_hi_and_version);

   MD5Init(&ctx);
   MD5Update(&ctx, (u_char const *)&net_nsid, sizeof(UUID));
   MD5Update(&ctx, (u_char const *)name, nameLen);
   MD5Final(hash, &ctx);

   /* the hash is in network byte order at this point */

   /* convert UUID to local byte order */
   memcpy(&uuid, hash, sizeof(UUID));
   uuid.time_low = ntohl(uuid.time_low);
   uuid.time_mid = ntohs(uuid.time_mid);
   uuid.time_hi_and_version = ntohs(uuid.time_hi_and_version);

   /* put in the variant and version bits */
   uuid.time_hi_and_version &= 0x0FFF;
   uuid.time_hi_and_version |= (3 << 12);
   uuid.clock_seq_hi_and_reserved &= 0x3F;
   uuid.clock_seq_hi_and_reserved |= 0x80;

   snprintf(strUuid, strUuidLen,
            "%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
            uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
            uuid.clock_seq_hi_and_reserved, uuid.clock_seq_low,
            (UINT8)uuid.node[0],(UINT8)uuid.node[1],(UINT8)uuid.node[2],
            (UINT8)uuid.node[3],(UINT8)uuid.node[4],(UINT8)uuid.node[5]);
}

static unsigned char cmsUtl_nibble2byte(unsigned char nibble)
{
   if (nibble <= 9)
   {
      return nibble + 0x30;
   }
   else if (nibble <= 0xF)
   {
      return nibble - 0xA + 0x41;
   }
   else
   {
      // This should never happen
      return 0x46;
   }
}

static unsigned char cmsUtl_byte2nibble(unsigned char byte)
{
   if (0x30 <= byte && byte <= 0x39)
   {
      return byte - 0x30;
   }
   else if (0x41 <= byte && byte <= 0x46)
   {
      return byte - 0x41 + 0xA;
   }
   else
   {
      // This should never happen
      return 0xF;
   }
}

UBOOL8 cmsUtl_isValidUuid(const char *uuid)
{
   int i;
   const char *p = uuid;

   if (uuid == NULL || strlen(uuid) != 36)
      return FALSE;

   for (i = 0 ; i < 36 ; i++, p++)
   {
      if (i == 8 || i == 13 || i == 18 || i == 23)
      {
         if (*p != '-')
            return FALSE;
      }
      else if (!isxdigit(*p))
         return FALSE;
   }
   
   return TRUE;
}

void cmsUtl_encodeHexStr(char *pDst,
                         unsigned int dstLen,
                         const char *pSrc,
                         unsigned int srcLen)
{
   int i, j, len;

   if (NULL == pSrc || NULL == pDst || 0 == srcLen || 4 > dstLen)
   {
      return;
   }

   /* Truncate data that is too long */
   if (srcLen > dstLen / 2 - 1)
   {
      len = dstLen / 2 - 1;
   }
   else
   {
      len = srcLen;
   }

   /* Convert string to hex string */
   for (i = 0; i < len; i++)
   {
      j = i * 2;
      pDst[j] = cmsUtl_nibble2byte(pSrc[i] >> 4);
      pDst[j + 1] = cmsUtl_nibble2byte(pSrc[i] & 0x0F);
   }
   pDst[len * 2] = '\0';
}

int cmsUtl_decodeHexStr(char *pDst,
                        unsigned int dstLen,
                        const char *pSrc)
{
   unsigned int i, len;

   if (NULL == pSrc || NULL == pDst || 0 == dstLen)
   {
      return 0;
   }

   len = strlen(pSrc);
   if (len > (dstLen - 1) * 2)
   {
      len = (dstLen - 1) * 2;
   }

   /* Convert hex string to string */
   for (i = 0; i < len; i += 2)
   {
      pDst[i / 2] = (cmsUtl_byte2nibble(pSrc[i]) << 4)
                     | (cmsUtl_byte2nibble(pSrc[i + 1]));
   }
   pDst[i / 2] = '\0';

   return i / 2;
}

/****************************porting by xuyong ,2015-10-15**********************/
#define TCP_QUICK_ACK_SET            0x89a9 //2015-6-25
#define TCP_QUICK_ACK_GET            0x89aa

int cmsUtl_ty_dslite_tcp_quick_ack_set(int start,unsigned short vid, struct in6_addr *ip6_src,unsigned int sourIp,unsigned short sourcePort,
                                                struct in6_addr *ip6_dst,unsigned int destIp,unsigned short destPort,unsigned short wndsize)
{
    TY_TCP_CAPTURE_DATA data;
    int socketFd;
    struct ifreq intf;
    int ret;
    //char ipstr[64] = {0}, locip6[64] = {0};
    memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));

    data.startcapture = start;
    if (start)
    {
        data.vid = vid;
        data.wanFlag = 1;
        data.sourceIP = sourIp;
        data.source = sourcePort;
        data.destIP = destIp;
        data.source = sourcePort;
        //printf("%s %d sourIp:%x destIp:%x\n",__FUNCTION__,__LINE__,sourIp,destIp);
        if(ip6_src)
        {
            memcpy(&data.ip6_src, ip6_src, sizeof(data.ip6_src));
            //inet_ntop(AF_INET6, ip6_src, ipstr, sizeof(ipstr));
            //printf("%s %d ipstr:%s\n",__FUNCTION__,__LINE__,ipstr);
        }
        if(ip6_dst)
        {
            memcpy(&data.ip6_dst, ip6_dst, sizeof(data.ip6_src));
            //inet_ntop(AF_INET6, ip6_dst, locip6, sizeof(locip6));
            //printf("%s %d locip6:%s\n",__FUNCTION__,__LINE__,locip6);
            //for(i = 0;i < 16; i++)
            //    printf("%x",ip6_dst->s6_addr[i]);
            //printf("\n");
            //printf("%s %d locip6:%x%x%x%x\n",__FUNCTION__,__LINE__,
            //       ip6_dst->in6_u.u6_addr32[0],ip6_dst->in6_u.u6_addr32[1],ip6_dst->in6_u.u6_addr32[2],ip6_dst->in6_u.u6_addr32[3]);
        }
        data.dest = destPort;
        data.wndSize = wndsize;
    }
    else if (wndsize > 0)
    {
        data.wndSize = wndsize;
    }
    //printf("%s %d wndsize:%x\n",__FUNCTION__,__LINE__,wndsize);
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFd <= 0)
    {
        return 0;
    }
    strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
    intf.ifr_data = (char *)&data;
    if((ret = ioctl(socketFd, TCP_QUICK_ACK_SET, &intf)) < 0)
    {
        cmsLog_error("TCP_QUICK_ACK_SET : %s\n",  strerror(errno));
        close(socketFd);
        return 0;
    }
    close(socketFd);
    return data.index;
}
int cmsUtl_ty_tcp_quick_ack_setv6(int start,unsigned short vid, struct in6_addr *ip6_src, unsigned short sourcePort,
                                           struct in6_addr *ip6_dst,unsigned short destPort,unsigned short wndsize)
{
    TY_TCP_CAPTURE_DATA data;
    int socketFd;
    struct ifreq intf;
    int ret;
    //char ipstr[64] = {0}, locip6[64] = {0};
    memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));

    data.startcapture = start;
    //printf("%s %d data.startcapture:%d\n",__FUNCTION__,__LINE__,data.startcapture);
    if (start && ip6_dst && ip6_src)
    {
        data.vid = vid;
        data.wanFlag = 1;
        if(ip6_src)
        {
            memcpy(&data.ip6_src, ip6_src, sizeof(data.ip6_src));
            //inet_ntop(AF_INET6, ip6_src, ipstr, sizeof(ipstr));
            //printf("%s %d ipstr:%s\n",__FUNCTION__,__LINE__,ipstr);
        }
        if(ip6_dst)
        {
            memcpy(&data.ip6_dst, ip6_dst, sizeof(data.ip6_src));
            //inet_ntop(AF_INET6, ip6_dst, locip6, sizeof(locip6));
            //printf("%s %d locip6:%s\n",__FUNCTION__,__LINE__,locip6);
        }
        data.source = sourcePort;
        data.dest = destPort;
        data.wndSize = wndsize;
    }
    else if (wndsize > 0)
    {
        data.wndSize = wndsize;
    }
    //printf("%s %d wndsize:%x\n",__FUNCTION__,__LINE__,wndsize);
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFd <= 0)
    {
        return 0;
    }
    strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
    intf.ifr_data = (char *)&data;
    if((ret = ioctl(socketFd, TCP_QUICK_ACK_SET, &intf)) < 0)
    {
        cmsLog_error("TCP_QUICK_ACK_SET : %s\n",  strerror(errno));
        close(socketFd);
        return 0;
    }
    close(socketFd);
    return data.index;
}
int cmsUtl_ty_tcp_quick_ack_set(int start,unsigned short vid,unsigned int sourceIp,unsigned short sourcePort,
                                        unsigned int destIp,unsigned short destPort,unsigned short wndsize)
{
    TY_TCP_CAPTURE_DATA data;
    int socketFd;
    struct ifreq intf;
    int ret;

    memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));
    data.startcapture = start;

    if (start)
    {
        data.vid = vid;
        data.wanFlag = 1;
        data.sourceIP = sourceIp;
        data.source = sourcePort;
        data.destIP = destIp;
        data.dest = destPort;
        data.wndSize = wndsize;
    }
    else if (wndsize > 0)
    {
        data.wndSize = wndsize;
    }
    //printf("%s %d wndsize:%x\n",__FUNCTION__,__LINE__,wndsize);
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFd <= 0)
    {
        return 0;
    }
    strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
    intf.ifr_data = (char *)&data;
    if((ret = ioctl(socketFd, TCP_QUICK_ACK_SET, &intf)) < 0)
    {
        cmsLog_error("TCP_QUICK_ACK_SET : %s\n",  strerror(errno));
        close(socketFd);
        return 0;
    }

    close(socketFd);
    return data.index;
}

int cmsUtl_ty_tcp_quick_ack_get(int index,unsigned int *recvbytes,unsigned int * totalPacket)
{
    TY_TCP_CAPTURE_DATA data;
    int socketFd,ret;
    struct ifreq intf;
    memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));    
    data.index = index;
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFd <= 0)
    {
        return 0;
    }
    strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
    intf.ifr_data = (char *)&data;
    if((ret = ioctl(socketFd, TCP_QUICK_ACK_GET, &intf)) < 0)
    {
        cmsLog_error("TCP_QUICK_ACK_GET : %s\n",  strerror(errno));
        close(socketFd);
        return 0;
    }
    close(socketFd);
    printf("index:%d recv:%u\n",index,data.recvByte);
    *recvbytes = data.recvByte;
    //*totalPacket = data.recvAckPacket;
    *totalPacket = data.totalByte;
    return 1;
}

int cmsUtl_ty_tcp_quick_ack_sckcreate(void)
{
    int socketFd;
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    return socketFd;
}

void cmsUtl_ty_tcp_quick_ack_sckclose(int sock)
{
    if(sock > 0)
        close(sock);
}

int cmsUtl_ty_tcp_quick_ack_sckget(int socket, int index, unsigned int *recvbytes, unsigned int *totalPacket)
{
    TY_TCP_CAPTURE_DATA data;
    struct ifreq intf;
    if(socket <= 0)
    {
        *recvbytes = 0;
        *totalPacket = 0;
        return 0;
    }
    memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));    
    data.index = index;
    strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
    intf.ifr_data = (char *)&data;
    if(ioctl(socket, TCP_QUICK_ACK_GET, &intf) < 0)
    {
        cmsLog_error("TCP_QUICK_ACK_GET : %s\n",  strerror(errno));
        return 0;
    }
    printf("%s %d index:%d recvByte:%u\n",__FUNCTION__,__LINE__,index,data.recvByte);
    *recvbytes = data.recvByte;
    //*totalPacket = data.recvAckPacket;
    *totalPacket = data.totalByte;
    return 1;
}

int cmsUtl_ty_tcp_quick_ack_init(void)
{
    cmsUtl_ty_tcp_quick_ack_set(3,0,0,0,0,0,0);
    return 0;
}

//add by wuchuan 20160226 begin
/*    get str field value cmsUtl_getFieldValue("a|b|c",'|',0,fieldValue)
    str:    the str
    dot:    the dot char | :
    index:    form 0 to ...
    fieldValue:the value of result
*/
void cmsUtl_getFieldValue(const char *str,char *dot,int index,char *fieldValue)
{
    int num=0;
    char *result = NULL;
    char pList[BUFLEN_512];
    
    if ((!str)||(!dot)||(!fieldValue))
        return;
    memset(pList,0x0,sizeof(pList));
    cmsUtl_strncpy(pList,str,sizeof(pList));
    result = strtok( pList, dot);
    while (result != NULL)
    {
        if (num == index)
        {
            cmsUtl_strcpy(fieldValue,result);
            break;
        }
        num++;
        result = strtok( NULL, dot);  
    }
}
/*    set str field value cmsUtl_setFieldValue("a|b|c",'|',0,fieldValue)
    str:    the str
    dot:    the dot char | :
    index:    form 0 to ...
    fieldValue:the value you wan to set
*/
void cmsUtl_setFieldValue(char *str,char *dot,int index,char *fieldValue)
{
    int num=0;
    char *result = NULL;
    char pList[128];
    
    if ((!str)||(!dot)||(!fieldValue))
        return;
    
    cmsUtl_strncpy(pList,str,sizeof(pList));
    memset(str,0,strlen(pList));
    result = strtok( pList, dot);
    while (result != NULL)
    {
        if (num == index)
        {
            if (num==0)
                cmsUtl_strcpy(str,fieldValue);
            else
                sprintf(str,"%s%s%s",str,dot,fieldValue);
        }
        else
        {
            if (num==0)
                cmsUtl_strcpy(str,result);
            else
                sprintf(str,"%s%s%s",str,dot,result);
        }            
        num++;
        result = strtok( NULL, dot);  
    }
}
void cmsUtl_delSpecChar(char *str)
{
    char tmpStr[128]={0};
    strcpy(tmpStr,str);
    int index=0;
    char cTmp;
    
    if (!str)
        return;
    
    memset(str,0,strlen(str));
    
    while ((cTmp=tmpStr[index]) != '\0')
    {
        if (((cTmp >= 'a')&&(cTmp <= 'z'))
            ||((cTmp >= 'A')&&(cTmp <= 'Z'))
            ||((cTmp >= '0')&&(cTmp <= '9')))
            sprintf(str,"%s%c",str,cTmp);
        index++;
    }
}
//add the num by the code "678:0+676:0+691:0+999:0"
//changeNumStrByCode("678:0+676:0+691:0+999:0","691",2)
void changeNumStrByCode(char *str,char *code,int var)
{
    char fieldValue[64]={0};
    char resultStr[32]={0};
    char fieldCode[32]={0};
    char fieldNum[8]={0};
    int numTmp=0;
    int iLoop=0;
    
    if ((!str)||(!code))
        return;
    
    while(1)
    {    
        fieldValue[0] = '\0';
        cmsUtl_getFieldValue(str,"+",iLoop,fieldValue);
        //printf("fieldValue:%s\n",fieldValue);
        if (fieldValue[0] == '\0')
            break;
        
        cmsUtl_getFieldValue(fieldValue,":",0,fieldCode);
        if (!strcmp(fieldCode,code))
        {
            cmsUtl_getFieldValue(fieldValue,":",1,fieldNum);
            numTmp=atoi(fieldNum);
            numTmp+=var;
            sprintf(resultStr,"%s:%d",fieldCode,numTmp);
            //printf("resultStr:%s\n",resultStr);
            cmsUtl_setFieldValue(str,"+",iLoop,resultStr);
            break;
        }
        iLoop++;
    }
}
void getNextSepStr(char *result,char **ptrCur,char **rst,char Sep)
{
 	if (!result)
		return;
	if (*rst==NULL)
		goto getStr;
	else
	{
		if(*ptrCur)
		{
			*ptrCur = *rst+1;
			goto getStr;
		}
		else
		{
			result[0]='\0';
			return;
		}
	}
getStr:
	if(*ptrCur)
	{
		*rst=strchr(*ptrCur,Sep);
		if (*rst)
		{
			strncpy(result,*ptrCur,*rst-*ptrCur);
			result[*rst-*ptrCur]='\0';
		}
		else
		{
			strcpy(result,*ptrCur);
			*rst = result;
			*ptrCur = NULL;
		}
	}
	else
		result[0]='\0';
}
//add by wuchuan 20160226 end
/*------yangtai add 20180808 for UDP Speed measuring interface-------*/

int ty_udp_speed_measure_set (int start,unsigned short vid,unsigned int sourceIp,unsigned short sourcePort,
                                        unsigned int destIp,unsigned short destPort)
{
	TY_TCP_CAPTURE_DATA data;
	int socketFd;
	struct ifreq intf;
	int ret;
#if defined(CHIP_6848) || defined(CHIP_6838) || defined(CHIP_6846)
	char cmd[128] = {0};
#endif

	memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));
	data.startcapture = start;

#if defined(CHIP_6848) || defined(CHIP_6838)
	sprintf(cmd, "bs /b/c cpu/index=host reason_cfg[{dir=ds,reason=ip_flow_miss}]={queue=3,meter=-1}");
	system(cmd);
#endif
#if defined(CHIP_6846)
	sprintf(cmd, "bs /b/c cpu/index=host reason_cfg[{dir=ds,reason=ip_flow_miss}]={meter=-1}");
    system(cmd);
#endif

	if (start)
	{
		data.vid = vid;
		data.wanFlag = 1;
		data.sourceIP = destIp;
		data.source = destPort;
		data.destIP = sourceIp;
		data.dest = sourcePort;
	}

	//printf("%s: sourceIp:0x%x sourcePort:%d destIP:0x%x destPort:%d\n",__FUNCTION__,sourceIp,sourcePort,destIp,destPort);

	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socketFd <= 0)
	{
		return 0;
	}
	strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
	intf.ifr_data = (char *)&data;
	if((ret = ioctl(socketFd, TCP_QUICK_ACK_SET, &intf)) < 0)
	{
		cmsLog_error("TCP_QUICK_ACK_SET : %s\n",  strerror(errno));
		close(socketFd);
		return 0;
	}

	close(socketFd);
	return data.index;
}

int ty_udp_speed_measure_get (int index,unsigned int *recvbytes,unsigned int *totalPacket)
{
	TY_TCP_CAPTURE_DATA data;
	int socketFd,ret;
	struct ifreq intf;
	memset(&data,0,sizeof(TY_TCP_CAPTURE_DATA));	
	data.index = index;
	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socketFd <= 0)
	{
		return 0;
	}
	strncpy(intf.ifr_name, "eth0", sizeof(intf.ifr_name));
	intf.ifr_data = (char *)&data;
	if((ret = ioctl(socketFd, TCP_QUICK_ACK_GET, &intf)) < 0)
	{
		cmsLog_error("TCP_QUICK_ACK_GET : %s\n",  strerror(errno));
		close(socketFd);
		return 0;
	}
	close(socketFd);
	printf("%s %d index:%d recvByte:%u\n",__FUNCTION__,__LINE__,index,data.recvByte);
	*recvbytes = data.recvByte;
	//*totalPacket = data.recvAckPacket;
	*totalPacket = data.totalByte;
	return 1;
}



int ty_udp_speed_measure_stop (void)
{
#if defined(CHIP_6848) || defined(CHIP_6838) || defined(CHIP_6846)
	char cmd[128] = {0};
#endif
#if defined(CHIP_6848) || defined(CHIP_6838)
	sprintf(cmd, "bs /b/c cpu/index=host reason_cfg[{dir=ds,reason=ip_flow_miss}]={queue=3,meter=2}");
    system(cmd);
#endif
#if defined(CHIP_6846)
	sprintf(cmd, "bs /b/c cpu/index=host reason_cfg[{dir=ds,reason=ip_flow_miss}]={meter=2}");
    system(cmd);
#endif
    cmsUtl_ty_tcp_quick_ack_set(3,0,0,0,0,0,0);
    return 0;
}


/*-----yangtai end-----*/
//a by xc 2020.6.18 for conv
int ty_getCmdResult(char *cmd, char res[], int len, int skip)
{
	FILE *fp = NULL;
	int ret = -1;

	if ((fp = popen(cmd, "r"))) 
	{
		if (skip > 0)
			fread(res, 1, skip, fp);
		ret = fread(res, 1, len, fp);
		pclose(fp);
		cmsLog_notice("%s %d\n", cmd, ret);
	}

	return ret;
}

SINT32 hgUtil_snprintf(const char *func, UINT32 line, char *str, UINT32 size, const char *format, ...)
{
    SINT32 ret = 0;
    va_list paraList;

    if (NULL == str || NULL == format)
    {
        cmsLog_error("[%s][%u]: NULL pointer str=%p, format =%p!", func, line, str, format);
        return 0;
    }

    va_start(paraList, format);
    ret = vsnprintf(str, size, format, paraList);
    va_end(paraList);

    return ret;
}

char *hgUtil_strncat2(const char *func, UINT32 line, char *dest, const char *src, SINT32 dlen)
{
   SINT32 n = 0;

   if ((src == NULL) || (dest == NULL) || dlen <= 0)
   {
      cmsLog_error("[%s][%u]: NULL pointer! src:%p dest:%p dlen:%d !", func, line, src, dest, dlen);
      return dest;
   }

   n = dlen - strlen(dest);
   if (n <= 0)
   {
      cmsLog_error("[%s][%u]: dlen < strlen(dest)", func, line);
      return dest;
   }

   if (strlen(src) + 1 > n)
   {
      cmsLog_error("[%s][%u]: truncating! src string length > dest buffer", func, line);
      strncat(dest, src, n - 1);
      dest[dlen - 1] ='\0';
   }
   else
   {
      strncat(dest, src, n);
   }

   return dest;
}


