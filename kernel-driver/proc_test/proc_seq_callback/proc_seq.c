/*
 * @*************************************:
 * @FilePath     : /kernel-driver/proc_test/proc_seq_callback/proc_seq.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-12 13:18:42
 * @LastEditors  : dof
 * @LastEditTime : 2025-04-22 17:07:32
 * @Descripttion : proc deq 读取大文件系统， 由于proc 只有单页缓存，读取大文件时有bug，采用seq, 定义proc 库，简化代码
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

#ifndef PP
#define PP(fmt, args...) printk("[zzzzz :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)
#endif

// static struct proc_dir_entry *entry;
static loff_t offset = 1;

static void *l_start(struct seq_file *m, loff_t *pos)
{
    loff_t index = *pos;
    loff_t i = 0;
    struct task_struct *task;
    PP();

    if (index == 0)
    {
        seq_printf(m, "Current all the processes in system:\n"
                      "%-24s%-5s\n",
                   "name", "pid");
        printk(KERN_EMERG "++++++++++=========>%5d\n", 0);
        //        offset = 1;
        return &init_task;
    }
    else
    {
        for (i = 0, task = &init_task; i < index; i++)
        {
            task = next_task(task);
        }
        BUG_ON(i != *pos);
        if (task == &init_task)
        {
            return NULL;
        }

        printk(KERN_EMERG "++++++++++>%5d\n", task->pid);
        return task;
    }
}

static void *l_next(struct seq_file *m, void *p, loff_t *pos)
{
    struct task_struct *task = (struct task_struct *)p;
    PP();
    task = next_task(task);
    if ((*pos != 0) && (task == &init_task))
    {
        //    if ((task == &init_task)) {
        //        printk(KERN_EMERG "=====>%5d\n", task->pid);
        return NULL;
    }

    printk(KERN_EMERG "=====>%5d\n", task->pid);
    offset = ++(*pos);

    return task;
}

static void l_stop(struct seq_file *m, void *p)
{
    PP();
    printk(KERN_EMERG "------>\n");
}

static int l_show(struct seq_file *m, void *p)
{
    struct task_struct *task = (struct task_struct *)p;
    PP();
    seq_printf(m, "%-24s%-5d\t%lld\n", task->comm, task->pid, offset);
    //    seq_printf(m, "======>%-24s%-5d\n", task->comm, task->pid);
    return 0;
}

static struct seq_operations exam_seq_op = {
    .start = l_start,
    .next = l_next,
    .stop = l_stop,
    .show = l_show};

static int exam_seq_open(struct inode *inode, struct file *file)
{
    PP();
    return seq_open(file, &exam_seq_op);
}

static const struct proc_ops exam_seq_fops = {
    .proc_open = exam_seq_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
};

static int __init exam_seq_init(void)
{
    static struct proc_dir_entry *entry;
    struct proc_dir_entry *my_dir = proc_mkdir("my_module", NULL);
    PP();
    entry = proc_create_data("exam_esq_file", 0444, NULL, &exam_seq_fops, NULL);
    if (!entry)
        printk(KERN_EMERG "proc_create error.\n");

    printk(KERN_EMERG "exam_seq_init.\n");
    return 0;
}

static void __exit exam_seq_exit(void)
{
    PP();
    remove_proc_entry("exam_esq_file", proc_mkdir("my_module", NULL));
    remove_proc_entry("exam_esq_file", NULL);
    printk(KERN_EMERG "exam_seq_exit.\n");
}

module_init(exam_seq_init);
module_exit(exam_seq_exit);
MODULE_LICENSE("GPL");