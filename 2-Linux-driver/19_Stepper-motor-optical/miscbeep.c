#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: miscbeep.c
作者	  	: dof
版本	   	: V1.0
描述	   	: 采用MISC的蜂鸣器驱动程序。
其他	   	: 无
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/8/20 dof创建
***************************************************************/
#define MISCBEEP_NAME "stepmotor" /* 设备名字 */
#define MISCBEEP_MINOR 144        /* 子设备号 */
#define BEEPOFF 0                 /* 关蜂鸣器 */
#define BEEPON 1                  /* 开蜂鸣器 */
#define BEEP_2 2                  /* Clockwise */
#define BEEP_3 3                  /* Counterclockwise  */
#define BEEP_4 4                  /* Continuous operation   */

// uint16_t CCW[8] = {0x08, 0x0c, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09};
// uint16_t CW[8] = {0x09, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08};

// uint16_t CCW[8] = {0x08, 0x02, 0x04, 0x01};
// uint16_t  CW[8] = {0x01, 0x04, 0x02, 0x08};

uint16_t CCW[4] = {0x0a, 0x06, 0x05, 0x09};
uint16_t CW[4] = {0x09, 0x05, 0x06, 0x0a};

/* miscbeep设备结构体 */
struct miscbeep_dev
{
    dev_t devid;            /* 设备号 	 */
    struct cdev cdev;       /* cdev 	*/
    struct class *class;    /* 类 		*/
    struct device *device;  /* 设备 	 */
    struct device_node *nd; /* 设备节点 */
    int beep_gpio[4];       /* beep所使用的GPIO编号		*/
};

struct miscbeep_dev miscbeep; /* beep设备 */

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int miscbeep_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &miscbeep; /* 设置私有数据 */

    return 0;
}

#if 0
/**
 * [motor_ccw 电机顺时针一周]
 */
static void motor_ccw(int time)
{
    uint8_t i, j;
    for (j = 0; j < 4; j++) // 电机内部运转一周
    {
        for (i = 0; i < 4; i++)
        {
            gpio_set_value(miscbeep.beep_gpio[0], ~(CCW[i] >> 3) & 0x01);
            gpio_set_value(miscbeep.beep_gpio[1], ~(CCW[i] >> 2) & 0x01);
            gpio_set_value(miscbeep.beep_gpio[2], ~(CCW[i] >> 1) & 0x01);
            gpio_set_value(miscbeep.beep_gpio[3], ~(CCW[i] >> 0) & 0x01);
            mdelay(time);
            gpio_set_value(miscbeep.beep_gpio[0], 0);
            gpio_set_value(miscbeep.beep_gpio[1], 0);
            gpio_set_value(miscbeep.beep_gpio[2], 0);
            gpio_set_value(miscbeep.beep_gpio[3], 0);
            mdelay(time);
        }
    }
}

/**
 * [motor_cw 电机逆时针一周]
 */
static void motor_cw(int time)
{
    uint8_t i, j;
    for (j = 0; j < 4; j++) // 电机内部运转一周
    {
        for (i = 0; i < 4; i++)
        {
            gpio_set_value(miscbeep.beep_gpio[0], ~(CW[i] >> 3) & 0x01);
            gpio_set_value(miscbeep.beep_gpio[1], ~(CW[i] >> 2) & 0x01);
            gpio_set_value(miscbeep.beep_gpio[2], ~(CW[i] >> 1) & 0x01);
            gpio_set_value(miscbeep.beep_gpio[3], ~(CW[i] >> 0) & 0x01);
            mdelay(time);
            gpio_set_value(miscbeep.beep_gpio[0], 0);
            gpio_set_value(miscbeep.beep_gpio[1], 0);
            gpio_set_value(miscbeep.beep_gpio[2], 0);
            gpio_set_value(miscbeep.beep_gpio[3], 0);
            mdelay(time);
        }
    }
}

// 指定运转角度
static void motor_angle_speed(int angle, int direction)
{
    int i, N;
    int t = 2;
    N = angle * 64 / 360; // 算出 需要运行的圈数
    for (i = 0; i < N; i++)
    {
        if (0 == direction)
        {
            motor_ccw(t);
        }
        else if (1 == direction)
        {
            motor_cw(t);
        }
    }
}

// 持续运转
static void motor_speed(uint8_t direction)
{
    int t = 2;
    do
    {
        if (0 == direction)
        {
            motor_ccw(t);
        }
        else if (1 == direction)
        {
            motor_cw(t);
        }
    } while (1);
}
#endif

uint8_t direction = -1;

void step_motor(void)
{
    uint8_t time = 3;
    uint8_t i = 0;
    if (0 == direction)
    {
        gpio_set_value(miscbeep.beep_gpio[0], 0);
    }
    else if (1 == direction)
    {
        gpio_set_value(miscbeep.beep_gpio[0], 1);
    }
    else
    {
        // To do
    }
    //  16 * 20
    for (i = 0; i < 20; i++)
    {
        mdelay(time);
        gpio_set_value(miscbeep.beep_gpio[1], 1);
        mdelay(time);
        gpio_set_value(miscbeep.beep_gpio[1], 0);
    }
}

/*
 * @description		: 向设备写数据
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t miscbeep_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char beepstat;
    struct miscbeep_dev *dev = filp->private_data;

    retvalue = copy_from_user(databuf, buf, cnt);
    if (retvalue < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    beepstat = databuf[0];   /* 获取状态值 */
    if (beepstat == BEEPOFF) // close
    {
        gpio_set_value(dev->beep_gpio[0], 0);
        gpio_set_value(dev->beep_gpio[1], 0);
        gpio_set_value(dev->beep_gpio[2], 0);
        gpio_set_value(dev->beep_gpio[3], 0);
    }
    else if (beepstat == BEEPON) // open
    {
        gpio_set_value(dev->beep_gpio[0], 1);
        gpio_set_value(dev->beep_gpio[1], 1);
        gpio_set_value(dev->beep_gpio[2], 1);
        gpio_set_value(dev->beep_gpio[3], 1);
    }
    else if (beepstat == BEEP_2)
    {
        direction = 0;
        step_motor();
    }
    else if (beepstat == BEEP_3)
    {
        direction = 1;
        step_motor();
    }
    else if (beepstat == BEEP_4)
    {
        /* code */
    }

    return 0;
}

/* 设备操作函数 */
static struct file_operations miscbeep_fops =
    {
        .owner = THIS_MODULE,
        .open = miscbeep_open,
        .write = miscbeep_write,
};

/* MISC设备结构体 */
static struct miscdevice beep_miscdev =
    {
        .minor = MISCBEEP_MINOR,
        .name = MISCBEEP_NAME,
        .fops = &miscbeep_fops,
};

/*
 * @description     : flatform驱动的probe函数，当驱动与
 *                    设备匹配以后此函数就会执行
 * @param - dev     : platform设备
 * @return          : 0，成功;其他负值,失败
 */
static int miscbeep_probe(struct platform_device *dev)
{
    int ret = 0;
    int i = 0;
    printk("beep driver and device was matched!\r\n");
    /* 设置BEEP所使用的GPIO */
    /* 1、获取设备节点：dofbeep */
    miscbeep.nd = of_find_node_by_path("/dofStepperMotor");
    if (miscbeep.nd == NULL)
    {
        printk("beep node not find!\r\n");
        return -EINVAL;
    }

    for (i = 0; i < 4; i++)
    {
        /* 2、 获取设备树中的gpio属性，得到BEEP所使用的BEEP编号 */
        miscbeep.beep_gpio[i] = of_get_named_gpio(miscbeep.nd, "stepmotor-gpios", i);
        if (miscbeep.beep_gpio[i] < 0)
        {
            printk("can't get beep-gpio");
            return -EINVAL;
        }

        /* 3、设置GPIO5_IO01为输出，并且输出高电平，默认关闭BEEP */
        ret = gpio_direction_output(miscbeep.beep_gpio[i], 0);
        if (ret < 0)
        {
            printk("can't set gpio!\r\n");
        }
    }

    /* 一般情况下会注册对应的字符设备，但是这里我们使用MISC设备
     * 所以我们不需要自己注册字符设备驱动，只需要注册misc设备驱动即可
     */
    ret = misc_register(&beep_miscdev);
    if (ret < 0)
    {
        printk("misc device register failed!\r\n");
        return -EFAULT;
    }

    return 0;
}

/*
 * @description     : platform驱动的remove函数，移除platform驱动的时候此函数会执行
 * @param - dev     : platform设备
 * @return          : 0，成功;其他负值,失败
 */
static int miscbeep_remove(struct platform_device *dev)
{
    /* 注销设备的时候关闭LED灯 */
    gpio_set_value(miscbeep.beep_gpio[0], 1);
    gpio_set_value(miscbeep.beep_gpio[1], 1);
    gpio_set_value(miscbeep.beep_gpio[2], 1);
    gpio_set_value(miscbeep.beep_gpio[3], 1);

    /* 注销misc设备 */
    misc_deregister(&beep_miscdev);
    return 0;
}

/* 匹配列表 */
static const struct of_device_id beep_of_match[] =
    {
        {.compatible = "dof-stepmotor"},
        {/* Sentinel */}};

/* platform驱动结构体 */
static struct platform_driver beep_driver =
    {
        .driver = {
            .name = "imx6ul-beep",           /* 驱动名字，用于和设备匹配 */
            .of_match_table = beep_of_match, /* 设备树匹配表          */
        },
        .probe = miscbeep_probe,
        .remove = miscbeep_remove,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init miscbeep_init(void)
{
    return platform_driver_register(&beep_driver);
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit miscbeep_exit(void)
{
    platform_driver_unregister(&beep_driver);
}

module_init(miscbeep_init);
module_exit(miscbeep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("dof");
