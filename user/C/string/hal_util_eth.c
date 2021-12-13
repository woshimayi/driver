#include "fwk.h"
#include <errno.h>
#include "hal_util.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <net/if.h>
#include <linux/mii.h>

typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include "hal_util_eth.h"
#include "mtkswitch_api.h"
#ifdef JASON_DEBUG
	#include "bcmswapitypes.h"
	#include "generated/autoconf.h"
	#include "bcmnet.h"
	#include "bcm_vlan.h"
	#include "vlanctl_api.h"
#endif /* DESKTOP_LINUX */
#include "hal_gpon_mac.h"

#define err(A)

#define ETH_NET_DEV_FILE "/proc/net/dev"
#define ETH_BUFF_LEN 512
#define ETH_NUM_VALUE_STRING_LEN 32

#define WAN_GPON_IF_NAME_PREFIX "pon"
#define WAN_EPON_IF_NAME_PREFIX "pon"
#define LAN_IF_NAME_PREFIX "eth"
#define LAN_IF_NAME_PORT_BASE 0
#define WLAN_IF_NAME_PREFIX "wl" //Here, do not change this value to "ra"

#define WAN_PROTO_IPOE_STR "ipoe"
#define WAN_PROTO_PPPOE_STR "pppoe"
#define WAN_PROTO_BRIDGE_STR "bridge"

#define INTERFACE_RETRY_COUNT 50
#define USLEEP_COUNT 100000

#define VLANMUX_DISABLE -1
#define HAL_MAX_MCAST_IFNAMES 3

#define LAN_PORT_NUM 4 /* assume that we have four eth ports */
#define LAN_PORT_STATUS_FILE "/proc/tc3162/eth_port_status"

#define MIN_VLAN_ID 0
#define MAX_VLAN_ID 4096
#define VLAN_NAME_TYPE_FLAG "DEV_PLUS_VID" /* vconfig set name type flag */

#define TW_MCAST_UPDATE_PORT_VLAN_INFO 0x01
#define TW_MCAST_UPDATE_MAPING_VLAN_INFO 0x02
#define TW_BRIDGE_UPDATE_PORT_MAPING_WAN_INFO 0x03
#define TW_MCAST_UPDATE_PUBLIC_VLAN_INTERFACE 0x04


#define TW_MTK_OFF_PPP_FILE "/proc/tc3162/mtk_off_ppp"
#define TW_LAN_WAN_BIND_FILE "/proc/net/port_bind_kernel"

#define EMPTY_STRING(s)    ((s == NULL) || (*s == '\0'))

typedef struct
{
	char name[IFNAMSIZ];
	int swPort;
} HAL_ETH_PORT_INFO;


#ifndef DESKTOP_LINUX
//static HAL_ETH_PORT_INFO sg_portInfo[4] = {{"eth0", 1}, {"eth1", 2}, {"eth2", 3}, {"eth3", 4}};

static HAL_ETH_PORT_INFO portInfo[4] = {{"eth0", 0}, {"eth1", 1}, {"eth2", 2}, {"eth3", 3}};

static HAL_ETH_PORT_INFO portInfoReverse[4] = {{"eth0", 3}, {"eth1", 2}, {"eth2", 1}, {"eth3", 0}};

static HAL_ETH_MCAST_IF sg_mcastIfNames[HAL_MAX_MCAST_IFNAMES] =
{
	{
		.ifName = "gponm.aniR",
		.ifType = IF_TYPE_ROUTE,
	},
	{
		.ifName = "gponm.aniB",
		.ifType = IF_TYPE_BRIDGE,
	},
	{
		.ifName = "gponm.aniIB",
		.ifType = IF_TYPE_INTER_BRIDGE,
	}
};
#endif

typedef struct
{
	UINT32 used;
	char L3IfName[IFNAMSIZ];
	char vlanIfName[IFNAMSIZ];
} HAL_VLAN_ETH_NAME_LIST;


static HAL_VLAN_ETH_NAME_LIST sg_vlanEthNameList[UTIL_WAN_CONN_MAX_NUM] =
{
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},

	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}}
};

typedef struct
{
	UINT32 used;
	UINT32 action; //1->add, 2->delete
	char L3IfName[IFNAMSIZ];
	char lanIfName[IFNAMSIZ];
} HAL_LAN_MAP_L3IF_LIST;

typedef struct
{
	UINT32 used;
	char wanIfName[32];
	char mVlanIfName[32];
} HAL_VLAN_MAP_MC_WAN_LIST;


static HAL_VLAN_MAP_MC_WAN_LIST sg_vlanMapMcWanList[UTIL_WAN_CONN_MAX_NUM] =
{
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},

	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}},
	{0, {'\0'}, {'\0'}}
};

extern int bcm_phy_mode_set(int unit, int port, int speed, int duplex);
extern int bcm_phy_mode_get(int unit, int port, int *speed, int *duplex);

#define MAX_VALUE_OF_HEX   0xFFFFFF
#define CYCLE_VALUE_OF_HEX  0x1000000

VOS_RET_E Hal_increaseMacValue(UINT8 macAddr[6], SINT32 increase)
{
	int i = 0;
	char *p = NULL;
	unsigned long umacLow = 0;
	char macTmp[BUFLEN_16] = {0};
	char macLow[BUFLEN_16] = {0};
	char macHigh[BUFLEN_16] = {0};
	UINT8 macdst[6];
	VOS_RET_E ret = VOS_RET_SUCCESS;

	memset(macTmp, 0, sizeof(macTmp));
	memset(macLow, 0, sizeof(macLow));
	memset(macHigh, 0, sizeof(macHigh));

	/* backup mac addr to string */
	for (i = 0; i < 6; i++)
	{
		char buf[BUFLEN_4];

		memset(buf, 0, sizeof(buf));
		UTIL_SNPRINTF(buf, sizeof(buf) - 1, "%02X",  macAddr[i]);
		UTIL_STRNCAT(macTmp, buf, sizeof(macTmp));
	}

	UTIL_STRNCPY(macHigh, macTmp, sizeof(macHigh));
	UTIL_STRNCPY(macLow, "0x", sizeof(macLow));

	UTIL_STRNCAT(macLow, macHigh + 6, sizeof(macLow) - strlen(macLow));
	memset(macHigh + 6, 0, sizeof(macHigh) - 6);

	umacLow = strtoul(macLow, NULL, 16);

	if ((umacLow + increase) > MAX_VALUE_OF_HEX)
	{
		umacLow = umacLow + increase - CYCLE_VALUE_OF_HEX;
	}
	else
	{
		umacLow += increase;
	}

	memset(macLow, 0, sizeof(macLow));
	UTIL_SNPRINTF(macLow, sizeof(macLow), "%6lX", umacLow);

	for (i = 0; i < 6; i++)
	{
		if (macLow[i] == ' ')
		{
			macLow[i] = '0';
		}
	}

	/* generate the dest mac addr */
	memset(macTmp, 0, sizeof(macTmp));
	UTIL_SNPRINTF(macTmp, sizeof(macTmp), "%s%s", macHigh, macLow);
	p = macTmp;

	memset(&macdst, 0, sizeof(UINT8) * 6);

	for (i = 0; i < 6; i++)
	{
		char bufTmp[BUFLEN_4];
		memset(bufTmp, 0, sizeof(bufTmp));
		UTIL_STRNCPY(bufTmp, p, 3);
		macdst[i] = (UINT8)strtoul((char *)bufTmp, NULL, 16);
		macAddr[i] = macdst[i];
		p += 2;
	}

	return ret;
}


#ifndef DESKTOP_LINUX
static SINT32 hal_ethGetInterfaceIndex(const char *vlanDevName)
{
	struct ifreq ifr;
	SINT32 s = 0;

	if (vlanDevName == NULL)
		return -1;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}

	UTIL_STRNCPY(ifr.ifr_name, vlanDevName, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
	{
		close(s);
		return 0;
	}
	close(s);
	return ifr.ifr_ifindex;
}

static UBOOL8 hal_ethWaitInterfaceExists(const char *devName)
{
	SINT32 retry = 0;
	SINT32 ret;

	while (retry < INTERFACE_RETRY_COUNT)
	{
		if ((ret = hal_ethGetInterfaceIndex(devName)) <= 0)
		{
			usleep(USLEEP_COUNT);
			vosLog_error("devName [%s] not exist,retry %d, ret %d\n", devName, retry, ret);
			retry++;
		}
		else
		{
			return TRUE;
		}
	} /* while */
	return FALSE;
}


static UBOOL8 hal_configIfMacAddr(const char *devName, const UINT8 mac[6])
{
	SINT32 retry = 0;
	UINT8 getMac[6] = {0, 0, 0, 0, 0, 0};
	UINT8 tempMac[6] = {0, 0, 0, 0, 0, 0};
	char cmd[BUFLEN_256];

	while (retry < INTERFACE_RETRY_COUNT)
	{
		UTIL_getIfMacAddr((char *)devName, (char *)getMac);
		vosLog_debug("before set devName[%s] mac=[%02x:%02x:%02x:%02x:%02x:%02x] \n",
		             devName, getMac[0], getMac[1], getMac[2], getMac[3], getMac[4], getMac[5]);


		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd),
		              "ifconfig %s down && ifconfig %s hw ether %02x:%02x:%02x:%02x:%02x:%02x",
		              devName,
		              devName,
		              mac[0],
		              mac[1],
		              mac[2],
		              mac[3],
		              mac[4],
		              mac[5]);

		vosLog_debug("cmd = [%s] \n", cmd);
		UTIL_DO_SYSTEM_ACTION(cmd);

		UTIL_getIfMacAddr((char *)devName, (char *)getMac);

		vosLog_debug("after set devName[%s] mac=[%02x:%02x:%02x:%02x:%02x:%02x] \n",
		             devName, getMac[0], getMac[1], getMac[2], getMac[3], getMac[4], getMac[5]);

		if (memcmp(getMac, tempMac, 6) == 0)
		{
			usleep(USLEEP_COUNT);
			vosLog_error("set devName=[%s] mac fail retry=%d \n", devName, retry);
			retry++;
		}
		else
		{
			return TRUE;
		}
	} /* while */
	return FALSE;
}

#endif /* DESKTOP_LINUX */


#ifndef DESKTOP_LINUX
UBOOL8 hal_ethIsInterfaceExist(const char *ifcName)
{
	return (hal_ethGetInterfaceIndex(ifcName) > 0) ? TRUE : FALSE;
}
#endif /* DESKTOP_LINUX */


#ifdef DESKTOP_LINUX
static VOS_RET_E hal_ethSetIptvMethod(UBOOL8 enableCrossVlan)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
	/*
	    SINT32 fd = 0;
	    SINT32 rc = 0;

	    fd = open(BCM_GMP_MW_CHRDEV_FULLNAME, O_WRONLY);
	    if (fd < 0)
	    {
	        vosLog_error("open failed. fd=%d", fd);
	        return VOS_RET_INTERNAL_ERROR;
	    }

	    rc = ioctl(fd, enableCrossVlan ? BCM_GMP_MW_IPTV_VID_FILTER_ENABLE : BCM_GMP_MW_IPTV_VID_FILTER_DISABLE, 0);
	    if (rc < 0)
	    {
	        vosLog_error("ioctl failed. rc=%d", rc);
	        ret = VOS_RET_INTERNAL_ERROR;
	        goto Exit;
	    }

	    ret = VOS_RET_SUCCESS;

	Exit:
	    close(fd);*/
	return ret;
}
#endif


static VOS_RET_E hal_ethAddDelWanIpSubnet(const char *vlanIfName,
        UBOOL8 add,
        UBOOL8 isBridge,
        UBOOL8 isPPPoE,
        SINT32 vid)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif
}


/** Abstract              :Get interface link state.
 *
 * @param devName         :Interface name
 * @param interfaceState  :Pointer of result
 *
 * @return                :Success -- VOS_RET_SUCCESS, Failure -- VOS_RET_INTERNAL_ERROR.
 */
VOS_RET_E HAL_ethGetIfLinkState(const char *ifName, UBOOL8 *linkUp)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_INTERNAL_ERROR;
	FILE *fp = NULL;
	char string[16] = {0};
	int portNum = 0, i = 0;
	int lanStatus[LAN_PORT_NUM];

	if (NULL == ifName)
	{
		vosLog_error("Invalid args!");
		return VOS_RET_INVALID_ARGUMENTS;
	}

	fp = fopen(LAN_PORT_STATUS_FILE, "r");
	if (fp != NULL)
	{
		memset(string, 0, sizeof(string));
		fgets(string, sizeof(string), fp);
		portNum = sscanf(string, "%d %d %d %d", lanStatus, lanStatus + 1, lanStatus + 2,
		                 lanStatus + 3);
		fclose(fp);

		if (portNum == LAN_PORT_NUM)
		{
			char ethName[16];

			for (i = 0; i < LAN_PORT_NUM; i ++)
			{
				memset(ethName, 0, sizeof(ethName));
				HAL_ethGetLanIfName(i, ethName, sizeof(ethName));

				if (0 == util_strcmp(ifName, ethName))
				{
					*linkUp = (lanStatus[i] == 1 ? TRUE : FALSE);
					ret = VOS_RET_SUCCESS;
					break;
				}
			}
		}
	}

	return ret;
#endif /* DESKTOP_LINUX */
}

VOS_RET_E HAL_ethConvertWlanIntfName(char *ifName, int ifNameLen)
{
	if ((0 == util_strcasecmp(ifName, "wl0"))
	        || (0 == util_strcasecmp(ifName, "wl0.")))
	{
		UTIL_STRNCPY(ifName, "ra0", ifNameLen);
	}
	else if (0 == util_strncmp(ifName, "wl0.1", 5))
	{
		UTIL_STRNCPY(ifName, "ra1", ifNameLen);
	}
	else if (0 == util_strncmp(ifName, "wl0.2", 5))
	{
		UTIL_STRNCPY(ifName, "ra2", ifNameLen);
	}
	else if (0 == util_strncmp(ifName, "wl0.3", 5))
	{
		UTIL_STRNCPY(ifName, "ra3", ifNameLen);
	}
	return VOS_RET_SUCCESS;
}


static VOS_RET_E HAL_ethSetPubMvlanInterface(const char *vlanIfName)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else
	int ret = VOS_RET_SUCCESS;
	int fd = -1;

	if (NULL == vlanIfName)
	{
		return VOS_RET_INVALID_ARGUMENTS;
	}

	fd = open("/dev/tw_mcast", O_RDWR);
	if (fd < 0)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	vosLog_debug("vlanIfName[%s]\n", vlanIfName);
	ret = ioctl(fd, TW_MCAST_UPDATE_PUBLIC_VLAN_INTERFACE, (void *)vlanIfName);

	close(fd);

	return ret;

#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethAddFilterRules(const char *vlanIfName, const char *wanIfName)
{
	vosLog_debug("vlanIfName = %s, wanIfName = %s", vlanIfName, wanIfName);

	UTIL_DO_SYSTEM_ACTION("ebtables -D %s -p IPv4 -i %s --ip-proto igmp -j ACCEPT > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN,
	                      vlanIfName, wanIfName);
	UTIL_DO_SYSTEM_ACTION("ebtables -D %s -p IPv6 -i %s --ip6-icmpv6type 130 -j ACCEPT > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN,
	                      vlanIfName, wanIfName);
	UTIL_DO_SYSTEM_ACTION("ebtables -D %s -i %s -j DROP > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN, vlanIfName, wanIfName);
	UTIL_DO_SYSTEM_ACTION("ebtables -D %s -o %s -j DROP > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN, vlanIfName);

	//for loop
	UTIL_DO_SYSTEM_ACTION("ebtables -A %s -p IPv4 -i %s --ip-proto igmp -j ACCEPT > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN,
	                      vlanIfName, wanIfName);
	UTIL_DO_SYSTEM_ACTION("ebtables -A %s -p IPv6 -i %s --ip6-icmpv6type 130 -j ACCEPT > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN,
	                      vlanIfName, wanIfName);
	UTIL_DO_SYSTEM_ACTION("ebtables -A %s -i %s -j DROP > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN, vlanIfName, wanIfName);
	//when wan is other bridge, lan packets could not through pon interface of public mcast
	UTIL_DO_SYSTEM_ACTION("ebtables -A %s -o %s -j DROP > /var/err",
	                      UTIL_WAN_CONN_FWD_CHAIN, vlanIfName);

	HAL_ethSetPubMvlanInterface(vlanIfName);

	return VOS_RET_SUCCESS;
}


VOS_RET_E HAL_ethCounterMibDump(const int port, HAL_ETH_SW_MIBREGS *ethcounter)
{
	return VOS_RET_SUCCESS;
}


/** Abstract          :Get interface statistics.
 *
 * @param devName     :Interface name
 * @param statistics  :Pointer of result
 *
 * @return            :Success -- VOS_RET_SUCCESS, Failure -- VOS_RET_INTERNAL_ERROR.
 */
VOS_RET_E HAL_ethGetIfStatistics(const char *devName,
                                 HAL_ETH_STATISTICS_T *statistics)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	int count = 0;
	char *p = NULL;
	char line[ETH_BUFF_LEN], buf[ETH_BUFF_LEN];
	char dummy[ETH_NUM_VALUE_STRING_LEN];
	char rxByte[ETH_NUM_VALUE_STRING_LEN];
	char rxPacket[ETH_NUM_VALUE_STRING_LEN];
	char rxErr[ETH_NUM_VALUE_STRING_LEN];
	char rxDrop[ETH_NUM_VALUE_STRING_LEN];
	char txByte[ETH_NUM_VALUE_STRING_LEN];
	char txPacket[ETH_NUM_VALUE_STRING_LEN];
	char txErr[ETH_NUM_VALUE_STRING_LEN];
	char txDrop[ETH_NUM_VALUE_STRING_LEN];
	char ifName[32] = {0};

	if (NULL == devName)
	{
		statistics->rxByte = 0;
		statistics->rxPacket = 0;
		statistics->rxError = 0;
		statistics->rxDrop = 0;
		statistics->txByte = 0;
		statistics->txPacket = 0;
		statistics->txDrop = 0;
		statistics->txError = 0;

		vosLog_error("Dev name is NULL. \n");
		return VOS_RET_INTERNAL_ERROR;
	}

	UTIL_STRNCPY(ifName, devName, sizeof(ifName));

	HAL_ethConvertWlanIntfName(ifName, sizeof(ifName));

	/* getstats put device statistics into this file, read the stats */
	FILE *fs = fopen(ETH_NET_DEV_FILE, "r");
	if (NULL == fs)
	{
		vosLog_error("Open proc file failed. \n");
		return VOS_RET_INTERNAL_ERROR;
	}

	// find interface
	while (fgets(line, sizeof(line), fs))
	{
		/* read pass 2 header lines */
		if (count++ < 2)
		{
			continue;
		}

		/* normally line will have the following example value
		* "eth0: 19140785 181329 0 0 0 0 0 0 372073009 454948 0 0 0 0 0 0"
		* but when the number is too big then line will have the following example value
		* "eth0:19140785 181329 0 0 0 0 0 0 372073009 454948 0 0 0 0 0 0"
		* so to make the parsing correctly, the following codes are added
		* to insert space between ':' and number
		*/
		p = strchr(line, ':');
		if (NULL != p)
		{
			p++;
		}

		if ((NULL != p) && (isdigit(*p)))
		{
			UTIL_STRNCPY(buf, p, sizeof(buf));
			*p = ' ';
			UTIL_STRNCPY(++p, buf, sizeof(line));
		}

		/* if interface is found then store statistic values */
		if (NULL != strstr(line, ifName))
		{
			sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			       dummy, rxByte, rxPacket, rxErr, rxDrop, dummy, dummy, dummy, dummy,
			       txByte, txPacket, txErr, txDrop, dummy, dummy, dummy, dummy);
			util_strtoul(rxByte, NULL, 0, &statistics->rxByte);
			util_strtoul(rxPacket, NULL, 0, &statistics->rxPacket);
			util_strtoul(rxErr, NULL, 0, &statistics->rxError);
			util_strtoul(rxDrop, NULL, 0, &statistics->rxDrop);
			util_strtoul(txByte, NULL, 0, &statistics->txByte);
			util_strtoul(txPacket, NULL, 0, &statistics->txPacket);
			util_strtoul(txErr, NULL, 0, &statistics->txError);
			util_strtoul(txDrop, NULL, 0, &statistics->txDrop);

			break;
		} /* devName */
	} /* while */

	fclose(fs);

	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


/** Abstract          :Clear interface statistics.
 *
 * @param devName     :Interface name
 *
 * @return            :Success -- VOS_RET_SUCCESS, Failure -- VOS_RET_INTERNAL_ERROR.
 */
VOS_RET_E HAL_ethClearIfStatistics(const char *ifName)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}

#ifndef DESKTOP_LINUX
static int hal_ethFindPortNoByName(const char *name)
{
	int i = 0;

	for (i = 0; i < 4; i++)
	{
		if (0 == strcmp(name, portInfo[i].name))
			return portInfo[i].swPort;
	}

	return -1;
}

static int hal_ethFindPortNoByNameReverse(const char *name)
{
	int i = 0;

	for (i = 0; i < 4; i++)
	{
		if (0 == strcmp(name, portInfoReverse[i].name))
			return portInfoReverse[i].swPort;
	}

	return -1;
}

#endif /* DESKTOP_LINUX */

#ifndef DESKTOP_LINUX
	/*static int hal_ethFindSwPortNoByName(const char *name)
	{
	int i = 0;

	for (i = 0; i < 4; i++)
	{
	if(0 == strcmp(name, sg_portInfo[i].name))
	return sg_portInfo[i].swPort;
	}

	return -1;
	}*/
#endif /* DESKTOP_LINUX */

VOS_RET_E HAL_ethPortInit(void)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else
	int port = 0;
	UBOOL8 port_enable = FALSE;

	for (port = 0; port < 4 ; port++)
	{
		HAL_ethSetIfState(portInfo[port].name, port_enable);
	}
	port_enable = TRUE;
	for (port = 0; port < 4 ; port++)
	{
		HAL_ethSetIfState(portInfo[port].name, port_enable);
	}

	/*
	Soft Qdma Flag specification:
	bit 0:multicast, bit 1:ftp packet, bit 2:http download src port 80,
	bit 3:http download src port 8080, bit 6:tcp download src port 12345
	*/
	/*open ftp soft_qdma*/
	UTIL_DO_SYSTEM_ACTION("echo 2 > /proc/tc3162/soft_qdma ");

	return VOS_RET_SUCCESS;
#endif

}

/** Abstract       :Set interface mode.
 *
 * @param devName  :Interface name
 * @param mode     :Mode value
 *
 * @return         :Success -- VOS_RET_SUCCESS, Failure -- VOS_RET_INTERNAL_ERROR.
 */
VOS_RET_E HAL_ethSetIfNegMode(const char *devName, HAL_ETH_NEG_MODE_E mode)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */

#if 1
	VOS_RET_E ret = VOS_RET_SUCCESS;
	u8 speedMode = 0;
	int portIdx = -1;

	portIdx = hal_ethFindPortNoByNameReverse(devName);
	vosLog_debug(" devName = %s, mode = 0x%x  \n", devName, mode);
	if (portIdx < 0)
	{
		vosLog_error("no recognition devname:%s portIdx = %d ", devName, portIdx);
		return VOS_RET_INTERNAL_ERROR;
	}
	return VOS_RET_SUCCESS;

	switch (mode)
	{
		case  HAL_ETH_NEG_MODE_AUTO:
			speedMode = 0x0;
			break;
		case HAL_ETH_NEG_MODE_10M_HALF:
			speedMode = 0x11;
			break;
		case HAL_ETH_NEG_MODE_10M_FULL:
			speedMode = 0x01;
			break;
		case HAL_ETH_NEG_MODE_100M_HALF:
			speedMode = 0x12;
			break;
		case HAL_ETH_NEG_MODE_100M_FULL:
			speedMode = 0x02;
			break;
		case HAL_ETH_NEG_MODE_1000M:
			speedMode = 0x03;
			break;
		default:
			break;
	}

	vosLog_debug(" portIdx = %d, speedMode = 0x%x  \n", portIdx, speedMode);

	ret = macMT7530SetAutoDetection(speedMode, portIdx);

	return ret;
#else
	int swPortNo = 0;
	char cmd[100] = {0};

	swPortNo = hal_ethFindSwPortNoByName(devName);
	if (swPortNo == -1)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	return VOS_RET_SUCCESS;

	switch (mode)
	{
		case HAL_ETH_NEG_MODE_AUTO:
			snprintf(cmd, sizeof(cmd), "ethcmd eth0 media-type auto port %d &", swPortNo);
			break;
		case HAL_ETH_NEG_MODE_10M_HALF:
			snprintf(cmd, sizeof(cmd), "ethcmd eth0 media-type 10HD  port %d &", swPortNo);
			break;

		case HAL_ETH_NEG_MODE_10M_FULL:
			snprintf(cmd, sizeof(cmd), "ethcmd eth0 media-type 10FD  port %d &", swPortNo);
			break;

		case HAL_ETH_NEG_MODE_100M_HALF:
			snprintf(cmd, sizeof(cmd), "ethcmd eth0 media-type 100HD  port %d &", swPortNo);
			break;

		case HAL_ETH_NEG_MODE_100M_FULL:
			snprintf(cmd, sizeof(cmd), "ethcmd eth0 media-type 100FD  port %d &", swPortNo);
			break;

		case HAL_ETH_NEG_MODE_1000M:
			break;

		default:
			break;
	}

	if (cmd == NULL)
	{
		vosLog_error("Invalid parameter.");
	}
	else
	{
		vosLog_error("cmd:%s", cmd);
		system(cmd);
	}
	return VOS_RET_SUCCESS;
#endif
#endif /* DESKTOP_LINUX */
}


/** Abstract       :Get interface mode.
 *
 * @param devName  :Interface name
 * @param mode     :Pointer of mode value
 *
 * @return         :Success -- VOS_RET_SUCCESS, Failure -- VOS_RET_INTERNAL_ERROR.
 */
VOS_RET_E HAL_ethGetIfNegMode(const char *devName, HAL_ETH_NEG_MODE_E *mode)
{

#ifdef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */

#if 1
	VOS_RET_E ret = VOS_RET_SUCCESS;
	u8 sw_port = 0;
	int port_index = 0;
	u8 speedMode = 0;
	port_index = hal_ethFindPortNoByNameReverse(devName);
	vosLog_debug("port_index = %d  \n", port_index);

	if (port_index >= 0)
	{
		sw_port = (u8)port_index;
		ret = macMT7530GetPortSpeedMode(sw_port, &speedMode);
		vosLog_debug("ret = %u, speedMode = 0x%x  \n", ret, speedMode);
		switch (speedMode)
		{
			case 0x0:
				*mode = HAL_ETH_NEG_MODE_AUTO;
				break;
			case 0x01:
				*mode =  HAL_ETH_NEG_MODE_10M_FULL;
				break;
			case 0x02:
				*mode =  HAL_ETH_NEG_MODE_100M_FULL;
				break;
			case 0x03:
				*mode =  HAL_ETH_NEG_MODE_1000M;
				break;
			case 0x11:
				*mode =  HAL_ETH_NEG_MODE_10M_HALF;
				break;
			case 0x12:
				*mode =  HAL_ETH_NEG_MODE_100M_HALF;
				break;
			case 0x13:
				*mode =  HAL_ETH_NEG_MODE_1000M;
				break;
			default:
				vosLog_error("no recognition mode! mode = %d \n", mode);
				ret = VOS_RET_INTERNAL_ERROR;
				break;
		}
	}

	return ret;
#else
	int swPortNo = 0;
	int val = 0;
	int speed = 0;
	int duplex = 0;

	swPortNo = hal_ethFindSwPortNoByName(devName);

	if (swPortNo == -1)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	val = bcm_phy_mode_get(0, swPortNo, &speed, &duplex);

	if (duplex == 0)
	{
		if (speed == 0)
		{
			*mode = HAL_ETH_NEG_MODE_AUTO;
		}
		else if (speed == 10)
		{
			*mode = HAL_ETH_NEG_MODE_10M_FULL;
		}
		else if (speed == 100)
		{
			*mode = HAL_ETH_NEG_MODE_100M_FULL;
		}
		else if (speed == 1000)
		{
			*mode = HAL_ETH_NEG_MODE_1000M;
		}
	}
	else if (duplex == 1)
	{
		if (speed == 10)
		{
			*mode = HAL_ETH_NEG_MODE_10M_HALF;
		}
		else if (speed == 100)
		{
			*mode = HAL_ETH_NEG_MODE_100M_HALF;
		}
	}

	return val;
#endif

#endif /* DESKTOP_LINUX */

}

VOS_RET_E HAL_ethGetStatistics(const char *devName, unsigned long long *upBytes,
                               unsigned long long *downBytes)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
	u8 sw_port = 0;
	int port_index = 0;
	u32 outptr_upCnt = 0;
	u32 outptr_downCnt = 0;
	port_index = hal_ethFindPortNoByNameReverse(devName);
	vosLog_debug("port_index = %d  \n", port_index);

	if (port_index >= 0)
	{
		sw_port = (u8)port_index;
		macMT7530GetPortRxOctetsCnt(sw_port, &outptr_upCnt);
		macMT7530GetPortTxOctetsCnt(sw_port, &outptr_downCnt);
		*upBytes = outptr_upCnt;
		*downBytes = outptr_downCnt;
	}
	return ret;
}

VOS_RET_E HAL_ethSetIfState(const char *ifName, UBOOL8 enable)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;
	int swPortNo = 0;
	swPortNo = hal_ethFindPortNoByName(ifName);
	if (swPortNo == -1)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	HAL_ethSetPortState(swPortNo, enable);
	return ret;
#endif /* DESKTOP_LINUX */
}

VOS_RET_E HAL_ethGetWanIfName(UINT8 port, char *ifName, UINT32 ifNameLen)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (ifName == NULL)
	{
		vosLog_error("ifName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	if (SF_FEATURE_UPLINK_TYPE_EPON)
	{
		UTIL_SNPRINTF(ifName, ifNameLen, "%s", WAN_EPON_IF_NAME_PREFIX);
	}
	else
	{
		UTIL_SNPRINTF(ifName, ifNameLen, "%s", WAN_GPON_IF_NAME_PREFIX);
	}

	return ret;
}


static UINT8 hal_ethGetPortNameBase(void)
{
	return LAN_IF_NAME_PORT_BASE;
}


VOS_RET_E HAL_ethGetLanIfName(UINT8 port, char *ifName, UINT32 ifNameLen)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (ifName == NULL)
	{
		vosLog_error("ifName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	UTIL_SNPRINTF(ifName, ifNameLen, "%s%d", LAN_IF_NAME_PREFIX,
	              port + hal_ethGetPortNameBase());

	return ret;
}


VOS_RET_E HAL_ethGetLanPanelIfNo(UINT8 instId, UINT8 *panelNo)
{
	*panelNo = instId - 1; //nomally panel no is equal to phy port no
	return VOS_RET_SUCCESS;
}


VOS_RET_E HAL_ethGetWlanIfName(UINT8 port, char *ifName, UINT32 ifNameLen)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
	UINT32 devId = port / 4;
	port = port % 4;

	if (ifName == NULL)
	{
		vosLog_error("ifName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	if (port == 0)
	{
		UTIL_SNPRINTF(ifName, ifNameLen, "%s%d", WLAN_IF_NAME_PREFIX, devId);
	}
	else
	{
		UTIL_SNPRINTF(ifName, ifNameLen, "%s%d.%d", WLAN_IF_NAME_PREFIX, devId, port);
	}

	return ret;
}


VOS_RET_E HAL_ethGetWanVlanIfName(const char *ifName, SINT32 idx, SINT16 vid,
                                  UINT8 priority,
                                  char *vlanIfName, UINT32 vlanIfNameLen)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (ifName == NULL || vlanIfName == NULL)
	{
		vosLog_error("ifName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	UTIL_SNPRINTF(vlanIfName, vlanIfNameLen, "%s.%d", ifName, idx);

	return ret;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethGetLanVlanIfName(const char *ifName, SINT16 vid,
                                  char *vlanIfName, UINT32 vlanIfNameLen)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (ifName == NULL || vlanIfName == NULL)
	{
		vosLog_error("ifName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	UTIL_SNPRINTF(vlanIfName, vlanIfNameLen, "%s.%d", ifName, vid);

	return ret;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethGetLanDefVlanIfName(const char *ifName, char *vlanIfName,
                                     UINT32 vlanIfNameLen)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (ifName == NULL || vlanIfName == NULL)
	{
		vosLog_error("ifName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	UTIL_SNPRINTF(vlanIfName, vlanIfNameLen, "%s.%d", ifName, 0);

	return ret;
#endif /* DESKTOP_LINUX */
}

static VOS_RET_E HAL_ethSetLanMapWanList(HAL_LAN_MAP_L3IF_LIST *data)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	int ret = 0;
	int fd = -1;

	if (NULL == data)
		return VOS_RET_INVALID_ARGUMENTS;

	fd = open("/dev/tw_mcast", O_RDWR);
	if (fd < 0)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	vosLog_debug("lanIfName[%s] L3IfName[%s]\n", data->lanIfName, data->L3IfName);
	ret = ioctl(fd, TW_BRIDGE_UPDATE_PORT_MAPING_WAN_INFO, (void *)data);

	close(fd);

	return ret;

#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethCreateRouteWanVlanIf(const char *ifName, SINT32 idx,
                                      SINT16 vid, UINT8 priority, UBOOL8 isPppoe,
                                      UBOOL8 isInternet, const char *lanIf, UBOOL8 isBridge, UBOOL8 ipv6Enable)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;
	char vlanIfName[BUFLEN_64];
	char L3IfName[BUFLEN_64];
	char cmd[BUFLEN_256];
	UINT8 mac[18];
#ifdef MTK_SMUXCTL
	SINT32 vlanMode = 3; //vlan_mode[1~3:untag,transparent,tag]
	char protoStr[BUFLEN_16];
#else
	UINT16 macId;
	UINT16 i;
#endif
	char *pPath = NULL;
	char *saveStr = NULL;
	char fullPath[BUFLEN_1024] = {0};
	HAL_LAN_MAP_L3IF_LIST lanMapL3IfList;


	vosLog_debug("ifName=%s idx=%d vid=%d priority=%d isPppoe=%d isInternet %d",
	             ifName, idx, vid, priority, isPppoe,
	             isInternet);

	memset(vlanIfName, 0, sizeof(vlanIfName));
	memset(L3IfName, 0, sizeof(L3IfName));
	memset(mac, 0, 18);

#ifdef MTK_SMUXCTL
	if (-1 == vid)
	{
		vlanMode = 2; //vlan_mode[1~3:untag,transparent,tag]
	}
	else
	{
		vlanMode = 3; //vlan_mode[1~3:untag,transparent,tag]
	}

	if (TRUE == isPppoe)
	{
		UTIL_STRNCPY(protoStr, WAN_PROTO_PPPOE_STR, sizeof(protoStr));
	}
	else
	{
		UTIL_STRNCPY(protoStr, WAN_PROTO_IPOE_STR, sizeof(protoStr));
	}

	if ((ret = HAL_ethGetWanVlanIfName(ifName, idx, vid, priority, vlanIfName,
	                                   sizeof(vlanIfName))) != VOS_RET_SUCCESS)
	{
		vosLog_error("HAL_ethCreateRouteWanVlanIf error! ret = %d", ret);
		return ret;
	}

	if (hal_ethIsInterfaceExist(vlanIfName) == FALSE)
	{
		memset(cmd, 0, sizeof(cmd));
		//smuxctl add  [pppoe | ipoe | bridge] pvc_name if_name vlan_mode[1~3:untag,transparent,tag] vid pbit multicast_tci
		UTIL_SNPRINTF(cmd, sizeof(cmd), "smuxctl add %s %s %s %d %d %d %d",
		              protoStr,
		              ifName,
		              vlanIfName,
		              vlanMode,
		              ((vid == -1) ? 0 : vid),
		              priority,
		              0);

		UTIL_DO_SYSTEM_ACTION(cmd);
	}
#else

	ret = HAL_sysGetMacAddr(mac);
	if (VOS_RET_SUCCESS != ret)
	{
		vosLog_error("get baseMac failed, ret = %d", ret);
		return ret;
	}

	if (SF_FEATURE_SUPPORT_WLAN)
	{
		if (SF_FEATURE_SUPPORT_WLAN_5G)
			macId = idx + 3;
		else
			macId = idx + 2;
	}
	else
	{
		macId = idx + 1;
	}

	Hal_increaseMacValue(mac, macId);


	if (vid > MIN_VLAN_ID && vid < MAX_VLAN_ID)
	{
		memset(vlanIfName, 0, sizeof(vlanIfName));
		UTIL_SNPRINTF(vlanIfName, sizeof(vlanIfName), VLAN_DEV_FORMAT, ifName, vid);

		if (hal_ethIsInterfaceExist(vlanIfName) == FALSE)
		{
			memset(cmd, 0, sizeof(cmd));
			UTIL_SNPRINTF(cmd, sizeof(cmd), "vconfig set_name_type %s",
			              VLAN_NAME_TYPE_FLAG);
			UTIL_DO_SYSTEM_ACTION(cmd);

			memset(cmd, 0, sizeof(cmd));
			UTIL_SNPRINTF(cmd, sizeof(cmd), "vconfig add %s %d", ifName, vid);
			UTIL_DO_SYSTEM_ACTION(cmd);

			if (!hal_ethWaitInterfaceExists(vlanIfName))
			{
				vosLog_error("Failed to create %s", vlanIfName);
				return VOS_RET_INTERNAL_ERROR;
			}

#if 0
			memset(cmd, 0, sizeof(cmd));
			UTIL_SNPRINTF(cmd, sizeof(cmd),
			              "ifconfig %s down && ifconfig %s hw ether %02x:%02x:%02x:%02x:%02x:%02x",
			              vlanIfName,
			              vlanIfName,
			              mac[0],
			              mac[1],
			              mac[2],
			              mac[3],
			              mac[4],
			              mac[5]);
			UTIL_DO_SYSTEM_ACTION(cmd);
#else
			hal_configIfMacAddr(vlanIfName, mac);
#endif

			if (! ipv6Enable)
			{
				UTIL_DO_SYSTEM_ACTION("echo 1 > /proc/sys/net/ipv6/conf/%s/disable_ipv6 ",
				                      vlanIfName);
			}

			memset(cmd, 0, sizeof(cmd));
			UTIL_SNPRINTF(cmd, sizeof(cmd), "ifconfig %s up", vlanIfName);
			UTIL_DO_SYSTEM_ACTION(cmd);

			for (i = 0; i <= 7; i++)
			{
				//UTIL_DO_SYSTEM_ACTION("vconfig set_egress_map %s.%u %u %u",
				//                 ifName, vid, i + 20, i);
				UTIL_DO_SYSTEM_ACTION("vconfig set_egress_map %s %u %u",
				                      vlanIfName, i, priority);
			}

		}
	}
	else
	{
		//UTIL_SNPRINTF(vlanIfName, sizeof(vlanIfName), "%s", ifName);
		UTIL_STRNCPY(vlanIfName, ifName, sizeof(vlanIfName));
	}

	if ((ret = HAL_ethGetWanVlanIfName(ifName, idx, vid, priority, L3IfName,
	                                   sizeof(L3IfName))) != VOS_RET_SUCCESS)
	{
		vosLog_error("HAL_ethCreateRouteWanVlanIf error! ret = %d", ret);
		return ret;
	}

	if (hal_ethIsInterfaceExist(L3IfName) == FALSE)
	{
		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd), "vnetconfig add %s %s",
		              L3IfName,
		              vlanIfName);
		UTIL_DO_SYSTEM_ACTION(cmd);

		if (!hal_ethWaitInterfaceExists(L3IfName))
		{
			vosLog_error("Failed to create %s", L3IfName);
			return VOS_RET_INTERNAL_ERROR;
		}

#if 0
		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd),
		              "ifconfig %s down && ifconfig %s hw ether %02x:%02x:%02x:%02x:%02x:%02x",
		              L3IfName,
		              L3IfName,
		              mac[0],
		              mac[1],
		              mac[2],
		              mac[3],
		              mac[4],
		              mac[5]);
		UTIL_DO_SYSTEM_ACTION(cmd);
#else
		hal_configIfMacAddr(L3IfName, mac);
#endif

		for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
		{
			if ((0 == sg_vlanEthNameList[i].used))
			{
				memset(sg_vlanEthNameList[i].vlanIfName, 0,
				       sizeof(sg_vlanEthNameList[i].vlanIfName));
				UTIL_STRNCPY(sg_vlanEthNameList[i].vlanIfName, vlanIfName,
				             sizeof(sg_vlanEthNameList[i].vlanIfName));

				memset(sg_vlanEthNameList[i].L3IfName, 0,
				       sizeof(sg_vlanEthNameList[i].L3IfName));
				UTIL_STRNCPY(sg_vlanEthNameList[i].L3IfName, L3IfName,
				             sizeof(sg_vlanEthNameList[i].L3IfName));

				sg_vlanEthNameList[i].used = 1;
				break;
			}
		}

		if (SF_FEATURE_SUPPORT_LAN_WAN_BIND && isBridge && (NULL != lanIf))
		{
			UTIL_STRNCPY(fullPath, lanIf, sizeof(fullPath));
			pPath = strtok_r(fullPath, ",", &saveStr);

			UTIL_DO_SYSTEM_ACTION("echo 1 > %s", TW_LAN_WAN_BIND_FILE);

			while (NULL != pPath)
			{
				memset(lanMapL3IfList.lanIfName, 0, sizeof(lanMapL3IfList.lanIfName));
				UTIL_STRNCPY(lanMapL3IfList.lanIfName, pPath, sizeof(lanMapL3IfList.lanIfName));

				memset(lanMapL3IfList.L3IfName, 0, sizeof(lanMapL3IfList.L3IfName));
				UTIL_STRNCPY(lanMapL3IfList.L3IfName, L3IfName,
				             sizeof(lanMapL3IfList.L3IfName));

				lanMapL3IfList.action = 1; //add flag
				lanMapL3IfList.used = 1;

				HAL_ethSetLanMapWanList(&lanMapL3IfList);
				pPath = strtok_r(NULL, ",", &saveStr);
			}
		}
	}

	if (SF_FEATURE_LOCATION_RUSSIA)
	{
		if (isInternet)
		{
			UTIL_DO_SYSTEM_ACTION("echo 1 > %s", TW_MTK_OFF_PPP_FILE);
		}
	}
#endif

	return ret;

#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethDeletePubVlanIf(const char *wanIfName, const char *wanConnName)
{
	SINT32 ret = VOS_RET_SUCCESS;
	SINT32 i;
	SINT32 j;
	SINT32 n = 0;

	vosLog_debug("wanIfName = %s", wanIfName);

	for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
	{
		printf("before delete [index=%d] used=%d, wanIfName=[%s], mVlanIfName=[%s] \n",
		       i,
		       sg_vlanMapMcWanList[i].used,
		       sg_vlanMapMcWanList[i].wanIfName[0] ? sg_vlanMapMcWanList[i].wanIfName : "",
		       sg_vlanMapMcWanList[i].mVlanIfName[0] ? sg_vlanMapMcWanList[i].mVlanIfName :
		       "");
	}

	for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
	{
		if ((sg_vlanMapMcWanList[i].used)
		        && (!util_strcmp(sg_vlanMapMcWanList[i].wanIfName, wanIfName)))
		{
			for (j = 0; j < UTIL_WAN_CONN_MAX_NUM; j++)
			{
				if (!util_strcmp(sg_vlanMapMcWanList[j].mVlanIfName,
				                 sg_vlanMapMcWanList[i].mVlanIfName))
				{
					n++;
				}
			}

			if ((n == 1) && (hal_ethIsInterfaceExist(sg_vlanMapMcWanList[i].mVlanIfName)))
			{
				if (NULL != strstr(wanConnName, "OTHER_B")
				        || NULL != strstr(wanConnName, "INTERNET_B")
				        || NULL != strstr(wanConnName, "IPTV_B"))
				{
					UTIL_DO_SYSTEM_ACTION("ebtables -D %s -p IPv4 -i %s --ip-proto igmp -j ACCEPT > /var/err",
					                      UTIL_WAN_CONN_FWD_CHAIN,
					                      sg_vlanMapMcWanList[i].mVlanIfName);
					UTIL_DO_SYSTEM_ACTION("ebtables -D %s -p IPv6 -i %s --ip6-icmpv6type 130 -j ACCEPT > /var/err",
					                      UTIL_WAN_CONN_FWD_CHAIN,
					                      sg_vlanMapMcWanList[i].mVlanIfName);
					UTIL_DO_SYSTEM_ACTION("ebtables -D %s -i %s -j DROP > /var/err",
					                      UTIL_WAN_CONN_FWD_CHAIN,
					                      sg_vlanMapMcWanList[i].mVlanIfName);
					UTIL_DO_SYSTEM_ACTION("ebtables -D %s -o %s -j DROP > /var/err",
					                      UTIL_WAN_CONN_FWD_CHAIN,
					                      sg_vlanMapMcWanList[i].mVlanIfName);
				}
				UTIL_DO_SYSTEM_ACTION("ifconfig %s down", sg_vlanMapMcWanList[i].mVlanIfName);
				UTIL_DO_SYSTEM_ACTION("vconfig rem %s", sg_vlanMapMcWanList[i].mVlanIfName);

				sg_vlanMapMcWanList[i].used = 0;
				memset(sg_vlanMapMcWanList[i].wanIfName, 0,
				       sizeof(sg_vlanMapMcWanList[i].wanIfName));
				memset(sg_vlanMapMcWanList[i].mVlanIfName, 0,
				       sizeof(sg_vlanMapMcWanList[i].mVlanIfName));
			}
		}
	}

	for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
	{
		printf("after  delete [index=%d] used=%d, wanIfName=[%s], mVlanIfName=[%s] \n",
		       i,
		       sg_vlanMapMcWanList[i].used,
		       sg_vlanMapMcWanList[i].wanIfName[0] ? sg_vlanMapMcWanList[i].wanIfName : "",
		       sg_vlanMapMcWanList[i].mVlanIfName[0] ? sg_vlanMapMcWanList[i].mVlanIfName :
		       "");
	}


	return ret;
}


static VOS_RET_E HAL_ethSetRouteWanPubMvlan(HAL_VLAN_MAP_MC_WAN_LIST *data)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	int ret = 0;
	int fd = -1;

	if (NULL == data)
		return VOS_RET_INVALID_ARGUMENTS;

	fd = open("/dev/tw_mcast", O_RDWR);
	if (fd < 0)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	vosLog_debug("mVlanIfName[%s] wanIfName[%s]\n", data->mVlanIfName,
	             data->wanIfName);
	ret = ioctl(fd, TW_MCAST_UPDATE_MAPING_VLAN_INFO, (void *)data);

	close(fd);

	return ret;

#endif /* DESKTOP_LINUX */
}

VOS_RET_E HAL_ethCreatePubWanVlanIf(const char *ifName, SINT32 idx, SINT16 vid,
                                    UINT8 priority, UBOOL8 isPppoe,
                                    const char *wanConnName, const char *wanIfName)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;
	char vlanIfName[BUFLEN_64];
	char cmd[BUFLEN_256];
	UINT8 mac[18];
	UINT16 macId;
	UINT16 i;

	vosLog_debug("ifName=%s idx=%d vid=%d wanIfName=%s", ifName, idx, vid,
	             wanIfName);

	memset(vlanIfName, 0, sizeof(vlanIfName));
	memset(mac, 0, 18);

	HAL_ethSetPubMvlanInterface("no");

	if (vid > MIN_VLAN_ID && vid < MAX_VLAN_ID)
	{
		memset(vlanIfName, 0, sizeof(vlanIfName));
		UTIL_SNPRINTF(vlanIfName, sizeof(vlanIfName), VLAN_DEV_FORMAT, ifName, vid);
		vosLog_debug("vlanIfName=[%s]\n", vlanIfName);

		if (hal_ethIsInterfaceExist(vlanIfName) == TRUE)
		{
			if ((NULL != strstr(wanConnName, "INTERNET_R"))
			        || (NULL != strstr(wanConnName, "OTHER_B"))
			        || NULL != strstr(wanConnName, "INTERNET_B")
			        || NULL != strstr(wanConnName, "IPTV"))
			{
				vosLog_debug("wanIfName=[%s]\n", wanIfName);
				HAL_ethDeletePubVlanIf(wanIfName, wanConnName);
			}
		}

		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd), "vconfig set_name_type %s",
		              VLAN_NAME_TYPE_FLAG);
		UTIL_DO_SYSTEM_ACTION(cmd);

		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd), "vconfig add %s %d", ifName, vid);
		UTIL_DO_SYSTEM_ACTION(cmd);

		ret = HAL_sysGetMacAddr(mac);
		vosLog_debug("mac[%02x:%02x:%02x:%02x:%02x:%02x]\n", mac[0], mac[1], mac[2],
		             mac[3], mac[4], mac[5]);

		if (VOS_RET_SUCCESS != ret)
		{
			vosLog_error("get baseMac failed, ret = %d", ret);
			return ret;
		}

		if (SF_FEATURE_SUPPORT_WLAN)
		{
			if (SF_FEATURE_SUPPORT_WLAN_5G)
				macId = idx + 3;
			else
				macId = idx + 2;
		}
		else
		{
			macId = idx + 1;
		}

		Hal_increaseMacValue(mac, macId);
		vosLog_debug("mac[%02x:%02x:%02x:%02x:%02x:%02x]\n", mac[0], mac[1], mac[2],
		             mac[3], mac[4], mac[5]);

		if (!hal_ethWaitInterfaceExists(vlanIfName))
		{
			vosLog_error("Failed to create %s", vlanIfName);
			return VOS_RET_INTERNAL_ERROR;
		}

#if 0
		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd),
		              "ifconfig %s down && ifconfig %s hw ether %02x:%02x:%02x:%02x:%02x:%02x",
		              vlanIfName,
		              vlanIfName,
		              mac[0],
		              mac[1],
		              mac[2],
		              mac[3],
		              mac[4],
		              mac[5]);
		UTIL_DO_SYSTEM_ACTION(cmd);

#else
		hal_configIfMacAddr(vlanIfName, mac);
#endif

		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd), "ifconfig %s up", vlanIfName);
		UTIL_DO_SYSTEM_ACTION(cmd);


		for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
		{
			printf("before create [index=%d] used=%d, wanIfName=[%s], mVlanIfName=[%s] \n",
			       i,
			       sg_vlanMapMcWanList[i].used,
			       sg_vlanMapMcWanList[i].wanIfName[0] ? sg_vlanMapMcWanList[i].wanIfName : "",
			       sg_vlanMapMcWanList[i].mVlanIfName[0] ? sg_vlanMapMcWanList[i].mVlanIfName :
			       "");
		}


		for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
		{
			if ((0 == sg_vlanMapMcWanList[i].used))
			{
				memset(sg_vlanMapMcWanList[i].mVlanIfName, 0,
				       sizeof(sg_vlanMapMcWanList[i].mVlanIfName));
				UTIL_STRNCPY(sg_vlanMapMcWanList[i].mVlanIfName, vlanIfName,
				             sizeof(sg_vlanMapMcWanList[i].mVlanIfName));

				memset(sg_vlanMapMcWanList[i].wanIfName, 0,
				       sizeof(sg_vlanMapMcWanList[i].wanIfName));
				UTIL_STRNCPY(sg_vlanMapMcWanList[i].wanIfName, wanIfName,
				             sizeof(sg_vlanMapMcWanList[i].wanIfName));

				sg_vlanMapMcWanList[i].used = 1;

				break;
			}
		}

		for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
		{
			printf("after  create [index=%d] used=%d, wanIfName=[%s], mVlanIfName=[%s] \n",
			       i,
			       sg_vlanMapMcWanList[i].used,
			       sg_vlanMapMcWanList[i].wanIfName[0] ? sg_vlanMapMcWanList[i].wanIfName : "",
			       sg_vlanMapMcWanList[i].mVlanIfName[0] ? sg_vlanMapMcWanList[i].mVlanIfName :
			       "");
		}

		if ((NULL != strstr(wanConnName, "OTHER_B"))
		        || (NULL != strstr(wanConnName, "IPTV_B")))
		{
			memset(cmd, 0, sizeof(cmd));
			UTIL_SNPRINTF(cmd, sizeof(cmd), "brctl addif br1 %s ", vlanIfName);
			UTIL_DO_SYSTEM_ACTION(cmd);

			HAL_ethAddFilterRules(vlanIfName, wanIfName);
		}
		else if (NULL != strstr(wanConnName, "INTERNET_B"))
		{
			memset(cmd, 0, sizeof(cmd));
			UTIL_SNPRINTF(cmd, sizeof(cmd), "brctl addif br0 %s ", vlanIfName);
			UTIL_DO_SYSTEM_ACTION(cmd);

			HAL_ethAddFilterRules(vlanIfName, wanIfName);
		}
		else if ((NULL != strstr(wanConnName, "INTERNET_R"))
		         || (NULL != strstr(wanConnName, "IPTV_R")))
		{
			HAL_ethSetRouteWanPubMvlan(&sg_vlanMapMcWanList[i]);
		}
	}
	else if (vid == -1)
	{
		HAL_ethDeletePubVlanIf(wanIfName, wanConnName);
		HAL_ethSetPubMvlanInterface("no");
	}
	else
	{
		ret = VOS_RET_INVALID_ARGUMENTS;
		vosLog_error("#######VOS_RET_INVALID_ARGUMENTS, vid:%d#####\n", vid);
	}

	return ret;

#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethCreateRouteIphostVlanIf(const char *ifName,
        const char *vlanIfName, SINT16 vid, UINT8 priority,
        UBOOL8 isPppoe)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;
	UINT32 tagRuleId = VLANCTL_DONT_CARE;
	char ifName1[32] = {0}, vlanIfName1[32] = {0};
	vlanCtl_init();
	vlanCtl_setIfSuffix(".");
	strncpy(vlanIfName1, vlanIfName, strlen(vlanIfName));
	strncpy(ifName1, ifName, strlen(ifName));
	/* Create untagged virtual interface */
	vlanCtl_createVlanInterfaceByName(ifName1, (char *)vlanIfName1, TRUE, 1);
	vlanCtl_setRealDevMode(ifName1, BCM_VLAN_MODE_ONT);

	if (!hal_ethWaitInterfaceExists(vlanIfName1))
	{
		vosLog_error("Failed to create %s", vlanIfName1);
		return VOS_RET_INTERNAL_ERROR;
	}

	if (vid != VLANMUX_DISABLE)
	{
		/* ******** tagged virtual interface ******** */

		/* ======== Set tx rules ======== */

		vlanCtl_initTagRule();

		/* Match the transmitting VOPI against vlanIfName */
		vlanCtl_filterOnTxVlanDevice(vlanIfName1);

		/* If hit, push an outer tag */
		vlanCtl_cmdPushVlanTag();

		/* Set pbits and vid in tag number 0, which is always the outer tag of the frame. */
		vlanCtl_cmdSetTagVid(vid, 0);
		vlanCtl_cmdSetTagPbits(priority, 0);

		/* Set rule to the top of the non-tag tx tables. */
		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_TX,
		                      0, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);

		vlanCtl_initTagRule();

		/* Match the transmitting VOPI against vlanIfName */
		vlanCtl_filterOnTxVlanDevice(vlanIfName1);

		/* If hit, push an outer tag */
		//vlanCtl_cmdPushVlanTag();

		/* Set pbits and vid in tag number 0, which is always the outer tag of the frame. */
		vlanCtl_cmdSetTagVid(vid, 0);
		vlanCtl_cmdSetTagPbits(priority, 0);

		/* Set rule to the top of the single-tag tx tables. */
		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_TX,
		                      1, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);

		/* ------------------------ */
		vlanCtl_initTagRule();

		/* Match the transmitting VOPI against vlanIfName */
		vlanCtl_filterOnTxVlanDevice(vlanIfName1);

		/* Match vid value of VLAN header against 0 */
		vlanCtl_filterOnTagVid(0, 0);

		/* If hit, set vid in tag number 0, which is always the outer tag of the frame. */
		vlanCtl_cmdSetTagVid(vid, 0);

		/* Set rule to the top of the single-tag tx tables. */
		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_TX,
		                      1, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);

		/* ======== Set rx rules ======== */

		/* Note: Always set bridge interface rx rules at the bottom of the tables
		 * using VLANCTL_POSITION_APPEND. Always set route interface rx rules at
		 * the top of the tables using VLANCTL_POSITION_BEFORE.
		 */

		vlanCtl_initTagRule();

		/* Set rx vlan interface for this rule */
		vlanCtl_setReceiveVlanDevice(vlanIfName1);

		if (SF_FEATURE_UPLINK_TYPE_GPON)
		{
			//set default action of veip0 rx direction as drop
			vlanCtl_setDefaultAction(ifName1, VLANCTL_DIRECTION_RX, 0, VLANCTL_ACTION_DROP,
			                         NULL);
			vlanCtl_setDefaultAction(ifName1, VLANCTL_DIRECTION_RX, 1, VLANCTL_ACTION_DROP,
			                         NULL);
			vlanCtl_setDefaultAction(ifName1, VLANCTL_DIRECTION_RX, 2, VLANCTL_ACTION_DROP,
			                         NULL);
		}

		/* Filter on receive interface and not allow for multicast.
		 * If hit, drop the frame.
		 */
		vlanCtl_filterOnVlanDeviceMacAddr(0);
		vlanCtl_cmdDropFrame();

#if 0
		/* Set rule to the top of the non-tag, single-tag and double-tag rx tables. */
		vlanCtl_insertTagRule(ifName, VLANCTL_DIRECTION_RX,
		                      0, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);

		vlanCtl_insertTagRule(ifName, VLANCTL_DIRECTION_RX,
		                      1, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);
#endif

		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_RX,
		                      2, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);

		vlanCtl_initTagRule();

		/* Set rx vlan interface for this rule */
		vlanCtl_setReceiveVlanDevice(vlanIfName1);

		/* Filter on receive interface and allow for multicast..
		* Filter on vid of tag number 0, which is always the outer tag of the frame.
		* If hit, pop the outer tag of the frame and forward it to the rx vlan interface.
		*/
		vlanCtl_filterOnVlanDeviceMacAddr(1);
		vlanCtl_filterOnTagVid(vid, 0);
		vlanCtl_cmdPopVlanTag();

		/* Set rule to the top of the single-tag and double-tag rx tables. */
		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_RX,
		                      1, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);
		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_RX,
		                      2, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);
	}
	else
	{
		/* ******** untagged virtual interface ******** */
		vlanCtl_initTagRule();

		/* Set rx vlan interface for this rule */
		vlanCtl_setReceiveVlanDevice(vlanIfName1);

		/* Filter on receive interface and allow for multicast frame.
		* If hit, forward the frame to the rx vlan interface.
		*/
		vlanCtl_filterOnVlanDeviceMacAddr(1);

		/* Set rule to the top of the non-tag rx tables. */
		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_RX,
		                      0, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);

		/* ------------------------ */
		vlanCtl_initTagRule();

		/* Set rx vlan interface for this rule */
		vlanCtl_setReceiveVlanDevice(vlanIfName1);

		/* Filter on receive interface and not allow for multicast.
		* If hit, drop the frame.
		*/
		vlanCtl_filterOnVlanDeviceMacAddr(0);
		vlanCtl_cmdDropFrame();

#if 0
		/* Set rule to the top of the single-tag and double-tag rx tables. */
		vlanCtl_insertTagRule(ifName, VLANCTL_DIRECTION_RX,
		                      1, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);
#endif

		vlanCtl_insertTagRule(ifName1, VLANCTL_DIRECTION_RX,
		                      2, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);
	}

	if (SF_FEATURE_SUPPORT_TR247)
	{
		vlanCtl_createVlanFlows(HAL_GPON_DEF_IFNAME, vlanIfName1);
		vlanCtl_createVlanFlows(vlanIfName1, HAL_GPON_DEF_IFNAME);

		UTIL_DO_SYSTEM_ACTION("bs /b/c port/index=wan0 cfg={sal=yes,dal=yes,dal_miss_action=host}");
	}
	printf("vlanCtl_createVlanFlows %s %s\n", HAL_GPON_DEF_IFNAME, vlanIfName1);

	vlanCtl_cleanup();

	return ret;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethCreateBridgeWanVlanIf(const char *ifName, SINT32 idx,
                                       SINT16 vid, UINT8 priority, UINT32 tpid,
                                       HAL_VLAN_MODE_E vlanMode, const char *lanIf, UBOOL8 ipv6Enable)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret;
	UBOOL8 isBridge = TRUE;

	ret = HAL_ethCreateRouteWanVlanIf(ifName, idx, vid, priority, 0, 0, lanIf,
	                                  isBridge, ipv6Enable);

	return ret;
#endif
}


VOS_RET_E HAL_ethDelRouteWanVlanIf(const char *vlanIfName, SINT32 idx,
                                   SINT16 vid, UINT8 priority)
{
	vosLog_error("vlanIfName = %s, idx = %d, vid = %d, priority = %u", vlanIfName,
	             idx, vid, priority);
	hal_ethAddDelWanIpSubnet(vlanIfName, FALSE, FALSE, TRUE, vid);
	return HAL_ethDeleteVlanIf(vlanIfName);
}


VOS_RET_E HAL_ethDelBridgeWanVlanIf(const char *vlanIfName, SINT32 idx,
                                    SINT16 vid, UINT8 priority, const char *lanIf)
{
	char *pPath = NULL;
	char *saveStr = NULL;
	char fullPath[BUFLEN_1024] = {0};
	HAL_LAN_MAP_L3IF_LIST lanMapL3IfList;

	vosLog_debug("vlanIfName = %s, idx = %d, vid = %d, priority = %u", vlanIfName,
	             idx, vid, priority);
	hal_ethAddDelWanIpSubnet(vlanIfName, FALSE, TRUE, TRUE, vid);

	if (SF_FEATURE_SUPPORT_LAN_WAN_BIND && (NULL != lanIf))
	{
		UTIL_STRNCPY(fullPath, lanIf, sizeof(fullPath));
		pPath = strtok_r(fullPath, ",", &saveStr);

		while (NULL != pPath)
		{
			lanMapL3IfList.action = 2;
			UTIL_STRNCPY(lanMapL3IfList.lanIfName, pPath, sizeof(lanMapL3IfList.lanIfName));
			UTIL_STRNCPY(lanMapL3IfList.L3IfName, vlanIfName,
			             sizeof(lanMapL3IfList.L3IfName));

			HAL_ethSetLanMapWanList(&lanMapL3IfList);

			pPath = strtok_r(NULL, ",", &saveStr);
		}
	}

	return HAL_ethDeleteVlanIf(vlanIfName);
}


VOS_RET_E HAL_ethDeleteVlanIf(const char *vlanIfName)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	SINT32 ret = VOS_RET_SUCCESS;
	SINT32 i;
	SINT32 j;
	SINT32 n = 0;
	char bridgeEmulatorIfName[BUFLEN_32] = {0};

	vosLog_error("vlanIfName = %s", vlanIfName);

	/* bridgeEmulatorIfName: pon.16 */
	UTIL_SNPRINTF(bridgeEmulatorIfName, sizeof(bridgeEmulatorIfName), "pon.%d",
	              IFC_WAN_MAX);

#ifdef MTK_SMUXCTL
	if (SF_AND_IF(SF_FEATURE_UPLINK_TYPE_GPON || SF_FEATURE_UPLINK_TYPE_EPON)
	        hal_ethIsInterfaceExist(vlanIfName)
	        SF_AND_ENDIF)
	{
		UTIL_DO_SYSTEM_ACTION("smuxctl rem %s", vlanIfName);
	}
#else
	if (SF_AND_IF(SF_FEATURE_UPLINK_TYPE_GPON || SF_FEATURE_UPLINK_TYPE_EPON)
	        hal_ethIsInterfaceExist(vlanIfName)
	        SF_AND_ENDIF)
	{
		if (0 == util_strncmp(vlanIfName, WAN_EPON_IF_NAME_PREFIX,
		                      strlen(WAN_EPON_IF_NAME_PREFIX)))
		{
			UTIL_DO_SYSTEM_ACTION("vnetconfig del %s", vlanIfName);

			for (i = 0; i < UTIL_WAN_CONN_MAX_NUM; i++)
			{
				if ((sg_vlanEthNameList[i].used)
				        && (!util_strcmp(sg_vlanEthNameList[i].L3IfName, vlanIfName)))
				{
					/* For bridge emulator if name info, should clear all of them */
					if (!util_strcmp(sg_vlanEthNameList[i].L3IfName, bridgeEmulatorIfName))
					{
						sg_vlanEthNameList[i].used = 0;
						memset(sg_vlanEthNameList[i].L3IfName, 0,
						       sizeof(sg_vlanEthNameList[i].L3IfName));
						memset(sg_vlanEthNameList[i].vlanIfName, 0,
						       sizeof(sg_vlanEthNameList[i].vlanIfName));
					}
					else
					{
						for (j = 0; j < UTIL_WAN_CONN_MAX_NUM; j++)
						{
							if (!util_strcmp(sg_vlanEthNameList[j].vlanIfName,
							                 sg_vlanEthNameList[i].vlanIfName))
							{
								n++;
							}
						}

						if (SF_FEATURE_CUSTOMER_JISHIHUITONG)
						{
							if ((n == 1) && (hal_ethIsInterfaceExist(sg_vlanEthNameList[i].vlanIfName)))
							{
								UTIL_DO_SYSTEM_ACTION("vconfig rem %s", sg_vlanEthNameList[i].vlanIfName);
							}
							sg_vlanEthNameList[i].used = 0;
							memset(sg_vlanEthNameList[i].L3IfName, 0,
							       sizeof(sg_vlanEthNameList[i].L3IfName));
							memset(sg_vlanEthNameList[i].vlanIfName, 0,
							       sizeof(sg_vlanEthNameList[i].vlanIfName));
						}
						else
						{
							if ((n == 1) && (hal_ethIsInterfaceExist(sg_vlanEthNameList[i].vlanIfName)))
							{
								UTIL_DO_SYSTEM_ACTION("vconfig rem %s", sg_vlanEthNameList[i].vlanIfName);

								sg_vlanEthNameList[i].used = 0;
								memset(sg_vlanEthNameList[i].L3IfName, 0,
								       sizeof(sg_vlanEthNameList[i].L3IfName));
								memset(sg_vlanEthNameList[i].vlanIfName, 0,
								       sizeof(sg_vlanEthNameList[i].vlanIfName));
							}
						}
					}
				}
			}
		}
		else
		{
			UTIL_DO_SYSTEM_ACTION("vconfig rem %s", vlanIfName);
		}
	}
#endif

	return ret;
#endif /* DESKTOP_LINUX */
}

VOS_RET_E HAL_ethSetWanIgmpState(const char *vlanIfName, const UBOOL8 isBridge,
                                 SINT16 vid, UBOOL8 enable)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;
	char wanIfName[BUFLEN_32] = {0};
	UINT32 tagRuleId = VLANCTL_DONT_CARE;

	if (vlanIfName == NULL)
	{
		vosLog_error("vlanIfName is NULL!");
		return VOS_RET_INVALID_PARAM_VALUE;
	}
	if (SF_FEATURE_SUPPORT_TR247)
	{
		if (util_strlen(vlanIfName) >= 5)
		{
			memcpy(wanIfName, vlanIfName, 5);
			wanIfName[5] = '\0';
		}

	}
	else
	{
		if ((ret = HAL_ethGetWanIfName(0, wanIfName,
		                               sizeof(wanIfName))) != VOS_RET_SUCCESS)
		{
			vosLog_error("HAL_ethGetWanIfName error! ret = %d", ret);
			return ret;
		}
	}
	vlanCtl_init();
	if (enable)
	{
		vlanCtl_initTagRule();

		vlanCtl_setReceiveVlanDevice((char *)vlanIfName);
		vlanCtl_filterOnTagVid(vid, 0);
		if (!isBridge)
		{
			vlanCtl_filterOnIpProto(2);
		}
		vlanCtl_cmdPopVlanTag();

		/* Set rule to the top of the single-tag and double-tag rx tables. */
		vlanCtl_insertTagRule(wanIfName, VLANCTL_DIRECTION_RX,
		                      1, VLANCTL_POSITION_BEFORE,
		                      VLANCTL_DONT_CARE, &tagRuleId);
	}

	vlanCtl_cleanup();

	return ret;
#endif /* DESKTOP_LINUX */
}



VOS_RET_E HAL_ethSetWanMldState(const char *vlanIfName, UBOOL8 enable)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


#ifndef DESKTOP_LINUX
VOS_RET_E hal_ethDelGponTagRuleById(const char *devInterface, SINT32 ruleId,
                                    SINT32 vid)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (ruleId != -1)
	{
		vlanCtl_init();
		ret = vlanCtl_removeTagRule((char *)devInterface,
		                            VLANCTL_DIRECTION_RX,
		                            1,
		                            ruleId);
		//        1);

		vosLog_notice("===> vlanCtl_removeTagRule, realLanIf=%s, oldVlan=%d, dir=%d, nbrOfTags=%d, tagRuleId=%d, ret=%d\n",
		              devInterface,
		              vid,
		              VLANCTL_DIRECTION_RX,
		              1,
		              ruleId,
		              ret);

		vlanCtl_cleanup();
	}

	return ret;
#endif
}

VOS_RET_E hal_ethDelOldGponTagRule(const char *devInterface,
                                   const char *wanConnName)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
	char temp[128] = {0};
	SINT32 oldTagRuleId = -1;
	SINT32 oldVlan = 0;
	FILE *fp = NULL;

	fp = fopen("/tmp/mcastAniFilterTagPopRule", "r");

	if (fp == NULL)
	{
		printf("Open /tmp/mcastAniFilterTagPopRule failed!...................\n");
		return VOS_RET_OPEN_FILE_ERROR;
	}

	while ((fgets(temp, 1024, fp)) != NULL)
	{
		if (strstr(temp, (wanConnName + 2)) != NULL)
		{
			strtok(temp, " ");
			oldVlan = atoi(strtok(NULL, " "));
			oldTagRuleId = atoi(strtok(NULL, " "));
			ret = hal_ethDelGponTagRuleById(devInterface, oldTagRuleId, oldVlan);
			if (VOS_RET_SUCCESS != ret)
			{
				vosLog_error("Fail to remove tag rule Id %d.\n", oldTagRuleId);
				/* Should we return ? */
			}
		}
	}

	fclose(fp);

	return VOS_RET_SUCCESS;
}


SINT32 hal_ethGetGponTagRuleId(const char *wanConnName)
{
	char temp[128] = {0};
	SINT32 oldTagRuleId = -1;
	SINT32 oldVlan = 0;
	FILE *fp = NULL;

	fp = fopen("/tmp/mcastAniFilterTagPopRule", "r");

	if (fp == NULL)
	{
		printf("Open /tmp/mcastAniFilterTagPopRule failed!...................\n");
		return -1;
	}

	while ((fgets(temp, 1024, fp)) != NULL)
	{
		if (strstr(temp, (wanConnName + 2)) != NULL)
		{
			strtok(temp, " ");
			oldVlan = atoi(strtok(NULL, " "));
			oldTagRuleId = atoi(strtok(NULL, " "));
			break;
		}
	}

	fclose(fp);

	return oldTagRuleId;
}

#ifdef DESKTOP_LINUX
/* record wan conn multicast vlan rule file: /tmp/mcastAniFilterTagPopRule */
static SINT32 hal_ethAddGponMcastVlanRule(const char *wanConnName,
        UINT32 vlanId, UINT32 tagRuleId)
{
	char temp[128] = {0};
	FILE *fp = NULL;

	fp = fopen("/tmp/mcastAniFilterTagPopRule", "a+");

	if (fp == NULL)
		return -1;

#if 0
	while ((fgets(temp, 1024, fp)) != NULL)
	{
		if (strstr(temp, (wanConnName + 2)) != NULL)
		{
			fclose(fp);
			return 0;
		}

		memset(temp, 0, sizeof(temp));
	}
#endif

	memset(temp, 0, sizeof(temp));
	UTIL_SNPRINTF(temp, sizeof(temp), "%s %d %d\r\n", (wanConnName + 2), vlanId,
	              tagRuleId);
	fputs(temp, fp);

	fclose(fp);

	return 0;
}
#endif


#ifdef DESKTOP_LINUX
static SINT32 hal_ethDelGponMcastVlanRule(const char *wanConnName)
{
	char trunkName[64] = {0};
	char temp[128] = {0};
	FILE *fp1, *fp2;

	fp1 = fopen("/tmp/mcastAniFilterTagPopRule", "a+");
	if (fp1 == NULL)
		return -1;

	fp2 = fopen("/tmp/mcastAniFilterTagPopRule.tmp", "w+");
	if (fp2 == NULL)
	{
		fclose(fp1);
		return -1;
	}


	UTIL_SNPRINTF(trunkName, sizeof(trunkName), "%s", (wanConnName + 2));

	while ((fgets(temp, 1024, fp1)) != NULL)
	{
		if (strstr(temp, trunkName) == NULL)
			fputs(temp, fp2);

		memset(temp, 0, sizeof(temp));
	}

	fclose(fp1);
	fclose(fp2);

	UTIL_DO_SYSTEM_ACTION("rm -rf /tmp/mcastAniFilterTagPopRule");
	UTIL_DO_SYSTEM_ACTION("mv /tmp/mcastAniFilterTagPopRule.tmp /tmp/mcastAniFilterTagPopRule");

	return 0;
}
#endif
#endif /* DESKTOP_LINUX */


VOS_RET_E HAL_ethGetMcastIfByIdx(HAL_ETH_MCAST_IF *mcastIf, UINT32 *idx)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_INVALID_ARGUMENTS;
#else
	if (*idx >= HAL_MAX_MCAST_IFNAMES)
	{
		vosLog_debug("Index is invalid.\n");
		return VOS_RET_INVALID_ARGUMENTS;
	}

	memcpy(mcastIf, &sg_mcastIfNames[*idx], sizeof(HAL_ETH_MCAST_IF));
	(*idx)++;

	return VOS_RET_SUCCESS;
#endif
}


VOS_RET_E HAL_ethSetWanMvlan(const char *vlanIfName, SINT32 priVid,
                             SINT32 pubVid, UBOOL8 isBridge, UBOOL8 isInternet,
                             const char *ifname)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	int idx = 0;
	vosLog_debug("vlanIfName[%s] ifname[%s]\n", vlanIfName, ifname);

	if (NULL != vlanIfName)
	{
		if (SF_FEATURE_LOCATION_BEIJING && SF_FEATURE_ISP_CU)
		{
			if (NULL != strstr(vlanIfName, "IPTV"))
			{
				idx = ifname[4] - '0' + 8;
				vosLog_debug("idx [%d]\n", idx);
				//epon interface and gpon interface is the same
				HAL_ethCreatePubWanVlanIf(WAN_EPON_IF_NAME_PREFIX, idx, pubVid, 0, 0,
				                          vlanIfName, ifname);
			}
		}
		else
		{
			if ((NULL != strstr(vlanIfName, "OTHER_B"))
			        || (NULL != strstr(vlanIfName, "INTERNET_R"))
			        || (NULL != strstr(vlanIfName, "IPTV"))
			        || (NULL != strstr(vlanIfName, "INTERNET_B")))
			{
				idx = ifname[4] - '0' + 8;
				vosLog_debug("idx [%d]\n", idx);
				//epon interface and gpon interface is the same
				HAL_ethCreatePubWanVlanIf(WAN_EPON_IF_NAME_PREFIX, idx, pubVid, 0, 0,
				                          vlanIfName, ifname);
			}
		}
	}

	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethSetWanPubMvlan(void *vlanInfo)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	int ret = 0;
	int fd = -1;

	if (NULL == vlanInfo)
		return VOS_RET_INVALID_ARGUMENTS;

	fd = open("/dev/tw_mcast", O_RDWR);
	if (fd < 0)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	ret = ioctl(fd, TW_MCAST_UPDATE_PORT_VLAN_INFO, vlanInfo);

	close(fd);

	return ret;

#endif /* DESKTOP_LINUX */
}

VOS_RET_E HAL_ethSetWanMCastvlan(const char *vlanIfName, SINT32 priVid,
                                 SINT32 pubVid, UBOOL8 isBridge,
                                 UBOOL8 isInternet, const char *ifname)
{
	HAL_ethSetWanMvlan(vlanIfName, priVid,  pubVid, isBridge, isInternet, ifname);

	return VOS_RET_SUCCESS;
}

VOS_RET_E HAL_ethCreateLanVlanIf(const char *ifName, SINT32 vid)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	char cmd[BUFLEN_256] = {0}, vlanDevName[BUFLEN_32] = {0};

	if (vid > MIN_VLAN_ID && vid < MAX_VLAN_ID)
	{
		UTIL_DO_SYSTEM_ACTION("vconfig set_name_type DEV_PLUS_VID");
		UTIL_DO_SYSTEM_ACTION("vconfig add %s %d", ifName, vid);

		memset(vlanDevName, 0, sizeof(vlanDevName));
		UTIL_SNPRINTF(vlanDevName, sizeof(vlanDevName), VLAN_DEV_FORMAT, ifName, vid);

		memset(cmd, 0, sizeof(cmd));
		UTIL_SNPRINTF(cmd, sizeof(cmd), "ifconfig %s up", vlanDevName);
		UTIL_DO_SYSTEM_ACTION(cmd);
	}

	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethCreateLanDefVlanIf(const char *ifName,
                                    HAL_VLAN_MODE_E vlanMode)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	VOS_RET_E ret = VOS_RET_SUCCESS;
	return ret;
#endif /* DESKTOP_LINUX */
}

SINT32 HAL_getInterfaceIndex(const char *interfaceName)
{
	struct ifreq ifr;
	SINT32 s = 0, ret = -1;

	if (interfaceName == NULL)
		return ret;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return ret;

	UTIL_STRNCPY(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
		ret = 0;
	else
		ret = ifr.ifr_ifindex;

	close(s);

	return ret;
}

void HAL_createEthernetFlows(UINT32 numOfAssoIntf,
                             char assoIfName[][BUFLEN_32],
                             char *l3Name)
{
#ifndef JASON_DEBUG
	return;
#else
	UINT32 i;

	if (NULL == l3Name)
	{
		vosLog_error("l3Name is NULL");
		return;
	}

	vlanCtl_init();
	for (i = 0; i < numOfAssoIntf; i++)
	{
		if ((IS_EMPTY_STRING(assoIfName[i]))
		        || (!HAL_getInterfaceIndex(assoIfName[i])))
		{
			vosLog_error("interface %p is null or does not exist.", assoIfName[i]);
			continue;
		}

		vlanCtl_createVlanFlows(l3Name, assoIfName[i]);
		vlanCtl_createVlanFlows(assoIfName[i], l3Name);
		vosLog_debug("create vlan blog rule: rxDev=%s txDev=%s", assoIfName[i], l3Name);
	}
	vlanCtl_cleanup();
#endif
}

VOS_RET_E HAL_deleteEthernetFlows(char *rxL3Name, char *txL3Name, char *txIface)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else
	int ret;
	/*delete blg rules*/
	vlanCtl_init();
	if (txIface == NULL)
	{
		ret = vlanCtl_deleteVlanFlows(rxL3Name, "");
		ret = vlanCtl_deleteVlanFlows("", rxL3Name);
		vosLog_debug("remove vlan blog rule: bi-direction Dev=%s", rxL3Name);
	}
	else
	{
		ret = vlanCtl_deleteVlanFlows(rxL3Name, txL3Name);
		vosLog_debug("remove vlan blog rule: rxDev=%s txDev=%s", rxL3Name, txL3Name);
	}

	vlanCtl_cleanup();

	if (ret == 0)
		return VOS_RET_SUCCESS;
	else
		return VOS_RET_INTERNAL_ERROR;
#endif
}


VOS_RET_E HAL_ethAddIptvIpv6Address(const char *ifName,
                                    const HAL_ETH_GMP_MW_WAN_IPTV_IPV6_ADDR_T *iptvAddr)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
#ifdef DESKTOP_LINUX
	return ret;
#else
	/*
	    SINT32 fd = 0;
	    SINT32 rc = 0;

	    fd = open(BCM_GMP_MW_CHRDEV_FULLNAME, O_WRONLY);
	    if (fd < 0)
	    {
	        vosLog_error("open failed. fd=%d", fd);
	        return VOS_RET_INTERNAL_ERROR;
	    }

	    rc = ioctl(fd, BCM_GMP_MW_ADD_WAN_IPTV_IP6_ADDRESS, iptvAddr);
	    if (rc < 0)
	    {
	        vosLog_error("ioctl failed. rc=%d", rc);
	        return VOS_RET_INTERNAL_ERROR;
	    }*/

	return ret;

#endif
}


VOS_RET_E HAL_ethDelIptvIpv6Address(const char *ifName,
                                    const HAL_ETH_GMP_MW_WAN_IPTV_IPV6_ADDR_T *iptvAddr)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;

#ifdef DESKTOP_LINUX
	return ret;
#else
	/*
	    SINT32 fd = 0;
	    SINT32 rc = 0;

	    fd = open(BCM_GMP_MW_CHRDEV_FULLNAME, O_WRONLY);
	    if (fd < 0)
	    {
	        vosLog_error("open failed. fd=%d", fd);
	        return VOS_RET_INTERNAL_ERROR;
	    }

	    rc = ioctl(fd, BCM_GMP_MW_REMOVE_WAN_IPTV_IP6_ADDRESS, iptvAddr);
	    if (rc < 0)
	    {
	        vosLog_error("ioctl failed. rc=%d", rc);
	        return VOS_RET_INTERNAL_ERROR;
	    }*/

	return ret;

#endif
}


VOS_RET_E HAL_ethSetPortEgressRate(const char *ifName, UINT32 kBitsSec,
                                   UINT32 kBitsburst)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else
	UINT8 *port = (UINT8 *)ifName;

	if (NULL == port)
	{
		return VOS_RET_INVALID_ARGUMENTS;
	}

	if (0 == kBitsSec)
	{
		UTIL_DO_SYSTEM_ACTION("ethcmd ratectl 0 %u 1000000", MTK_PORT_CONVERT(*port));
	}
	else
	{
		UTIL_DO_SYSTEM_ACTION("ethcmd ratectl 0 %u %u",
		                      MTK_PORT_CONVERT(*port), kBitsSec < 1024 ? 1024 : kBitsSec);
	}

	return VOS_RET_SUCCESS;

#endif
}


#ifndef DESKTOP_LINUX
UINT16 hal_ethGetPhyIdByName(const char *name)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else
	SINT32 socketFd;

	struct ifreq intf;
	struct ifreq *pifr = &intf;

	struct mii_ioctl_data miiVal;

	UINT16 *data;

	if ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket open error");
		return -1;
	}

	if (socketFd  >= 0)
	{
		UTIL_STRNCPY(pifr->ifr_name, name, sizeof(pifr->ifr_name));

		intf.ifr_data = (char *)&miiVal;

		data = (unsigned short *)(&intf.ifr_data);

		data[0] = 0;

		if (ioctl(socketFd, SIOCGMIIPHY, &intf) != -1)
		{
			close(socketFd);
			return data[0];
		}

		close(socketFd);
	}

	return -1;
#endif
}


UINT16 hal_ethSetPhyState(UINT16 phyId, UBOOL8 state)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else
	SINT32 socketFd;
	UINT16 writeVal = 0;
	UINT16 *data;
	struct ifreq intf;
	struct ifreq *pifr = &intf;

	if ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket open error");
		return -1;
	}

	if (socketFd  >= 0)
	{
		UTIL_STRNCPY(pifr->ifr_name, "eth0", sizeof(pifr->ifr_name));

		data = (unsigned short *)(&intf.ifr_data);
		data[0] = phyId;
		data[1] = MII_BMCR;
		if (ioctl(socketFd, SIOCGMIIREG, &intf) != -1)
		{
			if (state)
			{
				writeVal = data[3] & (~(1 << 11));
				vosLog_debug("writeVal %x", writeVal);
			}
			else
			{
				writeVal = data[3] | (1 << 11);
				vosLog_debug("writeVal %x", writeVal);
			}
		}

		data[2] = writeVal;

		if (ioctl(socketFd, SIOCSMIIREG, &intf) == -1)
		{
			vosLog_debug("ioctl SIOCSMIIREG fail");
		}

		close(socketFd);
	}
	return 0;
#endif
}
#endif /* DESKTOP_LINUX */

VOS_RET_E HAL_ethSetPortState(int port, UBOOL8 enable)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	char cmd[128] = {0};
	UINT32 phyId = 0;

	if (port >= 0 && port <= 4)
	{
		if (port == 0)
		{
			phyId = 4; //phy4
		}
		else if (port == 1)
		{
			phyId = 3;//phy3
		}
		else if (port == 2)
		{
			phyId = 2;//phy2
		}
		else if (port == 3)
		{
			phyId = 1;//phy1
		}

		vosLog_debug("Eth[%d]-Phy[%d] change state into [%s].\n", port, phyId,
		             (enable) ? "Enable" : "Disable");

		/* User app enable port only, kernel would disable phy port when detected loopback. */
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd) - 1, "echo %d %d > /proc/tc3162/eth_loopback", phyId,
		         enable);
		UTIL_DO_SYSTEM_ACTION(cmd);
	}
	else
	{
		vosLog_error("Port Number Invalid!!!\n");
		return VOS_RET_INVALID_ARGUMENTS;
	}

	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}

VOS_RET_E HAL_ethResetPort(const char *ifName)
{
	char vport[BUFLEN_32] = {0};

	if (SF_FEATURE_CUSTOMER_JISHIHUITONG)
	{
		return VOS_RET_SUCCESS;
	}
	if (util_strstr(ifName, "eth") == NULL)
	{
		vosLog_error("invalid ifName");
		return VOS_RET_INVALID_PARAM_VALUE;
	}

	UTIL_SNPRINTF(vport, sizeof(vport), "%s.0", ifName);

	UTIL_DO_SYSTEM_ACTION("ifconfig %s down", ifName);
	UTIL_DO_SYSTEM_ACTION("ifconfig %s up", ifName);
	UTIL_DO_SYSTEM_ACTION("ifconfig %s up", vport);

	return VOS_RET_SUCCESS;
}


VOS_RET_E HAL_addSwitchVlanTable(const char *connType, const char *serList,
                                 SINT32 vid)
{
	return VOS_RET_SUCCESS;
}


VOS_RET_E HAL_ethSetSwitchForMmeIntf(char *ifName, SINT32 vid)
{
	return VOS_RET_SUCCESS;
}

VOS_RET_E HAL_ethSetPortIngressRate(const char *ifName, UINT32 kBitsSec,
                                    UINT32 kBitsburst)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else

	UINT8 *port = (UINT8 *)ifName;

	if (NULL == port)
	{
		return VOS_RET_INVALID_ARGUMENTS;
	}

	if (0 == kBitsSec)
	{
		UTIL_DO_SYSTEM_ACTION("ethcmd ratectl 1 %u 1000000", MTK_PORT_CONVERT(*port));
	}
	else
	{
		UTIL_DO_SYSTEM_ACTION("ethcmd ratectl 1 %u %u",
		                      MTK_PORT_CONVERT(*port), kBitsSec < 1024 ? 1024 : kBitsSec);
	}

	return VOS_RET_SUCCESS;
#endif
}


VOS_RET_E HAL_ethCreateRouteTransparentVlanIf(const char *ifName, SINT32 idx,
        SINT16 vid, UINT8 priority,
        UBOOL8 isPppoe)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}



VOS_RET_E HAL_ethCreateBridgeTransparentVlanIf(const char *ifName, SINT32 idx,
        SINT16 vid, UINT8 priority)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethDeleteConnInterface(const char *vlanDevName, int vid, int pri)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif
}

VOS_RET_E HAL_ethCreateConnInterface(const char *vlanDevName, int vid, int pri,
                                     SINT16 MultiVid, UBOOL8 *omciVlanMode)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif
}

VOS_RET_E HAL_ethDisablePortRecive(UINT32 swPort)
{
	return VOS_RET_SUCCESS;
}


VOS_RET_E HAL_ethEnablePortRecive(UINT32 swPort)
{
	return VOS_RET_SUCCESS;
}

VOS_RET_E HAL_ethSetPortPower(UINT32 port, UINT32 portState)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	UBOOL8 enable = FALSE;
	char cmd[128] = {0};
	UINT32 phyId = 0;

	if (port >= 0 && port <= 4)
	{
		enable = (0 == portState) ? TRUE : FALSE;

		if (port == 0)
		{
			phyId = 4; //phy4
		}
		else if (port == 1)
		{
			phyId = 3;//phy3
		}
		else if (port == 2)
		{
			phyId = 2;//phy2
		}
		else if (port == 3)
		{
			phyId = 1;//phy1
		}

		printf("\nEth[%d]-Phy[%d] change state into [%s].\n", port, phyId,
		       (0 == portState) ? "Enable" : "Disable");

		/* User app enable port only, kernel would disable phy port when detected loopback. */
		if (enable)
		{
			memset(cmd, 0, sizeof(cmd));
			snprintf(cmd, sizeof(cmd) - 1, "echo %d %d > /proc/tc3162/eth_loopback", phyId,
			         enable);
			UTIL_DO_SYSTEM_ACTION(cmd);
		}
	}
	else
	{
		vosLog_error("Port Number Invalid!!!\n");
		return VOS_RET_INVALID_ARGUMENTS;
	}

	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethRestoreLanMcastRule()
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	int port = 0;
	char devName[BUFLEN_128] = {0};
	char vlanDevName[BUFLEN_128] = {0};
	unsigned int tagRuleId = 0;
	UINT32 ethNum = 0;

	if (VOS_RET_SUCCESS != HAL_sysGetEthPortNum(&ethNum))
	{
		vosLog_error("Failed to get eth number!\n");
		return VOS_RET_INTERNAL_ERROR;
	}

	for (port = 0; port < ethNum; port++)
	{
		UTIL_SNPRINTF(devName, sizeof(devName), "eth%d", port);
		UTIL_SNPRINTF(vlanDevName, sizeof(vlanDevName), "eth%d.0", port);
		vlanCtl_init();
		vlanCtl_initTagRule();
		vlanCtl_removeAllTagRule(vlanDevName);


		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_cmdPopVlanTag();
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);
		vlanCtl_cleanup();
	}

	return VOS_RET_SUCCESS;
#endif
}


VOS_RET_E HAL_ethDelEponOamVlan(const UINT8 lanPort, const UINT16 vlan,
                                UINT32 tagRuleId)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethAddEponOamVlan(const UINT8 lanPort, const UINT16 vlan,
                                UINT32 tagRuleId)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


#ifndef DESKTOP_LINUX
#ifdef DESKTOP_LINUX
static VOS_RET_E hal_ethVlanExist(const UINT8 lanPort, const UINT16 eponVlan)
{
	return VOS_RET_SUCCESS;
}
#endif

#ifdef DESKTOP_LINUX
static VOS_RET_E hal_ethGetEponOamVlan(const UINT8 lanPort,
                                       UINT16 vlan[BUFLEN_8], UINT8 *vlanNum)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}
#endif


VOS_RET_E HAL_ethGetEponOamRuleId(const UINT8 lanPort, const UINT16 vlan,
                                  char *ruleIdString)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}
#endif


VOS_RET_E HAL_ethSetEponOamRuleId(const UINT8 lanPort, const UINT16 vlan,
                                  const UINT16 ruleId)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
	return VOS_RET_SUCCESS;
#endif /* DESKTOP_LINUX */
}


VOS_RET_E HAL_ethUpdateLanPortMcastTag(const UINT8 lanPort,
                                       const UINT16 eponVlan, const UINT16 iptvVlan,
                                       const UINT8 tagOp)
{
#ifndef JASON_DEBUG
	return VOS_RET_SUCCESS;
#else
	char devName[BUFLEN_128] = {0};
	char vlanDevName[BUFLEN_128] = {0};
	unsigned int tagRuleId = 0;
	UINT16 vlan[BUFLEN_8] = {0};
	UINT8 vlanNum = 0;
	int i;

	UTIL_SNPRINTF(devName, sizeof(devName), "eth%d", lanPort);
	UTIL_SNPRINTF(vlanDevName, sizeof(vlanDevName), "eth%d.0", lanPort);

	//Whatever which tagOp oam issued, delete the existed rule first(rx, tags=0, ethX->ethX.0).
	vlanCtl_init();
	vlanCtl_initTagRule();
	vlanCtl_removeAllTagRule(vlanDevName);
	vlanCtl_cleanup();

	if ((NULL == devName) || (NULL == vlanDevName))
	{
		vosLog_error("devName or vlanDevName is NULL.");
		return VOS_RET_INTERNAL_ERROR;
	}

	vlanCtl_init();
	vlanCtl_initTagRule();
	//read oamfile to
	//first, get the whole vlan of configured in this port
	if ((0 == tagOp) || (1 == tagOp))
	{
		//handle all vlan exist in the port.
		hal_ethGetEponOamVlan(lanPort, vlan, &vlanNum);
	}
	else if (2 == tagOp)
	{
		//only handle the configured vlan from oam.
		if (VOS_RET_SUCCESS != hal_ethVlanExist(lanPort, eponVlan))
		{
			vosLog_error("tagOp is 2, but the eponVlan doesn't exist as a multicast vlan in Port %d.",
			             lanPort);
			return VOS_RET_INTERNAL_ERROR;
		}
		else
		{
			vosLog_notice("tagOp is 2, and the eponVlan is exist in Port %d.", lanPort);
			vlanNum = 1;
			vlan[0] = eponVlan;
		}
	}

	if (0 == tagOp)
	{
		//²»°þ³ý
		//multicast interface, igmp
		for (i = 0; i < vlanNum; i++)
		{
			vosLog_debug("(set ruleId)vlan[%d]:%d", i, vlan[i]);
			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
			vlanCtl_filterOnIpProto(2);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      0,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
			vlanCtl_filterOnIpProto(2);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      1,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			//multicast interface, udp
			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
			vlanCtl_filterOnIpProto(11);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      0,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
			vlanCtl_filterOnIpProto(11);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      1,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			//multicast interface, udp
			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
			vlanCtl_filterOnIpProto(58);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      0,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
			vlanCtl_filterOnIpProto(58);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      1,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);


			//unicast
			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      0,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_cmdSetTagVid(vlan[i], 0);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      1,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

		}

		//rx, tags=0:NOP, tags=1:POP
		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_cmdPopVlanTag();
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);
	}
	else if (1 == tagOp)
	{
		//°þ³ý
		for (i = 0; i < vlanNum; i ++)
		{
			vosLog_debug("vlan[%d]:%d, devName:%s", i, vlan[i], devName);
			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      0,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);

			vlanCtl_initTagRule();
			vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
			vlanCtl_insertTagRule(devName,
			                      VLANCTL_DIRECTION_TX,
			                      1,
			                      VLANCTL_POSITION_APPEND,
			                      VLANCTL_DONT_CARE,
			                      &tagRuleId);
		}

		//rx, tags=0:NOP, tags=1:POP
		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_cmdPopVlanTag();
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);
	}
	else if (2 == tagOp)
	{
		vosLog_debug("devName:%s, iptvVlan:%d", devName, iptvVlan);
		//×ª»»
		//multicast interface, igmp
		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
		vlanCtl_filterOnIpProto(2);
		vlanCtl_cmdSetTagVid(iptvVlan, 0);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
		vlanCtl_filterOnIpProto(2);
		vlanCtl_cmdSetTagVid(iptvVlan, 0);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		//multicast interface, udp
		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
		vlanCtl_filterOnIpProto(17);
		vlanCtl_cmdSetTagVid(iptvVlan, 0);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
		vlanCtl_filterOnIpProto(17);
		vlanCtl_cmdSetTagVid(iptvVlan, 0);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		//multicast interface, udp
		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
		vlanCtl_filterOnIpProto(58);
		vlanCtl_cmdSetTagVid(iptvVlan, 0);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_filterOnFlags(BCM_VLAN_FILTER_FLAGS_IS_MULTICAST);
		vlanCtl_filterOnIpProto(58);
		vlanCtl_cmdSetTagVid(iptvVlan, 0);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		//unicast
		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_filterOnTxVlanDevice((char *)vlanDevName);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_TX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		//rx, tags=0:NOP, tags=1:POP
		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      0,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);

		vlanCtl_initTagRule();
		vlanCtl_setReceiveVlanDevice((char *)vlanDevName);
		vlanCtl_cmdPopVlanTag();
		vlanCtl_insertTagRule(devName,
		                      VLANCTL_DIRECTION_RX,
		                      1,
		                      VLANCTL_POSITION_APPEND,
		                      VLANCTL_DONT_CARE,
		                      &tagRuleId);
	}

	vlanCtl_cleanup();

	return VOS_RET_SUCCESS;
#endif
}


VOS_RET_E HAL_ethSetDevStatsStatus(UBOOL8 enable)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else
	if (enable)
	{
		UTIL_DO_SYSTEM_ACTION("fc disable");
		UTIL_DO_SYSTEM_ACTION("fc flush");
	}
	else
	{
		UTIL_DO_SYSTEM_ACTION("fc enable");
		UTIL_DO_SYSTEM_ACTION("fc flush");
	}

	return VOS_RET_SUCCESS;
#endif
}


VOS_RET_E HAL_delHwnatByEthIfName(const char *ifName, const char *macAddr)
{
#ifdef DESKTOP_LINUX
	return VOS_RET_SUCCESS;
#else
	FILE *fb = NULL;
	char arpBuf[1024] = {0};
	char ipAddr[128] = {0};

	if ((ifName == NULL) || (macAddr == NULL))
	{
		return VOS_RET_INTERNAL_ERROR;
	}
	vosLog_debug("ifName:%s, macAddr:%s", ifName, macAddr);

	UTIL_DO_SYSTEM_ACTION("brctl showmacs br0 > /tmp/macsTemp");
	fb = fopen("/tmp/macsTemp", "r+");
	if (NULL == fb)
	{
		vosLog_error("Can't open /tmp/macsTemp");
		return VOS_RET_INTERNAL_ERROR;
	}

	memset(arpBuf, 0, sizeof(arpBuf));
	while (NULL != fgets(arpBuf, sizeof(arpBuf), fb))
	{
		int portTemp = 0;
		char macEthAddr[UTIL_MAC_ADDR_LENGTH] = {0};
		char portNum[4] = {0};
		memset(macEthAddr, 0, sizeof(macEthAddr));
		memset(portNum, 0, sizeof(portNum));
		vosLog_debug("arpBuf:%s", arpBuf);
		if (util_strstr(arpBuf, "local"))
		{
			continue;
		}
		sscanf(arpBuf, " %s %17s", portNum, macEthAddr);
		portTemp = atoi(ifName + 3) + 1;
		vosLog_debug("portTemp:%d, macEthAddr:%s,portNum:%s", portTemp, macEthAddr,
		             portNum);
		if (portTemp == atoi(portNum))
		{
			FILE *fb1 = NULL;
			UTIL_DO_SYSTEM_ACTION("arp -a > /tmp/arpTemp");

			fb1 = fopen("/tmp/arpTemp", "r+");
			if (NULL == fb1)
			{
				vosLog_error("Can't open /tmp/arpTemp");
				return VOS_RET_INTERNAL_ERROR;
			}

			memset(arpBuf, 0, sizeof(arpBuf));
			memset(ipAddr, 0, sizeof(ipAddr));
			while (NULL != fgets(arpBuf, sizeof(arpBuf), fb1))
			{
				if (util_strstr(arpBuf, macEthAddr))
				{
					vosLog_debug("arpBuf:%s", arpBuf);
					//? (192.168.1.222) at d8:cb:8a:62:b4:e3 [ether]  on br1
					sscanf(arpBuf, "%*[^(](%[^)]s", ipAddr);
					break;
				}
			}
			vosLog_debug("ipAddr:%s", ipAddr);
			fclose(fb1);
			UTIL_DO_SYSTEM_ACTION("rm -rf /tmp/arpTemp");

			if (IS_EMPTY_STRING(ipAddr))
			{
				vosLog_error("ipAddr is NULL!");
				return VOS_RET_INTERNAL_ERROR;
			}

			UTIL_DO_SYSTEM_ACTION("hw_nat -g | grep %s > /tmp/hwnatTemp", ipAddr);
			fb1 = fopen("/tmp/hwnatTemp", "r+");
			if (NULL == fb1)
			{
				vosLog_error("Can't open /tmp/hwnatTemp");
				return VOS_RET_INTERNAL_ERROR;
			}

			char numEntry[16] = {0};
			memset(arpBuf, 0, sizeof(arpBuf));
			while (NULL != fgets(arpBuf, sizeof(arpBuf), fb1))
			{
				vosLog_debug("arpBuf2:%s", arpBuf);
				if (util_strstr(arpBuf, ipAddr))
				{
					sscanf(arpBuf, "%*[^=]=%s : %*s => %*s", numEntry);
					vosLog_debug("numEntry:%s", numEntry);
					UTIL_DO_SYSTEM_ACTION("hw_nat -+%d", atoi(numEntry));
				}
			}

			UTIL_DO_SYSTEM_ACTION("rm -rf /tmp/hwnatTemp");
			fclose(fb1);
		}

	}
	vosLog_debug("ipAddr:%s", ipAddr);
	fclose(fb);
	UTIL_DO_SYSTEM_ACTION("rm -rf /tmp/macsTemp");

	return VOS_RET_SUCCESS;
#endif
}


VOS_RET_E HAL_ethGetPortFromName(char *ethName, UINT8 *port)
{
	if (!ethName)
	{
		return VOS_RET_INTERNAL_ERROR;
	}

	if (util_strstr(ethName, "eth"))
	{
		*port = atoi(ethName + 3);
	}

	if (util_strstr(ethName, "wl"))
	{
		*port = atoi(ethName + 3) + 4;
	}

	return VOS_RET_SUCCESS;
}

