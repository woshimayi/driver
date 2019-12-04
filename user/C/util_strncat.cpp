#include<stdio.h>
#include<string.h>
#include<stdlib.h>

char *util_strncat(const char *func, int line, char *dest, const char *src, int dlen)
{
   int n = 0;

   if ((src == NULL) || (dest == NULL) || dlen <= 0)
   {
      printf("%s:%u:null pointer reference src =%p ,dest =%p, dlen = %d", func, line, src, dest, dlen);
      return dest;
   }

   n = dlen - strlen(dest);
   if (n <= 0)
   {
      printf("%s:%u:dlen < util_strlen(dest)", func, line);
      return dest;
   }

   if (strlen(src) + 1 > n)
   {
      printf("%s:%u:truncating:src string length > dest buffer", func, line);
      strncat(dest, src, n - 1);
      dest[dlen - 1] ='\0';
   }
   else
   {
      strncat(dest, src, n);
   }

   return dest;
}

#define UTIL_STRNCAT(dest, src, n)    util_strncat(__FUNCTION__, __LINE__, dest, src, n)


int main()
{
	char * num = ",";
	char * num1 ="sdfsdf";
	
	UTIL_STRNCAT(num, num1, sizeof((char*)num1)); 
	
	printf("%s\n", num);
		return 0;
}

