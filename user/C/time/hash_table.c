/*
 * @*************************************:
 * @FilePath     : /user/C/time/hash_table.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-28 15:53:26
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-05 17:03:10
 * @Descripttion :  hash table
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 100

typedef struct _hash_node
{
    char *key;
    int value;
    struct _hash_node *next;
} HashNode;

typedef enum
{
    HG_LOG_START = 0,
    HG_LOG_RMS = 1,    // "RMS"
    HG_LOG_PLUGIN = 2, // "PLUGIN"
    HG_LOG_WEB = 3,    // "WEB"
    HG_LOG_OTHERS = 4, // "OTHERS"
    HG_LOG_INVALID = 5 // "ERROR"
} HG_LOG_CODE;

typedef struct
{
    HG_LOG_CODE hg_log_code;
    const char *hg_log_abbr;
    const char *hg_log_full;
} HG_LOG_DEF;

typedef enum
{
   EID_INVALID=0,
   EID_TR69C=1,        /* Begin TR69 controlled Management App EID range */
   EID_TR64C=2,
   EID_HTTPD=3,
   EID_SNMPD=4,
   EID_CONSOLED=5,
   EID_TELNETD=6,
   EID_SSHD=7,
   EID_UPNP=8,
   EID_AVAILABLE1=9,
   EID_AVAILABLE10=10,
   EID_AVAILABLE11=11,
   EID_AVAILABLE12=12,
   EID_AVAILABLE2=13,
   EID_AVAILABLE14=14,
   EID_AVAILABLE15=15,  /* End TR69 controlled Management App EID range */
   EID_WLNVRAM=17,
   EID_WLWAPID=18,
   EID_WLEVENT=19,
   EID_SMD=20,
   EID_SSK=21,
   EID_PPP=22,
   EID_DHCPC=23,
   EID_DHCPD=24,
   EID_FTPD=25,
   EID_TFTPD=26,
   EID_TFTP=27,
   EID_DNSPROBE=28,
   EID_XMPPC=29,
   EID_SYSLOGD=30,
   EID_KLOGD=31,
   EID_WEBSOCKD=32,
   EID_DDNSD=33,
   EID_ZEBRA=34,
   EID_RIPD=35,
   EID_SNTP=36,
   EID_URLFILTERD=37,
   EID_CWMPD=38,
   EID_TRACERT=39,
   EID_PING=40,
   EID_DHCP6C=41,
   EID_DHCP6S=42,
   EID_RADVD=43,
   EID_DNSPROXY=44,
   EID_IPPD=45,
   EID_FTP=46,
   EID_DSLDIAGD=48,
   EID_SOAPSERVER=49,
   EID_PWRCTL=50,
   EID_HOTPLUG=51,
   EID_L2TPD=52,
   EID_SAMBA=53,
   EID_PPTPD=54,
   EID_DECT=56,
   EID_OMCID=60,
   EID_OMCIPMD=61,   
   EID_RASTATUS6=62,
   EID_EPON_APP=70,
   EID_EPON_OAM_PORT_LOOP_DETECT=71,
   EID_VECTORINGD=80,
   EID_WLSSK=89,
   EID_UNITTEST=90,
   EID_MISC=91,
   EID_WLMNGR=92,
   EID_WLWPS=93,
   EID_CMFD=94,
   EID_MCPD=95,
   EID_MOCAD=96,
   EID_RNGD=97,
   EID_DMSD=98,
   EID_SWMDK=100,
   EID_OLT_TEST=101,
   EID_BMUD=103,
   EID_BMUCTL=104,
   EID_PLC_NVM=105,
   EID_PLC_BOOT=106,
   EID_MCPCTL=107,
   EID_HOMEPLUGD=108,
   EID_HOMEPLUGCTL=109,
   EID_PLC_L2UPGRADE=110,
   EID_1905=111,
   EID_I5CTL=112,
   EID_NTPD=113,
   EID_TR69C_2=114,        /* TR69C instance 2 */
   EID_MDM_CMD=115,
   EID_MDM_TCPING=116,
   EID_HGTCPDUMP=117,
   EID_VOICE=150,
   EID_ECMS=160,
   EID_WLCTDM=161,
   EID_WLCTDM_TEST=162,
   EID_DECTDBGD=199,
   EID_SEND_CMS_MSG=280,
   EID_OSGID=500,              /* Begin BCM Modular Software EID range */
   EID_LINMOSD=510,
   EID_MODUPDTV=511,
   EID_PMD=512,
   EID_DAD=513,
   EID_OPENWRTD=514,
   EID_LXC_MONITOR=515,
   EID_DMAD=517,
   EID_DOCKERMD=519,
   EID_FIREWALLD=520,
   EID_MODSW_RESERVED_END=539, /* End BCM Modular Software EID range */
   EID_DOWNLOAD_DIAG=540,      /* Begin Diag reserved range          */
   EID_UPLOAD_DIAG=541,
   EID_UDPECHO=542,
   EID_TMSCTL=543,
   EID_SPEEDSVC=544,
   EID_NFCD=545,
   EID_WANCONF=546,
   EID_TMCTL_UTETS=547,
   EID_BDMF_UTETS=548,
   EID_PERIODICSTAT=550,
   EID_HAUTO=551,
   EID_BBCD=552, /*Broadband Commander Daemon*/
   EID_WLDIAG=553, /*Wifi Neighboring iDiagnostic*/
   EID_DIAG_RESERVED_END=559,  /* End Diag reserved range            */
   EID_BCM_USERSPACE_MAX=3999, /* End BCM userspace threads EID range */
   EID_BCM_KTHREAD_MIN=4000,   /* Begin BCM kernel threads EID range */
   EID_BCM_KTHREAD_MAX=4999,   /* End   BCM kernel threads EID range */
   EID_CUSTOMER_MIN=5000,      /* Customers can use EID starting from 5000 */
   EID_HBUSD=5001,             /*hbus daemon*/
   EID_WAND=5002,              /*wan daemon*/
   EID_MISCD=5003,             /*miscellaneous daemon*/
   EID_MDMD=5004,              /*mdm daemon*/   
   EID_VOICE_H248=5005, /* for voice h248 app */
   EID_MOBILEAPP=5006,
   EID_TYDAEMON=5007,
   EID_GWD=5008,
   EID_UPLOADTEST=5010, /* added by chenwenjin 2016-3-28*/
   EID_BUCPEABILITY=5020,/*yangtai add for  Universal broadband service ability 20161021*/
   EID_SIMULATION_SPEED=5021,/*yangtai for simulation speed 20170906*/
   EID_NATIVED=5022,             /* natived*/  
   EID_SSK_TY=5023,             /*move many things from ssk to ssk_ty*/ 
   EID_TYPING=5040,             /* added by LiYong */

   EID_WLSSK_TY=5041,
   /*Only for test use from EID_TEST_1 to EID_TEST_3. chenxi 2021-07-29 */
   EID_TEST_1=5101,             /*only for test use*/
   EID_TEST_2=5102,             /*only for test use*/
   EID_TEST_3=5103,             /*only for test use*/   

   EID_URLLOAD=5104,            /* bucpe speedtest application */
   
   EID_HAL=6001,
   EID_TEST_STUB=6002,
   EID_TEST_WAN=6003,
   EID_WAN_HAL=6004, 
   EID_WLAN_MONITOR=6005, 
   EID_CUSTOMER_MAX=9999,      /* Customers can use 5000-9999 */
   EID_RESERVED=10000,   /* keep EID's >= 10000 reserved for future use */
   EID_LAST=65535
} HG_LOG_EID;

typedef struct
{
    HG_LOG_EID hg_log_code;
    const char *hg_log_abbr;
} HG_EID;

#if 0
static HG_LOG_DEF hgLogDefTable[] = {
    {HG_LOG_START, NULL, NULL},
	{HG_LOG_RMS,    "RMS",    "remoteRmsOption"},
	{HG_LOG_PLUGIN, "PLUGIN", "mobilePluginOption"},
	{HG_LOG_WEB,    "WEB",    "lanWebOption"},
	{HG_LOG_OTHERS, "OTHERS", "othersOption"},
};
#elif 0

static HG_LOG_DEF hgLogDefTable[] = {
    [HG_LOG_RMS]
    { HG_LOG_RMS, "RMS", "remoteRmsOption" },
    // [HG_LOG_PLUGIN]
    // { HG_LOG_PLUGIN, "PLUGIN", "mobilePluginOption" },
    [HG_LOG_WEB]
    { HG_LOG_WEB, "WEB", "lanWebOption" },
    [HG_LOG_OTHERS]
    { HG_LOG_OTHERS, "OTHERS", "othersOption" },
};
#else
static HG_LOG_DEF hgLogDefTable[] = {
    [HG_LOG_RMS] =
        {HG_LOG_RMS, "RMS"},
    [HG_LOG_PLUGIN] =
        {HG_LOG_PLUGIN, "PLUGIN"},
    [HG_LOG_WEB] =
        {HG_LOG_WEB, "WEB"},
    [HG_LOG_OTHERS] =
        {HG_LOG_OTHERS, "OTHERS"},
};
#endif

char *hgs_get_log_abbr_name(void)
{
    return "ssssssss";
}

#define HI_CWMP_LOG(lvl, type, args...)                                                                 \
    {                                                                                                   \
        char buff[LOGMSG_TEXTLEN] = {0};                                                                \
        snprintf_s(buff, sizeof(buff), sizeof(buff) - 1, "[%s] " fmt, hgs_get_log_abbr_name(), ##args); \
        igdCmLogApi(buff, strlen(buff), LOGMSG_CWMP_MOD, lvl, type);                                    \
    }

static HG_EID hgLogEid[] = {
    [EID_TR69C] = {EID_TR69C, "RMS"},
    [EID_HTTPD] = {EID_HTTPD, "WEB"},
    [EID_SMD] = {EID_SMD, "SMD"},
    [EID_SSK] = {EID_SSK, "SSK"},
    [EID_MDM_CMD] = {EID_MDM_CMD, "CLI"},
    // [EID_MDM_TCPING] = {EID_MDM_TCPING, "TCPING"},
    [EID_HGTCPDUMP] = {EID_HGTCPDUMP, "HGTCPDUMP"},
    [EID_OSGID] = {EID_OSGID, "OSGI"},
    [EID_MISCD] = {EID_MISCD, "MISCD"},
    [EID_MDMD] = {EID_MDMD, "WEB"},
    [EID_GWD] = {EID_GWD, "PLUGIN"},
    [EID_SSK_TY] = {EID_SSK_TY, "SSK"},
    [EID_WLSSK_TY] = {EID_WLSSK_TY, "SSK"},
    // [EID_RESERVED] = {EID_RESERVED, "EID_RESERVED"},
    [EID_LAST] = {EID_LAST, "EID_RESERVED"},
};

int main(int argc, char const *argv[])
{
    // char str[1024] = {0};
    // char cmd[1024] = "1234567890";
    // snprintf(str, sizeof(str), sizeof(cmd), "%s", cmd);
    // printf("str = %s", str);
    // printf("%s\n", hgLogDefTable[HG_LOG_WEB].hg_log_abbr);
    printf("%s %d %d\n", hgLogEid[EID_LAST].hg_log_abbr, sizeof(hgLogEid)/sizeof(hgLogEid[0]), sizeof(hgLogEid));

    for (int i = 0; i < sizeof(hgLogEid)/sizeof(hgLogEid[0]); i++)
    {  
        // printf("%d ", hgLogEid[i]);
        if (hgLogEid[i].hg_log_abbr)
        {
            printf("%s\n", hgLogEid[i].hg_log_abbr);
        }
        // printf("\n");
        /* code */
    }
    

    int *i = "ssssss";
    int *j = "ssssss";
    // printf("%d\n", memcmp(i, j, strlen(i)));

    return 0;
}