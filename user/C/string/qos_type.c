#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>


#define hi_char8 char
#define hi_uint32 unsigned int
#define hi_int32 int
#define uint32_t unsigned int
#define bool int
#define false 0
#define true 1

#if 0
#define HI_QOS_ERR(fmt, arg...) \
    printf("[QOS_ERROR:%s(%d)]" fmt, __func__, __LINE__, ##arg)

#define HI_QOS_RET_CHECK(ret)                                                                 \
    do                                                                                        \
    {                                                                                         \
        if (ret)                                                                              \
        {                                                                                     \
            printf("\r\n error=0x%08x function=%s line=%d\r\n", ret, __FUNCTION__, __LINE__); \
            return ret;                                                                       \
        }                                                                                     \
    } while (0)

#define HI_QOS_SAFE_FUNC_RET_CHECK(ret_s)                                                                \
    do                                                                                                   \
    {                                                                                                    \
        if ((ret_s) != 0)                                                                                \
        {                                                                                                \
            printf("\r\nsafe func ret_s=0x%x, function=%s, line=%d\r\n", ret_s, __FUNCTION__, __LINE__); \
            return HI_RET_FAIL;                                                                          \
        }                                                                                                \
    } while (0)

#define PP(ret)                                                            \
    do                                                                                               \
    {                                                                                                \
        if (ret == -1)                                                                               \
        {                                                                                            \
            printf("\r\nsprintf_s ret=0x%x, function=%s, line=%d\r\n", ret, __FUNCTION__, __LINE__); \
            return HI_RET_FAIL;                                                                      \
        }                                                                                            \
    } while (0)

enum hi_netif_proto_type
{
    HI_NETIF_PROTO_TYPE_ARP = 0x1,
    HI_NETIF_PROTO_TYPE_ICMP,
    HI_NETIF_PROTO_TYPE_ICMP_V6,
    HI_NETIF_PROTO_TYPE_DHCP,
    HI_NETIF_PROTO_TYPE_DHCP_V6,
    HI_NETIF_PROTO_TYPE_IGMP,
    HI_NETIF_PROTO_TYPE_MLD,
    HI_NETIF_PROTO_TYPE_PPPOE,
    HI_NETIF_PROTO_TYPE_BC,
    HI_NETIF_PROTO_TYPE_ALL,
    HI_NETIF_PROTO_TYPE_ETYPE,
    HI_NETIF_PROTO_TYPE_UUC,
    HI_NETIF_PROTO_TYPE_UMC,
    HI_NETIF_PROTO_TYPE_TCP_SYN,
    HI_NETIF_PROTO_TYPE_PPPOE_DISCOVERY,
    HI_NETIF_PROTO_TYPE_PPP_LCP,
    HI_NETIF_PROTO_TYPE_UDP,
    HI_NETIF_PROTO_TYPE_BUTT
};

typedef enum
{
    HI_QOS_FLOW_TYPE_SMAC = 1,  /**< SMAC匹配 */
    HI_QOS_FLOW_TYPE_DMAC,      /**< DMAC匹配 */
    HI_QOS_FLOW_TYPE_SIP,       /**< SIP匹配 */
    HI_QOS_FLOW_TYPE_DIP,       /**< DIP匹配 */
    HI_QOS_FLOW_TYPE_SIPV6,     /**< SIPV6匹配 */
    HI_QOS_FLOW_TYPE_DIPV6,     /**< DIPV6匹配 */
    HI_QOS_FLOW_TYPE_SPORT,     /**< SPORT匹配 */
    HI_QOS_FLOW_TYPE_DPORT,     /**< DPORT匹配 */
    HI_QOS_FLOW_TYPE_8021P,     /**< 802.1P匹配 */
    HI_QOS_FLOW_TYPE_TOS,       /**< TOS匹配 */
    HI_QOS_FLOW_TYPE_DSCP,      /**< DSCP匹配 */
    HI_QOS_FLOW_TYPE_WANINTF,   /**< WANINTF匹配 */
    HI_QOS_FLOW_TYPE_LANINTF,   /**< LANINTF匹配 */
    HI_QOS_FLOW_TYPE_TC,        /**< TC匹配 */
    HI_QOS_FLOW_TYPE_FL,        /**< FL匹配 */
    HI_QOS_FLOW_TYPE_VLAN,      /**< VLAN匹配 */
    HI_QOS_FLOW_TYPE_IPVERSION, /**< IPVERSION匹配 */
    HI_QOS_FLOW_TYPE_DROP,      /**< DROP */
    HI_QOS_FLOW_TYPE_DEFAULT,   /* 协议匹配 */
} hi_qos_flow_type_e;

typedef enum
{
    HI_COMMON_PROTO_TCP = 0x1,
    HI_COMMON_PROTO_UDP = 0x2,
    HI_COMMON_PROTO_ICMP = 0x4,
    HI_COMMON_PROTO_RTP = 0x8,
    HI_COMMON_PROTO_ICMPV6 = 0x10,
} hi_common_proto_e;

enum hi_err
{
    HI_RET_SUCC = 0,                        /* Success */
    HI_RET_FAIL = 0x70000000,               /* Failure */
    HI_RET_BUSY = (HI_RET_FAIL | 0x80),     /* Busy */
    HI_RET_FULL = (HI_RET_FAIL | 0x81),     /* Full */
    HI_RET_TMO = (HI_RET_FAIL | 0x82),      /* Timeout */
    HI_RET_INV_PARM = (HI_RET_FAIL | 0x83), /* Invalid parameter */
    HI_RET_NULL_PTR = (HI_RET_FAIL | 0x84), /* Null pointer */
};

/**< QOS 优先级标记 */
typedef enum
{
    HI_QOS_MARK_OFF = 0,  /**< 不启用 */
    HI_QOS_MARK_ORIGINAL, /**< 保持原有值 */
    HI_QOS_MARK_EDIT,     /**< 修改为指定值 */
    HI_QOS_MARK_SOURCE,   /**< 根据DSCP/802.1p标记 */
} hi_qos_mark_e;

typedef enum
{
    HI_QOS_DISABLED = 0, /**< disabled */
    HI_QOS_ENABLED,      /**< enabled */
    HI_QOS_ERROR,        /**< error */
} hi_qos_status_e;

typedef struct
{
    hi_qos_flow_type_e em_type; /**< 类型 */
    hi_char8 uc_min[64];        /**< 最小值字符串输入(for ip, wanif,and mac: "192.168.1.1") */
    hi_char8 uc_max[64];        /**< 最大值字符串输入(for ip, wanif and mac: "192.168.1.1") */
    hi_uint32 ui_min;           /**< 最小值(for others) */
    hi_uint32 ui_max;           /**< 最大值(for others) */
    hi_uint32 ui_exclude;       /**< 是否使用非匹配 */
    hi_uint32 ui_proto_list;    /**< 选择协议类型，按hi_common_proto_e 定义bit位掩码 */
    hi_uint32 ui_pexclude;      /**< 协议类型是否使用非匹配 */
} hi_qos_flow_class_s;

typedef struct
{
    hi_uint32 ui_key;               /**< 流分类索引 */
    hi_uint32 ui_enable;            /**< 使能该流分类规则 */
    hi_qos_status_e em_status;      /**< 状态 */
    hi_uint32 ui_pri;               /**< 该流分类的优先级 */
    hi_qos_mark_e em_dscp_mark;     /**< 是否启用DSCP重标记 */
    hi_uint32 ui_dscp;              /**< DSCP标记值 0-63 */
    hi_qos_mark_e em_tc_mark;       /**< 是否启用TC重标记 */
    hi_uint32 ui_tc;                /**< TC标记值 0-255 */
    hi_qos_mark_e em_8021p_mark;    /**< 8021p重标记方式 */
    hi_uint32 ui_8021p;             /**< 802.1p标记值 0-7 */
    hi_uint32 ui_forwarding_policy; /**< 转发规则的索引号 */
    hi_uint32 ui_class_policer;     /**< 流量限速的索引号(0xff表示丢弃) */
    hi_uint32 ui_queue;             /**< 队列的索引号 */
    hi_uint32 queue_enable;
    hi_qos_flow_class_s st_flow_class[4]; /**< 流分类匹配项，0~3顺序使用，未使用的请置为全0 */
    hi_uint32 ui_qos_id_v4;
    hi_uint32 ui_qos_id_v6;
} hi_qos_flow_queue_s;

hi_int32 hi_os_execcmd(hi_char8 *pc_command)
{
    hi_int32 pid = 0;
    hi_int32 status = 0;
    hi_char8 *argv[] = {"sh", "-c", pc_command, NULL};

    if (pc_command == NULL)
    {
        return -HI_RET_FAIL;
    }

    pid = fork();
    if (pid < 0)
    {
        return -HI_RET_FAIL;
    }
    else if (pid == 0)
    {
        // 子进程执行分支
        execv("/bin/sh", argv);
        exit(127);
    }

    /* wait for child process return */
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    return WIFEXITED(status) ? (HI_RET_SUCC) : (-HI_RET_FAIL);
}

/*****************************************************************************
 函 数 名  : __qos_flow_rule_ebtables_exec
 功能描述  : ebtables规则下发
 输入参数  : hi_char8 *puc_cmd
 输出参数  : 无
 返 回 值  : hi_int32
*****************************************************************************/
hi_int32 __qos_flow_rule_ebtables_exec(hi_char8 *puc_cmd, hi_int32 cmd_size)
{
    hi_int32 ret = HI_RET_FAIL;
    hi_char8 cmd_origin[256] = {0};
    hi_char8 cmd[256] = {0};
    hi_char8 cmd_tmp[256] = {0};
    hi_char8 cfe_act[16] = {0};
    hi_char8 *start = NULL;
    hi_char8 *end = NULL;

    if (puc_cmd == NULL)
    {
        HI_QOS_ERR("NULL pointer\n");
        return HI_RET_FAIL;
    }

    strncpy(cmd_origin, puc_cmd, strlen(puc_cmd));
    puc_cmd = cmd_origin;

    end = strstr(puc_cmd, "cfe");
    if (end == NULL)
    {
        HI_QOS_ERR("wrong ebtables cmd\n");
        return HI_RET_FAIL;
    }
    end += (sizeof("cfe") - 1);
    strncpy(cmd_tmp, puc_cmd, (end - puc_cmd));

    /* ebtables qos only have cfe target */
    start = strstr(end, "-j ");
    if (start != NULL)
    {
        *start = '\0';
    }

    start = strstr(end, "--");
    if (start == NULL)
    {
        HI_QOS_ERR("wrong ebtables cmd\n");
        return HI_RET_FAIL;
    }
    end = strstr(start + (sizeof("--") - 1), "--");
    while (end != NULL)
    {
        strncpy(cfe_act, start, (end - start));
        snprintf(cmd, sizeof(cmd), "%s %s", cmd_tmp, cfe_act);
        printf("cmd(%s)\n", cmd);
        /* ebtables不支持set dscp */
        if (!strstr(cmd, "set-dscp"))
        {
            ret = hi_os_execcmd(cmd);
        }
        if (ret != 0)
        {
            HI_QOS_ERR("cmd(%s) RET:%x\n", cmd, ret);
        }

        start = end;
        end = strstr(start + (sizeof("--") - 1), "--");
    }

    strncpy(cfe_act, start, (puc_cmd + strlen(puc_cmd) - start));
    snprintf(cmd, sizeof(cmd), "%s %s", cmd_tmp, cfe_act);
    printf("cmd(%s)\n", cmd);
    /* ebtables不支持set dscp */
    if (!strstr(cmd, "set-dscp"))
    {
        ret = hi_os_execcmd(cmd);
    }
    if (ret != 0)
    {
        HI_QOS_ERR("cmd(%s) RET:%x\n", cmd, ret);
    }
    return HI_RET_SUCC;
}

/*****************************************************************************
 函 数 名  : __qos_flow_rule_iptables_exec
 功能描述  : iptables规则下发，同时下发ipv4和ipv6，注意一条规则只能有一个动
             作 -j 选项
 输入参数  : hi_char8 *puc_cmd
 输出参数  : 无
 返 回 值  : hi_int32
*****************************************************************************/
hi_int32 __qos_flow_rule_iptables_exec(hi_char8 *puc_cmd, hi_int32 type)
{
    int ret_s;
    hi_int32 ret = 0;
    hi_char8 auc_match[256] = {0};
    hi_char8 auc_act[256] = {0};
    hi_char8 auc_cmd[256] = {0};
    hi_uint32 ui_cnt = 0;
    hi_uint32 ui_len = 0;
    hi_uint32 ui_offset = 0;
    hi_char8 *puc_tmp = puc_cmd;
    hi_char8 *puc_pre = puc_cmd;

    /* 循环检查iptables命令里有几个动作(-j)，每个动作执行一次iptables配置命令 */
    printf("cmd:%s\n", puc_cmd);
    do
    {
        puc_tmp = strstr(puc_tmp, "-j");
        if (puc_tmp != NULL)
        {
            ui_len = puc_tmp - puc_pre;
        }
        else
        {
            ui_len = puc_cmd + strlen(puc_cmd) - puc_pre;
        }

        /* 第一个-j拷贝match部分，后续拷贝动作同时生成命令并执行 */
        if (!ui_cnt)
        {
            strncpy(auc_match, puc_cmd + ui_offset, ui_len);
        }
        else
        {
            strncpy(auc_act, puc_cmd + ui_offset, ui_len);

            if (type == 4)
            {
                ret = snprintf(auc_cmd, sizeof(auc_cmd), "iptables ");
                PP(ret);
            }
            else if (type == 6)
            {
                ret = snprintf(auc_cmd, sizeof(auc_cmd), "ip6tables ");
                PP(ret);
            }
            else
            {
                printf("invalid type=%d\n", type);
                return -1;
            }

            ret_s = strcat(auc_cmd, auc_match);
            HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
            ret_s = strcat(auc_cmd, auc_act);
            HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
            printf("Exec:%s \r\n", auc_cmd);
            ret = hi_os_execcmd(auc_cmd);
            HI_QOS_RET_CHECK(ret);
        }

        ui_cnt++;
        ui_offset += ui_len;
        puc_pre = puc_tmp;
        if (puc_tmp != 0)
        {
            puc_tmp += 2;
        }
    } while (puc_pre !=
             NULL); // 以puc_pre来判断查找字符串结束，puc_tmp每次会偏移2个字节以跳过命中的-j

    return HI_RET_SUCC;
}

/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 此函数仅支持一个字符串参数形式
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_common1(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    ret = sprintf(key, key_size, str_format, min);
    PP(ret);

    return HI_RET_SUCC;
}

static hi_uint32 get_key_port(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    hi_int32 ret = 0;
    hi_char8 str[64] = {0};

    if (strcmp(min, max) != 0)
    {
        ret = sprintf(str, sizeof(str), "%s:%s", min, max);
        PP(ret);
        ret = sprintf(key, key_size, str_format, str);
        PP(ret);
    }
    else
    {
        ret = sprintf(key, key_size, str_format, min);
        PP(ret);
    }

    return HI_RET_SUCC;
}

/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 此函数仅支持一个整形参数形式
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_int_common1(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key,
                                     hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    ret = sprintf(key, key_size, str_format, strtoul(min, NULL, 0));
    PP(ret);

    return HI_RET_SUCC;
}
/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 此函数仅支持两个字符串参数形式
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_common2(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    hi_int32 ret = sprintf(key, key_size, str_format, min, max);
    PP(ret);
    return HI_RET_SUCC;
}
/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: tos匹配需要移位，这里需要特殊处理
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_ipt_tos(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    ret = sprintf(key, key_size, str_format, strtoul(min, NULL, 0));
    PP(ret);

    return HI_RET_SUCC;
}

/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: dscp匹配需要移位，这里需要特殊处理
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_ipt_dscp(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    ret = sprintf(key, key_size, str_format, strtoul(min, NULL, 0) << 2);
    PP(ret);

    return HI_RET_SUCC;
}

static hi_uint32 get_key_ipt_tc(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    ret = sprintf(key, key_size, str_format, strtoul(min, NULL, 0));
    PP(ret);

    return HI_RET_SUCC;
}

/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 根据lan名称构造lan设备的mark值
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_ipt_lan_intf(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key,
                                      hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    ret = sprintf(key, key_size, str_format, (strtoul(min, NULL, 0)) << 4);
    PP(ret);
    return HI_RET_SUCC;
}

/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 根据wan名称寻找wan设备的名字，然后构造规则
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_ipt_wan_intf(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key,
                                      hi_uint32 key_size)
{
    hi_int32 ret = 0;
    hi_char8 auc_tmp[64] = {0};

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }
    // ret = __qos_wanif_devname_get(min, auc_tmp, sizeof(auc_tmp));
    // if (ret)
    // {
    // 	HI_QOS_ERR("wan name = %s get dev name failed!\n", min);
    // 	return HI_RET_FAIL;
    // }
    // ret = sprintf(key, key_size, str_format, auc_tmp);
    PP(ret);

    return HI_RET_SUCC;
}
/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 根据整形形式查找lan侧设备的名字，lan1~lan4,ssid1~ssid8
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_ebt_lan_intf(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key,
                                      hi_uint32 key_size)
{
    hi_int32 ret = 0;
    hi_char8 auc_tmp[64] = {0};

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }

    // __qos_port_switch_to_dev((1 << (strtoul(min, NULL, 0) - 1)), auc_tmp, sizeof(auc_tmp));
    ret = sprintf(key, key_size, str_format, auc_tmp);
    PP(ret);
    return HI_RET_SUCC;
}

/*
 * 功能描述: 通过模板字符串str_format获取规则中的key部分
 * 注意事项: 根据4或者6，匹配校验格式
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_key_ip_version(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key,
                                    hi_uint32 key_size)
{
    hi_int32 ret = 0;

    if (strcmp(min, max) != 0)
    {
        return HI_RET_FAIL;
    }
    if ((min[0] != '4') && (min[0] != '6'))
    {
        return HI_RET_FAIL;
    }
    ret = sprintf(key, key_size, "-p ipv%s ", min);
    PP(ret);
    return HI_RET_SUCC;
}

static hi_uint32 get_key_empty(hi_char8 *min, hi_char8 *max, hi_char8 *str_format, hi_char8 *key, hi_uint32 key_size)
{
    if (strlen(str_format) > 0 || key_size == 0)
    {
        return HI_RET_FAIL;
    }

    key[0] = '\0';

    return HI_RET_SUCC;
}

hi_int32 hi_os_vcmd(const hi_char8 *format, ...)
{
    int ret;
    char buf[512] = ""; /* exec cmd max len 512 */

    va_list marker;

    va_start(marker, format);
    ret = vsnprintf(buf, sizeof(buf), format, marker);
    va_end(marker);
    if (ret >= 0)
    {
        ret = hi_os_execcmd(buf);
    }

    if (ret != HI_RET_SUCC)
    {
        printf("[ERR EXEC(%08x)]%s\n", ret, buf);
    }
    return ret;
}

/*
 * 功能描述: 字符串类型根据下发的值进行整理
 * 注意事项: 只下发一个，最大值最小值都等于这个，下发不同则赋值不同的最大值最小值
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_str_range(hi_char8 *src_max, hi_char8 *src_min, hi_char8 *dst_max, hi_int32 dst_max_len,
                               hi_char8 *dst_min, hi_int32 dst_min_len)
{
    hi_int32 ret = 0;
    if ((src_max[0] == 0) && (src_min[0] != 0))
    {
        ret = strncpy(dst_min, dst_min_len, src_min);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret);
        ret = strncpy(dst_max, dst_max_len, src_min);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret);
    }
    else if ((src_max[0] != 0) && (src_min[0] == 0))
    {
        ret = strncpy(dst_min, dst_min_len, src_max);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret);
        ret = strncpy(dst_max, dst_max_len, src_max);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret);
    }
    else if ((src_max[0] != 0) && (src_min[0] != 0))
    {
        ret = strncpy(dst_min, dst_min_len, src_min);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret);
        ret = strncpy(dst_max, dst_max_len, src_max);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret);
    }
    return HI_RET_SUCC;
}
/*
 * 功能描述: 数字类型根据下发的值进行整理，这里转换成字符串形式，方面后面统一处理
 * 注意事项: 只下发一个，最大值最小值都等于这个，下发不同则赋值不同的最大值最小值
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 get_int_range(hi_int32 src_max, hi_int32 src_min, hi_char8 *dst_max, hi_int32 dst_max_len,
                               hi_char8 *dst_min, hi_int32 dst_min_len)
{
    hi_int32 ret = 0;

    if ((src_max == 0) && (src_min != 0))
    {
        ret = sprintf(dst_min, dst_min_len, "%d", src_min);
        PP(ret);
        ret = sprintf(dst_max, dst_max_len, "%d", src_min);
        PP(ret);
    }
    else if ((src_max != 0) && (src_min == 0))
    {
        ret = sprintf(dst_min, dst_min_len, "%d", src_max);
        PP(ret);
        ret = sprintf(dst_max, dst_max_len, "%d", src_max);
        PP(ret);
    }
    else if ((src_max != 0) && (src_min != 0))
    {
        ret = sprintf(dst_min, dst_min_len, "%d", src_min);
        PP(ret);
        ret = sprintf(dst_max, dst_max_len, "%d", src_max);
        PP(ret);
    }
    return HI_RET_SUCC;
}
/*
 * 功能描述: 路由协议的规则最终匹配，同时支持v4
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 ipt_act_v4_common(hi_qos_flow_queue_s *pst_flow, hi_char8 *key, hi_char8 *act_v4, hi_char8 *act_v6)
{
    char chain_name[64] = {0};
    hi_char8 auc_cmd_chain[256] = {0};
    hi_int32 ret = 0;
    hi_char8 auc_cmd[256] = {0};
    ret = sprintf(chain_name, sizeof(chain_name), "HI_QOS_CHAIN_%d", pst_flow->ui_pri);
    PP(ret);
    ret = sprintf(auc_cmd_chain, sizeof(auc_cmd_chain), "-t mangle -%c %s",
                  (pst_flow->ui_enable) ? ('A') : ('D'), chain_name);
    PP(ret);
    if (pst_flow->st_flow_class[0].ui_proto_list == 0)
    {
        if (key == NULL)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p all  %s", auc_cmd_chain, act_v4);
        }
        else if (pst_flow->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_SPORT ||
                 pst_flow->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_DPORT)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p tcp %s %s", auc_cmd_chain, key, act_v4);
            PP(ret);
            ret = __qos_flow_rule_iptables_exec(auc_cmd, 4);
            HI_QOS_RET_CHECK(ret);
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p udp %s %s", auc_cmd_chain, key, act_v4);
        }
        else
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p all %s %s", auc_cmd_chain, key, act_v4);
        }
        PP(ret);
        ret = __qos_flow_rule_iptables_exec(auc_cmd, 4);
        HI_QOS_RET_CHECK(ret);
    }
    else
    {
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_TCP)
        {
            if (key == NULL)
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p tcp %s", auc_cmd_chain, act_v4);
            }
            else
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p tcp %s %s", auc_cmd_chain, key, act_v4);
            }
            PP(ret);
            ret = __qos_flow_rule_iptables_exec(auc_cmd, 4);
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_UDP)
        {
            if (key == NULL)
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p udp %s", auc_cmd_chain, act_v4);
            }
            else
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p udp %s %s", auc_cmd_chain, key, act_v4);
            }
            PP(ret);
            ret = __qos_flow_rule_iptables_exec(auc_cmd, 4);
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_ICMP)
        {
            if (pst_flow->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_SPORT &&
                pst_flow->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_DPORT)
            {
                if (key == NULL)
                {
                    ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p icmp %s", auc_cmd_chain, act_v4);
                }
                else
                {
                    ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p icmp  %s %s", auc_cmd_chain, key, act_v4);
                }
                PP(ret);
                ret = __qos_flow_rule_iptables_exec(auc_cmd, 4);
                HI_QOS_RET_CHECK(ret);
            }
        }
    }
    return HI_RET_SUCC;
}
/*
 * 功能描述: 路由协议的规则最终匹配，同时支持v6
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 ipt_act_v6_common(hi_qos_flow_queue_s *pst_flow, hi_char8 *key, hi_char8 *act_v4, hi_char8 *act_v6)
{
    char chain_name[64] = {0};
    hi_char8 auc_cmd_chain[256] = {0};
    hi_int32 ret = 0;
    hi_char8 auc_cmd[256] = {0};
    ret = sprintf(chain_name, sizeof(chain_name), "HI_QOS_CHAIN_%d", pst_flow->ui_pri);
    PP(ret);
    ret = sprintf(auc_cmd_chain, sizeof(auc_cmd_chain), "-t mangle -%c %s",
                  (pst_flow->ui_enable) ? ('A') : ('D'), chain_name);
    PP(ret);
    if (pst_flow->st_flow_class[0].ui_proto_list == 0)
    {
        if (key == NULL)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p all %s", auc_cmd_chain, act_v6);
        }
        else if (pst_flow->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_SPORT ||
                 pst_flow->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_DPORT)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p tcp %s %s", auc_cmd_chain, key, act_v6);
            PP(ret);
            ret = __qos_flow_rule_iptables_exec(auc_cmd, 6);
            HI_QOS_RET_CHECK(ret);
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p udp %s %s", auc_cmd_chain, key, act_v6);
        }
        else
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p all %s %s", auc_cmd_chain, key, act_v6);
        }
        PP(ret);
        ret = __qos_flow_rule_iptables_exec(auc_cmd, 6);
        HI_QOS_RET_CHECK(ret);
    }
    else
    {
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_TCP)
        {
            if (key == NULL)
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p tcp %s", auc_cmd_chain, act_v6);
            }
            else
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p tcp  %s %s", auc_cmd_chain, key, act_v6);
            }
            PP(ret);
            ret = __qos_flow_rule_iptables_exec(auc_cmd, 6);
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_UDP)
        {
            if (key == NULL)
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p udp %s", auc_cmd_chain, act_v6);
            }
            else
            {
                ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p udp  %s %s", auc_cmd_chain, key, act_v6);
            }
            PP(ret);
            ret = __qos_flow_rule_iptables_exec(auc_cmd, 6);
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_ICMPV6)
        {
            if (pst_flow->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_SPORT &&
                pst_flow->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_DPORT)
            {
                if (key == NULL)
                {
                    ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p icmpv6 %s", auc_cmd_chain, act_v6);
                }
                else
                {
                    ret = sprintf(auc_cmd, sizeof(auc_cmd), "%s -p icmpv6 %s %s", auc_cmd_chain, key, act_v6);
                }
                PP(ret);
                ret = __qos_flow_rule_iptables_exec(auc_cmd, 6);
                HI_QOS_RET_CHECK(ret);
            }
        }
    }
    return HI_RET_SUCC;
}
/*
 * 功能描述: 路由协议的规则最终匹配，同时支持v4 v6
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 ipt_act_all_common(hi_qos_flow_queue_s *pst_flow, hi_char8 *key, hi_char8 *act_v4, hi_char8 *act_v6)
{
    hi_int32 ret = 0;

    ret = ipt_act_v4_common(pst_flow, key, act_v4, act_v6);
    HI_QOS_RET_CHECK(ret);
    ret = ipt_act_v6_common(pst_flow, key, act_v4, act_v6);
    HI_QOS_RET_CHECK(ret);
    return HI_RET_SUCC;
}

static hi_uint32 ipt_act_ipv_protocol_act(hi_qos_flow_queue_s *pst_flow, hi_char8 *key, hi_char8 *act_v4,
                                          hi_char8 *act_v6)
{
    hi_int32 ret = 0;

    if (pst_flow->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_IPVERSION)
    {
        HI_QOS_ERR("ipt_act_ipv_protocol_act type error\n");
        return HI_RET_FAIL;
    }

    if (strstr(key, "ipv4") != NULL)
    {
        ret = ipt_act_v4_common(pst_flow, NULL, act_v4, act_v6);
        printf("ipt_act_ipv_protocol_act ipv4\n");
    }
    else if (strstr(key, "ipv6") != NULL)
    {
        ret = ipt_act_v6_common(pst_flow, NULL, act_v4, act_v6);
        printf("ipt_act_ipv_protocol_act ipv6\n");
    }
    else
    {
        HI_QOS_ERR("ipt_act_ipv_protocol_act key error\n");
        return HI_RET_FAIL;
    }

    return ret;
}

static hi_uint32 ebt_act_ipv_protocol_act(hi_qos_flow_queue_s *pst_flow, hi_char8 *key, hi_char8 *act_v4,
                                          hi_char8 *act_v6)
{
    hi_char8 *act = NULL;
    hi_int32 ret = 0;
    hi_char8 auc_cmd_chain[256] = {0};
    hi_char8 auc_cmd[256] = {0};
    char chain_name[64] = {0};
    if (strstr(key, "ipv4") != NULL)
    {
        act = act_v4;
        printf("ebt_act_ipv_protocol_act ipv4\n");
    }
    else if (strstr(key, "ipv6") != NULL)
    {
        act = act_v6;
        printf("ebt_act_ipv_protocol_act ipv6\n");
    }
    else
    {
        HI_QOS_ERR("ebt_act_ipv_protocol_act key error\n");
        return HI_RET_FAIL;
    }

    if (pst_flow->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_IPVERSION)
    {
        HI_QOS_ERR("ebt_act_ipv_protocol_act type error\n");
        return HI_RET_FAIL;
    }
    ret = sprintf(chain_name, sizeof(chain_name), "HI_QOS_CHAIN_%d", pst_flow->ui_pri);
    PP(ret);
    ret = sprintf(auc_cmd_chain, sizeof(auc_cmd_chain), "-%c %s", (pst_flow->ui_enable) ? ('A') : ('D'),
                  chain_name);
    PP(ret);

    if (pst_flow->st_flow_class[0].ui_proto_list == 0)
    {
        ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s %s", auc_cmd_chain, key, act);
        PP(ret);
        ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
        HI_QOS_RET_CHECK(ret);
    }
    else
    {
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_TCP)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s --ip-proto tcp %s", auc_cmd_chain, key, act);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_UDP)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s --ip-proto udp %s", auc_cmd_chain, key, act);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_ICMP)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s --ip-proto icmp %s", auc_cmd_chain, key, act);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_ICMPV6)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv6 --ip6-proto ipv6-icmp %s %s", auc_cmd_chain, key,
                          act_v6);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
    }
    return HI_RET_SUCC;
}

/*
 * 功能描述: 桥协议的规则最终匹配，仅下发ebtables规则
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static hi_uint32 ebt_act_common(hi_qos_flow_queue_s *pst_flow, hi_char8 *key, hi_char8 *act_v4, hi_char8 *act_v6)
{
    char chain_name[64] = {0};
    hi_char8 auc_cmd_chain[256] = {0};
    hi_int32 ret = 0;
    hi_char8 auc_cmd[256] = {0};
    ret = sprintf(chain_name, sizeof(chain_name), "HI_QOS_CHAIN_%d", pst_flow->ui_pri);
    PP(ret);
    ret = sprintf(auc_cmd_chain, sizeof(auc_cmd_chain), "-%c %s", (pst_flow->ui_enable) ? ('A') : ('D'),
                  chain_name);
    PP(ret);

    /* 已经指定协议了 */
    if (strstr(key, "-p") != NULL)
    {
        ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s %s", auc_cmd_chain, key, act_v4);
        PP(ret);
        ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
        HI_QOS_RET_CHECK(ret);
        return HI_RET_SUCC;
    }
    if (pst_flow->st_flow_class[0].ui_proto_list == 0)
    {
        ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s %s", auc_cmd_chain, key, act_v4);
        PP(ret);
        ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
        HI_QOS_RET_CHECK(ret);
    }
    else
    {
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_TCP)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv4 --ip-proto tcp %s %s", auc_cmd_chain, key, act_v4);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv6 --ip6-proto tcp %s %s", auc_cmd_chain, key, act_v6);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_UDP)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv4 --ip-proto udp %s %s", auc_cmd_chain, key, act_v4);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv6 --ip6-proto udp %s %s", auc_cmd_chain, key, act_v6);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_ICMP)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv4 --ip-proto icmp %s %s", auc_cmd_chain, key, act_v4);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
        if (pst_flow->st_flow_class[0].ui_proto_list & HI_COMMON_PROTO_ICMPV6)
        {
            ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s -p ipv6 --ip6-proto ipv6-icmp %s %s", auc_cmd_chain, key,
                          act_v6);
            PP(ret);
            ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));
            HI_QOS_RET_CHECK(ret);
        }
    }
    return HI_RET_SUCC;
}
/*
 * 功能描述: 802.1p规则匹配
 * 注意事项: 路由模式下测试不生效，导致规则被加入了PREROUTING链中，但可能会和后面的app规则冲突，重构代码逻辑暂且保留此代码
             后续需要实现iptables匹配802.1p的规则。
 * 函数作者: 凡海飞 f00505368  2020年8月1日
 */
static uint32_t ebt_act_8021p(hi_qos_flow_queue_s *flow_queue, char *key, char *act_v4, char *act_v6)
{
    char auc_cmd_chain[256] = {0};
    int32_t ret = 0;
    char auc_cmd[256] = {0};
    ret = sprintf(auc_cmd_chain, sizeof(auc_cmd_chain), "-t nat -%c PREROUTING -i ! wan+",
                  (flow_queue->ui_enable) ? ('A') : ('D'));
    PP(ret);
    /* 已经指定协议了 */
    if (strstr(key, "-p") == NULL)
    {
        return HI_RET_FAIL;
    }
    ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s %s", auc_cmd_chain, key, act_v4);
    PP(ret);
    ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));

    (void)memset(auc_cmd_chain, 0, sizeof(auc_cmd_chain));
    (void)memset(auc_cmd, 0, sizeof(auc_cmd));
    ret = sprintf(auc_cmd_chain, sizeof(auc_cmd_chain), "-t nat -%c PREROUTING -i ! ppp+",
                  (flow_queue->ui_enable) ? ('A') : ('D'));
    PP(ret);
    ret = sprintf(auc_cmd, sizeof(auc_cmd), "ebtables %s %s %s", auc_cmd_chain, key, act_v4);
    PP(ret);
    ret = __qos_flow_rule_ebtables_exec(auc_cmd, sizeof(auc_cmd));

    HI_QOS_RET_CHECK(ret);
    return HI_RET_SUCC;
}

typedef struct
{
    hi_qos_flow_type_e type;
    char *ebt_key_str_format;
    uint32_t (*get_ebt_key)(char *min, char *max, char *str_format, char *key, uint32_t key_size);
    uint32_t (*ebt_act)(hi_qos_flow_queue_s *pst_flow, char *key, char *act_v4, char *act_v6);
    char *ipt_key_str_format;
    bool ipt_range;
    uint32_t (*get_ipt_key)(char *min, char *max, char *str_format, char *key, uint32_t key_size);
    uint32_t (*ipt_act)(hi_qos_flow_queue_s *pst_flow, char *key, char *act_v4, char *act_v6);
} hi_qos_flow_act;

static hi_qos_flow_act get_key_by_uplink_flow_type[] =
{
	//type				          ebt_key_str_format           get_ebt_key           ebt_act                    ipt_key_str_format               ipt_range       get_ipt_key                         ipt_act
	{HI_QOS_FLOW_TYPE_SMAC,       NULL,                        NULL,                 NULL,                      "-m mac --mac-source %s ",       false,           get_key_common1,                   ipt_act_all_common},
	{HI_QOS_FLOW_TYPE_DMAC,       "-d %s ",                    get_key_common1,      ebt_act_common,            NULL,                            false,           NULL,                              NULL},
	{HI_QOS_FLOW_TYPE_SIP,        NULL,                        NULL,                 NULL,                      "-m iprange --src-range %s-%s ", false,           get_key_common2,                   ipt_act_v4_common},
	{HI_QOS_FLOW_TYPE_DIP,        NULL,                        NULL,                 NULL,                      "-m iprange --dst-range %s-%s ", false,           get_key_common2,                   ipt_act_v4_common},
	{HI_QOS_FLOW_TYPE_SIPV6,      NULL,                        NULL,                 NULL,                      "-s %s ",                        false,           get_key_common1,                   ipt_act_v6_common},
	{HI_QOS_FLOW_TYPE_DIPV6,      NULL,                        NULL,                 NULL,                      "-d %s ",                        false,           get_key_common1,                   ipt_act_v6_common},
	{HI_QOS_FLOW_TYPE_SPORT,      NULL,                        NULL,                 NULL,                      "--sport %s ",                   false,           get_key_port,                      ipt_act_all_common},
	{HI_QOS_FLOW_TYPE_DPORT,      NULL,                        NULL,                 NULL,                      "--dport %s ",                   false,           get_key_port,                      ipt_act_all_common},
	{HI_QOS_FLOW_TYPE_TOS,        NULL,                        NULL,                 NULL,                      "-m tos --tos 0x%x/0xff ",       true,            get_key_ipt_tos,                   ipt_act_v4_common},
	{HI_QOS_FLOW_TYPE_DSCP,       NULL,                        NULL,                 NULL,                      "-m tos --tos 0x%x/0xfc ",       true,            get_key_ipt_dscp,                  ipt_act_all_common},
	{HI_QOS_FLOW_TYPE_8021P,      "-p 802_1Q --vlan-prio %u ", get_key_int_common1,  ebt_act_8021p,             NULL,                            false,           NULL,                              NULL},
	{HI_QOS_FLOW_TYPE_WANINTF,    NULL,                        NULL,                 NULL,                      "-o %s ",                        false,           get_key_ipt_wan_intf,              ipt_act_all_common},
	{HI_QOS_FLOW_TYPE_LANINTF,    "-i %s ",                    get_key_ebt_lan_intf, ebt_act_common,            "-m mark --mark 0x%02x/0x1f0",   false,           get_key_ipt_lan_intf,              ipt_act_all_common},
	{HI_QOS_FLOW_TYPE_TC,         NULL,                        NULL,                 NULL,                      "-m tos --tos 0x%x/0xff ",       false,           get_key_ipt_tc,                    ipt_act_v6_common},
	{HI_QOS_FLOW_TYPE_VLAN,       "-p 802_1Q --vlan-id %u ",   get_key_int_common1,  ebt_act_8021p,             NULL,                            false,           NULL,                              NULL},
	{HI_QOS_FLOW_TYPE_IPVERSION,  "-p %s ",                    get_key_ip_version,   ebt_act_ipv_protocol_act,  "-p %s ",                        false,           get_key_ip_version,                ipt_act_ipv_protocol_act},
	{HI_QOS_FLOW_TYPE_DEFAULT,    "",                          get_key_empty,        ebt_act_common,            "",                              false,           get_key_empty,                     ipt_act_all_common},
};


hi_uint32 __qos_flow_act_parse(hi_qos_flow_queue_s *pst_flow_queue, hi_char8 *act_v4, hi_uint32 act_v4_size,
                               hi_char8 *act_v6, hi_uint32 act_v6_size)
{
    int ret_s;
    bool no_act = true;
    hi_int32 ret;
    hi_char8 auc_cmd[256] = {0};
    hi_char8 auc_dscp_cmd[64] = {0};

    if (act_v4 == NULL || act_v4_size == 0 || act_v6 == NULL || act_v6_size == 0)
    {
        return HI_RET_FAIL;
    }

    /*解析cfe动作*/
    if (pst_flow_queue->ui_class_policer == 0xff)
    {
        ret_s = strcat(act_v4, "-j DROP ");
        HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
        printf("act_v4=(%s)\n", act_v4);
        return HI_RET_SUCC;
    }

    ret_s = strcat(act_v4, "-j cfe ");
    HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);

    if (pst_flow_queue->ui_queue && pst_flow_queue->queue_enable > 0)
    {
        memset(auc_cmd, 0, sizeof(auc_cmd));
        ret = snprintf(auc_cmd, sizeof(auc_cmd), "--pq %d ",
                       8 - pst_flow_queue->ui_queue); // 优先级与芯片相反 1~8 对应 7~0
        PP(ret);
        ret_s = strcat(act_v4, auc_cmd);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
        no_act = false;
    }

    if (pst_flow_queue->em_8021p_mark == HI_QOS_MARK_EDIT)
    {
        memset(auc_cmd, 0, sizeof(auc_cmd));
        ret = snprintf(auc_cmd, sizeof(auc_cmd), "--spri %d ", pst_flow_queue->ui_8021p);
        PP(ret);
        ret_s = strcat(act_v4, auc_cmd);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
        no_act = false;
    }

    ret_s = strncpy(act_v6, act_v6_size, act_v4);
    HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);

    if (pst_flow_queue->ui_qos_id_v4 != 0)
    {
        printf("call qos del, id=%u\n", pst_flow_queue->ui_qos_id_v4);
        // qos_id_free(pst_flow_queue->ui_qos_id_v4);
    }

    if (pst_flow_queue->em_dscp_mark == HI_QOS_MARK_EDIT)
    {
        // pst_flow_queue->ui_qos_id_v4 = qos_id_alloc(0, HI_QOS_MARK_EDIT, 0);
        printf("flow_queue->ui_qos_id_v4=%u\n", pst_flow_queue->ui_qos_id_v4);
    }
    memset(auc_cmd, 0, sizeof(auc_cmd));
    ret = snprintf(auc_cmd, sizeof(auc_cmd), "--qid %d ", pst_flow_queue->ui_qos_id_v4);
    PP(ret);
    ret_s = strcat(act_v4, auc_cmd);
    HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);

    /* TOS、DSCP匹配要求流的五元组一样，导致加速不能识别 */
    if ((pst_flow_queue->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_TOS) ||
        (pst_flow_queue->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_DSCP) ||
        (pst_flow_queue->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_8021P))
    {
        memset(auc_cmd, 0, sizeof(auc_cmd));
        ret = snprintf(auc_cmd, sizeof(auc_cmd), "--acc no ");
        PP(ret);
        ret_s = strcat(act_v4, auc_cmd);
    }

    /* HI_QOS_FLOW_TYPE_DMAC only set EBTABLES rule, EBTABLES not support '-j DSCP' option */
    if (pst_flow_queue->em_dscp_mark == HI_QOS_MARK_EDIT &&
        pst_flow_queue->st_flow_class[0].em_type != HI_QOS_FLOW_TYPE_DMAC)
    {
        memset(auc_dscp_cmd, 0, sizeof(auc_dscp_cmd));
        ret = snprintf(auc_dscp_cmd, sizeof(auc_dscp_cmd), "-j DSCP --set-dscp 0x%02x", pst_flow_queue->ui_dscp);
        PP(ret);
        ret_s = strcat(act_v4, auc_dscp_cmd);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
        no_act = false;
    }

    if (pst_flow_queue->ui_qos_id_v6 != 0)
    {
        printf("call qos del, qos_id_v6=%u\n", pst_flow_queue->ui_qos_id_v6);
        // qos_id_free(pst_flow_queue->ui_qos_id_v6);
    }

    if (pst_flow_queue->em_tc_mark == HI_QOS_MARK_EDIT)
    {
        // pst_flow_queue->ui_qos_id_v6 = qos_id_alloc(0, HI_QOS_MARK_EDIT, 0);
        printf("flow_queue->ui_qos_id_v6=%u\n", pst_flow_queue->ui_qos_id_v6);
    }
    memset(auc_cmd, 0, sizeof(auc_cmd));
    // ret = snprintf(auc_cmd, sizeof(auc_cmd), "--qid %d ", pst_flow_queue->ui_qos_id_v6);
    PP(ret);
    ret_s = strcat(act_v6, auc_cmd);
    HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);

    /* TC匹配要求流全部一样，只有TC字段不一样的流打上不同的802.1p,这样的话无法走加速，82t的五元组加速会导致识别成一条流 */
    if ((pst_flow_queue->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_TC) ||
        (pst_flow_queue->st_flow_class[0].em_type == HI_QOS_FLOW_TYPE_8021P))
    {
        memset(auc_cmd, 0, sizeof(auc_cmd));
        ret = snprintf(auc_cmd, sizeof(auc_cmd), "--acc no ");
        PP(ret);
        ret_s = strcat(act_v6, auc_cmd);
    }

    if (pst_flow_queue->em_tc_mark == HI_QOS_MARK_EDIT)
    {
        memset(auc_dscp_cmd, 0, sizeof(auc_dscp_cmd));
        ret = snprintf(auc_dscp_cmd, sizeof(auc_dscp_cmd), "-j DSCP --set-dscp 0x%02x",
                       pst_flow_queue->ui_tc);
        PP(ret);
        ret_s = strcat(act_v6, auc_dscp_cmd);
        HI_QOS_SAFE_FUNC_RET_CHECK(ret_s);
        no_act = false;
    }

    if (no_act)
    {
        (void)memset(act_v4, 0, act_v4_size);
        (void)memset(act_v6, 0, act_v6_size);
    }
    else
    {
        printf("act_v4=(%s) act_v6=(%s)\n", act_v4, act_v6);
    }

    return HI_RET_SUCC;
}

static int32_t __qos_flow_queue_parse_exec(hi_qos_flow_queue_s *flow_queue)
{
    hi_qos_flow_act *flow_act = get_key_by_uplink_flow_type;
    uint32_t flow_act_num = sizeof(get_key_by_uplink_flow_type) / sizeof(hi_qos_flow_act);
    uint32_t i, j, value;
    uint32_t set_pq = 0;
    int32_t ret = 0;
    char ebt_key_cmd[256] = {0};
    char ipt_key_cmd[256] = {0};
    char act_v4[256] = {0};
    char act_v6[256] = {0};
    char min_value[64] = "";
    char max_value[64] = "";
    char dev_name[256] = {0};
    ret = __qos_flow_act_parse(flow_queue, act_v4, sizeof(act_v4), act_v6, sizeof(act_v6));
    HI_QOS_RET_CHECK(ret);
    if (strlen(act_v4) == 0 && strlen(act_v6) == 0)
    {
        printf("no act qos\n");
        return HI_RET_SUCC;
    }
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < flow_act_num; j++)
        {
            if (flow_queue->st_flow_class[i].em_type != flow_act[j].type)
                continue;
            if ((flow_queue->st_flow_class[i].em_type <= HI_QOS_FLOW_TYPE_DIPV6) ||
                (flow_queue->st_flow_class[i].em_type == HI_QOS_FLOW_TYPE_WANINTF))
            {
                get_str_range(flow_queue->st_flow_class[i].uc_max, flow_queue->st_flow_class[i].uc_min,
                              max_value, sizeof(max_value), min_value, sizeof(min_value));
            }
            else
            {
                get_int_range(flow_queue->st_flow_class[i].ui_max, flow_queue->st_flow_class[i].ui_min,
                              max_value, sizeof(max_value), min_value, sizeof(min_value));
            }
            /* 获取ebt key, 执行ebtables规则 */
            if ((flow_act[j].get_ebt_key != NULL) && (flow_act[j].ebt_act != NULL))
            {
                ret = flow_act[j].get_ebt_key(min_value, max_value, flow_act[j].ebt_key_str_format,
                                              ebt_key_cmd, sizeof(ebt_key_cmd));
                HI_QOS_RET_CHECK(ret);
                ret = flow_act[j].ebt_act(flow_queue, ebt_key_cmd, act_v4, act_v6);
                HI_QOS_RET_CHECK(ret);
                if (flow_queue->st_flow_class[i].em_type == HI_QOS_FLOW_TYPE_DMAC)
                    hi_os_vcmd("ebtables %s INPUT %s %s\n", (flow_queue->ui_enable) ? ("-I") : ("-D"),
                               ebt_key_cmd, act_v4);
            }
            /* 获取ibt key, 执行iptables规则 */
            if ((flow_act[j].get_ipt_key != NULL) && (flow_act[j].ipt_act != NULL))
            {
                if (flow_act[j].ipt_range)
                {
                    for (value = flow_queue->st_flow_class[i].ui_min;
                         value <= flow_queue->st_flow_class[i].ui_max; value++)
                    {
                        get_int_range(value, value, max_value, sizeof(max_value),
                                      min_value, sizeof(min_value));
                        ret = flow_act[j].get_ipt_key(min_value, max_value,
                                                      flow_act[j].ipt_key_str_format, ipt_key_cmd, sizeof(ipt_key_cmd));
                        HI_QOS_RET_CHECK(ret);
                        ret = flow_act[j].ipt_act(flow_queue, ipt_key_cmd, act_v4, act_v6);
                        HI_QOS_RET_CHECK(ret);
                    }
                }
                else
                {
                    ret = flow_act[j].get_ipt_key(min_value, max_value, flow_act[j].ipt_key_str_format,
                                                  ipt_key_cmd, sizeof(ipt_key_cmd));
                    HI_QOS_RET_CHECK(ret);
                    ret = flow_act[j].ipt_act(flow_queue, ipt_key_cmd, act_v4, act_v6);
                    HI_QOS_RET_CHECK(ret);
                }
            }
            // set dev pq
            if (flow_queue->st_flow_class[i].em_type == HI_QOS_FLOW_TYPE_WANINTF)
            {
                // if (__qos_wanif_devname_get(min_value, dev_name, sizeof(dev_name)) == HI_RET_SUCC)
                // {
                // 	set_pq = flow_queue->ui_enable ? (8 - flow_queue->ui_queue) : 0;
                // ret = hi_netif_set_proto_pq_rate(dev_name, HI_NETIF_PROTO_TYPE_ALL, 0, set_pq);
                // if (ret != HI_RET_SUCC)
                // 	HI_QOS_ERR("failed to set wan %s pq\n", dev_name);
                // }
            }
        }
    }
    return HI_RET_SUCC;
}
#endif

#undef PP // off: dbgcfg emu:0:0 on: dbgcfg emu:8:0
#define PP(fmt, ...) printf("\033[0;32;31m[CM :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)

#if 1
int main(int argc, char const *argv[])
{
    /* code */
    char str[256] = "iptables -t mangle -A HI_QOS_CHAIN_4 -p udp -m mark --mark 0x20/0x1f0  -j DSCP --set-dscp 0x38";
    char *str1 = &str[0];
    {
        int ret_s;
        hi_int32 ret = 0;
        hi_char8 auc_match[256] = {0};
        hi_char8 auc_act[256] = {0};
        hi_char8 auc_cmd[256] = {0};
        hi_uint32 ui_cnt = 0;
        hi_uint32 ui_len = 0;
        hi_uint32 ui_offset = 0;
        hi_char8 *puc_tmp = str1;
        hi_char8 *puc_pre = str1;
        int type = 4;

        /* 循环检查iptables命令里有几个动作(-j)，每个动作执行一次iptables配置命令 */
        PP("cmd:%s\n", str1);
        do
        {
            puc_tmp = strstr(puc_tmp, "-j");
            if (puc_tmp != NULL)
            {
                ui_len = puc_tmp - puc_pre;
            }
            else
            {
                ui_len = str + strlen(str) - puc_pre;
            }
            
            /* 第一个-j拷贝match部分，后续拷贝动作同时生成命令并执行 */
            if (!ui_cnt)
            {
                snprintf(auc_match, sizeof(auc_match), "%.*s", ui_len, str1 + ui_offset);
                auc_match[ui_len] = '\0';
                PP("auc_match = %s", auc_match);
            }
            else
            {
                snprintf(auc_act, sizeof(auc_match), "%.*s", ui_len, str1 + ui_offset);
                auc_match[ui_len] = '\0';
                PP("auc_match = %s", auc_match);

                if (type == 4)
                {
                    ret = snprintf(auc_cmd, sizeof(auc_cmd), "iptables %s%s", auc_match, auc_act);
                    PP("%d", ret);
                }
                else if (type == 6)
                {
                    ret = snprintf(auc_cmd, sizeof(auc_cmd), "ip6tables %s%s",  auc_match, auc_act);
                    PP("%d", ret);
                }
                else
                {
                    PP("invalid type=%d\n", type);
                    return -1;
                }
    
                // strcat(auc_cmd, auc_match);
                // strcat(auc_cmd, auc_act);
                PP("Exec:%s \r\n", auc_cmd);
                // ret = hi_os_execcmd(auc_cmd);
                PP("%d", ret);
            }
    
            ui_cnt++;
            ui_offset += ui_len;
            puc_pre = puc_tmp;
            if (puc_tmp != 0)
            {
                puc_tmp += 2;
            }
        }
        while (puc_pre != NULL);   //以puc_pre来判断查找字符串结束，puc_tmp每次会偏移2个字节以跳过命中的-j
    
        return 0;
    }

    return 0;
}
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DOUBLE_DASH_LEN 2 // "--"的长度

int process_ebtables_command(char *puc_cmd) {
    int ret = -1;
    char cmd_origin[256] = {0};
    char cmd[256] = {0};
    char cmd_tmp[256] = {0};
    char cfe_act[128] = {0}; // 增加 cfe_act 的大小
    char *start = NULL;
    char *end = NULL;

    if (puc_cmd == NULL) {
        PP("NULL pointer\n");
        return -1;
    }
    PP("puc_cmd = %s\n", puc_cmd);

    strncpy(cmd_origin, puc_cmd, strlen(puc_cmd));
    PP("cmd_origin = %s\n", cmd_origin);

    // 查找 "-j cfe"
    start = strstr(cmd_origin, "-j cfe");
    if (start == NULL) {
        PP("wrong ebtables cmd, no -j cfe\n");
        return -1;
    }

    // 提取 "-j cfe" 及其参数
    end = strstr(start, "--");
    if (end == NULL) {
        snprintf(cmd_tmp, sizeof(cmd_tmp), "%.*s", (int)(strlen(cmd_origin)), cmd_origin);
        PP("cmd_tmp = %s\n", cmd_tmp);
    } else {
        snprintf(cmd_tmp, sizeof(cmd_tmp), "%.*s", (int)(end - cmd_origin), cmd_origin);
        PP("cmd_tmp = %s\n", cmd_tmp);
    }

    // 分割 "--" 参数并执行命令
    start = end;
    while (start != NULL) {
        end = strstr(start + DOUBLE_DASH_LEN, "--");
        if (end != NULL) {
            snprintf(cfe_act, sizeof(cfe_act), "%.*s", (int)(end - start), start);
            PP("cfe_act = %s\n", cfe_act);
        } else {
            snprintf(cfe_act, sizeof(cfe_act), "%s", start);
            PP("cfe_act = %s\n", cfe_act);
        }

        snprintf(cmd, sizeof(cmd), "%s %s", cmd_tmp, cfe_act);
        PP("cmd(%s)\n", cmd);

        // ebtables 不支持 set-dscp
        if (strstr(cmd, "set-dscp") == NULL) {
            // ret = system(cmd); // 使用system代替hg_os_execcmd
            PP();
            if (ret != 0)
            {
                PP("cmd(%s) RET:%d\n", cmd, ret);
            }
        }

        if (end == NULL) {
            break;
        }
        start = end;
    }

    return 0;
}

int main() {
    char cmd[] = "ebtables -A HI_QOS_CHAIN_4 -p ipv4 --ip-proto udp -i eth1 -j mark --mark-or 0x20 --mark-target CONTINUE -j cfe --pq 7 --spri 5 --qid 1 --acc no -j DSCP --set-dscp 0x18";
    process_ebtables_command(cmd);
    return 0;
}
#endif
