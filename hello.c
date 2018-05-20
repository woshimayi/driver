#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual bsd/GPL");

static int hello_init(void)
{
	printk("hello world");
	return 0;
}

static void hello_exit(void)
{
	printk("hello exit");
}


module_init(hello_init);
module_exit(hello_exit);
