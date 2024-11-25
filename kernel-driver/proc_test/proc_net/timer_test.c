/*
 * @*************************************:
 * @FilePath     : \kernel-driver\proc_test\proc_net\timer_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-16 19:24:26
 * @LastEditors  : dof
 * @LastEditTime : 2024-11-25 11:52:34
 * @Descripttion :  使用  register_pernet_subsys  注册网络子系统
 * @compile      :
 * @**************************************:
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <net/net_namespace.h>


#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/capability.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/mm.h>
#include <linux/inet.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/fddidevice.h>
#include <linux/if_arp.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/rculist.h>

#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#endif

#include <net/net_namespace.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/ax25.h>
#include <net/netrom.h>
#include <net/dst_metadata.h>
#include <net/ip_tunnels.h>

#include <net/neighbour.h>

#include <linux/uaccess.h>

#include <linux/netfilter_arp.h>

#include <linux/netdevice.h>

#include <linux/timer.h>
#include <linux/jiffies.h>

#undef PP
#define PP(fmt, args...) printk("[%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)
// #define PP(fmt, args...)

static struct proc_dir_entry *entry;

struct timer_list my_timer;

struct net *g_net;

static void *l_start(struct seq_file *m, loff_t *pos)
{
    static unsigned long counter = 0;
    /* beginning a new sequence ? */
    if (*pos == 0)
    {
        /* yes => return a non null value to begin the sequence */
        return &counter;
    }
    seq_printf(m, "ifname");

    return NULL;
}

static void *l_next(struct seq_file *m, void *p, loff_t *pos)
{
    unsigned long *tmp_v = (unsigned long *)p;
    (*tmp_v)++;
    (*pos)++;
    return NULL;
}

static void l_stop(struct seq_file *m, void *p)
{
}

static int l_show(struct seq_file *m, void *p)
{
    struct net_device *dev;
    for_each_netdev(g_net, dev)
    {
        seq_printf(m, "%-15s %-3d\n", dev->name, dev->ifindex);
    }
    return 0;
}

static struct seq_operations exam_seq_op = {
    .start = l_start,
    .next = l_next,
    .stop = l_stop,
    .show = l_show};

static int exam_seq_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &exam_seq_op);
}

static const struct proc_ops exam_seq_fops = {
    .proc_open = exam_seq_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
};

static int my_protocol_init(struct net *net)
{
    g_net = net;
    entry = proc_create_data("exam_esq_file", 0444, NULL, &exam_seq_fops, NULL);
    if (!entry)
        PP("proc_create error.\n");

    return 0;
}

void my_timer_function(struct timer_list *data)
{
    // 定时器回调函数，在这里执行你的任务
    PP("Timer expired!\n");
    my_timer.expires = jiffies + HZ; // 必须重新填充时间，否则会陷入无限循环
    add_timer(&my_timer);
}

void my_init_timer(void)
{
    timer_setup(&my_timer, my_timer_function, 0);
    my_timer.expires = jiffies + HZ; // 设置超时时间为1秒
    add_timer(&my_timer);
}

static void my_protocol_exit(struct net *net)
{
    remove_proc_entry("exam_esq_file", NULL);
    del_timer_sync(&my_timer);
    printk(KERN_INFO "My protocol exited\n");
}

static struct pernet_operations my_protocol_ops = {
    .init = my_protocol_init,
    .exit = my_protocol_exit,
};

static int __init my_module_init(void)
{
    my_init_timer();
    register_pernet_subsys(&my_protocol_ops);
    return 0;
}

static void __exit my_module_exit(void)
{
    unregister_pernet_subsys(&my_protocol_ops);
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");