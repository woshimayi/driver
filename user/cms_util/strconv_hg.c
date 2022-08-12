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

static int checkStr(char *s1, char *s2) 
{ 
    for (int i = 0; i < strlen(s2); i++) 
    { 
        if (s1[i] != s2[i]) return 0; 
    } 
    return 1; 
} 
static void reStr(char *s1, char *s2, char *s3)
{ 
    int s1len = strlen(s1), 
    s2len = strlen(s2), 
    s3len = strlen(s3); 
    int n2 = s2len - s3len;

	if (n2 > 0) 
    { 
        for (int i = s2len; i < s1len; i++) 
        { 
            s1[i - n2] = s1[i]; 
        } 
        s1[s1len - n2] = '\0'; 
    } 
    else if (n2 < 0)
    {  
        for (int i = s1len; i >= s2len; i--) 
        { 
            s1[i - n2] = s1[i]; 
        } 
        s1[s1len - n2 + 1] = '\0'; 
    } 
    for (int i = 0; i < s3len; i++) 
    { 
        s1[i] = s3[i]; 
    } 
} 
int cmsUtl_replaceStr(char *s1, char *s2, char *s3)
{ 
    int n = 0;
    int s1len = 0;

    if (!s1 || !s2 || !s3)
        return 0;
    s1len = strlen(s1);
    if(!s1len)
    {
        return 0;
    }
    for (int i = s1len-1; i >= 0; i--) 
    { 
        if (s1[i] == s2[0] && checkStr(&s1[i], s2) == 1) 
        { 
            reStr(&s1[i], s2, s3);
            n++; 
        } 
    } 
    return n; 
} 

int cmsUtl_findChar (const char *s1,const char c)
{
    int i=0,num=0;
	
	if(!s1) return num;

	while(*(s1+i))
    {
       if(s1[i] == c)
	   	++num;
	   ++i;
    }
    return num;
}

