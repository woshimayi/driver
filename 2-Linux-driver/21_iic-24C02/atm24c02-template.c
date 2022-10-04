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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "atm24c02reg.h"

#define ATM24C02_CNT 1
#define ATM24C02_NAME "atm24c02"

struct atm24c02_dev
{
    dev_t devid;                /* 设备号 	 */
    struct cdev cdev;           /* cdev 	*/
    struct class *class;        /* 类 		*/
    struct device *device;      /* 设备 	 */
    struct device_node *nd;     /* 设备节点 */
    int major;                  /* 主设备号 */
    void *private_data;         /* 私有数据 */
    unsigned short ir, als, ps; /* 三个光传感器数据 */
};

static struct atm24c02_dev atm24c02dev;

/*
 * @description	: 从atm24c02读取多个寄存器数据
 * @param - dev:  atm24c02设备
 * @param - reg:  要读取的寄存器首地址
 * @param - val:  读取到的数据
 * @param - len:  要读取的数据长度
 * @return 		: 操作结果
 */
static int atm24c02_read_regs(struct atm24c02_dev *dev, u8 reg, void *val, int len)
{
    int ret;
    struct i2c_msg msg[2];
    struct i2c_client *client = (struct i2c_client *)dev->private_data;

    /* msg[0]为发送要读取的首地址 */
    msg[0].addr = client->addr; /* atm24c02地址 */
    msg[0].flags = 0;           /* 标记为发送数据 */
    msg[0].buf = &reg;          /* 读取的首地址 */
    msg[0].len = 1;             /* reg长度*/

    /* msg[1]读取数据 */
    msg[1].addr = client->addr; /* atm24c02地址 */
    msg[1].flags = I2C_M_RD;    /* 标记为读取数据*/
    msg[1].buf = val;           /* 读取数据缓冲区 */
    msg[1].len = len;           /* 要读取的数据长度*/

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret == 2)
    {
        ret = 0;
    }
    else
    {
        printk("i2c rd failed=%d reg=%06x len=%d\n", ret, reg, len);
        ret = -EREMOTEIO;
    }
    return ret;
}

/*
 * @description	: 向atm24c02多个寄存器写入数据
 * @param - dev:  atm24c02设备
 * @param - reg:  要写入的寄存器首地址
 * @param - val:  要写入的数据缓冲区
 * @param - len:  要写入的数据长度
 * @return 	  :   操作结果
 */
static s32 atm24c02_write_regs(struct atm24c02_dev *dev, u8 reg, u8 *buf, u8 len)
{
    u8 b[256];
    struct i2c_msg msg;
    struct i2c_client *client = (struct i2c_client *)dev->private_data;

    b[0] = reg;              /* 寄存器首地址 */
    memcpy(&b[1], buf, len); /* 将要写入的数据拷贝到数组b里面 */

    msg.addr = client->addr; /* atm24c02地址 */
    msg.flags = 0;           /* 标记为写数据 */

    msg.buf = b;       /* 要写入的数据缓冲区 */
    msg.len = len + 1; /* 要写入的数据长度 */

    return i2c_transfer(client->adapter, &msg, 1);
}

/*
 * @description	: 读取atm24c02指定寄存器值，读取一个寄存器
 * @param - dev:  atm24c02设备
 * @param - reg:  要读取的寄存器
 * @return 	  :   读取到的寄存器值
 */
static unsigned char atm24c02_read_reg(struct atm24c02_dev *dev, u8 reg)
{
    u8 data = 0;

    atm24c02_read_regs(dev, reg, &data, 1);
    return data;

#if 0
    struct i2c_client *client = (struct i2c_client *)dev->private_data;
    return i2c_smbus_read_byte_data(client, reg);
#endif
}

/*
 * @description	: 向atm24c02指定寄存器写入指定的值，写一个寄存器
 * @param - dev:  atm24c02设备
 * @param - reg:  要写的寄存器
 * @param - data: 要写入的值
 * @return   :    无
 */
static void atm24c02_write_reg(struct atm24c02_dev *dev, u8 reg, u8 data)
{
    u8 buf = 0;
    buf = data;
    atm24c02_write_regs(dev, reg, &buf, 1);
}

/*
 * @description	: 读取ATM24C02的数据，读取原始数据，包括ALS,PS和IR, 注意！
 *				: 如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
 * @param - ir	: ir数据
 * @param - ps 	: ps数据
 * @param - ps 	: als数据
 * @return 		: 无。
 */
void atm24c02_readdata(struct atm24c02_dev *dev)
{
    unsigned char i = 0;
    unsigned char buf[6];

    /* 循环读取所有传感器数据 */
    for (i = 0; i < 6; i++)
    {
        buf[i] = atm24c02_read_reg(dev, ATM24C02_IRDATALOW + i);
    }

    if (buf[0] & 0X80) /* IR_OF位为1,则数据无效 */
        dev->ir = 0;
    else /* 读取IR传感器的数据   		*/
        dev->ir = ((unsigned short)buf[1] << 2) | (buf[0] & 0X03);

    dev->als = ((unsigned short)buf[3] << 8) | buf[2]; /* 读取ALS传感器的数据 			 */

    if (buf[4] & 0x40) /* IR_OF位为1,则数据无效 			*/
        dev->ps = 0;
    else /* 读取PS传感器的数据    */
        dev->ps = ((unsigned short)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int atm24c02_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &atm24c02dev;

    /* 初始化ATM24C02 */
    // atm24c02_write_reg(&atm24c02dev, ATM24C02_SYSTEMCONG, 0x04); /* 复位ATM24C02 			*/
    // mdelay(50);                                                 /* ATM24C02复位最少10ms 	*/
    // atm24c02_write_reg(&atm24c02dev, ATM24C02_SYSTEMCONG, 0X03); /* 开启ALS、PS+IR 		*/
    return 0;
}

/*
 * @description		: 从设备读取数据
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t atm24c02_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
    short data[3];
    long err = 0;

    struct atm24c02_dev *dev = (struct atm24c02_dev *)filp->private_data;

    atm24c02_readdata(dev);

    data[0] = dev->ir;
    data[1] = dev->als;
    data[2] = dev->ps;
    err = copy_to_user(buf, data, sizeof(data));
    return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int atm24c02_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* ATM24C02操作函数 */
static const struct file_operations atm24c02_ops =
    {
        .owner = THIS_MODULE,
        .open = atm24c02_open,
        .read = atm24c02_read,
        .release = atm24c02_release,
};

/*
 * @description     : i2c驱动的probe函数，当驱动与
 *                    设备匹配以后此函数就会执行
 * @param - client  : i2c设备
 * @param - id      : i2c设备ID
 * @return          : 0，成功;其他负值,失败
 */
static int atm24c02_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    /* 1、构建设备号 */
    if (atm24c02dev.major)
    {
        atm24c02dev.devid = MKDEV(atm24c02dev.major, 0);
        register_chrdev_region(atm24c02dev.devid, ATM24C02_CNT, ATM24C02_NAME);
    }
    else
    {
        alloc_chrdev_region(&atm24c02dev.devid, 0, ATM24C02_CNT, ATM24C02_NAME);
        atm24c02dev.major = MAJOR(atm24c02dev.devid);
    }

    /* 2、注册设备 */
    cdev_init(&atm24c02dev.cdev, &atm24c02_ops);
    cdev_add(&atm24c02dev.cdev, atm24c02dev.devid, ATM24C02_CNT);

    /* 3、创建类 */
    atm24c02dev.class = class_create(THIS_MODULE, ATM24C02_NAME);
    if (IS_ERR(atm24c02dev.class))
    {
        return PTR_ERR(atm24c02dev.class);
    }

    /* 4、创建设备 */
    atm24c02dev.device = device_create(atm24c02dev.class, NULL, atm24c02dev.devid, NULL, ATM24C02_NAME);
    if (IS_ERR(atm24c02dev.device))
    {
        return PTR_ERR(atm24c02dev.device);
    }

    atm24c02dev.private_data = client;

    return 0;
}

/*
 * @description     : i2c驱动的remove函数，移除i2c驱动的时候此函数会执行
 * @param - client 	: i2c设备
 * @return          : 0，成功;其他负值,失败
 */
static int atm24c02_remove(struct i2c_client *client)
{
    /* 删除设备 */
    cdev_del(&atm24c02dev.cdev);
    unregister_chrdev_region(atm24c02dev.devid, ATM24C02_CNT);

    /* 注销掉类和设备 */
    device_destroy(atm24c02dev.class, atm24c02dev.devid);
    class_destroy(atm24c02dev.class);
    return 0;
}

/* 传统匹配方式ID列表 */
static const struct i2c_device_id atm24c02_id[] =
    {
        {"alientek,atm24c02", 0},
        {}};

/* 设备树匹配列表 */
static const struct of_device_id atm24c02_of_match[] =
    {
        {.compatible = "dof,atm24c02"},
        {/* Sentinel */}};

/* i2c驱动结构体 */
static struct i2c_driver atm24c02_driver =
    {
        .probe = atm24c02_probe,
        .remove = atm24c02_remove,
        .driver = {
            .owner = THIS_MODULE,
            .name = "atm24c02",
            .of_match_table = atm24c02_of_match,
        },
        .id_table = atm24c02_id,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init atm24c02_init(void)
{
    int ret = 0;

    ret = i2c_add_driver(&atm24c02_driver);
    return ret;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit atm24c02_exit(void)
{
    i2c_del_driver(&atm24c02_driver);
}

/* module_i2c_driver(atm24c02_driver) */

module_init(atm24c02_init);
module_exit(atm24c02_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("dof");
