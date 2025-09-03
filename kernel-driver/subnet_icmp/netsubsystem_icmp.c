#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/rtnetlink.h>
#include <linux/net_namespace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <net/rtnetlink.h>
#include <net/netns/packet.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>

// 为内核日志添加彩色和函数信息，便于调试
#undef PP
#define PP(fmt, ...) printk("\033[0;32m[zzzzz :%s(%d)] " fmt "\033[0m\r\n", __func__, __LINE__, ##__VA_ARGS__)

// 虚拟网络设备私有数据结构，如果需要可以添加其他成员
struct mynet_priv
{
    struct net_device *netdev;
};


// 函数声明
static int mynet_receive(struct sk_buff *skb, struct net_device *netdev, struct packet_type *ptype, struct net_device *orig_dev);

// 协议处理器结构
static struct packet_type mynet_packet_type = {
    .type = htons(0x8888), // 自定义协议类型
    .dev = NULL,
    .func = mynet_receive,
    .af_packet_priv = NULL,
};

// 添加发送响应数据包的函数
static int send_response_packet(struct sk_buff *original_skb, struct net_device *netdev,
                                __be16 msg_type, __be16 seq_num)
{
    struct sk_buff *response_skb;
    struct ethhdr *eth_hdr;
    unsigned char *custom_hdr;

    PP("Preparing to send response packet...");

    // 打印发送接口信息
    if (netdev)
    {
        PP("Sending response via interface: %s", netdev->name);
        PP("Interface MAC: %pM", netdev->dev_addr);
    }
    else
    {
        PP("Sending response via unknown interface");
    }

    // 分配新的数据包缓冲区
    response_skb = alloc_skb(ETH_FRAME_LEN, GFP_ATOMIC);
    if (!response_skb)
    {
        PP("Failed to allocate response skb");
        return -ENOMEM;
    }

    // 设置数据包长度
    skb_put(response_skb, sizeof(struct ethhdr) + 20); // 以太网头部 + 自定义协议头部

    // 设置网络头部偏移
    skb_set_network_header(response_skb, sizeof(struct ethhdr));

    // 构建以太网头部
    eth_hdr = (struct ethhdr *)response_skb->data;

    // 交换源和目的MAC地址（响应包）
    if (original_skb && skb_mac_header(original_skb))
    {
        struct ethhdr *orig_eth = (struct ethhdr *)skb_mac_header(original_skb);
        memcpy(eth_hdr->h_dest, orig_eth->h_source, ETH_ALEN); // 目的MAC = 原源MAC
        memcpy(eth_hdr->h_source, orig_eth->h_dest, ETH_ALEN); // 源MAC = 原目的MAC
    }
    else
    {
        // 如果没有原始数据包，使用默认MAC地址
        unsigned char default_dest[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
        unsigned char default_src[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
        memcpy(eth_hdr->h_dest, default_dest, ETH_ALEN);
        memcpy(eth_hdr->h_source, default_src, ETH_ALEN);
    }

    eth_hdr->h_proto = mynet_packet_type.type; // 自定义协议类型

    // 构建自定义协议头部
    custom_hdr = (unsigned char *)(response_skb->data + sizeof(struct ethhdr));

    // 交换源和目的IP地址（响应包）
    if (original_skb && skb_network_header(original_skb))
    {
        unsigned char *orig_custom_hdr = skb_network_header(original_skb);
        __be32 orig_src = *((__be32 *)(orig_custom_hdr + 0));
        __be32 orig_dst = *((__be32 *)(orig_custom_hdr + 4));

        // 响应包：源IP = 原目的IP，目的IP = 原源IP
        *((__be32 *)(custom_hdr + 0)) = orig_dst; // 源地址
        *((__be32 *)(custom_hdr + 4)) = orig_src; // 目的地址
    }
    else
    {
        // 默认IP地址
        *((__be32 *)(custom_hdr + 0)) = htonl(0xC0A80101); // 192.168.1.1
        *((__be32 *)(custom_hdr + 4)) = htonl(0xC0A80164); // 192.168.1.100
    }

    // 设置其他字段
    *((__be16 *)(custom_hdr + 8)) = htons(ntohs(seq_num) + 1000); // 序列号 + 1000
    *((__be16 *)(custom_hdr + 10)) = htons(0x0004);               // 响应消息类型
    *((__be32 *)(custom_hdr + 12)) = htonl(0);                    // 数据长度

    // 设置数据包属性
    response_skb->dev = netdev;
    response_skb->protocol =  mynet_packet_type.type;
    response_skb->ip_summed = CHECKSUM_NONE;

    PP("Response packet prepared:");
    PP("  Source MAC: %pM", eth_hdr->h_source);
    PP("  Dest MAC: %pM", eth_hdr->h_dest);
    PP("  Source IP: %pI4", (__be32 *)(custom_hdr + 0));
    PP("  Dest IP: %pI4", (__be32 *)(custom_hdr + 4));
    PP("  Seq: %d, Type: 0x%04x", ntohs(*((__be16 *)(custom_hdr + 8))),
       ntohs(*((__be16 *)(custom_hdr + 10))));

    // 发送响应数据包
    if (netdev)
    {
        // 通过网络设备发送
        int result = dev_queue_xmit(response_skb); // 修正：使用 dev_queue_xmit
        if (result == NETDEV_TX_OK)
        {
            PP("Response packet sent successfully via dev_queue_xmit");
        }
        else
        {
            PP("Failed to send response packet via dev_queue_xmit: %d", result);
            kfree_skb(response_skb);
            return result;
        }
    }
    else
    {
        // 直接注入网络栈
        netif_rx(response_skb);
        PP("Response packet injected into network stack");
    }

    return 0;
}

// 设备的接收函数，处理从网络栈注入的数据包
static int mynet_receive(struct sk_buff *skb, struct net_device *netdev, struct packet_type *ptype, struct net_device *orig_dev)
{
    struct ethhdr *eth_hdr_ptr;
    __be32 temp_ip;
    __be16 temp_proto;

    if (!skb)
    {
        return -EINVAL;
    }

    PP("=== Protocol handler triggered ===");
    PP("Received packet of size %d tytpe = 0x%04x", skb->len, ptype->type);

    // 打印接口信息
    if (skb->dev)
    {
        PP("Receiving interface: %s", skb->dev->name);
        PP("Interface MAC: %pM", skb->dev->dev_addr);
    }
    else if (netdev)
    {
        PP("Receiving interface: %s", netdev->name);
        PP("Interface MAC: %pM", netdev->dev_addr);
    }
    else if (orig_dev)
    {
        PP("Receiving interface: %s", orig_dev->name);
        PP("Interface MAC: %pM", orig_dev->dev_addr);
    }
    else
    {
        PP("Receiving interface: Unknown (no device info)");
    }

    // 注意：作为协议处理器使用时，netdev可能为NULL
    if (netdev)
    {
        PP("Device: %s, MAC: %pM", netdev->name, netdev->dev_addr);
        // 更新接收统计
        netdev->stats.rx_packets++;
        netdev->stats.rx_bytes += skb->len;
    }

    // 确保数据包有以太网头部
    if (!pskb_may_pull(skb, sizeof(struct ethhdr)))
    {
        PP("Packet too short for ethernet header");
        return -EINVAL;
    }

    skb_reset_mac_header(skb);
    eth_hdr_ptr = eth_hdr(skb);

    temp_proto = eth_hdr_ptr->h_proto;
    print_hex_dump(KERN_ERR, "PKT: ", DUMP_PREFIX_OFFSET, 16, 1, eth_hdr_ptr, sizeof(struct ethhdr), true);
    // 处理自定义协议 (0x8888)
    if (ptype->type == 0x8888)
    {
        PP("Custom protocol packet received, protocol: 0x%04x", ntohs(temp_proto));

        // 检查数据包长度是否足够
        if (!pskb_may_pull(skb, skb_network_offset(skb) + 20)) // 自定义头部至少20字节
        {
            PP("Packet too short for custom protocol header");
            return -EINVAL;
        }

        // 获取自定义协议头部
        unsigned char *custom_hdr = skb_network_header(skb);
        print_hex_dump(KERN_ERR, "CUSTOM_PKT: ", DUMP_PREFIX_OFFSET, 16, 1, custom_hdr, 32, true);

        // 解析自定义协议字段（示例）
        __be32 src_addr = *((__be32 *)(custom_hdr + 0));  // 源地址偏移0
        __be32 dst_addr = *((__be32 *)(custom_hdr + 4));  // 目的地址偏移4
        __be16 seq_num = *((__be16 *)(custom_hdr + 8));   // 序列号偏移8
        __be16 msg_type = *((__be16 *)(custom_hdr + 10)); // 消息类型偏移10
        __be32 data_len = *((__be32 *)(custom_hdr + 12)); // 数据长度偏移12

        PP("Custom protocol - Source: %pI4, Destination: %pI4", &src_addr, &dst_addr);
        PP("Custom protocol - Seq: %d, Type: 0x%04x, DataLen: %d", ntohs(seq_num), ntohs(msg_type), ntohl(data_len));

        // 处理自定义协议逻辑
        if (ntohs(msg_type) == 0x0001) // 心跳包
        {
            PP("Processing heartbeat packet");
        }
        else if (ntohs(msg_type) == 0x0002) // 数据包
        {
            PP("Processing data packet");
        }
        else if (ntohs(msg_type) == 0x0003) // 控制包
        {
            PP("Processing control packet");
        }
        else
        {
            PP("Unknown message type: 0x%04x", ntohs(msg_type));
        }

        PP("Custom protocol packet processed successfully");
        PP();

        // 发送响应数据包
        if (ntohs(msg_type) != 0x0004)
        { // 避免响应包触发新的响应
            PP("Sending response packet...");
            int response_result = send_response_packet(skb, netdev, msg_type, seq_num);
            if (response_result == 0)
            {
                PP("Response packet sent successfully");
            }
            else
            {
                PP("Failed to send response packet: %d", response_result);
            }
        }
    }
    else
    {
        // PP("Unhandled protocol, dropping packet");
        kfree_skb(skb);
        return -EPROTONOSUPPORT;
    }
    // 对于协议处理器接收的数据包，我们通常不需要重新注入网络栈
    // 直接释放数据包
    kfree_skb(skb);
    return 0;
}

// 模块初始化函数
static int __init mynet_echo_init(void)
{
    PP("Module loaded, looking for target interface...");

    // 注册协议处理器
    dev_add_pack(&mynet_packet_type);

    PP("Protocol handler registered for type 0x8888");

    return 0;
}

// 模块卸载函数
static void __exit mynet_echo_exit(void)
{
    // 注销协议处理器
    dev_remove_pack(&mynet_packet_type);
    PP("Protocol handler unregistered for type 0x8888");

    PP("Module unloaded, original interface restored.");
}

module_init(mynet_echo_init);
module_exit(mynet_echo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Custom protocol handler using dev_add_pack for protocol type 0x8888");