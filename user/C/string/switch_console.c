/*
 * @*************************************: 
 * @FilePath: /user/C/string/switch_console.c
 * @version: 
 * @Author: dof
 * @Date: 2023-02-02 13:33:41
 * @LastEditors: dof
 * @LastEditTime: 2023-02-02 13:36:59
 * @Descripttion:  关闭串口终端
 * @**************************************: 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef enum
{
    PERSISTENT,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
    SET_CS_PARAM,
    BACKUP_PSI,
    PSI_ACCESS_RIGHT,
    SYSLOG,
    ENVRAM
} BOARD_IOCTL_ACTION;

typedef struct boardIoctParms
{
    union {
     char *string;
     char *value;
 };
    union {
      char *buf;
      char *param;
    };
    union {
        int strLen;
        int value_length;
    };
    int offset;
    BOARD_IOCTL_ACTION action;
    int result;
} BOARD_IOCTL_PARMS;


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


