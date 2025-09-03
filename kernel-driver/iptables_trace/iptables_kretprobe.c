#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/string.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/version.h>

#undef PP
#define PP(fmt, ...) printk("\033[0;32m[zzzzz :%s(%d)] " fmt "\033[0m\r\n", __func__, __LINE__, ##__VA_ARGS__)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("IPTables Hybrid Tracer");
MODULE_DESCRIPTION("Hybrid kprobe/kretprobe tracer for iptables decisions");

// 跟踪数据结构
#define XT_TABLE_MAXNAMELEN 32
struct iptables_trace_data
{
    ktime_t entry_time;
    char src_ip[16];
    char dst_ip[16];
    u8 protocol;
    int hook;
    char table_name[XT_TABLE_MAXNAMELEN];
    const struct net_device *in;
    const struct net_device *out;
};

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

// 全局变量
static unsigned int tracing_enabled = 1;
static struct kprobe ipt_do_table_kp;
static struct kretprobe ipt_do_table_rp;

// 获取链名称
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
    switch (verdict & NF_VERDICT_MASK)
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

// 获取设备名称
static const char *get_dev_name(const struct net_device *dev)
{
    return dev ? dev->name : "none";
}

// kprobe 预处理函数
static int ipt_do_table_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    struct sk_buff *skb;
    const struct nf_hook_state *state;
    void *priv;
    struct iphdr *iph;
    struct iptables_trace_data *data;

    if (!tracing_enabled)
        return 0;

    // 获取函数参数（多架构支持）
#if defined(CONFIG_X86_64)
    skb = (struct sk_buff *)regs->di;
    state = (const struct nf_hook_state *)regs->si;
    priv = (void *)regs->dx;
#elif defined(CONFIG_ARM64)
    skb = (struct sk_buff *)regs->regs[0];
    state = (const struct nf_hook_state *)regs->regs[1];
    priv = (void *)regs->regs[2];
#elif defined(CONFIG_ARM)
    skb = (struct sk_buff *)regs->uregs[0];
    state = (const struct nf_hook_state *)regs->uregs[1];
    priv = (void *)regs->uregs[2];
#else
    PP("Unsupported architecture\n");
    return 0;
#endif

    if (!skb || !state || !skb->data)
        return 0;

    // 检查是否为IP数据包
    if (skb->protocol != htons(ETH_P_IP))
        return 0;

    iph = ip_hdr(skb);
    if (!iph)
        return 0;

    // Linux 5.10: kprobe 没有 data 字段，kretprobe 需用 entry_handler 设置 ri->data
    // 此处只分配和填充数据，返回指针，由 entry_handler 赋值
    data = kmalloc(sizeof(*data), GFP_ATOMIC);
    if (!data)
    {
        PP("Failed to allocate trace data\n");
        return 0;
    }

    data->entry_time = ktime_get();
    data->hook = state->hook;
    data->protocol = iph->protocol;
    data->in = state->in;
    data->out = state->out;
    snprintf(data->src_ip, sizeof(data->src_ip), "%pI4", &iph->saddr);
    snprintf(data->dst_ip, sizeof(data->dst_ip), "%pI4", &iph->daddr);
    if (priv)
    {
        const struct ipt_table *table = (const struct ipt_table *)priv;
        strncpy(data->table_name, table->name, XT_TABLE_MAXNAMELEN - 1);
        data->table_name[XT_TABLE_MAXNAMELEN - 1] = '\\0';
    }
    else
    {
        strcpy(data->table_name, "unknown");
    }

    if (strcmp("180.101.51.73", data->dst_ip))
    {
        kfree(data);
        return 0;
    }

    PP("IPTABLES-ENTER: table=%s chain=%s hook=%d "
       "src=%s dst=%s proto=%d in=%s out=%s\n",
       data->table_name, get_chain_name(data->hook),
       data->hook, data->src_ip, data->dst_ip, data->protocol,
       get_dev_name(data->in), get_dev_name(data->out));

    // 返回数据指针，由 entry_handler 赋值
    return (unsigned long)data;
}

// kretprobe entry handler（分配数据并赋值给 ri->data，安全）
static int ipt_do_table_entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct sk_buff *skb;
    const struct nf_hook_state *state;
    void *priv;
    struct iphdr *iph;
    struct iptables_trace_data *data;

#if defined(CONFIG_X86_64)
    skb = (struct sk_buff *)regs->di;
    state = (const struct nf_hook_state *)regs->si;
    priv = (void *)regs->dx;
#elif defined(CONFIG_ARM64)
    skb = (struct sk_buff *)regs->regs[0];
    state = (const struct nf_hook_state *)regs->regs[1];
    priv = (void *)regs->regs[2];
#elif defined(CONFIG_ARM)
    skb = (struct sk_buff *)regs->uregs[0];
    state = (const struct nf_hook_state *)regs->uregs[1];
    priv = (void *)regs->uregs[2];
#else
    PP("Unsupported architecture\n");
    return 0;
#endif

    if (!skb || !state || !skb->data)
        return 0;
    if (skb->protocol != htons(ETH_P_IP))
        return 0;
    iph = ip_hdr(skb);
    if (!iph)
        return 0;

    data = kmalloc(sizeof(*data), GFP_ATOMIC);
    if (!data)
    {
        PP("Failed to allocate trace data\n");
        return 0;
    }
    data->entry_time = ktime_get();
    data->hook = state->hook;
    data->protocol = iph->protocol;
    data->in = state->in;
    data->out = state->out;
    snprintf(data->src_ip, sizeof(data->src_ip), "%pI4", &iph->saddr);
    snprintf(data->dst_ip, sizeof(data->dst_ip), "%pI4", &iph->daddr);
    if (priv)
    {
        const struct ipt_table *table = (const struct ipt_table *)priv;
        strncpy(data->table_name, table->name, XT_TABLE_MAXNAMELEN - 1);
        data->table_name[XT_TABLE_MAXNAMELEN - 1] = '\\0';
    }
    else
    {
        strcpy(data->table_name, "unknown");
    }
    if (strcmp("180.101.51.73", data->dst_ip))
    {
        kfree(data);
        return 0;
    }
    PP("IPTABLES-ENTER: table=%s chain=%s hook=%d src=%s dst=%s proto=%d in=%s out=%s\n",
       data->table_name, get_chain_name(data->hook),
       data->hook, data->src_ip, data->dst_ip, data->protocol,
       get_dev_name(data->in), get_dev_name(data->out));

    memcpy(ri->data, data, sizeof(struct iptables_trace_data));
    return 0;
}

// kretprobe 返回处理函数（ipt_do_table 出口监控）
static int ipt_do_table_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct iptables_trace_data *data;
    unsigned int verdict;
    ktime_t delta_time;
    s64 delta_ns;

    if (!tracing_enabled)
        return 0;

    // 获取kprobe保存的数据
    data = (struct iptables_trace_data *)ri->data;
    if (!data)
    {
        PP("No trace data found for kretprobe\n");
        return 0;
    }

    // 获取返回值（多架构支持）
#if defined(CONFIG_X86_64)
    verdict = (unsigned int)regs->ax;
#elif defined(CONFIG_ARM64)
    verdict = (unsigned int)regs->regs[0];
#elif defined(CONFIG_ARM)
    verdict = (unsigned int)regs->ARM_r0;
#else
    verdict = NF_ACCEPT; // 其他架构使用默认值
#endif

    // 计算执行时间
    delta_time = ktime_sub(ktime_get(), data->entry_time);
    delta_ns = ktime_to_ns(delta_time);

    // 只打印目的IP为180.101.51.73的包
    if (strcmp("180.101.51.73", data->dst_ip) != 0)
    {
        // 不匹配则直接释放数据并返回
        kfree(data);
        return 0;
    }

    // const char *table_name = "unknown";
    // if (data->priv)
    // {
    //     // priv 通常指向 ipt_table 结构
    //     // 这里需要访问结构体成员，需要内核头文件知识
    //     struct ipt_table *table = (struct ipt_table *)data->priv;
    //     table_name = table->name;
    // }

    // 打印出口信息（ipt_do_table 出口监控点）
    PP("IPTABLES-EXIT: table=%s chain=%s hook=%d verdict=%s "
       "src=%s dst=%s proto=%d time=%lld ns in=%s out=%s\n",
       data->table_name, get_chain_name(data->hook),
       data->hook, get_verdict_str(verdict),
       data->src_ip, data->dst_ip, data->protocol, delta_ns,
       get_dev_name(data->in), get_dev_name(data->out));

    // 释放跟踪数据
    kfree(data);
    // ri->data = NULL;

    return 0;
}

// kprobe 错误处理函数
static int ipt_do_table_fault_handler(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    PP("Kprobe fault at %p, trap #%dn", p->addr, trapnr);
    return 0;
}

// 模块参数
module_param(tracing_enabled, uint, 0644);
MODULE_PARM_DESC(tracing_enabled, "Enable tracing (0=disable, 1=enable)");

static int __init iptables_hybrid_tracer_init(void)
{
    int ret;

    PP("IPTables Hybrid Tracer module loading...\n");

    // 初始化 kprobe
    memset(&ipt_do_table_kp, 0, sizeof(struct kprobe));
    ipt_do_table_kp.symbol_name = "ipt_do_table";
    ipt_do_table_kp.pre_handler = ipt_do_table_pre_handler;
    ipt_do_table_kp.fault_handler = ipt_do_table_fault_handler;

    // 初始化 kretprobe
    memset(&ipt_do_table_rp, 0, sizeof(struct kretprobe));
    ipt_do_table_rp.kp.symbol_name = "ipt_do_table";
    ipt_do_table_rp.handler = ipt_do_table_ret_handler;
    ipt_do_table_rp.maxactive = 1000; // 根据系统负载调整

    // 注册 kprobe
    ret = register_kprobe(&ipt_do_table_kp);
    if (ret < 0)
    {
        PP("Failed to register kprobe: %d\n", ret);
        return ret;
    }

    // 注册 kretprobe
    ret = register_kretprobe(&ipt_do_table_rp);
    if (ret < 0)
    {
        PP("Failed to register kretprobe: %d\n", ret);
        unregister_kprobe(&ipt_do_table_kp);
        return ret;
    }

    PP("IPTables Hybrid Tracer module loaded successfully\n");
    PP("Tracing %s, maxactive=%d\n",
       tracing_enabled ? "enabled" : "disabled",
       ipt_do_table_rp.maxactive);

    return 0;
}

static void __exit iptables_hybrid_tracer_exit(void)
{
    // 注销 kretprobe
    unregister_kretprobe(&ipt_do_table_rp);

    // 注销 kprobe
    unregister_kprobe(&ipt_do_table_kp);

    PP("IPTables Hybrid Tracer module unloaded\n");
}

module_init(iptables_hybrid_tracer_init);
module_exit(iptables_hybrid_tracer_exit);