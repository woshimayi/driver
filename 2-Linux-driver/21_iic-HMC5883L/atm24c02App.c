#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: atm24c02App.c
作者	  	: dof
版本	   	: V1.0
描述	   	: atm24c02设备测试APP。
其他	   	: 无
使用方法	 ：./atm24c02App /dev/atm24c02
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/9/20 dof创建
***************************************************************/

typedef struct HMCVALUE
{
    unsigned short hmcXvalue;
    unsigned short hmcYvalue;
    unsigned short hmcZvalue;
    double hmcAxis_x;
    double hmcAxis_y;
    double hmcAxis_z;
} HMCVALUE;

#define u_int16 unsigned short

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
    int fd;
    char *filename;
    char databuf[16];
    // unsigned short ir, als, ps;
    int ret = 0;
    HMCVALUE hmcValue;

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("can't open file %s\r\n", filename);
        return -1;
    }

    // ret = write(fd, databuf, sizeof(databuf));
    // if (ret == 0)
    // {
    //     printf("write error \n");
    // }

    // sleep(2);

    while (1)
    {
        memset(databuf, '0', sizeof(databuf));
        // memset((void *)hmcValue, '0', sizeof(hmcValue));
        ret = read(fd, databuf, sizeof(databuf));
        if (ret == 0) /* 数据读取成功 */
        {
            hmcValue.hmcXvalue = (u_int16)(databuf[0] << 8 | databuf[1]);
            hmcValue.hmcYvalue = (u_int16)(databuf[2] << 8 | databuf[3]);
            hmcValue.hmcZvalue = (u_int16)(databuf[4] << 8 | databuf[5]);
            // printf("x=%d y=%d z=%d\r", hmcValue.hmcXvalue, hmcValue.hmcYvalue, hmcValue.hmcZvalue);

            hmcValue.hmcAxis_x = atan2((double)hmcValue.hmcYvalue, (double)hmcValue.hmcXvalue) * (180 / 3.14159265) + 180; // angle in degrees
            hmcValue.hmcAxis_y = atan2((double)hmcValue.hmcYvalue, (double)hmcValue.hmcZvalue) * (180 / 3.14159265) + 180; // angle in degrees
            hmcValue.hmcAxis_z = atan2((double)hmcValue.hmcZvalue, (double)hmcValue.hmcXvalue) * (180 / 3.14159265) + 180; // angle in degrees
            printf("hmcAxis = %f  %f %f\r", hmcValue.hmcAxis_x, hmcValue.hmcAxis_y, hmcValue.hmcAxis_z);
            // printf("\033[1A");
            // printf("\033[K");
            sleep(1);
        }
    }
    // usleep(200000); /*100ms */
    // }
    close(fd); /* 关闭文件 */
    return 0;
}