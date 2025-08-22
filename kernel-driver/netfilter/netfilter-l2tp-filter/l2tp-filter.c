/*
 * @*************************************:
 * @FilePath     : \proc_net\timer_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-16 19:24:26
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-21 17:27:59
 * @Descripttion :  Netfilter L2TP PPP LCP 过滤模块 - 兼容VLAN包头
 * @compile      : make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
 * @**************************************:
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/ip.h>
#include <net/udp.h>

#undef PP
#define PP(fmt, ...) printk("\033[0;32;32m[zzzzz :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)

#define L2TP_PORT 1701	   // L2TP 默认端口
#define PPP_LCP_CODE 0x01  // LCP 请求的代码
#define FILTER_VLAN_ID 100 // 需要过滤的 VLAN ID

static inline const char *hook_name(unsigned int h)
{
	switch (h)
	{
	case NF_INET_PRE_ROUTING:
		return "PRE_ROUTING";
	case NF_INET_LOCAL_IN:
		return "LOCAL_IN";
	case NF_INET_FORWARD:
		return "FORWARD";
	case NF_INET_LOCAL_OUT:
		return "LOCAL_OUT";
	case NF_INET_POST_ROUTING:
		return "POST_ROUTING";
	default:
		return "?";
	}
}

// Netfilter 钩子函数
static unsigned int l2tp_filter_hook(void *priv,
									 struct sk_buff *skb,
									 const struct nf_hook_state *state)
{
	struct iphdr *iph;
	struct udphdr *udph;
	unsigned char *data;
	struct vlan_hdr *vlan_header;
	unsigned int ip_header_len, l2tp_header_len;
	unsigned short vlan_id = 0;
	struct ethhdr *eth;

	// 获取以太网头部
	eth = (struct ethhdr *)skb_mac_header(skb);
	if (!eth)
	{
		return NF_ACCEPT;
	}

	// 检查是否是 VLAN 标记的以太网帧
	// if (ntohs(eth->h_proto) != ETH_P_8021Q) {
	// 	return NF_ACCEPT;
	// }
	// print_hex_dump(KERN_ERR, "PKT: ", DUMP_PREFIX_OFFSET, 16, 1, eth, 32, true);
	// vlan_header = (struct vlan_hdr *)(eth + 1); // 获取 VLAN 头部
	// vlan_id = ntohs(vlan_header->h_vlan_TCI) & 0x0FFF;  // 获取 VLAN ID
	// PP("vlan = %d", vlan_id);

	// 获取 IP 头部
	iph = ip_hdr(skb);
	if (!iph)
	{
		return NF_ACCEPT;
	}

	// 检查 UDP 数据包是否是 L2TP 包 (默认端口 1701)
	if (iph->protocol == IPPROTO_UDP)
	{
		udph = udp_hdr(skb);
		if (ntohs(udph->dest) != L2TP_PORT && ntohs(udph->source) != L2TP_PORT)
		{
			return NF_ACCEPT;
		}
		// 获取 L2TP 数据包的起始地址
		data = (unsigned char *)udph + sizeof(struct udphdr);
		// print_hex_dump(KERN_ERR, "PKT: ", DUMP_PREFIX_OFFSET, 16, 1, data, 32, true);
		ip_header_len = iph->ihl * 4;
		l2tp_header_len = 6; // L2TP header length

		// 检查数据包长度是否足够
		if (skb->len < (ip_header_len + sizeof(struct udphdr) + l2tp_header_len + 12))
		{
			return NF_ACCEPT;
		}

		// 假设 L2TP 中直接传递 PPP 数据，L2TP+PPP 报文中 LCP 数据通常从第 12 字节开始
		// LCP 在 PPP 帧中通常位于第 4 字节位置（1字节代码，1字节标识，1字节长度，数据部分）
		unsigned char lcp_code = data[10]; // 这应该是 LCP 的代码字段
		unsigned char lcp_code_1 = data[11];
		// PP("lcp_code = 0x%x", lcp_code);

		// 如果是 LCP 请求
		if ((lcp_code == 0xc0 && lcp_code_1 == 0x21) || (data[8] == 0xc0 && data[9] == 0x21))
		{
			PP("pf=%u hook=%u(%s) in=%s out=%s",
			   state->pf, state->hook, hook_name(state->hook),
			   state->in ? state->in->name : "-", state->out ? state->out->name : "-");

			unsigned long tag = (unsigned long)priv; // 0x1/0x2/0x3
			// PP("hook=%u tag=%lu in=%s out=%s",
			// 				state->hook, tag,
			// 				state->in ? state->in->name : "-", state->out ? state->out->name : "-");
			PP("L2TP PPP LCP packet detected, ACCEPT... (0x%x 0x%x) (0x%x 0x%x)\n", data[8], data[9], data[10], data[11]);
			// PP("acc_disable = %d", skb->hi_ext_skbuff.qos.acc_disable);

			skb->hi_ext_skbuff.qos.pq = 6;
			skb->hi_ext_skbuff.qos.pq_en = 1;
			// skb->hi_ext_skbuff.qos.acc_disable = 1;
			return NF_ACCEPT;
		}
	}

	return NF_ACCEPT;
}

// Netfilter 钩子操作结构
static struct nf_hook_ops l2tp_filter_ops[] = {
	{
		.hook = l2tp_filter_hook,
		.pf = PF_INET,					// IPv4 协议族
		.hooknum = NF_INET_PRE_ROUTING, // 入站/转发
		.priority = NF_IP_PRI_FIRST,
		.priv = (void *)0x1,           // 标记来源：PRE_ROUTING
	},
	{
		.hook = l2tp_filter_hook,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT, // 本地发出的报文
		.priority = NF_IP_PRI_FIRST,
		.priv = (void *)0x2,           // 标记来源：LOCAL_OUT
	},
	{
		.hook = l2tp_filter_hook,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING, // 所有将要发出的报文
		.priority = NF_IP_PRI_FIRST,
		.priv = (void *)0x3,             // 标记来源：POST_ROUTING
	},
};

// 模块初始化函数
static int __init l2tp_filter_init(void)
{
	int ret;
	ret = nf_register_net_hooks(&init_net, l2tp_filter_ops, ARRAY_SIZE(l2tp_filter_ops));
	if (ret)
	{
		PP("Failed to register netfilter hooks\n");
		return ret;
	}

	PP("L2TP filter module with VLAN support loaded\n");
	return 0;
}

// 模块卸载函数
static void __exit l2tp_filter_exit(void)
{
	nf_unregister_net_hooks(&init_net, l2tp_filter_ops, ARRAY_SIZE(l2tp_filter_ops));
	PP("L2TP filter module with VLAN support unloaded\n");
}

module_init(l2tp_filter_init);
module_exit(l2tp_filter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Netfilter L2TP PPP LCP Filter with VLAN support");
