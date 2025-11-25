/*
 * @*************************************: 
 * @FilePath     : /network/param_parse.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2025-08-21 09:54:40
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-22 17:47:44
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */



 int main(int argc, char const *argv[])
 {
     printf("0x%04x\n", ntohs(0x0002));
     char *aucAttachContext = "192.85.1.3";
     char *table = "iptables";

     if ((!strcmp(table, "iptables") && strstr(aucAttachContext, ":")) || (!strcmp(table, "ip6tables") && !strstr(aucAttachContext, ":")))
     {
         printf("dddd\n");
     }

     return 0;
 }
 