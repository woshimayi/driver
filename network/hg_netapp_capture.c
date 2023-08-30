/******************************************************************************
                  Copyright (C), 2023-2023, HSAN
 ******************************************************************************
  Filename      : hi_netapp_capture.c
  Version       : 初稿
  Author        : hsan
  Creation      : 2023/06/21
  Description   :
******************************************************************************/
/*****************************************************************************
 *                                INCLUDE                                    *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <pcap.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*****************************************************************************
 *                                LOCAL_DEFINE                               *
 *****************************************************************************/
#define HI_NETAPP_CAPTURE_BOARD_BUSY_USAGE 80
#define HI_NETAPP_CAPTURE_MAX (10 * 1024 * 1024)   /* 10MByte */
#define HI_NETAPP_CAPTURE_TIMEOUT (60 * 10 * 1000) /* 单位：ms */
#define CAPTURE_STACK_SIZE 0x40000
#define HI_NETAPP_CAPTURE_SIZE 128
#define HI_NETAPP_CAPTURE_INTERFACE_LEN 64

#define FALSE 0
#define TRUE 1

/*****************************************************************************
 *                                LOCAL_TYPEDEF                              *
 *****************************************************************************/

struct hi_netif_uc_cfg
{
    unsigned int enable;
    unsigned int lrn_pkt_num;
    unsigned int age_en;
};

typedef struct hi_sysinfo_auid
{
    unsigned char auc_sn[8];
    unsigned char auc_pwd[10];
} hi_sysinfo_auid_s;

typedef struct
{
    unsigned int type;
} hi_capture_notify_info_s;

typedef struct hi_sysinfo_data
{
    unsigned char auc_llid0_mac[6];
    unsigned char auc_llid1_mac[6];
    unsigned char auc_llid2_mac[6];
    unsigned char auc_llid3_mac[6];
    unsigned char auc_llid4_mac[6];
    unsigned char auc_llid5_mac[6];
    unsigned char auc_llid6_mac[6];
    unsigned char auc_llid7_mac[6];
    hi_sysinfo_auid_s st_auid;
    unsigned char auc_gw_mac[6];
    char ac_product_sn[68];
    char ac_reg_id[40];
    char ac_psk[20];
    char ac_vendor_id[20];
    char ac_version[20];
    char ac_mode[12];
    char ac_sw_version[68];
    char ac_sdk_version[52];
    char ac_hw_version[68];
    char ac_equid[28];
    unsigned int ui_product_code;
    char ac_loid[28];
    char ac_lopwd[20];
    char ac_manufactureroui[12];
    char ac_province[16];
    char ac_operator[12];
    unsigned int ui_smart;
    unsigned int ui_aes;
    char ac_pon_adapt[12];
    unsigned int ui_web_over_cwmpwan;
    unsigned int ui_voip_type;
    unsigned int ui_factory;
    char ac_manufacturer[68];
    unsigned int ui_device_mode;
    unsigned int ui_factory_mask;
} hi_sysinfo_data_s;

typedef struct
{
    pcap_t *device;
    unsigned int ui_packet_receive;
    unsigned int ui_capture_state;
    unsigned int ui_start_time;
    unsigned int ui_timeout;
    char ac_output_path[HI_NETAPP_CAPTURE_SIZE];
} hi_netapp_capture_info_s;

typedef enum
{
    HI_NETAPP_CAPTURE_STOP,
    HI_NETAPP_CAPTURE_RUNNING,
    HI_NETAPP_CAPTURE_COMPLETE,
    HI_NETAPP_CAPTURE_OUTOFRES,
    HI_NETAPP_CAPTURE_NOPACKET,
    HI_NETAPP_CAPTURE_OTHER,
    HI_NETAPP_CAPTURE_STATUS_NUM,
} hi_netapp_capture_status_e;

typedef struct
{
    unsigned int ui_time;
    unsigned int ui_port;
    int em_port;
    char ac_protocol[HI_NETAPP_CAPTURE_INTERFACE_LEN];
    char ac_interface[HI_NETAPP_CAPTURE_SIZE];
    char ac_ipaddr[HI_NETAPP_CAPTURE_INTERFACE_LEN];
} hi_netapp_capture_config_s;

/*****************************************************************************
 *                                LOCAL_VARIABLE                             *
 *****************************************************************************/
static hi_netapp_capture_info_s g_capture_info = {0};
static unsigned int g_capture_state = HI_NETAPP_CAPTURE_STOP;
pthread_t g_capture_stop = NULL, g_capture_start = NULL;
/*****************************************************************************
 *                                LOCAL_FUNCTION                             *
 *****************************************************************************/

char *hi_tolower(char *str)
{
    char *tmp = str;
    if (NULL == str)
    {
        return NULL;
    }

    while (*tmp != '\0')
    {
        *tmp = (char)tolower(*tmp);
        tmp++;
    }

    return str;
}

int memsize(unsigned int *pui_total, unsigned int *pui_free)
{
#define PROC_MEMINFO "/proc/meminfo"
    FILE *pf = NULL;
    char readBuf[256];
    char *pos, *pRem;
    int totoMem, availMem;

    pf = fopen(PROC_MEMINFO, "r");
    if (NULL == pf)
    {
        return -1;
    }
    if (0 > fread(readBuf, 1, sizeof(readBuf) - 1, pf)) // lint !e568 !e685
    {
        fclose(pf);
        return -1;
    }
    fclose(pf);

    pos = strstr(readBuf, "MemTotal:");
    if (NULL == pos)
    {
        return -1;
    }
    pos += strlen("MemTotal:");
    totoMem = strtol(pos, &pRem, 0);
    if (0 == totoMem)
    {
        return -1;
    }
    pos = strstr(pos, "MemAvailable:");
    if (NULL == pos)
    {
        return -1;
    }
    pos += strlen("MemAvailable:");
    availMem = strtol(pos, &pRem, 0);

    *pui_total = totoMem;
    *pui_free = availMem;
    return 0;
}

int get_MemoryRate(unsigned int *memusage)
{
    unsigned int ui_total = 0, ui_free = 0;
    memsize(&ui_total, &ui_free);
    MEM_FREE_COUNT(ui_total, ui_free);

    if (ui_total > (512 << 10)) {
        ui_total = (1024 << 10);
    } else if (ui_total > (256<<10)) {
        ui_total = (512 << 10);
    } else if (ui_total > (128<<10)) {
        ui_total = (256 << 10);
    } else  {
        ui_total = (128 << 10);
    }

    *memusage = ((ui_total-ui_free)*100/ui_total);
    return 0;
}


int hi_os_cpuusage(int *pi_total, int *pi_idle)
{
#define PROC_STAT "/proc/stat"
    int stat[8]; /* user , nice , sys, idle, iowait, irq, sirq, steal, guest */
    FILE *fp = NULL;
    if (NULL == (fp = fopen(PROC_STAT, "r"))) {
        return -1;
    }
    fscanf(fp, "cpu %d %d %d %d %d %d %d %d",
            &stat[0],&stat[1],&stat[2],&stat[3],&stat[4],&stat[5],&stat[6],&stat[7]);
    fclose(fp);

    *pi_total = stat[0]+stat[1]+stat[2]+stat[3]+stat[4]+stat[5]+stat[6];
    *pi_idle = stat[3];

	return 0;
}


int get_CPURate(unsigned int *cpuusage)
{
    int used     = 0;
    int usage    = 0;
    int crtTotal = 0;
    int crtIdel  = 0;
    static int lastTotal = 0;
    static int lastIdel = 0;
    static int lastUsage = 1;
#define HI_CPUUSAGE_CAL_PERIOD (100 * 3) // jiffies tick

    hi_os_cpuusage(&crtTotal, &crtIdel);
    if ((crtTotal - lastTotal) >= HI_CPUUSAGE_CAL_PERIOD)  {
        used = (crtTotal - lastTotal) - (crtIdel - lastIdel);
        usage = (used * 100) / (crtTotal - lastTotal);
	    lastIdel = crtIdel;
	    lastTotal = crtTotal;
	    lastUsage = usage;
    } else {
        usage = lastUsage;
    }

    // CPU_RATE_COUNT(usage);
    *cpuusage = usage;
	return 0;
}

int get_SysDuration(unsigned int *sysduration)
{
    struct sysinfo info;
    sysinfo(&info);
    *sysduration = info.uptime;
    return info.uptime;
}



static int hi_netapp_capturetask_create(pthread_t *task, char *name, void *taskEntry,
                                        unsigned int stackSize)
{
    pthread_attr_t attr;
    pthread_attr_t *pattr = &attr;

    pthread_attr_init(pattr);
    pthread_attr_setdetachstate(pattr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(pattr, stackSize);

    if (pthread_create(task, pattr, taskEntry, 0) == 0)
    {
        pthread_attr_destroy(pattr);
        return 0;
    }
    pthread_attr_destroy(pattr);
    return -1;
}

static void hi_netapp_capture_set_filter(char *filter_app, hi_netapp_capture_config_s *config)
{
    char filter[HI_NETAPP_CAPTURE_SIZE] = {0};
    if (strcmp(config->ac_protocol, "ALL") != 0)
    {
        hi_tolower(config->ac_protocol);
        snprintf(filter_app, HI_NETAPP_CAPTURE_SIZE, "%s", config->ac_protocol);
    }
    if (config->ui_port != 0)
    {
        snprintf(filter_app + strlen(filter_app), HI_NETAPP_CAPTURE_SIZE, "%sport %d",
                 (strlen(filter_app) > 0) ? " and " : "", config->ui_port);
    }
    if (strlen(config->ac_ipaddr) > 1)
    {
        snprintf(filter_app + strlen(filter_app), HI_NETAPP_CAPTURE_SIZE, "%snet %s",
                 (strlen(filter_app) > 0) ? " and " : "", config->ac_ipaddr);
    }
    if (strlen(filter_app) > 0)
    {
        (void)memcpy(filter, filter_app, HI_NETAPP_CAPTURE_SIZE);
        snprintf(filter_app + strlen(filter_app), HI_NETAPP_CAPTURE_SIZE, " or (vlan and %s)", filter);
    }
    printf("the filter criteria are [%s]\n", filter_app);
}

static void hi_netapp_capture_set_output_path(char *ac_interface)
{
    struct tm nowtime;
    struct timeval tv;
    char time_now[128];
    hi_sysinfo_data_s st_sysinfo = {0};

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &nowtime);
    snprintf(time_now, sizeof(time_now), "%04d%02d%02d%02d%02d%02d",
             nowtime.tm_year + 1900, nowtime.tm_mon + 1, nowtime.tm_mday,
             nowtime.tm_hour, nowtime.tm_min, nowtime.tm_sec);

    if (HI_IPC_CALL("hi_sysinfo_data_get", &st_sysinfo) != 0)
    {
        printf("sysinfo not find\n");
        return;
    }

    snprintf(g_capture_info.ac_output_path, HI_NETAPP_CAPTURE_SIZE, "/tmp/%02X%02X%02X%02X%02X%02X_%s_%s.pcap",
             st_sysinfo.auc_gw_mac[0], st_sysinfo.auc_gw_mac[1], st_sysinfo.auc_gw_mac[2],
             st_sysinfo.auc_gw_mac[3], st_sysinfo.auc_gw_mac[4], st_sysinfo.auc_gw_mac[5],
             ac_interface, time_now);

    printf("output path is [%s]\n", g_capture_info.ac_output_path);
}

static unsigned int hi_netapp_check_board_usage(void)
{
    unsigned int cpu_rate, memory_rate;
    get_MemoryRate(&memory_rate);
    get_CPURate(&cpu_rate);
    if (cpu_rate >= HI_NETAPP_CAPTURE_BOARD_BUSY_USAGE || memory_rate >= HI_NETAPP_CAPTURE_BOARD_BUSY_USAGE)
    {
        printf("\n[CAPTURE] the usage of the device is too high!\n");
        printf("[CAPTURE] memory_rate[%d] cpu_rate[%d].\n\n", memory_rate, cpu_rate);
        unlink(g_capture_info.ac_output_path);
        return 0x04;
    }
    return 0;
}

static unsigned int hi_netapp_check_flash_usage(void)
{
    struct stat statbuf;
    stat(g_capture_info.ac_output_path, &statbuf); /* 单位:byte */
    if (statbuf.st_size > HI_NETAPP_CAPTURE_MAX)
    {
        printf("\n\n[CAPTURE] packet capture: the current file exceeds 10MByte!\n");
        printf("[CAPTURE] file[%ld].\n\n", statbuf.st_size);
        return -1;
    }
    return 0;
}

static unsigned int hi_netapp_capture_check_timeout(void)
{
    unsigned int ui_current_time;
    get_SysDuration(&ui_current_time);
    if ((ui_current_time - g_capture_info.ui_start_time) >= g_capture_info.ui_timeout)
    {
        printf("\n\n[CAPTURE] packet capture completed.\n\n");
        return -1;
    }
    return 0;
}

static unsigned int hi_netapp_capture_packet_accelerate_enable(unsigned int en)
{
    unsigned int i_ret;
    struct hi_netif_uc_cfg cfg;
    i_ret = HI_IPC_CALL("hi_netif_get_uc_cfg", &cfg);
    if (i_ret != 0)
    {
        return -1;
    }
    cfg.enable = en;
    i_ret = HI_IPC_CALL("hi_netif_set_uc_cfg", &cfg);
    if (i_ret != 0)
    {
        return -1;
    }
    return 0;
}

static void hi_netapp_capture_stop(void)
{
    // hi_capture_notify_info_s event = {0};

    while (1)
    {
        sleep(1);
        if (g_capture_info.ui_capture_state == HI_NETAPP_CAPTURE_STOP)
        {
            continue;
        }
        if (hi_netapp_check_board_usage() != 0)
        {
            hi_netapp_capture_packet_accelerate_enable(TRUE);
            // system("echo 0 > /sys/module/hi_kcfe_res/parameters/g_cfe_capture_en");
            g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
            g_capture_state = HI_NETAPP_CAPTURE_OUTOFRES;
            pcap_breakloop(g_capture_info.device);
            continue;
        }
        if ((hi_netapp_check_flash_usage() != 0) || (hi_netapp_capture_check_timeout() != 0))
        {
            hi_netapp_capture_packet_accelerate_enable(TRUE);
            // system("echo 0 > /sys/module/hi_kcfe_res/parameters/g_cfe_capture_en");
            g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
            g_capture_state = HI_NETAPP_CAPTURE_COMPLETE;
            // hi_notifier_call(HI_CAPTURE_NOTIFIER_NAME, &event);
            pcap_breakloop(g_capture_info.device);
            continue;
        }
    }
}

static void getpacket(unsigned char *arg, const struct pcap_pkthdr *pkthdr, const unsigned char *packet)
{
    pcap_dump(arg, pkthdr, packet); // 输出数据到文件
    g_capture_info.ui_packet_receive++;
    printf("Packet length: %d\n", pkthdr->len);
    printf("Number of capture bytes: %d\n", pkthdr->caplen);
    printf("Recieved time: %s", ctime((const time_t *)&pkthdr->ts.tv_sec));
}

static void hi_netapp_capture_thread(void)
{
    pcap_dumper_t *out_pcap;
    pcap_t *device = g_capture_info.device;

    /* 开始抓包 */
    out_pcap = pcap_dump_open(device, g_capture_info.ac_output_path);
    if (out_pcap == NULL)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_dump_open(): [%s]\n", pcap_geterr(device));
        pcap_dump_flush(out_pcap);
        pcap_dump_close(out_pcap);
        pcap_close(device);
        return;
    }
    if (pcap_loop(device, 0, getpacket, (unsigned char *)out_pcap) != PCAP_ERROR_BREAK)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_loop(): [%s]\n", pcap_geterr(device));
        pcap_dump_flush(out_pcap);
        pcap_dump_close(out_pcap);
        pcap_close(device);
        return;
    }

    chmod(g_capture_info.ac_output_path, 777);        // 777 :rwx
    chown(g_capture_info.ac_output_path, 1001, 1001); // 1001: owner,  1001: group

    pcap_dump_flush(out_pcap); /* 刷新缓冲区 */
    pcap_dump_close(out_pcap); /* 关闭资源 */
    pcap_close(device);
    return;
}

static unsigned int hi_netapp_capture_thread_start(pcap_t *device)
{
    int i_ret;
    if (g_capture_start != NULL)
    {
        pthread_cancel(g_capture_start);
        pthread_join(g_capture_start, NULL);
        printf("cancel the last capture thread success.\n");
    }

    if (hi_netapp_capture_packet_accelerate_enable(FALSE) != 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("failed to disable packet accelerate.\n");
        return -1;
    }
    // system("echo 1 > /sys/module/hi_kcfe_res/parameters/g_cfe_capture_en");

    /* 开始抓包 */
    i_ret = pthread_create(&g_capture_start, NULL, (void *)hi_netapp_capture_thread, NULL);
    if (i_ret != 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        return -1;
    }
    return 0;
}

static void hi_netapp_capture_recovery(hi_netapp_capture_config_s *cap_config)
{
    printf("capture config : time[%u] port[%u] protocol[%s], interface[%s][%d] ip[%s]\n",
           cap_config->ui_time, cap_config->ui_port, cap_config->ac_protocol,
           cap_config->ac_interface, cap_config->em_port, cap_config->ac_ipaddr);
    (void)memset(&g_capture_info, 0, sizeof(g_capture_info));
    g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
    g_capture_info.ui_timeout = cap_config->ui_time;
    g_capture_state = HI_NETAPP_CAPTURE_OTHER;
    hi_netapp_capture_set_output_path(cap_config->ac_interface);
}

/*****************************************************************************
 *                                PUBLIC_FUNCTION                            *
 *****************************************************************************/
int hi_netapp_capture_start(hi_netapp_capture_config_s *cap_config)
{
    pcap_t *device = NULL;
    struct bpf_program filter;
    char filter_app[HI_NETAPP_CAPTURE_SIZE] = {0};
    char errbuf[HI_NETAPP_CAPTURE_SIZE] = {0};
    char device_str[HI_NETAPP_CAPTURE_INTERFACE_LEN] = {0};

    /* 抓包前恢复配置 */
    hi_netapp_capture_recovery(cap_config);

    /* 获取设备操作句柄 */
    if (hi_netif_get_alport_ifname(cap_config->em_port, device_str, sizeof(device_str)) != 0)
    {
        (void)memcpy(device_str, "pon", strlen("pon"));
    }
    printf("packet capture network port: [%s]\n", device_str);
    device = pcap_create(device_str, errbuf);
    if (!device)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_create(): [%s]\n", errbuf);
        return -1;
    }

    /* 设置抓包读取超时 */
    if (pcap_set_timeout(device, 1000) != 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_set_timeout(): [%s]\n", errbuf);
        return -1;
    }

    /* 激活设备 */
    if (pcap_activate(device) < 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_activate(): [%s]\n", pcap_geterr(device));
        pcap_close(device);
        return -1;
    }
    g_capture_info.device = device;

    /* 设置过滤规则 */
    hi_netapp_capture_set_filter(filter_app, cap_config);
    if (pcap_compile(device, &filter, filter_app, 1, 0) != 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_compile(): [%s]\n", pcap_geterr(device));
        pcap_close(device);
        return -1;
    }
    if (pcap_setfilter(device, &filter) != 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap_setfilter(): [%s]\n", pcap_geterr(device));
        pcap_close(device);
        return -1;
    }
    /* 获取抓包开始时间 */
    g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_RUNNING;
    get_SysDuration(&g_capture_info.ui_start_time);

    printf("\n[CAPTURE] start capture packet.\n\n");
    if (hi_netapp_capture_thread_start(device) != 0)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        printf("error: pcap captrue start fail!\n");
        return -1;
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}

// HI_DEF_IPC(hi_netapp_capture_get, hi_netapp_capture_result_s *, st_result)
// {
//     struct stat statbuf;
//     stat(g_capture_info.ac_output_path, &statbuf);

//     st_result->ui_diagnostics_state = g_capture_state;
//     if (g_capture_state == HI_NETAPP_CAPTURE_COMPLETE)
//     {
//         if (g_capture_info.ui_packet_receive == 0)
//         {
//             st_result->ui_diagnostics_state = HI_NETAPP_CAPTURE_NOPACKET;
//             return 0;
//         }
//         if (memcpy(st_result->ac_capture_name, HI_NETAPP_CAPTURE_SIZE, g_capture_info.ac_output_path,
//                      HI_NETAPP_CAPTURE_SIZE) != 0)
//         {
//             st_result->ui_diagnostics_state = HI_NETAPP_CAPTURE_OTHER;
//             return HI_RET_MALLOC_FAIL;
//         }
//         st_result->ui_capture_size = statbuf.st_size;
//         return 0;
//     }
//     return 0;
// }
// /*****************************************************************************
//  *                                INIT/EXIT                                  *
//  *****************************************************************************/
// int hi_netapp_capture_init(void)
// {
//     /* Create configuration thread */
//     if (hi_netapp_capturetask_create(&g_capture_stop, "stopcapturing", (void *)hi_netapp_capture_stop,
//         CAPTURE_STACK_SIZE) != 0) {
//         return -1;
//     }

//     if (0 != hi_notifier_create(HI_CAPTURE_NOTIFIER_NAME,
//                                           sizeof(hi_capture_notify_info_s))) {
//         printf("create packet capture notifier failed.\n");
//         return -1;
//     }

//     return 0;
// }

// int hi_netapp_capture_exit(void)
// {
//     if (g_capture_stop != NULL) {
//         pthread_cancel(g_capture_stop);
//     }
//     if (g_capture_start != NULL) {
//         pthread_cancel(g_capture_start);
//     }
//     hi_notifier_destroy(HI_CAPTURE_NOTIFIER_NAME);
//     return 0;
// }
