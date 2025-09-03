#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h> // 桥接框架头文件
#include <linux/if_ether.h>         // 以太网帧头
#include <linux/if_packet.h>        // L2层协议

#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <linux/inet.h>
#include <linux/in.h>
#include <net/netfilter/nf_log.h>

#undef PP
#define PP(fmt, ...) printk("\033[0;32;32m[zzzzz :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)

// char *ip_src = "192.168.1.100";
// char *ip_dst = "192.168.1.1";

// __be32 ip_addr_src, ip_addr_dst;
// in4_pton(ip_src, -1, (u8 *)&ip_addr_src, '\0', NULL);
// in4_pton(ip_dst, -1, (u8 *)&ip_addr_dst, '\0', NULL);

/**
 * IP_STR_TO_BE32 - 将点分十进制IP字符串转换为 __be32 整数
 * @_ip_str: 要转换的IP字符串 (const char *)
 *
 * 这是一个语句表达式宏，它会调用 in4_pton。
 * 成功时，返回网络字节序的 __be32 整数。
 * 失败时，返回 0 (INADDR_ANY) 或者 INADDR_NONE (通常是 -1, 更明确的错误值)。
 * 这里我们选择返回 0，因为 0.0.0.0 (INADDR_ANY) 本身就是一个有效的地址，
 * 但在很多场景下可以作为 "未指定" 或失败的标志。
 */
static inline __be32 ip_str_to_be32(const char *_ip_str)
{
    __be32 __addr = 0;
    if (in4_pton(_ip_str, -1, (u8 *)&__addr, '\0', NULL) == 0)
    {
        /* 如果转换失败, 确保 __addr 为 0 */
        __addr = 0;
    }
    return __addr;
}

static inline const char *l2_hook_name(unsigned int h)
{
    switch (h)
    {
    case NF_BR_PRE_ROUTING:
        return "BR_PRE_ROUTING";
    case NF_BR_LOCAL_IN:
        return "BR_LOCAL_IN";
    case NF_BR_FORWARD:
        return "BR_FORWARD";
    case NF_BR_LOCAL_OUT:
        return "BR_LOCAL_OUT";
    case NF_BR_POST_ROUTING:
        return "BR_POST_ROUTING";
    case NF_BR_BROUTING:
        return "BR_BROUTING";
    case NF_BR_NUMHOOKS:
        return "BR_NUMHOOKS";
    default:
        return "";
    }
}

static inline const char *l3_hook_name(unsigned int h)
{
    switch (h)
    {
    case NF_INET_PRE_ROUTING:
        return "IP_PRE_ROUTING";
    case NF_INET_LOCAL_IN:
        return "IP_LOCAL_IN";
    case NF_INET_FORWARD:
        return "IP_FORWARD";
    case NF_INET_LOCAL_OUT:
        return "IP_LOCAL_OUT";
    case NF_INET_POST_ROUTING:
        return "IP_POST_ROUTING";
    default:
        return "?";
    }
}
// Bitmask to indicate which fields in the struct are active for a given rule
#define UNIFIED_MATCH_ETH_SRC (1 << 0)
#define UNIFIED_MATCH_ETH_DST (1 << 1)
#define UNIFIED_MATCH_ETH_PROTO (1 << 2)
#define UNIFIED_MATCH_IP_SRC (1 << 3)
#define UNIFIED_MATCH_IP_DST (1 << 4)
#define UNIFIED_MATCH_IP_ADDR (1 << 5)
#define UNIFIED_MATCH_IP_PROTO (1 << 6)
#define UNIFIED_MATCH_TCP_SPORT (1 << 7)
#define UNIFIED_MATCH_TCP_DPORT (1 << 8)
#define UNIFIED_MATCH_UDP_SPORT (1 << 9)
#define UNIFIED_MATCH_UDP_DPORT (1 << 10)
#define UNIFIED_MATCH_ICMP_TYPE (1 << 11)
#define UNIFIED_MATCH_ICMP_CODE (1 << 12)

struct unified_rule_info
{
    // A bitmask indicating which fields below are valid for this rule.
    __u32 valid_fields;

    // Layer 2 Fields
    unsigned char src_mac[ETH_ALEN];
    unsigned char dst_mac[ETH_ALEN];
    __be16 eth_proto;

    // Layer 3 Fields
    char src_ip[16];
    char dst_ip[16];
    char addr_ip[16];
    __u8 ip_proto;

    // Layer 4 Fields (using a union to save space)
    union
    {
        struct
        {
            __be16 sport;
            __be16 dport;
        } tcp;
        struct
        {
            __be16 sport;
            __be16 dport;
        } udp;
        struct
        {
            __u8 type;
            __u8 code;
        } icmp;
    } l4;
};

struct unified_rule_info g_info = {
    .valid_fields = UNIFIED_MATCH_IP_PROTO | UNIFIED_MATCH_IP_ADDR,
    .addr_ip = "172.16.27.28",
    // .dst_ip = "192.168.1.100",
    .ip_proto = IPPROTO_ICMP};

static bool unified_match_func(const struct sk_buff *skb, struct unified_rule_info *info)
{
    bool ret = true;
    if (NULL == info)
    {
        return false;
    }

    struct ethhdr *eth_header;
    struct iphdr *ip_header;
    void *l4_header_ptr;

    // --- Layer 2 (Ethernet) Matching ---
    if (info->valid_fields & (UNIFIED_MATCH_ETH_SRC | UNIFIED_MATCH_ETH_DST | UNIFIED_MATCH_ETH_PROTO))
    {
        // skb_mac_header(skb) gets the MAC header
        if (!skb_mac_header_was_set(skb))
        {
            PP();
            ret = false; // Not an ethernet frame or header not accessible
        }
        eth_header = eth_hdr(skb);

        if ((info->valid_fields & UNIFIED_MATCH_ETH_SRC) &&
            !ether_addr_equal(eth_header->h_source, info->src_mac))
        {
            PP();
            ret = false; // Source MAC doesn't match
        }

        if ((info->valid_fields & UNIFIED_MATCH_ETH_DST) &&
            !ether_addr_equal(eth_header->h_dest, info->dst_mac))
        {
            PP();
            ret = false; // Destination MAC doesn't match
        }

        if ((info->valid_fields & UNIFIED_MATCH_ETH_PROTO) &&
            eth_header->h_proto != info->eth_proto)
        {
            PP();
            ret = false; // EtherType doesn't match
        }
    }

    // --- Layer 3 (IP) Matching ---
    if (info->valid_fields & (UNIFIED_MATCH_IP_ADDR | UNIFIED_MATCH_IP_SRC | UNIFIED_MATCH_IP_DST | UNIFIED_MATCH_IP_PROTO))
    {
        // skb->network_header points to the IP header
        ip_header = ip_hdr(skb);

        if ((info->valid_fields & UNIFIED_MATCH_IP_SRC) &&
            ip_header->saddr != ip_str_to_be32(info->src_ip))
        {
            ret = false; // Source IP doesn't match
        }

        // PP("ip_header->daddr = %pI4 dst_ip = %s", &ip_header->daddr, info->dst_ip);
        if ((info->valid_fields & UNIFIED_MATCH_IP_DST) &&
            ip_header->daddr != ip_str_to_be32(info->dst_ip))
        {
            // PP();
            ret = false; // Destination IP doesn't match
        }

        if ((info->valid_fields & UNIFIED_MATCH_IP_ADDR) &&
            (ip_header->daddr != ip_str_to_be32(info->addr_ip) && ip_header->saddr != ip_str_to_be32(info->addr_ip)))
        {
            // PP();
            ret = false; // Destination IP doesn't match
        }

        // This check is crucial for Layer 4 parsing
        if ((info->valid_fields & UNIFIED_MATCH_IP_PROTO) &&
            ip_header->protocol != info->ip_proto)
        {
            ret = false; // IP protocol doesn't match
        }
    }

    // --- Layer 4 (TCP/UDP/ICMP) Matching ---
    // Calculate the start of the L4 header
    ip_header = ip_hdr(skb); // Re-fetch in case it wasn't fetched before
    l4_header_ptr = (void *)skb->data + skb_transport_offset(skb);

    // Check packet length to ensure L4 header is complete
    if (skb->len < skb_transport_offset(skb) + sizeof(struct tcphdr) /* use largest header for initial check */)
    {
        // Not enough data for a L4 header, can't match L4 rules
        if (info->valid_fields & (UNIFIED_MATCH_TCP_DPORT | UNIFIED_MATCH_UDP_DPORT | UNIFIED_MATCH_ICMP_TYPE))
            ret = false;
    }

    switch (ip_header->protocol)
    {
    case IPPROTO_TCP:
        if (info->valid_fields & (UNIFIED_MATCH_TCP_SPORT | UNIFIED_MATCH_TCP_DPORT))
        {
            struct tcphdr *tcp_header = (struct tcphdr *)l4_header_ptr;
            if ((info->valid_fields & UNIFIED_MATCH_TCP_SPORT) &&
                tcp_header->source != info->l4.tcp.sport)
            {
                PP();
                ret = false;
            }
            if ((info->valid_fields & UNIFIED_MATCH_TCP_DPORT) &&
                tcp_header->dest != info->l4.tcp.dport)
            {
                ret = false;
            }
        }
        break;
    case IPPROTO_UDP:
        if (info->valid_fields & (UNIFIED_MATCH_UDP_SPORT | UNIFIED_MATCH_UDP_DPORT))
        {
            struct udphdr *udp_header = (struct udphdr *)l4_header_ptr;
            if ((info->valid_fields & UNIFIED_MATCH_UDP_SPORT) &&
                udp_header->source != info->l4.udp.sport)
            {
                PP();
                ret = false;
            }
            if ((info->valid_fields & UNIFIED_MATCH_UDP_DPORT) &&
                udp_header->dest != info->l4.udp.dport)
            {
                PP();
                ret = false;
            }
        }
        break;
    case IPPROTO_ICMP:
        if (info->valid_fields & (UNIFIED_MATCH_ICMP_TYPE | UNIFIED_MATCH_ICMP_CODE))
        {
            struct icmphdr *icmp_header = (struct icmphdr *)l4_header_ptr;
            if ((info->valid_fields & UNIFIED_MATCH_ICMP_TYPE) &&
                icmp_header->type != info->l4.icmp.type)
            {
                PP("icmp_header->type = 0x%04x | 0x%04x", icmp_header->type, info->l4.icmp.type);
                ret = false;
            }
            if ((info->valid_fields & UNIFIED_MATCH_ICMP_CODE) &&
                icmp_header->code != info->l4.icmp.code)
            {
                PP();
                ret = false;
            }
        }
        break;
    default:
        break;
    }

    // If we survived all checks, it's a match!
    return ret;
}

static unsigned int get_skb_protocol(struct sk_buff *skb)
{
    struct iphdr *iph;
    int detected = 0;

    // 确保数据包是完整的
    if (!skb || !skb_mac_header_was_set(skb))
    {
        return NF_ACCEPT;
    }

    // 获取 IP 头部
    iph = ip_hdr(skb);
    if (!iph)
    {
        return NF_ACCEPT;
    }

    switch (iph->protocol)
    {
        case IPPROTO_TCP:
        {
            // 处理 TCP 流量
            // 获取 TCP 头部
            struct tcphdr *tcph = (struct tcphdr *)(skb_network_header(skb) + ip_hdr(skb)->ihl * 4);
            if (tcph)
            {
                // 例如：丢弃所有 SSH (端口 22) 流量
                if (ntohs(tcph->dest) == 22)
                {
                    pr_info("Dropped SSH packet to %pI4:%d.\n", &iph->daddr, ntohs(tcph->dest));
                    return NF_DROP;
                }
            }
            break;
        }

        case IPPROTO_UDP:
        {
            // 处理 UDP 流量
            // 获取 UDP 头部
            struct udphdr *udph = (struct udphdr *)(skb_network_header(skb) + ip_hdr(skb)->ihl * 4);
            if (udph)
            {
                // 例如：丢弃所有 DNS (端口 53) 流量
                if (ntohs(udph->dest) == 53)
                {
                    pr_info("Dropped DNS packet to %pI4:%d.\n", &iph->daddr, ntohs(udph->dest));
                    return NF_DROP;
                }
            }
            break;
        }
        case IPPROTO_ICMP:
        {
            // 处理 ICMP 流量
            pr_info("ICMP packet from %pI4 to %pI4.\n", &iph->saddr, &iph->daddr);
            detected = 1;
            break;
        }
        default:
        {
            // 处理其他协议
            break;
        }
    }

    return 0;
}

static unsigned int l2_filter_hook_func(void *priv,
                                        struct sk_buff *skb,
                                        const struct nf_hook_state *state)
{
    // 获取以太网头部
    struct ethhdr *eth_hdr_ptr = eth_hdr(skb);
    int detected = 0;

    if (!eth_hdr_ptr)
    {
        return NF_ACCEPT; // 没有以太网头部，直接放行
    }
    struct iphdr *iph = ip_hdr(skb);
    if (!iph)
    {
        return NF_ACCEPT;
    }

    // switch (ntohs(eth_hdr_ptr->h_proto))
    // {
    // // 假设我们想要过滤 ARP 报文 (协议类型 ETH_P_ARP = 0x0806)
    // case ETH_P_ARP:
    // {
    //     /* code */
    //     break;
    // }

    // // 检查以太网协议类型
    // // case ETH_P_IP:
    // // 自定义协议栈
    // case 0x8888:
    // {
    //     // 假设我们想处理 IPV4 报文
    //     if (iph->saddr == in_aton("127.0.0.1") || iph->daddr == in_aton("127.0.0.1"))
    //     {
    //         return NF_ACCEPT;
    //     }
    //     detected = 1;

    //     // 你可以在这里添加更复杂的逻辑
    //     // 例如，检查源/目的IP，或传输层协议
    //     break;
    //     default:
    //     break;
    // }

    detected = unified_match_func(skb, &g_info);
    if (detected)
    {
        PP("BR Packet - %pM --> %pM  \nsrc: %pI4, --> dst: %pI4", eth_hdr_ptr->h_source, eth_hdr_ptr->h_dest, &iph->saddr, &iph->daddr);
        PP("\tIP Packet - Protocol: %d, TTL: %d", iph->protocol, iph->ttl);
        PP("\tpf=%u hook=%u(%s) in=%s out=%s\n\n",
           state->pf, state->hook, l2_hook_name(state->hook),
           state->in ? state->in->name : "-", state->out ? state->out->name : "-");
    }

    // 对于其他所有报文，默认放行
    return NF_ACCEPT;
}

static unsigned int l3_filter_hook(void *priv,
                                   struct sk_buff *skb,
                                   const struct nf_hook_state *state)
{
    struct iphdr *iph;
    int detected = 0;
    int ret = 0;

    // 确保数据包是完整的
    if (!skb || !skb_mac_header_was_set(skb))
    {
        return NF_ACCEPT;
    }

    // 获取 IP 头部
    iph = ip_hdr(skb);
    if (!iph)
    {
        return NF_ACCEPT;
    }

    if (iph->saddr == in_aton("127.0.0.1") || iph->daddr == in_aton("127.0.0.1"))
    {
        return NF_ACCEPT;
    }

    // ret = get_skb_protocol(skb);
    detected = unified_match_func(skb, &g_info);
    if (detected)
    {
        PP("IP Packet Details:");
        PP("\tsrc IP: %pI4 --> dst IP: %pI4", &iph->saddr, &iph->daddr);
        PP("\tProtocol: %d ", iph->protocol);
        PP("\tTTL: %d", iph->ttl);
        PP("\tTotal Length: %d", ntohs(iph->tot_len));
        PP("\tpf=%u hook=%u(%s) in=%s out=%s\n\n",
           state->pf, state->hook, l3_hook_name(state->hook),
           state->in ? state->in->name : "-", state->out ? state->out->name : "-");
    }

    // 默认放行所有数据包
    return NF_ACCEPT;
}

static struct nf_hook_ops l2_filter_ops[] = {
    {
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_PRE_ROUTING, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE,         // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST,  // 优先级最高，尽早处理
    },
    {
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_LOCAL_IN,   // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE,        // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
    {
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_FORWARD,    // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE,        // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
    {
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_LOCAL_OUT,  // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE,        // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
    {
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_POST_ROUTING, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE,          // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST,   // 优先级最高，尽早处理
    }};

// Netfilter 钩子操作结构
static struct nf_hook_ops l3_filter_ops[] = {
    {
        .hook = l3_filter_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_PRE_ROUTING,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = l3_filter_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = l3_filter_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = l3_filter_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_LOCAL_OUT,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = l3_filter_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_POST_ROUTING,
        .priority = NF_IP_PRI_FIRST,
    },
};

// 模块初始化函数
static int __init l2_filter_init(void)
{
    int ret;
    PP("Netfilter L2 Filter Module: Loading...\n");
    ret = nf_register_net_hooks(&init_net, l2_filter_ops, ARRAY_SIZE(l2_filter_ops));
    ret |= nf_register_net_hooks(&init_net, l3_filter_ops, ARRAY_SIZE(l3_filter_ops));

    for (int i = 0; i < ARRAY_SIZE(l2_filter_ops); i++)
    {
        if (!l2_filter_ops[i].hook)
        {
            PP("Error: Hook %d has no hook function", i);
            return -EINVAL;
        }
        PP("Hook %d: pf=%d, hooknum=%d, priority=%d",
           i, l2_filter_ops[i].pf, l2_filter_ops[i].hooknum, l2_filter_ops[i].priority);
    }

    if (ret < 0)
    {
        pr_err("Failed to register netfilter hooks\n");
        return ret;
    }
    PP("Netfilter L2 Filter Module: Loaded successfully.\n");
    return 0;
}

// 模块退出函数
static void __exit l2_filter_exit(void)
{
    PP("Netfilter L2 Filter Module: Unloading...\n");
    nf_unregister_net_hooks(&init_net, l2_filter_ops, ARRAY_SIZE(l2_filter_ops));
    nf_unregister_net_hooks(&init_net, l3_filter_ops, ARRAY_SIZE(l3_filter_ops));
    PP("Netfilter L2 Filter Module: Unloaded.\n");
}

module_init(l2_filter_init);
module_exit(l2_filter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dof");
MODULE_DESCRIPTION("A simple Netfilter L2 packet filter for Linux 5.10.");