#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <arpa/inet.h>
#include <ctype.h>

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

// 去除字符串首尾空格
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

// ==================== 1. MAC地址验证（单独处理） ====================
int validate_mac_format_single(const char *str)
{
    regex_t regex;
    int ret;

    // MAC地址格式：xx:xx:xx:xx:xx:xx/数字
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

    // 初始化指针数组
    for (int i = 0; i < max_items; i++)
    {
        (*mac_list)[i] = NULL;
    }

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
                // 内存分配失败，清理并返回
                for (int i = 0; i < item_count; i++)
                {
                    free((*mac_list)[i]);
                }
                free(*mac_list);
                *mac_list = NULL;
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            // 如果有一个无效的MAC地址，整个列表无效
            for (int i = 0; i < item_count; i++)
            {
                free((*mac_list)[i]);
            }
            free(*mac_list);
            *mac_list = NULL;
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
        *mac_list = NULL;
        return 0;
    }

    return 1;
}

// ==================== 2. 字符串/数字验证（单独处理） ====================
int validate_string_number_single(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // 字符串/数字格式：字符串/数字，字符串长度≤128
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
    {
        (*str_list)[i] = NULL;
    }

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
                {
                    free((*str_list)[i]);
                }
                free(*str_list);
                *str_list = NULL;
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            // 如果有一个无效的，整个列表无效
            for (int i = 0; i < item_count; i++)
            {
                free((*str_list)[i]);
            }
            free(*str_list);
            *str_list = NULL;
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
        *str_list = NULL;
        return 0;
    }

    return 1;
}

// ==================== 3. IP范围验证（单独处理） ====================
int validate_ip_range_single(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[4];

    // IP范围格式：ip-ip/数字
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

        // 验证第一个IP
        if (inet_pton(AF_INET, ip1, &addr4) == 1)
        {
            valid1 = 1; // IPv4
        }
        else if (inet_pton(AF_INET6, ip1, &addr6) == 1)
        {
            valid1 = 2; // IPv6
        }

        // 验证第二个IP
        if (inet_pton(AF_INET, ip2, &addr4) == 1)
        {
            valid2 = 1;
        }
        else if (inet_pton(AF_INET6, ip2, &addr6) == 1)
        {
            valid2 = 2;
        }

        regfree(&regex);

        // 两个IP必须都有效且类型相同
        if (valid1 > 0 && valid2 > 0 && valid1 == valid2)
        {
            return valid1; // 返回类型：1=IPv4, 2=IPv6
        }
    }

    regfree(&regex);
    return 0; // 无效
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
    {
        (*ip_list)[i] = NULL;
    }

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
                // IP类型不一致，整个列表无效
                for (int i = 0; i < item_count; i++)
                {
                    free((*ip_list)[i]);
                }
                free(*ip_list);
                *ip_list = NULL;
                free(str);
                return 0;
            }

            (*ip_list)[item_count] = strdup(item);
            if (!(*ip_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                {
                    free((*ip_list)[i]);
                }
                free(*ip_list);
                *ip_list = NULL;
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            // 如果有一个无效的，整个列表无效
            for (int i = 0; i < item_count; i++)
            {
                free((*ip_list)[i]);
            }
                free(*ip_list);
                *ip_list = NULL;
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
        *ip_list = NULL;
        return 0;
    }

    return 1;
}

// ==================== 4. VLAN验证（单独处理） ====================
int validate_vlan_single(const char *str)
{
    regex_t regex;
    int ret;
    regmatch_t matches[3];

    // VLAN格式：vlan/数字，VLAN ID范围：1-4094
    char *pattern = "^[[:space:]]*([Vv][Ll][Aa][Nn])[[:space:]]*/[[:space:]]*([0-9]+)[[:space:]]*$";

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (ret != 0)
    {
        return 0;
    }

    ret = regexec(&regex, str, 3, matches, 0);
    if (ret == 0)
    {
        if (matches[2].rm_so == -1)
        {
            regfree(&regex);
            return 0;
        }

        char num_str[32];
        int num_len = matches[2].rm_eo - matches[2].rm_so;

        if (num_len > 0 && num_len < sizeof(num_str))
        {
            strncpy(num_str, str + matches[2].rm_so, num_len);
            num_str[num_len] = '\0';

            char *endptr;
            long vlan_id = strtol(num_str, &endptr, 10);

            regfree(&regex);

            if (endptr != num_str && *endptr == '\0')
            {
                return (vlan_id >= 1 && vlan_id <= 4094) ? 1 : 0;
            }
        }
    }

    regfree(&regex);
    return 0;
}

int parse_vlan_list(const char *input, char ***vlan_list, int *count)
{
    if (!input || !vlan_list || !count)
        return 0;

    char *str = strdup(input);
    if (!str)
        return 0;

    char *token;
    char *saveptr = NULL;
    int item_count = 0;
    int max_items = 100;

    *vlan_list = malloc(max_items * sizeof(char *));
    if (!*vlan_list)
    {
        free(str);
        return 0;
    }

    for (int i = 0; i < max_items; i++)
    {
        (*vlan_list)[i] = NULL;
    }

    token = strtok_r(str, ",", &saveptr);
    while (token != NULL && item_count < max_items)
    {
        char item[256];
        strncpy(item, token, sizeof(item) - 1);
        item[sizeof(item) - 1] = '\0';
        trim_string(item);

        if (validate_vlan_single(item))
        {
            (*vlan_list)[item_count] = strdup(item);
            if (!(*vlan_list)[item_count])
            {
                for (int i = 0; i < item_count; i++)
                {
                    free((*vlan_list)[i]);
                }
                free(*vlan_list);
                *vlan_list = NULL;
                free(str);
                return 0;
            }
            item_count++;
        }
        else
        {
            // 如果有一个无效的，整个列表无效
            for (int i = 0; i < item_count; i++)
            {
                free((*vlan_list)[i]);
            }
                free(*vlan_list);
                *vlan_list = NULL;
            free(str);
            return 0;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(str);
    *count = item_count;

    if (item_count == 0)
    {
        free(*vlan_list);
        *vlan_list = NULL;
        return 0;
    }

    return 1;
}

// ==================== 5. 主验证函数（指定类型） ====================
typedef enum
{
    TYPE_MAC,
    TYPE_STRING_NUM,
    TYPE_IP_RANGE,
    TYPE_VLAN
} InputType;

int validate_specific_type(const char *input, InputType type, void ***result_list, int *count, int *ip_type)
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
        return parse_ip_range_list(input, (char ***)result_list, count, ip_type);

    case TYPE_VLAN:
        return parse_vlan_list(input, (char ***)result_list, count);

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

// ==================== 6. 测试函数 ====================
void test_mac_format()
{
    printf("=== MAC地址格式测试 ===\n\n");

    const char *test_cases[] = {
        "00:1A:2B:3C:4D:5E/123,11:22:33:44:55:66/456,AA:BB:CC:DD:EE:FF/789",
        "00:1A:2B:3C:4D:5E/123,invalid/456,AA:BB:CC:DD:EE:FF/789", // 包含无效
        "",                                                        // 空
        "00:1A:2B:3C:4D:5E/123",                                   // 单个
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        printf("测试 %d: %s\n", i + 1, test_cases[i]);

        char **mac_list = NULL;
        int count = 0;

        if (validate_specific_type(test_cases[i], TYPE_MAC, (void ***)&mac_list, &count, NULL))
        {
            printf("  结果: ✓ 验证成功，找到 %d 个有效MAC地址:\n", count);
            for (int j = 0; j < count; j++)
            {
                printf("    %d. %s\n", j + 1, mac_list[j]);
            }
            free_result_list((void **)mac_list, count);
        }
        else
        {
            printf("  结果: ✗ 验证失败\n");
        }
        printf("\n");
    }
}

void test_string_number_format()
{
    printf("=== 字符串/数字格式测试 ===\n\n");

    const char *test_cases[] = {
        "test1/100,test2/200,test3/300",
        "test123/456,invalid-format/789,hello/999", // 包含无效
        "a/1,b/2,c/3",
        "very_long_string_name_1234567890_abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ/123", // 长字符串
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        printf("测试 %d: %s\n", i + 1, test_cases[i]);

        char **str_list = NULL;
        int count = 0;

        if (validate_specific_type(test_cases[i], TYPE_STRING_NUM, (void ***)&str_list, &count, NULL))
        {
            printf("  结果: ✓ 验证成功，找到 %d 个有效字符串/数字:\n", count);
            for (int j = 0; j < count; j++)
            {
                printf("    %d. %s\n", j + 1, str_list[j]);
            }
            free_result_list((void **)str_list, count);
        }
        else
        {
            printf("  结果: ✗ 验证失败\n");
        }
        printf("\n");
    }
}

void test_ip_range_format()
{
    printf("=== IP范围格式测试 ===\n\n");

    const char *test_cases[] = {
        "192.168.1.1-192.168.1.10/24,10.0.0.1-10.0.0.255/16",
        "2001:db8::1-2001:db8::ffff/64,fe80::1-fe80::ffff/10",
        "192.168.1.1-192.168.1.10/24,2001:db8::1-2001:db8::ffff/64", // 混合类型
        "invalid-ip-range/24,192.168.1.1-192.168.1.10/24",           // 包含无效
        "192.168.1.1-192.168.1.10/24",                               // 单个
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        printf("测试 %d: %s\n", i + 1, test_cases[i]);

        char **ip_list = NULL;
        int count = 0;
        int ip_type = 0;

        if (validate_specific_type(test_cases[i], TYPE_IP_RANGE, (void ***)&ip_list, &count, &ip_type))
        {
            const char *type_str = (ip_type == 1) ? "IPv4" : "IPv6";
            printf("  结果: ✓ 验证成功，找到 %d 个有效%s范围:\n", count, type_str);
            for (int j = 0; j < count; j++)
            {
                printf("    %d. %s\n", j + 1, ip_list[j]);
            }
            free_result_list((void **)ip_list, count);
        }
        else
        {
            printf("  结果: ✗ 验证失败\n");
        }
        printf("\n");
    }
}

void test_vlan_format()
{
    printf("=== VLAN格式测试 ===\n\n");

    const char *test_cases[] = {
        "vlan/100,vlan/200,VLAN/300,Vlan/400",
        "vlan/1,vlan/4094",
        "vlan/100,vlan/0,vlan/200", // 包含无效
        "vlan/5000,vlan/100",       // 包含超出范围
        "vlan/100",                 // 单个
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        printf("测试 %d: %s\n", i + 1, test_cases[i]);

        char **vlan_list = NULL;
        int count = 0;

        if (validate_specific_type(test_cases[i], TYPE_VLAN, (void ***)&vlan_list, &count, NULL))
        {
            printf("  结果: ✓ 验证成功，找到 %d 个有效VLAN:\n", count);
            for (int j = 0; j < count; j++)
            {
                printf("    %d. %s\n", j + 1, vlan_list[j]);
            }
            free_result_list((void **)vlan_list, count);
        }
        else
        {
            printf("  结果: ✗ 验证失败\n");
        }
        printf("\n");
    }
}

void test_mixed_input_rejection()
{
    printf("=== 混合类型输入拒绝测试 ===\n\n");

    // 这些输入应该被拒绝，因为它们混合了不同类型
    const char *mixed_tests[] = {
        "00:1A:2B:3C:4D:5E/123,vlan/100",             // MAC + VLAN
        "test123/456,192.168.1.1-192.168.1.10/24",    // 字符串 + IP
        "vlan/100,00:1A:2B:3C:4D:5E/123,test123/456", // VLAN + MAC + 字符串
    };

    InputType types[] = {TYPE_MAC, TYPE_STRING_NUM, TYPE_VLAN};
    const char *type_names[] = {"MAC地址", "字符串/数字", "VLAN"};

    for (int type_idx = 0; type_idx < 3; type_idx++)
    {
        printf("以 %s 类型验证混合输入:\n", type_names[type_idx]);

        for (int i = 0; i < sizeof(mixed_tests) / sizeof(mixed_tests[0]); i++)
        {
            printf("  输入: %s\n", mixed_tests[i]);

            void **list = NULL;
            int count = 0;
            int ip_type = 0;

            int result = validate_specific_type(mixed_tests[i], types[type_idx], &list, &count, &ip_type);

            if (result)
            {
                printf("  结果: ✗ 错误地接受了混合类型输入\n");
                free_result_list(list, count);
            }
            else
            {
                printf("  结果: ✓ 正确拒绝了混合类型输入\n");
            }
            printf("\n");
        }
    }
}

void test_edge_cases()
{
    printf("=== 边界情况测试 ===\n\n");

    struct
    {
        const char *input;
        InputType type;
        const char *description;
        int should_pass;
    } edge_cases[] = {
        {"", TYPE_MAC, "空字符串", 0},
        {"   ", TYPE_VLAN, "只有空格", 0},
        {"vlan/100,", TYPE_VLAN, "末尾多余逗号", 0},
        {",vlan/100", TYPE_VLAN, "开头多余逗号", 0},
        {"vlan/100,,vlan/200", TYPE_VLAN, "连续逗号", 0},
        {"  vlan/100  ,  vlan/200  ", TYPE_VLAN, "带空格", 1},
        {"vlan/1", TYPE_VLAN, "最小VLAN", 1},
        {"vlan/4094", TYPE_VLAN, "最大VLAN", 1},
        {"vlan/0", TYPE_VLAN, "VLAN太小", 0},
        {"vlan/4095", TYPE_VLAN, "VLAN太大", 0},
        {"00:00:00:00:00:00/0", TYPE_MAC, "全零MAC", 1},
        {"FF:FF:FF:FF:FF:FF/65535", TYPE_MAC, "全FF MAC", 1},
        {"a/1", TYPE_STRING_NUM, "最小字符串", 1},
        {"192.168.1.1-192.168.1.10/0", TYPE_IP_RANGE, "掩码为0", 1},
    };

    for (int i = 0; i < sizeof(edge_cases) / sizeof(edge_cases[0]); i++)
    {
        printf("测试 %d: %s\n", i + 1, edge_cases[i].description);
        printf("  输入: %s\n", edge_cases[i].input);
        printf("  类型: %d\n", edge_cases[i].type);

        void **list = NULL;
        int count = 0;
        int ip_type = 0;

        int result = validate_specific_type(edge_cases[i].input, edge_cases[i].type, &list, &count, &ip_type);

        if (result == edge_cases[i].should_pass)
        {
            printf("  结果: ✓ 符合预期 (%s)\n", result ? "通过" : "拒绝");
            if (result)
            {
                printf("  找到 %d 个有效项目\n", count);
                for (int j = 0; j < count; j++)
                {
                    printf("    %d. %s\n", j + 1, (char *)list[j]);
                }
            }
        }
        else
        {
            printf("  结果: ✗ 不符合预期 (预期: %s, 实际: %s)\n",
                   edge_cases[i].should_pass ? "通过" : "拒绝",
                   result ? "通过" : "拒绝");
        }

        if (list)
            free_result_list(list, count);
        printf("\n");
    }
}

// ==================== 主函数 ====================
int main()
{
    printf("============================================\n");
    printf("    分离式格式验证测试程序\n");
    printf("============================================\n\n");

    // 运行各种测试
    test_mac_format();
    test_string_number_format();
    test_ip_range_format();
    test_vlan_format();

    printf("============================================\n");
    printf("    混合类型拒绝测试\n");
    printf("============================================\n\n");

    test_mixed_input_rejection();

    printf("============================================\n");
    printf("    边界情况测试\n");
    printf("============================================\n\n");

    test_edge_cases();

    printf("============================================\n");
    printf("    测试完成\n");
    printf("============================================\n");

    return 0;
}