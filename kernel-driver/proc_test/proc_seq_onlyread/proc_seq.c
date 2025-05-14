/*
 * @*************************************:
 * @FilePath     : /kernel-driver/proc_test/proc_seq_onlyread/proc_seq.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-12 13:18:42
 * @LastEditors  : dof
 * @LastEditTime : 2025-04-22 17:36:29
 * @Descripttion : proc create dir
 * @compile      :
 * @**************************************:
 */

// #include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/sched.h> // #include <linux/sched/signal.h>
#include <linux/version.h>

#ifndef PP
#define PP(fmt, args...) printk("[zzzzz :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)
#endif

#define PROC_DIR_NAME "my_module"
#define PROC_FILE_NAME "my_attribute_new"

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_file = NULL;

// 当用户读取 /proc/my_module/my_attribute_new 时调用的函数
static int my_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Hello from my_module!\n");
    return 0;
}

// 打开 /proc 文件时调用
static int my_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_proc_show, NULL);
}

// /proc 文件操作结构体
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops my_proc_fops = {
    .proc_open = my_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};
#else
static const struct file_operations my_proc_fops = {
    .open = my_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};
#endif

// 模块初始化函数
static int __init my_module_init(void)
{
    // 1. 创建 /proc/my_module 目录
    proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (!proc_dir)
    {
        pr_err("Failed to create /proc/%s\n", PROC_DIR_NAME);
        return -ENOMEM;
    }

    // 2. 在 /proc/my_module 下创建文件 my_attribute_new
    proc_file = proc_create(PROC_FILE_NAME, 0644, proc_dir, &my_proc_fops);
    if (!proc_file)
    {
        pr_err("Failed to create /proc/%s/%s\n", PROC_DIR_NAME, PROC_FILE_NAME);
        proc_remove(proc_dir);
        return -ENOMEM;
    }

    pr_info("Module loaded: /proc/%s/%s created\n", PROC_DIR_NAME, PROC_FILE_NAME);
    return 0;
}

// 模块退出函数
static void __exit my_module_exit(void)
{
    // 删除 /proc/my_module/my_attribute_new
    if (proc_file)
        proc_remove(proc_file);

    // 删除 /proc/my_module 目录
    if (proc_dir)
        proc_remove(proc_dir);

    pr_info("Module unloaded: /proc/%s removed\n", PROC_DIR_NAME);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example module with /proc interface");