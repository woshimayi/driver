#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual bsd/GPL");

static int number = 100;
module_param(number, int, S_IRUGO);
static int hello_init(void)
{
	printk(KERN_ALERT"hello world number is: %d\n", number);
	return 0;
}
module_init(hello_init);

static void hello_exit(void)
{
	printk("hello exit");
}
module_exit(hello_exit);

void just_a_try(void)
{
	printk(KERN_ALERT"sysbol just a try!\n");
}
EXPORT_SYMBOL_GPL(just_a_try);

