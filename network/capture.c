/*
 * @*************************************: 
 * @FilePath: /network/capture.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-25 16:53:12
 * @LastEditors: dof
 * @LastEditTime: 2023-08-25 16:59:29
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <libpcap/pcap.h>


typedef struct
{
        unsigned int ui_time;
        unsigned int ui_port;
        enum hi_net_al_port em_port;
        char ac_protocol[64];
        char ac_interface[64];
        char ac_ipaddr[64];
} hi_netapp_capture_config_s;


int capture_set()
{
    pcap_t *device = NULL;
    struct bpf_program filter;
    char  filter_app[128] = {0};
    char  errbuf[128]={0};
    char  device_str[64] = {0};

    /* 抓包前恢复配置 */
    hi_netapp_capture_recovery(cap_config);

    /* 获取设备操作句柄 */
    if (hi_netif_get_alport_ifname(cap_config->em_port, device_str, sizeof (device_str)) != HI_RET_SUCC) {
        (void)memcpy_s(device_str, 64, "pon", strlen("pon"));
    }
    HI_SAL_NET_DBG("packet capture network port: [%s]\n", device_str);
    device = pcap_create(device_str, errbuf);
    if(!device)
    {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        HI_SAL_NET_DBG("error: pcap_create(): [%s]\n", errbuf);
        return HI_RET_FAIL;
    }

    /* 设置抓包读取超时 */
    if (pcap_set_timeout(device, 1000) != 0) {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        HI_SAL_NET_DBG("error: pcap_set_timeout(): [%s]\n", errbuf);
        return HI_RET_FAIL;
    }

    /* 激活设备 */
    if (pcap_activate(device) < 0){
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        HI_SAL_NET_DBG("error: pcap_activate(): [%s]\n", pcap_geterr(device));
        pcap_close(device);
        return HI_RET_FAIL;
    }
    g_capture_info.device = device;

    /* 设置过滤规则 */
    hi_netapp_capture_set_filter(filter_app, cap_config);
    if (pcap_compile(device, &filter, filter_app, 1, 0) != HI_RET_SUCC) {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        HI_SAL_NET_DBG("error: pcap_compile(): [%s]\n", pcap_geterr(device));
        pcap_close(device);
        return HI_RET_FAIL;
    }
    if (pcap_setfilter(device, &filter) != HI_RET_SUCC) {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        HI_SAL_NET_DBG("error: pcap_setfilter(): [%s]\n", pcap_geterr(device));
        pcap_close(device);
        return HI_RET_FAIL;
    }
    /* 获取抓包开始时间 */
    g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_RUNNING;
    get_SysDuration(&g_capture_info.ui_start_time);

    hi_os_printf("\n[CAPTURE] start capture packet.\n\n");
    if (hi_netapp_capture_thread_start(device) != HI_RET_SUCC) {
        g_capture_info.ui_capture_state = HI_NETAPP_CAPTURE_STOP;
        HI_SAL_NET_DBG("error: pcap captrue start fail!\n");
        return HI_RET_FAIL;
    }
    return HI_RET_SUCC;

}


int main(int argc, char const *argv[])
{
	
	return 0;
}
