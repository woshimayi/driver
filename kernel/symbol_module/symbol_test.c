#include <linux/init.h>
#include <linux/module.h>

extern void just_a_try(void);

static int __init test_init(void)
{
	printk(KERN_ALERT"Can you see the sysbol exported beform\n");
	just_a_try();
	return 0;
}
module_init(test_init);

static void __exit test_exit(void)
{
	printk(KERN_ALERT"module test finished\n");
}
module_exit(test_exit);

MODULE_LICENSE("GPL v2");
