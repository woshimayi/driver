/*
 * @*************************************:
 * @FilePath     : /network/dns/dns_paras.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-12-25 10:53:46
 * @LastEditors  : dof
 * @LastEditTime : 2025-12-25 13:08:56
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

/**
 * @brief 域名解析函数（支持 IPv4/IPv6）
 */
int lookup(const char *doMain, char *ipBuf, size_t bufLen)
{
    if (NULL == doMain || NULL == ipBuf)
    {
        return -1;
    }

    struct addrinfo hints, *res, *p;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // 核心：允许返回 IPv4 或 IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(doMain, NULL, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error for %s: %s\n", doMain, gai_strerror(status));
        return -1;
    }

    int found = -1;
    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        if (p->ai_family == AF_INET)
        { // 处理 IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        }
        else if (p->ai_family == AF_INET6)
        { // 处理 IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
        else
        {
            continue;
        }

        if (inet_ntop(p->ai_family, addr, ipBuf, bufLen) != NULL)
        {
            found = 0;
            break;
        }
    }

    freeaddrinfo(res);
    return found;
}

int main()
{
    // 准备缓冲区，INET6_ADDRSTRLEN 足够容纳 IPv6 字符串
    char ip_result[INET6_ADDRSTRLEN];

    // 测试案例列表
    const char *test_domains[] = {
        "ipv4.google.com",    // 强制 IPv4
        "ipv6.google.com",    // 强制 IPv6
        "www.baidu.com",      // 国内常用域名
        "localhost",          // 本地回环
        "invalid.domain.test" // 错误域名
    };

    int num_tests = sizeof(test_domains) / sizeof(test_domains[0]);

    printf("--- 开始 DNS 解析测试 ---\n");
    for (int i = 0; i < num_tests; i++)
    {
        memset(ip_result, 0, sizeof(ip_result));

        printf("正在解析: %-20s -> ", test_domains[i]);

        if (lookup(test_domains[i], ip_result, sizeof(ip_result)) == 0)
        {
            printf("[成功] IP: %s\n", ip_result);
        }
        else
        {
            printf("[失败]\n");
        }
    }
    printf("--- 测试结束 ---\n");

    return 0;
}

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <arpa/inet.h>

// /**
//  * @brief 域名解析函数（支持 IPv4/IPv6）
//  */
// int lookup(const char *doMain, char *ipBuf, size_t bufLen)
// {
//     if (NULL == doMain || NULL == ipBuf)
//         return -1;

//     struct addrinfo hints, *res, *p;
//     int status;

//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_UNSPEC; // 兼容 IPv4 和 IPv6
//     hints.ai_socktype = SOCK_STREAM;

//     if ((status = getaddrinfo(doMain, NULL, &hints, &res)) != 0)
//     {
//         return -1;
//     }

//     int found = -1;
//     for (p = res; p != NULL; p = p->ai_next)
//     {
//         void *addr;
//         if (p->ai_family == AF_INET)
//         {
//             addr = &(((struct sockaddr_in *)p->ai_addr)->sin_addr);
//         }
//         else if (p->ai_family == AF_INET6)
//         {
//             addr = &(((struct sockaddr_in6 *)p->ai_addr)->sin6_addr);
//         }
//         else
//             continue;

//         if (inet_ntop(p->ai_family, addr, ipBuf, bufLen) != NULL)
//         {
//             found = 0;
//             break;
//         }
//     }
//     freeaddrinfo(res);
//     return found;
// }

// /**
//  * @brief 替换 URL 中的域名为 IP
//  */
// void replace_domain_with_ip(const char *original_url, char *new_url, size_t max_len)
// {
//     const char *domain_start = strstr(original_url, "://");
//     if (!domain_start)
//     {
//         strncpy(new_url, original_url, max_len);
//         return;
//     }
//     domain_start += 3; // 跳过 "://"

//     const char *domain_end = strchr(domain_start, '/');
//     if (!domain_end)
//         domain_end = domain_start + strlen(domain_start);

//     // 提取域名
//     size_t domain_len = domain_end - domain_start;
//     char domain[256];
//     strncpy(domain, domain_start, domain_len);
//     domain[domain_len] = '\0';

//     // 解析域名为 IP
//     char ip[INET6_ADDRSTRLEN];
//     if (lookup(domain, ip, sizeof(ip)) == 0)
//     {
//         // 如果是 IPv6，按照标准需要在 IP 两端加 []
//         char formatted_ip[INET6_ADDRSTRLEN + 2];
//         if (strchr(ip, ':'))
//         {
//             snprintf(formatted_ip, sizeof(formatted_ip), "[%s]", ip);
//         }
//         else
//         {
//             strncpy(formatted_ip, ip, sizeof(formatted_ip));
//         }

//         // 构造新的 URL
//         snprintf(new_url, max_len, "%.*s%s%s",
//                  (int)(domain_start - original_url), original_url,
//                  formatted_ip, domain_end);
//     }
//     else
//     {
//         printf("无法解析域名: %s\n", domain);
//         strncpy(new_url, original_url, max_len);
//     }
// }

// int main()
// {
//     const char *url = "http://dldir1.qq.com/qqfile/qq/PCQQ9.7.5/QQ9.7.5.28965.exe";
//     char new_url[512];

//     printf("原 URL: %s\n", url);
//     replace_domain_with_ip(url, new_url, sizeof(new_url));
//     printf("新 URL: %s\n", new_url);

//     return 0;
// }