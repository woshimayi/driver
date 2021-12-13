#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef struct
{
	UINT16 Vlan;
	UINT8  pBits;

	UINT8  WanName[MaxWanNameLen];
	UINT8  ConnectionMode;
	UINT8  IpMode;
	UINT8  ServiceType;
	UINT32 BindInterface;
	UINT32 IpV4Addr;
	UINT32 IpV4Mask;
	UINT32 DefaultIpV4GW;
	UINT32 DnsIpV4;
	UINT32 BackUpDnsIpV4;

	UINT8  UserName[MaxWanNameLen];
	UINT8  PassWord[MaxWanPassWord];
} _WanConnectParameter;


int main()
{
	_WanConnectParameter wanConnectParameter = {, 19, 1, 1, 1, 1};
	return 0;
}

