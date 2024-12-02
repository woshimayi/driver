/*
 * @*************************************: 
 * @FilePath     : \netSubSystem\l3_sysSubNet.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-12-02 14:48:06
 * @LastEditors  : dof
 * @LastEditTime : 2024-12-02 15:12:07
 * @Descripttion :  自定义网络子系统， 过滤自定义协议，保存文件
 * @compile      :  
 * @**************************************: 
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4.h>
#include <linux/slab.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/inet_sock.h>
#include <net/inet_common.h>

#define NETWORK_SUBSYS_ID 155 // 唯一子系统 ID

#undef PP
#define PP(fmt, args...) printk("[%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)

int subnetsys = NETWORK_SUBSYS_ID;

struct network_subsys_data
{
    struct nf_hook_ops nfho; // Netfilter 钩子操作
};

// 数据包处理函数
static unsigned int packet_handler(void *priv, struct sk_buff *skb,
                                   const struct nf_hook_state *state)
{

    struct iphdr *iph;
    unsigned char *payload;
    unsigned int payload_len;
    char buffer[512] = {0};

    // 验证数据包有效性
    if (!skb)
    {
        // PP("Received NULL skb\n");
        return NET_RX_DROP;
    }

    // 提取 IP 头部
    iph = ip_hdr(skb);
    if (!iph)
    {
        PP("Failed to extract IP header\n");
        return NF_ACCEPT;
    }

    // 检查是否为自定义协议
    if (iph->protocol != NETWORK_SUBSYS_ID)
    {
        return NF_ACCEPT; // 不是自定义协议，继续传递数据包
    }
    PP("Network Subsystem: Packet from %pI4 to %pI4\n", &iph->saddr, &iph->daddr);

    // 提取负载数据
    payload = (unsigned char *)((unsigned char *)iph + (iph->ihl * 4));
    payload_len = ntohs(iph->tot_len) - (iph->ihl * 4);

    if (payload_len >= sizeof(buffer))
    {
        // PP("Payload too large to handle: %u bytes\n", payload_len);
        return NET_RX_DROP;
    }

    memcpy(buffer, payload, payload_len);
    buffer[payload_len] = '\0'; // 确保数据以 NULL 结尾

    PP("Custom L3 Protocol: Received data: %d|%s\n", payload_len, buffer);

    if (payload_len) // 如果有数据保存到文件
    {
        PP();
        // 打开文件
        struct file *file;
        loff_t pos = 0;

        // 使用kernel_write替代vfs_write
        file = filp_open("/tmp/custom_protocol_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (IS_ERR(file))
        {
            PP("Failed to open file\n");
            return PTR_ERR(file);
        }

        // 获取文件当前大小，作为写入的起始位置
        pos = vfs_llseek(file, 0, SEEK_END);
        if (pos < 0)
        {
            PP("Failed to seek to end of file\n");
            return PTR_ERR(file);
        }

        // 写入数据到文件
        if (kernel_write(file, buffer, payload_len, &pos) < 0)
        {
            PP("Failed to write to file\n");
        }

        // 关闭文件
        filp_close(file, NULL);
    }

    return NF_ACCEPT; // 继续传递数据包
}

static struct nf_hook_ops nfho;

// 初始化网络子系统
static int network_subsys_init_net(struct net *net)
{
    struct network_subsys_data *data;

    PP("Initializing network subsystem for netns\n");

    data = kzalloc(sizeof(struct network_subsys_data), GFP_KERNEL);
    if (!data)
    {
        PP("Failed to allocate network subsystem data\n");
        return -ENOMEM;
    }

    // 配置 Netfilter 钩子
    nfho.hook = packet_handler;
    nfho.pf = NFPROTO_IPV4;             // IPv4 协议
    nfho.hooknum = NF_INET_PRE_ROUTING; // 路由前拦截
    nfho.priority = NF_IP_PRI_FIRST;    // 优先级最高

    // 注册 Netfilter 钩子
    if (nf_register_net_hook(net, &nfho))
    {
        PP("Failed to register Netfilter hook\n");
        // kfree(data);
        return -EINVAL;
    }
    PP();

    // 保存数据到网络命名空间
    net_generic(net, NETWORK_SUBSYS_ID);

    return 0;
}

// 退出网络子系统, 卸载会出现kernel段错误
static void network_subsys_exit_net(struct net *net)
{
    PP();
    struct network_subsys_data *data = net_generic(net, NETWORK_SUBSYS_ID);

    if (!data)
    {
        PP();
        return;
    }

    // 注销 Netfilter 钩子
    nf_unregister_net_hook(net, &nfho);

    PP("Exiting network subsystem for netns\n");

    PP();
    kfree(data);
}

// pernet 操作结构
static struct pernet_operations __net_initdata net_ops = {
    .init = network_subsys_init_net,
    .exit = network_subsys_exit_net,
    .id = &subnetsys,
    .size = sizeof(struct network_subsys_data *),
};

static DEFINE_PER_CPU(struct sock *, ipv4_dof_sk);

// 模块初始化
static int __init network_subsys_init(void)
{
    int ret;

    PP("Loading network subsystem module\n");

    // ret = register_pernet_subsys(&net_ops);
    // if (ret)
    // {
    //     PP("Failed to register pernet subsystem: %d\n", ret);
    //     return ret;
    // }

    PP("Network subsystem module loaded\n");

    int err, i;

    for_each_possible_cpu(i)
    {
        struct sock *sk;

        err = inet_ctl_sock_create(&sk, PF_INET, SOCK_RAW, NETWORK_SUBSYS_ID, &init_net);
        if (err < 0)
            return err;

        per_cpu(ipv4_dof_sk, i) = sk;

        /* Enough space for 2 64K DOF_ packets, including
         * sk_buff/skb_shared_info struct overhead.
         */
        sk->sk_sndbuf = 2 * SKB_TRUESIZE(64 * 1024);
        /*
         * Speedup sock_wfree()
         */
        sock_set_flag(sk, SOCK_USE_WRITE_QUEUE);
        inet_sk(sk)->pmtudisc = IP_PMTUDISC_DONT;
    }
    return register_pernet_subsys(&net_ops);

    return 0;
}

// 模块卸载
static void __exit network_subsys_exit(void)
{
    PP("Unloading network subsystem module\n");

    unregister_pernet_subsys(&net_ops);

    PP("Network subsystem module unloaded\n");
}

module_init(network_subsys_init);
module_exit(network_subsys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Standalone network subsystem with pernet support");
