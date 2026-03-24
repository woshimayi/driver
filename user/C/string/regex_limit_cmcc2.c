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

            
            if (endptr1 != num1_str && *endptr1 == '\0' &&
                endptr2 != num2_str && *endptr2 == '\0')
                {
                    // 检查数字范围
                    if (num1 >= min1 && num1 <= max1 &&
                        num2 >= min2 && num2 <= max2)
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
                *num_list = NULL;
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
            *num_list = NULL;
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
        *num_list = NULL;
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
typedef enum
{
    INTERFACE_TYPE_UNKNOWN = 0,
    INTERFACE_TYPE_LAN,
    INTERFACE_TYPE_SSID,
    INTERFACE_TYPE_MIXED
} InterfaceType;

int validate_interface_item(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

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

            char upper_name[32];
            for (int i = 0; i < name_len; i++)
            {
                upper_name[i] = toupper(interface_name[i]);
            }
            upper_name[name_len] = '\0';

            if (strncmp(upper_name, "LAN", 3) == 0)
            {
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

    char *start = interface_name;
    while (*start && isspace((unsigned char)*start))
        start++;

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';

    char upper_name[32];
    for (int i = 0; start[i]; i++)
    {
        upper_name[i] = toupper(start[i]);
    }
    upper_name[strlen(start)] = '\0';

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
    TYPE_NUM_NUM, // 修改：原来的TYPE_VLAN改为TYPE_NUM_NUM
    TYPE_INTERFACE
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
    case TYPE_NUM_NUM:
        return parse_num_num_list(input, (char ***)result_list, count);
    case TYPE_INTERFACE:
        return parse_interface_list(input, (char ***)result_list, count, (InterfaceType *)extra_info);
    default:
        return 0;
    }
}

// 带范围的验证函数
int validate_specific_type_with_range(const char *input, InputType type,
                                      void ***result_list, int *count, int *extra_info,
                                      long min1, long max1, long min2, long max2)
{
    if (!input || !result_list || !count)
        return 0;

    if (type == TYPE_NUM_NUM)
    {
        return parse_num_num_list_with_range(input, (char ***)result_list, count,
                                             min1, max1, min2, max2);
    }
    else
    {
        return validate_specific_type(input, type, result_list, count, extra_info);
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
    case TYPE_NUM_NUM:
        return "数字/数字";
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
void test_num_num_format()
{
    PP("=== 数字/数字格式测试 ===\n\n");

    const char *test_cases[] = {
        // 基本测试
        "100/200",
        "0/0",
        "1/1",
        "9999/8888",

        // 多个项目
        "100/200,300/400,500/600",
        "1/2,3/4,5/6",

        // 带空格
        " 100/200 ",
        " 100/200 , 300/400 , 500/600 ",

        // 错误格式
        "100/",        // 缺少第二个数字
        "/200",        // 缺少第一个数字
        "100-200",     // 使用短横线
        "100:200",     // 使用冒号
        "abc/def",     // 非数字
        "100/200/300", // 多余部分
        "100.5/200",   // 浮点数
        "-100/200",    // 负数
        "100/-200",    // 负数

        // 边界测试
        "0/0",
        "999999999/999999999", // 大数字
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, test_cases[i]);

        char **list = NULL;
        int count = 0;

        if (validate_specific_type(test_cases[i], TYPE_NUM_NUM,
                                   (void ***)&list, &count, NULL))
        {
            PP("  结果: ✓ 验证成功，找到 %d 个有效数字/数字:\n", count);
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

void test_num_num_with_range()
{
    PP("=== 带范围的数字/数字测试 ===\n\n");

    // 模拟VLAN范围：第一个数字1-4094，第二个数字任意
    PP("模拟VLAN范围测试（第一个数字1-4094）:\n");

    const char *vlan_range_tests[] = {
        "1/100",     // 有效
        "100/200",   // 有效
        "4094/1000", // 有效（边界）
        "0/100",     // 无效（太小）
        "4095/100",  // 无效（太大）
        "5000/100",  // 无效（太大）
    };

    for (int i = 0; i < sizeof(vlan_range_tests) / sizeof(vlan_range_tests[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, vlan_range_tests[i]);

        char **list = NULL;
        int count = 0;
        int extra_info = 0;

        // 第一个数字范围1-4094，第二个数字范围0-999999
        if (validate_specific_type_with_range(vlan_range_tests[i], TYPE_NUM_NUM,
                                              (void ***)&list, &count, &extra_info,
                                              1, 4094, 0, 999999))
        {
            PP("  结果: ✓ 验证成功\n");
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

    // 测试端口范围：第一个数字1-65535，第二个数字1-65535
    PP("\n模拟端口范围测试（两个数字都是1-65535）:\n");

    const char *port_range_tests[] = {
        "80/443",    // 有效
        "1/65535",   // 有效（边界）
        "8080/8443", // 有效
        "0/80",      // 无效（太小）
        "80/0",      // 无效（太小）
        "65536/80",  // 无效（太大）
        "80/65536",  // 无效（太大）
    };

    for (int i = 0; i < sizeof(port_range_tests) / sizeof(port_range_tests[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, port_range_tests[i]);

        char **list = NULL;
        int count = 0;
        int extra_info = 0;

        if (validate_specific_type_with_range(port_range_tests[i], TYPE_NUM_NUM,
                                              (void ***)&list, &count, &extra_info,
                                              1, 65535, 1, 65535))
        {
            PP("  结果: ✓ 验证成功\n");
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

void test_all_formats()
{
    PP("=== 所有格式综合测试 ===\n\n");

    struct
    {
        const char *input;
        InputType type;
        const char *description;
        int use_range;
        long min1, max1, min2, max2;
    } test_cases[] = {
        // {"00:1A:2B:3C:4D:5E/123,11:22:33:44:55:66/456", TYPE_MAC, "MAC地址", 0, 0, 0, 0, 0},
        // {"test123/456,hello/789", TYPE_STRING_NUM, "字符串/数字", 0, 0, 0, 0, 0},
        // {"192.168.1.1-192.168.1.10/24", TYPE_IP_RANGE, "IPv4范围", 0, 0, 0, 0, 0},
        {"100/200,300/400", TYPE_NUM_NUM, "数字/数字（无范围限制）", 0, 0, 0, 0, 0},
        {"1/100,4094/200", TYPE_NUM_NUM, "数字/数字（VLAN范围）", 1, 1, 4094, 0, 999999},
        {"80/443,8080/8443", TYPE_NUM_NUM, "数字/数字（端口范围）", 1, 1, 65535, 1, 65535},
        {"dddd80/443,8080/8443", TYPE_NUM_NUM, "数字/数字（端口范围）", 1, 1, 65535, 1, 65535},
        {"9563/443", TYPE_NUM_NUM, "数字/数字（端口范围）", 1, 1, 4095, 1, 65535},
        // {"LAN1/100,SSID1/200", TYPE_INTERFACE, "混合接口", 0, 0, 0, 0, 0},
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, test_cases[i].description);
        PP("  输入: %s\n", test_cases[i].input);

        char **list = NULL;
        int count = 0;
        int extra_info = 0;

        int result;
        if (test_cases[i].use_range)
        {
            result = validate_specific_type_with_range(test_cases[i].input, test_cases[i].type,
                                                       (void ***)&list, &count, &extra_info,
                                                       test_cases[i].min1, test_cases[i].max1,
                                                       test_cases[i].min2, test_cases[i].max2);
        }
        else
        {
            result = validate_specific_type(test_cases[i].input, test_cases[i].type,
                                            (void ***)&list, &count, &extra_info);
        }

        if (result)
        {
            PP("  结果: ✓ 验证成功，找到 %d 个有效项目:\n", count);
            for (int j = 0; j < count; j++)
                PP("    %d. %s\n", j + 1, list[j]);

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

void test_num_num_edge_cases()
{
    PP("=== 数字/数字边界情况测试 ===\n\n");

    struct
    {
        const char *input;
        const char *description;
        int should_pass;
    } edge_cases[] = {
        // 基本边界
        {"0/0", "两个零", 1},
        {"1/1", "最小正整数", 1},
        {"999999999/999999999", "大数字", 1},

        // 格式边界
        {" 0/0 ", "带空格", 1},
        {"  100/200  ,  300/400  ", "多个带空格", 1},
        {"0/0,1/1,2/2,3/3,4/4", "多个项目", 1},

        // 错误边界
        {"", "空字符串", 0},
        {"   ", "只有空格", 0},
        {",,", "只有逗号", 0},
        {"100/200,", "末尾逗号", 0},
        {",100/200", "开头逗号", 0},
        {"100/200,,300/400", "连续逗号", 0},

        // 数字边界
        {"-1/100", "负数第一个数字", 0},
        {"100/-1", "负数第二个数字", 0},
        {"-1/-1", "两个负数", 0},
        {"100.5/200", "浮点数", 0},
        {"100/200.5", "浮点数", 0},
        {"0x100/200", "十六进制", 0},
        {"100/0x200", "十六进制", 0},

        // 特殊字符
        {"100/200/300", "多余斜杠", 0},
        {"100-200", "使用短横线", 0},
        {"100:200", "使用冒号", 0},
        {"100 200", "使用空格", 0},
        {"100\\200", "使用反斜杠", 0},

        // 超长数字
        {"12345678901234567890/200", "超长数字", 1}, // 可能溢出，但格式正确
    };

    for (int i = 0; i < sizeof(edge_cases) / sizeof(edge_cases[0]); i++)
    {
        PP("测试 %d: %s\n", i + 1, edge_cases[i].description);
        PP("  输入: %s\n", edge_cases[i].input);

        char **list = NULL;
        int count = 0;

        int result = validate_specific_type(edge_cases[i].input, TYPE_NUM_NUM,
                                            (void ***)&list, &count, NULL);

        if (result == edge_cases[i].should_pass)
        {
            PP("  结果: ✓ 符合预期 (%s)\n", result ? "通过" : "拒绝");
            if (result)
            {
                PP("  找到 %d 个有效项目:\n", count);
                for (int j = 0; j < count; j++)
                    PP("    %d. %s\n", j + 1, list[j]);
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
    PP("    数字/数字格式验证测试程序\n");
    PP("============================================\n\n");

    // 运行数字/数字测试
    // test_num_num_format();

    PP("============================================\n");
    PP("    带范围的数字/数字测试\n");
    PP("============================================\n\n");

    // test_num_num_with_range();

    PP("============================================\n");
    PP("    所有格式综合测试\n");
    PP("============================================\n\n");

    test_all_formats();

    PP("============================================\n");
    PP("    数字/数字边界情况测试\n");
    PP("============================================\n\n");

    // test_num_num_edge_cases();

    PP("============================================\n");
    PP("    测试完成\n");
    PP("============================================\n");

    return 0;
}