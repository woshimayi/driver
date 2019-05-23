#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platfrom_device.h>
#include <asm/io.h>



static int major = 250;
static inr minor = 0;
static dev_t devno;
static struct class *cls;
static struct device * test_device;


#define TCFG0       0x0000
#define TCFG1       0x0004
#define TCON        0x0008
#define TCNTB0      0x000C
#define TCMPB0      0x0010


static unsigned int * gpd0con;
static void * timer_base;

#define MAGIC_NUMBER    'k'
#define BEEP_ON     _IO(MAGIC_NUMBER, 0)
#define BEEP_OFF    _IO(MAGIC_NUMBER, 1)
#define BEEP_FREQ   _IO(MAGIC_NUMBER, 2)


static int beep_open(struct inode * inode, struct file * filep)
{

}

static int beep_release(struct inode * inode, struct file * filep)
{

}

static long beep_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{

}

static struct file_operation beep_ops=
{
    .open = beep_open,
    .release = beep_release,
    .unlocked_ioctl = beep_ioctl,
};


static int beep_probe(struct platform_device * pdev)
{
    
}

static int beep_remove(struct platform_device * pdev)
{
    
}

static struct platform_driver beep_driver=
{
    .driver.name = "bigbang",
    .probe = beep_probe,
    .remove = beep_remove,
};


static void beep_init()
{

}

static void beep_exit()
{

}


MODULE_LICENSE("GPL");
MODULE_init(beep_init);
MODULE_init(beep_exit);


