#ifndef  __DNS_PARSE_H_
#define  __DNS_PARSE_H_


#ifndef DNS_QUERY_H
#define DNS_QUERY_H

#include <stdbool.h>

// DNS查询结果结构体
struct dns_result {
    char ipv4_address[16];    // 点分十进制IPv4地址
    char ipv6_address[46];    // 冒号分隔IPv6地址
    char cname[256];          // 规范名称
    int error_code;           // 错误代码 (0表示成功)
    char error_message[256];  // 错误描述
};

/**
 * @brief 执行DNS查询
 * 
 * @param domain 要查询的域名
 * @param dns_server DNS服务器地址(IPv4或IPv6)
 * @param query_type 查询类型 (4=A记录, 6=AAAA记录)
 * @param interface 网络接口名称(可选，NULL表示使用默认)
 * @param timeout_sec 超时时间(秒)
 * @return struct dns_result 包含查询结果的结构体
 */
struct dns_result dns_query(const char *domain, 
                           const char *dns_server, 
                           int query_type, 
                           const char *interface, 
                           int timeout_sec);

#endif // DNS_QUERY_H


#endif