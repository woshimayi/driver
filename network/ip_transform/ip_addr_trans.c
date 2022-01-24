/*
 * @*************************************: 
 * @FilePath: /network/ip_transform/ip_addr_trans.c
 * @version: 
 * @Author: dof
 * @Date: 2021-12-28 15:46:42
 * @LastEditors: dof
 * @LastEditTime: 2022-01-17 20:09:05
 * @Descripttion:  ip 地址比较大小
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int ipv6_compare(char * min, char * max)
{
	struct in6_addr ip1;
	struct in6_addr ip2;
	int result = 0;

	inet_pton(AF_INET6, min, &ip1);
	inet_pton(AF_INET6, max, &ip2);

	result = memcmp(&ip1, &ip2, sizeof(struct in6_addr));
	if (result > 0)
		printf("%s > %s\n", min, max);
	else if (result < 0)
		printf("%s < %s\n", min, max);
	else
		printf("%s is == %s\n", min, max);

	return 0;
}


int ipv4_compare(char * min, char * max)
{
	struct in_addr ip1;
	struct in_addr ip2;
	int result = 0;

	inet_pton(AF_INET, min, &ip1);
	inet_pton(AF_INET, max, &ip2);

	result = memcmp(&ip1, &ip2, sizeof(struct in_addr));
	if (result < 0)
	{
		printf("%s < %s\n", min, max);
	}
	else if (result > 0)
	{
		printf("%s > %s\n", min, max);
	}
	else
	{
		printf("%s == %s\n", min, max);
	}

	return result;
}

#if 0
UBOOL8 cmsUtl_isValidIpAddress(SINT32 af, const char* address)
{
   if ( IS_EMPTY_STRING(address) ) return FALSE;
#ifdef SUPPORT_IPV6
   if (af == AF_INET6)
   {
      struct in6_addr in6Addr;
      UINT32 plen;
      char   addr[CMS_IPADDR_LENGTH];

      if (cmsUtl_parsePrefixAddress(address, addr, &plen) != CMSRET_SUCCESS)
      {
         cmsLog_debug("Invalid ipv6 address=%s", address);
         return FALSE;
      }

      if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
      {
         cmsLog_debug("Invalid ipv6 address=%s", address);
         return FALSE;
      }

      return TRUE;
   }
   else
#endif
   {
      if (af == AF_INET)
      {
         return cmsUtl_isValidIpv4Address(address);
      }
      else
      {
         return FALSE;
      }
   }
}  /* End of cmsUtl_isValidIpAddress() */


UBOOL8 cmsUtl_isValidMacAddress(const char* input)
{
   UBOOL8 ret =  TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_32];
   UINT32 i, num;

   if (input == NULL || strlen(input) != MAC_STR_LEN)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   if ((strlen(pToken) != 2) ||
       (cmsUtl_strtoul(pToken, NULL, 16, &num) != CMSRET_SUCCESS))
   {
      ret = FALSE;
   }
   else
   {
      for ( i = 0; i < 5; i++ )
      {
         pToken = strtok_r(NULL, ":", &pLast);
         if ((strlen(pToken) != 2) ||
             (cmsUtl_strtoul(pToken, NULL, 16, &num) != CMSRET_SUCCESS))
         {
            ret = FALSE;
            break;
         }
      }
   }

   return ret;
}


UBOOL8 cmsUtl_isValidPortNumber(const char * portNumberStr)
{
   UINT32 portNum;

   if (cmsUtl_strtoul(portNumberStr, NULL, 10, &portNum) != CMSRET_SUCCESS) 
   {
      return FALSE;
   }

   return (portNum < (64 * 1024));
}
#endif


int macaddr_compare(char *min, char *max)
{
	return strcmp(min, max);
}

int main()
{
	char *min = "192.168.4.1";
	char *max = "192.168.4.1";
	int ret = 0;

	if (ipv4_compare(min, max))
	{
		printf("ssss\n");
	}

	char *str1 = "2000::10";
	char *str2 = "2000::2";

	if (ipv6_compare(str1, str2))
	{
		printf("ssss\n");
	}

	char *mac1 = "00:11:22:33:44:57";
	char *mac2 = "00:11:22:33:44:56";
	printf("mac %d \n", macaddr_compare(mac1, mac2));

	return 0;
}