/*
 * @*************************************:
 * @FilePath     : /network/valid_param/valid_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-11-08 14:27:16
 * @LastEditors  : dof
 * @LastEditTime : 2024-11-22 11:05:44
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#undef PP
#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

int is_valid_mac(const char *mac)
{
    regex_t regex;
    int reti = 0, ret = 0;
    char msgbuf[100];

    /* 正则表达式模式，匹配常见的MAC地址格式 */
    char *pattern = "^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$";

    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti)
    {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    reti = regexec(&regex, mac, 0, NULL, 0);
    if (!reti)
    {
        regfree(&regex);
        // return 1; // 匹配成功，合法
        ret = 1;
    }
    else if (reti == REG_NOMATCH)
    {
        regfree(&regex);
        // return 0; // 不匹配，非法
        ret = 0;
    }
    else
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        // return 0;
        ret = 0;
    }

   	if (0 == strcmp("00:00:00:00:00:00", mac))
	{
		ret = 0;
	}

    return ret;
}

#include <stdio.h>
#include <regex.h>

int is_valid_ip(char *ip)
{
    regex_t regex;
    int reti;
    char msgbuf[100];

    /* 正则表达式编译 */
    reti = regcomp(&regex, "^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$", 0);
    if (reti)
    {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    /* 匹配 */
    reti = regexec(&regex, ip, 0, NULL, 0);
    if (!reti)
    {
        puts("IP is valid");
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        return 1;
    }
    else if (reti == REG_NOMATCH)
    {
        puts("IP is invalid");
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        return 0;
    }
    else
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        return 0;
    }

    regfree(&regex);
    return 0;
}

int32_t hi_ipaddr_pton(const char *ipaddr, hi_addr *addr)
{
	return (inet_pton(AF_INET, ipaddr, addr) == 1) ? (HI_RET_SUCC) : (-HI_RET_FAIL);
}

int32_t hi_ip6addr_pton(const char *ip6addr, hi_addr6 *addr6)
{
	return (inet_pton(AF_INET6, ip6addr, addr6) == 1) ? (HI_RET_SUCC) : (-HI_RET_FAIL);
}

typedef struct {
	union {
		uint8_t addr8[16];
		uint16_t addr16[8];
		uint32_t addr32[4];
	};
} hi_addr6;


int32_t hi_ip6subnet_ntop(const struct hi_addr6_subnet *subnet, char *subnet_str, uint32_t size)
{
	char addr6_str[HI_ADDR6STRLEN];
	if (inet_ntop(AF_INET6, &subnet->addr, addr6_str, HI_ADDR6STRLEN) == NULL)
		return -HI_RET_FAIL;
	if (sprintf_s(subnet_str, size, "%s/%u", addr6_str, subnet->mask) < 0)
		return -HI_RET_FAIL;
	return HI_RET_SUCC;
}

int32_t hi_ip6addr_valid(const char *ip6addr)
{
	hi_addr6 addr6;
	return (hi_ip6addr_pton(ip6addr, &addr6) == HI_RET_SUCC);
}

int32_t hi_ipaddr_valid(const char *ipaddr)
{
	hi_addr addr4;
	return (hi_ipaddr_pton(ipaddr, &addr4) == HI_RET_SUCC);
}

int32_t hi_ip6_addr_cmp(hi_addr6 *ip1, hi_addr6 *ip2)
{
	int32_t i, ans = 0;
	for (i = 0; i < ARRAY_SIZE(ip1->addr8) && ans == 0; i++) {
		ans = ip1->addr8[i] - ip2->addr8[i];
	}
	return ans;
}



int week_calc(unsigned int weekday, char *retStr, unsigned int len)
{
    char cmd[128] = {0};
    int s = 0;

    if (weekday == 0x7f)
    {
        return 1;
    }

    for (int i = 0; i < 8; i++)
    {
        s = (weekday >> i & 0x1);
        if (!s)
        {
            continue;
        }

        strcat(cmd, (strlen(cmd) && i && s) ? "," : "");
        switch (i)
        {
        case 0:
            strcat(cmd, "Sun");
            break;
        case 1:
            strcat(cmd, "Mon");
            break;
        case 2:
            strcat(cmd, "Tue");
            break;
        case 3:
            strcat(cmd, "Wed");
            break;
        case 4:
            strcat(cmd, "Thu");
            break;
        case 5:
            strcat(cmd, "Fri");
            break;
        case 6:
            strcat(cmd, "Sat");
            break;
        default:
            break;
        }
    }

    strncpy(retStr, cmd, len);
    PP("cmd = %d", strlen(cmd));

    return 0;
}

char igdCmHextoChar(unsigned char srchex)
{
    switch (srchex)
    {
    case 0x0:
        return '0';
    case 0x1:
        return '1';
    case 0x2:
        return '2';
    case 0x3:
        return '3';
    case 0x4:
        return '4';
    case 0x5:
        return '5';
    case 0x6:
        return '6';
    case 0x7:
        return '7';
    case 0x8:
        return '8';
    case 0x9:
        return '9';
    case 0xa:
        return 'A';
    case 0xb:
        return 'B';
    case 0xc:
        return 'C';
    case 0xd:
        return 'D';
    case 0xe:
        return 'E';
    case 0xf:
        return 'F';
    default:
        return ' '; /* input hex error */
    }
}

int igdCmMactoChar(unsigned char *mac_digital, char *mac_char)
{
    unsigned int i;

    if ((mac_char == NULL) || (mac_digital == NULL))
        return (1);

    for (i = 0; i < 6; i++)
    {
        mac_char[(i * 3) + 0] = igdCmHextoChar(mac_digital[i] / 0x10);
        mac_char[(i * 3) + 1] = igdCmHextoChar(mac_digital[i] % 0x10);
        if (((i * 3) + 2) < 17)
            mac_char[(i * 3) + 2] = ':';
        else
            mac_char[(i * 3) + 2] = '\0';
    }

    return (0);
}

int cwmp_sys_chartohex(char srcchar)
{
    switch (srcchar)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'A':
    case 'a':
        return 10;
    case 'B':
    case 'b':
        return 11;
    case 'C':
    case 'c':
        return 12;
    case 'D':
    case 'd':
        return 13;
    case 'E':
    case 'e':
        return 14;
    case 'F':
    case 'f':
        return 15;
    default:
        return 16; /* input char error */
    }
}

int cwmp_sys_chartomac(const char *mac_char, char *mac_digital)
{
    char i;
    char p0, p1;
    /*null check should before than dereference*/
    if ((mac_char == NULL) || (mac_digital == NULL))
    {
        return (2);
    }
    if (strlen(mac_char) != 17)
    {
        return (1);
    }

    for (i = 0; i < 17; i = i + 3)
    {
        if ((p0 = cwmp_sys_chartohex(mac_char[i])) >= 16)
        {
            return (3);
        }
        if ((p1 = cwmp_sys_chartohex(mac_char[i + 1])) >= 16)
        {
            return (3);
        }
        mac_digital[i / 3] = p0 * 16 + p1;
        if (((i + 2) < 17) && (mac_char[i + 2] != ':'))
        {
            return (4);
        }
    }

    return 0;
}

char cwmp_sys_hextochar(char srchex)
{
    switch (srchex)
    {
    case 0x0:
        return '0';
    case 0x1:
        return '1';
    case 0x2:
        return '2';
    case 0x3:
        return '3';
    case 0x4:
        return '4';
    case 0x5:
        return '5';
    case 0x6:
        return '6';
    case 0x7:
        return '7';
    case 0x8:
        return '8';
    case 0x9:
        return '9';
    case 0xa:
        return 'A';
    case 0xb:
        return 'B';
    case 0xc:
        return 'C';
    case 0xd:
        return 'D';
    case 0xe:
        return 'E';
    case 0xf:
        return 'F';
    default:
        return ' '; /* input hex error */
    }
}

int cwmp_sys_mactochar(char *mac_digital, char *mac_char)
{
    char i;

    if ((mac_char == NULL) || (mac_digital == NULL))
    {
        return (1);
    }

    for (i = 0; i < 6; i++)
    {
        mac_char[(i * 3) + 0] = cwmp_sys_hextochar(mac_digital[i] / 0x10);
        mac_char[(i * 3) + 1] = cwmp_sys_hextochar(mac_digital[i] % 0x10);
        if (((i * 3) + 2) < 17)
        {
            mac_char[(i * 3) + 2] = ':';
        }
        else
        {
            mac_char[(i * 3) + 2] = '\0';
        }
    }

    return 0;
}


#include <time.h>



#define SKBMARK_FLOW_ID_S 11
#define SKBMARK_FLOW_ID_M (0xFF << SKBMARK_FLOW_ID_S)
#define SKBMARK_GET_FLOW_ID(MARK) \
    ((MARK & SKBMARK_FLOW_ID_M) >> SKBMARK_FLOW_ID_S)
#define SKBMARK_SET_FLOW_ID(MARK, FLOW) \
    ((MARK & ~SKBMARK_FLOW_ID_M) | (FLOW << SKBMARK_FLOW_ID_S))

int main(int argc, char const *argv[])
{
    char *mac = "F4:6B:8A:5E:BB:1A";
    if (is_valid_mac(mac))
    {
        printf("ssss\n");
    }

    // char mac_digital[32]  = "F4:6B:8r:5E:BB:1E";
    char mac_digital[32] = {0};
    char mac_char[32] = {0};
    // igdCmMactoChar(mac_digital, mac_char);

    // cwmp_sys_chartomac("F4:6B:8e:5E:BB:1E", mac_char);
    // printf("mac_char = %s\n", mac_char);

    // cwmp_sys_mactochar(mac_char, mac_digital);
    // printf("mac_digital = %s\n", mac_digital);

    // char cmd[256] = {0};

    // for (int i = 128; i > 0; i--)
    // for (int i = 0; i < 128; i++)
    // {
    //     printf("mark = 0x%x\n", SKBMARK_SET_FLOW_ID(0, i+1));
    // }
    // {
    //     int weekRet = week_calc(i, cmd, sizeof(cmd));
    //     if (!weekRet)
    //     {
    //         printf("cmd = %5d |%s|\n", i, cmd);
    //     }
    // }
    // if (is_valid_ip("192.168.1.25"))
    // {
    //     printf("ssss");
    // }
    int i = 0;
    int j = 0;
    // printf("---%s|%s---\n", i?"sss":"", i?(char *)i:"");

    int sourcePort = 0;
    char port[32] = {0};

    // char *str = "2023-12-25 12:30:00";
    char *str = "2024-11-20T11:13:56.564760";

    int usec = 0;
    if (strrchr(str, '.'))
    {
        sscanf(strrchr(str, '.') + 1, "%ld", &usec);
        PP("usec = %d", usec);
    }

    struct tm tm = {0};
    strptime(str, "%Y-%m-%dT%H:%M:%S", &tm);
    time_t t = mktime(&tm);
    PP("字符串表示的时间对应的秒数：%ld | %d\n", t, usec);
    return 0;


    // snprintf(port, sizeof(port), "--sport %d", sourcePort?sourcePort:atoi(""));
    // PP("%s", port);

    return 0;
}


mac 合法性
mac 大小比较
mac 自增
mac 自减
mac 转 mac 字符
mac 转 mac 数字

ipv4 合法性
ipv4 大小比较
ipv4 subnet 计算

ipv6 合法性
ipv6 大小比较
ipv6 subnet 计算
