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

// 虚拟网络设备私有数据结构
struct mynet_priv
{
    struct net_device *netdev;
};

// 设备的 ndo_start_xmit 回调函数
static netdev_tx_t mynet_xmit(struct sk_buff *skb, struct net_device *netdev)
{
    if (!skb || !netdev)
    {
        return NETDEV_TX_OK;
    }
    netdev->stats.tx_packets++;
    netdev->stats.tx_bytes += skb->len;
    pr_info("mynet_echo: Transmitting packet of size %d from %s.\n", skb->len, netdev->name);
    dev_kfree_skb(skb);
    return NETDEV_TX_OK;
}

// 设备的接收函数
static int mynet_receive(struct sk_buff *skb, struct net_device *netdev)
{
    struct ethhdr *eth_hdr;
    __be32 temp_ip;
    __be16 temp_proto;

    if (!skb)
    {
        return -EINVAL;
    }
    netdev->stats.rx_packets++;
    netdev->stats.rx_bytes += skb->len;

    pr_info("mynet_echo: Received packet of size %d on %s.\n", skb->len, netdev->name);

    // 确保数据包有以太网头部
    if (!pskb_may_pull(skb, sizeof(struct ethhdr)))
    {
        return -EINVAL;
    }
    skb_reset_mac_header(skb);
    eth_hdr = eth_hdr(skb);

    // 交换 MAC 地址
    memcpy(eth_hdr->h_dest, eth_hdr->h_source, ETH_ALEN);
    memcpy(eth_hdr->h_source, netdev->dev_addr, ETH_ALEN);

    temp_proto = eth_hdr->h_proto;

    // 处理 ARP 协议
    if (ntohs(temp_proto) == ETH_P_ARP)
    {
        pr_info("mynet_echo: ARP packet received, echoing it back.\n");
        // ARP 协议已经处理了回送逻辑，这里直接注入
    }
    // 处理 IP 协议
    else if (ntohs(temp_proto) == ETH_P_IP)
    {
        struct iphdr *iph;
        if (!pskb_may_pull(skb, skb_network_offset(skb) + sizeof(struct iphdr)))
        {
            return -EINVAL;
        }
        iph = ip_hdr(skb);

        // 交换源 IP 和目的 IP
        temp_ip = iph->saddr;
        iph->saddr = iph->daddr;
        iph->daddr = temp_ip;

        pr_info("mynet_echo: IPv4 packet received, echoing it back.\n");
    }
    else
    {
        pr_info("mynet_echo: Unhandled protocol, dropping packet.\n");
        kfree_skb(skb);
        return -EPROTONOSUPPORT;
    }

    // 将数据包重新注入网络栈
    skb->dev = netdev;
    skb->protocol = temp_proto;
    skb->ip_summed = CHECKSUM_NONE;
    skb_set_network_header(skb, skb->dev->hard_header_len);
    netif_rx(skb);

    return 0;
}

// 设备 ndo_open 回调函数
static int mynet_open(struct net_device *netdev)
{
    netif_start_queue(netdev);
    pr_info("mynet_echo: Device '%s' opened.\n", netdev->name);
    return 0;
}

// 设备 ndo_stop 回调函数
static int mynet_stop(struct net_device *netdev)
{
    netif_stop_queue(netdev);
    pr_info("mynet_echo: Device '%s' closed.\n", netdev->name);
    return 0;
}

// Netdev 操作集
static const struct net_device_ops mynet_netdev_ops = {
    .ndo_open = mynet_open,
    .ndo_stop = mynet_stop,
    .ndo_start_xmit = mynet_xmit,
};

// 设备初始化函数
static void mynet_setup(struct net_device *netdev)
{
    ether_setup(netdev);
    netdev->netdev_ops = &mynet_netdev_ops;
    eth_hw_addr_random(netdev);
    netdev->mtu = 1500;
}

// RTNetlink 回调函数，处理来自用户态的创建请求
static int mynet_rtnl_doit(struct sk_buff *skb, struct nlmsghdr *nlh, struct netlink_ext_ack *extack)
{
    struct net_device *dev;
    char name[IFNAMSIZ];

    // 获取命名空间
    struct net *net = rtnl_link_get_net(nlh);
    if (!net)
    {
        return -EINVAL;
    }

    // 从 nlmsg 获取设备名称
    snprintf(name, IFNAMSIZ, "mynet_echo");

    // 分配网络设备
    dev = alloc_netdev(sizeof(struct mynet_priv), name, NET_NAME_ENUM, mynet_setup);
    if (!dev)
    {
        return -ENOMEM;
    }

    // 设置设备所属的命名空间
    dev_net_set(dev, net);

    // 注册设备
    if (register_netdev(dev) < 0)
    {
        pr_err("mynet_echo: Failed to register device in namespace.\n");
        free_netdev(dev);
        return -EFAULT;
    }
    pr_info("mynet_echo: Registered new device '%s' in namespace '%s'.\n", dev->name, net->user_ns->name);
    return 0;
}

// RTNetlink 链接操作集，用于与 ip link 命令交互
static struct rtnl_link_ops mynet_link_ops = {
    .kind = "mynet_echo",
    .maxtype = IFLA_MAX,
    .ndo_doit = mynet_rtnl_doit,
};

// 模块初始化函数
static int __init mynet_echo_init(void)
{
    pr_info("mynet_echo: Module loaded, registering RTNetlink handler.\n");
    return rtnl_link_register(&mynet_link_ops);
}

// 模块卸载函数
static void __exit mynet_echo_exit(void)
{
    rtnl_link_unregister(&mynet_link_ops);
    pr_info("mynet_echo: Module unloaded, RTNetlink handler unregistered.\n");
}

module_init(mynet_echo_init);
module_exit(mynet_echo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple network subsystem with ICMP-like functionality and namespace support.");