#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"

/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: miscbeepApp.c
作者	  	: dof
版本	   	: V1.0
描述	   	: MISC驱动框架下的beep测试APP。
其他	   	: 无
使用方法	 ：./miscbeepApp  /dev/miscbeep  0 关闭蜂鸣器
		     ./misdcbeepApp /dev/miscbeep  1 打开蜂鸣器
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/8/20 dof创建
***************************************************************/
#define BEEPOFF 0
#define BEEPON 1
/* 命令值 */
#define CLOSE_CMD 		(_IO(0XEF, 0x1))	/* 关闭定时器 */
#define OPEN_CMD		(_IO(0XEF, 0x2))	/* 打开定时器 */
#define SETPERIOD_CMD	(_IO(0XEF, 0x3))	/* 设置定时器周期命令 */


typedef struct stepmotor
{
    int ctrl;  // 0:stop 1:线路检测 2:Clockwise 顺指针 3:Counter clockwise 逆时针
    int angle; // angle 运转角度
    int con;   // continue 持续运转
} _stepMotor;

#define INPUT_DEV "/dev/input/event1"
#define STEP_DEV "/dev/stepmotor"

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
    int fd, retvalue;
    char *filename;
    unsigned char databuf[1];
    unsigned char str[100];
    unsigned int cmd;
    unsigned int arg;

    fd = open(STEP_DEV, O_RDWR); /* 打开beep驱动 */
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", argv[1]);
        return -1;
    }

    while (1)
    {
        printf("Input CMD:");
        retvalue = scanf("%d", &cmd);
        if (retvalue != 1)  				/* 参数输入错误 */
        {
            scanf("%s", str);				/* 防止卡死 */
        }

        if (cmd == 1)				/* 关闭LED灯 */
            cmd = CLOSE_CMD;
        else if (cmd == 2)			/* 打开LED灯 */
            cmd = OPEN_CMD;
        else if (cmd == 3)
        {
            cmd = SETPERIOD_CMD;	/* 设置周期值 */
            printf("Input Timer Period:");
            retvalue = scanf("%d", &arg);
            if (retvalue != 1)  			/* 参数输入错误 */
            {
                scanf("%s", str);			/* 防止卡死 */
            }
        }
        ioctl(fd, cmd, arg);		/* 控制定时器的打开和关闭 */
    }

    

    retvalue = close(fd); /* 关闭文件 */
    if (retvalue < 0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
