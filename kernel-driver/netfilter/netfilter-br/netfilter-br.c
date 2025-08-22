#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h> // 桥接框架头文件
#include <linux/skbuff.h>
#include <linux/if_ether.h> // 以太网帧头
#include <linux/if_packet.h> // L2层协议
#include <linux/ip.h>
#include <linux/udp.h>

static inline const char *hook_name(unsigned int h)
{
	switch (h)
	{
		case NF_BR_PRE_ROUTING:     return "NF_BR_PRE_ROUTING";
		case NF_BR_LOCAL_IN:        return "NF_BR_LOCAL_IN";
		case NF_BR_FORWARD:         return "NF_BR_FORWARD";
		case NF_BR_LOCAL_OUT:       return "NF_BR_LOCAL_OUT";
		case NF_BR_POST_ROUTING:    return "NF_BR_POST_ROUTING";
		case NF_BR_BROUTING:        return "NF_BR_BROUTING";
		case NF_BR_NUMHOOKS:        return "NF_BR_NUMHOOKS";
		default:                    return "";
	}
}

static unsigned int l2_filter_hook_func(void *priv,
                                        struct sk_buff *skb,
                                        const struct nf_hook_state *state)
{
    // 获取以太网头部
    struct ethhdr *eth_hdr_ptr  = eth_hdr(skb);

    if (!eth_hdr_ptr ) {
        return NF_ACCEPT; // 没有以太网头部，直接放行
    }

    // 检查以太网协议类型
    // 假设我们想要过滤 ARP 报文 (协议类型 ETH_P_ARP = 0x0806)
    if (ntohs(eth_hdr_ptr ->h_proto) == ETH_P_ARP) {
        pr_info("Netfilter Bridge: Detected and dropping an ARP packet.\n");
        return NF_DROP; // 丢弃该报文
    }

    // 假设我们想处理 IPV4 报文
    if (ntohs(eth_hdr_ptr ->h_proto) == ETH_P_IP) {
        // 在这里可以进一步解析IP头部
        struct iphdr *iph = ip_hdr(skb);
        if (iph) {
            // 你可以在这里添加更复杂的逻辑
            // 例如，检查源/目的IP，或传输层协议
            // pr_info("Netfilter Bridge: IP packet detected.\n");
			PP("pf=%u hook=%u(%s) in=%s out=%s",
			   state->pf, state->hook, hook_name(state->hook),
			   state->in ? state->in->name : "-", state->out ? state->out->name : "-");
        }
    }

    // 对于其他所有报文，默认放行
    return NF_ACCEPT;
}



static struct nf_hook_ops l2_filter_ops[] = {
    {
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_PRE_ROUTING, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE, // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
	{
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_LOCAL_IN, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE, // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
	{
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_FORWARD, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE, // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
	{
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_LOCAL_OUT, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE, // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
	{
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_POST_ROUTING, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE, // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    },
	{
        .hook = l2_filter_hook_func,
        .hooknum = NF_BR_BROUTING, // 桥接框架的 hook 点
        .pf = NFPROTO_BRIDGE, // 协议族必须是桥接
        .priority = NF_BR_PRI_FIRST, // 优先级最高，尽早处理
    }
};


// 模块初始化函数
static int __init l2_filter_init(void)
{
    int ret;
    pr_info("Netfilter L2 Filter Module: Loading...\n");
    ret = nf_register_net_hooks(&init_net, l2_filter_ops, ARRAY_SIZE(l2_filter_ops));
    if (ret < 0) {
        pr_err("Failed to register netfilter hooks\n");
        return ret;
    }
    pr_info("Netfilter L2 Filter Module: Loaded successfully.\n");
    return 0;
}

// 模块退出函数
static void __exit l2_filter_exit(void)
{
    pr_info("Netfilter L2 Filter Module: Unloading...\n");
    nf_unregister_net_hooks(&init_net, l2_filter_ops, ARRAY_SIZE(l2_filter_ops));
    pr_info("Netfilter L2 Filter Module: Unloaded.\n");
}

module_init(l2_filter_init);
module_exit(l2_filter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Netfilter L2 packet filter for Linux 5.10.");