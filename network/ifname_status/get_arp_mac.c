/*
 * @*************************************:
 * @FilePath: /network/ifname_status/get_arp_mac.c
 * @version:
 * @Author: dof
 * @Date: 2023-02-24 11:57:15
 * @LastEditors: dof
 * @LastEditTime: 2023-02-24 12:00:40
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>

#define TRUE  1
#define FALSE 0


#define IS_EMPTY_STRING(s)    ((s == NULL) || (*s == '\0'))


void rutNtwk_getMacByIPFromArp(char *mac, const char *lanip)
{
	FILE *fs = NULL;
	int count = 0;
	char line[512] = {0}, buf[512] = {0};
	char ip[32] = {0};
	char macStr[32] = {0};
	char find = FALSE;

	if (lanip == NULL || mac == NULL)
	{
		printf("lanip or mac is NULL");
		return;
	}

	system("cat /proc/net/arp > /var/arp_tmp");
	fs = fopen("/var/arp_tmp", "r");
	if (fs == NULL)
	{
		printf("fopen /var/arp_tmp error");
		return;
	}
	while (!find && fgets(line, sizeof(line), fs))
	{
		/* read pass 1 header lines */
		if (count++ < 1)
		{
			continue;
		}
		/*
		 IP address       HW type     Flags       HW address            Mask     Device
		 192.168.1.2    0x1         0x2         20:f4:1b:60:6b:66     *        br0
		 192.168.1.3      0x1         0x2         3c:97:0e:5b:99:db     *        br0
		 *
		 */
		memset(buf, 0, sizeof(buf));
		strcpy(buf, line);
		sscanf(buf, "%s %*s %*s %s %*s", ip, macStr);

		if (!IS_EMPTY_STRING(ip))
		{
			printf("ip:[%s], mac:[%s]\n", ip, macStr);
			if (!cmsUtl_strcmp(lanip, ip))
			{
				find = TRUE;
				strcpy(mac, macStr);
				printf("mac:[%s]\n", mac);
			}
		}
		memset(line, 0, sizeof(line));
	}

	fclose(fs);

	return;
}
