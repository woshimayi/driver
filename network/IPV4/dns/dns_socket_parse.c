/*
 * @*************************************: 
 * @FilePath: /network/dns/dns_socket_parse.c
 * @version: 
 * @Author: dof
 * @Date: 2022-08-02 11:44:10
 * @LastEditors: dof
 * @LastEditTime: 2022-08-02 15:29:27
 * @Descripttion:  socket 解析 dns 地址
 * @**************************************: 
 */


typedef enum
{
	PARAMETERS_IS_NULL = 0,
	TRANS_DNSSERVER_ERROR,
	SOCKET_CREATE_FAIL,
	CLIENT_IP_ERROR,
	BIND_IP_ERROR,
	SEND_DNS_REQUEST_FAIL,
	READ_SOCKET_FAIL,
	SEND_SUCCESS,
	PARSE_DNS_SUCCESS,
	GET_IFNAMEIP_ERROR,
	GET_IFNAMEIP_SUCCESS,
	GET_DNSSERVER_ERROR,
	GET_DNSSERVER_SUCCESS,
	PARSE_DNS_ERROR,
}ParseRet;


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
	
	/* ���� sockfd������ */ 
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


int main(int argc, char const *argv[])
{
	GuangDong_ParaseIpv6Domain ("veip0.1", "www.tytest6.net", const char *dnsserver, char *domainIp,const char IsIpv6)	
	return 0;
}
