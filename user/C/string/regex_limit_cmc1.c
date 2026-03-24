#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <arpa/inet.h>
#include <ctype.h>

#undef PP // off: dbgcfg emu:0:0 on: dbgcfg emu:8:0
#define PP(fmt, ...) printf("\033[0;32;31m[zzzzz :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##__VA_ARGS__)

// ==================== 通用工具函数 ====================
void safe_strncpy(char *dest, const char *src, size_t dest_size, int start, int len)
{
    if (dest == NULL || src == NULL || dest_size == 0 || start < 0 || len <= 0)
    {
        if (dest && dest_size > 0)
            dest[0] = '\0';
        return;
    }

    size_t copy_len = (size_t)len;
    if (copy_len >= dest_size)
    {
        copy_len = dest_size - 1;
    }

    if (copy_len > 0)
    {
        strncpy(dest, src + start, copy_len);
        dest[copy_len] = '\0';
    }
    else
    {
        dest[0] = '\0';
    }
}

void trim_string(char *str)
{
    if (!str)
        return;

    char *start = str;
    while (*start && isspace((unsigned char)*start))
    {
        start++;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
    {
        end--;
    }

    *(end + 1) = '\0';

    if (start != str)
    {
        memmove(str, start, end - start + 2);
    }
}

// ==================== 1. MAC地址验证 ====================
int validate_mac_format_single(const char *str)
{
    regex_t regex;
    int ret;

    char *pattern = "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}/[0-9]+$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    return ret == 0 ? 1 : 0;
}

int parse_mac_list(const char *input, char ***mac_list, int *count)
{
    if (!input || !mac_list || !count)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;

    *mac_list = malloc(max_items * sizeof(char *));
    if (!*mac_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
        (*mac_list)[i] = NULL;

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        if (validate_mac_format_single(item))
        {
            (*mac_list)[item_count] = strdup(item);
            if (!(*mac_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                    free((*mac_list)[i]);
                free(*mac_list);
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            for (int i = 0; i < item_count; i++)
                free((*mac_list)[i]);
            free(*mac_list);
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    if (item_count == 0)
    {
        free(*mac_list);
        return 0;
    }

    return 1;
}

// ==================== 2. 字符串/数字验证 ====================
int validate_string_number_single(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    char *pattern = "^([a-zA-Z0-9_]+)/([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        if (matches[1].rm_so == -1)
        {
            regfree(&regex);
            return 0;
        }

        int len = matches[1].rm_eo - matches[1].rm_so;
        regfree(&regex);
        return (len <= 128 && len > 0) ? 1 : 0;
    }

    regfree(&regex);
    return 0;
}

int parse_string_number_list(const char *input, char ***str_list, int *count)
{
    if (!input || !str_list || !count)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;

    *str_list = malloc(max_items * sizeof(char *));
    if (!*str_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
        (*str_list)[i] = NULL;

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        if (validate_string_number_single(item))
        {
            (*str_list)[item_count] = strdup(item);
            if (!(*str_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                    free((*str_list)[i]);
                free(*str_list);
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            for (int i = 0; i < item_count; i++)
                free((*str_list)[i]);
            free(*str_list);
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    if (item_count == 0)
    {
        free(*str_list);
        return 0;
    }

    return 1;
}

// ==================== 3. IP范围验证 ====================
int validate_ip_range_single(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[4];

    char *pattern = "^([^/]+)-([^/]+)/([0-9]+)$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 4, matches, 0);
    if (ret == 0)
    {
        char ip1[256], ip2[256];

        if (matches[1].rm_so == -1 || matches[2].rm_so == -1)
        {
            regfree(&regex);
            return 0;
        }

        safe_strncpy(ip1, str, sizeof(ip1), matches[1].rm_so,
                     matches[1].rm_eo - matches[1].rm_so);
        safe_strncpy(ip2, str, sizeof(ip2), matches[2].rm_so,
                     matches[2].rm_eo - matches[2].rm_so);

        struct in_addr addr4;
        struct in6_addr addr6;

        int valid1 = 0, valid2 = 0;

        if (inet_pton(AF_INET, ip1, &addr4) == 1)
        {
            valid1 = 1;
        }
        else if (inet_pton(AF_INET6, ip1, &addr6) == 1)
        {
            valid1 = 2;
        }

        if (inet_pton(AF_INET, ip2, &addr4) == 1)
        {
            valid2 = 1;
        }
        else if (inet_pton(AF_INET6, ip2, &addr6) == 1)
        {
            valid2 = 2;
        }

        regfree(&regex);

        if (valid1 > 0 && valid2 > 0 && valid1 == valid2)
        {
            return valid1;
        }
    }

    regfree(&regex);
    return 0;
}

int parse_ip_range_list(const char *input, char ***ip_list, int *count, int *ip_type)
{
    if (!input || !ip_list || !count || !ip_type)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;
    int first_type = 0;

    *ip_list = malloc(max_items * sizeof(char *));
    if (!*ip_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
        (*ip_list)[i] = NULL;

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        int current_type = validate_ip_range_single(item);

        if (current_type > 0)
        {
            if (item_count == 0)
            {
                first_type = current_type;
                *ip_type = current_type;
            }
            else if (current_type != first_type)
            {
                for (int i = 0; i < item_count; i++)
                    free((*ip_list)[i]);
                free(*ip_list);
                free(str);
                return 0;
            }

            (*ip_list)[item_count] = strdup(item);
            if (!(*ip_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                    free((*ip_list)[i]);
                free(*ip_list);
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            for (int i = 0; i < item_count; i++)
                free((*ip_list)[i]);
            free(*ip_list);
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    if (item_count == 0)
    {
        free(*ip_list);
        return 0;
    }

    return 1;
}

// ==================== 4. VLAN验证 ====================
// ==================== 4. 数字/数字验证（修改VLAN为num/num） ====================
// 数字/数字格式：第一个数字范围，第二个数字范围
int validate_num_num_single(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 数字/数字格式：num/num
    // 两个数字都可以是任意正整数
    char *pattern = "^[[:space:]]*([0-9]+)[[:space:]]*/[[:space:]]*([0-9]+)[[:space:]]*$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        if (matches[1].rm_so == -1 || matches[2].rm_so == -1)
        {
            regfree(&regex);
            return 0;
        }

        char num1_str[32], num2_str[32];
        int num1_len = matches[1].rm_eo - matches[1].rm_so;
        int num2_len = matches[2].rm_eo - matches[2].rm_so;

        if (num1_len > 0 && num1_len < sizeof(num1_str) &&
            num2_len > 0 && num2_len < sizeof(num2_str))
        {
            strncpy(num1_str, str + matches[1].rm_so, num1_len);
            num1_str[num1_len] = '\0';

            strncpy(num2_str, str + matches[2].rm_so, num2_len);
            num2_str[num2_len] = '\0';

            char *endptr1, *endptr2;
            long num1 = strtol(num1_str, &endptr1, 10);
            long num2 = strtol(num2_str, &endptr2, 10);

            regfree(&regex);

            // 验证转换成功且数字非负
            if (endptr1 != num1_str && *endptr1 == '\0' &&
                endptr2 != num2_str && *endptr2 == '\0' &&
                num1 >= 0 && num2 >= 0)
            {
                return 1;
            }
        }
    }

    regfree(&regex);
    return 0;
}

// 增强版：可以限制数字范围
int validate_num_num_with_range(const char *str, long min1, long max1, long min2, long max2)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    char *pattern = "^[[:space:]]*([0-9]+)[[:space:]]*/[[:space:]]*([0-9]+)[[:space:]]*$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        if (matches[1].rm_so == -1 || matches[2].rm_so == -1)
        {
            regfree(&regex);
            return 0;
        }

        char num1_str[32], num2_str[32];
        int num1_len = matches[1].rm_eo - matches[1].rm_so;
        int num2_len = matches[2].rm_eo - matches[2].rm_so;

        if (num1_len > 0 && num1_len < sizeof(num1_str) &&
            num2_len > 0 && num2_len < sizeof(num2_str))
        {
            strncpy(num1_str, str + matches[1].rm_so, num1_len);
            num1_str[num1_len] = '\0';

            strncpy(num2_str, str + matches[2].rm_so, num2_len);
            num2_str[num2_len] = '\0';

            char *endptr1, *endptr2;
            long num1 = strtol(num1_str, &endptr1, 10);
            long num2 = strtol(num2_str, &endptr2, 10);

            regfree(&regex);

            if (endptr1 != num1_str && *endptr1 == '\0' &&
                endptr2 != num2_str && *endptr2 == '\0')
            {
                // 检查数字范围
                if (num1 >= min1 && num1 <= max1 &&
                    num2 >= min2 && num2 <= max2)
                {
                    return 1;
                }
            }
        }
    }

    regfree(&regex);
    return 0;
}

// 解析数字/数字列表
int parse_num_num_list(const char *input, char ***num_list, int *count)
{
    if (!input || !num_list || !count)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;

    *num_list = malloc(max_items * sizeof(char *));
    if (!*num_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
        (*num_list)[i] = NULL;

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        if (validate_num_num_single(item))
        {
            (*num_list)[item_count] = strdup(item);
            if (!(*num_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                    free((*num_list)[i]);
                free(*num_list);
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            for (int i = 0; i < item_count; i++)
                free((*num_list)[i]);
            free(*num_list);
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    if (item_count == 0)
    {
        free(*num_list);
        return 0;
    }

    return 1;
}

// 支持不同数字范围的版本
int parse_num_num_list_with_range(const char *input, char ***num_list, int *count,
                                  long min1, long max1, long min2, long max2)
{
    if (!input || !num_list || !count)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;

    *num_list = malloc(max_items * sizeof(char *));
    if (!*num_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
        (*num_list)[i] = NULL;

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        if (validate_num_num_with_range(item, min1, max1, min2, max2))
        {
            (*num_list)[item_count] = strdup(item);
            if (!(*num_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                    free((*num_list)[i]);
                free(*num_list);
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            for (int i = 0; i < item_count; i++)
                free((*num_list)[i]);
            free(*num_list);
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    if (item_count == 0)
    {
        free(*num_list);
        return 0;
    }

    return 1;
}

// ==================== 5. 合并的接口验证（允许LAN和SSID混合） ====================
// 接口类型枚举
typedef enum
{
    INTERFACE_TYPE_UNKNOWN = 0,
    INTERFACE_TYPE_LAN,
    INTERFACE_TYPE_SSID,
    INTERFACE_TYPE_MIXED // LAN和SSID混合
} InterfaceType;

// 验证单个接口（LAN或SSID）
int validate_interface_item(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 合并的正则表达式，匹配 LANx/数字 或 SSIDx/数字
    // LAN1-8, SSID1-12，不区分大小写
    char *pattern = "^[[:space:]]*([Ll][Aa][Nn]([1-8])|[Ss][Ss][Ii][Dd]([1-9]|1[0-2]))[[:space:]]*/[[:space:]]*([0-9]+)[[:space:]]*$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        if (matches[1].rm_so == -1)
        {
            regfree(&regex);
            return 0;
        }

        char interface_name[32];
        int name_len = matches[1].rm_eo - matches[1].rm_so;

        if (name_len > 0 && name_len < sizeof(interface_name))
        {
            strncpy(interface_name, str + matches[1].rm_so, name_len);
            interface_name[name_len] = '\0';

            // 转换为大写以便比较
            char upper_name[32];
            for (int i = 0; i < name_len; i++)
            {
                upper_name[i] = toupper(interface_name[i]);
            }
            upper_name[name_len] = '\0';

            // 检查是LAN还是SSID
            if (strncmp(upper_name, "LAN", 3) == 0)
            {
                // 验证LAN编号 (1-8)
                char *num_str = upper_name + 3;
                int lan_num = atoi(num_str);
                if (lan_num >= 1 && lan_num <= 8)
                {
                    regfree(&regex);
                    return 1;
                }
            }
            else if (strncmp(upper_name, "SSID", 4) == 0)
            {
                // 验证SSID编号 (1-12)
                char *num_str = upper_name + 4;
                int ssid_num = atoi(num_str);
                if (ssid_num >= 1 && ssid_num <= 12)
                {
                    regfree(&regex);
                    return 1;
                }
            }
        }
    }

    regfree(&regex);
    return 0;
}

// 判断接口类型
InterfaceType detect_interface_type(const char *str)
{
    char interface_name[32];
    char *slash = strchr(str, '/');
    if (!slash)
        return INTERFACE_TYPE_UNKNOWN;

    int name_len = slash - str;
    if (name_len <= 0 || name_len >= sizeof(interface_name))
    {
        return INTERFACE_TYPE_UNKNOWN;
    }

    strncpy(interface_name, str, name_len);
    interface_name[name_len] = '\0';

    // 去除空格
    char *start = interface_name;
    while (*start && isspace((unsigned char)*start))
        start++;

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';

    // 转换为大写
    char upper_name[32];
    for (int i = 0; start[i]; i++)
    {
        upper_name[i] = toupper(start[i]);
    }
    upper_name[strlen(start)] = '\0';

    // 检查类型
    if (strncmp(upper_name, "LAN", 3) == 0)
    {
        char *num_str = upper_name + 3;
        int lan_num = atoi(num_str);
        if (lan_num >= 1 && lan_num <= 8)
        {
            return INTERFACE_TYPE_LAN;
        }
    }
    else if (strncmp(upper_name, "SSID", 4) == 0)
    {
        char *num_str = upper_name + 4;
        int ssid_num = atoi(num_str);
        if (ssid_num >= 1 && ssid_num <= 12)
        {
            return INTERFACE_TYPE_SSID;
        }
    }

    return INTERFACE_TYPE_UNKNOWN;
}

// 解析接口列表（允许LAN和SSID混合）
int parse_interface_list(const char *input, char ***interface_list, int *count, InterfaceType *interface_type)
{
    if (!input || !interface_list || !count || !interface_type)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;
    int lan_count = 0, ssid_count = 0;

    *interface_list = malloc(max_items * sizeof(char *));
    if (!*interface_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
        (*interface_list)[i] = NULL;

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        if (validate_interface_item(item))
        {
            (*interface_list)[item_count] = strdup(item);
            if (!(*interface_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                    free((*interface_list)[i]);
                free(*interface_list);
                *interface_list = NULL;
                free(str);
                return 0;
            }

            // 统计类型
            InterfaceType item_type = detect_interface_type(item);
            if (item_type == INTERFACE_TYPE_LAN)
            {
                lan_count++;
            }
            else if (item_type == INTERFACE_TYPE_SSID)
            {
                ssid_count++;
            }

            item_count++;
        }
        else
        {
            for (int i = 0; i < item_count; i++)
                free((*interface_list)[i]);
            free(*interface_list);
            *interface_list = NULL;
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    // 确定整体类型
    if (lan_count > 0 && ssid_count == 0)
    {
        *interface_type = INTERFACE_TYPE_LAN;
    }
    else if (ssid_count > 0 && lan_count == 0)
    {
        *interface_type = INTERFACE_TYPE_SSID;
    }
    else if (lan_count > 0 && ssid_count > 0)
    {
        *interface_type = INTERFACE_TYPE_MIXED;
    }
    else
    {
        *interface_type = INTERFACE_TYPE_UNKNOWN;
    }

    if (item_count == 0)
    {
        free(*interface_list);
        *interface_list = NULL;
        return 0;
    }

    return 1;
}

// ==================== 6. 主验证函数 ====================
typedef enum
{
    TYPE_MAC,
    TYPE_STRING_NUM,
    TYPE_IP_RANGE,
    TYPE_VLAN,
    TYPE_INTERFACE // 合并的LAN/SSID接口
} InputType;

int validate_specific_type(const char *input, InputType type,
                           void ***result_list, int *count, int *extra_info)
{
    if (!input || !result_list || !count)
        return 0;

    switch (type)
    {
    case TYPE_MAC:
        return parse_mac_list(input, (char ***)result_list, count);
    case TYPE_STRING_NUM:
        return parse_string_number_list(input, (char ***)result_list, count);
    case TYPE_IP_RANGE:
        return parse_ip_range_list(input, (char ***)result_list, count, extra_info);
    case TYPE_VLAN:
        return parse_vlan_list(input, (char ***)result_list, count);
    case TYPE_INTERFACE:
        return parse_interface_list(input, (char ***)result_list, count, (InterfaceType *)extra_info);
    default:
        return 0;
    }
}

void free_result_list(void **list, int count)
{
    if (!list)
        return;

    for (int i = 0; i < count; i++)
    {
        if (list[i])
        {
            free(list[i]);
        }
    }
    free(list);
}

const char *get_type_name(InputType type)
{
    switch (type)
    {
    case TYPE_MAC:
        return "MAC地址";
    case TYPE_STRING_NUM:
        return "字符串/数字";
    case TYPE_IP_RANGE:
        return "IP范围";
    case TYPE_VLAN:
        return "VLAN";
    case TYPE_INTERFACE:
        return "接口(LAN/SSID)";
    default:
        return "未知类型";
    }
}

const char *get_interface_type_name(InterfaceType type)
{
    switch (type)
    {
    case INTERFACE_TYPE_LAN:
        return "LAN";
    case INTERFACE_TYPE_SSID:
        return "SSID";
    case INTERFACE_TYPE_MIXED:
        return "LAN/SSID混合";
    default:
        return "未知接口";
    }
}

// ==================== 7. 测试函数 ====================
void test_interface_mixed_format()
{
    PP("=== 接口格式测试（允许LAN和SSID混合） ===\n\n");

    const char *test_cases[] = {
        // 纯LAN测试
        "LAN1/100,LAN2/200,LAN3/300,LAN4/400,LAN5/500,LAN6/600,LAN7/700,LAN8/800",
        "lan1/100,LAN2/200,LaN3/300,LAN4/400",

        // 纯SSID测试
        "SSID1/100,SSID2/200,SSID3/300,SSID4/400,SSID5/500,SSID6/600,SSID7/700,SSID8/800,SSID9/900,SSID10/1000,SSID11/1100,SSID12/1200",
        "ssid1/100,SSID2/200,SsId3/300,SSID10/1000",

        // LAN和SSID混合测试（现在应该成功）
        "LAN1/100,SSID1/200",
        "SSID1/100,LAN2/200",
        "LAN1/100,LAN2/200,SSID3/300,SSID4/400,LAN5/500",
        "SSID1/100,LAN2/200,SSID3/300,LAN4/400,SSID5/500",

        // 包含无效项的测试（应该失败）
        "LAN1/100,LAN2/200,LAN9/300",     // 包含无效LAN9
        "SSID1/100,SSID2/200,SSID13/300", // 包含无效SSID13
        "LAN1/100,invalid/200,SSID3/300", // 包含无效格式

        // 单个测试
        "LAN1/100",
        "SSID12/1000",

        // 边界测试
        "LAN1/0",                                       // 数字为0
        "SSID1/65535",                                  // 大数字
        "LAN1/100,LAN2/200,SSID3/300,SSID4/400,LAN5/0", // 混合中包含0
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, test_cases[i]);

        char **list = NULL;
        int count = 0;
        InterfaceType interface_type = INTERFACE_TYPE_UNKNOWN;

        if (validate_specific_type(test_cases[i], TYPE_INTERFACE,
                                   (void ***)&list, &count, (int *)&interface_type))
        {
            const char *type_name = get_interface_type_name(interface_type);
            PP("  结果: ✓ 验证成功，找到 %d 个有效接口 (%s):\n", count, type_name);
            for (int j = 0; j < count; j++)
                PP("    %d. %s\n", j + 1, list[j]);
            free_result_list((void **)list, count);
        }
        else
        {
            PP("  结果: ✗ 验证失败\n");
        }
        PP("\n");
    }
}

void test_interface_statistics()
{
    PP("=== 接口类型统计测试 ===\n\n");

    struct
    {
        const char *input;
        const char *description;
        InterfaceType expected_type;
    } test_cases[] = {
        {"LAN1/100,LAN2/200,LAN3/300", "纯LAN列表", INTERFACE_TYPE_LAN},
        {"SSID1/100,SSID2/200,SSID10/1000", "纯SSID列表", INTERFACE_TYPE_SSID},
        {"LAN1/100,SSID1/200", "LAN+SSID混合", INTERFACE_TYPE_MIXED},
        {"SSID1/100,LAN2/200,SSID3/300", "SSID+LAN+SSID", INTERFACE_TYPE_MIXED},
        {"LAN1/100,LAN2/200,SSID3/300,LAN4/400", "LAN+LAN+SSID+LAN", INTERFACE_TYPE_MIXED},
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, test_cases[i].description);
        PP("  输入: %s\n", test_cases[i].input);

        char **list = NULL;
        int count = 0;
        InterfaceType interface_type = INTERFACE_TYPE_UNKNOWN;

        if (validate_specific_type(test_cases[i].input, TYPE_INTERFACE,
                                   (void ***)&list, &count, (int *)&interface_type))
        {
            const char *actual_name = get_interface_type_name(interface_type);
            const char *expected_name = get_interface_type_name(test_cases[i].expected_type);

            PP("  结果: ✓ 验证成功\n");
            PP("  类型: %s (预期: %s)\n", actual_name, expected_name);
            PP("  数量: %d 个接口\n", count);

            // 统计LAN和SSID数量
            int lan_count = 0, ssid_count = 0;
            for (int j = 0; j < count; j++)
            {
                InterfaceType item_type = detect_interface_type(list[j]);
                if (item_type == INTERFACE_TYPE_LAN)
                    lan_count++;
                else if (item_type == INTERFACE_TYPE_SSID)
                    ssid_count++;
            }
            PP("  明细: LAN=%d, SSID=%d\n", lan_count, ssid_count);

            for (int j = 0; j < count; j++)
            {
                InterfaceType item_type = detect_interface_type(list[j]);
                const char *item_type_name = (item_type == INTERFACE_TYPE_LAN) ? "LAN" : "SSID";
                PP("    %d. %s [%s]\n", j + 1, list[j], item_type_name);
            }

            free_result_list((void **)list, count);
        }
        else
        {
            PP("  结果: ✗ 验证失败\n");
        }
        PP("\n");
    }
}

void test_all_formats_with_mixed_interface()
{
    PP("=== 所有格式综合测试（包含混合接口） ===\n\n");

    struct
    {
        const char *input;
        InputType type;
        const char *description;
    } test_cases[] = {
        {"00:1A:2B:3C:4D:5E/123,11:22:33:44:55:66/456", TYPE_MAC, "MAC地址"},
        {"test123/456,hello/789", TYPE_STRING_NUM, "字符串/数字"},
        {"192.168.1.1-192.168.1.10/24", TYPE_IP_RANGE, "IPv4范围"},
        {"192.168.1.1-192.168.1.10/24,192.168.1.10-192.168.1.100/24", TYPE_IP_RANGE, "IPv4范围"},
        {"2001:db8::1-2001:db8::ffff/64", TYPE_IP_RANGE, "IPv6范围"},
        {"vlan/100", TYPE_VLAN, "VLAN"},
        {"vlan/100,VLAN/200", TYPE_VLAN, "VLAN"},
        {"LAN1/100,LAN2/200,LAN3/300", TYPE_INTERFACE, "纯LAN接口"},
        {"SSID1/100,SSID2/200,SSID10/1000", TYPE_INTERFACE, "纯SSID接口"},
        {"LAN1/100,SSID1/200,LAN2/300,SSID2/400", TYPE_INTERFACE, "混合接口"},
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, test_cases[i].description);
        PP("  输入: %s\n", test_cases[i].input);

        char **list = NULL;
        int count = 0;
        int extra_info = 0;

        if (validate_specific_type(test_cases[i].input, test_cases[i].type,
                                   (void ***)&list, &count, &extra_info))
        {
            PP("  结果: ✓ 验证成功，找到 %d 个有效项目:\n", count);
            for (int j = 0; j < count; j++)
            {
                PP("    %d. %s\n", j + 1, list[j]);
            }

            // 显示额外信息
            if (test_cases[i].type == TYPE_IP_RANGE)
            {
                PP("  类型: %s\n", extra_info == 1 ? "IPv4" : "IPv6");
            }
            else if (test_cases[i].type == TYPE_INTERFACE)
            {
                const char *type_name = get_interface_type_name((InterfaceType)extra_info);
                PP("  接口类型: %s\n", type_name);
            }

            free_result_list((void **)list, count);
        }
        else
        {
            PP("  结果: ✗ 验证失败\n");
        }
        PP("\n");
    }
}

void test_interface_edge_cases_mixed()
{
    PP("=== 混合接口边界情况测试 ===\n\n");

    struct
    {
        const char *input;
        const char *description;
        int should_pass;
    } edge_cases[] = {
        // 正常混合
        {"LAN1/100,SSID1/200", "简单混合", 1},
        {"LAN1/100,LAN2/200,SSID3/300,SSID4/400", "多项目混合", 1},
        {"SSID1/100,LAN2/200,SSID3/300,LAN4/400", "交替混合", 1},

        // 边界值混合
        {"LAN1/1,SSID1/1", "最小数字混合", 1},
        {"LAN8/65535,SSID12/65535", "最大编号混合", 1},
        {"LAN1/0,SSID1/0", "数字为0混合", 1},

        // 大小写混合
        {"lan1/100,SSID1/200", "小写LAN+大写SSID", 1},
        {"LAN1/100,ssid1/200", "大写LAN+小写SSID", 1},
        {"Lan1/100,SsId1/200", "混合大小写", 1},

        // 格式错误
        {"LAN1-100,SSID1/200", "LAN格式错误", 0},
        {"LAN1/100,SSID1:200", "SSID格式错误", 0},
        {"LAN1/100,SSID1/", "SSID缺少数字", 0},
        {"/100,SSID1/200", "LAN缺少名称", 0},

        // 无效编号
        {"LAN9/100,SSID1/200", "无效LAN9", 0},
        {"LAN1/100,SSID13/200", "无效SSID13", 0},
        {"LAN0/100,SSID1/200", "无效LAN0", 0},
        {"LAN1/100,SSID0/200", "无效SSID0", 0},

        // 带空格
        {"  LAN1/100  ,  SSID1/200  ", "带空格混合", 1},
        {"LAN1/100 , SSID1/200 , LAN2/300", "多项目带空格", 1},
    };

    for (int i = 0; i < sizeof(edge_cases) / sizeof(edge_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, edge_cases[i].description);
        PP("  输入: %s\n", edge_cases[i].input);

        char **list = NULL;
        int count = 0;
        InterfaceType interface_type = INTERFACE_TYPE_UNKNOWN;

        int result = validate_specific_type(edge_cases[i].input, TYPE_INTERFACE,
                                            (void ***)&list, &count, (int *)&interface_type);

        if (result == edge_cases[i].should_pass)
        {
            PP("  结果: ✓ 符合预期 (%s)\n", result ? "通过" : "拒绝");
            if (result)
            {
                const char *type_name = get_interface_type_name(interface_type);
                PP("  找到 %d 个有效接口 (%s):\n", count, type_name);

                // 显示详细类型
                for (int j = 0; j < count; j++)
                {
                    InterfaceType item_type = detect_interface_type(list[j]);
                    const char *item_type_name = (item_type == INTERFACE_TYPE_LAN) ? "LAN" : "SSID";
                    PP("    %d. %s [%s]\n", j + 1, list[j], item_type_name);
                }
            }
        }
        else
        {
            PP("  结果: ✗ 不符合预期 (预期: %s, 实际: %s)\n",
               edge_cases[i].should_pass ? "通过" : "拒绝",
               result ? "通过" : "拒绝");
        }

        if (list)
            free_result_list((void **)list, count);
        PP("\n");
    }
}

// ==================== 主函数 ====================
int main()
{
    PP("============================================\n");
    PP("    允许混合的接口格式验证测试程序\n");
    PP("============================================\n\n");

    // 运行混合接口测试
    // test_interface_mixed_format();

    PP("============================================\n");
    PP("    接口类型统计测试\n");
    PP("============================================\n\n");

    // test_interface_statistics();

    PP("============================================\n");
    PP("    所有格式综合测试\n");
    PP("============================================\n\n");

    test_all_formats_with_mixed_interface();

    PP("============================================\n");
    PP("    混合接口边界情况测试\n");
    PP("============================================\n\n");

    // test_interface_edge_cases_mixed();

    PP("============================================\n");
    PP("    测试完成\n");
    PP("============================================\n");

    return 0;
}