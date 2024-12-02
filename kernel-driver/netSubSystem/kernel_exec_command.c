#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/init.h>

#undef PP
#define PP(fmt, args...) printk("[%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)

static int __init my_module_init(void)
{
    PP("Initializing custom protocol module\n");

    char *argv[] = {"/bin/ls", "-lh", "/", NULL};
    char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};

    PP("About to call usermodehelper\n");

    int ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret != 0)
    {
        PP("Failed to execute user space program: %d\n", ret);
    }
    else
    {
        PP("User space program executed successfully\n");
    }

    return 0;
}

static void __exit my_module_exit(void)
{
    PP("Exiting custom protocol module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Improved Custom Protocol Kernel Module");

