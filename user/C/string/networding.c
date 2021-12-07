#include <sys/types.h>  
#include <string.h>  
#include <stdlib.h>  
#include <sys/ioctl.h>  
#include <stdio.h>  
#include <errno.h>  



int get_netportstatus(const char *interface)  {  
     char cmd[1024];
     char  *tt;
     FILE  *fp;
     int devflag;
     devflag = get_netflag(interface);
      
     if  (devflag == DEV_DOWN)
         {  
              sprintf(cmd, "ifconfig %s up", interface);
              system(cmd);
             
          }  
          sprintf(cmd, "ethtool %s | grep "Link detected" > /tmp/eth.temp", interface);
      system(cmd);
      
      if  (devflag == DEV_DOWN)
         {  
               sprintf(cmd, "ifconfig %s down", interface);
               system(cmd);
          }  
          fp = fopen("/tmp/eth.temp", "r");
      if  (fp == NULL)
         {  
               system("rm -rf /tmp/eth.temp");
               return  -1;
          }  
          fgets(cmd, 1024, fp);
      fclose(fp);
      system("rm -rf /tmp/eth.temp");
      tt = strstr(cmd, "no");
      if  (tt != NULL)
         return LINK_DOWN;
      tt = strstr(cmd, "yes");
      if  (tt != NULL)
         return LINK_UP;
      return  -1;
}  

  
  
struct ethtool_value 
{
            __uint32_t      cmd;
            __uint32_t      data;
};
  
  
  
int main(int ,  char * [])  
{
      
        struct ethtool_value edata;
        int fd  =   -1,  err  =  0;
        struct ifreq ifr;
      
            memset(&ifr,  0,  sizeof(ifr));
      
            strcpy(ifr.ifr_name,  "eth0");
      
            fd  =  socket(AF_INET,  SOCK_DGRAM,  0);
      
            if  (fd  <  0)
         {
                        perror("Cannot get control socket");
                        return 70;
    }  
      
            edata.cmd  =  0x0000000a;
            ifr.ifr_data  =  (caddr_t)& edata;
            err  =  ioctl(fd,  0x8946,  &ifr);
            if  (err  ==  0)
         {
          
                        fprintf(stdout,  "Link detected: %sn",   
                                                        
    } else if  (errno  !=  EOPNOTSUPP)
         {
                        perror("Cannot get link status");
    }  
       return 0;
}  



//如果网卡已脸上网线，返回0，否则返回-1.  
int check_nic(char *nic)  
{
        struct ifreq ifr;
        int skfd  =  socket(AF_INET,  SOCK_DGRAM,  0);
      
        strcpy(ifr.ifr_name,  nic_name);
        if  (ioctl(skfd,  SIOCGIFFLAGS,  &ifr)  <  0)
            {
                return  -1;
    }  
        if (ifr.ifr_flags & IFF_RUNNING)
                return 0;
      // 网卡已插上网线  
        else return  -1;
}  


