/*
 * @*************************************: 
 * @FilePath     : \netSubSystem\l2_netfilter.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-12-02 14:54:40
 * @LastEditors  : dof
 * @LastEditTime : 2024-12-02 15:16:00
 * @Descripttion :  	自定义防火墙规则
 *                      实现网络地址转换（NAT）
 *                      进行网络流量监控
 *                      实现入侵检测系统
 * @compile      :  
 * @**************************************: 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <net/protocol.h>
#include <linux/netfilter_ipv4.h>
#include <linux/uaccess.h>
#include <linux/kmod.h>

#undef PP
#define PP(fmt, args...) printk("[%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)

#define MY_L3_PROTOCOL 155 // 自定义协议号

// 数据接收处理函数
static int my_l3_protocol_rcv(struct sk_buff *skb)
{
    struct iphdr *iph;
    unsigned char *payload;
    unsigned int payload_len;
    char buffer[512] = {0};

    // 验证数据包有效性
    if (!skb)
    {
        PP("Received NULL skb\n");
        return NET_RX_DROP;
    }

    // 提取 IP 头部
    iph = ip_hdr(skb);
    if (!iph)
    {
        PP("Failed to extract IP header\n");
        return NET_RX_DROP;
    }

    // 检查是否为自定义协议
    if (iph->protocol != MY_L3_PROTOCOL)
    {
        return NET_RX_SUCCESS; // 不是自定义协议，正常返回
    }

    // 提取负载数据
    payload = (unsigned char *)((unsigned char *)iph + (iph->ihl * 4));
    payload_len = ntohs(iph->tot_len) - (iph->ihl * 4);

    if (payload_len >= sizeof(buffer))
    {
        PP("Payload too large to handle: %u bytes\n", payload_len);
        return NET_RX_DROP;
    }

    memcpy(buffer, payload, payload_len);
    buffer[payload_len] = '\0'; // 确保数据以 NULL 结尾

    PP("Custom L3 Protocol: Received data: %d|%s\n", payload_len, buffer);
    if (payload_len)
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
        PP("pos = %lld\n", pos);

        // 写入数据到文件
        if (kernel_write(file, buffer, payload_len, &pos) < 0)
        {
            PP("Failed to write to file\n");
        }
        PP();

        // 关闭文件
        filp_close(file, NULL);
        PP();
    }

    // char *argv[] = {"/bin/touch", "/tmp/123", NULL};
    // char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};
    // 调用用户空间程序
    // int ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    // if (ret != 0)
    // {
    //     PP("Failed to execute user space program: %d\n", ret);
    // }
    // else
    // {
    //     PP("User space program executed successfully\n");
    // }

    return NET_RX_SUCCESS;
}

// 钩子函数
static struct nf_hook_ops nfho;

// 钩子处理函数
static unsigned int hook_func(void *priv, struct sk_buff *skb,
                              const struct nf_hook_state *state)
{
    return my_l3_protocol_rcv(skb);
}

// 模块加载函数
static int __init custom_l3_protocol_init(void)
{
    PP("Loading custom L3 protocol module\n");

    // 注册 Netfilter 钩子
    nfho.hook = hook_func;              // 设置钩子处理函数
    nfho.pf = NFPROTO_IPV4;             // 仅处理 IPv4 数据包
    nfho.hooknum = NF_INET_PRE_ROUTING; // 在数据包进入路由前拦截
    nfho.priority = NF_IP_PRI_FIRST;    // 优先级设置为最高
    nf_register_net_hook(&init_net, &nfho);

    PP("Custom L3 protocol module loaded\n");
    return 0;
}

// 模块卸载函数
static void __exit custom_l3_protocol_exit(void)
{
    PP("Unloading custom L3 protocol module\n");

    // 注销 Netfilter 钩子
    nf_unregister_net_hook(&init_net, &nfho);

    PP("Custom L3 protocol module unloaded\n");
}

module_init(custom_l3_protocol_init);
module_exit(custom_l3_protocol_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Linux Kernel Module for Custom Layer 3 Protocol");