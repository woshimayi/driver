// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/ktime.h>

#undef PP
#define PP(fmt, ...) printk("\033[0;32m[ipt_do_chain_trace:%s(%d)] " fmt "\033[0m\r\n", __func__, __LINE__, ##__VA_ARGS__)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xt_do_chain Tracer");
MODULE_DESCRIPTION("Trace xt_do_chain chain name using kprobes");

static struct kprobe ipt_do_chain_kp;

static int ipt_do_chain_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    void *chain = NULL;
    const char *chain_name = "unknown";
#if defined(CONFIG_X86_64)
    chain = (void *)regs->di;
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    chain = (void *)regs->uregs[0];
#else
    return 0;
#endif
    // ipt_do_chain第一个参数通常是struct ipt_chainstack * 或 struct ipt_entry *
    // 这里只能打印指针，无法直接获取链名
    PP("ipt_do_chain called, chain ptr=%p\n", chain);
    return 0;
}

static int __init ipt_do_chain_tracer_init(void)
{
    int ret;
    PP("ipt_do_chain Tracer module loading...\n");
    memset(&ipt_do_chain_kp, 0, sizeof(struct kprobe));
    ipt_do_chain_kp.symbol_name = "ipt_do_chain";
    ipt_do_chain_kp.pre_handler = ipt_do_chain_pre_handler;
    ret = register_kprobe(&ipt_do_chain_kp);
    if (ret < 0)
    {
        PP("Failed to register kprobe: %d\n", ret);
        return ret;
    }
    PP("ipt_do_chain Tracer module loaded successfully\n");
    return 0;
}

static void __exit ipt_do_chain_tracer_exit(void)
{
    unregister_kprobe(&ipt_do_chain_kp);
    PP("ipt_do_chain Tracer module unloaded\n");
}

module_init(ipt_do_chain_tracer_init);
module_exit(ipt_do_chain_tracer_exit);
