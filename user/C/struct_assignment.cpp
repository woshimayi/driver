#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef char UINT8;

typedef struct
{
    UINT8 SsidNo;
    UINT8 SsidAdmin;
    UINT8 SsidBroadCastAdmin;
    UINT8 SsidName[32];
    UINT8 EncryptMode;
    UINT8 EncryptKey[64]; 
}_OamSvaSsidConfig;


int main()
{
	_OamSvaSsidConfig oamSvaSsidConfig = 
						{
							 .SsidNo = 0x01,
							 .SsidAdmin = 1,
							 .SsidBroadCastAdmin = 1,
							 .SsidName = "JQM-DF8012",
							 .EncryptMode = 0x03,
							 .EncryptKey = "987654321"
						};
	printf("%d, %d, %d, %s %d, %s\n", oamSvaSsidConfig.SsidNo, oamSvaSsidConfig.SsidAdmin, oamSvaSsidConfig.SsidBroadCastAdmin, oamSvaSsidConfig.SsidName, oamSvaSsidConfig.EncryptMode, oamSvaSsidConfig.EncryptKey);
	return 0;
}

