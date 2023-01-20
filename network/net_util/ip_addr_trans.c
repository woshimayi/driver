/*
 * @*************************************: 
 * @FilePath: /network/net_util/ip_addr_trans.c
 * @version: 
 * @Author: dof
 * @Date: 2021-12-28 15:46:42
 * @LastEditors: dof
 * @LastEditTime: 2023-01-10 13:13:39
 * @Descripttion:  ip 地址比较大小
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

typedef unsigned char_t    unsigned char;


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

      if (cmsUtl_parsePrefixAddress(address, addr, &plen) != 0)
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
       (cmsUtl_strtoul(pToken, NULL, 16, &num) != 0))
   {
      ret = FALSE;
   }
   else
   {
      for ( i = 0; i < 5; i++ )
      {
         pToken = strtok_r(NULL, ":", &pLast);
         if ((strlen(pToken) != 2) ||
             (cmsUtl_strtoul(pToken, NULL, 16, &num) != 0))
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

   if (cmsUtl_strtoul(portNumberStr, NULL, 10, &portNum) != 0) 
   {
      return FALSE;
   }

   return (portNum < (64 * 1024));
}
#endif


/**
 * @brief      str to num
 * 
 * @param macStr  input
 * @param macNum  output
 * @return int 
 */
int cmsUtl_macStrToNum(const char *macStr, unsigned char *macNum) 
{
   unsigned int i;
   unsigned int macStrLen;
   
   if (macNum == NULL || macStr == NULL) 
   {
      printf("Invalid macNum/macStr %p/%p", macNum, macStr);
      return -1;
   }    
   macStrLen = strnlen_safe(macStr, 19);
   
   i=sscanf(macStr, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
          &(macNum[0]), &(macNum[1]), &(macNum[2]), &(macNum[3]), &(macNum[4]), &(macNum[5]));

   if (i != 6 && macStrLen == 14) {
     i=sscanf(macStr, "%2hhx%2hhx:%2hhx%2hhx:%2hhx%2hhx", 
              &(macNum[0]), &(macNum[1]), &(macNum[2]), &(macNum[3]), &(macNum[4]), &(macNum[5]));
   }

   if (i != 6 && macStrLen == 12) {
     i=sscanf(macStr, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", 
              &(macNum[0]), &(macNum[1]), &(macNum[2]), &(macNum[3]), &(macNum[4]), &(macNum[5]));
   }

   if (i != 6) {
      return -1;
   }
   
   return 0;
   
}

/**
 * @brief  mac num to str
 * 
 * @param macNum  input
 * @param macStr  output
 * @return int 
 */
int cmsUtl_macNumToStr(const unsigned char *macNum, char *macStr) 
{
   if (macNum == NULL || macStr == NULL) 
   {
      printf("Invalid macNum/macStr %p/%p", macNum, macStr);
      return -1;
   }  

   sprintf(macStr, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (unsigned char) macNum[0], (unsigned char) macNum[1], (unsigned char) macNum[2],
           (unsigned char) macNum[3], (unsigned char) macNum[4], (unsigned char) macNum[5]);

   return 0;
}


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