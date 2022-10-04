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

#define HMC5883L_CRA_BASE 0x00  //配置寄存器A(Configuration Register A)
#define HMC5883L_CRB_BASE 0x01  //配置寄存器B
#define HMC5883L_MR_BASE 0x02   //模式寄存器
#define HMC5883L_DXRA_BASE 0x03 //数据输出X MSB寄存器
#define HMC5883L_DXRB_BASE 0x04 //数据输出X LSB寄存器
#define HMC5883L_DZRA_BASE 0x05 //数据输出Z MSB寄存器
#define HMC5883L_DZRB_BASE 0x06 //数据输出Z LSB寄存器
#define HMC5883L_DYRA_BASE 0x07 //数据输出Y MSB寄存器
#define HMC5883L_DYRB_BASE 0x08 //数据输出Y LSB寄存器
#define HMC5883L_SB_BASE 0x09   //状态寄存器
#define HMC5883L_IRA_BASE 0x0A  //识别寄存器A
#define HMC5883L_IRB_BASE 0x0B  //识别寄存器B
#define HMC5883L_IRC_BASE 0x0C  //识别寄存器C

struct atm24c02_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int major;
    void *private_data;
    char buf[16];
};

static struct atm24c02_dev atm24c02dev;

/*  设备操作流程 */

// 写操作

static s32 atm24c02_write_regs(struct atm24c02_dev *dev, u8 reg, u8 *buf, u8 len)
{
    u8 b[256];
    struct i2c_msg msg;
    struct i2c_client *client = (struct i2c_client *)dev->private_data;

    b[0] = reg;              /* 寄存器首地址 */
    memcpy(&b[1], buf, len); /* 将要写入的数据拷贝到数组b里面 */

    msg.addr = client->addr; // 设备地址
    msg.flags = 0;           // 写标志位

    msg.buf = b;       // 写入的数据
    msg.len = len + 1; //  数据长度

    return i2c_transfer(client->adapter, &msg, 1);
}

static void atm24c02_write_reg(struct atm24c02_dev *dev, u8 reg, u8 data)
{
    u8 buf = 0;
    buf = data;

    atm24c02_write_regs(dev, reg, &buf, 1);
}

//  读操作
static int atm24c02_read_regs(struct atm24c02_dev *dev, u8 reg, void *val, int len)
{
    int ret;
    struct i2c_msg msg[2];
    struct i2c_client *client = (struct i2c_client *)dev->private_data;

    // 先写设备地址
    msg[0].addr = client->addr; // 设备地址
    msg[0].flags = 0;           // 写标志位
    msg[0].buf = &reg;          // 寄存器地址
    msg[0].len = 1;             // 寄存器地址长度

    // 读数据buf
    msg[1].addr = client->addr; // 设备地址
    msg[1].flags = I2C_M_RD;    // 读标志位
    msg[1].buf = val;           // 读取的数据
    msg[1].len = len;           // 读取长度

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret == 2)
    {
        ret = 0;
    }
    else
    {
        printk("i2c fail= %d reg = %06x len = %d\n", ret, reg, len);
        return -EREMOTEIO;
    }
    return ret;
}

static unsigned char atm24c02_read_reg(struct atm24c02_dev *dev, u8 reg)
{
    u8 data;
    atm24c02_read_regs(dev, reg, &data, 1);
    return data;
}

void atm24c02_readdata(struct atm24c02_dev *dev)
{
    int i = 0;
    unsigned char buf[256];

    for (i = 0; i < 6; i++)
    {
        dev->buf[i] = atm24c02_read_reg(dev, 0x03 + i);
    }
}

static int atm24c02_open(struct inode *inode, struct file *filp)
{
    char data[12] = {0};
    filp->private_data = &atm24c02dev; /* 设置私有数据 */

    mdelay(70);
    atm24c02_write_reg(&atm24c02dev, 0x00, 0x14);
    mdelay(50);
    atm24c02_write_reg(&atm24c02dev, 0x01, 0x00);
    mdelay(50);
    atm24c02_write_reg(&atm24c02dev, 0x02, 0x00);
    mdelay(50);

    data[0] = atm24c02_read_reg(&atm24c02dev, HMC5883L_IRA_BASE);
    mdelay(50);
    data[1] = atm24c02_read_reg(&atm24c02dev, HMC5883L_IRB_BASE);
    mdelay(50);
    data[2] = atm24c02_read_reg(&atm24c02dev, HMC5883L_IRC_BASE);
    mdelay(50);

    printk("0x%x 0x%x 0x%x\n", data[0], data[1], data[2]);

    return 0;
}

static ssize_t atm24c02_read(struct file *filp, char __user *buf, size_t cnt, loff_t off)
{
    short data[256];
    long err = 0;
    int i = 0;

    struct atm24c02_dev *dev = (struct atm24c02_dev *)filp->private_data;

    memset(dev->buf, '0', sizeof(dev->buf));
    data[0] = atm24c02_read_reg(dev, HMC5883L_SB_BASE);
    printk("0x%x ", data[0]);
    atm24c02_readdata(dev);
    err = copy_to_user(buf, dev->buf, sizeof(dev->buf));
    return 0;
}

static int atm24c02_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations atm24c02_ops =
    {
        .owner = THIS_MODULE,
        .open = atm24c02_open,
        .read = atm24c02_read,
        .release = atm24c02_release,
};

/*
 * 注册设备流程 
 */
static int
atm24c02_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
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

    cdev_init(&atm24c02dev.cdev, &atm24c02_ops);
    cdev_add(&atm24c02dev.cdev, atm24c02dev.devid, ATM24C02_CNT);

    atm24c02dev.class = class_create(THIS_MODULE, ATM24C02_NAME);
    if (IS_ERR(atm24c02dev.class))
    {
        return PTR_ERR(atm24c02dev.class);
    }

    atm24c02dev.device = device_create(atm24c02dev.class, NULL, atm24c02dev.devid, NULL, ATM24C02_NAME);
    if (IS_ERR(atm24c02dev.device))
    {
        return PTR_ERR(atm24c02dev.device);
    }

    atm24c02dev.private_data = client;

    return 0;
}

static int atm24c02_remove(struct i2c_client *client)
{
    cdev_del(&atm24c02dev.cdev);
    unregister_chrdev_region(atm24c02dev.devid, ATM24C02_CNT);

    device_destroy(atm24c02dev.class, atm24c02dev.devid);
    class_destroy(atm24c02dev.class);
    return 0;
}

static const struct i2c_device_id atm24c02_id[] =
    {
        {"dof,hmc5883", 0},
        {},
};

static const struct of_device_id atm24c02_of_match[] =
    {
        {.compatible = "dof,hmc5883"},
        {
            /* Sentinel */
        },
};

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

static int __init
atm24c02_init(void)
{
    i2c_add_driver(&atm24c02_driver);

    return 0;
}

static void __exit atm24c02_exit(void)
{
    i2c_del_driver(&atm24c02_driver);
}

module_init(atm24c02_init);
module_exit(atm24c02_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("dof");