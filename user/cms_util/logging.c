/***********************************************************************
 *
<:copyright-BRCM:2006:DUAL/GPL:standard

   Copyright (c) 2006 Broadcom
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


#include <fcntl.h>      /* open */
#include "cms.h"
#include "cms_log.h"
#include "cms_eid.h"
#include "cms_strconv.h"
#include "cms_mem.h"
#include "oal.h"
#include "bcm_ulog.h"

/** local definitions **/

/* default settings */

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/
static CmsEntityId gEid;     /**< try not to use this.  inefficient b/c requires eInfo lookup. */
static char *gAppName=NULL;  /**< name of app; set during init */
static CmsLogLevel             logLevel; /**< Message logging level.
                                          * This is set to one of the message
                                          * severity levels: LOG_ERR, LOG_NOTICE
                                          * or LOG_DEBUG. Messages with severity
                                          * level lower than or equal to logLevel
                                          * will be logged. Otherwise, they will
                                          * be ignored. This variable is local
                                          * to each process and is changeable
                                          * from CLI.
                                          */
static CmsLogDestination logDestination; /**< Message logging destination.
                                          * This is set to one of the
                                          * message logging destinations:
                                          * STDERR or SYSLOGD. This
                                          * variable is local to each
                                          * process and is changeable from
                                          * CLI.
                                          */
static UINT32 logHeaderMask; /**< Bitmask of which pieces of info we want
                              *   in the log line header.
                              */


UINT32 cmsLog_dest_to_mask(CmsLogDestination cmsLogDestination)
{
   if (cmsLogDestination == LOG_DEST_STDERR)
   {
      return BCMULOG_DESTMASK_STDERR;
   }
   else if (cmsLogDestination == LOG_DEST_SYSLOG)
   {
      return BCMULOG_DESTMASK_SYSLOG;
   }
   else
   {
      // bcmulog doesn't understand telnet destination.  Default to STDERR.
      return BCMULOG_DESTMASK_STDERR;
   }
}

void log_log(CmsLogLevel level, const char *func, UINT32 lineNum, const char *pFmt, ... )
{
   char logBuf[MAX_LOG_LINE_LENGTH];
   BcmuLogFormatInfo info;
   va_list ap;

   if (level > logLevel)
   {
      return;
   }

   /* Call external function to format the log line. */
   memset(&info, 0, sizeof(info));
   info.buf = logBuf;
   info.bufLen = sizeof(logBuf);
   info.logLevel = level;
   info.logDestMask = cmsLog_dest_to_mask(logDestination);
   info.logHeaderMask = logHeaderMask;
   info.lineNum = lineNum;
   cmsUtl_strncpy(info.funcName, func, sizeof(info.funcName));
   if (gAppName == NULL)
   {
      sprintf(info.appName, "%d", cmsEid_getPid());
   }
   else
   {
      cmsUtl_strncpy(info.appName, gAppName, sizeof(info.appName));
   }
   va_start(ap, pFmt);
   bcmuLog_formatLine(&info, pFmt, ap);
   va_end(ap);

   /* Do the output to configured destination */
   if (logDestination == LOG_DEST_STDERR)
   {
      fprintf(stderr, "%s\n", logBuf);
      fflush(stderr);
   }
   else if (logDestination == LOG_DEST_TELNET )
   {
      int logTelnetFd = -1;
#ifdef DESKTOP_LINUX
      /* Fedora Desktop Linux */
      logTelnetFd = open("/dev/pts/1", O_RDWR);
#else
      /* CPE use ptyp0 as the first pesudo terminal */
      logTelnetFd = open("/dev/ttyp0", O_RDWR);
#endif
      if(logTelnetFd != -1)
      {
         if (0 > write(logTelnetFd, logBuf, strlen(logBuf)))
            printf("write to telnet fd failed\n");
         if (0 > write(logTelnetFd, "\n", strlen("\n")))
            printf("write to telnet fd failed(2)\n");
         close(logTelnetFd);
      }
   }
   else
   {
      oalLog_syslog(level, logBuf);
   }

}  /* End of log_log() */


void cmsLog_initWithName(CmsEntityId eid, const char *appName)
{
   logLevel       = DEFAULT_LOG_LEVEL;
   logDestination = DEFAULT_LOG_DESTINATION;
   logHeaderMask  = DEFAULT_LOG_HEADER_MASK;

   gEid = eid;

   /*
    * highly unlikely this strdup will fail, but even if it does, the
    * code can handle a NULL gAppName.
    */
   gAppName = cmsMem_strdup(appName);

   oalLog_init();

   return;
}


void cmsLog_init(CmsEntityId eid)
{
   const CmsEntityInfo *eInfo;

   if (NULL != (eInfo = cmsEid_getEntityInfo(eid)))
   {
      cmsLog_initWithName(eid, eInfo->name);
   }
   else
   {
      cmsLog_initWithName(eid, NULL);
   }

   return;

}  /* End of cmsLog_init() */


void cmsLog_cleanup(void)
{
   oalLog_cleanup();
   CMSMEM_FREE_BUF_AND_NULL_PTR(gAppName);
   return;

}  /* End of cmsLog_cleanup() */


void cmsLog_setLevel(CmsLogLevel level)
{
   logLevel = level;
   bcmuLog_setLevel(level);
   return;
}


CmsLogLevel cmsLog_getLevel(void)
{
   return logLevel;
}


void cmsLog_setDestination(CmsLogDestination dest)
{
   logDestination = dest;
   bcmuLog_setDestMask(cmsLog_dest_to_mask(dest));
   return;
}


CmsLogDestination cmsLog_getDestination(void)
{
   return logDestination;
}


void cmsLog_setHeaderMask(UINT32 headerMask)
{
   logHeaderMask = headerMask;
   bcmuLog_setHdrMask(headerMask);
   return;
}


UINT32 cmsLog_getHeaderMask(void)
{
   return logHeaderMask;
}


int cmsLog_readPartial(int ptr, char* buffer)
{
   return (oal_readLogPartial(ptr, buffer));
}


#if 1
#include <sys/sysinfo.h>
#include <time.h>

static HGDBG_t hgDbgSetting;
void hgLoging(const char* func, int line,  ...) 
{ 
	va_list arg; 
	char   *fmt; 
	static char    spbuf[3000]; 
	time_t  nowtime; 
	struct tm *tm; 
	int     len; 

	va_start(arg, line); 
	fmt = va_arg(arg, char *); 
	nowtime = time(NULL); 
	memset(spbuf, 0, sizeof(spbuf)); 
	tm = localtime(&nowtime); 
	sprintf(spbuf,"[%d/%d %02d:%02d:%02d][%s][%d]:", 
	tm->tm_mon + 1, tm->tm_mday, 
	tm->tm_hour, tm->tm_min, tm->tm_sec, func, line);
	len = strlen(spbuf); 
	len += vsnprintf(&spbuf[len],3000-len-4, fmt, arg); 
	if(len > 0)
	{   
		if(spbuf[strlen(spbuf)-1] != '\n')
		{ 
			spbuf[strlen(spbuf)] = '\n'; 
		}
	} 
	va_end(arg); 
	printf("%s",spbuf);
} 
static UINT32 hgDbg_getDebugInfo(void)
{
    UINT32 debugFlags = hgDbgSetting.flags.all;
    return debugFlags;
}

UINT32 hgDbg_getModule(HGDBGMODULE module)
{
	UINT32 moduleResult = 0;
	HGDBG_t hgDbg;

	hgDbg.flags.all = hgDbg_getDebugInfo();

	switch (module)
	{
		case HGDBG_MOD1:
			moduleResult = hgDbg.flags.bits.mod1;
		break;
		case HGDBG_MOD2:
			moduleResult = hgDbg.flags.bits.mod2;
			//moduleResult = 1;
		break;
		case HGDBG_MOD3:
			moduleResult = hgDbg.flags.bits.mod3;
			//moduleResult = 1;
		break;
		case HGDBG_MOD4:
			moduleResult = hgDbg.flags.bits.mod4;
			moduleResult = 1;
		break;
		case HGDBG_MOD5:
			moduleResult = hgDbg.flags.bits.mod5;
		break;
		case HGDBG_MOD6:
			moduleResult = hgDbg.flags.bits.mod6;
			//moduleResult = 1;
		break;
		case HGDBG_MOD7:
			moduleResult = hgDbg.flags.bits.mod7;
		break;
		case HGDBG_MOD8:
			moduleResult = hgDbg.flags.bits.mod8;
		break;
		case HGDBG_MOD9:
			moduleResult = hgDbg.flags.bits.mod9;
		break;
		case HGDBG_MOD10:
			moduleResult = hgDbg.flags.bits.mod10;
		break;
		case HGDBG_MOD11:
			moduleResult = hgDbg.flags.bits.mod11;
		break;
		case HGDBG_MOD12:
			moduleResult = hgDbg.flags.bits.mod12;
		break;
		case HGDBG_MOD13:
			moduleResult = hgDbg.flags.bits.mod13;
		break;
		case HGDBG_MOD14:
			moduleResult = hgDbg.flags.bits.mod14;
			break;
		case HGDBG_MOD15:
			moduleResult = hgDbg.flags.bits.mod15;
			break;
		case HGDBG_MOD16:
			moduleResult = hgDbg.flags.bits.mod16;
			break;
		case HGDBG_MOD17:
			moduleResult = hgDbg.flags.bits.mod17;
			break;
		case HGDBG_MOD18:
			moduleResult = hgDbg.flags.bits.mod18;
			break;
		case HGDBG_MOD19:
			moduleResult = hgDbg.flags.bits.mod19;
			break;
		case HGDBG_MOD32:
			//moduleResult = 1;
			break;
		default:
			break;
	}
	return moduleResult;
}
void hgDbg_setLevel(UINT32 level)
{
    hgDbgSetting.flags.all = level;
}
UINT32 hgDbg_getLevel(void)
{
	return (UINT32)hgDbgSetting.flags.all;
}
#endif //#if 1