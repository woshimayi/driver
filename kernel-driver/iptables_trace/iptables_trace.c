#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/netfilter/nf_log.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/netfilter_bridge.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/inet.h>
#include <linux/icmp.h>
#include <net/net_namespace.h>

#undef PP
#define PP(fmt, ...) printk("\033[0;32m[zzzzz :%s(%d)] " fmt "\033[0m\r\n", __func__, __LINE__, ##__VA_ARGS__)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("IPTables Tracer");
MODULE_DESCRIPTION("Trace iptables chain decisions using kprobes");

static const char *get_dev_name(const struct net_device *dev);

// ====================================================================================================================
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

struct unified_rule_info g_info = {
    .valid_fields = UNIFIED_MATCH_IP_PROTO | UNIFIED_MATCH_IP_ADDR,
    .addr_ip = "172.16.27.28",
    // .dst_ip = "192.168.1.100",
    .ip_proto = IPPROTO_ICMP};

/**
 * @brief 统一匹配函数
 *
 * @param skb
 * @param info
 * @return true
 * @return false
 */
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

// ====================================================================================================================

// ebt_do_table kprobe 预处理函数
static int ebt_do_table_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    struct sk_buff *skb = NULL;
    void *state = NULL;
    void *priv = NULL;
#if defined(CONFIG_X86_64)
    skb = (struct sk_buff *)regs->di;
    state = (void *)regs->si;
    priv = (void *)regs->dx;
#elif defined(CONFIG_ARM64)
    skb = (struct sk_buff *)regs->regs[0];
    state = (void *)regs->regs[1];
    priv = (void *)regs->regs[2];
#elif defined(CONFIG_ARM)
    skb = (struct sk_buff *)regs->uregs[0];
    state = (void *)regs->uregs[1];
    priv = (void *)regs->uregs[2];
#else
    PP("ebt_do_table: Unsupported architecture\n");
    return 0;
#endif

    if (!skb)
        return 0;

    struct ethhdr *eth = eth_hdr(skb);
    if (!eth)
        return 0;

    char src[ETH_ALEN * 3], dst[ETH_ALEN * 3];
    snprintf(src, sizeof(src), "%pM", eth->h_source);
    snprintf(dst, sizeof(dst), "%pM", eth->h_dest);

    // 获取进出口接口名（需内核结构体，通常 state 为 struct nf_hook_state * 或 struct net_bridge_port *，此处仅演示）
    const char *in_name = "none", *out_name = "none";
    // 若 state 实为 struct nf_hook_state * 可如下获取：
    in_name = get_dev_name(((struct nf_hook_state *)state)->in);
    out_name = get_dev_name(((struct nf_hook_state *)state)->out);

    // 获取表名和链名
    const char *table_name = "unknown";
    if (priv)
    {
        table_name = ((struct ebt_table *)priv)->name;
    }
    int hooknum = -1;
    if (state)
    {
        struct nf_hook_state
        {
            int hook;
        };
        hooknum = ((struct nf_hook_state *)state)->hook;
    }
    // ebtables 链名与 hooknum 的映射
    const char *chain_name = "UNKNOWN";
    switch (hooknum)
    {
    case NF_BR_PRE_ROUTING:
        chain_name = "PREROUTING";
        break;
    case NF_BR_LOCAL_IN:
        chain_name = "INPUT";
        break;
    case NF_BR_FORWARD:
        chain_name = "FORWARD";
        break;
    case NF_BR_LOCAL_OUT:
        chain_name = "OUTPUT";
        break;
    case NF_BR_POST_ROUTING:
        chain_name = "POSTROUTING";
        break;
    }

    if (ntohs(eth->h_proto) == ETH_P_IP && skb)
    {
        struct iphdr *iph = ip_hdr(skb);
        if (iph)
        {
            char saddr[16], daddr[16];
            snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
            snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);

            if (unified_match_func(skb, &g_info))
            {
                PP("EBTABLES-ENTER: table=%s chain=%s in=%s out=%s src=%s dst=%s proto=0x%04x saddr=%s daddr=%s\n", table_name, chain_name, in_name, out_name, src, dst, ntohs(eth->h_proto), saddr, daddr);
            }

            return 0;
        }
    }
    // PP("EBTABLES-ENTER: in=%s out=%s table=%s chain=%s src=%s dst=%s proto=0x%04x \n", in_name, out_name, table_name, chain_name, src, dst, ntohs(eth->h_proto));
    return 0;
}

static void ebt_do_table_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    struct sk_buff *skb = NULL;
    void *state = NULL;
    void *priv = NULL;
    unsigned int verdict = 0;
#if defined(CONFIG_X86_64)
    skb = (struct sk_buff *)regs->di;
    state = (void *)regs->si;
    priv = (void *)regs->dx;
    verdict = regs->ax;
#elif defined(CONFIG_ARM64)
    skb = (struct sk_buff *)regs->regs[0];
    state = (void *)regs->regs[1];
    priv = (void *)regs->regs[2];
    verdict = regs->regs[0];
#elif defined(CONFIG_ARM)
    skb = (struct sk_buff *)regs->uregs[0];
    state = (void *)regs->uregs[1];
    priv = (void *)regs->uregs[2];
    verdict = regs->ARM_r0;
#else
    PP("ebt_do_table_post: Unsupported architecture\n");
    return;
#endif

    if (!skb)
        return;

    struct ethhdr *eth = eth_hdr(skb);
    if (!eth)
        return;

    char src[ETH_ALEN * 3], dst[ETH_ALEN * 3];
    snprintf(src, sizeof(src), "%pM", eth->h_source);
    snprintf(dst, sizeof(dst), "%pM", eth->h_dest);

    const char *in_name = "none", *out_name = "none";
    in_name = get_dev_name(((struct nf_hook_state *)state)->in);
    out_name = get_dev_name(((struct nf_hook_state *)state)->out);

    const char *table_name = "unknown";
    if (priv)
    {
        table_name = ((struct ebt_table *)priv)->name;
    }
    int hooknum = -1;
    if (state)
    {
        hooknum = ((struct nf_hook_state *)state)->hook;
    }

    const char *chain_name = "UNKNOWN";
    switch (hooknum)
    {
    case NF_BR_PRE_ROUTING:
        chain_name = "PREROUTING";
        break;
    case NF_BR_LOCAL_IN:
        chain_name = "INPUT";
        break;
    case NF_BR_FORWARD:
        chain_name = "FORWARD";
        break;
    case NF_BR_LOCAL_OUT:
        chain_name = "OUTPUT";
        break;
    case NF_BR_POST_ROUTING:
        chain_name = "POSTROUTING";
        break;
    }

    if (ntohs(eth->h_proto) == ETH_P_IP && skb)
    {
        struct iphdr *iph = ip_hdr(skb);
        if (iph)
        {
            char saddr[16], daddr[16];
            snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
            snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);
            if (unified_match_func(skb, &g_info))
            {
                PP("EBTABLES-EXIT: table=%s chain=%s in=%s out=%s src=%s dst=%s proto=0x%04x saddr=%s daddr=%s verdict=0x%x\n", table_name, chain_name, in_name, out_name, src, dst, ntohs(eth->h_proto), saddr, daddr, verdict);
            }
            return;
        }
    }
    // PP("EBTABLES-EXIT: in=%s out=%s table=%s chain=%s src=%s dst=%s proto=0x%04x verdict=0x%x\n", in_name, out_name, table_name, chain_name, src, dst, ntohs(eth->h_proto), verdict);
}

// ebt_do_table kprobe 注册
static struct kprobe ebt_do_table_kp;

// 全局变量
static unsigned int tracing_enabled = 1;
static struct kprobe ipt_do_table_kp;

// 获取链名称的辅助函数
static const char *get_chain_name(int hooknum)
{
    switch (hooknum)
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
        return "UNKNOWN";
    }
}

// 获取决策字符串
static const char *get_verdict_str(unsigned int verdict)
{
    switch (verdict)
    {
    case NF_ACCEPT:
        return "ACCEPT";
    case NF_DROP:
        return "DROP";
    case NF_STOLEN:
        return "STOLEN";
    case NF_QUEUE:
        return "QUEUE";
    case NF_REPEAT:
        return "REPEAT";
    case NF_STOP:
        return "STOP";
    default:
        return "UNKNOWN";
    }
}

static const char *get_proto_name(u8 protocol)
{
    switch (protocol)
    {
    case IPPROTO_TCP:
        return "TCP";
    case IPPROTO_UDP:
        return "UDP";
    case IPPROTO_ICMP:
        return "ICMP";
    case IPPROTO_ICMPV6:
        return "ICMPv6";
    case IPPROTO_ESP:
        return "ESP";
    case IPPROTO_AH:
        return "AH";
    default:
        return "OTHER";
    }
}

#define XT_TABLE_MAXNAMELEN 32
struct xt_table
{
    struct list_head list;

    /* What hooks you will enter on */
    unsigned int valid_hooks;

    /* Man behind the curtain... */
    struct xt_table_info __rcu *private;

    /* Set this to THIS_MODULE if you are a module, otherwise NULL */
    struct module *me;

    u_int8_t af;  /* address/protocol family */
    int priority; /* hook order */

    /* called when table is needed in the given netns */
    int (*table_init)(struct net *net);

    /* A unique name... */
    const char name[XT_TABLE_MAXNAMELEN];
};
#define ipt_table xt_table

// 获取设备名称
static const char *get_dev_name(const struct net_device *dev)
{
    return dev ? dev->name : "none";
}

// ipt_do_table 的 kprobe 预处理函数
static int ipt_do_table_pre_handler(struct kprobe *p, struct pt_regs *regs)
// Linux 5.10 nf_log_packet 用法示例：
// nf_log_packet(AF_INET, 0, skb, state->in, state->out, NULL,
//     "nf_log: src=%pI4 dst=%pI4 proto=%u", &iph->saddr, &iph->daddr, iph->protocol);
{
    struct sk_buff *skb;
    const struct nf_hook_state *state;
    void *priv;
    struct iphdr *iph;
    char saddr[16], daddr[16];
    int detected = 0;

    if (!tracing_enabled)
        return 0;

    // 获取函数参数（根据系统架构调整）
#if defined(CONFIG_X86_64)
    skb = (struct sk_buff *)regs->di;
    state = (const struct nf_hook_state *)regs->si;
    priv = (void *)regs->dx;
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    skb = (struct sk_buff *)regs->uregs[0];
    state = (const struct nf_hook_state *)regs->uregs[1];
    priv = (void *)regs->uregs[2];
#else
    return 0;
#endif

    if (!skb || !state)
        return 0;

    // 获取IP头
    iph = ip_hdr(skb);
    if (!iph)
        return 0;

    // 格式化IP地址
    snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
    snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);

    detected = unified_match_func(skb, &g_info);

    // 获取表名（通过priv参数，需要根据实际结构解析）
    const char *table_name = "unknown";
    if (priv)
    {
        // priv 通常指向 ipt_table 结构
        // 这里需要访问结构体成员，需要内核头文件知识
        struct ipt_table *table = (struct ipt_table *)priv;
        table_name = table->name;
    }

    if (!detected)
    {
        return 0;
    }
    PP("IPTABLES-ENTER: table=%s chain=%s in=%s out=%s hook=%d "
       "src=%s dst=%s proto=%s \n",
       table_name, get_chain_name(state->hook), get_dev_name(state->in), get_dev_name(state->out),
       state->hook, saddr, daddr, get_proto_name(iph->protocol));

    return 0;
}

// ipt_do_table 的 kprobe 后处理函数
static void ipt_do_table_post_handler(struct kprobe *p, struct pt_regs *regs,
                                      unsigned long flags)
{
    struct sk_buff *skb;
    const struct nf_hook_state *state;
    void *priv;
    struct iphdr *iph;
    char saddr[16], daddr[16];
    unsigned int verdict;
    int detected = 0;

    if (!tracing_enabled)
        return;

    // 获取函数参数
#if defined(CONFIG_X86_64)
    skb = (struct sk_buff *)regs->di;
    state = (const struct nf_hook_state *)regs->si;
    priv = (void *)regs->dx;
    verdict = regs->ax; // 返回值在RAX
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    skb = (struct sk_buff *)regs->uregs[0];
    state = (const struct nf_hook_state *)regs->uregs[1];
    priv = (void *)regs->uregs[2];
#if defined(CONFIG_ARM64)
    verdict = regs->regs[0];
#else
    verdict = regs->ARM_r0;
#endif
#else
    return;
#endif

    if (!skb || !state)
        return;

    iph = ip_hdr(skb);
    if (!iph)
        return;

    struct net *net = state->net;     // 获取网络命名空间
    // struct net_device *in_dev = state->in;   // 获取输入设备
    // struct net_device *out_dev = state->out; // 获取输出设备

    snprintf(saddr, sizeof(saddr), "%pI4", &iph->saddr);
    snprintf(daddr, sizeof(daddr), "%pI4", &iph->daddr);

    detected = unified_match_func(skb, &g_info);

    const char *table_name = "unknown";
    if (priv)
    {
        // priv 通常指向 ipt_table 结构
        // 这里需要访问结构体成员，需要内核头文件知识
        struct ipt_table *table = (struct ipt_table *)priv;
        table_name = table->name;
    }

    if (detected)
    {
        PP("IPTABLES-EXIT: table=%s chain=%s in=%s out=%s hook=%d verdict=%s "
           "src=%s dst=%s proto=%s \n",
           table_name, get_chain_name(state->hook), get_dev_name(state->in), get_dev_name(state->out), state->hook,
           get_verdict_str(verdict), saddr, daddr, get_proto_name(iph->protocol));
    }

}

// 模块参数
module_param(tracing_enabled, uint, 0644);
MODULE_PARM_DESC(tracing_enabled, "Enable tracing (0=disable, 1=enable)");

static int __init iptables_tracer_init(void)
{
    int ret;

    PP("IPTables Tracer module loading...\n");

    // 设置 kprobe
    memset(&ipt_do_table_kp, 0, sizeof(struct kprobe));
    ipt_do_table_kp.symbol_name = "ipt_do_table";
    ipt_do_table_kp.pre_handler = ipt_do_table_pre_handler;
    ipt_do_table_kp.post_handler = ipt_do_table_post_handler;

    // 注册 kprobe
    ret = register_kprobe(&ipt_do_table_kp);
    if (ret < 0)
    {
        PP("Failed to register kprobe: %d\n", ret);
        return ret;
    }

    // 注册 ebt_do_table kprobe
    memset(&ebt_do_table_kp, 0, sizeof(struct kprobe));
    ebt_do_table_kp.symbol_name = "ebt_do_table";
    ebt_do_table_kp.pre_handler = ebt_do_table_pre_handler;
    ebt_do_table_kp.post_handler = ebt_do_table_post_handler;
    ret = register_kprobe(&ebt_do_table_kp);
    if (ret < 0)
    {
        PP("Failed to register ebt_do_table kprobe: %d\n", ret);
    }

    PP("IPTables Tracer module loaded successfully\n");
    PP("Tracing %s\n", tracing_enabled ? "enabled" : "disabled");
    return 0;
}

static void __exit iptables_tracer_exit(void)
{
    unregister_kprobe(&ipt_do_table_kp);
    unregister_kprobe(&ebt_do_table_kp);
    PP("IPTables Tracer module unloaded\n");
}

module_init(iptables_tracer_init);
module_exit(iptables_tracer_exit);