/*
 * @*************************************:
 * @FilePath     : /user/C/string/regex_mac_limit.c
 * @version      :
 * @Author       : dof
 * @Date         : 2026-01-08 15:19:20
 * @LastEditors  : dof
 * @LastEditTime : 2026-01-09 10:37:25
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

// ====================================mac/num begain=================================================================================

// 验证单个 MAC/数字 格式
int validate_mac_number(const char *str)
{
    regex_t regex;
    int ret;

    // 正则表达式解释：
    // ^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}  匹配MAC地址
    // /[0-9]+$                             匹配斜杠和数字
    char *pattern = "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}/[0-9]+$";

    // 编译正则表达式
    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        fprintf(stderr, "无法编译正则表达式\n");
        return -1;
    }

    // 执行匹配
    ret = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

// 验证整个字符串（多个用逗号分隔的MAC/数字）
int validate_all_mac_numbers(const char *input)
{
    char *str = strdup(input);
    char *token;
    char *saveptr = NULL;
    int count = 0;

    if (!str)
    {
        return 0;
    }

    // 按逗号分割字符串
    token = strtok_r(str, ",", &saveptr);
    while (token != NULL)
    {
        count++;

        // 去除可能的空格
        char *mac_num = token;
        while (*mac_num == ' ')
            mac_num++;

        // 验证每个MAC/数字格式
        if (!validate_mac_number(mac_num))
        {
            free(str);
            return 0; // 验证失败
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);

    // 确保至少有一个MAC/数字
    return count > 0 ? 1 : 0;
}

// 增强版本：同时提取和验证数字部分的范围
int validate_mac_number_extended(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 使用分组捕获MAC和数字部分
    char *pattern = "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}/([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        // 提取数字部分
        char num_str[64];
        int len = matches[2].rm_eo - matches[2].rm_so;

        if (len < sizeof(num_str))
        {
            strncpy(num_str, str + matches[2].rm_so, len);
            num_str[len] = '\0';

            // 转换为整数并验证范围（如果需要）
            long number = strtol(num_str, NULL, 10);

            // 示例：验证数字在有效范围内（1-999999）
            if (number > 0 && number <= 999999)
            {
                regfree(&regex);
                return 1;
            }
        }
    }

    regfree(&regex);
    return 0;
}

// ====================================mac/num end=================================================================================

// ====================================string/num begain=================================================================================

// 验证单个 字符串/数字 格式，字符串长度不超过128
int validate_string_number(const char *str)
{
    regex_t regex;
    int ret;

    // 正则表达式解释：
    // ^[a-zA-Z0-9_]{1,128}/[0-9]+$  匹配1-128个字母数字下划线，然后斜杠和数字
    // 如果需要允许其他字符，可以修改 [a-zA-Z0-9_] 部分
    char *pattern = "^[a-zA-Z0-9_]{1,128}/[0-9]+$";

    // 编译正则表达式
    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        fprintf(stderr, "无法编译正则表达式\n");
        return -1;
    }

    // 执行匹配
    ret = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

// 增强版本：验证并限制字符串长度不超过128
int validate_string_number_with_length(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 使用分组捕获字符串和数字部分
    char *pattern = "^([a-zA-Z0-9_]+)/([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        // 提取字符串部分并检查长度
        char string_part[129]; // 128 + 1 for null terminator
        int len = matches[1].rm_eo - matches[1].rm_so;

        // 检查字符串部分是否超过128个字符
        if (len > 128)
        {
            regfree(&regex);
            return 0;
        }

        if (len < sizeof(string_part))
        {
            strncpy(string_part, str + matches[1].rm_so, len);
            string_part[len] = '\0';

            // 可以在这里对字符串进行额外验证
            regfree(&regex);
            return 1;
        }
    }

    regfree(&regex);
    return 0;
}

// 验证整个字符串（多个用逗号分隔的字符串/数字）
int validate_all_string_numbers(const char *input)
{
    char *str = strdup(input);
    char *token;
    char *saveptr = NULL;
    int count = 0;

    if (!str)
    {
        return 0;
    }

    // 按逗号分割字符串
    token = strtok_r(str, ",", &saveptr);
    while (token != NULL)
    {
        count++;

        // 去除可能的空格
        char *item = token;
        while (*item == ' ')
            item++;

        // 验证每个字符串/数字格式
        if (!validate_string_number_with_length(item))
        {
            free(str);
            return 0; // 验证失败
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);

    // 确保至少有一个项
    return count > 0 ? 1 : 0;
}

// 更灵活的版本：允许自定义字符串字符集
int validate_custom_format(const char *str, const char *allowed_chars, int max_length)
{
    regex_t regex;
    int ret;
    char pattern[256];

    // 构建动态正则表达式
    snprintf(pattern, sizeof(pattern), "^[%s]{1,%d}/[0-9]+$", allowed_chars, max_length);

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

// ====================================string/num end=================================================================================

// ====================================ip-ip/num end=================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <arpa/inet.h>

// 使用正则表达式验证IPv4地址格式
int validate_ipv4_regex(const char *ip)
{
    regex_t regex;
    int ret;

    // IPv4正则表达式：四组0-255的数字，用点分隔
    char *pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, ip, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

// 使用正则表达式验证IPv6地址格式
int validate_ipv6_regex(const char *ip)
{
    regex_t regex;
    int ret;

    // IPv6正则表达式（简化版本，支持常见格式）
    // 支持压缩格式（::），但不验证所有极端情况
    char *pattern = "^(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|"
                    "(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})?::"
                    "(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})?|"
                    "([0-9a-fA-F]{1,4}:){1,7}:|"
                    ":((:[0-9a-fA-F]{1,4}){1,7}|:)|"
                    "fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]+|"
                    "::(ffff(:0{1,4})?:)?((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)|"
                    "([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, ip, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

// 使用inet_pton验证IP地址（更准确）
int validate_ip_inet(const char *ip)
{
    // 尝试IPv4
    struct in_addr addr4;
    if (inet_pton(AF_INET, ip, &addr4) == 1)
    {
        return 1; // IPv4有效
    }

    // 尝试IPv6
    struct in6_addr addr6;
    if (inet_pton(AF_INET6, ip, &addr6) == 1)
    {
        return 2; // IPv6有效
    }

    return 0; // 无效IP
}

// 验证单个 ip-ip/数字 格式
int validate_ip_range_number(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[4];

    // 正则表达式：ip-ip/数字
    // 注意：这个正则表达式只验证格式，不验证IP的有效性
    char *pattern = "^([^/]+)-([^/]+)/([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 4, matches, 0);
    if (ret == 0)
    {
        // 提取两个IP地址和数字
        char ip1[256], ip2[256], num_str[64];
        int len1 = matches[1].rm_eo - matches[1].rm_so;
        int len2 = matches[2].rm_eo - matches[2].rm_so;
        int len3 = matches[3].rm_eo - matches[3].rm_so;

        if (len1 < sizeof(ip1) && len2 < sizeof(ip2) && len3 < sizeof(num_str))
        {
            strncpy(ip1, str + matches[1].rm_so, len1);
            ip1[len1] = '\0';

            strncpy(ip2, str + matches[2].rm_so, len2);
            ip2[len2] = '\0';

            strncpy(num_str, str + matches[3].rm_so, len3);
            num_str[len3] = '\0';

            // 验证IP地址
            int valid1 = validate_ip_inet(ip1);
            int valid2 = validate_ip_inet(ip2);

            // 验证数字
            long number = strtol(num_str, NULL, 10);

            regfree(&regex);

            // 两个IP必须都有效，且类型相同（都是IPv4或都是IPv6）
            if (valid1 > 0 && valid2 > 0 && valid1 == valid2 && number > 0)
            {
                return valid1; // 返回1表示IPv4，2表示IPv6
            }
        }
    }

    regfree(&regex);
    return 0;
}

// 增强版本：严格验证格式，包括CIDR掩码范围
int validate_ip_range_number_strict(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[4];

    // 更严格的正则表达式，分别匹配IPv4和IPv6
    // 这个正则表达式比较复杂，分为IPv4和IPv6两种情况
    char *pattern_ipv4 = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                         "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
                         "-"
                         "((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                         "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
                         "/"
                         "([0-9]+)$";

    char *pattern_ipv6 = "^([0-9a-fA-F:]+)-([0-9a-fA-F:]+)/([0-9]+)$";

    // 先尝试IPv4模式
    ret = regcomp(&regex, pattern_ipv4, REG_EXTENDED);
    if (ret == 0)
    {
        ret = regexec(&regex, str, 0, NULL, 0);
        regfree(&regex);

        if (ret == 0)
        {
            // 进一步使用inet_pton验证
            return validate_ip_range_number(str) == 1 ? 1 : 0;
        }
    }

    // 尝试IPv6模式
    ret = regcomp(&regex, pattern_ipv6, REG_EXTENDED);
    if (ret == 0)
    {
        ret = regexec(&regex, str, 0, NULL, 0);
        regfree(&regex);

        if (ret == 0)
        {
            // 进一步使用inet_pton验证
            return validate_ip_range_number(str) == 2 ? 2 : 0;
        }
    }

    return 0;
}

// 验证逗号分隔的多个 ip-ip/数字
int validate_all_ip_ranges(const char *input)
{
    char *str = strdup(input);
    char *token;
    char *saveptr = NULL;
    int count = 0;
    int first_type = 0; // 记录第一个IP段的类型

    if (!str)
    {
        return 0;
    }

    // 按逗号分割字符串
    token = strtok_r(str, ",", &saveptr);
    while (token != NULL)
    {
        count++;

        // 去除可能的空格
        char *item = token;
        while (*item == ' ')
            item++;

        // 验证每个IP段
        int current_type = validate_ip_range_number_strict(item);

        if (current_type == 0)
        {
            free(str);
            return 0; // 验证失败
        }

        // 检查所有IP段类型是否一致
        if (count == 1)
        {
            first_type = current_type;
        }
        else if (current_type != first_type)
        {
            free(str);
            return 0; // 类型不一致
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);

    // 确保至少有一个IP段
    return count > 0 ? first_type : 0;
}

// 测试函数
// ====================================ip-ip/num end=================================================================================

// ====================================vlan/num begain=================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

// VLAN ID 验证函数
int validate_vlan_id(const char *vlan_str)
{
    // VLAN ID 必须是 1-4094 之间的数字
    long vlan_id = strtol(vlan_str, NULL, 10);
    return (vlan_id >= 1 && vlan_id <= 4094) ? 1 : 0;
}

// 方法1：使用简单正则表达式验证 vlan/数字 格式
int validate_vlan_number_simple(const char *str)
{
    regex_t regex;
    int ret;

    // 正则表达式：vlan/数字，vlan是1-4094的数字
    char *pattern = "^vlan/([1-9][0-9]{0,3}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-4])$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

// 方法2：使用分组捕获，分别验证vlan和数字部分
int validate_vlan_number_extended(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 正则表达式：捕获整个vlan标签和数字
    char *pattern = "^(vlan)/([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        // 提取vlan标签和数字部分
        char vlan_tag[32], num_str[32];
        int vlan_len = matches[1].rm_eo - matches[1].rm_so;
        int num_len = matches[2].rm_eo - matches[2].rm_so;

        if (vlan_len < sizeof(vlan_tag) && num_len < sizeof(num_str))
        {
            strncpy(vlan_tag, str + matches[1].rm_so, vlan_len);
            vlan_tag[vlan_len] = '\0';

            strncpy(num_str, str + matches[2].rm_so, num_len);
            num_str[num_len] = '\0';

            // 验证vlan标签（不区分大小写）
            if (strcasecmp(vlan_tag, "vlan") != 0)
            {
                regfree(&regex);
                return 0;
            }

            // 验证VLAN ID（1-4094）
            long vlan_id = strtol(num_str, NULL, 10);

            regfree(&regex);
            return (vlan_id >= 1 && vlan_id <= 4094) ? 1 : 0;
        }
    }

    regfree(&regex);
    return 0;
}

// 方法3：支持不同格式的VLAN表示法（vlanX、VLAN X等）
int validate_vlan_format_flexible(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 支持多种VLAN格式：
    // vlan/123, VLAN/123, vlan123, VLAN123
    char *pattern = "^((vlan|VLAN)[[:space:]]*[/]?[[:space:]]*)?([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        char num_str[32];
        int num_len = matches[3].rm_eo - matches[3].rm_so;

        if (num_len < sizeof(num_str))
        {
            strncpy(num_str, str + matches[3].rm_so, num_len);
            num_str[num_len] = '\0';

            // 验证VLAN ID
            long vlan_id = strtol(num_str, NULL, 10);

            regfree(&regex);
            return (vlan_id >= 1 && vlan_id <= 4094) ? 1 : 0;
        }
    }

    regfree(&regex);
    return 0;
}

// 方法4：严格验证 vlan/数字 格式，vlan不区分大小写
int validate_vlan_number_strict(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 严格模式：vlan/数字，vlan不区分大小写，必须有斜杠
    char *pattern = "^[[:space:]]*([Vv][Ll][Aa][Nn])[[:space:]]*/[[:space:]]*([0-9]+)[[:space:]]*$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        char num_str[32];
        int num_len = matches[2].rm_eo - matches[2].rm_so;

        if (num_len < sizeof(num_str))
        {
            strncpy(num_str, str + matches[2].rm_so, num_len);
            num_str[num_len] = '\0';

            // 验证VLAN ID
            long vlan_id = strtol(num_str, NULL, 10);

            regfree(&regex);
            return (vlan_id >= 1 && vlan_id <= 4094) ? 1 : 0;
        }
    }

    regfree(&regex);
    return 0;
}

// 验证逗号分隔的多个 vlan/数字
int validate_all_vlan_numbers(const char *input)
{
    char *str = strdup(input);
    char *token;
    char *saveptr = NULL;
    int count = 0;

    if (!str)
    {
        return 0;
    }

    // 按逗号分割字符串
    token = strtok_r(str, ",", &saveptr);
    while (token != NULL)
    {
        count++;

        // 去除可能的空格
        char *item = token;
        while (*item == ' ')
            item++;

        // 去除末尾空格
        char *end = item + strlen(item) - 1;
        while (end > item && *end == ' ')
        {
            *end = '\0';
            end--;
        }

        // 验证每个vlan/数字格式
        if (!validate_vlan_number_strict(item))
        {
            free(str);
            return 0; // 验证失败
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);

    // 确保至少有一个vlan
    return count > 0 ? 1 : 0;
}

// 支持带范围的VLAN表示法（vlan/start-end/其他参数）
int validate_vlan_range_format(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[4];

    // 支持：vlan/100-200/参数 格式
    char *pattern = "^[[:space:]]*([Vv][Ll][Aa][Nn])[[:space:]]*/[[:space:]]*"
                    "([0-9]+)[[:space:]]*-[[:space:]]*([0-9]+)"
                    "([[:space:]]*/[[:space:]]*[0-9]+)?[[:space:]]*$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 4, matches, 0);
    if (ret == 0)
    {
        char start_str[32], end_str[32];
        int start_len = matches[2].rm_eo - matches[2].rm_so;
        int end_len = matches[3].rm_eo - matches[3].rm_so;

        if (start_len < sizeof(start_str) && end_len < sizeof(end_str))
        {
            strncpy(start_str, str + matches[2].rm_so, start_len);
            start_str[start_len] = '\0';

            strncpy(end_str, str + matches[3].rm_so, end_len);
            end_str[end_len] = '\0';

            // 验证VLAN范围
            long start_id = strtol(start_str, NULL, 10);
            long end_id = strtol(end_str, NULL, 10);

            regfree(&regex);

            // 验证范围有效性
            if (start_id >= 1 && start_id <= 4094 &&
                end_id >= 1 && end_id <= 4094 &&
                start_id <= end_id)
            {
                return 1;
            }
        }
    }

    regfree(&regex);
    return 0;
}

// ====================================vlan/num end=================================================================================

#define MAC_NUM_TEST 0
#define STRING_NUM_TEST 0
#define VLAN_NUM_TEST 1
#define HOST_NUM_TEST 0
#define IP_NUM_TEST 0

int main()
{

#if MAC_NUM_TEST
    // 测试用例
    const char *test_cases[] = {
        "00:1A:2B:3C:4D:5E/123",    // 正确
        "FF:FF:FF:FF:FF:FF/1",      // 正确
        "01:23:45:67:89:AB/999999", // 正确
        "00:1A:2B:3C:4D:5E/234",    // 正确
        "00:1a:2b:3c:4d:5e/10",     // 正确（小写字母）
        "00:1A:2B:3C:4D:5G/123",    // 错误（G不是十六进制）
        "00:1A:2B:3C:4D/123",       // 错误（只有5组）
        "00:1A:2B:3C:4D:5E:6F/123", // 错误（7组）
        "00-1A-2B-3C-4D-5E/123",    // 错误（使用短横线）
        "00:1A:2B:3C:4D:5E/",       // 错误（没有数字）
        "00:1A:2B:3C:4D:5E/0",      // 正确（数字为0，如果需要可以修改验证规则）
        "00:1A:2B:3C:4D:5E/abc",    // 错误（数字部分包含字母）
    };

    printf("单个MAC/数字验证测试：\n");
    printf("====================\n");
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        int result = validate_mac_number(test_cases[i]);
        printf("%-35s : %s\n",
               test_cases[i],
               result ? "✓ 格式正确" : "✗ 格式错误");
    }

    // 测试多个MAC/数字的情况
    printf("\n多个MAC/数字验证测试：\n");
    printf("======================\n");

    const char *multi_test = "00:1A:2B:3C:4D:5E/123,01:23:45:67:89:AB/456,FF:FF:FF:FF:FF:FF/789";
    int result = validate_all_mac_numbers(multi_test);
    printf("输入: %s\n", multi_test);
    printf("验证结果: %s\n", result ? "✓ 全部格式正确" : "✗ 格式错误");

    const char *multi_test_bad = "00:1A:2B:3C:4D:5E/123,01:23:45:67:89:AB,FF:FF:FF:FF:FF:FF/789";
    result = validate_all_mac_numbers(multi_test_bad);
    printf("\n输入: %s\n", multi_test_bad);
    printf("验证结果: %s\n", result ? "✓ 全部格式正确" : "✗ 格式错误");

#endif
// ================================================================================================================
#if STRING_NUM_TEST
    // 测试用例
    printf("单个 字符串/数字 验证测试：\n");
    printf("===========================\n");

    const char *test_cases[] = {
        "asdasdas/234",    // 正确
        "test123/1",       // 正确
        "hello_world/999", // 正确（包含下划线）
        "TEST/456",        // 正确
        "a/1",             // 正确（最小长度）

        // 测试边界情况
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_1234567890_extra/123", // 长度超过128会失败

        // 错误用例
        "测试/123",         // 错误（包含非ASCII字符）
        "hello-world/123",  // 错误（包含短横线）
        "hello world/123",  // 错误（包含空格）
        "test@example/123", // 错误（包含@符号）
        "test/",            // 错误（没有数字）
        "test/abc",         // 错误（数字部分包含字母）
        "/123",             // 错误（没有字符串部分）
        "test//123",        // 错误（多个斜杠）
        "test/123/extra",   // 错误（多余的部分）
    };

    // 生成一个刚好128字符的测试字符串
    char long_string[130];
    memset(long_string, 'a', 128);
    long_string[128] = '/';
    long_string[129] = '1';
    long_string[130] = '\0';

    printf("测试128字符边界：\n");
    printf("%s : %s\n", "128个a字符",
           validate_string_number_with_length(long_string) ? "✓ 格式正确" : "✗ 格式错误");

    // 129字符应该失败
    char too_long_string[132];
    memset(too_long_string, 'a', 129);
    too_long_string[129] = '/';
    too_long_string[130] = '1';
    too_long_string[131] = '\0';

    printf("%s : %s\n", "129个a字符",
           validate_string_number_with_length(too_long_string) ? "✓ 格式正确" : "✗ 格式错误");

    printf("\n详细测试：\n");
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        int result = validate_string_number_with_length(test_cases[i]);
        printf("%-60s : %s\n",
               test_cases[i],
               result ? "✓ 格式正确" : "✗ 格式错误");
    }

    // 测试多个字符串/数字的情况
    printf("\n多个字符串/数字验证测试：\n");
    printf("=========================\n");

    const char *multi_test = "asdasdas/234,test123/456,hello_world/789,example/123";
    int result = validate_all_string_numbers(multi_test);
    printf("输入: %s\n", multi_test);
    printf("验证结果: %s\n", result ? "✓ 全部格式正确" : "✗ 格式错误");

    const char *multi_test_bad = "asdasdas/234,test-123/456,hello_world/789";
    result = validate_all_string_numbers(multi_test_bad);
    printf("\n输入: %s\n", multi_test_bad);
    printf("验证结果: %s\n", result ? "✓ 全部格式正确" : "✗ 格式错误");

    // 测试自定义字符集
    printf("\n自定义字符集测试：\n");
    printf("==================\n");

    // 允许字母、数字、下划线和短横线
    const char *custom_allowed = "a-zA-Z0-9_-";
    const char *custom_tests[] = {
        "hello-world/123",  // 正确（使用自定义字符集）
        "test_123-456/789", // 正确
        "test@example/123", // 错误（@不在允许的字符集中）
    };

    for (int i = 0; i < sizeof(custom_tests) / sizeof(custom_tests[0]); i++)
    {
        int custom_result = validate_custom_format(custom_tests[i], custom_allowed, 128);
        printf("%-40s (自定义字符集) : %s\n",
               custom_tests[i],
               custom_result ? "✓ 格式正确" : "✗ 格式错误");
    }
#endif
    // ================================================================================================================

#if VLAN_NUM_TEST
    printf("VLAN格式验证测试：\n");
    printf("================\n\n");

    // 测试用例
    printf("基本 vlan/数字 格式测试：\n");
    printf("------------------------\n");
    const char *test_cases[] = {
        // 正确用例
        "vlan/1",
        "vlan/100",
        "vlan/1000",
        "vlan/4094",
        "VLAN/100",
        "Vlan/200",
        "vLan/300",
        "vlan/ 100 ",
        " vlan/100 ",

        // 错误用例
        "vlan/0",         // 错误：VLAN ID不能为0
        "vlan/4095",      // 错误：超过最大VLAN ID
        "vlan/9999",      // 错误：超过最大VLAN ID
        "vlan/abc",       // 错误：非数字
        "vlan/",          // 错误：缺少数字
        "vlan",           // 错误：只有vlan没有数字
        "/100",           // 错误：缺少vlan标签
        "vlan /100",      // 注意：严格模式下允许空格
        "vlan/100/extra", // 错误：多余部分
        "eth/100",        // 错误：不是vlan
        "vlan1/100",      // 错误：格式不对
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        int result = validate_vlan_number_strict(test_cases[i]);
        printf("%-20s : %s\n",
               test_cases[i],
               result ? "✓ 格式正确" : "✗ 格式错误");
    }

    // 测试VLAN范围格式
    printf("\nVLAN范围格式测试：\n");
    printf("-----------------\n");
    const char *range_tests[] = {
        "vlan/1-100",
        "vlan/100-200",
        "vlan/1000-2000",
        "VLAN/10-20",
        "vlan/ 100 - 200 ",

        // 错误用例
        "vlan/0-100",       // 错误：起始VLAN为0
        "vlan/100-0",       // 错误：结束VLAN为0
        "vlan/4095-5000",   // 错误：超过最大VLAN
        "vlan/200-100",     // 错误：起始大于结束
        "vlan/100-200/300", // 正确：带额外参数
    };

    for (int i = 0; i < sizeof(range_tests) / sizeof(range_tests[0]); i++)
    {
        int result = validate_vlan_range_format(range_tests[i]);
        printf("%-25s : %s\n",
               range_tests[i],
               result ? "✓ 格式正确" : "✗ 格式错误");
    }

    // 测试多个VLAN
    printf("\n多个VLAN测试（逗号分隔）：\n");
    printf("-------------------------\n");

    const char *multi_vlan = "vlan/100,vlan/200,vlan/300,vlan/400";
    int result = validate_all_vlan_numbers(multi_vlan);
    printf("输入: %s\n", multi_vlan);
    printf("验证结果: %s\n", result ? "✓ 所有格式正确" : "✗ 格式错误");

    const char *multi_vlan_spaces = " vlan/100 , vlan/200 , vlan/300 , vlan/400 ";
    result = validate_all_vlan_numbers(multi_vlan_spaces);
    printf("\n输入: %s\n", multi_vlan_spaces);
    printf("验证结果: %s\n", result ? "✓ 所有格式正确" : "✗ 格式错误");

    const char *multi_vlan_mixed = "VLAN/100,vlan/200,Vlan/300,vLaN/400";
    result = validate_all_vlan_numbers(multi_vlan_mixed);
    printf("\n输入: %s\n", multi_vlan_mixed);
    printf("验证结果: %s\n", result ? "✓ 所有格式正确" : "✗ 格式错误");

    const char *multi_vlan_bad = "vlan/100,vlan/4095,vlan/300";
    result = validate_all_vlan_numbers(multi_vlan_bad);
    printf("\n输入: %s\n", multi_vlan_bad);
    printf("验证结果: %s\n", result ? "✓ 所有格式正确" : "✗ 格式错误（包含无效VLAN ID）");

    // 测试不同验证方法
    printf("\n不同验证方法对比：\n");
    printf("-------------------\n");
    const char *compare_tests[] = {
        "vlan/100",
        "VLAN/100",
        "vlan/4094",
        "vlan/0",
        "vlan/4095",
        "vlan100",  // 灵活格式允许
        "VLAN 100", // 灵活格式允许
    };

    printf("%-15s %-10s %-10s %-10s\n",
           "输入", "简单方法", "严格方法", "灵活方法");
    printf("%-15s %-10s %-10s %-10s\n",
           "---", "---", "---", "---");

    for (int i = 0; i < sizeof(compare_tests) / sizeof(compare_tests[0]); i++)
    {
        int simple = validate_vlan_number_simple(compare_tests[i]);
        int strict = validate_vlan_number_strict(compare_tests[i]);
        int flexible = validate_vlan_format_flexible(compare_tests[i]);

        printf("%-15s %-10s %-10s %-10s\n",
               compare_tests[i],
               simple ? "✓" : "✗",
               strict ? "✓" : "✗",
               flexible ? "✓" : "✗");
    }

    // 批量测试VLAN ID边界值
    printf("\nVLAN ID边界值测试：\n");
    printf("------------------\n");

    char test_buffer[32];
    long boundary_values[] = {0, 1, 2, 100, 1000, 4093, 4094, 4095, 5000};

    for (int i = 0; i < sizeof(boundary_values) / sizeof(boundary_values[0]); i++)
    {
        snprintf(test_buffer, sizeof(test_buffer), "vlan/%ld", boundary_values[i]);
        int valid = validate_vlan_number_strict(test_buffer);
        printf("vlan/%-6ld : %s\n",
               boundary_values[i],
               valid ? "✓ 有效VLAN" : "✗ 无效VLAN");
    }

#endif

#if IP_NUM_TEST
    printf("IP范围验证测试：\n");
    printf("===============\n\n");

    // IPv4测试用例
    printf("IPv4测试：\n");
    printf("---------\n");
    const char *ipv4_tests[] = {
        "192.168.1.1-192.168.1.10/24",  // 正确
        "10.0.0.1-10.0.0.255/16",       // 正确
        "172.16.0.1-172.16.255.255/12", // 正确
        "0.0.0.0-255.255.255.255/0",    // 正确

        // 错误用例
        "192.168.1.1-192.168.1.10/",    // 错误：缺少数字
        "192.168.1.1-192.168.1.10/abc", // 错误：数字无效
        "192.168.1.1-192.168.1.256/24", // 错误：IP地址无效
        "192.168.1-192.168.1.10/24",    // 错误：IP格式错误
        "192.168.1.1:192.168.1.10/24",  // 错误：使用冒号而不是短横线
        "192.168.1.1-192.168.1.10/33",  // 错误：掩码超出范围（对IPv4）
    };

    for (int i = 0; i < sizeof(ipv4_tests) / sizeof(ipv4_tests[0]); i++)
    {
        int result = validate_ip_range_number_strict(ipv4_tests[i]);
        printf("%-40s : %s\n",
               ipv4_tests[i],
               result == 1 ? "✓ IPv4格式正确" : "✗ 格式错误");
    }

    // IPv6测试用例
    printf("\nIPv6测试：\n");
    printf("---------\n");
    const char *ipv6_tests[] = {
        "2001:0db8:85a3::8a2e:0370:7334-2001:0db8:85a3::8a2e:0370:7335/64",
        "fe80::1-fe80::ffff/10",
        "::1-::2/128",
        "2001:db8::1-2001:db8::ffff/32",

        // 错误用例
        "2001:db8::1-2001:db8::ffff/",    // 错误：缺少数字
        "2001:db8::1-2001:db8::ffff/abc", // 错误：数字无效
        "2001:db8::1-2001:db8:::ffff/64", // 错误：IPv6格式错误
        "2001:db8::1-192.168.1.1/64",     // 错误：IPv4和IPv6混合
    };

    for (int i = 0; i < sizeof(ipv6_tests) / sizeof(ipv6_tests[0]); i++)
    {
        int result = validate_ip_range_number_strict(ipv6_tests[i]);
        printf("%-70s : %s\n",
               ipv6_tests[i],
               result == 2 ? "✓ IPv6格式正确" : "✗ 格式错误");
    }

    // 测试多个IP段
    printf("\n多个IP段测试：\n");
    printf("--------------\n");

    const char *multi_ipv4 = "192.168.1.1-192.168.1.10/24,10.0.0.1-10.0.0.255/16,172.16.0.1-172.16.255.255/12";
    int result = validate_all_ip_ranges(multi_ipv4);
    printf("输入: %s\n", multi_ipv4);
    printf("验证结果: %s\n", result == 1 ? "✓ 所有IPv4格式正确" : "✗ 格式错误");

    const char *multi_ipv6 = "2001:db8::1-2001:db8::ffff/64,fe80::1-fe80::ffff/10";
    result = validate_all_ip_ranges(multi_ipv6);
    printf("\n输入: %s\n", multi_ipv6);
    printf("验证结果: %s\n", result == 2 ? "✓ 所有IPv6格式正确" : "✗ 格式错误");

    const char *multi_mixed = "192.168.1.1-192.168.1.10/24,2001:db8::1-2001:db8::ffff/64";
    result = validate_all_ip_ranges(multi_mixed);
    printf("\n输入: %s\n", multi_mixed);
    printf("验证结果: %s\n", result ? "✓ 格式正确" : "✗ 格式错误（混合类型不被允许）");

    // 测试IP地址验证函数
    printf("\n单个IP地址验证测试：\n");
    printf("--------------------\n");
    const char *single_ips[] = {
        "192.168.1.1",
        "255.255.255.255",
        "0.0.0.0",
        "256.256.256.256", // 无效
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "2001:db8::1",
        "fe80::1",
        "::1",
        "invalid_ip",
    };

    for (int i = 0; i < sizeof(single_ips) / sizeof(single_ips[0]); i++)
    {
        int type = validate_ip_inet(single_ips[i]);
        printf("%-40s : ", single_ips[i]);
        if (type == 1)
        {
            printf("✓ 有效IPv4\n");
        }
        else if (type == 2)
        {
            printf("✓ 有效IPv6\n");
        }
        else
        {
            printf("✗ 无效IP地址\n");
        }
    }

#endif

#if HOST_NUM_TEST
#endif

    return 0;
}