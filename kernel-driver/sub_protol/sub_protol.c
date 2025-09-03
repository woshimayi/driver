#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/protocol.h>
#include <linux/printk.h>
#include <linux/etherdevice.h>
#include <net/icmp.h>
#include <net/xfrm.h>
#include <linux/netfilter.h>

#undef PP
#define PP(fmt, ...) printk("\033[0;32m[zzzzz :%s(%d)] " fmt "\033[0m\r\n", __func__, __LINE__, ##__VA_ARGS__)

// IANA 建议的私有协议号范围，我们选择一个用于测试
#define MY_PROTO_NUMBER 253

// 简单协议头结构体，仅包含一个类型字段
struct my_proto_header
{
    u8 type;
};

// 简单的发送函数，用于回复数据包
static void my_proto_send_reply(struct net *net, __be32 saddr, __be32 daddr, const u8 *src_mac, const u8 *dst_mac);

#define ICMP_ECHOREPLY 0       /* Echo Reply			*/
#define ICMP_DEST_UNREACH 3    /* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH 4   /* Source Quench		*/
#define ICMP_REDIRECT 5        /* Redirect (change route)	*/
#define ICMP_ECHO 8            /* Echo Request			*/
#define ICMP_TIME_EXCEEDED 11  /* Time Exceeded		*/
#define ICMP_PARAMETERPROB 12  /* Parameter Problem		*/
#define ICMP_TIMESTAMP 13      /* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY 14 /* Timestamp Reply		*/
#define ICMP_INFO_REQUEST 15   /* Information Request		*/
#define ICMP_INFO_REPLY 16     /* Information Reply		*/
#define ICMP_ADDRESS 17        /* Address Mask Request		*/
#define ICMP_ADDRESSREPLY 18   /* Address Mask Reply		*/
#define NR_ICMP_TYPES 18

struct icmp_bxm
{
    struct sk_buff *skb;
    int offset;
    int data_len;

    struct
    {
        struct icmphdr icmph;
        __be32 times[3];
    } data;
    int head_len;
    struct ip_options_data replyopts;
};

bool ping_rcv(struct sk_buff *skb);
bool icmp_discard(struct sk_buff *skb);
bool icmp_unreach(struct sk_buff *skb);
bool icmp_redirect(struct sk_buff *skb);
bool icmp_echo(struct sk_buff *skb);
bool icmp_timestamp(struct sk_buff *skb);

struct icmp_control
{
    bool (*handler)(struct sk_buff *skb);
    short error; /* This ICMP is classed as an error message */
};

static const struct icmp_control icmp_pointers[] = {
    [ICMP_ECHOREPLY] = {
        .handler = ping_rcv,
    },
    [1] = {
        .handler = icmp_discard,
        .error = 1,
    },
    [2] = {
        .handler = icmp_discard,
        .error = 1,
    },
    [ICMP_DEST_UNREACH] = {
        .handler = icmp_unreach,
        .error = 1,
    },
    [ICMP_SOURCE_QUENCH] = {
        .handler = icmp_unreach,
        .error = 1,
    },
    [ICMP_REDIRECT] = {
        .handler = icmp_redirect,
        .error = 1,
    },
    [6] = {
        .handler = icmp_discard,
        .error = 1,
    },
    [7] = {
        .handler = icmp_discard,
        .error = 1,
    },
    [ICMP_ECHO] = {
        .handler = icmp_echo,
    },
    [9] = {
        .handler = icmp_discard,
        .error = 1,
    },
    [10] = {
        .handler = icmp_discard,
        .error = 1,
    },
    [ICMP_TIME_EXCEEDED] = {
        .handler = icmp_unreach,
        .error = 1,
    },
    [ICMP_PARAMETERPROB] = {
        .handler = icmp_unreach,
        .error = 1,
    },
    [ICMP_TIMESTAMP] = {
        .handler = icmp_timestamp,
    },
    [ICMP_TIMESTAMPREPLY] = {
        .handler = icmp_discard,
    },
    [ICMP_INFO_REQUEST] = {
        .handler = icmp_discard,
    },
    [ICMP_INFO_REPLY] = {
        .handler = icmp_discard,
    },
    [ICMP_ADDRESS] = {
        .handler = icmp_discard,
    },
    [ICMP_ADDRESSREPLY] = {
        .handler = icmp_discard,
    },
};


static inline void icmp_xmit_unlock(struct sock *sk)
{
    spin_unlock(&sk->sk_lock.slock);
}


static void icmp_reply(struct icmp_bxm *icmp_param, struct sk_buff *skb)
{
    struct iphdr *old_iph = ip_hdr(skb);
    struct icmphdr *old_icmph = icmp_hdr(skb);
    struct net *net = dev_net(skb->dev);
    struct rtable *rt;
    struct flowi4 fl4;
    struct sk_buff *nskb;
    struct iphdr *iph;
    struct icmphdr *icmph;
    int total_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + skb->len;

    memset(&fl4, 0, sizeof(fl4));
    fl4.daddr = old_iph->saddr;
    fl4.saddr = old_iph->daddr;
    fl4.flowi4_proto = MY_PROTO_NUMBER;

    rt = ip_route_output_flow(net, &fl4, NULL);
    if (IS_ERR(rt))
        return;

    nskb = alloc_skb(LL_RESERVED_SPACE(rt->dst.dev) + total_len, GFP_ATOMIC);
    if (!nskb) {
        ip_rt_put(rt);
        return;
    }
    skb_reserve(nskb, LL_RESERVED_SPACE(rt->dst.dev));
    skb_reset_network_header(nskb);
    iph = (struct iphdr *)skb_put(nskb, sizeof(struct iphdr));
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = old_iph->tos;
    iph->tot_len = htons(total_len);
    iph->id = 0;
    iph->frag_off = htons(IP_DF);
    iph->ttl = 64;
    iph->protocol = MY_PROTO_NUMBER;
    iph->saddr = old_iph->daddr;
    iph->daddr = old_iph->saddr;
    iph->check = 0;
    ip_send_check(iph);

    icmph = (struct icmphdr *)skb_put(nskb, sizeof(struct icmphdr));
    memcpy(icmph, old_icmph, sizeof(struct icmphdr));
    icmph->type = ICMP_ECHOREPLY;
    icmph->checksum = 0;
    // 拷贝原始ICMP数据部分
    if (skb->len > sizeof(struct icmphdr))
        skb_copy_bits(skb, sizeof(struct iphdr) + sizeof(struct icmphdr), skb_put(nskb, skb->len - sizeof(struct icmphdr)), skb->len - sizeof(struct icmphdr));
    icmph->checksum = ip_compute_csum((unsigned char *)icmph, nskb->len - sizeof(struct iphdr));

    nskb->dev = rt->dst.dev;
    skb_dst_set(nskb, &rt->dst);
    ip_local_out(net, NULL, nskb);
    ip_rt_put(rt);
}

bool ping_rcv(struct sk_buff *skb)
{
    PP();
    return true;
}

bool icmp_discard(struct sk_buff *skb)
{
    PP();
    return true;
}

bool icmp_unreach(struct sk_buff *skb)
{
    PP();
    return true;
}

bool icmp_redirect(struct sk_buff *skb)
{
    PP();
    return true;
}

bool icmp_echo(struct sk_buff *skb)
{
    struct net *net;

    net = dev_net(skb_dst(skb)->dev);
    if (!net->ipv4.sysctl_icmp_echo_ignore_all)
    {
        struct icmp_bxm icmp_param;

        icmp_param.data.icmph = *icmp_hdr(skb);
        icmp_param.data.icmph.type = ICMP_ECHOREPLY;
        icmp_param.skb = skb;
        icmp_param.offset = 0;
        icmp_param.data_len = skb->len;
        icmp_param.head_len = sizeof(struct icmphdr);
        icmp_reply(&icmp_param, skb);
    }

    return true;
}

bool icmp_timestamp(struct sk_buff *skb)
{
    PP();
    return true;
}

// 接收数据包处理函数
static int my_proto_rcv(struct sk_buff *skb)
{
    struct icmphdr *icmph;
    struct rtable *rt = skb_rtable(skb);
    struct net *net = dev_net(rt->dst.dev);
    bool success;
    PP();

    if (!xfrm4_policy_check(NULL, XFRM_POLICY_IN, skb))
    {
        struct sec_path *sp = skb_sec_path(skb);
        int nh;

        if (!(sp && sp->xvec[sp->len - 1]->props.flags &
                        XFRM_STATE_ICMP))
            goto drop;

        if (!pskb_may_pull(skb, sizeof(*icmph) + sizeof(struct iphdr)))
            goto drop;

        nh = skb_network_offset(skb);
        skb_set_network_header(skb, sizeof(*icmph));

        if (!xfrm4_policy_check_reverse(NULL, XFRM_POLICY_IN, skb))
            goto drop;

        skb_set_network_header(skb, nh);
    }
    PP();

    __ICMP_INC_STATS(net, ICMP_MIB_INMSGS);

    if (skb_checksum_simple_validate(skb))
        goto csum_error;

    if (!pskb_pull(skb, sizeof(*icmph)))
        goto error;

    icmph = icmp_hdr(skb);

    ICMPMSGIN_INC_STATS(net, icmph->type);
    /*
     *	18 is the highest 'known' ICMP type. Anything else is a mystery
     *
     *	RFC 1122: 3.2.2  Unknown ICMP messages types MUST be silently
     *		  discarded.
     */
    PP("icmph->type = %d\n", icmph->type);
    if (icmph->type > NR_ICMP_TYPES)
        goto error;

    /*
     *	Parse the ICMP message
     */

    if (rt->rt_flags & (RTCF_BROADCAST | RTCF_MULTICAST))
    {
        /*
         *	RFC 1122: 3.2.2.6 An ICMP_ECHO to broadcast MAY be
         *	  silently ignored (we let user decide with a sysctl).
         *	RFC 1122: 3.2.2.8 An ICMP_TIMESTAMP MAY be silently
         *	  discarded if to broadcast/multicast.
         */
        if ((icmph->type == ICMP_ECHO ||
             icmph->type == ICMP_TIMESTAMP) &&
            net->ipv4.sysctl_icmp_echo_ignore_broadcasts)
        {
            goto error;
        }
        if (icmph->type != ICMP_ECHO &&
            icmph->type != ICMP_TIMESTAMP &&
            icmph->type != ICMP_ADDRESS &&
            icmph->type != ICMP_ADDRESSREPLY)
        {
            goto error;
        }
    }

    success = icmp_pointers[icmph->type].handler(skb);

    if (success)
    {
        consume_skb(skb);
        return NET_RX_SUCCESS;
    }

drop:
    kfree_skb(skb);
    return NET_RX_DROP;
csum_error:
    __ICMP_INC_STATS(net, ICMP_MIB_CSUMERRORS);
error:
    __ICMP_INC_STATS(net, ICMP_MIB_INERRORS);
    goto drop;
}

// 协议注册结构体
static struct net_protocol my_proto_protocol = {
    .handler = my_proto_rcv,
    .no_policy = 1,
    .netns_ok = 1,
};

// 发送回复函数
static void my_proto_send_reply(struct net *net, __be32 saddr, __be32 daddr, const u8 *src_mac, const u8 *dst_mac)
{
    struct sk_buff *skb;
    struct iphdr *iph;
    struct my_proto_header *myh;
    struct flowi4 fl4;
    struct rtable *rt;
    struct net_device *dev;

    // 准备路由信息，源和目的地址交换
    memset(&fl4, 0, sizeof(fl4));
    fl4.daddr = saddr; // 回复的目标是原始请求的源地址
    fl4.saddr = daddr; // 回复的源是原始请求的目的地址

    // 查找路由
    rt = ip_route_output_flow(net, &fl4, NULL);
    if (IS_ERR(rt))
    {
        PP("MY_PROTO: No route to host.\n");
        return;
    }

    dev = rt->dst.dev;
    if (!dev)
    {
        PP("MY_PROTO: Route device is NULL!\n");
        ip_rt_put(rt);
        return;
    }

    // 分配 sk_buff，并为链路层头部预留空间
    skb = alloc_skb(LL_RESERVED_SPACE(dev) + ETH_HLEN + sizeof(struct iphdr) + sizeof(struct my_proto_header), GFP_ATOMIC);
    if (!skb)
    {
        PP("MY_PROTO: Failed to allocate skb.\n");
        ip_rt_put(rt);
        return;
    }

    PP();
    // 预留链路层空间
    skb_reserve(skb, LL_RESERVED_SPACE(dev));
    // struct ethhdr *eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
    // PP();
    // skb_reset_mac_header(skb);
    // PP();
    // memcpy(eth->h_dest, src_mac, ETH_ALEN);
    // memcpy(eth->h_source, dst_mac, ETH_ALEN);
    // eth->h_proto = htons(ETH_P_IP);
    // PP("MY_PROTO: Ethernet header set: src %pM, dst %pM", eth->h_source, eth->h_dest);
    skb_push(skb, sizeof(struct iphdr));
    PP();
    skb_reset_network_header(skb);
    PP();
    iph = ip_hdr(skb);
    PP();

    // 1. 填充 IP 头部
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(skb->len);
    iph->id = 0;
    iph->frag_off = htons(IP_DF);
    iph->ttl = 64;
    iph->protocol = MY_PROTO_NUMBER;
    iph->check = 0; // IP 校验和将在 ip_local_out_skb 内部计算
    iph->saddr = daddr;
    iph->daddr = saddr;

    PP("%pI4 -> %pI4\n", &iph->saddr, &iph->daddr);

    // 2. 填充自定义协议头部
    skb_put(skb, sizeof(struct my_proto_header));
    skb_reset_transport_header(skb);
    myh = (struct my_proto_header *)skb_transport_header(skb);
    myh->type = 2; // 回复类型
    PP();
    // 3. 关联路由信息
    skb->dev = dev;
    skb_dst_set(skb, &rt->dst);

    // 打印MAC地址（如果有链路层头部）
    if (skb_mac_header_was_set(skb) && skb_mac_header(skb) && skb_mac_header(skb) + ETH_HLEN <= skb->data + skb->len)
    {
        const struct ethhdr *eth = (const struct ethhdr *)skb_mac_header(skb);
        PP("MAC src: %pM, dst: %pM\n", eth->h_source, eth->h_dest);
    }
    else
    {
        PP("No valid MAC header in skb\n");
    }
    print_hex_dump(KERN_ERR, "MY_PROTO: Sending packet data: ", DUMP_PREFIX_OFFSET, 16, 1, skb->data, skb->len, true);
    PP();
    // 4. 发送数据包
    // ip_local_out_skb 会处理 IP 校验和并释放 skb 和路由
    // ip_local_out_skb(skb);
    ip_local_out(net, NULL, skb);
}

// 模块初始化函数
static int __init my_proto_init(void)
{
    int ret;
    PP("MY_PROTO: Module loading...\n");
    ret = inet_add_protocol(&my_proto_protocol, MY_PROTO_NUMBER);
    if (ret)
    {
        PP("MY_PROTO: Failed to add protocol.\n");
        return ret;
    }
    PP("MY_PROTO: Protocol registered successfully (num %d).\n", MY_PROTO_NUMBER);
    return 0;
}

// 模块卸载函数
static void __exit my_proto_exit(void)
{
    inet_del_protocol(&my_proto_protocol, MY_PROTO_NUMBER);
    PP("MY_PROTO: Protocol unregistered. Module exited.\n");
}

module_init(my_proto_init);
module_exit(my_proto_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel L4 protocol module.");
MODULE_VERSION("0.1");