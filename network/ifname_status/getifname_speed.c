/*
 * @*************************************:
 * @FilePath: /network/ifname_status/getifname_speed.c
 * @version:
 * @Author: dof
 * @Date: 2024-02-20 10:55:54
 * @LastEditors: dof
 * @LastEditTime: 2024-03-01 09:43:32
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#if 0
int main(int argc, char *argv[]) {
  // 获取网卡名称
  char *ifname = "eth0";
  if (ifname == NULL) {
    printf("用法: %s <网卡名称>\n", argv[0]);
    return 1;
  }

  // 打开网卡句柄
  int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (fd < 0) {
    perror("打开网卡句柄失败");
    return 1;
  }

  // 获取网卡信息
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    perror("获取网卡信息失败");
    close(fd);
    return 1;
  }

  // 获取网卡支持的速率
  struct ethtool_link_settings ecmd;
  memset(&ecmd, 0, sizeof(ecmd));
  ecmd.cmd = ETHTOOL_GLINK;
  if (ioctl(fd, SIOCETHTOOL, &ecmd) < 0) {
    perror("获取网卡支持的速率失败");
    close(fd);
    return 1;
  }

  // 遍历支持的速率并找到最大速率
  int max_speed = 0;
//   for (int i = 0; i < ecmd.link_modes.supported_count; i++) {
//     if (ecmd.link_modes.supported[i] > max_speed) {
//       max_speed = ecmd.link_modes.supported[i];
//     }
//   }

  // 打印最大速率
  printf("网卡 %s 支持的最大速率: %d Mb/s\n", ifname, max_speed);

  // 关闭网卡句柄
  close(fd);

  return 0;
}
#elif 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>

int main(int argc, char *argv[]) {
//   if (argc != 2) {
//     fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
//     return EXIT_FAILURE;
//   }

  char *interface = "eth0";

  // 打开网络接口
  int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (fd < 0) {
    perror("socket");
    return EXIT_FAILURE;
  }

  // 获取接口索引
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, interface, IFNAMSIZ);
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    perror("ioctl(SIOCGIFINDEX)");
    close(fd);
    return EXIT_FAILURE;
  }

  // 获取接口信息
  struct ethtool_cmd ecmd;
  memset(&ecmd, 0, sizeof(ecmd));
  ecmd.cmd = ETHTOOL_GSET;
  ifr.ifr_data = (void *)&ecmd;
  if (ioctl(fd, SIOCETHTOOL, &ifr) < 0) {
    perror("ioctl(SIOCGIFETHTOOL)");
    close(fd);
    return EXIT_FAILURE;
  }

  // 打印最大速率
  printf("最大支持速率: %d Mb/s\n", ecmd.speed);

  // 关闭网络接口
  close(fd);

  return EXIT_SUCCESS;
}
#else
int main(int argc, char const *argv[])
{

    // 读取管道输出并打印
    char line[1024] = {0};
    char line1[1024] = {0};
    int maxSpeed = 0;
    char result[128] = {0};
    char tmp_cmd[128] = {0};
   

    FILE *fp1 = popen("ifconfig -a | grep eth | awk '{print $1}'", "r");
    while (fgets(line1, sizeof(line1), fp1))
    {

        FILE *fp = popen("ethtool eth0 | grep Full", "r");
        char tmp_cmd[128] = {0};
        //printf("line1 %s|%s", line1, strtok(line1, "."));
		
		if (strchr(line1, '.'))
		{
			printf("line1 %s|%s", line1, strtok(line1, "."));
			continue;
		}
		
		memset(line, 0, sizeof(line));
        snprintf(tmp_cmd, sizeof(tmp_cmd), "%s", line1);
        while (fgets(line, sizeof(line), fp))
        {
            if (strstr(line, "Supported link modes"))
            {
                // printf("%s", line);
            }
            if (strstr(line, "Advertised link modes"))
            {
                break;
            }
            printf("%s", line);
            sscanf(line, "%dbaseT/Full", &maxSpeed);
        }
        printf("maxspeed = %d\n", maxSpeed);
        switch (maxSpeed)
        {
        case 1000:
            strcat(result, "GE");
                break;
        case 2500:
            strcat(result, "2.5GE");
                break;
        case 10000:
            strcat(result, "10GE");
            break;
        default:
            strcat(result, "FE");
            break;
        }
		strcat(result, ",");
        printf("cmd = %s\n", result);

        // 关闭管道
        pclose(fp);

    }
    
	result[strlen(result)-1] = '\0';
    printf("cmd = %s\n", result);
    pclose(fp1);

    return 0;
}
