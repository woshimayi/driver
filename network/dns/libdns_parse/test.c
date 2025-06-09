#include <stdio.h>
#include "dns_parse.h"

int main()
{
    // IPv4查询示例
    // struct dns_result result = dns_query("time.nist.gov", "202.96.209.5", 4, "eth0", 5);
    struct dns_result result = dns_query("time.nist.gov", NULL, 4, NULL, 5);
    if (result.error_code == 0)
    {
        printf("IPv4 Address: %s\n", result.ipv4_address);
        printf("CNAME: %s\n", result.cname);
    }
    else
    {
        printf("Error: %s (code: %d)\n", result.error_message, result.error_code);
    }

    // IPv6查询示例
    // result = dns_query("www.baidu.com", "2001:4860:4860::8888", 6, "eth0", 5);
    result = dns_query("www.baidu.com", NULL, 6, NULL, 5);
    if (result.error_code == 0)
    {
        printf("IPv6 Address: %s\n", result.ipv6_address);
        printf("CNAME: %s\n", result.cname);
    }
    else
    {
        printf("Error: ipv6 %s (code: %d)\n", result.error_message, result.error_code);
    }

    return 0;
}
