/*
 * @FilePath: \module_read\test.c
 * @version: 
 * @Author: dof
 * @Date: 2021-03-01 16:32:26
 * @LastEditors: dof
 * @LastEditTime: 2021-03-02 09:57:27
 * @Descripttion: 
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "head.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int ret;
    int fd;
    char buf[125] = {0};
    char wbuf[125] = "i am from test";

    fd = open("/dev/globalmem", O_RDWR);
    if (fd < 0)
    {
        printf(" open ./globalmem fail!!!\n");
        return -1;
    }
    else
    {
        printf(" open success,fd = %d\n", fd);
    }

    printf("init data wbuf = %s\n", wbuf);
    ret = write(fd, wbuf, strlen(wbuf) + 1);
    if (ret >= 0)
    {
        printf("success write\n");
        ioctl(fd,HELLO_ONE);
        ioctl(fd,HELLO_TWO,99);
    }


    memset(buf, '\0', sizeof(buf));
    ret = read(fd, buf, strlen(buf));
    if (ret <= 0)
    {
        printf(" read fail!!!\n");
    }
    else
    {
        printf("buf = %s,ret = %d\n", buf, ret);
    }

    close(fd);
    return 0;
}
