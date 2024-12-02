#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/uaccess.h>
#include <linux/kmod.h>
#include <linux/etherdevice.h>

#define MY_PROTOCOL 0x88B5 // 自定义协议号（这里用的是以太网协议号的保留范围）

#undef PP
#define PP(fmt, args...) printk("[%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)

static struct packet_type my_packet_type; // 协议类型描述符

// 数据接收处理函数
static int my_protocol_rcv(struct sk_buff *skb, struct net_device *dev,
                           struct packet_type *pt, struct net_device *orig_dev)
{
    char *data;
    int data_len;
    char buffer[128]; // 用于存储解析后的数据
    char *argv[] = {"/bin/touch", "/tmp/sssss", NULL};
    char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL};

    if (!skb)
    {
        PP("Received NULL skb\n");
        return NET_RX_DROP;
    }

    // 提取数据长度
    data_len = skb->len;

    // 检查数据长度是否超出缓冲区
    if (data_len >= sizeof(buffer))
    {
        PP("Packet too large to handle (%d bytes), dropping\n", data_len);
        return NET_RX_DROP;
    }

    // 提取数据内容
    data = skb->data;
    memcpy(buffer, data, data_len);
    buffer[data_len] = '\0'; // 确保以 NULL 结尾

    PP("Custom Protocol: Received data (%d bytes): %s\n", data_len, buffer);

    // 调用用户空间程序处理数据
    int ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret != 0)
    {
        PP("Failed to execute user space program: %d\n", ret);
    }
    else
    {
        PP("User space program executed successfully\n");
    }

    return NET_RX_SUCCESS;
}

// 模块加载函数
static int __init custom_protocol_init(void)
{
    PP("Loading custom protocol module\n");

    // 初始化协议处理器
    my_packet_type.type = htons(MY_PROTOCOL); // 监听指定协议号
    my_packet_type.func = my_protocol_rcv;    // 设置接收处理回调
    my_packet_type.dev = NULL;                // NULL 表示监听所有设备

    // 注册协议
    dev_add_pack(&my_packet_type);
    PP("Custom protocol handler registered\n");

    return 0;
}

// 模块卸载函数
static void __exit custom_protocol_exit(void)
{
    PP("Unloading custom protocol module\n");

    // 注销协议
    dev_remove_pack(&my_packet_type);
    PP("Custom protocol handler unregistered\n");
}

module_init(custom_protocol_init);
module_exit(custom_protocol_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Linux Kernel Module for Custom Protocol Handling");

