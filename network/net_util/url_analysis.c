#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int www_ParseUrl(const char *url, char *proto, char *host, int *port, char *uri)
{
    int n;
    char *p;

    *port = 0;
    strncpy(uri, "", 8);

    /* proto */
    p = (char *) url;
    if ((p = strchr(url, ':')) == NULL)
    {
        return -1;
    }
    n = p - url;
    strncpy(proto, url, n);
    proto[n] = '\0';

    /* skip "://" */
    if (*p++ != ':')
        return -1;
    if (*p++ != '/')
        return -1;
    if (*p++ != '/')
        return -1;

    /* host */
    {
        char *hp = host;

        while (*p && *p != ':' && *p != '/')
        {
            *hp++ = *p++;
        }
        *hp = '\0';
    }
    if (strlen(host) == 0)
        return -1;

    /* end */
    if (*p == '\0')
    {
        *port = 0;
        strncpy(uri, "", 8);
        return 0;
    }

    /* port */
    if (*p == ':')
    {
        char buf[10];
        char *pp = buf;

        p++;
        while (isdigit(*p))
        {
            *pp++ = *p++;
        }
        *pp = '\0';
        if (strlen(buf) == 0)
            return -1;
        *port = atoi(buf);
    }

    /* uri */
    if (*p == '/')
    {
        char *up = uri;
        while ((*up++ = *p++));
    }

    return 0;
}


typedef enum
{
	HI_VPN_ATTACH_IP_TYPE_EMPTY=0,
	HI_VPN_ATTACH_IP_TYPE_MASK,  /*IP MASK */
	HI_VPN_ATTACH_IP_TYPE_DMAIN, /*域名*/
	HI_VPN_ATTACH_IP_TYPE_RANGE,/*IP range*/
	HI_VPN_ATTACH_IP_TYPE_REG,/*正则表达式*/
	HI_VPN_ATTACH_IP_TYPE_MAC, /**/
	HI_VPN_ATTACH_IP_TYPE_SINGLE, /*单独IP*/
}hi_vpn_attach_type_e;


typedef struct {
    int type;
    char str[128];
} vpn_type_to_string;

vpn_type_to_string g_vpn_type_to_string[] = {
    [HI_VPN_ATTACH_IP_TYPE_EMPTY] = {HI_VPN_ATTACH_IP_TYPE_EMPTY, "HI_VPN_ATTACH_IP_TYPE_EMPTY"},
    [HI_VPN_ATTACH_IP_TYPE_MASK] = {HI_VPN_ATTACH_IP_TYPE_MASK, "HI_VPN_ATTACH_IP_TYPE_MASK"},
    [HI_VPN_ATTACH_IP_TYPE_DMAIN] = {HI_VPN_ATTACH_IP_TYPE_DMAIN, "HI_VPN_ATTACH_IP_TYPE_DMAIN"},
    [HI_VPN_ATTACH_IP_TYPE_RANGE] = {HI_VPN_ATTACH_IP_TYPE_RANGE, "HI_VPN_ATTACH_IP_TYPE_RANGE"},
    [HI_VPN_ATTACH_IP_TYPE_REG] = {HI_VPN_ATTACH_IP_TYPE_REG, "HI_VPN_ATTACH_IP_TYPE_REG"},
    [HI_VPN_ATTACH_IP_TYPE_MAC] = {HI_VPN_ATTACH_IP_TYPE_MAC, "HI_VPN_ATTACH_IP_TYPE_MAC"},
    [HI_VPN_ATTACH_IP_TYPE_SINGLE] = {HI_VPN_ATTACH_IP_TYPE_SINGLE, "HI_VPN_ATTACH_IP_TYPE_SINGLE"}};

unsigned int check_ip_type(char  * pst_str)
{
    unsigned int  itype = HI_VPN_ATTACH_IP_TYPE_EMPTY ;
    char  *pc_tmp = pst_str;

    if ( NULL == pst_str )
    {
        return HI_VPN_ATTACH_IP_TYPE_EMPTY;
    }
    if( ('\0' == *pc_tmp)||(strlen((const char  *)pc_tmp)<7) )
    {
        return HI_VPN_ATTACH_IP_TYPE_EMPTY;
    }

    while ( '\0' != *pc_tmp)
    {
        if ( isalpha(*pc_tmp) )
        {
            itype = HI_VPN_ATTACH_IP_TYPE_DMAIN;
            break;
        }
        if (isdigit(*pc_tmp))
        {
            itype = HI_VPN_ATTACH_IP_TYPE_SINGLE;
        }
        pc_tmp++;
    }
    if(itype  == HI_VPN_ATTACH_IP_TYPE_SINGLE)
    {
        if(strstr((const char  *)pst_str,"/")!=NULL)
            itype = HI_VPN_ATTACH_IP_TYPE_MASK ;
        else if(strstr((const char  *)pst_str,"-")!=NULL)
            itype = HI_VPN_ATTACH_IP_TYPE_RANGE;
    }
    return itype ;
}



int main()
{
    char *url = "http://61.163.160.69:8084/webff/versions/3FE56752AOCK07.3FE56752AOCK07";
    char proto[128] = {0};
    char host[128] = {0};
    int port = 0;
    char uri[128] = {0};
    char buf[32] = {0};
    www_ParseUrl(url, proto, host, &port, uri);
    printf("proto = %s\nhost= %s\nport=%d\nuri=%s\n", proto, host, port, uri);
    //itoa(port, buf, 10);
    // printf("%s\n", strcat(host, ":"));
    // printf("%s\n", strcat(host, buf));

    printf("%s\n", g_vpn_type_to_string[check_ip_type("192.168.1.100-192.168.1.200")].str);
    printf("%s\n", g_vpn_type_to_string[check_ip_type("www.baidu.com,www.jd.com")].str);
    printf("%s\n", g_vpn_type_to_string[2].str);

    return 0;
}


