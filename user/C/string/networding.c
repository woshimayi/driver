#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<errno.h>



intget_netportstatus(constchar*interface) {
	charcmd[1024];
	char *tt;
	FILE *fp;
	intdevflag;
	devflag = get_netflag(interface);
	
	if (devflag == DEV_DOWN)
		{
		   sprintf(cmd, "ifconfig%sup", interface);
		   system(cmd);
		   
		  }
		sprintf(cmd, "ethtool%s|grep"Linkdetected">/tmp/eth.temp", interface);
	system(cmd);
	
	if (devflag == DEV_DOWN)
		{
		   sprintf(cmd, "ifconfig%sdown", interface);
		   system(cmd);
		  }
		fp = fopen("/tmp/eth.temp", "r");
	if (fp == NULL)
		{
		   system("rm-rf/tmp/eth.temp");
		   return -1;
		  }
		fgets(cmd, 1024, fp);
	fclose(fp);
	system("rm-rf/tmp/eth.temp");
	tt = strstr(cmd, "no");
	if (tt != NULL)
		returnLINK_DOWN;
	tt = strstr(cmd, "yes");
	if (tt != NULL)
		returnLINK_UP;
	return -1;
}



structethtool_value
{
	__uint32_tcmd;
	__uint32_tdata;
};



intmain(int, char *[])
{
	
	structethtool_valueedata;
	intfd =  -1, err = 0;
	structifreqifr;
	
	memset(&ifr, 0, sizeof(ifr));
	
	strcpy(ifr.ifr_name, "eth0");
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (fd < 0)
		{
		perror("Cannotgetcontrolsocket");
		return70;
	}
	
	edata.cmd = 0x0000000a;
	ifr.ifr_data = (caddr_t)& edata;
	err = ioctl(fd, 0x8946, &ifr);
	if (err == 0)
		{
		
		fprintf(stdout, "Linkdetected:%sn", 
		                                        
	}elseif (errno != EOPNOTSUPP)
		{
		perror("Cannotgetlinkstatus");
	}
	return0;
}



//如果网卡已脸上网线，返回0，否则返回-1.
intcheck_nic(char*nic)
{
	structifreqifr;
	intskfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strcpy(ifr.ifr_name, nic_name);
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
		{
		return -1;
	}
	if (ifr.ifr_flags&IFF_RUNNING)
		return0;
	//网卡已插上网线
	elsereturn -1;
}


