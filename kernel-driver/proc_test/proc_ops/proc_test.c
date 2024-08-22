/*
 * @*************************************:
 * @FilePath     : /kernel-driver/proc_test/proc_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-12 10:58:17
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-12 13:09:08
 * @Descripttion : proc struct proc_ops test
 * @compile      :
 * @**************************************:
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

int our_proc_open(struct inode *, struct file *);
ssize_t our_proc_read(struct file *, char __user *, size_t, loff_t *);
ssize_t our_proc_write(struct file *, const char __user *, size_t, loff_t *);

struct proc_ops ops = {

	.proc_open = our_proc_open,
	.proc_read = our_proc_read,
	.proc_write = our_proc_write
};

int our_proc_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "HELLO,our_proc_open\n");
	return 0;
};

ssize_t our_proc_read(struct file *file, char __user *buffer, size_t size, loff_t *off_t)
{
	printk(KERN_ALERT "HELLO,our_proc_read\n");
	return 0;
};

ssize_t our_proc_write(struct file *file, const char __user *buffer, size_t size, loff_t * off_t)
{
	char buf[32] = {0};

	printk(KERN_ALERT "\nHELLO,our_proc_write\n");

	if (*off_t > 0 || size > 32)
		return -EFAULT;

	if (copy_from_user(buf, buffer, size))
		return -EFAULT;
	
	printk(KERN_ALERT "HELLO,our_proc_write %s\n", buf);

	return size;
}

static int __init our_proc_init(void)
{

	printk(KERN_ALERT "HELLO,KERNEL\n");

	// 挂载proc
	proc_create_data("our_proc", 0644, NULL, &ops, NULL);
	return 0;
}

static void __exit our_proc_exit(void)
{
	printk(KERN_ALERT "HELLO,KERNEL BYE BYE\n");
	remove_proc_entry("our_proc", NULL);
}

module_init(our_proc_init);
module_exit(our_proc_exit);
MODULE_LICENSE("Dual BSD/GPL");

MODULE_AUTHOR("Dof");
