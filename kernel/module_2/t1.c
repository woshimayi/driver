#include <linux/init.h>
#include <linux/module.h>

int init_module()
{
    printk(KERN_ALERT"Hi,hello world");
    return 0;
}

MODULE_LICENSE("GPL v2");
