/*
 * @*************************************:
 * @FilePath     : /user/C/time/hash_iptables.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-19 17:06:46
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-21 10:49:39
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

enum
{
    IPT_NULL = 0,
    /*filter*/
    /*standard chain*/
    IPT_FLT_INPUT,
    IPT_FLT_FORWARD,
    IPT_FLT_OUTPUT,

    /*customer chain*/
    IPT_FLT_TEST,
    IPT_FLT_SERV_WHITELIST,
    IPT_FLT_SERV_WAN_ACCEPT_TR069,
    IPT_FLT_SERV_WAN_ACCEPT_VOICE,
    IPT_FLT_SERV_WAN_ACCEPT_VXLAN,
    IPT_FLT_SERV_LAN_BLACKLIST,
    IPT_FLT_SERV_WAN_BLACKLIST,
    IPT_FLT_LAN_NES_FILTER,
    IPT_FLT_WAN_NES_FILTER,
    IPT_FLT_BOUND_IN,
    IPT_FLT_BOUND_FWD,
    IPT_FLT_MINIUPNPD,
    IPT_FLT_IPT_FIREWALL_WHITE,
    IPT_FLT_IPT_FIREWALL_WHITE_LAN,
    IPT_FLT_FWD_FIREWALL_WHITE,
    IPT_FLT_IPT_FIREWALL_BLACK,
    IPT_FLT_FWD_FIREWALL_BLACK,
    IPT_FLT_IPT_FIREWALL_SAFEDOS,
    IPT_FLT_FWD_FIREWALL_SAFEDOS,
    IPT_FLT_HG_SAFEDOS_DDOSTCP,
    IPT_FLT_FWD_DEFAULT_BLACKLIST,
    IPT_FLT_IPT_SERVICE_WHITELIST,
    IPT_FLT_FWD_SERVICE_WHITELIST,
    IPT_FLT_IPT_SERVICE_BLACKLIST,
    IPT_FLT_FWD_SERVICE_BLACKLIST,
    IPT_FLT_FWD_DMZ,
    IPT_FLT_HG_VIRTUAL_SERVER,
    IPT_FLT_HG_IN_NES_FILTER,
    IPT_FLT_IPT_OPT_INCOMING_FILTER,
    IPT_FLT_IPT_REMOTE_MNG,
    IPT_FLT_IPT_REMOTE_MNG_WEB,
    IPT_FLT_IPT_INPUT_WAN,

    /*nat*/
    /*standard chain*/
    IPT_NAT_PREROUTING,
    IPT_NAT_POSTROUTING,
    IPT_NAT_OUTPUT,

    /*customer chain*/
    IPT_NAT_PREROUTING_LOCAL_NET,
    IPT_NAT_PREROUTING_DMZ,
    IPT_NAT_PREROUTING_DNS_DNAT,
    IPT_NAT_POST_SNAT,
    IPT_NAT_MINIUPNPD,

    /*mangle*/
    /*standard chain*/
    IPT_MANGLE_PREROUTING,
    IPT_MANGLE_INPUT,
    IPT_MANGLE_FORWARD,
    IPT_MANGLE_OUTPUT,
    IPT_MANGLE_POSTROUTING,

    /*customer chain*/
    IPT_MANGLE_POSTROUTING_QOS_DSCP,
    IPT_MGL_PRE_SAFE_DOS_DEFAULT,
    IPT_MGL_FWD_ALG_FORWARD,
    IPT_MGL_FWD_URL_FILTER,
    IPT_MGL_IPT_FWD_DNS_FILTER,

    /*raw*/
    /*standard chain*/
    IPT_RAW_PREROUTING,
    IPT_RAW_OUTPUT,

    /*customer chain*/
    IPT_RAW_PRE_ALG,

    /*ipv6 filter*/
    /*standard chain*/
    IPT6_FLT_INPUT,
    IPT6_FLT_FORWARD,
    IPT6_FLT_OUTPUT,

    /*customer chain*/
    IPT6_FLT_SERV_WHITELIST,
    IPT6_FLT_SERV_WAN_ACCEPT_TR069,
    IPT6_FLT_SERV_WAN_ACCEPT_VOICE,
    IPT6_FLT_SERV_WAN_ACCEPT_VXLAN,
    IPT6_FLT_SERV_LAN_BLACKLIST,
    IPT6_FLT_SERV_WAN_BLACKLIST,
    IPT6_FLT_LAN_NES_FILTER,
    IPT6_FLT_WAN_NES_FILTER,
    IPT6_FLT_BOUND_IN,
    IPT6_FLT_BOUND_FWD,
    IPT6_FLT_MINIUPNPD,
    IPT6_FLT_IPT_FIREWALL_WHITE,
    IPT6_FLT_IPT_FIREWALL_WHITE_LAN,
    IPT6_FLT_FWD_FIREWALL_WHITE,
    IPT6_FLT_IPT_FIREWALL_BLACK,
    IPT6_FLT_FWD_FIREWALL_BLACK,
    IPT6_FLT_IPT_FIREWALL_SAFEDOS,
    IPT6_FLT_FWD_FIREWALL_SAFEDOS,
    IPT6_FLT_HG_SAFEDOS_DDOSTCP,
    IPT6_FLT_FWD_DEFAULT_BLACKLIST,
    IPT6_FLT_IPT_SERVICE_WHITELIST,
    IPT6_FLT_FWD_SERVICE_WHITELIST,
    IPT6_FLT_IPT_SERVICE_BLACKLIST,
    IPT6_FLT_FWD_SERVICE_BLACKLIST,
    IPT6_FLT_FWD_DMZ,
    IPT6_FLT_FWD_IPV6_SESSION,
    IPT6_FLT_HG_VIRTUAL_SERVER,
    IPT6_FLT_HG_IN_NES_FILTER,
    IPT6_FLT_IPT_OPT_INCOMING_FILTER,
    IPT6_FLT_IPT_INPUT_WAN,

    /*ipv6 mangle*/
    /*standard chain*/
    IPT6_MANGLE_PREROUTING,
    IPT6_MANGLE_INPUT,
    IPT6_MANGLE_FORWARD,
    IPT6_MANGLE_OUTPUT,
    IPT6_MANGLE_POSTROUTING,

    /*customer chain*/
    IPT6_MGL_PRE_SAFE_DOS_DEFAULT,
    IPT6_MGL_FWD_ALG_FORWARD,
    IPT6_MGL_FWD_URL_FILTER,
    IPT6_MGL_IPT_FWD_DNS_FILTER,

    /*ipv6 NAT*/
    /*standard chain*/
    IPT6_NAT_PREROUTING,
    IPT6_NAT_INPUT,
    IPT6_NAT_OUTPUT,
    IPT6_NAT_POSTROUTING,

    /*customer chain*/
    IPT6_NAT_PREROUTING_DNS_DNAT,

    /*ipv6 raw*/
    /*standard chain*/
    IPT6_RAW_PREROUTING,
    IPT6_RAW_OUTPUT,

    /*customer chain*/
    IPT6_RAW_PRE_ALG,

    /*mark the end of iptables*/
    IPT_END
} IPTABLES_CHAIN;

void IPT_FLT_INPUT_fun(void *handle)
{
    PP();
}

void IPT_FLT_FORWARD_fun(void *handle)
{
    PP();
}

void IPT_FLT_OUTPUT_fun(void *handle)
{
    PP();
}

void IPT_FLT_TEST_fun(void *handle)
{
    PP();
}

void IPT_FLT_SERV_WHITELIST_fun(void *handle)
{
    PP();
}

void IPT_FLT_SERV_WAN_ACCEPT_TR069_fun(void *handle)
{
    PP();
}

void IPT_FLT_SERV_WAN_ACCEPT_VOICE_fun(void *handle)
{
    PP();
}

void IPT_FLT_SERV_WAN_ACCEPT_VXLAN_fun(void *handle)
{
    PP();
}

void IPT_FLT_SERV_LAN_BLACKLIST_fun(void *handle)
{
    PP();
}

void IPT_FLT_SERV_WAN_BLACKLIST_fun(void *handle)
{
    PP();
}

void IPT_FLT_LAN_NES_FILTER_fun(void *handle)
{
    PP();
}

void IPT_FLT_WAN_NES_FILTER_fun(void *handle)
{
    PP();
}

void IPT_FLT_BOUND_IN_fun(void *handle)
{
    PP();
}

void IPT_FLT_BOUND_FWD_fun(void *handle)
{
    PP();
}

void IPT_FLT_MINIUPNPD_fun(void *handle)
{
    PP();
}

typedef void (*FuncAddChainRules)(void *handle);

typedef struct
{
    int chain;
    FuncAddChainRules add_chain_rules;
} _ipt_emu;

_ipt_emu dof_ipt_table[IPT_END] = {0};

#define g_register_init(id, fun)                           \
    do                                                     \
    {                                                      \
        _ipt_emu *ipt_tmp = &dof_ipt_table[id];            \
        ipt_tmp->chain = id;                               \
        ipt_tmp->add_chain_rules = (FuncAddChainRules)fun; \
    } while (0)

void g_init()
{
    g_register_init(IPT_FLT_INPUT, IPT_FLT_INPUT_fun);
    g_register_init(IPT_FLT_FORWARD, IPT_FLT_FORWARD_fun);
    g_register_init(IPT_FLT_OUTPUT, IPT_FLT_OUTPUT_fun);
    g_register_init(IPT_FLT_TEST, IPT_FLT_TEST_fun);
    g_register_init(IPT_FLT_SERV_WHITELIST, IPT_FLT_SERV_WHITELIST_fun);
    g_register_init(IPT_FLT_SERV_WAN_ACCEPT_TR069, IPT_FLT_SERV_WAN_ACCEPT_TR069_fun);
    g_register_init(IPT_FLT_SERV_WAN_ACCEPT_VOICE, IPT_FLT_SERV_WAN_ACCEPT_VOICE_fun);
    g_register_init(IPT_FLT_SERV_WAN_ACCEPT_VXLAN, IPT_FLT_SERV_WAN_ACCEPT_VXLAN_fun);
    g_register_init(IPT_FLT_SERV_LAN_BLACKLIST, IPT_FLT_SERV_LAN_BLACKLIST_fun);
    g_register_init(IPT_FLT_SERV_WAN_BLACKLIST, IPT_FLT_SERV_WAN_BLACKLIST_fun);
    g_register_init(IPT_FLT_LAN_NES_FILTER, IPT_FLT_LAN_NES_FILTER_fun);
    g_register_init(IPT_FLT_WAN_NES_FILTER, IPT_FLT_WAN_NES_FILTER_fun);
    g_register_init(IPT_FLT_BOUND_IN, IPT_FLT_BOUND_IN_fun);
    g_register_init(IPT_FLT_BOUND_FWD, IPT_FLT_BOUND_FWD_fun);
    g_register_init(IPT_FLT_MINIUPNPD, IPT_FLT_MINIUPNPD_fun);
}

void hash_search(int chain, void *handle)
{
    printf("hash_search %d\n", chain);
    _ipt_emu *tmp = &dof_ipt_table[chain];
    if (tmp->chain == chain && NULL != tmp->add_chain_rules)
    {
        printf("%d\n", tmp->chain);
        tmp->add_chain_rules(handle);
    }
}

int ipt[IPT_END] = {0};

int hash_set(int id)
{
    ipt[id] = 1;
}

int main(int argc, char const *argv[])
{
    printf("hello world\n");
    void *handle = "sss";
    g_init();

    hash_set(IPT_FLT_SERV_WAN_ACCEPT_TR069);
    hash_set(IPT_FLT_LAN_NES_FILTER);

    for (int i = 0; i < IPT_END; i++)
    {
        if (ipt[i])
        {
            hash_search(IPT_FLT_SERV_WAN_ACCEPT_TR069, handle);
        }
    }
    return 0;
}
