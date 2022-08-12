/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

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
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <sys/ioctl.h>

#include "board.h"

#include "cms.h"
#include "cms_util.h"
#include "cms_boardioctl.h"

/*
 * See:
 * bcmdrivers/opensource/include/bcm963xx/board.h
 * bcmdrivers/opensource/char/board/bcm963xx/impl1/board.c and bcm63xx_led.c
 */

void cmsLed_setWanConnected(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedWanData, kLedStateOn, NULL);
}


void cmsLed_setWanDisconnected(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedWanData, kLedStateOff, NULL);
}


void cmsLed_setWanFailed(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedWanData, kLedStateFail, NULL);
}

//__HG_BGN__ Added by zhangxs, 2021-12-07
void cmsLed_setVoip1Connected(void)
{
    int led = kLedVoip1;
    
    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, led, kLedStateOn, NULL);
}

void cmsLed_setVoip1Disconnected(void)
{
    int led = kLedVoip1;

    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, led, kLedStateOff, NULL);
}

void cmsLed_setVoip1DataTransmission(void)
{
    int led = kLedVoip1;

    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, led, kLedStateFastBlinkContinues, NULL);
}

void cmsLed_setVoip2Connected(void)
{
    int led = kLedVoip2;
        
    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, led, kLedStateOn, NULL);
}

void cmsLed_setVoip2Disconnected(void)
{
    int led = kLedVoip2;

    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, led, kLedStateOff, NULL);
}
void cmsLed_setVoip2DataTransmission(void)
{
    int led = kLedVoip2;

    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, led, kLedStateFastBlinkContinues, NULL);
}

//__HG_END__ Added by zhangxs, 2021-12-07

//================================================================================
//__HG_BGN__ Added by fc, 2021-11-23
#if defined(BCM_PON) || defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
void cmsLed_setOpticalLinkSuccess(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedOpticalLink, kLedStateOff, NULL);
}
void cmsLed_setOpticalLinkFailed(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedOpticalLink, kLedStateSlowBlinkContinues, NULL);
}

/*--------------------------tjh added 02/12/2014-------------------------------*/
void cmsLed_setOpticalLinkShutdown(void)
{
    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedOpticalLink, kLedStateOn, NULL);
}
/*-----------------------------------------------------------------------------*/
/*--------------------------tjh added 03/10/2014-------------------------------*/
void cmsLed_setGponRegisterSuccess(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedGpon, kLedStateOn, NULL);
}
void cmsLed_setGponRegisterFailed(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedGpon, kLedStateOff, NULL);
}
void cmsLed_setGponRegistering(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedGpon, kLedStateFastBlinkContinues, NULL);
}
#endif
/*-----------------------------------------------------------------------------*/
//__HG_END__ Added by fc, 2021-11-23
//================================================================================

void cmsLed_setPowerOn(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedPower, kLedStateOn, NULL);
}


void cmsLed_setPowerOff(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedPower, kLedStateOff, NULL);
}

void cmsLed_setWpsOn(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedSes, kLedStateOn, NULL);
}

void cmsLed_setWpsOff(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedSes, kLedStateOff, NULL);
}
void cmsLed_setWpsBlink(void)
{
   devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedSes, kLedStateSlowBlinkContinues, NULL);
}

#if 0
void cmsLed_setLanOn(int index)
{
    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedLan1+index, kLedStateOn, NULL);
}

void cmsLed_setLanOff(int index)
{
    devCtl_boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, kLedLan1+index, kLedStateOff, NULL);
}
#endif
//m by xc 2022.2.18 for chip num
int wlGetChipId(const char *name)
{
	static int wl0 = 0, wl1 = 0;
	int *wl;
	char buf[100];
	FILE *fp = NULL;
	char *p;
	
	if (!cmsUtl_strcmp(name, "wl0"))
	{
		if (wl0 > 0)
			return wl0;
		wl = &wl0;
	}
	else
	{
		if (wl1 > 0)
			return wl1;
		wl = &wl1;
	}
	snprintf(buf, sizeof(buf), "wl -i %s revinfo", name);
	if ((fp = popen(buf, "r")) == NULL) 
	{
		perror("popen sta_info error");
		return -1;
	}
	while (fgets(buf, sizeof(buf), fp)) 
	{	
		if (!strncmp(buf, "chipnum", 7) && (p = strrchr(buf, 'x')))
		{
			*wl = strtoul(p + 1, NULL, 16);
			break;
		}
	}
	pclose(fp);
	return *wl;
}

void wlSetWirelessLed(const char *name, int on)
{
	char cmd[100], ifname[20];
	char *p;
	int chipId;
	
	strcpy(ifname, name);
	if ((p = strchr(ifname, '.')))
		*p = 0;
	chipId = wlGetChipId(ifname);
	if (chipId == 0x6878)
		sprintf(cmd, "wl -i %s leds %d", ifname, on ? 1 : 0);
	else if (chipId == 0x4352)
		sprintf(cmd, "wl -i %s ledbh 10 %d", ifname, on ? 8 : 0);
	else if (chipId == 0xaaac)
		sprintf(cmd, "wl -i %s ledbh 6 %d", ifname, on ? 8 : 0);
	else
		sprintf(cmd, "wl -i %s ledbh 3 %d", ifname, on ? 8 : 0);
	system(cmd);
	cmsLog_debug("%s\n", name, cmd);		
}

void cmsLed_switch(int flag)
{
#ifdef WIRELESS
#if defined(CONFIG_BCM96878)
    if(flag == 0)
    {
        system("bin/wlctl -i wl1 ledbh 3 0");
    }
    else
    {
        system("bin/wlctl -i wl1 ledbh 3 1");
    }
    
#elif defined(CONFIG_BCM96846)
   if(flag == 0)
   {
       system("bin/wlctl -i wl0 ledbh 6 0");
       system("bin/wlctl -i wl1 ledbh 6 0");
   }
   else
   {
       system("bin/wlctl -i wl0 ledbh 6 1");
       system("bin/wlctl -i wl1 ledbh 6 1");
   }

#endif
	if (flag)
	{
		wlSetWirelessLed("wl0", 1);
	}	
	else
	{	
		wlSetWirelessLed("wl0", 0);
	}
#endif    
   devCtl_boardIoctl(BOARD_IOCTL_LED_SWITCH, 0, NULL, flag, 0, NULL);
}

void cmsLed_setAllOnOff(int flag)
{
#ifdef WIRELESS
#if defined(CONFIG_BCM96878)
    if(flag == 0)
    {
        system("bin/wlctl -i wl1 ledbh 3 0");
    }
    else
    {
        system("bin/wlctl -i wl1 ledbh 3 1");
    }
    
#elif defined(CONFIG_BCM96846)
   if(flag == 0)
   {
       system("bin/wlctl -i wl0 ledbh 6 0");
       system("bin/wlctl -i wl1 ledbh 6 0");
   }
   else
   {
       system("bin/wlctl -i wl0 ledbh 6 1");
       system("bin/wlctl -i wl1 ledbh 6 1");
   }

#endif
	if (flag)
	{
		wlSetWirelessLed("wl0", 1);
	}	
	else
	{	
		wlSetWirelessLed("wl0", 0);
	}
#endif    
   devCtl_boardIoctl(BOARD_IOCTL_LED_SET_ONOFF, 0, NULL, flag, 0, NULL);
}

int consoleSetSwitch(int onoff)
{
	int boardFd;
	BOARD_IOCTL_PARMS ioctlParms;
	int rc = 0;
	int ret = 0;

	boardFd = open("/dev/brcmboard", O_WRONLY);
	if ( boardFd == -1 )
	{
		cmsLog_error("open /dev/brcmboard failed(%d)!",boardFd);
		return -1;
	}
	
	memset(&ioctlParms,0,sizeof(ioctlParms));
	ioctlParms.strLen = onoff;
	ioctlParms.result = -1;

	rc = ioctl(boardFd, BOARD_IOCTL_CONSOLE_SET_SWITCH, &ioctlParms);
	if(rc < 0)
	{
		cmsLog_error("set ioctl failed(%d)!",rc);
		ret = -1;
	}
	//cmsLog_error("set ioctl %d success(%d)!",onoff,rc);

	close(boardFd);
	return ret;
}

int consoleGetSwitch(int *onoff)
{
	int boardFd;
	int rc = 0;
	int ret = -1;

	boardFd = open("/dev/brcmboard", O_WRONLY);
	if ( boardFd == -1 )
	{
		cmsLog_error("open /dev/brcmboard failed(%d)!",boardFd);
		return ret;
	}
	
	rc = ioctl(boardFd, BOARD_IOCTL_CONSOLE_GET_SWITCH, (void *)NULL);

	close(boardFd);
	if(rc ==0 || rc == 1)
	{
		*onoff = rc;
		ret = 0;
	}
	//cmsLog_error("get consoleSwitch:%d onoff:%d ",rc,*onoff);
	return ret;
}