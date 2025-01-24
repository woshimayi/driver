/*
 * @*************************************:
 * @FilePath     : /network/net_util/mac_util.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-01-06 15:25:27
 * @LastEditors  : dof
 * @LastEditTime : 2025-01-06 15:41:30
 * @Descripttion :  mac char * to byte
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

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

int main()
{
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
    byte_to_mac_string(mac, mac_str);
    printf("MAC Address: %s\n", mac_str);

    // 验证MAC地址格式
    if (validate_mac_address(mac_str))
    {
        printf("Valid MAC address format\n");
    }
    else
    {
        printf("Invalid MAC address format\n");
    }

#endif
    return 0;
}
