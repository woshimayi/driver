#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>

static struct resource beep_resource[] = 
{
    [0] = {
        .start = 0x114000a0,
        .end = 0x114000a0 + 0x4,
        .flags = IORESOURCE_MEM,
    },

    [1] = {
        .start = 0x139D0000,
        .end = 0x139D0000 + 0x14,
        .flags = IORESOURCE_MEM,
    }
};


static void hello_release(struct device *dev)
{
    printk("hello_release\n");
    return;
}


static struct platform_device hello_device=
{
    .name = "bigbang",
    .id = -1,
    .dev.releases = ARRAY_SIZE(beep_resource),
    .resource = beep_resource,
};

static int hello_init(void)
{
    printk("hello init\n");
    return platform_device_register(&hello_device);
}

static void hello_exit(void)
{
    printk("hello exit\n");
    platform_device_unregister(&hello_device);
}

MODULE_LICENSE("GPL");
MODULE_INIT(hello_init);
MODULE_INIT(hello_exit);
