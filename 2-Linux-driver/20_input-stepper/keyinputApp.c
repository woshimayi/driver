/*
 * @*************************************: 
 * @FilePath: /2-Linux-driver/20_input-stepper/keyinputApp.c
 * @version: 
 * @Author: dof
 * @Date: 2021-06-12 00:44:45
 * @LastEditors: dof
 * @LastEditTime: 2021-06-12 02:14:51
 * @Descripttion:   input子系统测试APP
 * @**************************************: 
 */
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
#include <linux/input.h>

/* 定义一个input_event变量，存放输入事件信息 */
static struct input_event inputevent;

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
    int fd1, fd2;
    int err = 0;
    unsigned char databuf[1];

    fd1 = open(INPUT_DEV, O_RDWR);
    if (fd1 < 0)
    {
        printf("Can't open file %s\r\n", INPUT_DEV);
        return -1;
    }

    fd2 = open(STEP_DEV, O_RDWR);
    if (fd2 < 0)
    {
        printf("Can't open file %s\r\n", STEP_DEV);
        return -1;
    }

    while (1)
    {
        err = read(fd1, &inputevent, sizeof(inputevent));
        if (err > 0) /* 读取数据成功 */
        {
            switch (inputevent.type)
            {

            case EV_KEY:
                memset(databuf, '\0', sizeof(1));
                if (inputevent.code < BTN_MISC) /* 键盘键值 */
                {
                    printf("key %d %s\r\n", inputevent.code, inputevent.value ? "press" : "release");

                    if (inputevent.value)
                    {
                        if (inputevent.code == 114)
                        {
                            databuf[0] = atoi("2"); /* 要执行的操作：打开或关闭 */
                        }
                        else if (inputevent.code == 115)
                        {
                            databuf[0] = atoi("3"); /* 要执行的操作：打开或关闭 */
                        }
                        else
                        {
                            databuf[0] = atoi("0"); /* 要执行的操作：打开或关闭 */
                        }
                        write(fd2, databuf, sizeof(databuf));
                    }
                    else
                    {
                        databuf[0] = atoi("0"); /* 要执行的操作：打开或关闭 */
                        write(fd2, databuf, sizeof(databuf));
                    }
                }
                else
                {
                    printf("button %d %s\r\n", inputevent.code, inputevent.value ? "press" : "release");
                    databuf[0] = atoi("0"); /* 要执行的操作：打开或关闭 */
                    write(fd2, databuf, sizeof(databuf));
                }
                break;

            /* 其他类型的事件，自行处理 */
            case EV_REL:
                break;
            case EV_ABS:
                break;
            case EV_MSC:
                break;
            case EV_SW:
                break;
            }
        }
        else
        {
            printf("读取数据失败\r\n");
        }
    }
    return 0;
}
