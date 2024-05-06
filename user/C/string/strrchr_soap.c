#include <stdio.h>
#include <string.h>

int main()
{
    const char str[][1024] = {"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.Enable",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.ConnectionType",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_VLANMode",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_VLANIDMark",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_ServiceList",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_MulticastVlan",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_IPMode",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Enable",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Type",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Account",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION60.1.Password",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION125.1.Enable",
          "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.2.X_CMCC_DHCPOPTION125.1.Value"};
          const char ch = '.';
          char *ptr;
          char oldtmp[1024] = {0};
          char newtmp[1024] = {0};
          int saveFlag = 0;


          for (int i=0; i<sizeof(str)/sizeof(str[0]); i++)
          {
              printf("str[%d] = %s\n", i, str[i]);
#if 1
              ptr = strrchr(str[i], ch);

              if (ptr != NULL)
              {
                  //printf("字符 '%c 出现的位置为 %ld. \n",ch,  ptr - str[i] + 1);
                  //printf("|%c| 之后的字符串是 - |%s|\n", ch, ptr);
                  strncpy(newtmp, str[i], ptr-str[i]+1);
                  if (0 == i)
                  {
                      strncpy(oldtmp, newtmp, sizeof(newtmp));
                  }
                  if (strcmp(oldtmp, newtmp))
                  {
                      saveFlag = 1;
                      strncpy(oldtmp, newtmp, sizeof(newtmp));
                  }
                  if (saveFlag)
                  {
                    printf("\033[0;32;31m");
                    printf("save success\n");
                    printf("\033[1;37m");
                    saveFlag = 0;
                  }
                  printf(" tmp = %s\n\t%s\n\n", newtmp, oldtmp);
              }
              else
              {
                  printf("没有找到字符 'd' \n");
              }
          }
#endif
          return (0);
}
