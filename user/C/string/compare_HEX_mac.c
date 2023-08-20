/*
 * @*************************************:
 * @FilePath: /user/C/string/compare_HEX_mac.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-20 12:16:29
 * @Descripttion: mac 比较
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief
 *
 * @param macstring[in]
 * @param machex[out]
 */
void xg_macStringToHex(char *macstring, unsigned char *machex)
{
	char *res = NULL;
	char *savePtr = NULL;
	int i = 0;

	while (NULL != (res = strtok_r(macstring, ":", &savePtr)))
	{
		printf("0x%2.2x \n", (char)strtol(res, NULL, 16) & 0xff);

		*machex++ = (char)strtol(res, NULL, 16);
		macstring = NULL;
		if (NULL == machex)
		{
			break;
		}
	}
	
	for (int i = 0; i < 6; i++)
	{
	  printf("0X%x\t", machex++);
	}
	printf("\n");
}

static int strnlen_safe(const char *str, int max) {
  // note: strnlen is not supported on all compilers -- create our own version
  const char * end = memchr(str, '\0', max+1);
  if (end && *end == '\0') {
       return end-str;
  }
  return -1;
}


int cmsUtl_macStrToNum(const char *macStr, unsigned char *macNum) 
{
   int i;
   unsigned int macStrLen;
   
   if (macNum == NULL || macStr == NULL) 
   {
      printf("Invalid macNum/macStr %p/%p\n", macNum, macStr);
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


void getNthMacAddr( unsigned char *pucaMacAddr, unsigned long n)
{
    unsigned long macsequence = 0;
    /* Work with only least significant three bytes of base MAC address */
    macsequence = (pucaMacAddr[3] << 16) | (pucaMacAddr[4] << 8) | pucaMacAddr[5];
    macsequence = (macsequence + n) & 0xffffff;
    pucaMacAddr[3] = (macsequence >> 16) & 0xff;
    pucaMacAddr[4] = (macsequence >> 8) & 0xff;
    pucaMacAddr[5] = (macsequence ) & 0xff;

}


int main()
{
	char str1[] = "74:b9:eb:cb:0d:b2";
	char str2[] = {0};
	printf("%s\n %s\n", str1, str2);
	xg_macStringToHex(str1, str2);
	printf("%x\n %x\n", str1, str2);

	cmsUtl_macStrToNum(str1, str2);
	for (int i = 0; i < sizeof(str2); i++)
	{
	  printf("%x\t", str2[i]);
	}
	printf("\n");

	unsigned long mac = 0;
	getNthMacAddr(str2, 1);

	return 0;
}
