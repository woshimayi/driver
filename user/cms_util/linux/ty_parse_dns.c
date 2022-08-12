/*Author xukeming*/

#include "cms_log.h"
#include "parse_dns.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <time.h>

static char arrayStr[10][50] ={{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};     


static int  arraystr_length = 0;    
static unsigned short SendSessionID;
static struct SEND_PACKAGE sendPkg;  
static struct sockaddr_in6 fromv6; 

#define isspace0(c)	((c) == ' ')
#define DMP_DNS_REQUEST 		0
#define DMP_DNS_RESPONSE 		1

#define DMP_ERROR_ZERO				0
#define DMP_ERROR_FORMAT			1
#define DMP_ERROR_FAIL				2
#define DMP_ERROR_NAME				3
#define DMP_ERROR_NOT_SURPPORT	4
#define DMP_ERROR_REFUSED			5

#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"

static void getArrayByDomainStr(const char* Domain)   
{   
	char byChar = 0;      
	char strTemp[50] = {0};     
	int fir = 0, w;   
	int sec = 0;   

	//Added by xuyong ,fix the number of domain name section  is wrong
	int domainLen;	
	domainLen = strlen(Domain);
	
	cmsLog_debug("getArrayByDomainStr begin"); 

	//Added by xuyong ,fix the number of domain name section  is wrong
	//for(w = 0; w < 50; w++)      
	for(w = 0; w < domainLen; w++)
	{      
		byChar = Domain[w];      
		if('.' == byChar)      
		{    
			//int t = strlen(strTemp);   

			//Modified by xuyong,can't use memcpy,the end of string is confused
			//memcpy(arrayStr[sec], strTemp, t);    
			strcpy(arrayStr[sec],strTemp);
			
			sec++;   
			memset(strTemp,0,50);   
			fir = 0;   
		}      
		else      
		{      
			strTemp[fir]= byChar;   
			fir++;   
		}      
	}    
	if (strTemp[0] != 0)   
	{   
		//Modified by xuyong,can't use memcpy,the end of string is confused
		//memcpy(arrayStr[sec], strTemp, strlen(strTemp));    
		strcpy(arrayStr[sec], strTemp);    
		
		sec++;   
	}          
	arraystr_length = sec;   
} 

static void getPackageRequestIP(struct SEND_PACKAGE *sendPkg, const char* Domain, const char IsIpv6)   
{
        char byBuf[526] = {0};   
        char byTemp = 0;      
        int  pDataPtr = 0;   
        int  iLastLen = 0;                     //最终查询报文长度  
		int  i = 0;		
        char strElement[50] = {0};         
        struct DNS_REQUEST_HEAD dnsHead;    
        struct DNS_REQUEST_TAIL dnsTail;   
		
   	    cmsLog_debug("getPackageRequestIP begin");  
        //head(12字节)   初始化报头字段   
        dnsHead.usSessionID = htons(0X0300);  
	 SendSessionID = dnsHead.usSessionID;	
        dnsHead.usFlag.allFlag = htons(DNS_FLAG_RD);      
        dnsHead.usQuestions = htons(0X0001);      
        dnsHead.usAnswer = htons(0X0000);      
        dnsHead.usAuthority = htons(0X0000);      
        dnsHead.usAdditional = htons(0X0000);      
        memcpy(byBuf+pDataPtr, &dnsHead, DNS_HEAD_SIZE);      
        pDataPtr = pDataPtr + DNS_HEAD_SIZE;      
        //构造报文身体      
        getArrayByDomainStr(Domain);   
        for(i = 0; i < arraystr_length; i++)      
        {      
        	////Modified by xuyong,can't use memcpy,the end of string is confused
            //memcpy(strElement, arrayStr[i], strlen(arrayStr[i]));      
            strcpy(strElement, arrayStr[i]);
			
            //写长度      
            byTemp = strlen(strElement);      
            byBuf[pDataPtr] = byTemp;        
            ++pDataPtr;      
            //写内容      
            memcpy(byBuf+pDataPtr,strElement,byTemp);    
            memset(strElement,0,50);   
            pDataPtr = pDataPtr + byTemp;      
        }      
        //标识DomainStr已经完了      
        byBuf[pDataPtr] = 0X00;   
        ++pDataPtr;      
               
      
        //根据域名查找IP     
        dnsTail.usRequestType = IsIpv6 ? htons(DNS_RRTYPE_AAAA):htons(DNS_RRTYPE_A);       
        dnsTail.usRequestClass = htons(DNS_RRTYPE_A);      
        memcpy(byBuf+pDataPtr, &dnsTail, DNS_TAIL_SIZE);      
        pDataPtr = pDataPtr + 4;      
       
        iLastLen = pDataPtr;       
        sendPkg->pData = (char *)malloc(iLastLen);   
        memcpy(sendPkg->pData, byBuf, iLastLen);      
        sendPkg->iDataLen = iLastLen;   
}

static ParseRet processRequestReply(char *RecvBuf, const int BufLength, const char IsIpv6, const char *strdomain, char *outputip)   
{
      int pDataPtr = 0;   
      struct DNS_REQUEST_HEAD dnsHead;   
      struct DNS_REQUEST_TAIL dnsTail;   
      unsigned short usRedirectPtr = 0;      
      unsigned short usRecordType = 0;      
      unsigned short usRecordClass = 0;      
      unsigned short usTrueDataLen = 0;      
      unsigned int iTrueIP = 0;      
      struct in_addr userIP;         
	  char *pData = RecvBuf; 
	  struct in6_addr Temp;
	  char *pstrbegin;
	  char *pstrend;
	  char domainstr[100] = {0},domain[100] = {0};
	  int answerNum = 0;
	  unsigned char found = 0;
	  ParseRet ret = PARSE_DNS_SUCCESS;
	  
	  cmsLog_debug("processRequestReply begin");  
	  //DNS报头的装载      
      memcpy((void*)&dnsHead, pData+pDataPtr, DNS_HEAD_SIZE);   //pDataPtr=0   
      dnsHead.usFlag.allFlag = ntohs(dnsHead.usFlag.allFlag);  
      dnsHead.usQuestions = ntohs(dnsHead.usQuestions);      
      dnsHead.usAnswer = ntohs(dnsHead.usAnswer);          
      dnsHead.usAuthority = ntohs(dnsHead.usAuthority);           
      dnsHead.usAdditional = ntohs(dnsHead.usAdditional);   
	  
	  if (SendSessionID == dnsHead.usSessionID)
	  {
	  	if (DMP_DNS_RESPONSE != dnsHead.usFlag.bits.Response)
	  	{
	  		cmsLog_debug("[typing] fun=%s allFlag=0x%x", __FUNCTION__, dnsHead.usFlag.allFlag);
	  		return ret;
	  	}

	  	if (DMP_ERROR_ZERO != dnsHead.usFlag.bits.Reply_code)
	  	{
	  		cmsLog_debug("[typing] fun=%s allFlag=0x%x Reply_code=0x%x",__FUNCTION__, dnsHead.usFlag.allFlag, dnsHead.usFlag.bits.Reply_code);
	  		return PARSE_DNS_ERROR;
	  	}	

		if (0 == dnsHead.usAnswer)
		{
			cmsLog_debug("[typing] fun=%s allFlag=0x%x dnsHead.usAnswer=0",__FUNCTION__, dnsHead.usFlag.allFlag);
			return PARSE_DNS_ERROR;
		}
	  }
	  
      pDataPtr = pDataPtr + DNS_HEAD_SIZE;   //pDataPtr=12
	  pstrbegin = pData+pDataPtr;
	  
	  //body,temporary jump to End '\0'      
      while((pDataPtr < BufLength) && (0X00 != *(pData + pDataPtr)))      
      {      
          ++pDataPtr;      
      }     
		
      ++pDataPtr;    
	  pstrend = pData+pDataPtr;  
	  memcpy((void*)&domainstr, pstrbegin, (pstrend - pstrbegin));
	  
	  if (domainstr[0] != 0)
	  {
			int i = 0,j,c,z = 0;
			while((domainstr[i] != 0) && (i < sizeof(domainstr)) && (z < sizeof(domainstr)))
			{
				z++;						//防止死循环
				c = domainstr[i];
				for(j = 0; j < c; j ++)
				{
					domain[i+j] = domainstr[i+j+1];
				}
				if (domainstr[i + c + 1] != 0)
				{
					domain[i+j] = '.';
				}
				i = i + c + 1;
			}
	  }
	  cmsLog_debug("domain=%s",domain);
      //报文身体      
      memcpy((void*)&dnsTail, pData + pDataPtr, DNS_TAIL_SIZE);      
      pDataPtr = pDataPtr + DNS_TAIL_SIZE; 
	  
      while(pDataPtr < BufLength)      
      {      
          //DomainStr，重定向报文域名 2个字节      
          //一般为C00C，二进制为1100000000001100   
          //最开始2Bit为11，剩下的Bit构成一个指针，指向DomainStr        
          memcpy(&usRedirectPtr, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usRedirectPtr = ntohs(usRedirectPtr);  
		  
          if(0X00 == usRedirectPtr)      
          {      
			cmsLog_error("Domain name has not been IP, domain name may be illegal");      
			return ret;      
          }      
		  
          //类型,2 BYTES      
          memcpy(&usRecordType, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usRecordType = ntohs(usRecordType);    

		  if ((0 == answerNum) && (0xC00C == usRedirectPtr))
		  {
		  	found = 1;
		  }
		  
		  if (answerNum >= dnsHead.usAnswer)
		  {
		  	if (0xC00C == usRedirectPtr)
		  	{	
		  		found = 1;
		  	}
			else
			{
				found = 0;
			}
		  }
		  
          //类,2 BYTES      
          memcpy(&usRecordClass, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usRecordClass = ntohs(usRecordClass);      
          //TTL,4 BYTES      
          pDataPtr = pDataPtr + 4;      
          //后续真实数据长度，2 BYTES      
          memcpy(&usTrueDataLen, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usTrueDataLen = ntohs(usTrueDataLen);      
          if (IsIpv6)
          {
	          if((DNS_RRTYPE_AAAA == usRecordType)\
				&& (SendSessionID == dnsHead.usSessionID)\
				&& (!strcmp(strdomain, domain))
				&& (1 == found))   
	          {      
			      if (usTrueDataLen == 16)
				  {
					memcpy(&Temp.s6_addr, pData+pDataPtr, usTrueDataLen);   
					
					//inet_ntop(AF_INET6, &Temp.s6_addr, (char *)&outputip, sizeof(outputip));
					//modified by xuyong
					//can't use memcpy,the end token of string is mixed when there is 2 or more address and the first is longer than the second
					inet_ntop(AF_INET6,&Temp,outputip,49);
					
					cmsLog_debug("outputip=%s--------------",outputip);
				  }

	          }  
          }
		  else
		  {
		       if((DNS_RRTYPE_A == usRecordType)\
				&& (SendSessionID == dnsHead.usSessionID)\
				&& (!strcmp(strdomain, domain))
				&& (1 == found))   
	          {      
			      if (usTrueDataLen == 4)
				  {
					memcpy(&iTrueIP, pData+pDataPtr, 4);   
					userIP.s_addr = iTrueIP;  
					
					//memcpy(outputip, inet_ntoa(userIP), strlen(inet_ntoa(userIP))); 
					//modified by xuyong
					//can't use memcpy,the end token of string is mixed when there is 2 or more address and the second is longer than first
					inet_ntop(AF_INET,&userIP,outputip,16);
					
					
					cmsLog_debug("outputip=%s---------------------",outputip);    
				  }

	          }   
		  }
          pDataPtr = pDataPtr + usTrueDataLen;     
		  answerNum++;
      }      
	  return ret;
}

ParseRet ifnameGetIPv6addr(const char *ifname, char islinkaddr, char *hoststr)
{
	struct ifaddrs *ifaddr, *ifa;	
	 int family;   
	char *clientip = hoststr;
	
	cmsLog_debug("ifnameGetIPv6addr begin ifname=%s",ifname);

	 if ((ifname == NULL) || (hoststr == NULL))
	 {
		return GET_IFNAMEIP_ERROR;
	 }	
	 
	 if (getifaddrs(&ifaddr) == -1)    
	 {	 
		 cmsLog_error("getifaddrs");	 
		 return GET_IFNAMEIP_ERROR;
	 }	 
	
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)	
	 {	 
		if (ifa->ifa_addr == NULL)	 
			 continue;	 
	
		family = ifa->ifa_addr->sa_family;
	
		 if (strcmp(ifa->ifa_name, ifname) != 0)
		 {
			 continue;
		 }
		 
		if (family == AF_INET6)    
		{	
			  if (ifa->ifa_flags & IFF_UP) 
			 {
				 cmsLog_debug("the interface status is UP");
			 } else 
			 {
				 cmsLog_error("the interface status is DOWN");
				 freeifaddrs(ifaddr); 
				 return GET_IFNAMEIP_ERROR;   
			 }

			 inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, hoststr, 50);
			 cmsLog_error("zzzzz host: %s", hoststr);
			 break;
			
			if (((islinkaddr)&& ((((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr.s6_addr32[0] & htonl(0xFFC00000)) == htonl(0xFE800000)))
			 || ((!islinkaddr)&& ((((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr.s6_addr32[0] & htonl(0xFFC00000)) != htonl(0xFE800000))))
			{
				inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, hoststr, 50);
//				clientip = hoststr;
				cmsLog_error("zzzzz host: %s", hoststr);
				break;
			}
		}	
	}	
	
	freeifaddrs(ifaddr); 

	cmsLog_error("zzzzz clientip = %s", clientip);
	if (!clientip)
	{
		cmsLog_error("the interface: %s is error", ifname);	 
		return GET_IFNAMEIP_ERROR;
	}
	return GET_IFNAMEIP_SUCCESS;
}
/*added by lwy just for herself*/
ParseRet pParaseIpv4Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp)
{
	struct sockaddr_in	cli_addr;
	struct sockaddr_in	serv_addr, back_serv_addr;
    int sockfd;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	//char haveBackDns = 0;
	char hoststr[50] = {0};
	int sockopt;
	
	cmsLog_debug("ParaseIpv4Domain begin :%s,requeset dns:%s,dns server:%s",ifname,strdomain,dnsserver);
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	
	bzero(&serv_addr,sizeof(struct sockaddr_in)); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(53); /* 53 */
	if (ifnameGetIPv4addr(ifname, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_aton(tmpBuf, &serv_addr.sin_addr)<0)  
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		//haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in)); 
		back_serv_addr.sin_family = AF_INET;
		back_serv_addr.sin_port = htons(53); /* 53 */

		if(inet_aton(dnsSecondary, &back_serv_addr.sin_addr)<0) 
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	//sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(0); 
	
	if(inet_aton(hoststr, &cli_addr.sin_addr)<0)  
	{
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, 0); 
	
	if (sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))< 0)
	{
		cmsLog_error("sendto");
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
		return PARSE_DNS_SUCCESS;
	}
		
}

ParseRet ParaseIpv6Domain (const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp,const char IsIpv6)
{
	struct sockaddr_in6	cli_addr;
	struct sockaddr_in6	serv_addr, back_serv_addr;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};  

	struct timeval ts;
	ts.tv_sec = 2;
	ts.tv_usec = 0;
	
	cmsLog_debug("begin");
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	
	bzero(&serv_addr,sizeof(struct sockaddr_in6)); 
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(53); /* 53 */
	if (ifnameGetIPv6addr(ifname, 0, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	cmsLog_error("zzzzz hoststr = %s", hoststr);
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_pton(AF_INET6, tmpBuf, &serv_addr.sin6_addr)<0) 
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in6)); 
		back_serv_addr.sin6_family = AF_INET6;
		back_serv_addr.sin6_port = htons(53); /* 53 */

		if(inet_pton(AF_INET6, dnsSecondary, &back_serv_addr.sin6_addr)<0)
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	sockfd=socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in6));
	cli_addr.sin6_family = AF_INET6;
	cli_addr.sin6_port = htons(53); 
	
	if(inet_pton(AF_INET6, hoststr, &cli_addr.sin6_addr)<0)
	{ 
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
	int DontLinger = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&DontLinger, sizeof(int)) < 0)
	{
	   cmsLog_error("fail to set sock opt SO_REUSEADDR");
//	   return	   BIND_IP_ERROR;
	}

	sockopt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) < 0) 
	{
	   cmsLog_error("fail to set sock opt SO_REUSEADDR");
//	   return  	   BIND_IP_ERROR;
	}

//	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in6)) < 0)
//	{
//		cmsLog_error("bind error");
//		perror("bind");
//		close(sockfd);
//		return BIND_IP_ERROR;
//	}
	
	getPackageRequestIP(&sendPkg, strdomain, IsIpv6); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6))) < 0)
	{
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");
		//added by xukeming 2017.1.20 begin
		//optimize dns time
		if (0 == trytime)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}
		else if (1 == trytime)
		{
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		}
		else if (2 == trytime)
		{
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
		}
		else
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
		//added by xukeming 2017.1.20 end
		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime >= MAX_TRY_TIME)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);	  
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime = 0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end
					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						ParseRet ret2 = PARSE_DNS_SUCCESS;
						
						addrlen = sizeof(struct sockaddr_in6); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&fromv6, (socklen_t *)&addrlen);
						
						 //printf("-----------------fromv6.sin6_ipaddr:"NIP6_FMT"\n", NIP6(fromv6.sin6_addr));
						 //printf("-----------------serv_addr.sin6_ipaddr:"NIP6_FMT"\n", NIP6(serv_addr.sin6_addr));
						 
						if (!memcmp(fromv6.sin6_addr.s6_addr,serv_addr.sin6_addr.s6_addr, 16)\
							|| ((SwitchBackDns) && !memcmp(fromv6.sin6_addr.s6_addr,back_serv_addr.sin6_addr.s6_addr, 16)))
						{
							cmsLog_debug("m_iRecvDataLen=%d",m_iRecvDataLen);
							
							ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, IsIpv6, strdomain, outputip); 
							
							if (PARSE_DNS_SUCCESS != ret2)
							{
								if (haveBackDns) 
								{
									if (SwitchBackDns)
									{
										free(sendPkg.pData);      
										sendPkg.pData = NULL; 
										close(sockfd);
										return ret2;
									}
									else
									{
										SwitchBackDns = 1;
										trytime = 0;
									}
								}
								else
								{
									free(sendPkg.pData);      
									sendPkg.pData = NULL; 
									close(sockfd);
									return ret2;
								}
							}
							
							if (outputip[0] != 0)
							{
								strcpy(domainIp, outputip);
								free(sendPkg.pData);      
								sendPkg.pData = NULL; 
								close(sockfd);
								return PARSE_DNS_SUCCESS;
							}
						}
				}
			break;
		}
	}
}
//added by hlb,2016.03.30
ParseRet ParaseIpv6DomainTimeout (const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp, int waittime,const char IsIpv6)
{
	struct sockaddr_in6	cli_addr;
	struct sockaddr_in6	serv_addr, back_serv_addr;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};  
	ParseRet ret2 = PARSE_DNS_SUCCESS;
	
	cmsLog_debug("begin");
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	
	bzero(&serv_addr,sizeof(struct sockaddr_in6)); 
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(53); /* 53 */
	if (ifnameGetIPv6addr(ifname, 0, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_pton(AF_INET6, tmpBuf, &serv_addr.sin6_addr)<0) 
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in6)); 
		back_serv_addr.sin6_family = AF_INET6;
		back_serv_addr.sin6_port = htons(53); /* 53 */

		if(inet_pton(AF_INET6, dnsSecondary, &back_serv_addr.sin6_addr)<0)
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	sockfd=socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in6));
	cli_addr.sin6_family = AF_INET6;
	cli_addr.sin6_port = htons(0); 
	
	if(inet_pton(AF_INET6, hoststr, &cli_addr.sin6_addr)<0)
	{ 
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in6)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, IsIpv6); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6))) < 0)
	{
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");

		timeout.tv_sec = waittime;
		timeout.tv_usec = 0;
	
		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime >= MAX_TRY_TIME)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);	  
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime = 0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end
					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						addrlen = sizeof(struct sockaddr_in6); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&fromv6, (socklen_t *)&addrlen); 
						if (!memcmp(fromv6.sin6_addr.s6_addr,serv_addr.sin6_addr.s6_addr, 16)\
							|| ((SwitchBackDns) && !memcmp(fromv6.sin6_addr.s6_addr,back_serv_addr.sin6_addr.s6_addr, 16)))
						{
         						cmsLog_debug("m_iRecvDataLen=%d",m_iRecvDataLen);
         						ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, IsIpv6, strdomain, outputip); 
         
         						if (PARSE_DNS_SUCCESS != ret2)
         						{
         							free(sendPkg.pData);	  
         							sendPkg.pData = NULL; 
         							close(sockfd);
         							return ret2;
         						}
         
         						if (outputip[0] != 0)
         						{
         							strcpy(domainIp, outputip);
         							free(sendPkg.pData);      
         							sendPkg.pData = NULL; 
         							close(sockfd);
         							return PARSE_DNS_SUCCESS;
         						}
						}
				}
			break;
		}
	}
}


char IsIpv6Addr(char *str)
{
	int i;
	cmsLog_debug("IsIpv6Addr begin");
	
	for(i = 0; i < strlen(str); i++)
	{
		if (((str[i] >= '0') && (str[i] <= '9'))\
				||(str[i] == ':')\
				||((str[i] >= 'a')&&(str[i] <= 'f'))
				||((str[i] >= 'A')&&(str[i] <= 'F')))
		{
			continue;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}
static int ifnameGetIPv4addr1(const char *ifname, char *hoststr)
{
    int ret = -1;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    int sock;
    char *p = NULL;

    cmsLog_debug("ifnameGetIPv4addr1 ifname:%s", ifname);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (0 > (ret = ioctl(sock, SIOCGIFADDR, &ifr)))
    {
        printf("ifnameGetIPv4addr1 ioctl get SIOCGIFADDR failed!!\n");
		close(sock);
        return -1;
    }
    close(sock);

    sin = (struct sockaddr_in *)&(ifr.ifr_addr);
	
    if (NULL == (p = inet_ntoa(sin->sin_addr)))
    {
    	printf("ifnameGetIPv4addr1 find p null!!\n");
        return -1;
    }
	
    snprintf(hoststr, 1024, "%s", p);
    cmsLog_debug("ifnameGetIPv4addr1 success: %s", hoststr);

    return ret;
}

ParseRet ifnameGetIPv4addr(const char *ifname, char *hoststr)
{
	//ifnameGetIPv4addr may fail 20190612 by wuchuan begin
	if(ifnameGetIPv4addr1(ifname,hoststr) != -1)
	{
		return GET_IFNAMEIP_SUCCESS;
	}
	//ifnameGetIPv4addr may fail 20190612 by wuchuan end.	
	else
	{
		struct ifaddrs *ifaddr, *ifa;	
		int family;   
		char *p = NULL;
		
		cmsLog_debug("ifnameGetIPv4addr begin ifname=%s",ifname);
		 if ((ifname == NULL) || (hoststr == NULL))
		 {
			return GET_IFNAMEIP_ERROR;
		 }	
		 
		 if (getifaddrs(&ifaddr) == -1)    
		 {	 
			 cmsLog_error("getifaddrs");	 
			 return GET_IFNAMEIP_ERROR;
		 }	 
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)	
		{	 
			if (ifa->ifa_addr == NULL)
				 continue;	 
		
			family = ifa->ifa_addr->sa_family;
		
			 if (strcmp(ifa->ifa_name, ifname) != 0)
			 {
				 continue;
			 }	
			 
			if (family == AF_INET)    
			{	
				 if (ifa->ifa_flags & IFF_UP) 
				 {
					 cmsLog_debug("the interface status is UP");
				 } 
				 else 
				 {
					 cmsLog_error("the interface status is DOWN");
					 freeifaddrs(ifaddr); 
					 return GET_IFNAMEIP_ERROR;   
				 }

				p = inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);
				snprintf(hoststr, 1024, "%s", p);
				cmsLog_debug("host: %s", hoststr);	
				break;
			}	
		}	
		
		freeifaddrs(ifaddr); 
		
		if (!p)
		{
			cmsLog_error("the interface: %s is error", ifname);	 
			return GET_IFNAMEIP_ERROR;
		}
		return GET_IFNAMEIP_SUCCESS;
	}
}

ParseRet ParaseIpv4Domain (const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp)
{
	struct sockaddr_in	cli_addr;
	struct sockaddr_in	serv_addr, back_serv_addr, from;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};
	
	cmsLog_debug("ParaseIpv4Domain begin :%s,requeset dns:%s,dns server:%s",ifname,strdomain,dnsserver);
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	
	bzero(&serv_addr,sizeof(struct sockaddr_in)); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(53); /* 53 */
	if (ifnameGetIPv4addr(ifname, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_aton(tmpBuf, &serv_addr.sin_addr)<0)  
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in)); 
		back_serv_addr.sin_family = AF_INET;
		back_serv_addr.sin_port = htons(53); /* 53 */

		if(inet_aton(dnsSecondary, &back_serv_addr.sin_addr)<0) 
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	//sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(0); 
	
	if(inet_aton(hoststr, &cli_addr.sin_addr)<0)  
	{
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, 0); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))) < 0)
	{
		cmsLog_error("sendto");
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");
		//added by xukeming 2017.1.20 begin
		//optimize dns time
		if (0 == trytime)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}
		else if (1 == trytime)
		{
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		}
		else if (2 == trytime)
		{
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
		}
		else
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
		//added by xukeming 2017.1.20 end
		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime >= MAX_TRY_TIME)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in));
					
						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);      
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime = 0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end
					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						ParseRet ret2 = PARSE_DNS_SUCCESS;
						
						addrlen = sizeof(struct sockaddr); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&from, (socklen_t *)&addrlen); 
						if ((from.sin_addr.s_addr == serv_addr.sin_addr.s_addr)\
							|| ((SwitchBackDns) && (from.sin_addr.s_addr == back_serv_addr.sin_addr.s_addr)))
						{
							cmsLog_debug("m_iRecvDataLen=%d,SwitchBackDns=%d",m_iRecvDataLen,SwitchBackDns);
							
							ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, 0, strdomain, outputip); 
							
							if (PARSE_DNS_SUCCESS != ret2)
							{
								if (haveBackDns) 
								{
									if (SwitchBackDns)
									{
										free(sendPkg.pData);	  
										sendPkg.pData = NULL; 
										close(sockfd);
										return ret2;
									}
									else
									{
										SwitchBackDns = 1;
										trytime = 0;
									}
								}
								else
								{
									free(sendPkg.pData);	  
									sendPkg.pData = NULL; 
									close(sockfd);
									return ret2;
								}
							}

							if (outputip[0] != 0)
							{
								strcpy(domainIp, outputip);
								free(sendPkg.pData);      
								sendPkg.pData = NULL; 
								close(sockfd);
								return PARSE_DNS_SUCCESS;
							}
						}
				}
			break;
		}
	}
}

/* add by caoshaohua, 2015-1-4, get multiple IP, ---begin */

static void processRequestReply1(char *RecvBuf, const int BufLength, const char IsIpv6, const char *strdomain, char *outputip, int *ipNumber)   
{
      int pDataPtr = 0;   
      struct DNS_REQUEST_HEAD dnsHead;   
      struct DNS_REQUEST_TAIL dnsTail;   
      unsigned short usRedirectPtr = 0;      
      unsigned short usRecordType = 0;      
      unsigned short usRecordClass = 0;      
      unsigned short usTrueDataLen = 0;      
      unsigned int iTrueIP = 0;      
      struct in_addr userIP;         
	  char *pData = RecvBuf; 
	  struct in6_addr Temp;
	  char *pstrbegin;
	  char *pstrend;
	  char domainstr[100] = {0},domain[100] = {0};
	  int answerNum = 0;
	  unsigned char found = 0;
	  char ipAddr[128] = {0};
	  
	  cmsLog_debug("processRequestReply begin");  

	  *ipNumber = 0;
	  outputip[0] = '\0';
	  
	  //DNS报头的装载      
      memcpy((void*)&dnsHead, pData+pDataPtr, DNS_HEAD_SIZE);   //pDataPtr=0   
      pDataPtr = pDataPtr + DNS_HEAD_SIZE;   //pDataPtr=12
	  pstrbegin = pData+pDataPtr;
	  
	  //body,temporary jump to End '\0'      
      while((pDataPtr < BufLength) && (0X00 != *(pData + pDataPtr)))      
      {      
          ++pDataPtr;      
      }     
		
      ++pDataPtr;    
	  pstrend = pData+pDataPtr;  
	  memcpy((void*)&domainstr, pstrbegin, (pstrend - pstrbegin));
	  
	  if (domainstr[0] != 0)
	  {
			int i = 0,j,c,z = 0;
			while((domainstr[i] != 0) && (i < sizeof(domainstr)) && (z < sizeof(domainstr)))
			{
				z++;						//防止死循环
				c = domainstr[i];
				for(j = 0; j < c; j ++)
				{
					domain[i+j] = domainstr[i+j+1];
				}
				if (domainstr[i + c + 1] != 0)
				{
					domain[i+j] = '.';
				}
				i = i + c + 1;
			}
	  }
	  cmsLog_debug("domain=%s",domain);
      //报文身体      
      memcpy((void*)&dnsTail, pData + pDataPtr, DNS_TAIL_SIZE);      
      pDataPtr = pDataPtr + DNS_TAIL_SIZE; 
	  
      while(pDataPtr < BufLength)      
      {      
          //DomainStr，重定向报文域名 2个字节      
          //一般为C00C，二进制为1100000000001100   
          //最开始2Bit为11，剩下的Bit构成一个指针，指向DomainStr        
          memcpy(&usRedirectPtr, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usRedirectPtr = ntohs(usRedirectPtr);  
		  
          if(0X00 == usRedirectPtr)      
          {      
			cmsLog_error("Domain name has not been IP, domain name may be illegal");      
			return ;      
          }      
		  
          //类型,2 BYTES      
          memcpy(&usRecordType, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usRecordType = ntohs(usRecordType);    

		  if ((0 == answerNum) && (0xC00C == usRedirectPtr))
		  {
		  	found = 1;
		  }
		  
		  if (answerNum >= dnsHead.usAnswer)
		  {
		  	if (0xC00C == usRedirectPtr)
		  	{	
		  		found = 1;
		  	}
			else
			{
				found = 0;
			}
		  }
		  
          //类,2 BYTES      
          memcpy(&usRecordClass, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usRecordClass = ntohs(usRecordClass);      
          //TTL,4 BYTES      
          pDataPtr = pDataPtr + 4;      
          //后续真实数据长度，2 BYTES      
          memcpy(&usTrueDataLen, pData+pDataPtr, 2);      
          pDataPtr = pDataPtr + 2;      
          usTrueDataLen = ntohs(usTrueDataLen);      
          if (IsIpv6)
          {
	          if((DNS_RRTYPE_AAAA == usRecordType)\
				&& (SendSessionID == dnsHead.usSessionID)\
				&& (!strcmp(strdomain, domain))
				&& (1 == found))   
	          {      
			      if (usTrueDataLen == 16)
				  {
				  	memset(ipAddr, 0, sizeof(ipAddr));
					memcpy(&Temp.s6_addr, pData+pDataPtr, usTrueDataLen);   
					
					//inet_ntop(AF_INET6, &Temp.s6_addr, (char *)&outputip, sizeof(outputip));
					//modified by xuyong
					//can't use memcpy,the end token of string is mixed when there is 2 or more address and the first is longer than the second
					inet_ntop(AF_INET6,&Temp,ipAddr,49);
					if (*ipNumber == 0)
					{
						strcat(outputip, ipAddr);
					}
					else
					{
						strcat(outputip, ",");
						strcat(outputip, ipAddr);
					}
					(*ipNumber) += 1;
					
					cmsLog_debug("outputip=%s ipNumber %d--------------",outputip, *ipNumber);
				  }

	          }  
          }
		  else
		  {
		       if((DNS_RRTYPE_A == usRecordType)\
				&& (SendSessionID == dnsHead.usSessionID)\
				&& (!strcmp(strdomain, domain))
				&& (1 == found))   
	          {      
			      if (usTrueDataLen == 4)
				  {
				  	memset(ipAddr, 0, sizeof(ipAddr));
					memcpy(&iTrueIP, pData+pDataPtr, 4);   
					userIP.s_addr = iTrueIP;  
					
					//memcpy(outputip, inet_ntoa(userIP), strlen(inet_ntoa(userIP))); 
					//modified by xuyong
					//can't use memcpy,the end token of string is mixed when there is 2 or more address and the second is longer than first
					
					inet_ntop(AF_INET,&userIP,ipAddr,16);
					if (*ipNumber == 0)
					{
						strcat(outputip, ipAddr);
					}
					else
					{
						strcat(outputip, ",");
						strcat(outputip, ipAddr);
					}
					(*ipNumber) += 1;
					
					
					cmsLog_debug("outputip=%s ipNumber %d---------------------",outputip, *ipNumber);    
				  }

	          }   
		  }
          pDataPtr = pDataPtr + usTrueDataLen;     
		  answerNum++;
      }      	  
}

ParseRet ParaseIpv6Domain1 (const char *ifname, const char *strdomain, const char *dnsserver, int timeOut, char *domainIp,const char IsIpv6, int *domainIpNumber)
{
	struct sockaddr_in6	cli_addr;
	struct sockaddr_in6	serv_addr, back_serv_addr;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[512] = {0};  
	
	cmsLog_debug("begin");
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	domainIp[0] = '\0';
	*domainIpNumber = 0;
	
	bzero(&serv_addr,sizeof(struct sockaddr_in6)); 
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(53); /* 53 */
	if (ifnameGetIPv6addr(ifname, 0, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_pton(AF_INET6, tmpBuf, &serv_addr.sin6_addr)<0) 
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in6)); 
		back_serv_addr.sin6_family = AF_INET6;
		back_serv_addr.sin6_port = htons(53); /* 53 */

		if(inet_pton(AF_INET6, dnsSecondary, &back_serv_addr.sin6_addr)<0)
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	sockfd=socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in6));
	cli_addr.sin6_family = AF_INET6;
	cli_addr.sin6_port = htons(0); 
	
	if(inet_pton(AF_INET6, hoststr, &cli_addr.sin6_addr)<0)
	{ 
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in6)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, IsIpv6); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6))) < 0)
	{
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");

		timeout.tv_sec = (timeOut > 0) ? timeOut : 5;
		timeout.tv_usec = 0;
	
		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime >= MAX_TRY_TIME)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);	  
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime = 0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end
					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						addrlen = sizeof(struct sockaddr); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&fromv6, (socklen_t *)&addrlen); 
						if (!memcmp(fromv6.sin6_addr.s6_addr,serv_addr.sin6_addr.s6_addr, 16)\
							|| ((SwitchBackDns) && !memcmp(fromv6.sin6_addr.s6_addr,back_serv_addr.sin6_addr.s6_addr, 16)))
						cmsLog_debug("m_iRecvDataLen=%d",m_iRecvDataLen);
						processRequestReply1((char *)&m_szRecvBuf, m_iRecvDataLen, IsIpv6, strdomain, outputip, domainIpNumber); 

						if (outputip[0] != 0)
						{
							strcpy(domainIp, outputip);
							free(sendPkg.pData);      
							sendPkg.pData = NULL; 
							close(sockfd);
							return PARSE_DNS_SUCCESS;
						}
				}
			break;
		}
	}
}

ParseRet ParaseIpv4Domain1 (const char *ifname, const char *strdomain, const char *dnsserver, int timeOut, char *domainIp, int *domainIpNumber)
{
	struct sockaddr_in	cli_addr;
	struct sockaddr_in	serv_addr, back_serv_addr, from;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[512] = {0};
	
	cmsLog_debug("ParaseIpv4Domain begin :%s,requeset dns:%s,dns server:%s",ifname,strdomain,dnsserver);
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	domainIp[0] = '\0';
	*domainIpNumber = 0;
	
	bzero(&serv_addr,sizeof(struct sockaddr_in)); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(53); /* 53 */
	if (ifnameGetIPv4addr(ifname, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_aton(tmpBuf, &serv_addr.sin_addr)<0)  
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in)); 
		back_serv_addr.sin_family = AF_INET;
		back_serv_addr.sin_port = htons(53); /* 53 */

		if(inet_aton(dnsSecondary, &back_serv_addr.sin_addr)<0) 
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	//sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(0); 
	
	if(inet_aton(hoststr, &cli_addr.sin_addr)<0)  
	{
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, 0); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))) < 0)
	{
		cmsLog_error("sendto");
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");

		timeout.tv_sec = (timeOut > 0) ? timeOut : 5;
		timeout.tv_usec = 0;
	
		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime >= MAX_TRY_TIME)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in));
						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);	  
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime=0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end

					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						addrlen = sizeof(struct sockaddr); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&from, (socklen_t *)&addrlen); 
						if ((from.sin_addr.s_addr == serv_addr.sin_addr.s_addr)\
							|| ((SwitchBackDns) && (from.sin_addr.s_addr == back_serv_addr.sin_addr.s_addr)))
						{
							cmsLog_debug("m_iRecvDataLen=%d,SwitchBackDns=%d",m_iRecvDataLen,SwitchBackDns);
							processRequestReply1((char *)&m_szRecvBuf, m_iRecvDataLen, 0, strdomain, outputip, domainIpNumber); 

							if (outputip[0] != 0)
							{
								strcpy(domainIp, outputip);
								free(sendPkg.pData);      
								sendPkg.pData = NULL; 
								close(sockfd);
								return PARSE_DNS_SUCCESS;
							}
						}
				}
			break;
		}
	}
}
/* add by caoshaohua, 2015-1-4, get multiple IP, ---end */

char IsIpv4Addr(char *str)
{
	int i;
	cmsLog_debug("IsIpv4Addr begin");
	for(i = 0; i < strlen(str); i++)
	{
		if (((str[i] >= '0') && (str[i] <= '9'))\
				||(str[i] == '.'))
		{
			continue;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

//Added by xuyong
static int socket_fd =0;

void sendLinkPacket(char * sourceIfName,unsigned short vlanTag,unsigned short protoType)
{	
	char ef[64];/*以太帧缓冲区*/
	struct ethhdr * p_ethhdr;/*以太网头部指针*/	
	unsigned short *p_u16;

	unsigned char eth_dest[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	//unsigned char eth_source[6]={0x02,0x10,0x08,0x01,0x00,0x01};/*目的IP地址*/
	int n;

	struct ifreq req;	
	struct sockaddr sa;

	
	cmsLog_debug("sendLinkPacket begin");
	
	if (socket_fd ==0)
	{
		socket_fd = socket(AF_INET, SOCK_PACKET, htons(protoType));
		if (socket_fd <=0)
		{
			cmsLog_error("fail to create packet socket:");
			return;
		}
	}

	strcpy(req.ifr_name,sourceIfName);  
	if(ioctl(socket_fd,SIOCGIFHWADDR,&req) < 0)  
	{  
		cmsLog_error("error ioctl SIOCGIFHWADDR:");  
		return ;  
	}

	
	
	p_ethhdr = (struct ethhdr*)ef;
	
	memcpy((void *)p_ethhdr->h_dest, (void *)eth_dest, 6);	
	memcpy((void *)p_ethhdr->h_source, (void *)req.ifr_hwaddr.sa_data, 6);
	if (vlanTag != 0)
	{
		
		p_ethhdr->h_proto = htons(0x8100);

		p_u16 = (unsigned short *)(p_ethhdr +1);

		*p_u16 = htons(vlanTag);

		p_u16 ++;

		*p_u16 = htons(protoType);
	}
	else
	{
		p_ethhdr->h_proto = htons(protoType);
	}
	
	//n = write(socket_fd,ef, sizeof(ef));/*发送*/
	
	strcpy(sa.sa_data,"br0"); 
	n=sendto(socket_fd,&ef,64,0,&sa,sizeof(sa));

	if (n < 0)
	{
		cmsLog_error("fail to sendout packet:");
	}
 } 

void stopLinkPacket()
{
	if (socket_fd)
	{
		close(socket_fd);
		socket_fd =0;
	}
}

void dal_putLOIDRegisteringStatus(char * retValue)
{
   char buffer[256]={0};
   int i = 0;
   FILE* fp = NULL;

  if((fp = fopen("/var/PonLinkStatus", "r")) != NULL)
  {
     fgets(buffer, 256, fp);
     fclose(fp);

	   for (i = 0; i < strlen(buffer); i++)
	   {
	   		if (((buffer[i] >= 'a') && (buffer[i] <= 'z')) ||
				((buffer[i] >= 'A') && (buffer[i] <= 'Z')) ||
				((buffer[i] >= '0') && (buffer[i] <= '9')) || buffer[i] == '|')
	   			{
	   				continue;
	   			}
			buffer[i] = 0;
			break;
	   }
	 sprintf(retValue, "%s", buffer);
  }
  else
  {
     sprintf(retValue, "error");
  }
}

void dal_GetAcsAddr(char *AcsAddr)
{
   char buffer[256]={0};
   FILE* fp = NULL;
   int i = 0;
  if((fp = fopen("/var/AcsAddr", "r")) != NULL)
  {
     fgets(buffer, 256, fp);
     fclose(fp);
	 
	 for(i = 0; i < strlen(buffer); i++)
	 {
		if (((buffer[i] < '0') || (buffer[i] > '9')) && (buffer[i] != '.'))
		{
			buffer[i] = 0;
			break;
		}
	 }

	 sprintf(AcsAddr, "%s", buffer);
  }
  else
  {
     sprintf(AcsAddr, "error");
  }
}

//added by xukeming 2015-2-27 begin
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;
	
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	
	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}
	
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

//return 1 == SUCCESS , 0 == FAIL
int socket_ping(const char *local_ipaddr, const char *host, const int Count)
{

	char packet[40] = {0};
	char rev_packet[128] = {0};
	int sockopt;
	struct in_addr 	sin_addr2;		/* IPv4 address */
	struct sockaddr_in mySocketAddress;
	struct sockaddr_in pingaddr;
    int pingsock = -1;	
	struct icmp *pkt;
	int i;
	struct timeval timeout;
    int maxfd;
	fd_set rfds;
	int trytime = 0;
	
	pingsock = socket(AF_INET, SOCK_RAW, 1); /* 1 == ICMP */
	
	if (pingsock < 0) 
	{
		printf("socket_ping open socket fail\n");
		return 0;
	}
	
	memset(&pingaddr, 0, sizeof(struct sockaddr_in));
	
	pingaddr.sin_family = AF_INET;
	
	if (inet_pton(AF_INET, host, (struct in_addr *) &pingaddr.sin_addr) < 0)
	{
		close(pingsock);
		printf("socket_ping inet_pton fail\n");
		return 0;
	}
	
	{
		inet_pton(AF_INET, local_ipaddr, &sin_addr2);
		mySocketAddress.sin_family = AF_INET;
		mySocketAddress.sin_addr = sin_addr2;
		mySocketAddress.sin_port = 0;
		
	   sockopt = 1;
	   if (setsockopt(pingsock, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
	   {
	       printf("socket_ping fail to set sock opt SO_REUSEADDR\n");
		   close(pingsock);
		   return 0;
	   }
	   
	   /* bind() the socket to the interface */
		if (bind(pingsock, (struct sockaddr *)&mySocketAddress, sizeof(mySocketAddress)) < 0)
		{
			printf("socket_ping bind: Could not bind to port\n");
			close(pingsock);
			return 0;
		}
	}

	
	/* enable broadcast pings */
	sockopt = 1;
	setsockopt(pingsock, SOL_SOCKET, SO_BROADCAST, (char *) &sockopt, sizeof(sockopt));
	
	/* set recv buf for broadcast pings */
	sockopt = 48 * 1024;
	setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, (char *) &sockopt, sizeof(sockopt));
	
	
	printf("socket_ping PING %s (%s): %d data bytes\n", host, inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr), sizeof(packet));
	
	/* start the ping's going ... */
	pkt = (struct icmp *) packet;
	
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = 0;
	pkt->icmp_id = getpid() & 0xFFFF;
	
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));
	
	i = sendto(pingsock, packet, sizeof(packet), 0,
		(struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
	
	if (i < 0)
	{
		printf("socket_ping sendto fail\n");
	}
	else if ((size_t)i != sizeof(packet))
	{
		printf("socket_ping sendto length fail\n");
	}
	
	while(1)
	{
	    timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(pingsock, &rfds);
		maxfd = pingsock + 1; 
	
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
			{
				printf("socket_ping interrupt\n");
				close(pingsock);
				return 0;
			}
			case 0:
			{
				printf("socket_ping timeout\n");
				if (trytime < Count)
				{
					i = sendto(pingsock, packet, sizeof(packet), 0,
						(struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
					
					if (i < 0)
					{
						printf("socket_ping sendto fail\n");
					}
					else if ((size_t)i != sizeof(packet))
					{
						printf("socket_ping sendto length fail\n");
					}
					trytime++;
				}
				else
				{
				    close(pingsock);
					return 0;
				}
			}
			break;
			default:
				if (FD_ISSET(pingsock, &rfds)) 
				{
					struct sockaddr_in from;
					socklen_t fromlen = (socklen_t) sizeof(from);
					int c;
					struct icmp *icmppkt;
					struct iphdr *iphdr;
					int hlen;
					
					if ((c = recvfrom(pingsock, rev_packet, sizeof(rev_packet), 0,
						(struct sockaddr *) &from, &fromlen)) < 0) 
					{
						printf("socket_ping recvfrom fail\n");
						break;
					}
					
					/* check IP header */
					iphdr = (struct iphdr *) rev_packet;
					hlen = iphdr->ihl << 2;
					
					c -= hlen;
			
					if (c < 8)
					{
						printf("socket_ping ICMP packets length is less than 8\n");
						break;
					}
					
					icmppkt = (struct icmp *) (rev_packet + hlen);
			
					if ((icmppkt->icmp_type == ICMP_ECHOREPLY) && (icmppkt->icmp_id == (getpid() & 0xFFFF))) 
					{
						close(pingsock);
						return 1;
					} 
				}
			break;
		}
	}
}
//added by xukeming 2015-2-27 end
//end

/*---- transplanted by caoshaohua from 6838 SIP and H248, 2016.2.18,begin ----*/
#define MAC_BCAST_ADDR	"\xff\xff\xff\xff\xff\xff"

struct arpmsg {
	struct ethhdr ethhdr;	 		/* Ethernet header */
	u_short htype;				/* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;				/* protocol type (must be ETH_P_IP) */
	u_char  hlen;				/* hardware address length (must be 6) */
	u_char  plen;				/* protocol address length (must be 4) */
	u_short operation;			/* ARP opcode */
	u_char  sHaddr[6];			/* sender's hardware address */
	u_char  sInaddr[4];			/* sender's IP address */
	u_char  tHaddr[6];			/* target's hardware address */
	u_char  tInaddr[4];			/* target's IP address */
	u_char  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
};

struct ifInfo {
    char ifname[IFNAMSIZ];
    u_long addr;		/* network byte order */
    u_long mask;		/* network byte order */
    u_long bcast;		/* network byte order */
    u_char haddr[6];
    short flags;
};

static void mkArpMsg(int opcode, u_long tInaddr, u_char *tHaddr,
		 u_long sInaddr, u_char *sHaddr, struct arpmsg *msg) {
	bzero(msg, sizeof(*msg));
	bcopy(MAC_BCAST_ADDR, msg->ethhdr.h_dest, 6); /* MAC DA */
	bcopy(sHaddr, msg->ethhdr.h_source, 6);	/* MAC SA */
	msg->ethhdr.h_proto = htons(ETH_P_ARP);	/* protocol type (Ethernet) */
	msg->htype = htons(ARPHRD_ETHER);		/* hardware type */
	msg->ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	msg->hlen = 6;							/* hardware address length */
	msg->plen = 4;							/* protocol address length */
	msg->operation = htons(opcode);			/* ARP op code */
//brcm start
	bcopy((u_char *)&sInaddr, &msg->sInaddr[0], 4);	/* source IP address */
	bcopy(sHaddr, msg->sHaddr, 6);			/* source hardware address */
   bcopy((u_char *)&tInaddr, &msg->tInaddr[0], 4);	/* target IP address */
//brcm end
	if ( opcode == ARPOP_REPLY )
		bcopy(tHaddr, msg->tHaddr, 6);		/* target hardware address */
}

static int openRawSocket (int *s, u_short type) {
	int optval = 1;

	if((*s = socket (PF_PACKET, SOCK_PACKET, htons (type))) == -1) {
		cmsLog_error("Could not open raw socket");
		return -1;
	}
	
	if(setsockopt (*s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval)) == -1) {
		cmsLog_error("Could not setsocketopt on raw socket");
		return -1;
	}
	return 0;
}

unsigned long getclocktime(long *x)
{
	int rc;
	struct timespec ts;
	long localtime = 0;
	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (rc == 0)
	{
	   localtime = ts.tv_sec;
	}
	else
	{
	   cmsLog_debug("clock_gettime failed, set timestamp to 0");
	   if (NULL == x)
	   {
	   		localtime = time(0);
	   }
	   else
	   {
	   		time(x);
	   }
	}
	return localtime;
}


static int arpCheck(u_long inaddr, struct ifInfo *ifbuf, long timeout)  {
	int				s;			/* socket */
	int				rv;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpmsg	arp;
	fd_set			fdset;
	struct timeval	tm;
	time_t			prevTime;

	rv = 1;
	openRawSocket(&s, ETH_P_ARP);

	/* send arp request */
	mkArpMsg(ARPOP_REQUEST, inaddr, NULL, ifbuf->addr, ifbuf->haddr, &arp);
	bzero(&addr, sizeof(addr));
	strcpy(addr.sa_data, ifbuf->ifname);
	if ( sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0 ) rv = 0;
	
	/* wait arp reply, and check it */
	tm.tv_usec = 0;
	prevTime = getclocktime(NULL);
	while ( timeout > 0 ) {
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		tm.tv_sec  = timeout;
		if ( select(s+1, &fdset, (fd_set *)NULL, (fd_set *)NULL, &tm) < 0 ) {
			cmsLog_debug("Error on ARPING request: errno=%d", errno);
			if (errno != EINTR) rv = 0;
		} else if ( FD_ISSET(s, &fdset) ) {
			unsigned int sinaddr = 0;
			if (recv(s, &arp, sizeof(arp), 0) < 0 ) rv = 0;
			sinaddr = ((unsigned int)arp.sInaddr[0] << 24) + ((unsigned int)arp.sInaddr[1] << 16) + ((unsigned int)arp.sInaddr[2] << 8) + (unsigned int)arp.sInaddr[3];
			if(arp.operation == htons(ARPOP_REPLY) && 
			   bcmp(arp.tHaddr, ifbuf->haddr, 6) == 0 && 
			   sinaddr == inaddr ) {
				cmsLog_debug("Valid arp reply receved for this address");
				rv = 0;
				break;
			}
		}
		timeout -= getclocktime(NULL) - prevTime;
		prevTime = getclocktime(NULL);
	}
	close(s);
	cmsLog_debug("%salid arp replies for this address", rv ? "No v" : "V");	 
	return rv;
}

/* args:	yiaddr - what IP to ping (eg. on the NETtel cb189701)
 * retn: 	1 addr free
 *		0 addr used
 *		-1 error 
 */  
int arp_check(char* stryiaddr, char* strip, char *interface, long timeout) {
	struct ifInfo ifbuf;
	int fd, ret;
	struct ifreq ifr;
	unsigned char sMac[6];
	struct sockaddr_in yiaddr;
	struct sockaddr_in ip;

	ret = 1; // default action is to assign the address
	inet_aton(stryiaddr, &yiaddr.sin_addr);
	inet_aton(strip, &ip.sin_addr);

	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		cmsLog_error("socket failed!");
		return ret;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, interface);

	/* Retrieve MAC of the interface */
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
		memcpy(sMac, ifr.ifr_hwaddr.sa_data, 6);
		close(fd);
	}
	else
	{
		close(fd);
		return ret;
	}

	strcpy(ifbuf.ifname, interface);
	ifbuf.addr = ip.sin_addr.s_addr;
	ifbuf.mask = 0x0;
	ifbuf.bcast = 0x0;
	
	memcpy(ifbuf.haddr, sMac, 6);
	ifbuf.flags = 0;
	
	return arpCheck(yiaddr.sin_addr.s_addr, &ifbuf, timeout);
}
/*---- transplanted by caoshaohua from 6838 SIP and H248, 2016.2.18,end ----*/

ParseRet GuangDong_ParaseIpv4Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp)
{
	struct sockaddr_in	cli_addr;
	struct sockaddr_in	serv_addr, back_serv_addr, from;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};
	
	cmsLog_debug("ParaseIpv4Domain begin :%s,requeset dns:%s,dns server:%s",ifname,strdomain,dnsserver);
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	
	bzero(&serv_addr,sizeof(struct sockaddr_in)); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(53); /* 53 */
	if (ifnameGetIPv4addr(ifname, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_aton(tmpBuf, &serv_addr.sin_addr)<0)  
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in)); 
		back_serv_addr.sin_family = AF_INET;
		back_serv_addr.sin_port = htons(53); /* 53 */

		if(inet_aton(dnsSecondary, &back_serv_addr.sin_addr)<0) 
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	//sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(0); 
	
	if(inet_aton(hoststr, &cli_addr.sin_addr)<0)  
	{
		cmsLog_error("client Ip error"); 
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, 0); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))) < 0)
	{
		cmsLog_error("sendto");
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");
		//added by xukeming 2017.1.20 begin
		//optimize dns time
		if (0 == trytime)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}
		else if (1 == trytime)
		{
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		}
		else if (2 == trytime)
		{
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
		}
		else
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
	    //added by xukeming 2017.1.20 end
		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime > 0)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in));

						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);	  
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime = 0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end

					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						ParseRet ret2 = PARSE_DNS_SUCCESS;
						
						addrlen = sizeof(struct sockaddr); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&from, (socklen_t *)&addrlen); 
						if ((from.sin_addr.s_addr == serv_addr.sin_addr.s_addr)\
							|| ((SwitchBackDns) && (from.sin_addr.s_addr == back_serv_addr.sin_addr.s_addr)))
						{
							cmsLog_debug("m_iRecvDataLen=%d,SwitchBackDns=%d",m_iRecvDataLen,SwitchBackDns);
							
							ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, 0, strdomain, outputip); 
							
							if (PARSE_DNS_SUCCESS != ret2)
							{
								if (haveBackDns) 
								{
									if (SwitchBackDns)
									{
										free(sendPkg.pData);	  
										sendPkg.pData = NULL; 
										close(sockfd);
										return ret2;
									}
									else
									{
										SwitchBackDns = 1;
										trytime = 0;
									}
								}
								else
								{
									free(sendPkg.pData);	  
									sendPkg.pData = NULL; 
									close(sockfd);
									return ret2;
								}
							}

							if (outputip[0] != 0)
							{
								strcpy(domainIp, outputip);
								free(sendPkg.pData);      
								sendPkg.pData = NULL; 
								close(sockfd);
								return PARSE_DNS_SUCCESS;
							}
						}
				}
			break;
		}
	}
}

ParseRet GuangDong_ParaseIpv6Domain (const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp,const char IsIpv6)
{
	struct sockaddr_in6	cli_addr;
	struct sockaddr_in6	serv_addr, back_serv_addr;
	int ret;
    int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
    char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
    char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};  
	
	cmsLog_debug("begin");
	if ((NULL == domainIp)||(NULL == ifname)||(NULL == strdomain)||(NULL == dnsserver))
	{
		cmsLog_error("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}
	
	bzero(&serv_addr,sizeof(struct sockaddr_in6)); 
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(53); /* 53 */
	if (ifnameGetIPv6addr(ifname, 0, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");
	
	if (separator != NULL)
    {
		/* break the string into two strings */
		*separator = 0;
		separator++;
		 while ((isspace0(*separator)) && (*separator != 0))
		 {
			/* skip white space after comma */
			separator++;
		 }

		strcpy(dnsSecondary, separator);
		cmsLog_debug("dnsSecondary=%s", dnsSecondary);
    }
	
	if(inet_pton(AF_INET6, tmpBuf, &serv_addr.sin6_addr)<0) 
	{ 
		cmsLog_error("dnsserver error"); 
		return TRANS_DNSSERVER_ERROR; 
	} 
	
	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr,sizeof(struct sockaddr_in6)); 
		back_serv_addr.sin6_family = AF_INET6;
		back_serv_addr.sin6_port = htons(53); /* 53 */

		if(inet_pton(AF_INET6, dnsSecondary, &back_serv_addr.sin6_addr)<0)
		{ 
			cmsLog_error("dnsserver error"); 
			return TRANS_DNSSERVER_ERROR;  
		} 		
	}	
	
	/* 建立 sockfd描述符 */ 
	sockfd=socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n",strerror(errno)); 
		return SOCKET_CREATE_FAIL;
	} 
	
	bzero(&cli_addr,sizeof(struct sockaddr_in6));
	cli_addr.sin6_family = AF_INET6;
	cli_addr.sin6_port = htons(53); 
	
	if(inet_pton(AF_INET6, hoststr, &cli_addr.sin6_addr)<0)
	{ 
		cmsLog_error("client Ip error");
		close(sockfd);
		return CLIENT_IP_ERROR;
	} 
	
   sockopt = 1;
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
   {
       cmsLog_error("fail to set sock opt SO_REUSEADDR");
   }	
   
	if ( bind(sockfd,(struct sockaddr *)&cli_addr,sizeof(struct sockaddr_in6)) <0)
	{
		cmsLog_error("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}
	
	getPackageRequestIP(&sendPkg, strdomain, IsIpv6); 
	
	if ((ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6))) < 0)
	{
		cmsLog_error("send dns request error!");
		free(sendPkg.pData);      
		sendPkg.pData = NULL; 
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		cmsLog_debug("send dns request success!");
	}
	
	while(1)
	{
		cmsLog_debug("begin");
		//added by xukeming 2017.1.20 begin
		//optimize dns time
		if (0 == trytime)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}
		else if (1 == trytime)
		{
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		}
		else if (2 == trytime)
		{
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
		}
		else
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
		//added by xukeming 2017.1.20 end

		FD_ZERO( &rfds );
		FD_SET( sockfd, &rfds );
		maxfd = sockfd + 1;	
		
		switch (select( maxfd, &rfds, NULL, NULL, &timeout))
		{
			case -1:
				free(sendPkg.pData);      
				sendPkg.pData = NULL; 
				close(sockfd);
				return READ_SOCKET_FAIL;
			break;
			case 0:
				cmsLog_debug("timeout ");
				if (trytime < MAX_TRY_TIME)
				{
					if (SwitchBackDns)
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));
					}
					else
					{
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in6));
					}
					
					if (ret < 0)
					{
						cmsLog_error("send dns request error!");
						free(sendPkg.pData);      
						sendPkg.pData = NULL; 
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime++;
						cmsLog_debug("send dns request success!");
						break;
					}
				}
				
				if (trytime > 0)
				{
					if ((haveBackDns) && (!SwitchBackDns))
					{
						SwitchBackDns = 1;
						//added by xukeming 2017.1.20 begin
						//optimize dns time
						ret = sendto(sockfd,sendPkg.pData,sendPkg.iDataLen,0,(struct sockaddr *)&back_serv_addr,sizeof(struct sockaddr_in6));

						if (ret < 0)
						{
							cmsLog_error("send dns request error!");
							free(sendPkg.pData);	  
							sendPkg.pData = NULL; 
							close(sockfd);
							return SEND_DNS_REQUEST_FAIL;
						}
						else
						{
							trytime = 0;
							cmsLog_debug("send dns request success!");
							break;
						}
						//added by xukeming 2017.1.20 end
					}
					free(sendPkg.pData);      
					sendPkg.pData = NULL; 
					close(sockfd);
					return SEND_SUCCESS;
				}
			break;
			default:
				if (FD_ISSET(sockfd, &rfds)) 
				{
						ParseRet ret2 = PARSE_DNS_SUCCESS;
						
						addrlen = sizeof(struct sockaddr_in6); 
						m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&fromv6, (socklen_t *)&addrlen);
						
						 //printf("-----------------fromv6.sin6_ipaddr:"NIP6_FMT"\n", NIP6(fromv6.sin6_addr));
						 //printf("-----------------serv_addr.sin6_ipaddr:"NIP6_FMT"\n", NIP6(serv_addr.sin6_addr));
						 
						if (!memcmp(fromv6.sin6_addr.s6_addr,serv_addr.sin6_addr.s6_addr, 16)\
							|| ((SwitchBackDns) && !memcmp(fromv6.sin6_addr.s6_addr,back_serv_addr.sin6_addr.s6_addr, 16)))
						{
							cmsLog_debug("m_iRecvDataLen=%d",m_iRecvDataLen);
							
							ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, IsIpv6, strdomain, outputip); 
							
							if (PARSE_DNS_SUCCESS != ret2)
							{
								if (haveBackDns) 
								{
									if (SwitchBackDns)
									{
										free(sendPkg.pData);      
										sendPkg.pData = NULL; 
										close(sockfd);
										return ret2;
									}
									else
									{
										SwitchBackDns = 1;
										trytime = 0;
									}
								}
								else
								{
									free(sendPkg.pData);      
									sendPkg.pData = NULL; 
									close(sockfd);
									return ret2;
								}
							}
							
							if (outputip[0] != 0)
							{
								strcpy(domainIp, outputip);
								free(sendPkg.pData);      
								sendPkg.pData = NULL; 
								close(sockfd);
								return PARSE_DNS_SUCCESS;
							}
						}
				}
			break;
		}
	}
}


int lookup(const char *host, char * ipv6addr, uint32_t len)
{
	struct addrinfo hints;
	struct addrinfo *res, *cur;
	int ret;
	
	// struct sockaddr_in *addr;
	
	struct sockaddr_in6 *addr;
	
	char ipbuf[128];

	if (NULL == host 
		|| NULL == ipv6addr
		|| 0 == len)
	{
		return -1;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6; /* Allow IPv4 */
	hints.ai_flags = AI_ALL; /* For wildcard IP address */
	hints.ai_protocol = 0; /* Any protocol */
	hints.ai_socktype = SOCK_DGRAM;

	ret = getaddrinfo(host, NULL, &hints, &res);

	if (ret == -1)
	{
		perror("getaddrinfo");
		return ret;
	}

	for (cur = res; cur != NULL; cur = cur->ai_next)
	{
		addr = (struct sockaddr_in *)cur->ai_addr;
		printf("addr 1 = %s\n", inet_ntop(AF_INET6,
								 &addr->sin6_addr, ipbuf, sizeof(ipbuf)));
		printf("addr 2 = %s ret = %d\n", ipbuf, inet_pton(AF_INET6, ipbuf, &addr->sin6_addr));
	}
	freeaddrinfo(res);
	strncpy(ipv6addr, ipbuf, len);
	return 1;
}

