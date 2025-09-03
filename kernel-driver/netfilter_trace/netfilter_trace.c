#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/route.h>
#include <linux/netdevice.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Chain Tracer");
MODULE_DESCRIPTION("Trace iptables and ebtables chain decisions");

// 全局变量
static unsigned int trace_packets = 1; // 默认为1表示启用

// 获取链名称的辅助函数
static const char *get_chain_name(const struct nf_hook_state *state)
{
    if (!state || !state->net || !state->net->ipv4.iptable_filter)
        return "UNKNOWN";

    // 这里需要根据实际的netfilter实现来获取链名称
    // 这是一个简化的示例，实际实现需要更复杂的逻辑
    switch (state->hook)
    {
    case NF_INET_PRE_ROUTING:
        return "PREROUTING";
    case NF_INET_LOCAL_IN:
        return "INPUT";
    case NF_INET_FORWARD:
        return "FORWARD";
    case NF_INET_LOCAL_OUT:
        return "OUTPUT";
    case NF_INET_POST_ROUTING:
        return "POSTROUTING";
    default:
        return "UNKNOWN_CHAIN";
    }
}

// 获取表名称
static const char *get_table_name(const struct nf_hook_state *state)
{
    if (!state)
        return "UNKNOWN_TABLE";

    // 根据协议族判断表类型
    switch (state->pf)
    {
    case NFPROTO_IPV4:
        return "iptables";
    case NFPROTO_IPV6:
        return "ip6tables";
    case NFPROTO_ARP:
        return "arptables";
    case NFPROTO_BRIDGE:
        return "ebtables";
    default:
        return "unknown";
    }
}

// 获取设备名称
static const char *get_dev_name(const struct net_device *dev)
{
    return dev ? dev->name : "none";
}

// 增强的跟踪函数，尝试获取规则信息
static void trace_iptables_decision(struct sk_buff *skb,
                                    const struct nf_hook_state *state,
                                    unsigned int verdict)
{
    struct iphdr *iph = ip_hdr(skb);
    char saddr[16], daddr[16];
    const char *verdict_str;

    if (!iph)
        return;

    snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
    snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);

    verdict_str = (verdict == NF_ACCEPT) ? "ACCEPT" : (verdict == NF_DROP) ? "DROP"
                                                                           : "UNKNOWN";

    printk(KERN_INFO "IPTABLES-DECISION: table=%s chain=%s hook=%d "
                     "verdict=%s src=%s dst=%s proto=%d in=%s out=%s\n",
           get_table_name(state), get_chain_name(state), state->hook,
           verdict_str, saddr, daddr, iph->protocol,
           get_dev_name(state->in), get_dev_name(state->out));
}

// 包装函数，在实际的iptables决策点调用
unsigned int nf_trace_decision(unsigned int verdict, struct sk_buff *skb,
                               const struct nf_hook_state *state)
{
    trace_iptables_decision(skb, state, verdict);
    return verdict;
}
EXPORT_SYMBOL(nf_trace_decision);

// IPv4 钩子函数
static unsigned int ipv4_trace_hook(void *priv, struct sk_buff *skb,
                                    const struct nf_hook_state *state)
{
    struct iphdr *iph;
    char saddr[16], daddr[16];

    if (!trace_packets || !skb || !state)
        return NF_ACCEPT;

    iph = ip_hdr(skb);
    if (!iph)
        return NF_ACCEPT;

    // 格式化IP地址
    snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
    snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);

    printk(KERN_INFO "NFTRACE: table=%s chain=%s hook=%d verdict=ACCEPT "
                     "src=%s dst=%s protocol=%d in=%s out=%s\n",
           get_table_name(state), get_chain_name(state), state->hook,
           saddr, daddr, iph->protocol,
           get_dev_name(state->in), get_dev_name(state->out));

    return NF_ACCEPT;
}

// 用于记录DROP决策的钩子函数
static unsigned int ipv4_trace_drop_hook(void *priv, struct sk_buff *skb,
                                         const struct nf_hook_state *state)
{
    struct iphdr *iph;
    char saddr[16], daddr[16];

    if (!trace_packets || !skb || !state)
        return NF_DROP;

    iph = ip_hdr(skb);
    if (!iph)
        return NF_DROP;

    snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
    snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);

    printk(KERN_WARNING "NFTRACE: table=%s chain=%s hook=%d verdict=DROP "
                        "src=%s dst=%s protocol=%d in=%s out=%s\n",
           get_table_name(state), get_chain_name(state), state->hook,
           saddr, daddr, iph->protocol,
           get_dev_name(state->in), get_dev_name(state->out));

    return NF_DROP;
}

// ebtables 钩子函数
static unsigned int ebtables_trace_hook(void *priv, struct sk_buff *skb,
                                        const struct nf_hook_state *state)
{
    unsigned char *src_mac, *dst_mac;
    char src_mac_str[18], dst_mac_str[18];

    if (!trace_packets || !skb || !state || state->pf != NFPROTO_BRIDGE)
        return NF_ACCEPT;

    // 获取MAC地址
    src_mac = eth_hdr(skb)->h_source;
    dst_mac = eth_hdr(skb)->h_dest;

    // 格式化MAC地址
    snprintf(src_mac_str, sizeof(src_mac_str),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             src_mac[0], src_mac[1], src_mac[2],
             src_mac[3], src_mac[4], src_mac[5]);

    snprintf(dst_mac_str, sizeof(dst_mac_str),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             dst_mac[0], dst_mac[1], dst_mac[2],
             dst_mac[3], dst_mac[4], dst_mac[5]);

    printk(KERN_INFO "EBTRACE: table=ebtables chain=%s hook=%d verdict=ACCEPT "
                     "src_mac=%s dst_mac=%s in=%s out=%s\n",
           get_chain_name(state), state->hook,
           src_mac_str, dst_mac_str,
           get_dev_name(state->in), get_dev_name(state->out));

    return NF_ACCEPT;
}

// 定义钩子操作结构
static struct nf_hook_ops ipv4_trace_ops[] = {
    {
        .hook = ipv4_trace_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_PRE_ROUTING,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = ipv4_trace_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = ipv4_trace_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = ipv4_trace_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_LOCAL_OUT,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook = ipv4_trace_hook,
        .pf = NFPROTO_IPV4,
        .hooknum = NF_INET_POST_ROUTING,
        .priority = NF_IP_PRI_FIRST,
    }};

static struct nf_hook_ops ebtables_trace_ops[] = {
    {
        .hook = ebtables_trace_hook,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_PRE_ROUTING,
        .priority = NF_BR_PRI_FIRST,
    },
    {
        .hook = ebtables_trace_hook,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_LOCAL_IN,
        .priority = NF_BR_PRI_FIRST,
    },
    {
        .hook = ebtables_trace_hook,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_FORWARD,
        .priority = NF_BR_PRI_FIRST,
    },
    {
        .hook = ebtables_trace_hook,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_LOCAL_OUT,
        .priority = NF_BR_PRI_FIRST,
    },
    {
        .hook = ebtables_trace_hook,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_POST_ROUTING,
        .priority = NF_BR_PRI_FIRST,
    }};

// 模块参数
module_param(trace_packets, uint, 0644);
MODULE_PARM_DESC(trace_packets, "Enable packet tracing (0=disable, 1=enable)");

static int __init nftrace_module_init(void)
{
    int ret, i;

    printk(KERN_INFO "Netfilter Chain Tracer module loading...\n");

    // 注册 IPv4 钩子
    for (i = 0; i < ARRAY_SIZE(ipv4_trace_ops); i++)
    {
        ret = nf_register_net_hook(&init_net, &ipv4_trace_ops[i]);
        if (ret < 0)
        {
            printk(KERN_ERR "Failed to register IPv4 hook %d: %d\n", i, ret);
            goto error_ipv4;
        }
    }

    // 注册 ebtables 钩子
    for (i = 0; i < ARRAY_SIZE(ebtables_trace_ops); i++)
    {
        ret = nf_register_net_hook(&init_net, &ebtables_trace_ops[i]);
        if (ret < 0)
        {
            printk(KERN_ERR "Failed to register ebtables hook %d: %d\n", i, ret);
            goto error_ebtables;
        }
    }

    printk(KERN_INFO "Netfilter Chain Tracer module loaded successfully\n");
    printk(KERN_INFO "Tracing %s enabled\n", trace_packets ? "enabled" : "disabled");
    return 0;

error_ebtables:
    // 注销已注册的ebtables钩子
    for (i = 0; i < ARRAY_SIZE(ebtables_trace_ops); i++)
    {
        nf_unregister_net_hook(&init_net, &ebtables_trace_ops[i]);
    }

error_ipv4:
    // 注销已注册的IPv4钩子
    for (i = 0; i < ARRAY_SIZE(ipv4_trace_ops); i++)
    {
        nf_unregister_net_hook(&init_net, &ipv4_trace_ops[i]);
    }

    return ret;
}

static void __exit nftrace_module_exit(void)
{
    int i;

    // 注销所有钩子
    for (i = 0; i < ARRAY_SIZE(ipv4_trace_ops); i++)
    {
        nf_unregister_net_hook(&init_net, &ipv4_trace_ops[i]);
    }

    for (i = 0; i < ARRAY_SIZE(ebtables_trace_ops); i++)
    {
        nf_unregister_net_hook(&init_net, &ebtables_trace_ops[i]);
    }

    printk(KERN_INFO "Netfilter Chain Tracer module unloaded\n");
}

module_init(nftrace_module_init);
module_exit(nftrace_module_exit);