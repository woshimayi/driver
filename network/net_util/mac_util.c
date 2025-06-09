/*
 * @*************************************:
 * @FilePath     : /network/net_util/mac_util.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-01-06 15:25:27
 * @LastEditors  : dof
 * @LastEditTime : 2025-03-14 10:16:40
 * @Descripttion :  mac char * to byte
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <mcheck.h>
#include <sys/socket.h>

// 定义最大MAC地址长度
#define MAC_ADDR_LEN 6

// API：将MAC地址字符串转换为字符数组
/**
 * @brief
 *
 * @param mac_str
 * @param mac_bytes
 * @return int  // 0 表示成功。
 * -1 表示输入参数错误。
 * -2 表示正则表达式编译失败。
 * -3 表示 MAC 地址格式不正确
 * -4 表示解析 MAC 地址失败。
 */
int mac_address_to_bytes(const char *mac_str, unsigned char *mac_bytes)
{
    if (mac_str == NULL || mac_bytes == NULL)
    {
        return -1; // 参数错误
    }

    // 正则表达式：匹配 MAC 地址，格式为 XX:XX:XX:XX:XX:XX
    regex_t regex;
    const char *pattern = "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$"; // 支持大小写十六进制数字

    if (regcomp(&regex, pattern, REG_EXTENDED))
    {
        return -2; // 正则编译失败
    }

    // 正则匹配
    if (regexec(&regex, mac_str, 0, NULL, 0))
    {
        regfree(&regex);
        return -3; // MAC地址格式不正确
    }

    // 格式正确，开始解析 MAC 地址
    int byte_count = 0;
    for (int i = 0; i < 17; i += 3)
    {
        unsigned int byte;
        if (sscanf(mac_str + i, "%2x", &byte) != 1)
        {
            regfree(&regex);
            return -4; // 解析失败
        }
        mac_bytes[byte_count++] = (unsigned char)byte;
    }

    regfree(&regex);
    return 0; // 成功
}

/**
 * @brief 测试函数
 *
 * @param mac_bytes
 */
void print_mac_bytes(unsigned char *mac_bytes)
{
    for (int i = 0; i < MAC_ADDR_LEN; i++)
    {
        printf("%02X", mac_bytes[i]);
        if (i < MAC_ADDR_LEN - 1)
        {
            printf(":");
        }
    }
    printf("\n");
}

/**
 * @brief 转换字节数组为带冒号的MAC地址字符串
 *
 * @param mac
 * @param mac_str
 */
void byte_to_mac_string(unsigned int *mac, char *mac_str)
{
    // 格式化MAC地址，字节之间用冒号分隔
    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * @brief 使用正则表达式验证MAC地址格式
 *
 * @param mac_str
 * @return int
 */
int validate_mac_address(const char *mac_str)
{
    regex_t regex;
    const char *pattern = "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$";

    // 编译正则表达式
    if (regcomp(&regex, pattern, REG_EXTENDED))
    {
        return 0; // 正则编译失败
    }

    // 匹配正则表达式
    int result = regexec(&regex, mac_str, 0, NULL, 0);
    regfree(&regex);

    return result == 0; // 如果匹配成功，返回 1，失败返回 0
}

#if 0
int IsValidIpv4(char *ip)
{
    struct sockaddr_in IPAddr4;
    memset(&IPAddr4, 0, sizeof(struct sockaddr_in));
    if (1 == inet_pton(AF_INET, ip, &IPAddr4)) {
        return 1;
    } else {
        return 0;
    }
}

int IsValidIpv6(char *ip)
{
    struct sockaddr_in6 IPAddr6;
    memset(&IPAddr6, 0, sizeof(struct sockaddr_in6));
    if (1 == inet_pton(AF_INET6,ip,&IPAddr6)) {
        return 1;
    } else {
        return 0;
    }
}

int Ipv4Compare(char *str1, char *str2)
{
    struct sockaddr_in ip1;
    struct sockaddr_in ip2;
    memset(&ip1, 0, sizeof(struct sockaddr_in));
    memset(&ip2, 0, sizeof(struct sockaddr_in));

    inet_pton(AF_INET, str1, &ip1);
    inet_pton(AF_INET, str2, &ip2);
    return memcmp(&ip1, &ip2, sizeof(struct sockaddr_in));
}

int Ipv6Compare(char *str1, char *str2)
{
    struct sockaddr_in6 ip1;
    struct sockaddr_in6 ip2;
    memset(&ip1, 0, sizeof(struct sockaddr_in6));
    memset(&ip2, 0, sizeof(struct sockaddr_in6));
    inet_pton(AF_INET6, str1, &ip1);
    inet_pton(AF_INET6, str2, &ip2);
    return memcmp(&ip1, &ip2, sizeof(struct sockaddr_in6));
}


int IpCompare(char *str1, char *str2)
{
    if (IsValidIpv4(str1)) {
        return Ipv4Compare(str1, str2);
    } else {
        return Ipv6Compare(str1, str2);
    }
}

int  IsValidIpParam(int flag, char * setIp, char * IpInData, char *compareIp)
{
    // flag = 0,set start ip;flag = 1,set end ip
    if (flag == 0) {
        if (strcmp(compareIp, "") == 0) {
            if (strcmp(IpInData, "") == 0) {
                return 1;
            }
            if (IpCompare(setIp, IpInData) > 0) {
                return 0;
            }
            return 1;
        }
        if (IpCompare(setIp, compareIp) > 0) {
            return 0;
        }
        return 1;
    }
    if (strcmp(compareIp, "") == 0) {
        if (strcmp(IpInData, "") == 0) {
            return 1;
        }
        if (IpCompare(IpInData, setIp) > 0) {
            return 0;
        }
        return 1;
    }
    if (IpCompare(compareIp, setIp) > 0) {
        return 0;
    }
    return 1;
}
int  IsValidPortParam(int flag, int setPort, int portInData,int comparePort)
{
    // flag = 0,set start port;flag = 1,set end port
    if (flag == 0) {
        if (comparePort == 0) {
            if (setPort > portInData) {
                return 0;
            }
            return 1;
        }
        if (setPort > comparePort) {
            return 0;
        }
        return 1;
    }
    if (comparePort == 0) {
        if (setPort < portInData) {
            return 0;
        }
        return 1;
    }
    if (setPort < comparePort) {
        return 0;
    }
    return 1;
}
#endif

int natived_isValidMacAddress(const char *mac)
{
    regex_t regex;
    int reti = 0, ret = 0;
    char msgbuf[100];
    char *pattern = (char *)"^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$";

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
        ret = 1;
    }
    else if (reti == REG_NOMATCH)
    {
        regfree(&regex);
        ret = 0;
    }
    else
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        ret = 0;
    }

    if (0 == strcmp("00:00:00:00:00:00", mac))
    {
        ret = 0;
    }

    return ret;
}

#if 0
int main()
{
    mtrace();
#if 0
    const char *mac_str = "01:23:45:67:89:AB";
    unsigned char mac_bytes[MAC_ADDR_LEN];

    int result = mac_address_to_bytes(mac_str, mac_bytes);
    if (result == 0)
    {
        printf("MAC地址转换成功: ");
        print_mac_bytes(mac_bytes);
    }
    else
    {
        printf("转换失败，错误代码: %d\n", result);
    }
#else
    unsigned int mac[MAC_ADDR_LEN] = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};
    char mac_str[18]; // MAC 地址的最大长度，包括冒号和末尾的 '\0'

    // 转换字节为MAC地址字符串
    // byte_to_mac_string(mac, mac_str);
    // printf("MAC Address: %s\n", mac_str);

    // // 验证MAC地址格式
    // if (validate_mac_address(mac_str))
    // {
    //     printf("Valid MAC address format\n");
    // }
    // else
    // {
    //     printf("Invalid MAC address format\n");
    // }
    
    // setenv("MALLOC_TRACE", "./memleak.log", 1);
    printf("ret = %d\n", natived_isValidMacAddress("00:11:22:33:44:5g"));

#endif
    return 0;
}
#endif

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    regex_t preg;
    char *string = "a very simple simple simple string";
    char *pattern = "\\(sim[a-z]le\\) \\1";
    int rc;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    if (0 != (rc = regcomp(&preg, pattern, 0)))
    {
        printf("regcomp() failed, returning nonzero (%d)\n", rc);
        exit(EXIT_FAILURE);
    }

    if (0 != (rc = regexec(&preg, string, nmatch, pmatch, 0)))
    {
        printf("Failed to match '%s' with '%s',returning %d.\n",
               string, pattern, rc);
    }
    else
    {
        printf("With the whole expression, "
               "a matched substring \n\"%.*s\"\n is found at position %d to %d.\n",
               pmatch[0].rm_eo - pmatch[0].rm_so, &string[pmatch[0].rm_so],
               pmatch[0].rm_so, pmatch[0].rm_eo - 1);
        
        printf("With the sub-expression, "
               "a matched substring \n\"%.*s\"\n is found at position %d to %d.\n",
               pmatch[1].rm_eo - pmatch[1].rm_so, &string[pmatch[1].rm_so],
               pmatch[1].rm_so, pmatch[1].rm_eo - 1);
    }
    if (&preg)
    {
        printf("sssss\n");
        regfree(&preg);
    }
    return 0;

    /****************************************************************************
       The output should be similar to :

       With the whole expression, a matched substring "simple simple" is found
       at position 7 to 19.
       With the sub-expression, a matched substring "simple" is found
       at position 7 to 12.
    ****************************************************************************/
}