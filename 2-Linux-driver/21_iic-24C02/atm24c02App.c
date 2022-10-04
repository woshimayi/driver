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
    char databuf[256];
    unsigned short ir, als, ps;
    int ret = 0;
    int i = 0;

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

    ret = write(fd, databuf, sizeof(databuf));
    if (ret == 0)
    {
        printf("write error \n");
    }

    sleep(2);

    ret = read(fd, databuf, sizeof(databuf));
    if (ret == 0) /* 数据读取成功 */
    {

        printf("user dddddddddddddddddddddd\n");
        for (i = 0; i < 256; i++)
        {
            if (i % 8 == 0)
            {
                printf("\n");
            }
            printf("%4x", databuf[i]);
        }
        // printf("\033[1A");
        // printf("\033[K");
    }

    strncpy(databuf, "asdadd", 7);
    ret = write(fd, databuf, sizeof(databuf));

    // usleep(200000); /*100ms */
    // }
    close(fd); /* 关闭文件 */
    return 0;
}
