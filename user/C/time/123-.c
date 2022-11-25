/*
 * @*************************************: 
 * @FilePath: /user/C/time/123-.c
 * @version: 
 * @Author: dof
 * @Date: 2022-11-10 10:48:51
 * @LastEditors: dof
 * @LastEditTime: 2022-11-10 10:57:13
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;


struct ebt_ip_info_t {
        __be32 saddr;
        __be32 daddr;
        __be32 smsk;
        __be32 dmsk;
        __u8  tos;
        __u8  protocol;
        __u8  bitmask;
        __u8  invflags;
        union {
                __u16 sport[2];
                __u8 icmp_type[2];
                __u8 igmp_type[2];
        };
        union {
                __u16 dport[2];
                __u8 icmp_code[2];
        };
};

struct ebt_ip_info_T {
 #if 0
         __be32 saddr;
         __be32 daddr;
         __be32 smsk;
         __be32 dmsk;
         __u8  tos;
 #else
         __be32 saddr[2];
         __be32 daddr[2];
         __be32 smsk[2];
         __be32 dmsk[2];
         __u8  tos[2];
         __u8  dscp[2];
 
 #endif
         __u8  protocol;
         __u8  bitmask;
         __u8  invflags;
         union {
                 __u16 sport[2];
                 __u8 icmp_type[2];
                 __u8 igmp_type[2];
         };
         union {
                 __u16 dport[2];
                 __u8 icmp_code[2];
         };
 };

struct ebt_ip_info {
    /* tianyi for ip address range, 2013.11.12 */
#if 0
	__be32 saddr;
	__be32 daddr;
	__be32 smsk;
	__be32 dmsk;
	__u8  tos;
	__u8  dscp; /* brcm */

#else
	__be32 saddr[2];/* tianyi */
	__be32 daddr[2];
	__be32 smsk[2];
	__be32 dmsk[2];
    __u8  tos[2];
	__u8  dscp[2]; /* brcm */
#endif
	__u8  protocol;
	__u8  bitmask;
	__u8  invflags;
	__u16 sport[2];
	__u16 dport[2];
};



int main(int argc, char const *argv[])
{
	printf("%ld %ld\n", sizeof(struct ebt_ip_info), sizeof(struct ebt_ip_info_T));
	return 0;
}


    if (info->bitmask & EBT_IP6_TCLASS)
    {
        if (info->tclass[0] == info->tclass[1])
        {
	        if (NF_INVF(info, EBT_IP6_TCLASS, info->tclass[0] != ipv6_get_dsfield(ih6)))
		        return false;
        }
        else
        {
            bool m;

            m = ipv6_get_dsfield(ih6) < info->tclass[0];
            m |= ipv6_get_dsfield(ih6) > info->tclass[1];
            if (NF_INVF(info, EBT_IP6_TCLASS, m))
                return false;
        }
    }
    if (info->bitmask & EBT_IP6_FL)
    {
        u32 flowLabel = ntohl(*(u32 *)ih6);
		flowLabel = (flowLabel & 0x000fffff);
        
        if (info->flowlabel[0] == info->flowlabel[1])
        {
	        if (NF_INVF(info, EBT_IP6_FL, info->flowlabel[0] != flowLabel))
		        return false;
        }
        else
        {
            bool m;

            m = flowLabel < info->flowlabel[0];
            m |= flowLabel > info->flowlabel[1];
            if (NF_INVF(info, EBT_IP6_FL, m))
                return false;
        }
    }
    if (info->bitmask & EBT_IP6_SOURCE)
    {
       if (ipv6_addr_equal(&info->saddr_min, &info->saddr_max))
       {
            if (NF_INVF(info, EBT_IP6_SOURCE, ipv6_masked_addr_cmp(&ih6->saddr, &info->smsk_min, &info->saddr_min)))
                return false;
       }
       else
       {
            bool m;

            m  = iprange_ipv6_lt(&ih6->saddr, &info->saddr_min);
    		m |= iprange_ipv6_lt(&info->saddr_max, &ih6->saddr);
    		if (NF_INVF(info, EBT_IP6_SOURCE, m))
    			return false;
       }
    }
    if (info->bitmask & EBT_IP6_DEST)
    {
       if (ipv6_addr_equal(&info->daddr_min, &info->daddr_max))
       {
            if (NF_INVF(info, EBT_IP6_DEST, ipv6_masked_addr_cmp(&ih6->daddr, &info->dmsk_min, &info->daddr_min)))
                return false;
       }
       else
       {
            bool m;

            m  = iprange_ipv6_lt(&ih6->daddr, &info->daddr_min);
    		m |= iprange_ipv6_lt(&info->daddr_max, &ih6->daddr);
    		if (NF_INVF(info, EBT_IP6_DEST, m))
    			return false;
       }
    }
